
/*
Copyright (c) 2012 Maximus5
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

#define HIDE_USE_EXCEPTION_INFO
#include "Header.h"
#include "ConEmu.h"
#include "ConEmuCtrl.h"
#include "Macro.h"
#include "OptionsClass.h"
#include "RealConsole.h"
#include "TabBar.h"
#include "VirtualConsole.h"


const ConEmuHotKey* ConEmuSkipHotKey = ((ConEmuHotKey*)INVALID_HANDLE_VALUE);

bool CConEmuCtrl::mb_SkipOneAppsRelease = false;
HHOOK CConEmuCtrl::mh_SkipOneAppsRelease = NULL;


CConEmuCtrl::CConEmuCtrl()
{
	mb_InWinTabSwitch = mb_InCtrlTabSwitch = FALSE;
	dwControlKeyState = 0;
	bWin = bApps = bCaps = bNum = bScroll = bLAlt = bRAlt = bLCtrl = bRCtrl = bLShift = bRShift = false;
	m_SkippedMsg = 0; m_SkippedMsgWParam = 0; m_SkippedMsgLParam = 0;
	mb_LastSingleModifier = FALSE;
	mn_LastSingleModifier = mn_SingleModifierFixKey = mn_SingleModifierFixState = 0;
	mn_DoubleKeyConsoleNum = 0;
}

CConEmuCtrl::~CConEmuCtrl()
{
}

// pRCon may be NULL, pszChars may be NULL
const ConEmuHotKey* CConEmuCtrl::ProcessHotKey(DWORD VkMod, bool bKeyDown, const wchar_t *pszChars, CRealConsole* pRCon)
{
	UINT vk = gpSet->GetHotkey(VkMod);
	if (!(vk >= '0' && vk <= '9'))
		ResetDoubleKeyConsoleNum();

	const ConEmuHotKey* pHotKey = gpSetCls->GetHotKeyInfo(VkMod, bKeyDown, pRCon);

	if (pHotKey && (pHotKey != ConEmuSkipHotKey))
	{
		// ����� � ������� �� ������� ����� (FAR ����� ��������� ������ �� Alt)
		if (((VkMod & cvk_ALLMASK) == cvk_LAlt) || ((VkMod & cvk_ALLMASK) == cvk_RAlt))
		{
			if (pRCon && gpSet->isFixAltOnAltTab)
				pRCon->PostKeyPress(VK_CONTROL, LEFT_ALT_PRESSED, 0);
		}

		// ������ ���������� ��������
		if (pHotKey->fkey)
		{
			pHotKey->fkey(VkMod, false, pHotKey, pRCon);
		}
		else
		{
			_ASSERTE(pHotKey->fkey!=NULL);
		}
	}

	return pHotKey;
}

void CConEmuCtrl::UpdateControlKeyState()
{
	bCaps = (1 & (WORD)GetKeyState(VK_CAPITAL)) == 1;
	bNum = (1 & (WORD)GetKeyState(VK_NUMLOCK)) == 1;
	bScroll = (1 & (WORD)GetKeyState(VK_SCROLL)) == 1;
	bWin = isPressed(VK_LWIN) || isPressed(VK_RWIN);
	bApps = isPressed(VK_APPS);
	bLAlt = isPressed(VK_LMENU);
	bRAlt = isPressed(VK_RMENU);
	bLCtrl = isPressed(VK_LCONTROL);
	bRCtrl = isPressed(VK_RCONTROL);
	bLShift = isPressed(VK_LSHIFT);
	bRShift = isPressed(VK_RSHIFT);

	DWORD ControlKeyState = 0;

	//if (((DWORD)lParam & (DWORD)(1 << 24)) != 0)
	//	r.Event.KeyEvent.dwControlKeyState |= ENHANCED_KEY;

	if (bCaps)
		ControlKeyState |= CAPSLOCK_ON;

	if (bNum)
		ControlKeyState |= NUMLOCK_ON;

	if (bScroll)
		ControlKeyState |= SCROLLLOCK_ON;

	if (bLAlt)
		ControlKeyState |= LEFT_ALT_PRESSED;

	if (bRAlt)
		ControlKeyState |= RIGHT_ALT_PRESSED;

	if (bLCtrl)
		ControlKeyState |= LEFT_CTRL_PRESSED;

	if (bRCtrl)
		ControlKeyState |= RIGHT_CTRL_PRESSED;

	if (bLShift || bRShift)
		ControlKeyState |= SHIFT_PRESSED;

	dwControlKeyState = ControlKeyState;
}

// lParam - �� ��������� WM_KEYDOWN/WM_SYSKEYDOWN/...
DWORD CConEmuCtrl::GetControlKeyState(LPARAM lParam)
{
	return dwControlKeyState | ((((DWORD)lParam & (DWORD)(1 << 24)) != 0) ? ENHANCED_KEY : 0);
}

// true - ��������� �������� � �������, ���� ����������
// pRCon may be NULL, pszChars may be NULL
bool CConEmuCtrl::ProcessHotKeyMsg(UINT messg, WPARAM wParam, LPARAM lParam, const wchar_t *pszChars, CRealConsole* pRCon)
{
	_ASSERTE((messg == WM_KEYDOWN || messg == WM_SYSKEYDOWN) || (messg == WM_KEYUP || messg == WM_SYSKEYUP));

	WARNING("CConEmuCtrl:: �������� ����� ��� �����-�� ������ � ������ ��� ���������� �������");
	WARNING("CConEmuCtrl:: ��� � CConEmuMain::OnKeyboard ���� ���������� ������ � sm_SkipSingleHostkey, sw_SkipSingleHostkey, sl_SkipSingleHostkey");

	// �������� � ����������� "r.Event.KeyEvent.dwControlKeyState"
	UpdateControlKeyState();

	DWORD vk = (DWORD)(wParam & 0xFF);
	bool bKeyDown = (messg == WM_KEYDOWN || messg == WM_SYSKEYDOWN);
	bool bKeyUp = (messg == WM_KEYUP || messg == WM_SYSKEYUP);

	if (mn_DoubleKeyConsoleNum && (!(vk >= '0' && vk <= '9')))
	{
		//if (!(vk >= '0' && vk <= '9'))
		//	ResetDoubleKeyConsoleNum();

		int nNewIdx = -1;
		// ������� ��������� ����� �������
		if (mn_DoubleKeyConsoleNum>='1' && mn_DoubleKeyConsoleNum<='9')
			nNewIdx = mn_DoubleKeyConsoleNum - '1';
		else if (mn_DoubleKeyConsoleNum=='0')
			nNewIdx = 9;

		ResetDoubleKeyConsoleNum();

		if (nNewIdx >= 0)
			gpConEmu->ConActivate(nNewIdx);
	}

	if (bKeyUp)
	{
		if ((mb_InWinTabSwitch && (vk == VK_RWIN || vk == VK_LWIN))
			|| (mb_InCtrlTabSwitch && (vk == VK_CONTROL || vk == VK_LCONTROL || vk == VK_RCONTROL)))
		{
			mb_InWinTabSwitch = mb_InCtrlTabSwitch = FALSE;
			gpConEmu->TabCommand(ctc_SwitchCommit);
			WARNING("CConEmuCtrl:: � ��� ���������� ������ ���� ���������?");
		}
	}

	// �� ���� ������������ - �������� �� ��������
	if (vk == VK_LWIN || vk == VK_RWIN /*|| vk == VK_APPS*/
		|| vk == VK_SHIFT || vk == VK_LSHIFT || vk == VK_RSHIFT
		|| vk == VK_CONTROL || vk == VK_LCONTROL || vk == VK_RCONTROL
		|| vk == VK_MENU || vk == VK_LMENU || vk == VK_RMENU)
	{
		if (pRCon)
		{
			// ������, ���� ��� ��� ��������� ������������ ����������� - ��� ����� "���������",
			// ����� �� ��� ���������� �� ���������� Far-������ ��������
			if (bKeyUp)
			{
				FixSingleModifier(vk, pRCon);
			}
			else
			{
				_ASSERTE(bKeyDown);
				int ModCount = 0;
				if (bLAlt) ModCount++;
				if (bRAlt) ModCount++;
				if (bLCtrl) ModCount++;
				if (bRCtrl) ModCount++;
				if (bLShift || bRShift) ModCount++;

				if ((ModCount == 1) && (vk != VK_APPS) && (vk != VK_LWIN))
				{
					mb_LastSingleModifier = FALSE;
					mn_LastSingleModifier = 
						(vk == VK_LMENU || vk == VK_RMENU || vk == VK_MENU) ? VK_MENU :
						(vk == VK_LCONTROL || vk == VK_RCONTROL || vk == VK_CONTROL) ? VK_CONTROL :
						(vk == VK_LSHIFT || vk == VK_RSHIFT || vk == VK_SHIFT) ? VK_SHIFT : 0;

					if (!mn_LastSingleModifier)
					{
						// Win � ������ ������������ ����� �� ����������
					}
					else
					{
						mn_SingleModifierFixState = dwControlKeyState;
						switch (mn_LastSingleModifier)
						{
						case VK_MENU:
							mn_SingleModifierFixKey = VK_CONTROL;
							mn_SingleModifierFixState |= (LEFT_CTRL_PRESSED);
							break;
						case VK_CONTROL:
							mn_SingleModifierFixKey = VK_MENU;
							mn_SingleModifierFixState |= (RIGHT_ALT_PRESSED);
							break;
						case VK_SHIFT:
							mn_SingleModifierFixKey = VK_MENU;
							mn_SingleModifierFixState |= RIGHT_ALT_PRESSED;
							break;
						}
					}
				}
				else if (ModCount > 1)
				{
					// ������ �� �����
					mb_LastSingleModifier = FALSE;
					mn_LastSingleModifier = mn_SingleModifierFixKey = mn_SingleModifierFixState = 0;
				}
			}
		}
		return false;
	}

	DWORD nState = 0;

	if (bWin)
		nState |= cvk_Win;

	if ((vk != VK_APPS) && bApps)
		nState |= cvk_Apps;

	if (bLCtrl)
		nState |= cvk_LCtrl|cvk_Ctrl;
	if (bRCtrl)
		nState |= cvk_RCtrl|cvk_Ctrl;

	if (bLAlt)
		nState |= cvk_LAlt|cvk_Alt;
	if (bRAlt)
		nState |= cvk_RAlt|cvk_Alt;

	if (bLShift)
		nState |= cvk_LShift|cvk_Shift;
	if (bRShift)
		nState |= cvk_RShift|cvk_Shift;

	DWORD VkMod = nState | vk;
	
	if (bKeyDown)
		m_SkippedMsg = 0;

	const ConEmuHotKey* pHotKey = ProcessHotKey(VkMod, bKeyDown, pszChars, pRCon);

	// ��� "���������"
	if (pHotKey && mn_LastSingleModifier)
	{
		mb_LastSingleModifier = TRUE;
	}
	else if (!pHotKey && !(nState & (cvk_Ctrl|cvk_Alt|cvk_Shift)))
	{
		if (bKeyDown)
		{
			// ��� �� ������ ���� - ������ ��� Apps � ��� �� ������, �� ����� ���� �������������?
			if ((vk == VK_APPS) && gpSet->isModifierExist(vk))
			{
				m_SkippedMsg = messg; m_SkippedMsgWParam = wParam; m_SkippedMsgLParam = lParam;
				// ����������� ���� ��
				// *) ������� ������ ������, �� �������������� ���� (�������� Apps+U)
				// *) ���������� ������ Apps
				return ConEmuSkipHotKey;
			}
		}
		else if ((vk == VK_APPS) && m_SkippedMsg && pRCon)
		{
			// ����������� Apps. ������� ����� "�������" � ������� �� �������
			pRCon->ProcessKeyboard(m_SkippedMsg, m_SkippedMsgWParam, m_SkippedMsgLParam, NULL);
		}
	}

	return (pHotKey != NULL);
}

