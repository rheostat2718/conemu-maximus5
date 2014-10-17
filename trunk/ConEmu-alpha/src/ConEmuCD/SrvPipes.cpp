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

#include "ConEmuC.h"
#include "Queue.h"

#define DEBUGSTRINPUTPIPE(s) //DEBUGSTR(s) // ConEmuC: Recieved key... / ConEmuC: Recieved input

// Forwards
BOOL WINAPI InputServerCommand(LPVOID pInst, MSG64* pCmd, MSG64* &ppReply, DWORD &pcbReplySize, DWORD &pcbMaxReplySize, LPARAM lParam);
BOOL WINAPI CmdServerCommand(LPVOID pInst, CESERVER_REQ* pIn, CESERVER_REQ* &ppReply, DWORD &pcbReplySize, DWORD &pcbMaxReplySize, LPARAM lParam);
void WINAPI CmdServerFree(CESERVER_REQ* pReply, LPARAM lParam);
BOOL WINAPI DataServerCommand(LPVOID pInst, CESERVER_REQ* pIn, CESERVER_REQ* &ppReply, DWORD &pcbReplySize, DWORD &pcbMaxReplySize, LPARAM lParam);
void WINAPI DataServerFree(CESERVER_REQ* pReply, LPARAM lParam);

// Helpers
bool InputServerStart()
{
	if (!gpSrv || !*gpSrv->szInputname)
	{
		_ASSERTE(gpSrv!=NULL && *gpSrv->szInputname);
		return false;
	}

	gpSrv->InputServer.SetOverlapped(true);
	gpSrv->InputServer.SetLoopCommands(true);
	gpSrv->InputServer.SetPriority(THREAD_PRIORITY_ABOVE_NORMAL);
	gpSrv->InputServer.SetInputOnly(true);
	//gpSrv->InputServer.SetDummyAnswerSize(sizeof(CESERVER_REQ_HDR));

	if (!gpSrv->InputServer.StartPipeServer(true, gpSrv->szInputname, NULL, LocalSecurity(), InputServerCommand))
		return false;

	return true;
}

bool CmdServerStart()
{
	if (!gpSrv || !*gpSrv->szPipename)
	{
		_ASSERTE(gpSrv!=NULL && *gpSrv->szPipename);
		return false;
	}

	gpSrv->CmdServer.SetMaxCount(3);
	gpSrv->CmdServer.SetOverlapped(true);
	gpSrv->CmdServer.SetLoopCommands(false);
	//gpSrv->CmdServer.SetPriority(THREAD_PRIORITY_ABOVE_NORMAL);
	gpSrv->CmdServer.SetDummyAnswerSize(sizeof(CESERVER_REQ_HDR));

	if (!gpSrv->CmdServer.StartPipeServer(true, gpSrv->szPipename, NULL, LocalSecurity(), CmdServerCommand, CmdServerFree))
		return false;

	return true;
}

bool DataServerStart()
{
	if (!gpSrv || !*gpSrv->szGetDataPipe)
	{
		_ASSERTE(gpSrv!=NULL && *gpSrv->szGetDataPipe);
		return false;
	}

	gpSrv->DataServer.SetOverlapped(true);
	gpSrv->DataServer.SetLoopCommands(true);
	gpSrv->DataServer.SetPriority(THREAD_PRIORITY_ABOVE_NORMAL);
	gpSrv->DataServer.SetDummyAnswerSize(sizeof(CESERVER_REQ_HDR));

	if (!gpSrv->DataServer.StartPipeServer(true, gpSrv->szGetDataPipe, NULL, LocalSecurity(), DataServerCommand, DataServerFree))
		return false;

	return true;
}



