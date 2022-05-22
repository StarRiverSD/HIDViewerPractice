#pragma once
#include <string>
#include "HIDInfoExtractor.h"

namespace HIDViewer {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// HIDForm ���K�n
	/// </summary>
	public ref class HIDForm : public System::Windows::Forms::Form
	{
	public:
		HIDForm(void)
		{
			InitializeComponent();
			//
			//TODO:  �b���[�J�غc�禡�{���X
			//
		}

	protected:
		/// <summary>
		/// �M������ϥΤ����귽�C
		/// </summary>
		~HIDForm()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::TextBox^ hid_list_textbox;
	protected:
	private: System::Windows::Forms::Button^ refresh_button;
	private: System::Windows::Forms::Label^ hid_title_lable;


	protected:

	private:
		/// <summary>
		/// �]�p�u��һݪ��ܼơC
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// �����]�p�u��䴩�һݪ���k - �ФŨϥε{���X�s�边�ק�
		/// �o�Ӥ�k�����e�C
		/// </summary>
		void InitializeComponent(void)
		{
			this->hid_list_textbox = (gcnew System::Windows::Forms::TextBox());
			this->refresh_button = (gcnew System::Windows::Forms::Button());
			this->hid_title_lable = (gcnew System::Windows::Forms::Label());
			this->SuspendLayout();
			// 
			// hid_list_textbox
			// 
			this->hid_list_textbox->Font = (gcnew System::Drawing::Font(L"Arial", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->hid_list_textbox->Location = System::Drawing::Point(26, 70);
			this->hid_list_textbox->Multiline = true;
			this->hid_list_textbox->Name = L"hid_list_textbox";
			this->hid_list_textbox->ReadOnly = true;
			this->hid_list_textbox->ScrollBars = System::Windows::Forms::ScrollBars::Vertical;
			this->hid_list_textbox->Size = System::Drawing::Size(1358, 498);
			this->hid_list_textbox->TabIndex = 1;
			// 
			// refresh_button
			// 
			this->refresh_button->Location = System::Drawing::Point(1309, 600);
			this->refresh_button->Name = L"refresh_button";
			this->refresh_button->Size = System::Drawing::Size(75, 23);
			this->refresh_button->TabIndex = 2;
			this->refresh_button->Text = L"Refresh";
			this->refresh_button->UseVisualStyleBackColor = true;
			this->refresh_button->Click += gcnew System::EventHandler(this, &HIDForm::refresh_button_Click);
			// 
			// hid_title_lable
			// 
			this->hid_title_lable->AutoSize = true;
			this->hid_title_lable->Font = (gcnew System::Drawing::Font(L"Arial", 18, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->hid_title_lable->Location = System::Drawing::Point(32, 22);
			this->hid_title_lable->Name = L"hid_title_lable";
			this->hid_title_lable->Size = System::Drawing::Size(96, 27);
			this->hid_title_lable->TabIndex = 3;
			this->hid_title_lable->Text = L"HID List";
			// 
			// HIDForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 12);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(1405, 651);
			this->Controls->Add(this->hid_title_lable);
			this->Controls->Add(this->refresh_button);
			this->Controls->Add(this->hid_list_textbox);
			this->Name = L"HIDForm";
			this->Text = L"HIDForm";
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private:
		System::Void refresh_button_Click(System::Object^ sender, System::EventArgs^ e);
		HIDInfoExtractor* extractor;
	public:
		void Set_HID_Info_Extractor(HIDInfoExtractor* extractor);
		void Set_HID_Textbox(std::string hid_info);
		void Launch_Extraction();
	};
}
