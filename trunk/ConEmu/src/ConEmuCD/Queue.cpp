
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

#include "ConEmuC.h"
#include "Queue.h"

#define DEBUGSTRINPUTPIPE(s) //DEBUGSTR(s) // ConEmuC: Recieved key... / ConEmuC: Recieved input
#define DEBUGSTRINPUTEVENT(s) //DEBUGSTR(s) // SetEvent(gpSrv->hInputEvent)
#define DEBUGLOGINPUT(s) DEBUGSTR(s) // ConEmuC.MouseEvent(X=
#define DEBUGSTRINPUTWRITE(s) DEBUGSTR(s) // *** ConEmuC.MouseEvent(X=
#define DEBUGSTRINPUTWRITEALL(s) //DEBUGSTR(s) // *** WriteConsoleInput(Write=
#define DEBUGSTRINPUTWRITEFAIL(s) DEBUGSTR(s) // ### WriteConsoleInput(Write=

BOOL ProcessInputMessage(MSG64 &msg, INPUT_RECORD &r)
{
	memset(&r, 0, sizeof(r));
	BOOL lbOk = FALSE;

	if (!UnpackInputRecord(&msg, &r))
	{
		_ASSERT(FALSE);
	}
	else
	{
		TODO("������� ��������� ����� ���������, ����� ��� ���������� � �������?");
		//#ifdef _DEBUG
		//if (r.EventType == KEY_EVENT && (r.Event.KeyEvent.wVirtualKeyCode == 'C' || r.Event.KeyEvent.wVirtualKeyCode == VK_CANCEL))
		//{
		//	DEBUGSTR(L"  ---  CtrlC/CtrlBreak recieved\n");
		//}
		//#endif
		bool lbProcessEvent = false;
		bool lbIngoreKey = false;

		if (r.EventType == KEY_EVENT && r.Event.KeyEvent.bKeyDown &&
			(r.Event.KeyEvent.wVirtualKeyCode == 'C' || r.Event.KeyEvent.wVirtualKeyCode == VK_CANCEL)
			&& (      // ������������ ������ Ctrl
			(r.Event.KeyEvent.dwControlKeyState & CTRL_MODIFIERS) &&
			((r.Event.KeyEvent.dwControlKeyState & ALL_MODIFIERS)
			== (r.Event.KeyEvent.dwControlKeyState & CTRL_MODIFIERS))
			)
			)
		{
			lbProcessEvent = true;
			DEBUGSTR(L"  ---  CtrlC/CtrlBreak recieved\n");
			DWORD dwMode = 0;
			GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &dwMode);

			// CTRL+C (and Ctrl+Break?) is processed by the system and is not placed in the input buffer
			if ((dwMode & ENABLE_PROCESSED_INPUT) == ENABLE_PROCESSED_INPUT)
				lbIngoreKey = lbProcessEvent = true;
			else
				lbProcessEvent = false;

			if (lbProcessEvent)
			{
				BOOL lbRc = FALSE;
				DWORD dwEvent = (r.Event.KeyEvent.wVirtualKeyCode == 'C') ? CTRL_C_EVENT : CTRL_BREAK_EVENT;
				//&& (gpSrv->dwConsoleMode & ENABLE_PROCESSED_INPUT)

				//The SetConsoleMode function can disable the ENABLE_PROCESSED_INPUT mode for a console's input buffer,
				//so CTRL+C is reported as keyboard input rather than as a signal.
				// CTRL+BREAK is always treated as a signal
				if (  // ������������ ������ Ctrl
					(r.Event.KeyEvent.dwControlKeyState & CTRL_MODIFIERS) &&
					((r.Event.KeyEvent.dwControlKeyState & ALL_MODIFIERS)
					== (r.Event.KeyEvent.dwControlKeyState & CTRL_MODIFIERS))
					)
				{
					// ����� ��������, ������� �� ��������� ������� � ������ CREATE_NEW_PROCESS_GROUP
					// ����� � ��������������� ������� (WinXP SP3) ������ �����, � ��� ���������
					// �� Ctrl-Break, �� ������� ���������� Ctrl-C
					lbRc = GenerateConsoleCtrlEvent(dwEvent, 0);
					// ��� ������� (Ctrl+C) � ����� ����������(!) ����� �� ���� �� ������ ���������� ������� C � ������� Ctrl
				}
			}

			if (lbIngoreKey)
				return FALSE;

			// CtrlBreak �������� �����, ���� �������, ����� ������� FAR ������ ��������
			if (r.Event.KeyEvent.wVirtualKeyCode == VK_CANCEL)
			{
				// ��� ��������� CtrlBreak � �������� ������� - ����� ����� ���������
				// ����� ���, ��� ������� ������� ���� ������� ������,
				// ��� �� ������������ �������, � CtrlBreak �������������
				FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
				SendConsoleEvent(&r, 1);
				return FALSE;
			}
		}

#ifdef _DEBUG

		if (r.EventType == KEY_EVENT && r.Event.KeyEvent.bKeyDown &&
			r.Event.KeyEvent.wVirtualKeyCode == VK_F11)
		{
			DEBUGSTR(L"  ---  F11 recieved\n");
		}

#endif
#ifdef _DEBUG

		if (r.EventType == MOUSE_EVENT)
		{
			static DWORD nLastEventTick = 0;

			if (nLastEventTick && (GetTickCount() - nLastEventTick) > 2000)
			{
				OutputDebugString(L".\n");
			}

			wchar_t szDbg[60];
			_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"    ConEmuC.MouseEvent(X=%i,Y=%i,Btns=0x%04x,Moved=%i)\n", r.Event.MouseEvent.dwMousePosition.X, r.Event.MouseEvent.dwMousePosition.Y, r.Event.MouseEvent.dwButtonState, (r.Event.MouseEvent.dwEventFlags & MOUSE_MOVED));
			DEBUGLOGINPUT(szDbg);
			nLastEventTick = GetTickCount();
		}