// Bodies
BOOL WINAPI InputServerCommand(LPVOID pInst, MSG64* pCmd, MSG64* &ppReply, DWORD &pcbReplySize, DWORD &pcbMaxReplySize, LPARAM lParam)
{
	if (!pCmd || !pCmd->nCount || (pCmd->cbSize < sizeof(*pCmd)))
	{
		_ASSERTE(pCmd && pCmd->nCount && (pCmd->cbSize < sizeof(*pCmd)));
	}
	else
	{
		_ASSERTE(pCmd->cbSize == (sizeof(*pCmd)+(pCmd->nCount-1)*sizeof(pCmd->msg[0])));

		#ifdef _DEBUG
		wchar_t* pszPasting = (wchar_t*)malloc((pCmd->nCount+1)*sizeof(wchar_t));
		if (pszPasting)
			*pszPasting = 0;
		wchar_t* pszDbg = pszPasting;
		#endif

		for (DWORD i = 0; i < pCmd->nCount; i++)
		{
			// При посылке массовых нажатий клавиш (вставка из буфера)
			// очередь может "не успевать"
			if (i && !(i & 31))
			{
				InputLogger::Log(InputLogger::Event::evt_InputQueueFlush);
				gpSrv->InputQueue.WriteInputQueue(NULL, TRUE);
				Sleep(10);
			}

			#ifdef _DEBUG
			switch (pCmd->msg[i].message)
			{
			case WM_KEYDOWN: case WM_SYSKEYDOWN: DEBUGSTRINPUTPIPE(L"ConEmuC: Recieved key down\n"); break;
			case WM_KEYUP: case WM_SYSKEYUP: DEBUGSTRINPUTPIPE(L"ConEmuC: Recieved key up\n"); break;
			default: DEBUGSTRINPUTPIPE(L"ConEmuC: Recieved input\n");
			}
			#endif

			INPUT_RECORD r;

			// Сбросим флаг
			InputLogger::Log(InputLogger::Event::evt_ResetEvent, gpSrv->InputQueue.nUsedLen);
			ResetEvent(gpSrv->hInputWasRead);

			// Некорректные события - отсеиваются,
			// некоторые события (CtrlC/CtrlBreak) не пишутся в буферном режиме
			InputLogger::Log(InputLogger::Event::evt_ProcessInputMessage, gpSrv->InputQueue.nUsedLen);
			if (ProcessInputMessage(pCmd->msg[i], r))
			{
				//SendConsoleEvent(&r, 1);
				InputLogger::Log(InputLogger::Event::evt_WriteInputQueue1, r, gpSrv->InputQueue.nUsedLen);
				BOOL bWritten = gpSrv->InputQueue.WriteInputQueue(&r, FALSE);
				// Облом с записью в наш внутренний буфер может быть только при его переполнении
				// Т.е. InputThread еще не успел считать данные из внутреннего буфера
				// и еще не перекинул считанное в консольный буфер.
				if (!bWritten)
				{
					// Передернуть на всякий случай очередь
					InputLogger::Log(InputLogger::Event::evt_InputQueueFlush);
					gpSrv->InputQueue.WriteInputQueue(NULL, TRUE);

					// Подождем чуть-чуть
					InputLogger::Log(InputLogger::Event::evt_WaitIntputReady, gpSrv->InputQueue.nUsedLen);
					DWORD nReadyWait = WaitForSingleObject(gpSrv->hInputWasRead, 250);
					if (nReadyWait == WAIT_OBJECT_0)
					{
						InputLogger::Log(InputLogger::Event::evt_WriteInputQueue2, gpSrv->InputQueue.nUsedLen);
						bWritten = gpSrv->InputQueue.WriteInputQueue(&r, FALSE);
					}

					if (!bWritten)
					{
						InputLogger::Log(InputLogger::Event::evt_Overflow, gpSrv->InputQueue.nUsedLen);
						_InterlockedIncrement(&InputLogger::g_overflow);
						OutputDebugString(L"\n!!!ConEmuC input buffer overflow!!!\n");
						_ASSERTE(FALSE && "Input buffer overflow");
					}
					WARNING("Если буфер переполнен - ждать? Хотя если будем ждать здесь - может повиснуть GUI на записи в pipe...");
				}

				#ifdef _DEBUG
				if (bWritten && pszDbg && (r.EventType == KEY_EVENT) && r.Event.KeyEvent.bKeyDown)
				{
					*(pszDbg++) = r.Event.KeyEvent.uChar.UnicodeChar;
					*pszDbg = 0;
				}
				#endif
			}

			MCHKHEAP;
		}

		InputLogger::Log(InputLogger::Event::evt_InputQueueFlush);
		gpSrv->InputQueue.WriteInputQueue(NULL, TRUE);

		#ifdef _DEBUG
		SafeFree(pszPasting);
		#endif
	}

	return FALSE; // Inbound only
}

//void WINAPI InputServerFree(MSG64* pReply, LPARAM lParam)
//{
//	SafeFree(pReply);
//}







