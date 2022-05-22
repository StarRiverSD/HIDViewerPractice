#pragma once
#include <vector>
#include <string>

struct hid_device_information
{
	std::string manufacturer = "";
	std::string product = "";
	int pid = -1;
	int vid = -1;
	int input_length = -1;
	int output_length = -1;
};

class HIDInfoExtractor
{
protected:
	std::vector<hid_device_information> hid_device_informations;
public:
	int Extract_HID_Info();
	std::string Get_HID_Info_String();
};

