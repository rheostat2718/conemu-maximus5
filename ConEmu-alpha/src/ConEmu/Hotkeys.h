
/*
Copyright (c) 2013 Maximus5
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

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY EXPRESS OR
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


#pragma once

// Forward
struct ConEmuHotKey;
class CHotKeyDialog;

// ��������� ���������� ����� ������������ "�� ����������" �� ��������� ������ � �����������
extern const struct ConEmuHotKey* ConEmuSkipHotKey; // = ((ConEmuHotKey*)INVALID_HANDLE_VALUE)

#define CEHOTKEY_MODMASK    0xFFFFFF00
#define CEHOTKEY_NUMHOSTKEY 0xFFFFFF00
#define CEHOTKEY_ARRHOSTKEY 0xFEFEFE00
#define CEHOTKEY_NOMOD      0x80808000

enum ConEmuHotKeyType
{
	chk_User = 0,  // ������� ������������� hotkey
	chk_Modifier,  // ��� �����, ��������
	chk_Modifier2, // ��� �����, �������� (����� ����� ������ ����� ������ ������������)
	chk_NumHost,   // system hotkey (<HostKey>-Number, � ��� ������ <HostKey>-Arrows)
	chk_ArrHost,   // system hotkey (<HostKey>-Number, � ��� ������ <HostKey>-Arrows)
	chk_System,    // predefined hotkeys, ��������������� (����?)
	chk_Global,    // globally registered hotkey
	chk_Local,     // locally registered hotkey
	chk_Macro,     // GUI Macro
	chk_Task,      // Task hotkey
};


struct ConEmuHotKey
{
	// >0 StringTable resource ID
	// <0 TaskIdx, 1-based
	int DescrLangID;
	
	// 0 - hotkey, 1 - modifier (��� �����, ��������), 2 - system hotkey (��������� nMultiHotkeyModifier)
	ConEmuHotKeyType HkType;

	// May be NULL
	bool   (*Enabled)();

	wchar_t Name[64];
	
	DWORD VkMod;

    bool (WINAPI *fkey)(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon); // true-����������, false-���������� � �������
	bool OnKeyUp; // ��������� ���������� ����� ������������ "�� ����������" (����� ��������, ����, ...)

	wchar_t* GuiMacro;

	// May be NULL. if "true" - dont intercept this key with KeyboardHooks, try to pass it to Windows.
	bool   (*DontWinHook)(const ConEmuHotKey* pHK);

	// Internal
	size_t cchGuiMacroMax;
	bool   NotChanged;

	bool CanChangeVK() const;
	bool IsTaskHotKey() const;
	int GetTaskIndex() const; // 0-based
	void SetTaskIndex(int iTaskIdx); // 0-based

	LPCWSTR GetDescription(wchar_t* pszDescr, int cchMaxLen, bool bAddMacroIndex = false) const;

	void Free();

	// *** Service functions ***
	// ������� ��� ������������ (���� "Apps+Space")
	LPCWSTR GetHotkeyName(wchar_t (&szFull)[128], bool bShowNone = true) const;
	static LPCWSTR GetHotkeyName(DWORD aVkMod, wchar_t (&szFull)[128], bool bShowNone = true);

	// nHostMod � ������� 3-� ������ ����� ��������� VK (������������).
	// ������� ���������, ����� ��� �� �������������
	static void TestHostkeyModifiers(DWORD& nHostMod);
	// ����� ������ MOD_xxx ��� RegisterHotKey
	static DWORD GetHotKeyMod(DWORD VkMod);
	// ��������� ������� ��� �������������. ��������� ������� VkMod
	static DWORD MakeHotKey(BYTE Vk, BYTE vkMod1=0, BYTE vkMod2=0, BYTE vkMod3=0);
	// ������� ��� VK
	static DWORD GetHotkey(DWORD VkMod);
	// ������� ��� ������� (Apps, Win, Pause, ...)
	static void GetVkKeyName(BYTE vk, wchar_t (&szName)[32]);
	// ���� �� � ���� (VkMod) ������ - ����������� Mod (VK)
	static bool HasModifier(DWORD VkMod, BYTE Mod/*VK*/);
	// ������ ��� �������� ����������� � VkMod
	static DWORD SetModifier(DWORD VkMod, BYTE Mod/*VK*/, bool Xor=true);
	// ������� ����������� ������������ (idx = 1..3). ���������� 0 (����) ��� VK
	static DWORD GetModifier(DWORD VkMod, int idx/*1..3*/);

	static bool UseWinNumber();
	static bool UseWinArrows();
	static bool UseCTSShiftArrow(); // { return gpSet->isUseWinArrows; }; // { return (OverrideClipboard || !AppNames) ? isCTSShiftArrowStart : gpSet->AppStd.isCTSShiftArrowStart; };
	static bool UseCtrlTab();
	static bool InSelection();
	static bool DontHookJumps(const ConEmuHotKey* pHK);

	// *** Default and all possible ConEmu hotkeys ***
	static int AllocateHotkeys(ConEmuHotKey** ppHotKeys);
};

// IDD_HOTKEY { hkHotKeySelect, lbHotKeyList, lbHotKeyMod1, lbHotKeyMod2, lbHotKeyMod3 }
class CHotKeyDialog
{
private:
	HWND mh_Dlg;
	HWND mh_Parent;
	ConEmuHotKey m_HK;
public:
	static bool EditHotKey(HWND hParent, DWORD& VkMod);
	DWORD GetVkMod();
public:
	static DWORD dlgGetHotkey(HWND hDlg, UINT iEditCtrl = hkHotKeySelect, UINT iListCtrl = lbHotKeyList);
public:
	CHotKeyDialog(HWND hParent, DWORD aVkMod);
	~CHotKeyDialog();

	static INT_PTR CALLBACK hkDlgProc(HWND hDlg, UINT messg, WPARAM wParam, LPARAM lParam);
};