BOOL WINAPI CmdServerCommand(LPVOID pInst, CESERVER_REQ* pIn, CESERVER_REQ* &ppReply, DWORD &pcbReplySize, DWORD &pcbMaxReplySize, LPARAM lParam)
{
	BOOL lbRc = FALSE;
	CESERVER_REQ *pOut = NULL;
	ExecuteFreeResult(ppReply);

	if (pIn->hdr.nVersion != CESERVER_REQ_VER)
	{
		_ASSERTE(pIn->hdr.nVersion == CESERVER_REQ_VER);
		// переоткрыть пайп!
		gpSrv->CmdServer.BreakConnection(pInst);
		return FALSE;
	}

	if (pIn->hdr.bAsync)
		gpSrv->CmdServer.BreakConnection(pInst);

	if (ProcessSrvCommand(*pIn, &pOut))
	{
		lbRc = TRUE;
		ppReply = pOut;
		pcbReplySize = pOut->hdr.cbSize;
	}

	return lbRc;

	//CESERVER_REQ in= {{0}}, *pIn=NULL, *pOut=NULL;
	//DWORD cbBytesRead, cbWritten, dwErr = 0;
	//BOOL fSuccess;
	//HANDLE hPipe;
	//// The thread's parameter is a handle to a pipe instance.
	//hPipe = (HANDLE) lpvParam;
	//MCHKHEAP;
	//// Read client requests from the pipe.
	//memset(&in, 0, sizeof(in));
	//fSuccess = ReadFile(
	//	hPipe,        // handle to pipe
	//	&in,          // buffer to receive data
	//	sizeof(in),   // size of buffer
	//	&cbBytesRead, // number of bytes read
	//	NULL);        // not overlapped I/O

	//if ((!fSuccess && ((dwErr = GetLastError()) != ERROR_MORE_DATA)) ||
	//	cbBytesRead < sizeof(CESERVER_REQ_HDR) || in.hdr.cbSize < sizeof(CESERVER_REQ_HDR))
	//{
	//	goto wrap;
	//}

	//if (in.hdr.cbSize > cbBytesRead)
	//{
	//	DWORD cbNextRead = 0;
	//	// Тут именно calloc, а не ExecuteNewCmd, т.к. данные пришли снаружи, а не заполняются здесь
	//	pIn = (CESERVER_REQ*)calloc(in.hdr.cbSize, 1);

	//	if (!pIn)
	//		goto wrap;

	//	memmove(pIn, &in, cbBytesRead); // стояло ошибочное присвоение
	//	fSuccess = ReadFile(
	//		hPipe,        // handle to pipe
	//		((LPBYTE)pIn)+cbBytesRead,  // buffer to receive data
	//		in.hdr.cbSize - cbBytesRead,   // size of buffer
	//		&cbNextRead, // number of bytes read
	//		NULL);        // not overlapped I/O

	//	if (fSuccess)
	//		cbBytesRead += cbNextRead;
	//}

	//	if (!ProcessSrvCommand(pIn ? *pIn : in, &pOut) || pOut==NULL)
	//	{
	//		// Если результата нет - все равно что-нибудь запишем, иначе TransactNamedPipe может виснуть?
	//		CESERVER_REQ_HDR Out;
	//		ExecutePrepareCmd(&Out, in.hdr.nCmd, sizeof(Out));
	//		fSuccess = WriteFile(
	//			hPipe,        // handle to pipe
	//			&Out,         // buffer to write from
	//			Out.cbSize,    // number of bytes to write
	//			&cbWritten,   // number of bytes written
	//			NULL);        // not overlapped I/O
	//	}
	//	else
	//	{
	//		MCHKHEAP;
	//		// Write the reply to the pipe.
	//		fSuccess = WriteFile(
	//			hPipe,        // handle to pipe
	//			pOut,         // buffer to write from
	//			pOut->hdr.cbSize,  // number of bytes to write
	//			&cbWritten,   // number of bytes written
	//			NULL);        // not overlapped I/O
	//
	//		// освободить память
	//		if ((LPVOID)pOut != (LPVOID)gpStoredOutput)  // Если это НЕ сохраненный вывод
	//			ExecuteFreeResult(pOut);
	//	}
	//
	//	if (pIn)    // не освобождалась, хотя, таких длинных команд наверное не было
	//	{
	//		free(pIn); pIn = NULL;
	//	}
	//
	//	MCHKHEAP;
	//	//if (!fSuccess || pOut->hdr.cbSize != cbWritten) break;
	//	// Flush the pipe to allow the client to read the pipe's contents
	//	// before disconnecting. Then disconnect the pipe, and close the
	//	// handle to this pipe instance.
	//wrap: // Flush и Disconnect делать всегда
	//	FlushFileBuffers(hPipe);
	//	DisconnectNamedPipe(hPipe);
	//	SafeCloseHandle(hPipe);
	//	return 1;
}