#endif

		// ���������, ����� ���� ��������� ���������� ������������
		if (r.EventType == KEY_EVENT
			|| (r.EventType == MOUSE_EVENT
			&& (r.Event.MouseEvent.dwButtonState || r.Event.MouseEvent.dwEventFlags
			|| r.Event.MouseEvent.dwEventFlags == DOUBLE_CLICK)))
		{
			gpSrv->dwLastUserTick = GetTickCount();
		}

		lbOk = TRUE;
		//SendConsoleEvent(&r, 1);
	}

	return lbOk;
}

//// gpSrv->hInputThread && gpSrv->dwInputThreadId
//DWORD WINAPI InputThread(LPVOID lpvParam)
//{
//	MSG msg;
//	//while (GetMessage(&msg,0,0,0))
//	INPUT_RECORD rr[MAX_EVENTS_PACK];
//
//	while (TRUE) {
//		if (!PeekMessage(&msg,0,0,0,PM_REMOVE)) {
//			Sleep(10);
//			continue;
//		}
//		if (msg.message == WM_QUIT)
//			break;
//
//		if (ghQuitEvent) {
//			if (WaitForSingleObject(ghQuitEvent, 0) == WAIT_OBJECT_0)
//				break;
//		}
//		if (msg.message == WM_NULL) {
//			_ASSERTE(msg.message != WM_NULL);
//			continue;
//		}
//
//		//if (msg.message == INPUT_THREAD_ALIVE_MSG) {
//		//	//pRCon->mn_FlushOut = msg.wParam;
//		//	TODO("INPUT_THREAD_ALIVE_MSG");
//		//	continue;
//		//
//		//} else {
//
//		// ��������� ����� ���������, ����� ��� ���������� � �������?
//		UINT nCount = 0;
//		while (nCount < MAX_EVENTS_PACK)
//		{
//			if (msg.message == WM_NULL) {
//				_ASSERTE(msg.message != WM_NULL);
//			} else {
//				if (ProcessInputMessage(msg, rr[nCount]))
//					nCount++;
//			}
//			if (!PeekMessage(&msg,0,0,0,PM_REMOVE))
//				break;
//			if (msg.message == WM_QUIT)
//				break;
//		}
//		if (nCount && msg.message != WM_QUIT) {
//			SendConsoleEvent(rr, nCount);
//		}
//		//}
//	}
//
//	return 0;
//}

