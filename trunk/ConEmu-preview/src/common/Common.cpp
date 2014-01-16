﻿
/*
Copyright (c) 2009-2012 Maximus5
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
#include <windows.h>
#include "common.hpp"



BOOL PackInputRecord(const INPUT_RECORD* piRec, MSG64::MsgStr* pMsg)
{
	if (!pMsg || !piRec)
	{
		_ASSERTE(pMsg!=NULL && piRec!=NULL);
		return FALSE;
	}

	memset(pMsg, 0, sizeof(*pMsg));
	//pMsg->cbSize = sizeof(*pMsg);

	UINT nMsg = 0; WPARAM wParam = 0; LPARAM lParam = 0;

	if (piRec->EventType == KEY_EVENT)
	{
		nMsg = piRec->Event.KeyEvent.bKeyDown ? WM_KEYDOWN : WM_KEYUP;
		lParam |= (DWORD_PTR)(WORD)piRec->Event.KeyEvent.uChar.UnicodeChar;
		lParam |= ((DWORD_PTR)piRec->Event.KeyEvent.wVirtualKeyCode & 0xFF) << 16;
		lParam |= ((DWORD_PTR)piRec->Event.KeyEvent.wVirtualScanCode & 0xFF) << 24;
		wParam |= (DWORD_PTR)piRec->Event.KeyEvent.dwControlKeyState & 0xFFFF;
		wParam |= ((DWORD_PTR)piRec->Event.KeyEvent.wRepeatCount & 0xFF) << 16;
	}
	else if (piRec->EventType == MOUSE_EVENT)
	{
		switch(piRec->Event.MouseEvent.dwEventFlags)
		{
			case MOUSE_MOVED:
				nMsg = MOUSE_EVENT_MOVE;
				break;
			case 0:
				nMsg = MOUSE_EVENT_CLICK;
				break;
			case DOUBLE_CLICK:
				nMsg = MOUSE_EVENT_DBLCLICK;
				break;
			case MOUSE_WHEELED:
				nMsg = MOUSE_EVENT_WHEELED;
				break;
			case /*MOUSE_HWHEELED*/ 0x0008:
				nMsg = MOUSE_EVENT_HWHEELED;
				break;
			default:
				_ASSERT(FALSE);
		}

		lParam = ((DWORD_PTR)(WORD)piRec->Event.MouseEvent.dwMousePosition.X)
		         | (((DWORD_PTR)(WORD)piRec->Event.MouseEvent.dwMousePosition.Y) << 16);
		// max 0x0010/*FROM_LEFT_4ND_BUTTON_PRESSED*/
		wParam |= ((DWORD_PTR)piRec->Event.MouseEvent.dwButtonState) & 0xFF;
		// max - ENHANCED_KEY == 0x0100
		wParam |= (((DWORD_PTR)piRec->Event.MouseEvent.dwControlKeyState) & 0xFFFF) << 8;

		if (nMsg == MOUSE_EVENT_WHEELED || nMsg == MOUSE_EVENT_HWHEELED)
		{
			// HIWORD() - short (direction[1/-1])*count*120
			short nWheel = (short)((((DWORD)piRec->Event.MouseEvent.dwButtonState) & 0xFFFF0000) >> 16);
			char  nCount = nWheel / 120;
			wParam |= ((DWORD_PTR)(BYTE)nCount) << 24;
		}
	}
	else if (piRec->EventType == FOCUS_EVENT)
	{
		nMsg = piRec->Event.FocusEvent.bSetFocus ? WM_SETFOCUS : WM_KILLFOCUS;
	}
	else
	{
		_ASSERT(FALSE);
		return FALSE;
	}

	_ASSERTE(nMsg!=0);
	pMsg->message = nMsg;
	pMsg->wParam = wParam;
	pMsg->lParam = lParam;
	return TRUE;
}