void CConEmuCtrl::FixSingleModifier(DWORD Vk, CRealConsole* pRCon)
{
	if (pRCon && gpSet->isFixAltOnAltTab)
	{
		if (mn_LastSingleModifier && ((Vk == 0) || (mb_LastSingleModifier && (Vk == mn_LastSingleModifier))))
			pRCon->PostKeyPress(mn_SingleModifierFixKey, mn_SingleModifierFixState, 0);
	}
	// ������ �� �����
	mb_LastSingleModifier = FALSE;
	mn_LastSingleModifier = mn_SingleModifierFixKey = mn_SingleModifierFixState = 0;
}

// User (Keys)
// pRCon may be NULL
bool CConEmuCtrl::key_MinimizeRestore(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (TestOnly)
		return true;
	// ������ �������������� ����� WM_HOTKEY
	_ASSERTE(FALSE && "CConEmuCtrl::key_MinimizeRestore");
	gpConEmu->OnMinimizeRestore();
	return true;
}

// pRCon may be NULL
bool CConEmuCtrl::key_MultiNew(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (TestOnly)
		return true;
	// ������� ����� �������
	gpConEmu->Recreate(FALSE, gpSet->isMultiNewConfirm);
	return true;
}

// pRCon may be NULL
bool CConEmuCtrl::key_MultiNewShift(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (TestOnly)
		return true;
	// ������� ����� �������
	gpConEmu->Recreate(FALSE, TRUE/*Confirm*/);
	return true;
}

