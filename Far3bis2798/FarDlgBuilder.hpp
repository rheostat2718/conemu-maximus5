#pragma once

/*
FarDlgBuilder.hpp

������������ ��������������� �������� - ������ ��� ����������� ������������ � FAR
*/
/*
Copyright � 2010 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "DlgBuilder.hpp"
#include "config.hpp"

struct DialogItemEx;

/*
����� ��� ������������� ���������� ��������, ������������ ������ ���� FAR.
���������� FAR'������ ����� string ��� ������ � ���������� ������.

��� ����, ����� �������� ������� ������������ ����������
��������� �� �����������, ����� ������������ ����� DialogItemEx::Indent().

������������ automation (��������� ������ ������ �������� � ����������� �� ���������
�������). ����������� ��� ������ ������ LinkFlags().
*/
class DialogBuilder: public DialogBuilderBase<DialogItemEx>
{
	private:
		const wchar_t *HelpTopic;

		void LinkFlagsByID(DialogItemEx *Parent, int TargetID, FARDIALOGITEMFLAGS Flags);

	protected:
		virtual void InitDialogItem(DialogItemEx *Item, const TCHAR *Text);
		virtual int TextWidth(const DialogItemEx &Item);
		virtual const TCHAR *GetLangString(int MessageID);
		virtual int DoShowDialog();

		virtual DialogItemBinding<DialogItemEx> *CreateCheckBoxBinding(BOOL* Value, int Mask);
		DialogItemBinding<DialogItemEx> *CreateCheckBoxBinding(IntOption &Value, int Mask);
		DialogItemBinding<DialogItemEx> *CreateCheckBoxBinding(Bool3Option& Value);
		DialogItemBinding<DialogItemEx> *CreateCheckBoxBinding(BoolOption& Value);
		virtual DialogItemBinding<DialogItemEx> *CreateRadioButtonBinding(int *Value);
		DialogItemBinding<DialogItemEx> *CreateRadioButtonBinding(IntOption& Value);

	public:
		DialogBuilder(LNGID TitleMessageId, const wchar_t *HelpTopic);
		DialogBuilder();
		~DialogBuilder();

		// ��������� ���� ���� DI_EDIT ��� �������������� ���������� ���������� ��������.
		DialogItemEx *AddEditField(string *Value, int Width, const wchar_t *HistoryID = nullptr, FARDIALOGITEMFLAGS Flags = 0);
		DialogItemEx *AddEditField(StringOption& Value, int Width, const wchar_t *HistoryID = nullptr, FARDIALOGITEMFLAGS Flags = 0);

		// ��������� ���� ���� DI_FIXEDIT ��� �������������� ���������� ���������� ��������.
		DialogItemEx *AddFixEditField(string *Value, int Width, const wchar_t *Mask = nullptr);
		DialogItemEx *AddFixEditField(StringOption& Value, int Width, const wchar_t *Mask = nullptr);

		// ��������� ������������ ���� ���� DI_EDIT ��� �������� ���������� ���������� ��������.
		DialogItemEx *AddConstEditField(const wchar_t* Value, int Width, FARDIALOGITEMFLAGS Flags = 0);

		// ��������� ���� ���� DI_FIXEDIT ��� �������������� ���������� ��������� ��������.
		virtual DialogItemEx *AddIntEditField(int *Value, int Width);
		virtual DialogItemEx *AddIntEditField(IntOption& Value, int Width);
		virtual DialogItemEx *AddHexEditField(IntOption& Value, int Width);

		// ��������� ���������� ������ � ���������� ����������.
		DialogItemEx *AddComboBox(int *Value, int Width, DialogBuilderListItem *Items, size_t ItemCount, FARDIALOGITEMFLAGS Flags = DIF_NONE);
		DialogItemEx *AddComboBox(IntOption& Value, int Width, DialogBuilderListItem *Items, size_t ItemCount, FARDIALOGITEMFLAGS Flags = DIF_NONE);

		DialogItemEx *AddCheckbox(int TextMessageId, BOOL *Value, int Mask=0, bool ThreeState=false)
		{
			return DialogBuilderBase<DialogItemEx>::AddCheckbox(TextMessageId, Value, Mask, ThreeState);
		}
		DialogItemEx *AddCheckbox(int TextMessageId, IntOption& Value, int Mask=0, bool ThreeState=false);
		DialogItemEx *AddCheckbox(int TextMessageId, Bool3Option& Value);
		DialogItemEx *AddCheckbox(int TextMessageId, BoolOption& Value);
		DialogItemEx *AddCheckbox(const wchar_t* Caption, BoolOption& Value);


		void AddRadioButtons(int *Value, int OptionCount, const int MessageIDs[], bool FocusOnSelected=false)
		{
			return DialogBuilderBase<DialogItemEx>::AddRadioButtons(Value, OptionCount, MessageIDs, FocusOnSelected);
		}
		void AddRadioButtons(IntOption& Value, int OptionCount, const int MessageIDs[], bool FocusOnSelected=false);


		// ��������� ��������� ��������� Parent � Target. ����� Parent->Selected �����
		// false, ������������� ����� Flags � �������� Target; ����� ����� true -
		// ���������� �����.
		// ���� LinkLabels ����������� � true, �� ��������� ��������, ����������� � �������� Target
		// �������� AddTextBefore � AddTextAfter, ����� ����������� � ��������� Parent.
		void LinkFlags(DialogItemEx *Parent, DialogItemEx *Target, FARDIALOGITEMFLAGS Flags, bool LinkLabels=true);

		void AddOKCancel()
		{
			DialogBuilderBase<DialogItemEx>::AddOKCancel(MOk, MCancel);
		}

		void AddOK()
		{
			DialogBuilderBase<DialogItemEx>::AddOKCancel(MOk, -1);
		}
};
