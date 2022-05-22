#include "HIDForm.h"
#include <iostream>

namespace HIDViewer
{
	System::Void HIDForm::refresh_button_Click(System::Object^ sender, System::EventArgs^ e)
	{
		Launch_Extraction();
	}

	void HIDForm::Set_HID_Info_Extractor(HIDInfoExtractor* extractor)
	{
		this->extractor = extractor;
	}

	void HIDForm::Set_HID_Textbox(std::string hid_info)
	{
		hid_list_textbox->Text = gcnew String(hid_info.c_str());
	}

	void HIDForm::Launch_Extraction()
	{
		if (extractor->Extract_HID_Info() == 0)
		{
			Set_HID_Textbox(extractor->Get_HID_Info_String());
		}
		else
		{
			hid_list_textbox->Text = "Error";
		}
	}
}

