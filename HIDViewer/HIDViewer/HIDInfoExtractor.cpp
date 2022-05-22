#include "HIDInfoExtractor.h"
#include <Windows.h>
#include <stdio.h>
#include <winnt.h>
#include <errno.h>
#include <time.h>
#include <hidsdi.h>
#include <SetupAPI.h>

int Enum_Interface_Device(bool& result, HDEVINFO h_dev_info, GUID& hid_guid, int device_no, SP_DEVICE_INTERFACE_DATA& device_interface_data)
{
	result = SetupDiEnumInterfaceDevice(h_dev_info, 0, &hid_guid, device_no, &device_interface_data);

	if ((result == false) || (GetLastError() == ERROR_NO_MORE_ITEMS)) {
		return 1;
	}

	return 0;
}

int Get_Interface_Device_Detail(bool& result, HDEVINFO h_dev_info, int device_no, SP_DEVICE_INTERFACE_DATA& device_interface_data, PSP_INTERFACE_DEVICE_DETAIL_DATA& device_detail, ULONG& required_length)
{
	required_length = 0;
	SetupDiGetInterfaceDeviceDetail(h_dev_info, &device_interface_data, NULL, 0, &required_length, NULL);
	device_detail = (SP_INTERFACE_DEVICE_DETAIL_DATA*)malloc(required_length);
	device_detail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
	result = SetupDiGetInterfaceDeviceDetail(h_dev_info, &device_interface_data, device_detail, required_length, NULL, NULL);
	return result;
}

std::string Transform_PBuffer_To_String(PWCHAR buffer)
{
	std::wstring temp_wide_string(buffer);
	std::string temp_string(temp_wide_string.begin(), temp_wide_string.end());
	return temp_string;
}

void Get_Manufacturer(HANDLE hid_handle, hid_device_information& new_hid)
{
	PWCHAR manufacturer_buffer = (PWCHAR)malloc(127);

	if (HidD_GetManufacturerString(hid_handle, manufacturer_buffer, 127)) {
		new_hid.manufacturer = Transform_PBuffer_To_String(manufacturer_buffer);
	}

	free(manufacturer_buffer);
}

void Get_Product(HANDLE hid_handle, hid_device_information& new_hid)
{
	PWCHAR product_buffer = (PWCHAR)malloc(127);

	if (HidD_GetProductString(hid_handle, product_buffer, 127)) {
		new_hid.product = Transform_PBuffer_To_String(product_buffer);
	}

	free(product_buffer);
}

void Get_Report_Lengths(HANDLE hid_handle, hid_device_information& new_hid)
{
	PHIDP_PREPARSED_DATA preparsed_data;

	bool result = HidD_GetPreparsedData(hid_handle, &preparsed_data);

	if (result)
	{
		HIDP_CAPS cap;

		if (HidP_GetCaps(preparsed_data, &cap) == HIDP_STATUS_SUCCESS)
		{
			new_hid.input_length = cap.InputReportByteLength;
			new_hid.output_length = cap.OutputReportByteLength;
		}
	}

	HidD_FreePreparsedData(preparsed_data);
}

int HIDInfoExtractor::Extract_HID_Info()
{
#pragma region Define Common Template Parameters
	int device_no = 0;
	GUID hid_guid;
	HDEVINFO h_dev_info;
#pragma endregion
	HidD_GetHidGuid(&hid_guid);

	h_dev_info = SetupDiGetClassDevs(&hid_guid, NULL, NULL, (DIGCF_PRESENT | DIGCF_DEVICEINTERFACE));

	if (h_dev_info == INVALID_HANDLE_VALUE) {
		return 1;
	}

	SetLastError(NO_ERROR);

	hid_device_informations.clear();

#pragma region Extract Loop
	while (1)
	{
		SP_DEVICE_INTERFACE_DATA device_interface_data;
		device_interface_data.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		bool result;

		if (Enum_Interface_Device(result, h_dev_info, hid_guid, device_no, device_interface_data) == 1)
		{
			break;
		}

		PSP_INTERFACE_DEVICE_DETAIL_DATA device_detail;
		ULONG required_length;

		if (Get_Interface_Device_Detail(result, h_dev_info, device_no, device_interface_data, device_detail, required_length) == false) {
			free(device_detail);
			SetupDiDestroyDeviceInfoList(h_dev_info);
			return 1;
		}

		HANDLE hid_handle = CreateFile(device_detail->DevicePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		free(device_detail);

		if (hid_handle == INVALID_HANDLE_VALUE) {
			++device_no;
			continue;
		}

		_HIDD_ATTRIBUTES hidAttributes;
		result = HidD_GetAttributes(hid_handle, &hidAttributes);

		if (result == false) {
			CloseHandle(hid_handle);
			SetupDiDestroyDeviceInfoList(h_dev_info);
			return 1;
		}

		hid_device_information new_hid;
		new_hid.pid = hidAttributes.ProductID;
		new_hid.vid = hidAttributes.VendorID;
		Get_Report_Lengths(hid_handle, new_hid);
		Get_Manufacturer(hid_handle, new_hid);
		Get_Product(hid_handle, new_hid);
		hid_device_informations.push_back(new_hid);
		CloseHandle(hid_handle);
		++device_no;
	}
#pragma endregion

	SetupDiDestroyDeviceInfoList(h_dev_info);
	return 0;
}

void Append_Info_String(std::string& info_string, std::string entry, std::string value)
{
	info_string.append("#");
	info_string.append(entry);
	info_string.append(value);
	info_string.append("    ");
}

std::string HIDInfoExtractor::Get_HID_Info_String()
{
	std::string info_string = "";

	for (auto hid : hid_device_informations)
	{
		Append_Info_String(info_string, "Manufacturer: ", hid.manufacturer);
		Append_Info_String(info_string, "Product: ", hid.product);
		Append_Info_String(info_string, "PID: ", std::to_string(hid.pid));
		Append_Info_String(info_string, "VID: ", std::to_string(hid.vid));
		Append_Info_String(info_string, "Input packet length: ", std::to_string(hid.input_length));
		Append_Info_String(info_string, "Output packet length: ", std::to_string(hid.output_length));
		info_string.append("\r\n");
	}

	return info_string;
}