BOOL WriteInputQueue(const INPUT_RECORD *pr)
{
	INPUT_RECORD* pNext = gpSrv->pInputQueueWrite;

	// ���������, ���� �� ��������� ����� � ������
	if (gpSrv->pInputQueueRead != gpSrv->pInputQueueEnd)
	{
		if (gpSrv->pInputQueueRead < gpSrv->pInputQueueEnd
			&& ((gpSrv->pInputQueueWrite+1) == gpSrv->pInputQueueRead))
		{
			return FALSE;
		}
	}

	// OK
	*pNext = *pr;
	gpSrv->pInputQueueWrite++;

	if (gpSrv->pInputQueueWrite >= gpSrv->pInputQueueEnd)
		gpSrv->pInputQueueWrite = gpSrv->pInputQueue;

	DEBUGSTRINPUTEVENT(L"SetEvent(gpSrv->hInputEvent)\n");
	SetEvent(gpSrv->hInputEvent);

	// ��������� ��������� ������, ���� �� ����� ����� ��� ����
	if (gpSrv->pInputQueueRead == gpSrv->pInputQueueEnd)
		gpSrv->pInputQueueRead = pNext;

	return TRUE;
}

BOOL IsInputQueueEmpty()
{
	if (gpSrv->pInputQueueRead != gpSrv->pInputQueueEnd
		&& gpSrv->pInputQueueRead != gpSrv->pInputQueueWrite)
		return FALSE;

	return TRUE;
}

BOOL ReadInputQueue(INPUT_RECORD *prs, DWORD *pCount)
{
	DWORD nCount = 0;

	if (!IsInputQueueEmpty())
	{
		DWORD n = *pCount;
		INPUT_RECORD *pSrc = gpSrv->pInputQueueRead;
		INPUT_RECORD *pEnd = (gpSrv->pInputQueueRead < gpSrv->pInputQueueWrite) ? gpSrv->pInputQueueWrite : gpSrv->pInputQueueEnd;
		INPUT_RECORD *pDst = prs;

		while(n && pSrc < pEnd)
		{
			*pDst = *pSrc; nCount++; pSrc++;
			//// ��� ���������� ��������� � ������������ RealConsole&Far
			//if (pDst->EventType == KEY_EVENT
			//	// ��� ������� �� ���������� ������
			//	&& pDst->Event.KeyEvent.bKeyDown && pDst->Event.KeyEvent.uChar.UnicodeChar < 32
			//	&& pSrc < (pEnd = (gpSrv->pInputQueueRead < gpSrv->pInputQueueWrite) ? gpSrv->pInputQueueWrite : gpSrv->pInputQueueEnd)) // � ���� � ������ ��� ���-�� ����
			//{
			//	while (pSrc < (pEnd = (gpSrv->pInputQueueRead < gpSrv->pInputQueueWrite) ? gpSrv->pInputQueueWrite : gpSrv->pInputQueueEnd)
			//		&& pSrc->EventType == KEY_EVENT
			//		&& pSrc->Event.KeyEvent.bKeyDown
			//		&& pSrc->Event.KeyEvent.wVirtualKeyCode == pDst->Event.KeyEvent.wVirtualKeyCode
			//		&& pSrc->Event.KeyEvent.wVirtualScanCode == pDst->Event.KeyEvent.wVirtualScanCode
			//		&& pSrc->Event.KeyEvent.uChar.UnicodeChar == pDst->Event.KeyEvent.uChar.UnicodeChar
			//		&& pSrc->Event.KeyEvent.dwControlKeyState == pDst->Event.KeyEvent.dwControlKeyState)
			//	{
			//		pDst->Event.KeyEvent.wRepeatCount++; pSrc++;
			//	}
			//}
			n--; pDst++;
		}

		if (pSrc == gpSrv->pInputQueueEnd)
			pSrc = gpSrv->pInputQueue;

		TODO("�������� ������ ������ ������, ���� ������� ��� �����");
		//
		gpSrv->pInputQueueRead = pSrc;
	}

	*pCount = nCount;
	return (nCount>0);
}