void WINAPI CmdServerFree(CESERVER_REQ* pReply, LPARAM lParam)
{
	ExecuteFreeResult(pReply);
}



BOOL WINAPI DataServerCommand(LPVOID pInst, CESERVER_REQ* pIn, CESERVER_REQ* &ppReply, DWORD &pcbReplySize, DWORD &pcbMaxReplySize, LPARAM lParam)
{
	BOOL lbRc = FALSE;
	//CESERVER_REQ *pOut = NULL;
	//ExecuteFreeResult(ppReply);

	if ((pIn->hdr.nVersion != CESERVER_REQ_VER)
		|| (pIn->hdr.nCmd != CECMD_CONSOLEDATA))
	{
		_ASSERTE(pIn->hdr.nVersion == CESERVER_REQ_VER);
		_ASSERTE(pIn->hdr.nCmd == CECMD_CONSOLEDATA);
		// переоткрыть пайп!
		gpSrv->DataServer.BreakConnection(pInst);
		return FALSE;
	}


	if (gpSrv->pConsole->bDataChanged == FALSE)
	{
		pcbReplySize = sizeof(gpSrv->pConsole->info);
		if (ExecuteNewCmd(ppReply, pcbMaxReplySize, pIn->hdr.nCmd, pcbReplySize))
		{
			// Отдаем только инфу, без текста/атрибутов
			memmove(ppReply->Data, (&gpSrv->pConsole->info.cmd)+1, pcbReplySize - sizeof(ppReply->hdr));
			_ASSERTE(sizeof(gpSrv->pConsole->info.cmd) == sizeof(ppReply->hdr));
			((CESERVER_REQ_CONINFO_INFO*)ppReply)->nDataShift = 0;
			((CESERVER_REQ_CONINFO_INFO*)ppReply)->nDataCount = 0;
			lbRc = TRUE;
		}
	}
	else //if (Command.nCmd == CECMD_CONSOLEDATA)
	{
		_ASSERTE(pIn->hdr.nCmd == CECMD_CONSOLEDATA);
		gpSrv->pConsole->bDataChanged = FALSE;
		DWORD ccCells = gpSrv->pConsole->info.crWindow.X * gpSrv->pConsole->info.crWindow.Y;

		// Такого быть не должно, ReadConsoleData корректирует возможный размер
		if (ccCells > (size_t)(gpSrv->pConsole->info.crMaxSize.X * gpSrv->pConsole->info.crMaxSize.Y))
		{
			_ASSERTE(ccCells <= (size_t)(gpSrv->pConsole->info.crMaxSize.X * gpSrv->pConsole->info.crMaxSize.Y));
			ccCells = (gpSrv->pConsole->info.crMaxSize.X * gpSrv->pConsole->info.crMaxSize.Y);
		}

		gpSrv->pConsole->info.nDataShift = (DWORD)(((LPBYTE)gpSrv->pConsole->data) - ((LPBYTE)&(gpSrv->pConsole->info)));
		gpSrv->pConsole->info.nDataCount = ccCells;
		pcbReplySize = sizeof(gpSrv->pConsole->info) + ccCells * sizeof(CHAR_INFO);
		if (ExecuteNewCmd(ppReply, pcbMaxReplySize, pIn->hdr.nCmd, pcbReplySize))
		{
			memmove(ppReply->Data, (&gpSrv->pConsole->info.cmd)+1, pcbReplySize - sizeof(ppReply->hdr));
			_ASSERTE(sizeof(gpSrv->pConsole->info.cmd) == sizeof(ppReply->hdr));
			lbRc = TRUE;
		}
		//fSuccess = WriteFile(gpSrv->hGetDataPipe, &(gpSrv->pConsole->info), cbWrite, &cbBytesWritten, NULL);
	}

	return lbRc;
}

void WINAPI DataServerFree(CESERVER_REQ* pReply, LPARAM lParam)
{
	ExecuteFreeResult(pReply);
}