// pRCon may be NULL
bool CConEmuCtrl::key_MultiNewPopup(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (TestOnly)
		return true;
	// ������� ����� �������
	gpConEmu->mp_TabBar->OnNewConPopup();
	return true;
}

// pRCon may be NULL
bool CConEmuCtrl::key_MultiNewAttach(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (TestOnly)
		return true;
	// ������� ����� �������
	gpConEmu->OnSysCommand(ghWnd, IDM_ATTACHTO, 0);
	return true;
}

// pRCon may be NULL
bool CConEmuCtrl::key_MultiNext(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (TestOnly)
		return true;
	gpConEmu->ConActivateNext(FALSE/*abReverse*/);
	return true;
}

// pRCon may be NULL
bool CConEmuCtrl::key_MultiNextShift(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (TestOnly)
		return true;
	gpConEmu->ConActivateNext(TRUE/*abReverse*/);
	return true;
}

// pRCon may be NULL
bool CConEmuCtrl::key_MultiRecreate(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (TestOnly)
		return true;
	// ������������� �� ��������/������������ �������
	gpConEmu->Recreate(TRUE, TRUE);
	return true;
}

// pRCon may be NULL
bool CConEmuCtrl::key_AlternativeBuffer(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (TestOnly)
		return true;
	gpConEmu->AskChangeAlternative();
	return true;
}

// pRCon may be NULL
bool CConEmuCtrl::key_MultiBuffer(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (TestOnly)
		return true;
	gpConEmu->AskChangeBufferHeight();
	return true;
}

// pRCon may be NULL
bool CConEmuCtrl::key_MultiClose(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (!pRCon)
		return false;
	if (TestOnly)
		return true;

	pRCon->CloseConsole(false, true);
	return true;
}

// pRCon may be NULL
bool CConEmuCtrl::key_TerminateProcess(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (!pRCon)
		return false;
	if (TestOnly)
		return true;

	pRCon->CloseConsole(true, false);
	return true;
}

// pRCon may be NULL
bool CConEmuCtrl::key_MultiCmd(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (TestOnly)
		return true;

	RConStartArgs args;
	args.pszSpecialCmd = GetComspec(&gpSet->ComSpec); //lstrdup(L"cmd");
	gpConEmu->CreateCon(&args);
	return true;
}

// pRCon may be NULL
bool CConEmuCtrl::key_CTSVkBlockStart(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (!pRCon)
		return false;
	if (TestOnly)
		return true;

	// ������ ��������� ��� �������� ���������
	pRCon->StartSelection(FALSE/*abTextMode*/);
	return true;
}

// pRCon may be NULL
bool CConEmuCtrl::key_CTSVkTextStart(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (!pRCon)
		return FALSE;
	if (TestOnly)
		return true;

	// ������ ��������� ��� �������� ���������
	pRCon->StartSelection(TRUE/*abTextMode*/);
	return true;
}

// System (predefined, fixed)
// pRCon may be NULL
bool CConEmuCtrl::key_Settings(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (TestOnly)
		return true;
	//KeyUp!
	gpConEmu->OnSysCommand(ghWnd, ID_SETTINGS, 0);
	return true;
}

// pRCon may be NULL
bool CConEmuCtrl::key_AltSpace(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (gpSet->isSendAltSpace)
		return false;

	if (TestOnly)
		return true;

	gpConEmu->ShowSysmenu();
	return true;
}

// pRCon may be NULL
bool CConEmuCtrl::key_SystemMenu(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (TestOnly)
		return true;
	//Win-Alt-Space
	gpConEmu->ShowSysmenu();
	return true;
}

// pRCon may be NULL
bool CConEmuCtrl::key_TabMenu(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (!pRCon)
		return false;
	if (TestOnly)
		return true;

	//Win-Apps
	POINT ptCur = gpConEmu->CalcTabMenuPos(pRCon->VCon());
	pRCon->VCon()->ShowPopupMenu(ptCur);
	return true;
}