BOOL UnpackInputRecord(const MSG64::MsgStr* piMsg, INPUT_RECORD* pRec)
{
	if (!piMsg || !pRec)
	{
		_ASSERTE(piMsg!=NULL && pRec!=NULL);
		return FALSE;
	}

	memset(pRec, 0, sizeof(INPUT_RECORD));

	if (piMsg->message == 0)
		return FALSE;

	if (piMsg->message == WM_KEYDOWN || piMsg->message == WM_KEYUP)
	{
		pRec->EventType = KEY_EVENT;
		// lParam
		pRec->Event.KeyEvent.bKeyDown = (piMsg->message == WM_KEYDOWN);
		pRec->Event.KeyEvent.uChar.UnicodeChar = (WCHAR)(piMsg->lParam & 0xFFFF);
		pRec->Event.KeyEvent.wVirtualKeyCode   = (((DWORD)piMsg->lParam) & 0xFF0000) >> 16;
		pRec->Event.KeyEvent.wVirtualScanCode  = (((DWORD)piMsg->lParam) & 0xFF000000) >> 24;
		// wParam. Пока что тут может быть max(ENHANCED_KEY==0x0100)
		pRec->Event.KeyEvent.dwControlKeyState = ((DWORD)piMsg->wParam & 0xFFFF);
		pRec->Event.KeyEvent.wRepeatCount = ((DWORD)piMsg->wParam & 0xFF0000) >> 16;
	}
	else if (piMsg->message >= MOUSE_EVENT_FIRST && piMsg->message <= MOUSE_EVENT_LAST)
	{
		pRec->EventType = MOUSE_EVENT;

		switch(piMsg->message)
		{
			case MOUSE_EVENT_MOVE:
				pRec->Event.MouseEvent.dwEventFlags = MOUSE_MOVED;
				break;
			case MOUSE_EVENT_CLICK:
				pRec->Event.MouseEvent.dwEventFlags = 0;
				break;
			case MOUSE_EVENT_DBLCLICK:
				pRec->Event.MouseEvent.dwEventFlags = DOUBLE_CLICK;
				break;
			case MOUSE_EVENT_WHEELED:
				pRec->Event.MouseEvent.dwEventFlags = MOUSE_WHEELED;
				break;
			case MOUSE_EVENT_HWHEELED:
				pRec->Event.MouseEvent.dwEventFlags = /*MOUSE_HWHEELED*/ 0x0008;
				break;
		}

		pRec->Event.MouseEvent.dwMousePosition.X = LOWORD(piMsg->lParam);
		pRec->Event.MouseEvent.dwMousePosition.Y = HIWORD(piMsg->lParam);
		// max 0x0010/*FROM_LEFT_4ND_BUTTON_PRESSED*/
		pRec->Event.MouseEvent.dwButtonState = ((DWORD)piMsg->wParam) & 0xFF;
		// max - ENHANCED_KEY == 0x0100
		pRec->Event.MouseEvent.dwControlKeyState = (((DWORD)piMsg->wParam) & 0xFFFF00) >> 8;

		if (piMsg->message == MOUSE_EVENT_WHEELED || piMsg->message == MOUSE_EVENT_HWHEELED)
		{
			// HIWORD() - short (direction[1/-1])*count*120
			signed char nDir = (signed char)((((DWORD)piMsg->wParam) & 0xFF000000) >> 24);
			WORD wDir = nDir*120;
			pRec->Event.MouseEvent.dwButtonState |= wDir << 16;
		}
	}
	else if (piMsg->message == WM_SETFOCUS || piMsg->message == WM_KILLFOCUS)
	{
		pRec->EventType = FOCUS_EVENT;
		pRec->Event.FocusEvent.bSetFocus = (piMsg->message == WM_SETFOCUS);
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

void TranslateKeyPress(WORD vkKey, DWORD dwControlState, wchar_t wch, int ScanCode, INPUT_RECORD* rDown, INPUT_RECORD* rUp)
{
	// Может приходить запрос на отсылку даже если текущий буфер НЕ rbt_Primary,
	// например, при начале выделения и автоматическом переключении на альтернативный буфер

	if (!vkKey && !dwControlState && wch)
	{
		USHORT vk = VkKeyScan(wch);
		if (vk && (vk != 0xFFFF))
		{
			vkKey = (vk & 0xFF);
			vk = vk >> 8;
			if ((vk & 7) == 6)
			{
				// For keyboard layouts that use the right-hand ALT key as a shift
				// key (for example, the French keyboard layout), the shift state is
				// represented by the value 6, because the right-hand ALT key is
				// converted internally into CTRL+ALT.
				dwControlState |= SHIFT_PRESSED;
			}
			else
			{
				if (vk & 1)
					dwControlState |= SHIFT_PRESSED;
				if (vk & 2)
					dwControlState |= LEFT_CTRL_PRESSED;
				if (vk & 4)
					dwControlState |= LEFT_ALT_PRESSED;
			}
		}
	}

	if (ScanCode == -1)
		ScanCode = MapVirtualKey(vkKey, 0/*MAPVK_VK_TO_VSC*/);

	INPUT_RECORD r = {KEY_EVENT};
	r.Event.KeyEvent.bKeyDown = TRUE;
	r.Event.KeyEvent.wRepeatCount = 1;
	r.Event.KeyEvent.wVirtualKeyCode = vkKey;
	r.Event.KeyEvent.wVirtualScanCode = ScanCode;
	r.Event.KeyEvent.uChar.UnicodeChar = wch;
	r.Event.KeyEvent.dwControlKeyState = dwControlState;
	*rDown = r;

	TODO("Может нужно в dwControlKeyState применять модификатор, если он и есть vkKey?");

	r.Event.KeyEvent.bKeyDown = FALSE;
	r.Event.KeyEvent.dwControlKeyState = dwControlState;
	*rUp = r;
}


//#ifdef CONEMU_MINIMAL
//#include "base64.h"
//#endif


BOOL gbInCommonShutdown = FALSE;
#ifdef _DEBUG
extern HANDLE ghInMyAssertTrap;
extern DWORD gnInMyAssertThread;
#endif

ShutdownConsole_t OnShutdownConsole = NULL;

void CommonShutdown()
{
	gbInCommonShutdown = TRUE;

	ShutdownSecurity();

	// Clean memory
	if (OnShutdownConsole)
		OnShutdownConsole();


#ifdef _DEBUG
	MyAssertShutdown();
#endif
}