#ifdef _DEBUG
BOOL GetNumberOfBufferEvents()
{
	DWORD nCount = 0;

	if (!IsInputQueueEmpty())
	{
		INPUT_RECORD *pSrc = gpSrv->pInputQueueRead;
		INPUT_RECORD *pEnd = (gpSrv->pInputQueueRead < gpSrv->pInputQueueWrite) ? gpSrv->pInputQueueWrite : gpSrv->pInputQueueEnd;

		while(pSrc < pEnd)
		{
			nCount++; pSrc++;
		}

		if (pSrc == gpSrv->pInputQueueEnd)
		{
			pSrc = gpSrv->pInputQueue;
			pEnd = (gpSrv->pInputQueueRead < gpSrv->pInputQueueWrite) ? gpSrv->pInputQueueWrite : gpSrv->pInputQueueEnd;

			while(pSrc < pEnd)
			{
				nCount++; pSrc++;
			}
		}
	}

	return nCount;
}
#endif

// ���������, ���� ���������� ����� ����� ������� ������� �����
// ���������� FALSE, ���� ������ �����������!
BOOL WaitConsoleReady(BOOL abReqEmpty)
{
	// ���� ������ ���� ������ - ������������ ��������� � ����� �������
	if (gpSrv->bInSyncResize)
		WaitForSingleObject(gpSrv->hAllowInputEvent, MAX_SYNCSETSIZE_WAIT);

	// ���� ����� �������� ������� ������� - ��������� ����������� 'Right selection fix'!

	DWORD nQuitWait = WaitForSingleObject(ghQuitEvent, 0);

	if (nQuitWait == WAIT_OBJECT_0)
		return FALSE;

	if (abReqEmpty)
	{
		//#ifdef USE_INPUT_SEMAPHORE
		//// ��� ������ ������������ ������ � ��������� �����
		//_ASSERTE(FALSE);
		//#endif

		DWORD nCurInputCount = 0; //, cbWritten = 0;
		//INPUT_RECORD irDummy[2] = {{0},{0}};
		HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE); // ��� ��� ghConIn

		// 27.06.2009 Maks - If input queue is not empty - wait for a while, to avoid conflicts with FAR reading queue
		// 19.02.2010 Maks - ������ �� GetNumberOfConsoleInputEvents
		//if (PeekConsoleInput(hIn, irDummy, 1, &(nCurInputCount = 0)) && nCurInputCount > 0) {
		if (GetNumberOfConsoleInputEvents(hIn, &(nCurInputCount = 0)) && nCurInputCount > 0)
		{
			DWORD dwStartTick = GetTickCount(), dwDelta, dwTick;

			do
			{
				//Sleep(5);
				nQuitWait = WaitForSingleObject(ghQuitEvent, 5);

				if (nQuitWait == WAIT_OBJECT_0)
					return FALSE;

				//if (!PeekConsoleInput(hIn, irDummy, 1, &(nCurInputCount = 0)))
				if (!GetNumberOfConsoleInputEvents(hIn, &(nCurInputCount = 0)))
					nCurInputCount = 0;

				dwTick = GetTickCount(); dwDelta = dwTick - dwStartTick;
			}
			while((nCurInputCount > 0) && (dwDelta < MAX_INPUT_QUEUE_EMPTY_WAIT));
		}

		if (WaitForSingleObject(ghQuitEvent, 0) == WAIT_OBJECT_0)
			return FALSE;
	}

	//return (nCurInputCount == 0);
	return TRUE; // ���� ����� - ������ TRUE
}