// pRCon may be NULL
bool CConEmuCtrl::key_AltF9(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (gpSet->isSendAltF9)
		return false;

	if (TestOnly)
		return true;

	gpConEmu->OnAltF9(TRUE);
	return true;
}

// pRCon may be NULL
bool CConEmuCtrl::key_ShowRealConsole(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (!pRCon)
		return false;
	if (TestOnly)
		return true;
	// ������ �������������� ����� WM_HOTKEY?
	pRCon->CtrlWinAltSpace();
	return true;
}

// pRCon may be NULL
bool CConEmuCtrl::key_AltEnter(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (gpSet->isSendAltEnter)
		return false;

	if (TestOnly)
		return true;

	//if (gpSet->isSendAltEnter)
	//{
	//	#if 0
	//	INPUT_RECORD r = {KEY_EVENT};
	//	//On Keyboard(hConWnd, WM_KEYDOWN, VK_MENU, 0); -- Alt ����� �� ����� - �� ��� ������
	//	WARNING("� ���� �� ��� ��������������?");
	//	//On Keyboard(hConWnd, WM_KEYDOWN, VK_RETURN, 0);
	//	r.Event.KeyEvent.bKeyDown = TRUE;
	//	r.Event.KeyEvent.wRepeatCount = 1;
	//	r.Event.KeyEvent.wVirtualKeyCode = VK_RETURN;
	//	r.Event.KeyEvent.wVirtualScanCode = /*28 �� ���� ����������*/MapVirtualKey(VK_RETURN, 0/*MAPVK_VK_TO_VSC*/);
	//	r.Event.KeyEvent.dwControlKeyState = NUMLOCK_ON|LEFT_ALT_PRESSED /*0x22*/;
	//	r.Event.KeyEvent.uChar.UnicodeChar = pszChars[0];
	//	PostConsoleEvent(&r);
	//	//On Keyboard(hConWnd, WM_KEYUP, VK_RETURN, 0);
	//	r.Event.KeyEvent.bKeyDown = FALSE;
	//	r.Event.KeyEvent.dwControlKeyState = NUMLOCK_ON;
	//	PostConsoleEvent(&r);
	//	//On Keyboard(hConWnd, WM_KEYUP, VK_MENU, 0); -- Alt ����� �� ����� - �� ����� ������ ��� �����
	//	#endif
	//	_ASSERTE(pszChars[0]==13);
	//	PostKeyPress(VK_RETURN, LEFT_ALT_PRESSED, pszChars[0]);
	//}
	//else
	//{
	//	// ����� � ������� �� ������� ����� (FAR ����� ��������� ������ �� Alt)
	//	if (gpSet->isFixAltOnAltTab)
	//		PostKeyPress(VK_CONTROL, LEFT_ALT_PRESSED, 0);
	//	// Change fullscreen mode
	//	gpConEmu->OnAltEnter();
	//	//isSkipNextAltUp = true;
	//}


	gpConEmu->OnAltEnter();
	return true;
}

// pRCon may be NULL
bool CConEmuCtrl::key_ForcedFullScreen(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (TestOnly)
		return true;

	// ������ �������������� ����� WM_HOTKEY
	_ASSERTE(FALSE && "CConEmuCtrl::key_ForcesFullScreen");
	gpConEmu->OnForcedFullScreen();
	return true;
}

// pRCon may be NULL
bool CConEmuCtrl::key_FullScreen(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (TestOnly)
		return true;

	gpConEmu->OnAltEnter();
	return true;
}

// ����� ��� key_BufferScrollUp/key_BufferScrollDown/key_BufferScrollPgUp/key_BufferScrollPgDn
// pRCon may be NULL
bool CConEmuCtrl::key_BufferScroll(bool TestOnly, BYTE vk, CRealConsole* pRCon)
{
	if (!pRCon
		|| !pRCon->isBufferHeight()
		|| pRCon->isFarBufferSupported())
	{
		return false;
	}

	if (TestOnly)
		return true;

	_ASSERTE(pRCon!=NULL);
	switch (vk)
	{
		case VK_DOWN:
			pRCon->OnScroll(SB_LINEDOWN);
			break;
		case VK_UP:
			pRCon->OnScroll(SB_LINEUP);
			break;
		case VK_NEXT:
			pRCon->OnScroll(SB_PAGEDOWN);
			break;
		case VK_PRIOR:
			pRCon->OnScroll(SB_PAGEUP);
			break;
	}
	return true;
}

// pRCon may be NULL
bool CConEmuCtrl::key_BufferScrollUp(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (!pRCon)
		return false;
	return key_BufferScroll(TestOnly, VK_UP, pRCon);
}

// pRCon may be NULL
bool CConEmuCtrl::key_BufferScrollDown(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (!pRCon)
		return false;
	return key_BufferScroll(TestOnly, VK_DOWN, pRCon);
}

// pRCon may be NULL
bool CConEmuCtrl::key_BufferScrollPgUp(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (!pRCon)
		return false;
	return key_BufferScroll(TestOnly, VK_PRIOR, pRCon);
}

// pRCon may be NULL
bool CConEmuCtrl::key_BufferScrollPgDn(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (!pRCon)
		return false;
	return key_BufferScroll(TestOnly, VK_NEXT, pRCon);
}

// pRCon may be NULL
bool CConEmuCtrl::key_CtrlTab(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (!gpSet->isTabSelf)
		return false;

	if (TestOnly)
		return true;

	//WARNING! Must be System Unchanged HOTKEY
	//WARNING! ������� ����� isPressed(VK_SHIFT)
	WARNING("CConEmuCtrl:: ���������� �� ����� ����� SwitchNext, ������, ��� �� ����� ���� ��������������!");
	gpConEmu->mp_TabBar->OnKeyboard(WM_KEYDOWN, VK_TAB, 0);
	if (gpSet->isTabLazy)
		gpConEmu->mb_InCtrlTabSwitch = TRUE;
	return true;
}

