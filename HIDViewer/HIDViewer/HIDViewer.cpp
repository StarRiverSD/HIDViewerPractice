#include <stdlib.h>
#include "HIDForm.h"

void Run_Form(HIDInfoExtractor* extractor)
{
	HIDViewer::HIDForm form;
	form.Set_HID_Info_Extractor(extractor);
	System::Windows::Forms::Application::Run(% form);
}

int main()
{
	HIDInfoExtractor* extractor = new HIDInfoExtractor();
	Run_Form(extractor);
	system("pause");
	return 0;
}