BOOL SendConsoleEvent(INPUT_RECORD* pr, UINT nCount)
{
	if (!nCount || !pr)
	{
		_ASSERTE(nCount>0 && pr!=NULL);
		return FALSE;
	}

	BOOL fSuccess = FALSE;
	//// ���� ������ ���� ������ - ������������ ��������� � ����� �������
	//if (gpSrv->bInSyncResize)
	//	WaitForSingleObject(gpSrv->hAllowInputEvent, MAX_SYNCSETSIZE_WAIT);
	//DWORD nCurInputCount = 0, cbWritten = 0;
	//INPUT_RECORD irDummy[2] = {{0},{0}};
	//HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE); // ��� ��� ghConIn
	// 02.04.2010 Maks - ���������� � WaitConsoleReady
	//// 27.06.2009 Maks - If input queue is not empty - wait for a while, to avoid conflicts with FAR reading queue
	//// 19.02.2010 Maks - ������ �� GetNumberOfConsoleInputEvents
	////if (PeekConsoleInput(hIn, irDummy, 1, &(nCurInputCount = 0)) && nCurInputCount > 0) {
	//if (GetNumberOfConsoleInputEvents(hIn, &(nCurInputCount = 0)) && nCurInputCount > 0) {
	//	DWORD dwStartTick = GetTickCount();
	//	WARNING("Do NOT wait, but place event in Cyclic queue");
	//	do {
	//		Sleep(5);
	//		//if (!PeekConsoleInput(hIn, irDummy, 1, &(nCurInputCount = 0)))
	//		if (!GetNumberOfConsoleInputEvents(hIn, &(nCurInputCount = 0)))
	//			nCurInputCount = 0;
	//	} while ((nCurInputCount > 0) && ((GetTickCount() - dwStartTick) < MAX_INPUT_QUEUE_EMPTY_WAIT));
	//}
	INPUT_RECORD* prNew = NULL;
	int nAllCount = 0;
	BOOL lbReqEmpty = FALSE;

	for(UINT n = 0; n < nCount; n++)
	{
		if (pr[n].EventType != KEY_EVENT)
		{
			nAllCount++;
			if (!lbReqEmpty && (pr[n].EventType == MOUSE_EVENT))
			{
				// �� ���� ��������� ����� ������� Windows.
				// ���� � ������ ������ ��� ���� ������� �������, �� ������
				// � ����� ���������� ��, �� ����������� ���������� �������� 0-�������.
				// � �����, �������� ������� ��������� �������, ��� ����� ��������� 
				// ��� ��������� ���� ������ ������ ������� ���� (��������� � ������� �������)
				if (pr[n].Event.MouseEvent.dwButtonState /*== RIGHTMOST_BUTTON_PRESSED*/)
					lbReqEmpty = TRUE;
			}
		}
		else
		{
			if (!pr[n].Event.KeyEvent.wRepeatCount)
			{
				_ASSERTE(pr[n].Event.KeyEvent.wRepeatCount!=0);
				pr[n].Event.KeyEvent.wRepeatCount = 1;
			}

			nAllCount += pr[n].Event.KeyEvent.wRepeatCount;
		}
	}

	if (nAllCount > (int)nCount)
	{
		prNew = (INPUT_RECORD*)malloc(sizeof(INPUT_RECORD)*nAllCount);

		if (prNew)
		{
			INPUT_RECORD* ppr = prNew;
			INPUT_RECORD* pprMod = NULL;

			for(UINT n = 0; n < nCount; n++)
			{
				*(ppr++) = pr[n];

				if (pr[n].EventType == KEY_EVENT)
				{
					UINT nCurCount = pr[n].Event.KeyEvent.wRepeatCount;

					if (nCurCount > 1)
					{
						pprMod = (ppr-1);
						pprMod->Event.KeyEvent.wRepeatCount = 1;

						for(UINT i = 1; i < nCurCount; i++)
						{
							*(ppr++) = *pprMod;
						}
					}
				}
				else if (!lbReqEmpty && (pr[n].EventType == MOUSE_EVENT))
				{
					// �� ���� ��������� ����� ������� Windows.
					// ���� � ������ ������ ��� ���� ������� �������, �� ������
					// � ����� ���������� ��, �� ����������� ���������� �������� 0-�������.
					// � �����, �������� ������� ��������� �������, ��� ����� ��������� 
					// ��� ��������� ���� ������ ������ ������� ���� (��������� � ������� �������)
					if (pr[n].Event.MouseEvent.dwButtonState == RIGHTMOST_BUTTON_PRESSED)
						lbReqEmpty = TRUE;
				}
			}

			pr = prNew;
			_ASSERTE(nAllCount == (ppr-prNew));
			nCount = (UINT)(ppr-prNew);
		}
	}

	// ���� �� ����� - ��� ����� �������
	WaitConsoleReady(lbReqEmpty);


	DWORD cbWritten = 0;
#ifdef _DEBUG
	wchar_t szDbg[255];
	for (UINT i = 0; i < nCount; i++)
	{
		if (pr[i].EventType == MOUSE_EVENT)
		{
			_wsprintf(szDbg, SKIPLEN(countof(szDbg))
				L"*** ConEmuC.MouseEvent(X=%i,Y=%i,Btns=0x%04x,Moved=%i)\n",
				pr[i].Event.MouseEvent.dwMousePosition.X, pr[i].Event.MouseEvent.dwMousePosition.Y, pr[i].Event.MouseEvent.dwButtonState, (pr[i].Event.MouseEvent.dwEventFlags & MOUSE_MOVED));
			DEBUGSTRINPUTWRITE(szDbg);

#ifdef _DEBUG
			{
				static int LastMsButton;
				if ((LastMsButton & 1) && (pr[i].Event.MouseEvent.dwButtonState == 0))
				{
					// LButton was Down, now - Up
					LastMsButton = pr[i].Event.MouseEvent.dwButtonState;
				}
				else if (!LastMsButton && (pr[i].Event.MouseEvent.dwButtonState & 1))
				{
					// LButton was Up, now - Down
					LastMsButton = pr[i].Event.MouseEvent.dwButtonState;
				}
				else
				{ //-V523
					LastMsButton = pr[i].Event.MouseEvent.dwButtonState;
				}
			}
#endif

		}
	}
	SetLastError(0);
#endif
	HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE); // ��� ��� ghConIn
	fSuccess = WriteConsoleInput(hIn, pr, nCount, &cbWritten);
