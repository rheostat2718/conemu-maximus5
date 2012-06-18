/*
codepage.cpp

������ � �������� ����������
*/
/*
Copyright � 1996 Eugene Roshal
Copyright � 2000 Far Group
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

#include "headers.hpp"
#pragma hdrstop

#include "codepage.hpp"
#include "vmenu.hpp"
#include "keys.hpp"
#include "language.hpp"
#include "dialog.hpp"
#include "interf.hpp"
#include "config.hpp"
#include "configdb.hpp"

// ���� ��� �������� ����� ������� �������
const wchar_t *NamesOfCodePagesKey = L"CodePages.Names";

const wchar_t *FavoriteCodePagesKey = L"CodePages.Favorites";

// ����������� ������� ��������
enum StandardCodePages
{
	SearchAll = 1,
	Auto = 2,
	OEM = 4,
	ANSI = 8,
	UTF7 = 16,
	UTF8 = 32,
	UTF16LE = 64,
	UTF16BE = 128,
	AllStandard = OEM | ANSI | UTF7 | UTF8 | UTF16BE | UTF16LE,
	DefaultCP = 256
};

// �������� ������ �������� ������� �� ������� ���������
enum CodePagesCallbackCallSource
{
	CodePageSelect,
	CodePagesFill,
	CodePageCheck
};

// ������ ��������� ������� �������������� ����� ������� ��������
enum
{
	EDITCP_BORDER,
	EDITCP_EDIT,
	EDITCP_SEPARATOR,
	EDITCP_OK,
	EDITCP_CANCEL,
	EDITCP_RESET,
};

// ������
static HANDLE dialog;
// ������������ �������
static UINT control;
// ����
static VMenu *CodePages = nullptr;
// ������� ������� ��������
static UINT currentCodePage;
// ���������� ��������� � ������������ ������ ��������
static int favoriteCodePages, normalCodePages;
// ������� ������������� ���������� ������� �������� ��� ������
static bool selectedCodePages;
// �������� ������ �������� ��� ������� EnumSystemCodePages
static CodePagesCallbackCallSource CallbackCallSource;
// ������� ����, ��� ������� �������� ��������������
static bool CodePageSupported;

wchar_t *FormatCodePageName(UINT CodePage, wchar_t *CodePageName, size_t Length, bool &IsCodePageNameCustom);

// �������� ������� �������� ��� �������� � ����
inline UINT GetMenuItemCodePage(int Position = -1)
{
	void* Data = CodePages->GetUserData(nullptr, 0, Position);
	return Data? *static_cast<UINT*>(Data) : 0;
}

inline UINT GetListItemCodePage(int Position = -1)
{
	intptr_t Data = SendDlgMessage(dialog, DM_LISTGETDATA, control, ToPtr(Position));
	return Data? *reinterpret_cast<UINT*>(Data) : 0;
}

// ��������� �������� ��� ��� ������� � �������� ����������� ������� ������� (������������ ������ ��� ������������ �� �������������)
inline bool IsPositionStandard(UINT position)
{
	return position<=(UINT)CodePages->GetItemCount()-favoriteCodePages-(favoriteCodePages?1:0)-normalCodePages-(normalCodePages?1:0);
}

// ��������� �������� ��� ��� ������� � �������� ������� ������� ������� (������������ ������ ��� ������������ �� �������������)
inline bool IsPositionFavorite(UINT position)
{
	return position>=(UINT)CodePages->GetItemCount()-normalCodePages;
}

// ��������� �������� ��� ��� ������� � �������� ������������ ������� ������� (������������ ������ ��� ������������ �� �������������)
inline bool IsPositionNormal(UINT position)
{
	UINT ItemCount = CodePages->GetItemCount();
	return position>=ItemCount-normalCodePages-favoriteCodePages-(normalCodePages?1:0) && position<ItemCount-normalCodePages;
}

// ��������� ������ ��� ����������� ������������� ������� ��������
void FormatCodePageString(UINT CodePage, const wchar_t *CodePageName, FormatString &CodePageNameString, bool IsCodePageNameCustom)
{
	if (static_cast<int>(CodePage) >= 0)  // CodePage != CP_DEFAULT, CP_REDETECT
	{
		CodePageNameString<<fmt::MinWidth(5)<<CodePage<<BoxSymbols[BS_V1]<<(!IsCodePageNameCustom||CallbackCallSource==CodePagesFill?L' ':L'*');
	}
	CodePageNameString<<CodePageName;
}

// ��������� ������� ��������
void AddCodePage(const wchar_t *codePageName, UINT codePage, int position, bool enabled, bool checked, bool IsCodePageNameCustom)
{
	if (CallbackCallSource == CodePagesFill)
	{
		// ��������� ������� ������������ ��������
		if (position==-1)
		{
			FarListInfo info={sizeof(FarListInfo)};
			SendDlgMessage(dialog, DM_LISTINFO, control, &info);
			position = static_cast<int>(info.ItemsNumber);
		}

		// ��������� �������
		FarListInsert item = {sizeof(FarListInsert),position};

		FormatString name;
		FormatCodePageString(codePage, codePageName, name, IsCodePageNameCustom);
		item.Item.Text = name;

		if (selectedCodePages && checked)
		{
			item.Item.Flags |= MIF_CHECKED;
		}

		if (!enabled)
		{
			item.Item.Flags |= MIF_GRAYED;
		}

		SendDlgMessage(dialog, DM_LISTINSERT, control, &item);
		// ������������� ������ ��� ��������
		FarListItemData data={sizeof(FarListItemData)};
		data.Index = position;
		data.Data = &codePage;
		data.DataSize = sizeof(codePage);
		SendDlgMessage(dialog, DM_LISTSETDATA, control, &data);
	}
	else
	{
		// ������ ����� ������� ����
		MenuItemEx item;
		item.Clear();

		if (!enabled)
			item.Flags |= MIF_GRAYED;

		FormatString name;
		FormatCodePageString(codePage, codePageName, name, IsCodePageNameCustom);
		item.strName = name;

		item.UserData = &codePage;
		item.UserDataSize = sizeof(codePage);

		// ��������� ����� ������� � ����
		if (position>=0)
			CodePages->AddItem(&item, position);
		else
			CodePages->AddItem(&item);

		// ���� ���� ������������� ������ �� ����������� �������
		if (currentCodePage==codePage)
		{
			if ((CodePages->GetSelectPos()==-1 || GetMenuItemCodePage()!=codePage))
			{
				CodePages->SetSelectPos(position>=0?position:CodePages->GetItemCount()-1, 1);
			}
		}
	}
}

// ��������� ����������� ������� ��������
void AddStandardCodePage(const wchar_t *codePageName, UINT codePage, int position = -1, bool enabled = true)
{
	bool checked = false;

	if (selectedCodePages && codePage!=CP_DEFAULT)
	{
		int selectType = 0;
		GeneralCfg->GetValue(FavoriteCodePagesKey, FormatString() << codePage, &selectType, 0);

		if (selectType & CPST_FIND)
			checked = true;
	}

	AddCodePage(codePageName, codePage, position, enabled, checked, false);
}

// ��������� �����������
void AddSeparator(LPCWSTR Label=nullptr,int position = -1)
{
	if (CallbackCallSource == CodePagesFill)
	{
		if (position==-1)
		{
			FarListInfo info={sizeof(FarListInfo)};
			SendDlgMessage(dialog, DM_LISTINFO, control, &info);
			position = static_cast<int>(info.ItemsNumber);
		}

		FarListInsert item = {sizeof(FarListInsert),position};
		item.Item.Text = Label;
		item.Item.Flags = LIF_SEPARATOR;
		SendDlgMessage(dialog, DM_LISTINSERT, control, &item);
	}
	else
	{
		MenuItemEx item;
		item.Clear();
		item.strName = Label;
		item.Flags = MIF_SEPARATOR;

		if (position>=0)
			CodePages->AddItem(&item, position);
		else
			CodePages->AddItem(&item);
	}
}

// �������� ���������� ��������� � ������
int GetItemsCount()
{
	if (CallbackCallSource == CodePageSelect)
	{
		return CodePages->GetItemCount();
	}
	else
	{
		FarListInfo info={sizeof(FarListInfo)};
		SendDlgMessage(dialog, DM_LISTINFO, control, &info);
		return static_cast<int>(info.ItemsNumber);
	}
}

// �������� ������� ��� ������� ������� � ������ ���������� �� ������ ������� ��������
int GetCodePageInsertPosition(UINT codePage, int start, int length)
{
	for (int position=start; position < start+length; position++)
	{
		UINT itemCodePage;

		if (CallbackCallSource == CodePageSelect)
			itemCodePage = GetMenuItemCodePage(position);
		else
			itemCodePage = GetListItemCodePage(position);

		if (itemCodePage >= codePage)
			return position;
	}

	return start+length;
}

// �������� ���������� � ������� ��������
bool GetCodePageInfo(UINT CodePage, CPINFOEX &CodePageInfoEx)
{
	if (!GetCPInfoEx(CodePage, 0, &CodePageInfoEx))
	{
		// GetCPInfoEx ���������� ������ ��� ������� ������� ��� ����� (�������� 1125), �������
		// ���� �� ���� ��������. ��� ���, ������ ��� ���������� ������� �������� ��-�� ������,
		// ������� �������� ��� �� ���������� ����� GetCPInfo
		CPINFO CodePageInfo;

		if (!GetCPInfo(CodePage, &CodePageInfo))
			return false;

		CodePageInfoEx.MaxCharSize = CodePageInfo.MaxCharSize;
		CodePageInfoEx.CodePageName[0] = L'\0';
	}

	// BUBUG: ���� �� ������������ ������������� ������� ��������
	if (CodePageInfoEx.MaxCharSize != 1)
		return false;

	return true;
}

// Callback-������� ��������� ������ ��������
BOOL WINAPI EnumCodePagesProc(const wchar_t *lpwszCodePage)
{
	UINT codePage = _wtoi(lpwszCodePage);

	// ��� ������� �������� ��� �� ���������� ���������� � ������� ��������� �������� �� �����������
	if (CallbackCallSource == CodePageCheck && codePage != currentCodePage)
		return TRUE;

	// �������� ���������� � ������� ��������. ���� ���������� �� �����-���� ������� �������� �� �������, ��
	// ��� ������� ���������� ����������, � ��� ��������� �� �������� ���������������� ������� �������� �������
	CPINFOEX cpiex;
	if (!GetCodePageInfo(codePage, cpiex))
		return CallbackCallSource == CodePageCheck ? FALSE : TRUE;

	// ��� ������� �������� ���������������� ������� �������� �� ������ ��� �������� � ����� ��������
	if (CallbackCallSource == CodePageCheck)
	{
		CodePageSupported = true;
		return FALSE;
	}

	// ��������� ��� ������ ��������
	bool IsCodePageNameCustom = false;
	wchar_t *codePageName = FormatCodePageName(_wtoi(lpwszCodePage), cpiex.CodePageName, sizeof(cpiex.CodePageName)/sizeof(wchar_t), IsCodePageNameCustom);
	// �������� ������� ����������� ������� ��������
	int selectType = 0;
	GeneralCfg->GetValue(FavoriteCodePagesKey, lpwszCodePage, &selectType, 0);

	// ��������� ������� �������� ���� � ����������, ���� � ��������� ������� ��������
	if (selectType & CPST_FAVORITE)
	{
		// ���� ���� ��������� ����������� ����� ���������� � ����������� ��������� ��������
		if (!favoriteCodePages)
			AddSeparator(MSG(MGetCodePageFavorites),GetItemsCount()-normalCodePages-(normalCodePages?1:0));

		// ��������� ������� �������� � ���������
		AddCodePage(
		    codePageName,
		    codePage,
		    GetCodePageInsertPosition(
		        codePage,
		        GetItemsCount()-normalCodePages-favoriteCodePages-(normalCodePages?1:0),
		        favoriteCodePages
		    ),
		    true,
		    selectType & CPST_FIND ? true : false,
			IsCodePageNameCustom
		);
		// ����������� ������� ��������� ������ ��������
		favoriteCodePages++;
	}
	else if (CallbackCallSource == CodePagesFill || !Opt.CPMenuMode)
	{
		// ��������� ����������� ����� ������������ � ���������� ��������� ��������
		if (!favoriteCodePages && !normalCodePages)
			AddSeparator(MSG(MGetCodePageOther));

		// ��������� ������� �������� � ����������
		AddCodePage(
		    codePageName,
		    codePage,
		    GetCodePageInsertPosition(
		        codePage,
		        GetItemsCount()-normalCodePages,
		        normalCodePages
		    ),
			true,
			false,
			IsCodePageNameCustom
		);
		// ����������� ������� ��������� ������ ��������
		normalCodePages++;
	}

	return TRUE;
}

// ��������� ��� ����������� ������� ��������
void AddCodePages(DWORD codePages)
{
	// ��������� ����������� ������� ��������

	UINT cp_auto = CP_DEFAULT;
	if ( 0 != (codePages & ::DefaultCP) )
	{
		AddStandardCodePage(MSG(MDefaultCP), CP_DEFAULT, -1, true);
		cp_auto = CP_REDETECT;
	}
	AddStandardCodePage((codePages & ::SearchAll) ? MSG(MFindFileAllCodePages) : MSG(MEditOpenAutoDetect), cp_auto, -1, (codePages & (::SearchAll | ::Auto)) != 0);
	AddSeparator(MSG(MGetCodePageSystem));
	AddStandardCodePage(L"OEM", GetOEMCP(), -1, (codePages & ::OEM) != 0);
	AddStandardCodePage(L"ANSI", GetACP(), -1, (codePages & ::ANSI) != 0);
	AddSeparator(MSG(MGetCodePageUnicode));
	if (codePages & ::UTF7) AddStandardCodePage(L"UTF-7", CP_UTF7, -1, true); //?? �� ��������������, �� � ����� ��?
	AddStandardCodePage(L"UTF-8", CP_UTF8, -1, (codePages & ::UTF8) != 0);
	AddStandardCodePage(L"UTF-16 (Little endian)", CP_UNICODE, -1, (codePages & ::UTF16LE) != 0);
	AddStandardCodePage(L"UTF-16 (Big endian)", CP_REVERSEBOM, -1, (codePages & ::UTF16BE) != 0);
	// �������� ������� �������� ������������� � �������
	EnumSystemCodePages((CODEPAGE_ENUMPROCW)EnumCodePagesProc, CP_INSTALLED);
}

// ��������� ����������/�������� �/�� ������ ��������� ������ ��������
void ProcessSelected(bool select)
{
	if (Opt.CPMenuMode && select)
		return;

	UINT itemPosition = CodePages->GetSelectPos();
	UINT codePage = GetMenuItemCodePage();

	if ((select && IsPositionFavorite(itemPosition)) || (!select && IsPositionNormal(itemPosition)))
	{
		// ����������� ����� ������� �������� � ������
		FormatString strCPName;
		strCPName<<codePage;
		// �������� ������� ��������� ����� � �������
		int selectType = 0;
		GeneralCfg->GetValue(FavoriteCodePagesKey, strCPName, &selectType, 0);

		// �������/��������� � �������� ���������� � ��������� ������� ��������
		if (select)
			GeneralCfg->SetValue(FavoriteCodePagesKey, strCPName, CPST_FAVORITE | (selectType & CPST_FIND ? CPST_FIND : 0));
		else if (selectType & CPST_FIND)
			GeneralCfg->SetValue(FavoriteCodePagesKey, strCPName, CPST_FIND);
		else
			GeneralCfg->DeleteValue(FavoriteCodePagesKey, strCPName);

		// ������ ����� ������� ����
		MenuItemEx newItem;
		newItem.Clear();
		newItem.strName = CodePages->GetItemPtr()->strName;
		newItem.UserData = &codePage;
		newItem.UserDataSize = sizeof(codePage);
		// ��������� ������� �������
		int position=CodePages->GetSelectPos();
		// ������� ������ ����� ����
		CodePages->DeleteItem(CodePages->GetSelectPos());

		// ��������� ����� ���� � ����� �����
		if (select)
		{
			// ��������� �����������, ���� ��������� ������� ������� ��� �� ����
			// � ����� ���������� ��������� ���������� ������� ��������
			if (!favoriteCodePages && normalCodePages>1)
				AddSeparator(MSG(MGetCodePageFavorites),CodePages->GetItemCount()-normalCodePages);

			// ���� �������, ���� �������� �������
			int newPosition = GetCodePageInsertPosition(
			                      codePage,
			                      CodePages->GetItemCount()-normalCodePages-favoriteCodePages,
			                      favoriteCodePages
			                  );
			// ��������� ������� �������� � ���������
			CodePages->AddItem(&newItem, newPosition);

			// ������� �����������, ���� ��� ������������ ������� �������
			if (normalCodePages==1)
				CodePages->DeleteItem(CodePages->GetItemCount()-1);

			// �������� �������� ���������� � ��������� ������� �������
			favoriteCodePages++;
			normalCodePages--;
			position++;
		}
		else
		{
			// ������� ����������, ���� ����� �������� �� ���������� �� �����
			// ��������� ������� ��������
			if (favoriteCodePages==1 && normalCodePages>0)
				CodePages->DeleteItem(CodePages->GetItemCount()-normalCodePages-2);

			// ��������� ������� � ���������� �������, ������ ���� ��� ������������
			if (!Opt.CPMenuMode)
			{
				// ��������� �����������, ���� �� ���� �� ����� ���������� ������� ��������
				if (!normalCodePages)
					AddSeparator(MSG(MGetCodePageOther));

				// ��������� ������� �������� � ����������
				CodePages->AddItem(
				    &newItem,
				    GetCodePageInsertPosition(
				        codePage,
				        CodePages->GetItemCount()-normalCodePages,
				        normalCodePages
				    )
				);
				normalCodePages++;
			}
			// ���� � ������ ������� ���������� ������ �� ������� ��������� ��������� �������, �� ������� � �����������
			else if (favoriteCodePages==1)
				CodePages->DeleteItem(CodePages->GetItemCount()-normalCodePages-1);

			favoriteCodePages--;

			if (position==CodePages->GetItemCount()-normalCodePages-1)
				position--;
		}

		// ������������� ������� � ����
		CodePages->SetSelectPos(position>=CodePages->GetItemCount() ? CodePages->GetItemCount()-1 : position, 1);

		// ���������� ����
		if (Opt.CPMenuMode)
			CodePages->SetPosition(-1, -1, 0, 0);

		CodePages->Show();
	}
}

// ��������� ���� ������ ������ ��������
void FillCodePagesVMenu(bool bShowUnicode, bool bShowUTF, bool bShowUTF7, bool bShowAutoDetect=false)
{
	UINT codePage = currentCodePage;

	if (CodePages->GetSelectPos()!=-1 && CodePages->GetSelectPos()<CodePages->GetItemCount()-normalCodePages)
		currentCodePage = GetMenuItemCodePage();

	// ������� ����
	favoriteCodePages = normalCodePages = 0;
	CodePages->DeleteItems();

	UnicodeString title = MSG(MGetCodePageTitle);
	if (Opt.CPMenuMode)
		title += L" *";
	CodePages->SetTitle(title);

	// ��������� ������� ��������
	// BUBUG: ����� ��������� ��������� UTF7 �������� bShowUTF7 ����� ������ ��������
	AddCodePages(::OEM | ::ANSI
		| (bShowUTF ? ::UTF8 : 0)
		| (bShowUTF7 ? ::UTF7 : 0)
		| (bShowUnicode ? (::UTF16BE | ::UTF16LE) : 0)
		| (bShowAutoDetect ? ::Auto : 0)
	);
	// ��������������� ����������� ������� ��������
	currentCodePage = codePage;
	// ������������� ����
	CodePages->SetPosition(-1, -1, 0, 0);
	// ���������� ����
	CodePages->Show();
}

// ����������� ��� ������� ��������
wchar_t *FormatCodePageName(UINT CodePage, wchar_t *CodePageName, size_t Length)
{
	bool IsCodePageNameCustom;
	return FormatCodePageName(CodePage, CodePageName, Length, IsCodePageNameCustom);
}

// ����������� ��� ������� ��������
wchar_t *FormatCodePageName(UINT CodePage, wchar_t *CodePageName, size_t Length, bool &IsCodePageNameCustom)
{
	if (!CodePageName || !Length)
		return CodePageName;

	// �������� �������� �������� ������������� ��� ������� ��������
	FormatString strCodePage;
	strCodePage<<CodePage;
	string strCodePageName;
	if (GeneralCfg->GetValue(NamesOfCodePagesKey, strCodePage, strCodePageName, L""))
	{
		Length = Min(Length-1, strCodePageName.GetLength());
		IsCodePageNameCustom = true;
	}
	else
		IsCodePageNameCustom = false;
	if (*CodePageName)
	{
		// ��� ������ �� ����� "XXXX (Name)", �, ��������, ��� wine ������ "Name"
		wchar_t *Name = wcschr(CodePageName, L'(');
		if (Name && *(++Name))
		{
			size_t NameLength = wcslen(Name)-1;
			if (Name[NameLength] == L')')
			{
				Name[NameLength] = L'\0';
			}
		}
		if (IsCodePageNameCustom)
		{
			if (strCodePageName==Name)
			{
				GeneralCfg->DeleteValue(NamesOfCodePagesKey, strCodePage);
				IsCodePageNameCustom = false;
				return Name;
			}
		}
		else
			return Name;
	}
	if (IsCodePageNameCustom)
	{
		wmemcpy(CodePageName, strCodePageName, Length);
		CodePageName[Length] = L'\0';
	}
	return CodePageName;
}

// ������� ��� ������� �������������� ����� ������� ��������
intptr_t WINAPI EditDialogProc(HANDLE hDlg, int Msg, int Param1, void* Param2)
{
	if (Msg==DN_CLOSE)
	{
		if (Param1==EDITCP_OK || Param1==EDITCP_RESET)
		{
			string strCodePageName;
			UINT CodePage = GetMenuItemCodePage();
			FormatString strCodePage;
			strCodePage<<CodePage;
			if (Param1==EDITCP_OK)
			{
				wchar_t *CodePageName = strCodePageName.GetBuffer(SendDlgMessage(hDlg, DM_GETTEXTPTR, EDITCP_EDIT, 0)+1);
				SendDlgMessage(hDlg, DM_GETTEXTPTR, EDITCP_EDIT, CodePageName);
				strCodePageName.ReleaseBuffer();
			}
			// ���� ��� ������� �������� ������, �� �������, ��� ��� �� ������
			if (!strCodePageName.GetLength())
				GeneralCfg->DeleteValue(NamesOfCodePagesKey, strCodePage);
			else
				GeneralCfg->SetValue(NamesOfCodePagesKey, strCodePage, strCodePageName);
			// �������� ���������� � ������� ��������
			CPINFOEX cpiex;
			if (GetCodePageInfo(CodePage, cpiex))
			{
				// ��������� ��� ������ ��������
				bool IsCodePageNameCustom = false;
				wchar_t *CodePageName = FormatCodePageName(CodePage, cpiex.CodePageName, sizeof(cpiex.CodePageName)/sizeof(wchar_t), IsCodePageNameCustom);
				// ��������� ������ �������������
				strCodePage.Clear();
				FormatCodePageString(CodePage, CodePageName, strCodePage, IsCodePageNameCustom);
				// ��������� ��� ������� ��������
				int Position = CodePages->GetSelectPos();
				CodePages->DeleteItem(Position);
				MenuItemEx NewItem;
				NewItem.Clear();
				NewItem.strName = strCodePage;
				NewItem.UserData = &CodePage;
				NewItem.UserDataSize = sizeof(CodePage);
				CodePages->AddItem(&NewItem, Position);
				CodePages->SetSelectPos(Position, 1);
			}
		}
	}
	return DefDlgProc(hDlg, Msg, Param1, Param2);
}

// ����� ��������� ����� ������� ��������
void EditCodePageName()
{
	UINT Position = CodePages->GetSelectPos();
	if (IsPositionStandard(Position))
		return;
	string CodePageName = CodePages->GetItemPtr(Position)->strName;
	size_t BoxPosition;
	if (!CodePageName.Pos(BoxPosition, BoxSymbols[BS_V1]))
		return;
	CodePageName.LShift(BoxPosition+2);
	FarDialogItem EditDialogData[]=
		{
			{DI_DOUBLEBOX, 3, 1, 50, 5, 0, nullptr, nullptr, 0, MSG(MGetCodePageEditCodePageName)},
			{DI_EDIT,      5, 2, 48, 2, 0, L"CodePageName", nullptr, DIF_FOCUS|DIF_HISTORY, CodePageName},
			{DI_TEXT,      0, 3,  0, 3, 0, nullptr, nullptr, DIF_SEPARATOR, L""},
			{DI_BUTTON,    0, 4,  0, 3, 0, nullptr, nullptr, DIF_DEFAULTBUTTON|DIF_CENTERGROUP, MSG(MOk)},
			{DI_BUTTON,    0, 4,  0, 3, 0, nullptr, nullptr, DIF_CENTERGROUP, MSG(MCancel)},
			{DI_BUTTON,    0, 4,  0, 3, 0, nullptr, nullptr, DIF_CENTERGROUP, MSG(MGetCodePageResetCodePageName)}
		};
	MakeDialogItemsEx(EditDialogData, EditDialog);
	Dialog Dlg(EditDialog, ARRAYSIZE(EditDialog), EditDialogProc);
	Dlg.SetPosition(-1, -1, 54, 7);
	Dlg.SetHelp(L"EditCodePageNameDlg");
	Dlg.Process();
}

UINT SelectCodePage(UINT nCurrent, bool bShowUnicode, bool bShowUTF, bool bShowUTF7, bool bShowAutoDetect)
{
	CallbackCallSource = CodePageSelect;
	currentCodePage = nCurrent;
	// ������ ����
	CodePages = new VMenu(L"", nullptr, 0, ScrY-4);
	CodePages->SetBottomTitle(MSG(!Opt.CPMenuMode?MGetCodePageBottomTitle:MGetCodePageBottomShortTitle));
	CodePages->SetFlags(VMENU_WRAPMODE|VMENU_AUTOHIGHLIGHT);
	CodePages->SetHelp(L"CodePagesMenu");
	// ��������� ������� ��������
	FillCodePagesVMenu(bShowUnicode, bShowUTF, bShowUTF7, bShowAutoDetect);
	// ���������� ����
	CodePages->Show();

	// ���� ��������� ��������� ����
	while (!CodePages->Done())
	{
		switch (CodePages->ReadInput())
		{
			// ��������� �������/������ ��������� ������ ��������
			case KEY_CTRLH:
			case KEY_RCTRLH:
				Opt.CPMenuMode = !Opt.CPMenuMode;
				CodePages->SetBottomTitle(MSG(!Opt.CPMenuMode?MGetCodePageBottomTitle:MGetCodePageBottomShortTitle));
				FillCodePagesVMenu(bShowUnicode, bShowUTF, bShowUTF7, bShowAutoDetect);
				break;
			// ��������� �������� ������� �������� �� ������ ���������
			case KEY_DEL:
			case KEY_NUMDEL:
				ProcessSelected(false);
				break;
			// ��������� ���������� ������� �������� � ������ ���������
			case KEY_INS:
			case KEY_NUMPAD0:
				ProcessSelected(true);
				break;
			// ����������� ��� ������� ��������
			case KEY_F4:
				EditCodePageName();
				break;
			default:
				CodePages->ProcessInput();
				break;
		}
	}

	// �������� ��������� ������� ��������
	UINT codePage = CodePages->Modal::GetExitCode() >= 0 ? static_cast<WORD>(GetMenuItemCodePage()) : (UINT)-1;
	delete CodePages;
	CodePages = nullptr;
	return codePage;
}

// ��������� ������ ��������� ��������
UINT FillCodePagesList(HANDLE dialogHandle, UINT controlId, UINT codePage, bool allowAuto, bool allowAll, bool allowDefault)
{
	CallbackCallSource = CodePagesFill;
	// ������������� ���������� ��� ������� �� ��������
	dialog = dialogHandle;
	control = controlId;
	currentCodePage = codePage;
	favoriteCodePages = normalCodePages = 0;
	selectedCodePages = !allowAuto && allowAll;
	// ��������� ���������� �������� � ������
	AddCodePages((allowDefault ? ::DefaultCP : 0) | (allowAuto ? ::Auto : 0) | (allowAll ? ::SearchAll : 0) | ::AllStandard);

	if (CallbackCallSource == CodePagesFill)
	{
		// ���� ���� �������� �������
		FarListInfo info={sizeof(FarListInfo)};
		SendDlgMessage(dialogHandle, DM_LISTINFO, control, &info);

		for (int i=0; i<static_cast<int>(info.ItemsNumber); i++)
		{
			if (GetListItemCodePage(i)==codePage)
			{
				FarListGetItem Item={sizeof(FarListGetItem),i};
				SendDlgMessage(dialog, DM_LISTGETITEM, control, &Item);
				SendDlgMessage(dialog, DM_SETTEXTPTR, control, const_cast<wchar_t*>(Item.Item.Text));
				FarListPos Pos={sizeof(FarListPos),i,-1};
				SendDlgMessage(dialog, DM_LISTSETCURPOS, control, &Pos);
				break;
			}
		}
	}

	// ���������� ����� ������� ������ ��������
	return favoriteCodePages;
}

bool IsCodePageSupported(UINT CodePage)
{
	// ��� ����������� ������� ������� ������ ��������� �� ����
	// BUGBUG: �� �� ����� ����������� ��� ����������� ������� ��������. ��� �� �����������
	if (CodePage == CP_DEFAULT || IsStandardCodePage(CodePage))
		return true;

	// �������� �� ���� ������� ��������� ������� � ��������� ������������ �� ��� ��� �
	CallbackCallSource = CodePageCheck;
	currentCodePage = CodePage;
	CodePageSupported = false;
	EnumSystemCodePages((CODEPAGE_ENUMPROCW)EnumCodePagesProc, CP_INSTALLED);
	return CodePageSupported;
}