// pRCon may be NULL
bool CConEmuCtrl::key_CtrlShiftTab(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (!gpSet->isTabSelf)
		return false;

	if (TestOnly)
		return true;

	//WARNING! Must be System Unchanged HOTKEY
	//WARNING! ������� ����� isPressed(VK_SHIFT)
	WARNING("CConEmuCtrl:: ���������� �� ����� ����� SwitchPrev, ������, ��� �� ����� ���� ��������������!");
	gpConEmu->mp_TabBar->OnKeyboard(WM_KEYDOWN, VK_TAB, 0);
	if (gpSet->isTabLazy)
		gpConEmu->mb_InCtrlTabSwitch = TRUE;
	return true;
}

// pRCon may be NULL
bool CConEmuCtrl::key_CtrlTab_Prev(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (!gpSet->isTabSelf || !gpConEmu->mp_TabBar->IsInSwitch())
		return false;

	if (TestOnly)
		return true;

	//WARNING! Must be System Unchanged HOTKEY
	//WARNING! ������� ����� isPressed(VK_SHIFT)
	WARNING("CConEmuCtrl:: ���������� �� ����� ����� Switch, ������, ��� �� ����� ���� ��������������!");
	if (!gpConEmu->mp_TabBar->OnKeyboard(WM_KEYDOWN, (VkMod & 0xFF), 0))
		return false;
	return true;
}

// pRCon may be NULL
bool CConEmuCtrl::key_CtrlTab_Next(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (!gpSet->isTabSelf || !gpConEmu->mp_TabBar->IsInSwitch())
		return false;

	if (TestOnly)
		return true;

	//WARNING! Must be System Unchanged HOTKEY
	//WARNING! ������� ����� isPressed(VK_SHIFT)
	WARNING("CConEmuCtrl:: ���������� �� ����� ����� Switch, ������, ��� �� ����� ���� ��������������!");
	if (!gpConEmu->mp_TabBar->OnKeyboard(WM_KEYDOWN, (VkMod & 0xFF), 0))
		return false;
	return true;
}

// ����� ��� key_WinWidthDec/key_WinWidthInc/key_WinHeightDec/key_WinHeightInc
// pRCon may be NULL
bool CConEmuCtrl::key_WinSize(BYTE vk)
{
	if (gpConEmu->isFullScreen() || gpConEmu->isZoomed() || gpConEmu->isIconic())
	{
		// ������ �� ������
	}
	else
	{
		CVirtualConsole* pVCon = gpConEmu->ActiveCon();
		RECT rcWindow = {};
		if (GetWindowRect(ghWnd, &rcWindow))
		{
			RECT rcMon = gpConEmu->CalcRect(CER_MONITOR, rcWindow, CER_MONITOR, pVCon);
			int nX = gpSetCls->FontWidth();
			int nY = gpSetCls->FontHeight();
			if (vk == VK_LEFT)
			{
				rcWindow.right = rcWindow.right - nX;
			}
			else if (vk == VK_RIGHT)
			{
				if ((rcWindow.right + nX) < rcMon.right)
					rcWindow.right = rcWindow.right + nX;
			}
			else if (vk == VK_UP)
			{	
				rcWindow.bottom = rcWindow.bottom - nY;
			}
			else if (vk == VK_DOWN)
			{
				if ((rcWindow.bottom + nY) < rcMon.bottom)
					rcWindow.bottom = rcWindow.bottom + nY;
			}
			
			if (rcWindow.right > rcWindow.left && rcWindow.bottom > rcWindow.top)
			{
				MoveWindow(ghWnd, rcWindow.left, rcWindow.top, (rcWindow.right - rcWindow.left), (rcWindow.bottom - rcWindow.top), 1);
			}
		}
		//
		//CRealConsole* pRCon = pVCon ? pVCon->RCon() : NULL;
		//if (pRCon)
		//{
		//	
		//	//if (!pRCon->GuiWnd())
		//	//{
		//	//	
		//	//}
		//	//else
		//	//{
		//	//	// ������ � ��� ������
		//	//}
		//}
	}
	return true;
}

// ��� ��� ���� - ���� ��������� � "HostKey"
// pRCon may be NULL
bool CConEmuCtrl::key_WinWidthDec(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (TestOnly)
		return true;
	return key_WinSize(VK_LEFT);
}

// pRCon may be NULL
bool CConEmuCtrl::key_WinWidthInc(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (TestOnly)
		return true;
	return key_WinSize(VK_RIGHT);
}

// pRCon may be NULL
bool CConEmuCtrl::key_WinHeightDec(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (TestOnly)
		return true;
	return key_WinSize(VK_UP);
}

// pRCon may be NULL
bool CConEmuCtrl::key_WinHeightInc(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (TestOnly)
		return true;
	return key_WinSize(VK_DOWN);
}


void CConEmuCtrl::ResetDoubleKeyConsoleNum(CRealConsole* pRCon)
{
	if (mn_DoubleKeyConsoleNum)
		mn_DoubleKeyConsoleNum = 0;
}