#ifdef _DEBUG
	DWORD dwErr = GetLastError();
	if (!fSuccess || (nCount != cbWritten))
	{
		_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"### WriteConsoleInput(Write=%i, Written=%i, Left=%i, Err=x%X)\n", nCount, cbWritten, GetNumberOfBufferEvents(), dwErr);
		DEBUGSTRINPUTWRITEFAIL(szDbg);
	}
	else
	{
		_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"*** WriteConsoleInput(Write=%i, Written=%i, Left=%i)\n", nCount, cbWritten, GetNumberOfBufferEvents());
		DEBUGSTRINPUTWRITEALL(szDbg);
	}
#endif
	_ASSERTE(fSuccess && cbWritten==nCount);

	if (prNew) free(prNew);

	return fSuccess;
}

DWORD WINAPI InputThread(LPVOID lpvParam)
{
	HANDLE hEvents[2] = {ghQuitEvent, gpSrv->hInputEvent};
	DWORD dwWait = 0;
	INPUT_RECORD ir[100];

	while ((dwWait = WaitForMultipleObjects(2, hEvents, FALSE, INPUT_QUEUE_TIMEOUT)) != WAIT_OBJECT_0)
	{
		if (IsInputQueueEmpty())
			continue;

		// -- ���������� � SendConsoleEvent
		//// ���� �� ����� - ��� ����� �������
		//if (!WaitConsoleReady())
		//	break;

		// ������ � �����
		DWORD nInputCount = sizeof(ir)/sizeof(ir[0]);

		//#ifdef USE_INPUT_SEMAPHORE
		//DWORD nSemaphore = ghConInSemaphore ? WaitForSingleObject(ghConInSemaphore, INSEMTIMEOUT_WRITE) : 1;
		//_ASSERTE(ghConInSemaphore && (nSemaphore == WAIT_OBJECT_0));
		//#endif

		if (ReadInputQueue(ir, &nInputCount))
		{
			_ASSERTE(nInputCount>0);

			#ifdef _DEBUG
			for(DWORD j = 0; j < nInputCount; j++)
			{
				if (ir[j].EventType == KEY_EVENT
					&& (ir[j].Event.KeyEvent.wVirtualKeyCode == 'C' || ir[j].Event.KeyEvent.wVirtualKeyCode == VK_CANCEL)
					&& (      // ������������ ������ Ctrl
					(ir[j].Event.KeyEvent.dwControlKeyState & CTRL_MODIFIERS) &&
					((ir[j].Event.KeyEvent.dwControlKeyState & ALL_MODIFIERS)
					== (ir[j].Event.KeyEvent.dwControlKeyState & CTRL_MODIFIERS)))
					)
				{
					DEBUGSTR(L"  ---  CtrlC/CtrlBreak recieved\n");
				}
			}
			#endif

			//DEBUGSTRINPUTPIPE(L"SendConsoleEvent\n");
			SendConsoleEvent(ir, nInputCount);
		}

		//#ifdef USE_INPUT_SEMAPHORE
		//if ((nSemaphore == WAIT_OBJECT_0) && ghConInSemaphore) ReleaseSemaphore(ghConInSemaphore, 1, NULL);
		//#endif

		// ���� �� ����� ������ � ������� � ������ ��� ���-�� ��������� - ����������
		if (!IsInputQueueEmpty())
			SetEvent(gpSrv->hInputEvent);
	}

	return 1;
}