// Console activate by number
// pRCon may be NULL
bool CConEmuCtrl::key_ConsoleNum(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (TestOnly)
		return true;

	int nNewIdx = -1;

	if (gpConEmu->GetVCon(10))
	{
		// �������� ������������� ���������, ���� ��� ������ 9-� �������� �������
		if (gpConEmu->mn_DoubleKeyConsoleNum)
		{
			if ((VkMod & 0xFF)>='0' && (VkMod & 0xFF)<='9')
			{
				nNewIdx = (((VkMod & 0xFF) - '0') + ((gpConEmu->mn_DoubleKeyConsoleNum - '0')*10)) - 1; // 0-based
			}

			gpConEmu->ResetDoubleKeyConsoleNum(pRCon);
		}
		else
		{
			// ��������� ������ �����
			if ((VkMod & 0xFF)>='0' && (VkMod & 0xFF)<='9')
			{
				gpConEmu->mn_DoubleKeyConsoleNum = (VkMod & 0xFF);
			}
			else
			{
				_ASSERTE((VkMod & 0xFF)>='0' && (VkMod & 0xFF)<='9');
			}
		}
	}
	else
	{
		if ((VkMod & 0xFF)>='1' && (VkMod & 0xFF)<='9')
			nNewIdx = (VkMod & 0xFF) - '1';
		else if ((VkMod & 0xFF)=='0')
			nNewIdx = 9;
		//else if ((VkMod & 0xFF)==VK_F11 || (VkMod & 0xFF)==VK_F12)
		//	gpConEmu->ConActivate(10+((VkMod & 0xFF)-VK_F11));

		gpConEmu->ResetDoubleKeyConsoleNum(pRCon);
	}

	if (nNewIdx >= 0)
		gpConEmu->ConActivate(nNewIdx);

	return true;
}

bool CConEmuCtrl::key_PicViewSlideshow(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (!pRCon)
		return false;
	//if (TestOnly) -- ����, ���.��������
	//	return true;

	if ((VkMod & cvk_VK_MASK) == VK_PAUSE)
	{
		// SlideShow � PicView2 �������������
		if (gpConEmu->isPictureView() /*&& !IsWindowUnicode(hPictureView)*/)
		{
			bool lbAllowed = true;
			if (IsWindowUnicode(gpConEmu->hPictureView))
			{
				// �� �������, ���� ����� "����������" SlideShow - ������� �� ��� ��������� TRUE
				UINT nMsg = RegisterWindowMessage(L"PicView:HasSlideShow");
				DWORD_PTR nRc = 0;
				LRESULT lRc = SendMessageTimeout(gpConEmu->hPictureView, nMsg, 0,0, SMTO_NORMAL, 1000, &nRc);
				if (!lRc || nRc == TRUE)
					lbAllowed = false;
			}

			if (!lbAllowed)
				return false;
			if (TestOnly)
				return true;

			gpConEmu->bPicViewSlideShow = !gpConEmu->bPicViewSlideShow;

			if (gpConEmu->bPicViewSlideShow)
			{
				if (gpSet->nSlideShowElapse<=500) gpSet->nSlideShowElapse=500;

				gpConEmu->dwLastSlideShowTick = GetTickCount() - gpSet->nSlideShowElapse;
			}

			return true;
		}
	}
	else if (gpConEmu->bPicViewSlideShow)
	{
		//KillTimer(hWnd, 3);
		if ((VkMod & cvk_VK_MASK) == 0xbd/* -_ */ || (VkMod & cvk_VK_MASK) == 0xbb/* =+ */)
		{
			if (TestOnly)
				return true;

			if ((VkMod & cvk_VK_MASK) == 0xbb)
			{
				gpSet->nSlideShowElapse = 1.2 * gpSet->nSlideShowElapse;
			}
			else
			{
				gpSet->nSlideShowElapse = gpSet->nSlideShowElapse / 1.2;

				if (gpSet->nSlideShowElapse<=500) gpSet->nSlideShowElapse=500;
			}

			return true;
		}
		//else
		//{
		//	//bPicViewSlideShow = false; // ������ ��������
		//	return false;
		//}
	}

	return false;
}

bool CConEmuCtrl::key_GuiMacro(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (TestOnly)
		return true;
	// ��������� ������
	if (hk->GuiMacro && *hk->GuiMacro)
	{
		// �.�. ExecuteMacro ���������� ��� �������
		wchar_t* pszCopy = lstrdup(hk->GuiMacro);
		if (!pszCopy)
		{
			_ASSERTE(pszCopy);
		}
		else
		{
			wchar_t* pszResult = CConEmuMacro::ExecuteMacro(pszCopy, pRCon);
			TODO("����� �������� StatusLine - ������ �� � ��� ��������� ��������");
			SafeFree(pszResult);
			free(pszCopy);
		}
	}
	return true;
}

void CConEmuCtrl::ChooseTabFromMenu(BOOL abFirstTabOnly, POINT pt, DWORD Align /*= TPM_CENTERALIGN|TPM_VCENTERALIGN*/)
{
	HMENU hPopup = gpConEmu->CreateVConListPopupMenu(NULL, abFirstTabOnly);

	if (!Align)
		Align = TPM_LEFTALIGN|TPM_TOPALIGN;

	int nTab = gpConEmu->trackPopupMenu(tmp_TabsList, hPopup, Align|TPM_RETURNCMD,
		pt.x, pt.y, 0, ghWnd, NULL);

	if (nTab >= IDM_VCON_FIRST && nTab <= IDM_VCON_LAST)
	{
		int nNewV = ((int)HIWORD(nTab))-1;
		int nNewR = ((int)LOWORD(nTab))-1;
		
		CVirtualConsole* pVCon = gpConEmu->GetVCon(nNewV);
		if (pVCon)
		{
			CRealConsole* pRCon = pVCon->RCon();
			if (pRCon)
			{
				pRCon->ActivateFarWindow(nNewR);
			}
			if (!gpConEmu->isActive(pVCon))
				gpConEmu->Activate(pVCon);
		}
	}

	DestroyMenu(hPopup);
}

// ��� ��������� ����� ���� NULL - ����� �� GuiMacro
bool CConEmuCtrl::key_ShowTabsList(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (pRCon && pRCon->GetFarPID(true))
	{
		TODO("���������� �� ������� �������");
		return false;
	}
	else
	{
		if (TestOnly)
			return true;

		//HMENU hPopup = gpConEmu->CreateVConListPopupMenu(NULL, FALSE);

		RECT rcWnd = {};
		if (pRCon)
		{
			GetWindowRect(pRCon->GetView(), &rcWnd);
		}
		else
		{
			GetClientRect(ghWnd, &rcWnd);
			MapWindowPoints(ghWnd, NULL, (LPPOINT)&rcWnd, 2);
		}

		POINT pt = {(rcWnd.left+rcWnd.right)/2, (rcWnd.top+rcWnd.bottom)/2};

		ChooseTabFromMenu(FALSE, pt, TPM_CENTERALIGN|TPM_VCENTERALIGN);

		//int nTab = gpConEmu->trackPopupMenu(tmp_TabsList, hPopup, TPM_CENTERALIGN|TPM_VCENTERALIGN|TPM_RETURNCMD,
		//	x, y, 0, ghWnd, NULL);
		//if (nTab >= IDM_VCON_FIRST && nTab <= IDM_VCON_LAST)
		//{
		//	int nNewV = ((int)HIWORD(nTab))-1;
		//	int nNewR = ((int)LOWORD(nTab))-1;
		//	
		//	CVirtualConsole* pVCon = gpConEmu->GetVCon(nNewV);
		//	if (pVCon)
		//	{
		//		CRealConsole* pRCon = pVCon->RCon();
		//		if (pRCon)
		//		{
		//			pRCon->ActivateFarWindow(nNewR);
		//		}
		//		if (!gpConEmu->isActive(pVCon))
		//			gpConEmu->Activate(pVCon);
		//	}
		//}

		//CESERVER_REQ_GETALLTABS::TabInfo* pTabs = NULL;
		//int Count = (int)CConEmuCtrl::GetOpenedTabs(pTabs);
		//if ((Count < 1) || !pTabs)
		//{
		//	SafeFree(pTabs);
		//	return false;
		//}

		//WARNING("������ ���� �������� ������� ��� DropDown � �������");
		//HMENU hPopup = CreatePopupMenu();

		//int nLastConsole = 0;
		//for (int i = 0, k = 0; i < Count; i++, k++)
		//{
		//	if (nLastConsole != pTabs[i].ConsoleIdx)
		//	{
		//		AppendMenu(hPopup, MIF_SEPARATOR, 0, NULL);
		//		nLastConsole = pTabs[i].ConsoleIdx;
		//	}
		//	DWORD nFlags = 0
		//		| (pTabs[i].Disabled ? (MF_GRAYED|MF_DISABLED) : MF_ENABLED)
		//		| 
		//	pItems[k].Selected = (pOut->GetAllTabs.Tabs[i].ActiveConsole && pOut->GetAllTabs.Tabs[i].ActiveTab);
		//	pItems[k].Checked = pOut->GetAllTabs.Tabs[i].ActiveTab;
		//	pItems[k].Disabled = pOut->GetAllTabs.Tabs[i].Disabled;
		//	pItems[k].MsgText = pOut->GetAllTabs.Tabs[i].Title;
		//	pItems[k].UserData = i;
		//}
		//DestroyMenu(hPopup);
	}

	//int AllCount = Count + pTabs[Count-1].ConsoleIdx;
	//if (pItems)
	//{
	//}

	return true;
}

bool CConEmuCtrl::key_PasteText(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (!pRCon || pRCon->isFar())
		return false;

	return key_PasteTextAllApp(VkMod, TestOnly, hk, pRCon);
}

bool CConEmuCtrl::key_PasteTextAllApp(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	// ���� ��� GUI App in Tab - ����� ����
	if (!pRCon || pRCon->GuiWnd())
		return false;

	const Settings::AppSettings* pApp = gpSet->GetAppSettings(pRCon->GetActiveAppSettingsId());
	if (!pApp->PasteAllLines())
		return false;

	if (TestOnly)
		return true;

	pRCon->Paste(false);
	return true;
}

bool CConEmuCtrl::key_PasteFirstLine(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	if (!pRCon || pRCon->isFar())
		return false;

	return key_PasteFirstLineAllApp(VkMod, TestOnly, hk, pRCon);
}

bool CConEmuCtrl::key_PasteFirstLineAllApp(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	// ���� ��� GUI App in Tab - ����� ����
	if (!pRCon || pRCon->GuiWnd())
		return false;

	const Settings::AppSettings* pApp = gpSet->GetAppSettings(pRCon->GetActiveAppSettingsId());
	if (!pApp->PasteFirstLine())
		return false;

	if (TestOnly)
		return true;

	pRCon->Paste(true);
	return true;
}


/* ************* Service functions ************* */

void CConEmuCtrl::TabCommand(ConEmuTabCommand nTabCmd)
{
	if ((int)nTabCmd < ctc_ShowHide || (int)nTabCmd > ctc_SwitchPrev)
	{
		_ASSERTE((int)nTabCmd >= ctc_ShowHide && (int)nTabCmd <= ctc_SwitchPrev);
		return;
	}

	if (!gpConEmu->isMainThread())
	{
		PostMessage(ghWnd, gpConEmu->mn_MsgTabCommand, nTabCmd, 0);
		return;
	}

	switch (nTabCmd)
	{
		case ctc_ShowHide:
		{
			if (gpConEmu->mp_TabBar->IsTabsShown())
				gpSet->isTabs = 0;
			else
				gpSet->isTabs = 1;

			gpConEmu->ForceShowTabs(gpSet->isTabs == 1);
		} break;
		case ctc_SwitchNext:
		{
			gpConEmu->mp_TabBar->SwitchNext();
		} break;
		case ctc_SwitchPrev:
		{
			gpConEmu->mp_TabBar->SwitchPrev();
		} break;
		case ctc_SwitchCommit:
		{
			gpConEmu->mp_TabBar->SwitchCommit();
		} break;
	};
}