//DWORD WINAPI InputPipeThread(LPVOID lpvParam)
//{
//	BOOL fConnected, fSuccess;
//	//DWORD nCurInputCount = 0;
//	//DWORD gpSrv->dwServerThreadId;
//	//HANDLE hPipe = NULL;
//	DWORD dwErr = 0;
//
//	// The main loop creates an instance of the named pipe and
//	// then waits for a client to connect to it. When the client
//	// connects, a thread is created to handle communications
//	// with that client, and the loop is repeated.
//
//	while(!gbQuit)
//	{
//		MCHKHEAP;
//		gpSrv->hInputPipe = Create NamedPipe(
//			gpSrv->szInputname,          // pipe name
//			PIPE_ACCESS_INBOUND,      // goes from client to server only
//			PIPE_TYPE_MESSAGE |       // message type pipe
//			PIPE_READMODE_MESSAGE |   // message-read mode
//			PIPE_WAIT,                // blocking mode
//			PIPE_UNLIMITED_INSTANCES, // max. instances
//			PIPEBUFSIZE,              // output buffer size
//			PIPEBUFSIZE,              // input buffer size
//			0,                        // client time-out
//			gpLocalSecurity);          // default security attribute
//
//		if (gpSrv->hInputPipe == INVALID_HANDLE_VALUE)
//		{
//			dwErr = GetLastError();
//			_ASSERTE(gpSrv->hInputPipe != INVALID_HANDLE_VALUE);
//			_printf("CreatePipe failed, ErrCode=0x%08X\n", dwErr);
//			Sleep(50);
//			//return 99;
//			continue;
//		}
//
//		// Wait for the client to connect; if it succeeds,
//		// the function returns a nonzero value. If the function
//		// returns zero, GetLastError returns ERROR_PIPE_CONNECTED.
//		fConnected = ConnectNamedPipe(gpSrv->hInputPipe, NULL) ?
//TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
//		MCHKHEAP;
//
//		if (fConnected)
//		{
//			//TODO:
//			DWORD cbBytesRead; //, cbWritten;
//			MSG64 imsg = {};
//
//			while(!gbQuit && (fSuccess = ReadFile(
//				gpSrv->hInputPipe,        // handle to pipe
//				&imsg,        // buffer to receive data
//				sizeof(imsg), // size of buffer
//				&cbBytesRead, // number of bytes read
//				NULL)) != FALSE)        // not overlapped I/O
//			{
//				// ������������� ����������� ���������� ����
//				if (gbQuit)
//					break;
//
//				MCHKHEAP;
//
//				if (imsg.message)
//				{
//#ifdef _DEBUG
//
//					switch(imsg.message)
//					{
//					case WM_KEYDOWN: case WM_SYSKEYDOWN: DEBUGSTRINPUTPIPE(L"ConEmuC: Recieved key down\n"); break;
//					case WM_KEYUP: case WM_SYSKEYUP: DEBUGSTRINPUTPIPE(L"ConEmuC: Recieved key up\n"); break;
//					default: DEBUGSTRINPUTPIPE(L"ConEmuC: Recieved input\n");
//					}
//
//#endif
//					INPUT_RECORD r;
//
//					// ������������ ������� - �����������,
//					// ��������� ������� (CtrlC/CtrlBreak) �� ������� � �������� ������
//					if (ProcessInputMessage(imsg, r))
//					{
//						//SendConsoleEvent(&r, 1);
//						if (!WriteInputQueue(&r))
//						{
//							_ASSERTE(FALSE);
//							WARNING("���� ����� ���������� - �����? ���� ���� ����� ����� ����� - ����� ��������� GUI �� ������ � pipe...");
//						}
//					}
//
//					MCHKHEAP;
//				}
//
//				// next
//				memset(&imsg,0,sizeof(imsg));
//				MCHKHEAP;
//			}
//
//			SafeCloseHandle(gpSrv->hInputPipe);
//		}
//		else
//			// The client could not connect, so close the pipe.
//			SafeCloseHandle(gpSrv->hInputPipe);
//	}
//
//	MCHKHEAP;
//	return 1;
//}