size_t CConEmuCtrl::GetOpenedTabs(CESERVER_REQ_GETALLTABS::TabInfo*& pTabs)
{
	_ASSERTE(pTabs==NULL);
	int nConCount = gpConEmu->GetConCount();
	int nActiveCon = gpConEmu->ActiveConNum();
	size_t cchMax = nConCount*16;
	size_t cchCount = 0;
	CVirtualConsole* pVCon;
	pTabs = (CESERVER_REQ_GETALLTABS::TabInfo*)calloc(cchMax, sizeof(*pTabs));
	for (int V = 0; (pVCon = gpConEmu->GetVCon(V)) != NULL; V++)
	{
		if (!pTabs)
		{
			_ASSERTE(pTabs!=NULL);
			break;
		}

		CRealConsole* pRCon = pVCon->RCon();
		if (!pRCon)
			continue;

		ConEmuTab tab;
		wchar_t szMark[6];
		for (int T = 0; pRCon->GetTab(T, &tab); T++)
		{
			if (cchCount >= cchMax)
			{
				pTabs = (CESERVER_REQ_GETALLTABS::TabInfo*)realloc(pTabs, (cchMax+32)*sizeof(*pTabs));
				if (!pTabs)
				{
					_ASSERTE(pTabs!=NULL);
					break;
				}
				cchMax += 32;
				_ASSERTE(cchCount<cchMax);
			}

			pTabs[cchCount].ActiveConsole = (V == nActiveCon);
			pTabs[cchCount].ActiveTab = (tab.Current != 0);
			pTabs[cchCount].Disabled = ((tab.Type & fwt_Disabled) == fwt_Disabled);
			pTabs[cchCount].ConsoleIdx = V;
			pTabs[cchCount].TabIdx = T;

			// Text
			//wcscpy_c(szMark, tab.Modified ? L" * " : L"   ");
			switch ((tab.Type & fwt_TypeMask))
			{
				case fwt_Editor:
					wcscpy_c(szMark, tab.Modified ? L" * " : L" E "); break;
				case fwt_Viewer:
					wcscpy_c(szMark, L" V "); break;
				default:
					wcscpy_c(szMark, L"   ");
			}
				
			if (V == nActiveCon)
			{
				if (T <= 9)
					_wsprintf(pTabs[cchCount].Title, SKIPLEN(countof(pTabs[cchCount].Title)) L"[%i/&%i]%s", V+1, T, szMark);
				//else if (T == 9)
				//	_wsprintf(pTabs[cchCount].Title, SKIPLEN(countof(pTabs[cchCount].Title)) L"[%i/1&0]%s", V+1, szMark);
				else
					_wsprintf(pTabs[cchCount].Title, SKIPLEN(countof(pTabs[cchCount].Title)) L"[%i/%i]%s", V+1, T, szMark);
			}
			else
			{
				_wsprintf(pTabs[cchCount].Title, SKIPLEN(countof(pTabs[cchCount].Title)) L"[%i/%i]%s", V+1, T, szMark);
			}

			int nCurLen = lstrlen(pTabs[cchCount].Title);
			lstrcpyn(pTabs[cchCount].Title+nCurLen, tab.Name, countof(pTabs[cchCount].Title)-nCurLen);

			cchCount++;
		}
	}
	return cchCount;
}

bool CConEmuCtrl::key_FindTextDlg(DWORD VkMod, bool TestOnly, const ConEmuHotKey* hk, CRealConsole* pRCon)
{
	// ���� ��� GUI App in Tab - ����
	if (!pRCon || (pRCon->GuiWnd() && !pRCon->isBufferHeight()))
		return false;
	if (TestOnly)
		return true;

	gpSetCls->FindTextDialog();
	return true;
}

void CConEmuCtrl::DoFindText(int nDirection, CRealConsole* pRCon /*= NULL*/)
{
	if (!pRCon)
	{
		pRCon = gpConEmu->ActiveCon() ? gpConEmu->ActiveCon()->RCon() : NULL;
		if (!pRCon)
			return;
	}

	pRCon->DoFindText(nDirection);
}

void CConEmuCtrl::DoEndFindText(CRealConsole* pRCon /*= NULL*/)
{
	if (!pRCon)
	{
		pRCon = gpConEmu->ActiveCon() ? gpConEmu->ActiveCon()->RCon() : NULL;
		if (!pRCon)
			return;
	}

	pRCon->DoEndFindText();
}

void CConEmuCtrl::SkipOneAppsRelease(bool abSkip)
{
	if (abSkip)
	{
		if (isPressed(VK_APPS))
		{
			// ������������ ���� ��������� VK_APPS
			mb_SkipOneAppsRelease = true;
			if (!mh_SkipOneAppsRelease)
				mh_SkipOneAppsRelease = SetWindowsHookEx(WH_GETMESSAGE, SkipOneAppsReleaseHook, NULL, GetCurrentThreadId());
		}
		else
		{
			abSkip = false;
		}
	}

	if (!abSkip)
	{
		if (mh_SkipOneAppsRelease)
		{
			UnhookWindowsHookEx(mh_SkipOneAppsRelease);
			mh_SkipOneAppsRelease = NULL;
			mb_SkipOneAppsRelease = false;
		}
	}
}

LRESULT CConEmuCtrl::SkipOneAppsReleaseHook(int code, WPARAM wParam, LPARAM lParam)
{
	if (code >= 0)
	{
		if (mb_SkipOneAppsRelease && lParam)
		{
			LPMSG pMsg = (LPMSG)lParam;

			if (pMsg->message == WM_CONTEXTMENU)
			{
				pMsg->message = WM_NULL;
				mb_SkipOneAppsRelease = false;
				return FALSE; // Skip one Apps
			}
		}
	}

	return CallNextHookEx(mh_SkipOneAppsRelease, code, wParam, lParam);
}
