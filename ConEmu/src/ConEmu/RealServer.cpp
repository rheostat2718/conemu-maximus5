
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

#define AssertCantActivate(x) //MBoxAssert(x)

#define SHOWDEBUGSTR
//#define ALLOWUSEFARSYNCHRO

#include "Header.h"
#include <Tlhelp32.h>
#include "../common/ConEmuCheck.h"
#include "../common/RgnDetect.h"
#include "../common/Execute.h"
#include "../common/PipeServer.h"
#include "RealServer.h"
#include "RealConsole.h"
#include "RealBuffer.h"
#include "VirtualConsole.h"
#include "TabBar.h"
#include "ConEmu.h"
#include "ConEmuApp.h"
#include "VConChild.h"
#include "ConEmuPipe.h"
#include "Macro.h"

#define DEBUGSTRCMD(s) DEBUGSTR(s)
#define DEBUGSTRLANG(s) //DEBUGSTR(s)// ; Sleep(2000)


CRealServer::CRealServer()
{
	mp_RCon = NULL;
	mh_GuiAttached = NULL;
	mp_RConServer = NULL;
}

CRealServer::~CRealServer()
{
	Stop(true);
}

void CRealServer::Init(CRealConsole* apRCon)
{
	mp_RCon = apRCon;
	mp_RConServer = (PipeServer<CESERVER_REQ>*)calloc(3,sizeof(*mp_RConServer));
	mp_RConServer->SetMaxCount(3);
	//mh_ServerSemaphore = NULL;
	//memset(mh_RConServerThreads, 0, sizeof(mh_RConServerThreads));
	//mh_ActiveRConServerThread = NULL;
	//memset(mn_RConServerThreadsId, 0, sizeof(mn_RConServerThreadsId));
}

bool CRealServer::Start()
{
	DWORD nConWndID = (DWORD)(((DWORD_PTR)mp_RCon->hConWnd) & 0xFFFFFFFF);
	_wsprintf(mp_RCon->ms_VConServer_Pipe, SKIPLEN(countof(mp_RCon->ms_VConServer_Pipe)) CEGUIPIPENAME, L".", nConWndID);

	if (!mh_GuiAttached)
	{
		wchar_t szEvent[64];
		
		_wsprintf(szEvent, SKIPLEN(countof(szEvent)) CEGUIRCONSTARTED, nConWndID);
		//// ������ ����� ������� � ������� ��� �� �������
		//mh_GuiAttached = OpenEvent(EVENT_MODIFY_STATE, FALSE, mp_RCon->ms_VConServer_Pipe);
		//// �����, ����� ������������ run as administrator - event ������� �� ����������?
		//if (!mh_GuiAttached) {
		mh_GuiAttached = CreateEvent(gpLocalSecurity, TRUE, FALSE, szEvent);
		_ASSERTE(mh_GuiAttached!=NULL);
		//}
	}

	mp_RConServer->SetOverlapped(true);
	mp_RConServer->SetLoopCommands(false);
	mp_RConServer->SetDummyAnswerSize(sizeof(CESERVER_REQ_HDR));
	
	// ConEmuC ������� ������� ���� ����� �������� �� CECMD_SRVSTARTSTOP
	if (!mp_RConServer->StartPipeServer(mp_RCon->ms_VConServer_Pipe, (LPARAM)this, LocalSecurity(),
			ServerCommand, ServerCommandFree, NULL, NULL, ServerThreadReady))
	{
		MBoxAssert("mp_RConServer->StartPipeServer"==0);
		return false;
	}

	//if (!mh_ServerSemaphore)
	//	mh_ServerSemaphore = CreateSemaphore(NULL, 1, 1, NULL);
	//for (int i=0; i<MAX_SERVER_THREADS; i++)
	//{
	//	if (mh_RConServerThreads[i])
	//		continue;
	//	mn_RConServerThreadsId[i] = 0;
	//	mh_RConServerThreads[i] = CreateThread(NULL, 0, RConServerThread, (LPVOID)this, 0, &mn_RConServerThreadsId[i]);
	//	_ASSERTE(mh_RConServerThreads[i]!=NULL);
	//}

	// ����� ConEmuC ����, ��� �� ������
	//     if (mh_GuiAttached) {
	//     	SetEvent(mh_GuiAttached);
	//Sleep(10);
	//     	SafeCloseHandle(mh_GuiAttached);
	//		}
	return true;
}

void CRealServer::Stop(bool abDeinitialize/*=false*/)
{
	//SafeCloseHandle(mh_ServerSemaphore);

	if (mp_RConServer)
	{
		mp_RConServer->StopPipeServer();
		
		if (abDeinitialize)
		{
			SafeFree(mp_RConServer);
		}
	}
	
	SafeCloseHandle(mh_GuiAttached);
}

//DWORD CRealServer::RConServerThread(LPVOID lpvParam)
//{
//	CRealConsole *pRCon = (CRealConsole*)lpvParam;
//	CVirtualConsole *pVCon = pRCon->mp_VCon;
//	BOOL fConnected = FALSE;
//	DWORD dwErr = 0;
//	HANDLE hPipe = NULL;
//	HANDLE hWait[2] = {NULL,NULL};
//	DWORD dwTID = GetCurrentThreadId();
//	MCHKHEAP
//	_ASSERTE(pVCon!=NULL);
//	_ASSERTE(pRCon->hConWnd!=NULL);
//	_ASSERTE(pRCon->ms_VConServer_Pipe[0]!=0);
//	_ASSERTE(pRCon->mh_ServerSemaphore!=NULL);
//	_ASSERTE(pRCon->mh_TermEvent!=NULL);
//	//swprintf_c(pRCon->ms_VConServer_Pipe, CEGUIPIPENAME, L".", (DWORD)pRCon->hConWnd); //��� mp_RCon->mn_ConEmuC_PID
//	// The main loop creates an instance of the named pipe and
//	// then waits for a client to connect to it. When the client
//	// connects, a thread is created to handle communications
//	// with that client, and the loop is repeated.
//	hWait[0] = pRCon->mh_TermEvent;
//	hWait[1] = pRCon->mh_ServerSemaphore;
//	MCHKHEAP
//
//	// ���� �� ����������� ���������� �������
//	do
//	{
//		while(!fConnected)
//		{
//			_ASSERTE(hPipe == NULL);
//			// !!! ���������� �������� �������� ����� Create NamedPipe ������, �.�. � ���� ������
//			//     ���� ������� � ������ �� ����� ����������� � �������
//			// ��������� ���������� ��������, ��� �������� �������
//			dwErr = WaitForMultipleObjects(2, hWait, FALSE, INFINITE);
//
//			if (dwErr == WAIT_OBJECT_0)
//			{
//				return 0; // ������� �����������
//			}
//
//			MCHKHEAP
//
//			for(int i=0; i<MAX_SERVER_THREADS; i++)
//			{
//				if (pRCon->mn_RConServerThreadsId[i] == dwTID)
//				{
//					pRCon->mh_ActiveRConServerThread = pRCon->mh_RConServerThreads[i]; break;
//				}
//			}
//
//			_ASSERTE(gpLocalSecurity);
//			hPipe = Create NamedPipe(
//			            pRCon->ms_VConServer_Pipe, // pipe name
//			            PIPE_ACCESS_DUPLEX,       // read/write access
//			            PIPE_TYPE_MESSAGE |       // message type pipe
//			            PIPE_READMODE_MESSAGE |   // message-read mode
//			            PIPE_WAIT,                // blocking mode
//			            PIPE_UNLIMITED_INSTANCES, // max. instances
//			            PIPEBUFSIZE,              // output buffer size
//			            PIPEBUFSIZE,              // input buffer size
//			            0,                        // client time-out
//			            gpLocalSecurity);          // default security attribute
//			MCHKHEAP
//
//			if (hPipe == INVALID_HANDLE_VALUE)
//			{
//				dwErr = GetLastError();
//				_ASSERTE(hPipe != INVALID_HANDLE_VALUE);
//				//DisplayLastError(L"Create NamedPipe failed");
//				hPipe = NULL;
//				// ��������� ���� ��� ������ ���� ������� ��������� ����
//				ReleaseSemaphore(hWait[1], 1, NULL);
//				//Sleep(50);
//				continue;
//			}
//
//			MCHKHEAP
//
//			// ����� ConEmuC ����, ��� ��������� ���� �����
//			if (pRCon->mh_GuiAttached)
//			{
//				SetEvent(pRCon->mh_GuiAttached);
//				SafeCloseHandle(pRCon->mh_GuiAttached);
//			}
//
//			// Wait for the client to connect; if it succeeds,
//			// the function returns a nonzero value. If the function
//			// returns zero, GetLastError returns ERROR_PIPE_CONNECTED.
//			fConnected = ConnectNamedPipe(hPipe, NULL) ? TRUE : ((dwErr = GetLastError()) == ERROR_PIPE_CONNECTED);
//			MCHKHEAP
//			// ����� ��������� ������ ���� ������� �����
//			ReleaseSemaphore(hWait[1], 1, NULL);
//
//			// ������� �����������!
//			if (WaitForSingleObject(hWait[0], 0) == WAIT_OBJECT_0)
//			{
//				//FlushFileBuffers(hPipe); -- ��� �� �����, �� ������ �� ����������
//				//DisconnectNamedPipe(hPipe);
//				SafeCloseHandle(hPipe);
//				return 0;
//			}
//
//			MCHKHEAP
//
//			if (fConnected)
//				break;
//			else
//				SafeCloseHandle(hPipe);
//		}
//
//		if (fConnected)
//		{
//			// ����� �������, ����� �� ������
//			fConnected = FALSE;
//			//// ��������� ������ ���� ������� ����� //2009-08-28 ���������� ����� ����� ConnectNamedPipe
//			//ReleaseSemaphore(hWait[1], 1, NULL);
//			MCHKHEAP
//
//			if (gpConEmu->isValid(pVCon))
//			{
//				_ASSERTE(pVCon==pRCon->mp_VCon);
//				pRCon->ServerThreadCommand(hPipe);    // ��� ������������� - ���������� � ���� ��������� ����
//			}
//			else
//			{
//				_ASSERTE(FALSE);
//				// ��������. VirtualConsole �������, � ���� ��� �� �����������! ���� ������ ����...
//				SafeCloseHandle(hPipe);
//				return 1;
//			}
//		}
//
//		MCHKHEAP
//		FlushFileBuffers(hPipe);
//		//DisconnectNamedPipe(hPipe);
//		SafeCloseHandle(hPipe);
//	} // ������� � �������� ������ instance �����
//	while(WaitForSingleObject(pRCon->mh_TermEvent, 0) != WAIT_OBJECT_0);
//
//	return 0;
//}

CESERVER_REQ* CRealServer::cmdStartStop(LPVOID pInst, CESERVER_REQ* pIn, UINT nDataSize)
{
	CESERVER_REQ* pOut = ExecuteNewCmd(pIn->hdr.nCmd, sizeof(CESERVER_REQ_HDR) + sizeof(CESERVER_REQ_STARTSTOPRET));
	
	//
	DWORD nStarted = pIn->StartStop.nStarted;
	HWND  hWnd     = (HWND)pIn->StartStop.hWnd;
#ifdef _DEBUG
	wchar_t szDbg[128];

	switch(nStarted)
	{
		case sst_ServerStart:
			_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"GUI received CECMD_CMDSTARTSTOP(ServerStart,%i,PID=%u)\n", pIn->hdr.nCreateTick, pIn->StartStop.dwPID);
			break;
		case sst_ServerStop:
			_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"GUI received CECMD_CMDSTARTSTOP(ServerStop,%i,PID=%u)\n", pIn->hdr.nCreateTick, pIn->StartStop.dwPID);
			break;
		case sst_ComspecStart:
			_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"GUI received CECMD_CMDSTARTSTOP(ComspecStart,%i,PID=%u)\n", pIn->hdr.nCreateTick, pIn->StartStop.dwPID);
			break;
		case sst_ComspecStop:
			_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"GUI received CECMD_CMDSTARTSTOP(ComspecStop,%i,PID=%u)\n", pIn->hdr.nCreateTick, pIn->StartStop.dwPID);
			break;
		case sst_AppStart:
			_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"GUI received CECMD_CMDSTARTSTOP(AppStart,%i,PID=%u)\n", pIn->hdr.nCreateTick, pIn->StartStop.dwPID);
			break;
		case sst_AppStop:
			_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"GUI received CECMD_CMDSTARTSTOP(AppStop,%i,PID=%u)\n", pIn->hdr.nCreateTick, pIn->StartStop.dwPID);
			break;
		case sst_App16Start:
			_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"GUI received CECMD_CMDSTARTSTOP(App16Start,%i,PID=%u)\n", pIn->hdr.nCreateTick, pIn->StartStop.dwPID);
			break;
		case sst_App16Stop:
			_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"GUI received CECMD_CMDSTARTSTOP(App16Stop,%i,PID=%u)\n", pIn->hdr.nCreateTick, pIn->StartStop.dwPID);
			break;
	}

	DEBUGSTRCMD(szDbg);
#endif
	_ASSERTE(pIn->StartStop.dwPID!=0);
	DWORD nPID     = pIn->StartStop.dwPID;
	DWORD nSubSystem = pIn->StartStop.nSubSystem;
	BOOL bRunViaCmdExe = pIn->StartStop.bRootIsCmdExe;
	BOOL bUserIsAdmin = pIn->StartStop.bUserIsAdmin;
	BOOL lbWasBuffer = pIn->StartStop.bWasBufferHeight;
	//DWORD nInputTID = pIn->StartStop.dwInputTID;
	_ASSERTE(sizeof(CESERVER_REQ_STARTSTOPRET) <= sizeof(CESERVER_REQ_STARTSTOP));
	//pIn->hdr.cbSize = sizeof(CESERVER_REQ_HDR) + sizeof(CESERVER_REQ_STARTSTOPRET);

	// ���� ������� ������������ (����� ��������� �������) - �������� ��� � mp_RCon->m_TerminatedPIDs
	TODO("���������, ����� �� ��������� �������������� sst_App16Stop? �� ����, ��� ����� ����� ���� ������ sst_ComspecStop ������?");
	if (nStarted == sst_ComspecStop || nStarted == sst_AppStop /*|| nStarted == sst_App16Stop*/)
	{
		bool lbPushed = false;
		// ����� �� ��� �������� � mp_RCon->m_TerminatedPIDs (���� �� ������ �� ����)
		for (size_t i = 0; i < countof(mp_RCon->m_TerminatedPIDs); i++)
		{
			if (mp_RCon->m_TerminatedPIDs[i] == nPID)
			{
				lbPushed = true;
				break;
			}
		}
		// ����� ������ ������ � ��������
		for (UINT k = 0; !lbPushed && k <= 1; k++)
		{
			UINT iStart = !k ? mp_RCon->mn_TerminatedIdx : 0;
			UINT iEnd = !k ? countof(mp_RCon->m_TerminatedPIDs) : min(mp_RCon->mn_TerminatedIdx,countof(mp_RCon->m_TerminatedPIDs));
			// ����������� �����
			for (UINT i = iStart; i < iEnd; i++)
			{
				if (!mp_RCon->m_TerminatedPIDs[i])
				{
					mp_RCon->m_TerminatedPIDs[i] = nPID;
					mp_RCon->mn_TerminatedIdx = ((i + 1) < countof(mp_RCon->m_TerminatedPIDs)) ? (i + 1) : 0;
					lbPushed = true;
					break;
				}
			}
		}
	}


	if (nStarted == sst_ServerStart || nStarted == sst_ComspecStart)
	{
		if (nStarted == sst_ServerStart)
		{
			mp_RCon->SetConStatus(L"Console server started...");

			// �������� ������ ���� �������� �����
			_ASSERTE(mp_RCon->mp_ABuf==mp_RCon->mp_RBuf);
			mp_RCon->mp_RBuf->InitMaxSize(pIn->StartStop.crMaxSize);
		}

		// ����� �������� ���������
		pOut->StartStopRet.bWasBufferHeight = mp_RCon->isBufferHeight(); // ����� comspec ����, ��� ����� ����� ����� ���������
		//DWORD nParentPID = 0;
		//if (nStarted == 2)
		//{
		//	ConProcess* pPrc = NULL;
		//	int i, nProcCount = GetProcesses(&pPrc);
		//	if (pPrc != NULL)
		//	{
		//		for (i = 0; i < nProcCount; i++) {
		//			if (pPrc[i].ProcessID == nPID) {
		//				nParentPID = pPrc[i].ParentPID; break;
		//			}
		//		}
		//		if (nParentPID == 0) {
		//			_ASSERTE(nParentPID != 0);
		//		} else {
		//			BOOL lbFar = FALSE;
		//			for (i = 0; i < nProcCount; i++) {
		//				if (pPrc[i].ProcessID == nParentPID) {
		//					lbFar = pPrc[i].IsFar; break;
		//				}
		//			}
		//			if (!lbFar) {
		//				_ASSERTE(lbFar);
		//				nParentPID = 0;
		//			}
		//		}
		//		free(pPrc);
		//	}
		//}
		//pOut->StartStopRet.bWasBufferHeight = FALSE;// (nStarted == 2) && (nParentPID == 0); // comspec ������ ��������� � ����������
		pOut->StartStopRet.hWnd = ghWnd;
		pOut->StartStopRet.hWndDC = mp_RCon->mp_VCon->GetView();
		pOut->StartStopRet.dwPID = GetCurrentProcessId();
		pOut->StartStopRet.dwSrvPID = mp_RCon->mn_ConEmuC_PID;
		pOut->StartStopRet.bNeedLangChange = FALSE;

		if (nStarted == sst_ServerStart)
		{
			//_ASSERTE(nInputTID);
			//_ASSERTE(mn_ConEmuC_Input_TID==0 || mn_ConEmuC_Input_TID==nInputTID);
			//mn_ConEmuC_Input_TID = nInputTID;
			//
			if (!mp_RCon->m_Args.bRunAsAdministrator && bUserIsAdmin)
				mp_RCon->m_Args.bRunAsAdministrator = TRUE;

			if (mp_RCon->mn_InRecreate>=1)
				mp_RCon->mn_InRecreate = 0; // �������� ������� ������� ������������

			// ���� ���� Layout �� ��� �������
			if ((gpSet->isMonitorConsoleLang & 2) == 2)
			{
				// ������� - ����, ���� ��� �� �������������
				//	SwitchKeyboardLayout(INPUTLANGCHANGE_SYSCHARSET,gpConEmu->GetActiveKeyboardLayout());
				pOut->StartStopRet.bNeedLangChange = TRUE;
				TODO("��������� �� x64, �� ����� �� ������� � 0xFFFFFFFFFFFFFFFFFFFFF");
				pOut->StartStopRet.NewConsoleLang = gpConEmu->GetActiveKeyboardLayout();
			}

			// ������ �� �������������� ����� ���������� ���� �������
			mp_RCon->SetHwnd(hWnd, TRUE);
			// ������� ��������, ��������� KeyboardLayout, � �.�.
			mp_RCon->OnServerStarted();
		}

		AllowSetForegroundWindow(nPID);
		
		COORD cr16bit = mp_RCon->mp_RBuf->GetDefaultNtvdmHeight();

		/*
		#define IMAGE_SUBSYSTEM_DOS_EXECUTABLE  255
		#define IMAGE_SUBSYSTEM_BATCH_FILE  254
		*/
		
		// ComSpec started
		if (nStarted == sst_ComspecStart)
		{
			// ��������������� � TRUE ���� ����� �������� 16������ ����������
			if (nSubSystem == IMAGE_SUBSYSTEM_DOS_EXECUTABLE/*255*/)
			{
				DEBUGSTRCMD(L"16 bit application STARTED, aquired from CECMD_CMDSTARTSTOP\n");

				//if (!(mp_RCon->mn_ProgramStatus & CES_NTVDM))
				//	mp_RCon->mn_ProgramStatus |= CES_NTVDM; -- � mp_RCon->OnDosAppStartStop

				mp_RCon->mn_Comspec4Ntvdm = nPID;
				mp_RCon->OnDosAppStartStop(sst_App16Start, nPID);
				//mp_RCon->mb_IgnoreCmdStop = TRUE; -- ���, � mp_RCon->OnDosAppStartStop

				mp_RCon->mp_RBuf->SetConsoleSize(cr16bit.X, cr16bit.Y, 0, CECMD_CMDSTARTED);
				pOut->StartStopRet.nBufferHeight = 0;
				pOut->StartStopRet.nWidth = cr16bit.X;
				pOut->StartStopRet.nHeight = cr16bit.Y;
			}
			else
			{
				BOOL bAllowBufferHeight = (gpSet->AutoBufferHeight || mp_RCon->isBufferHeight());
				if (pIn->StartStop.bForceBufferHeight)
					bAllowBufferHeight = (pIn->StartStop.nForceBufferHeight != 0);
				
				// �� ���� ��� ����� ��������
				mp_RCon->mb_IgnoreCmdStop = FALSE;
				// ���� ������������ ������ ��������� � ������ ������� ����������
				if (pIn->StartStop.bForceBufferHeight)
				{
					pOut->StartStopRet.nBufferHeight = pIn->StartStop.nForceBufferHeight;
				}
				else
				{
					// � ComSpec �������� ������ ��, ��� ������ � gpSet
					pOut->StartStopRet.nBufferHeight = bAllowBufferHeight ? gpSet->DefaultBufferHeight : 0;
				}
				// 111125 ���� "con.m_sbi.dwSize.X" � "con.m_sbi.dwSize.X"
				pOut->StartStopRet.nWidth = mp_RCon->mp_RBuf->GetBufferWidth()/*con.m_sbi.dwSize.X*/;
				pOut->StartStopRet.nHeight = mp_RCon->mp_RBuf->GetBufferHeight()/*con.m_sbi.dwSize.Y*/;

				if ((pOut->StartStopRet.nBufferHeight == 0) != (mp_RCon->isBufferHeight() == FALSE))
				{
					WARNING("��� �������� ����� �� ������������� ����� ������� ����� ������� �� ������� ConEmuC");
					//con.m_sbi.dwSize.Y = gpSet->DefaultBufferHeight; -- �� ����� ������ �����, � �� SetConsoleSize ������ skip
					mp_RCon->mp_RBuf->BuferModeChangeLock();
					mp_RCon->mp_RBuf->SetBufferHeightMode((pOut->StartStopRet.nBufferHeight != 0), TRUE); // ����� ������, ����� ������� ����������� ������������
					mp_RCon->mp_RBuf->SetConsoleSize(mp_RCon->mp_RBuf->GetTextWidth()/*con.m_sbi.dwSize.X*/, mp_RCon->mp_RBuf->TextHeight(), pOut->StartStopRet.nBufferHeight, CECMD_CMDSTARTED);
					mp_RCon->mp_RBuf->BuferModeChangeUnlock();
				}
			}
		}
		else if (nStarted == sst_ServerStart)
		{
			BOOL b = mp_RCon->mp_RBuf->BuferModeChangeLock();
		
			// Server
			if (nSubSystem == IMAGE_SUBSYSTEM_DOS_EXECUTABLE/*255*/)
			{
				pOut->StartStopRet.nBufferHeight = 0;
				pOut->StartStopRet.nWidth = cr16bit.X;
				pOut->StartStopRet.nHeight = cr16bit.Y;
			}
			else
			{
				pOut->StartStopRet.nWidth = mp_RCon->mp_RBuf->GetBufferWidth()/*con.m_sbi.dwSize.X*/;

				//0x101 - ������ ���������
				if (nSubSystem != 0x100   // 0x100 - ����� �� ���-�������
				        && (mp_RCon->mp_RBuf->isScroll()
				            || (mp_RCon->mn_DefaultBufferHeight && bRunViaCmdExe)))
				{
					_ASSERTE(mp_RCon->m_Args.bDetached || mp_RCon->mn_DefaultBufferHeight == mp_RCon->mp_RBuf->GetBufferHeight()/*con.m_sbi.dwSize.Y*/ || mp_RCon->mp_RBuf->GetBufferHeight()/*con.m_sbi.dwSize.Y*/ == mp_RCon->TextHeight());
					pOut->StartStopRet.nBufferHeight = max(mp_RCon->mp_RBuf->GetBufferHeight()/*con.m_sbi.dwSize.Y*/,mp_RCon->mn_DefaultBufferHeight);
					_ASSERTE(mp_RCon->mp_RBuf->TextHeight()/*con.nTextHeight*/ > 5);
					pOut->StartStopRet.nHeight = mp_RCon->mp_RBuf->TextHeight()/*con.nTextHeight*/;
					//111126 - �����. ���� ����� �����������
					//con.m_sbi.dwSize.Y = pOut->StartStopRet.nBufferHeight; // ����� ��������, ����� ����� ����� ���������� ���������������
				}
				else
				{
					_ASSERTE(!mp_RCon->mp_RBuf->isScroll());
					pOut->StartStopRet.nBufferHeight = 0;
					pOut->StartStopRet.nHeight = mp_RCon->mp_RBuf->TextHeight()/*con.m_sbi.dwSize.Y*/;
				}
				
			}

			mp_RCon->mp_RBuf->SetBufferHeightMode((pOut->StartStopRet.nBufferHeight != 0), TRUE);
			mp_RCon->mp_RBuf->SetChange2Size(pOut->StartStopRet.nWidth, pOut->StartStopRet.nHeight);
			
			if (b) mp_RCon->mp_RBuf->BuferModeChangeUnlock();
		}

		// 23.06.2009 Maks - ������ ����. ������ �������� � ApplyConsoleInfo
		//Process Add(nPID);

	} // (nStarted == sst_ServerStart || nStarted == sst_ComspecStart)
	else if (nStarted == sst_ServerStop || nStarted == sst_ComspecStop)
	{
		// ServerStop ����� �� �������� - ���������� CECMD_SRVSTARTSTOP � ConEmuWnd
		_ASSERTE(nStarted != sst_ServerStop);

		// 23.06.2009 Maks - ������ ����. ������ �������� � ApplyConsoleInfo
		//Process Delete(nPID);

		// ComSpec stopped
		if (nStarted == sst_ComspecStop)
		{
			BOOL lbNeedResizeWnd = FALSE;
			BOOL lbNeedResizeGui = FALSE;
			COORD crNewSize = {mp_RCon->TextWidth(),mp_RCon->TextHeight()};
			int nNewWidth=0, nNewHeight=0;

			if ((mp_RCon->mn_ProgramStatus & CES_NTVDM) == 0
			        && !(gpConEmu->mb_isFullScreen || gpConEmu->isZoomed()))
			{
				pOut->StartStopRet.bWasBufferHeight = FALSE;

				// � ��������� ������� (comspec ��� �������?) GetConsoleScreenBufferInfo ������������
				if (pOut->StartStop.sbi.dwSize.X && pOut->StartStop.sbi.dwSize.Y)
				{
					DWORD nScroll = 0;
					
					// ���������� �������� ������� �������, ������������ ��� �������� SBI
					// 111125 - bBufferHeight ������� �� nScroll (������� ��������� � ������� �������������� ���������)
					if (CRealBuffer::GetConWindowSize(pOut->StartStop.sbi, &nNewWidth, &nNewHeight, &nScroll))
					{
						lbNeedResizeGui = (crNewSize.X != nNewWidth || crNewSize.Y != nNewHeight);

						WARNING("ConResize: �����������, ��� ��� ����� ��������� GUI");
						if (nScroll || crNewSize.X != nNewWidth || crNewSize.Y != nNewHeight)
						{
							// ��� � ������� � ���, ����� �� ���������� ���������� ������ ������ ����,
							// ��� ��� ����� ������ ������ ����, ����� ������ ���� ConEmu
							// ����� ������ ����� (�� � �� �����������), � ��� ������ ������� �������...
							// ����, ����� ����, ��������, ������� "mode con lines=25 cols=80"
							_ASSERTE(crNewSize.X == nNewWidth && crNewSize.Y == nNewHeight);
							
							//gpConEmu->SyncWindowToConsole(); - ��� ������������ ������. �� ������ ��� �� ������� ����, �� ������ - ������ pVCon ����� ���� ��� �� �������
							lbNeedResizeWnd = TRUE;
							
							crNewSize.X = nNewWidth;
							crNewSize.Y = nNewHeight;
							
							//pOut->StartStopRet.bWasBufferHeight = TRUE; -- 111124 ������ ��������� pOut->StartStopRet.bWasBufferHeight=TRUE;
							_ASSERTE(nScroll!=0);
							pOut->StartStopRet.bWasBufferHeight = (nScroll!=0);
						}
					}
				}
			}

			if (mp_RCon->mb_IgnoreCmdStop || (mp_RCon->mn_ProgramStatus & CES_NTVDM) == CES_NTVDM)
			{
				// ����� ������������ ������ � WinXP � ����
				// ���� �������� 16������ ����������, ������ ����������� ������ ������� ������ ����� 80x25
				// ��� �� ������������� ��������� ������� ��� ������ �� 16���. ������� ����� ������������
				// ��� ������ ����. ��� ������� OnWinEvent.
				//SetBufferHeightMode(FALSE, TRUE);
				//PRAGMA_ERROR("���� �� ������� CECMD_CMDFINISHED �� ��������� WinEvents");
				mp_RCon->mb_IgnoreCmdStop = FALSE; // �������� ����� �������, ��� ������ ������ �� �����
				DEBUGSTRCMD(L"16 bit application TERMINATED (aquired from CECMD_CMDFINISHED)\n");

				//mp_RCon->mn_ProgramStatus &= ~CES_NTVDM; -- ������� ����� ������������� ������� �������, ����� �� ������
				if (lbWasBuffer)
				{
					mp_RCon->mp_RBuf->SetBufferHeightMode(TRUE, TRUE); // ����� ���������, ����� ������� ����������� ������������
				}

				mp_RCon->SyncConsole2Window(TRUE); // ����� ������ �� 16bit ������ ������ �� ����������� ������� �� ������� GUI

				if (mp_RCon->mn_Comspec4Ntvdm)
				{
					#ifdef _DEBUG
					if (mp_RCon->mn_Comspec4Ntvdm != nPID)
					{
						_ASSERTE(mp_RCon->mn_Comspec4Ntvdm == nPID);
					}
					#endif
					if ((nSubSystem == IMAGE_SUBSYSTEM_DOS_EXECUTABLE) || (mp_RCon->mn_Comspec4Ntvdm == nPID))
						mp_RCon->mn_Comspec4Ntvdm = 0;
				}

				// mp_RCon->mn_ProgramStatus &= ~CES_NTVDM;
				if ((nSubSystem == IMAGE_SUBSYSTEM_DOS_EXECUTABLE)
					|| ((mp_RCon->mn_ProgramStatus & CES_NTVDM) == CES_NTVDM))
				{
					_ASSERTE(nSubSystem == IMAGE_SUBSYSTEM_DOS_EXECUTABLE)
					mp_RCon->OnDosAppStartStop(sst_App16Stop, nPID);
				}

				lbNeedResizeWnd = FALSE;
				crNewSize.X = mp_RCon->TextWidth();
				crNewSize.Y = mp_RCon->TextHeight();
			} //else {

			// ������������ ������ ����� ��������� ConEmuC
			mp_RCon->mp_RBuf->BuferModeChangeLock();
			
			//111126 - �����, ���� ������� SetConsoleSize
			//con.m_sbi.dwSize.Y = crNewSize.Y;

			if (!lbWasBuffer)
			{
				mp_RCon->mp_RBuf->SetBufferHeightMode(FALSE, TRUE); // ����� ���������, ����� ������� ����������� ������������
			}

			#ifdef _DEBUG
			_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"Returns normal window size begin at %i\n", GetTickCount());
			DEBUGSTRCMD(szDbg);
			#endif
			
			// �����������. ����� ������ �� ������, ��� ������� ���������
			mp_RCon->mp_RBuf->SetConsoleSize(crNewSize.X, crNewSize.Y, 0, CECMD_CMDFINISHED);
			
			#ifdef _DEBUG
			_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"Finished returns normal window size begin at %i\n", GetTickCount());
			DEBUGSTRCMD(szDbg);
			#endif
			
			#ifdef _DEBUG
			#ifdef WIN64
			//				PRAGMA_ERROR("���� ����������, ��� ����� ����� �� Win7 x64 �������� ������ ����� � �������� ������� � ��������� ��� ������������ ������������� �������!");
			#endif
			#endif
			// ����� nChange2TextWidth, nChange2TextHeight ����� ������������?

			if (lbNeedResizeGui)
			{
				RECT rcCon = MakeRect(nNewWidth, nNewHeight);
				RECT rcNew = gpConEmu->CalcRect(CER_MAIN, rcCon, CER_CONSOLE);
				RECT rcWnd; GetWindowRect(ghWnd, &rcWnd);

				if (gpSet->isDesktopMode)
				{
					MapWindowPoints(NULL, gpConEmu->mh_ShellWindow, (LPPOINT)&rcWnd, 2);
				}

				MOVEWINDOW(ghWnd, rcWnd.left, rcWnd.top, rcNew.right, rcNew.bottom, 1);
			}

			mp_RCon->mp_RBuf->BuferModeChangeUnlock();
			//}
		}
		else
		{
			// ���� �� �������� �� ������!
			_ASSERTE(FALSE);
		}
	} // (nStarted == sst_ServerStop || nStarted == sst_ComspecStop)
	else if (nStarted == sst_App16Start || nStarted == sst_App16Stop)
	{
		mp_RCon->OnDosAppStartStop((enum StartStopType)nStarted, nPID);
	}
	
	// ������� ��������� � ��������

	//pOut = ExecuteNewCmd(pIn->hdr.nCmd, pIn->hdr.cbSize);
	//if (pIn->hdr.cbSize > sizeof(CESERVER_REQ_HDR))
	//	memmove(pOut->Data, pIn->Data, pIn->hdr.cbSize - (int)sizeof(CESERVER_REQ_HDR));
		
	return pOut;
}

//CESERVER_REQ* CRealServer::cmdGetGuiHwnd(LPVOID pInst, CESERVER_REQ* pIn, UINT nDataSize)
//{
//	CESERVER_REQ* pOut = NULL;
//	
//	DEBUGSTRCMD(L"GUI recieved CECMD_GETGUIHWND\n");
//	pOut = ExecuteNewCmd(pIn->hdr.nCmd, sizeof(CESERVER_REQ_HDR) + 2*sizeof(DWORD));
//	pOut->dwData[0] = (DWORD)ghWnd; //-V205
//	pOut->dwData[1] = (DWORD)mp_VCon->GetView(); //-V205
//	return pOut;
//}

CESERVER_REQ* CRealServer::cmdTabsChanged(LPVOID pInst, CESERVER_REQ* pIn, UINT nDataSize)
{
	CESERVER_REQ* pOut = NULL;
	
	DEBUGSTRCMD(L"GUI recieved CECMD_TABSCHANGED\n");
	
	BOOL fSuccess = FALSE;
	DWORD cbWritten = 0;

	if (nDataSize == 0)
	{
		// ��� �����������
		if (pIn->hdr.nSrcPID == mp_RCon->mn_FarPID)
		{
			mp_RCon->mn_ProgramStatus &= ~CES_FARACTIVE;

			for (UINT i = 0; i < mp_RCon->mn_FarPlugPIDsCount; i++)  // �������� �� ������ ��������
			{
				if (mp_RCon->m_FarPlugPIDs[i] == mp_RCon->mn_FarPID)
					mp_RCon->m_FarPlugPIDs[i] = 0;
			}

			mp_RCon->mn_FarPID_PluginDetected = mp_RCon->mn_FarPID = 0;
			mp_RCon->CloseFarMapData();

			if (mp_RCon->isActive()) gpConEmu->UpdateProcessDisplay(FALSE);  // �������� PID � ���� ���������
		}

		mp_RCon->SetTabs(NULL, 1);
	}
	else
	{
		_ASSERTE(nDataSize>=4); //-V112
		_ASSERTE(((pIn->Tabs.nTabCount-1)*sizeof(ConEmuTab))==(nDataSize-sizeof(CESERVER_REQ_CONEMUTAB)));
		BOOL lbCanUpdate = TRUE;

		// ���� ����������� ������ - ��������� �������� ���� FAR ��� ����-����� ������������
		if (pIn->Tabs.bMacroActive)
		{
			if (gpSet->isTabs == 2)
			{
				lbCanUpdate = FALSE;
				// ����� ������� � ������ ����������, ��� ����� ���������� ����������
				CESERVER_REQ *pRet = ExecuteNewCmd(CECMD_TABSCHANGED, sizeof(CESERVER_REQ_HDR) + sizeof(CESERVER_REQ_CONEMUTAB_RET));

				if (pRet)
				{
					pRet->TabsRet.bNeedPostTabSend = TRUE;
					// ���������� (�����, ����� ������ �� ����, ���� ��� �������� ���� ��������)
					fSuccess = mp_RConServer->DelayedWrite(pInst, pRet, pRet->hdr.cbSize);
					//fSuccess = WriteFile(
					//               hPipe,        // handle to pipe
					//               pRet,         // buffer to write from
					//               pRet->hdr.cbSize,  // number of bytes to write
					//               &cbWritten,   // number of bytes written
					//               NULL);        // not overlapped I/O
					ExecuteFreeResult(pRet);
					
					// ����� � ����� ������ �� ���������
					pOut = (CESERVER_REQ*)INVALID_HANDLE_VALUE;
				}
			}
		}

		// ���� �������� �������� - ���������� �������� ������ ������� ����� (� ����� FAR)
		// �� ������ ���� ����� SendTabs ��� ������ �� �������� ���� ���� (����� ������ ����������� � Synchro)
		if (pIn->Tabs.bMainThread && lbCanUpdate && (gpSet->isTabs == 2 && gpSet->bShowFarWindows))
		{
			TODO("��������� ����� ������, ���� ��������� ��������� �����");
			bool lbCurrentActive = gpConEmu->mp_TabBar->IsTabsActive();
			bool lbNewActive = lbCurrentActive;

			// ���� �������� ����� ����� - ��������� ����� �� ���������
			if (gpConEmu->GetVCon(1) == NULL)
			{
				lbNewActive = (pIn->Tabs.nTabCount > 1);
			}

			if (lbCurrentActive != lbNewActive)
			{
				enum ConEmuMargins tTabAction = lbNewActive ? CEM_TABACTIVATE : CEM_TABDEACTIVATE;
				RECT rcConsole = gpConEmu->CalcRect(CER_CONSOLE, gpConEmu->GetIdealRect(), CER_MAIN, NULL, NULL, tTabAction);
				
				mp_RCon->mp_RBuf->SetChange2Size(rcConsole.right, rcConsole.bottom);

				TODO("DoubleView: ��� �������");
				gpConEmu->ActiveCon()->SetRedraw(FALSE);

				gpConEmu->mp_TabBar->SetRedraw(FALSE);
				fSuccess = FALSE;
				// ����� ������� � ������ ����������, ��� ����� ������ �������
				CESERVER_REQ *pTmp = ExecuteNewCmd(CECMD_TABSCHANGED, sizeof(CESERVER_REQ_HDR) + sizeof(CESERVER_REQ_CONEMUTAB_RET));

				if (pTmp)
				{
					pTmp->TabsRet.bNeedResize = TRUE;
					pTmp->TabsRet.crNewSize.X = rcConsole.right;
					pTmp->TabsRet.crNewSize.Y = rcConsole.bottom;
					// ���������� (�����, ����� ������ �� ����, ���� ��� �������� ���� ��������)
					fSuccess = mp_RConServer->DelayedWrite(pInst, pTmp, pTmp->hdr.cbSize);
					//fSuccess = WriteFile(
					//               hPipe,        // handle to pipe
					//               pTmp,         // buffer to write from
					//               pTmp->hdr.cbSize,  // number of bytes to write
					//               &cbWritten,   // number of bytes written
					//               NULL);        // not overlapped I/O
					ExecuteFreeResult(pTmp);
					
					// ����� � ����� ������ �� ���������
					pOut = (CESERVER_REQ*)INVALID_HANDLE_VALUE;
				}

				if (fSuccess)    // ���������, ���� �� ������� ������ ��������� �������
				{
					mp_RCon->WaitConsoleSize(rcConsole.bottom, 500);
				}

				TODO("DoubleView: ��� �������");
				gpConEmu->ActiveCon()->SetRedraw(TRUE);
			}
		}

		if (lbCanUpdate)
		{
			TODO("DoubleView: ��� �������");
			gpConEmu->ActiveCon()->Invalidate();
			mp_RCon->SetTabs(pIn->Tabs.tabs, pIn->Tabs.nTabCount);
			gpConEmu->mp_TabBar->SetRedraw(TRUE);
			gpConEmu->ActiveCon()->Redraw();
		}
	}

	// ���� ��� �� �������� �������
	if (pOut == NULL)
		pOut = ExecuteNewCmd(pIn->hdr.nCmd, sizeof(CESERVER_REQ_HDR));

	return pOut;
}

CESERVER_REQ* CRealServer::cmdGetOutputFile(LPVOID pInst, CESERVER_REQ* pIn, UINT nDataSize)
{
	CESERVER_REQ* pOut = NULL;
	
	DEBUGSTRCMD(L"GUI recieved CECMD_GETOUTPUTFILE\n");
	_ASSERTE(nDataSize>=4); //-V112
	BOOL lbUnicode = pIn->OutputFile.bUnicode;
	pOut = ExecuteNewCmd(pIn->hdr.nCmd, sizeof(CESERVER_REQ_HDR) + sizeof(CESERVER_REQ_OUTPUTFILE));
	pOut->OutputFile.bUnicode = lbUnicode;
	pOut->OutputFile.szFilePathName[0] = 0; // ���������� mp_RCon->PrepareOutputFile

	if (!mp_RCon->PrepareOutputFile(lbUnicode, pOut->OutputFile.szFilePathName))
	{
		pOut->OutputFile.szFilePathName[0] = 0;
	}

	return pOut;
}

CESERVER_REQ* CRealServer::cmdGuiMacro(LPVOID pInst, CESERVER_REQ* pIn, UINT nDataSize)
{
	CESERVER_REQ* pOut = NULL;

	DEBUGSTRCMD(L"GUI recieved CECMD_GUIMACRO\n");	
	LPWSTR pszResult = CConEmuMacro::ExecuteMacro(pIn->GuiMacro.sMacro, mp_RCon);
	int nLen = pszResult ? _tcslen(pszResult) : 0;
	pOut = ExecuteNewCmd(pIn->hdr.nCmd, sizeof(CESERVER_REQ_HDR)+sizeof(CESERVER_REQ_GUIMACRO)+nLen*sizeof(wchar_t));

	if (pszResult)
	{
		lstrcpy(pOut->GuiMacro.sMacro, pszResult);
		pOut->GuiMacro.nSucceeded = 1;
		free(pszResult);
	}
	else
	{
		pOut->GuiMacro.sMacro[0] = 0;
		pOut->GuiMacro.nSucceeded = 0;
	}
	
	return pOut;
}

CESERVER_REQ* CRealServer::cmdLangChange(LPVOID pInst, CESERVER_REQ* pIn, UINT nDataSize)
{
	CESERVER_REQ* pOut = NULL;
	
	DEBUGSTRLANG(L"GUI recieved CECMD_LANGCHANGE\n");
	_ASSERTE(nDataSize>=4); //-V112
	// LayoutName: "00000409", "00010409", ...
	// � HKL �� ���� ����������, ��� ��� �������� DWORD
	// HKL � x64 �������� ���: "0x0000000000020409", "0xFFFFFFFFF0010409"
	DWORD dwName = pIn->dwData[0];
	gpConEmu->OnLangChangeConsole(mp_RCon->mp_VCon, dwName);

	pOut = ExecuteNewCmd(pIn->hdr.nCmd, sizeof(CESERVER_REQ_HDR));

	//#ifdef _DEBUG
	////Sleep(2000);
	//WCHAR szMsg[255];
	//// --> ������ ������ ��� �� "��������" �������. ����� ����������� Post'�� � �������� ����
	//HKL hkl = GetKeyboardLayout(0);
	//swprintf_c(szMsg, L"ConEmu: GetKeyboardLayout(0) on CECMD_LANGCHANGE after GetKeyboardLayout(0) = 0x%08I64X\n",
	//	(unsigned __int64)(DWORD_PTR)hkl);
	//DEBUGSTRLANG(szMsg);
	////Sleep(2000);
	//#endif
	//
	//wchar_t szName[10]; swprintf_c(szName, L"%08X", dwName);
	//DWORD_PTR dwNewKeybLayout = (DWORD_PTR)LoadKeyboardLayout(szName, 0);
	//
	//#ifdef _DEBUG
	//DEBUGSTRLANG(L"ConEmu: Calling GetKeyboardLayout(0)\n");
	////Sleep(2000);
	//hkl = GetKeyboardLayout(0);
	//swprintf_c(szMsg, L"ConEmu: GetKeyboardLayout(0) after LoadKeyboardLayout = 0x%08I64X\n",
	//	(unsigned __int64)(DWORD_PTR)hkl);
	//DEBUGSTRLANG(szMsg);
	////Sleep(2000);
	//#endif
	//
	//if ((gpSet->isMonitorConsoleLang & 1) == 1) {
	//    if (con.dwKeybLayout != dwNewKeybLayout) {
	//        con.dwKeybLayout = dwNewKeybLayout;
	//		if (mp_RCon->isActive()) {
	//            gpConEmu->SwitchKeyboardLayout(dwNewKeybLayout);
	//
	//			#ifdef _DEBUG
	//			hkl = GetKeyboardLayout(0);
	//			swprintf_c(szMsg, L"ConEmu: GetKeyboardLayout(0) after SwitchKeyboardLayout = 0x%08I64X\n",
	//				(unsigned __int64)(DWORD_PTR)hkl);
	//			DEBUGSTRLANG(szMsg);
	//			//Sleep(2000);
	//			#endif
	//		}
	//    }
	//}
	return pOut;
}

CESERVER_REQ* CRealServer::cmdTabsCmd(LPVOID pInst, CESERVER_REQ* pIn, UINT nDataSize)
{
	CESERVER_REQ* pOut = NULL;
	
	// 0: ��������/�������� ����, 1: ������� �� ���������, 2: ������� �� ����������, 3: commit switch
	DEBUGSTRCMD(L"GUI recieved CECMD_TABSCMD\n");
	_ASSERTE(nDataSize>=1);
	DWORD nTabCmd = pIn->Data[0];
	gpConEmu->TabCommand(nTabCmd);
	pOut = ExecuteNewCmd(pIn->hdr.nCmd, sizeof(CESERVER_REQ_HDR));

	return pOut;
}

CESERVER_REQ* CRealServer::cmdResources(LPVOID pInst, CESERVER_REQ* pIn, UINT nDataSize)
{
	CESERVER_REQ* pOut = NULL;
	
	DEBUGSTRCMD(L"GUI recieved CECMD_RESOURCES\n");
	_ASSERTE(nDataSize>=6);
	//mb_PluginDetected = TRUE; // ��������, ��� � ���� ���� ������ (���� ��� ����� ���� ������)
	DWORD nPID = pIn->dwData[0]; // ��������, ��� � ���� ���� ������
	mp_RCon->mb_SkipFarPidChange = TRUE;
	// ��������� ���� PID � ������ �����
	bool bAlreadyExist = false;
	int j = -1;

	for (UINT i = 0; i < mp_RCon->mn_FarPlugPIDsCount; i++)
	{
		if (mp_RCon->m_FarPlugPIDs[i] == nPID)
		{
			bAlreadyExist = true; break;
		}
		else if (mp_RCon->m_FarPlugPIDs[i] == 0)
		{
			j = i;
		}
	}

	if (!bAlreadyExist)
	{
		// ���� � ������ ����� ����� PID� ��� ��� - �� ����������� ���������
		if ((j == -1) && (mp_RCon->mn_FarPlugPIDsCount < countof(mp_RCon->m_FarPlugPIDs)))
			j = mp_RCon->mn_FarPlugPIDsCount++;

		if (j >= 0)
			mp_RCon->m_FarPlugPIDs[j] = nPID;
	}

	// ��������, ��� � ���� ���� ������
	mp_RCon->mn_FarPID_PluginDetected = nPID;
	mp_RCon->OpenFarMapData(); // ����������� ������� � ����������� � ����
	// ��������� ���������� PID ���� � MonitorThread (��� ����� ������������� mp_RCon->OpenFarMapData)
	mp_RCon->mb_SkipFarPidChange = FALSE;

	if (mp_RCon->isActive()) gpConEmu->UpdateProcessDisplay(FALSE);  // �������� PID � ���� ���������

	//mn_Far_PluginInputThreadId      = pIn->dwData[1];
	//CheckColorMapping(mp_RCon->mn_FarPID_PluginDetected);
	// 23.06.2009 Maks - ������ ����. ������ �������� � ApplyConsoleInfo
	//Process Add(mp_RCon->mn_FarPID_PluginDetected); // �� ������ ������, ����� �� ��� �� � ����� ������?
	wchar_t* pszRes = (wchar_t*)(&(pIn->dwData[1])), *pszNext;

	if (*pszRes)
	{
		//EnableComSpec(mp_RCon->mn_FarPID_PluginDetected, TRUE);
		//mp_RCon->UpdateFarSettings(mp_RCon->mn_FarPID_PluginDetected);
		wchar_t* pszItems[] = {mp_RCon->ms_EditorRus,mp_RCon->ms_ViewerRus,mp_RCon->ms_TempPanelRus/*,ms_NameTitle*/};

		for (UINT i = 0; i < countof(pszItems); i++)
		{
			pszNext = pszRes + _tcslen(pszRes)+1;

			if (_tcslen(pszRes)>=30) pszRes[30] = 0;

			lstrcpy(pszItems[i], pszRes);

			if (i < 2) lstrcat(pszItems[i], L" ");

			pszRes = pszNext;

			if (*pszRes == 0)
				break;
		}

		//pszNext = pszRes + _tcslen(pszRes)+1;
		//if (_tcslen(pszRes)>=30) pszRes[30] = 0;
		//lstrcpy(ms_EditorRus, pszRes); lstrcat(ms_EditorRus, L" ");
		//pszRes = pszNext;
		//if (*pszRes) {
		//    pszNext = pszRes + _tcslen(pszRes)+1;
		//    if (_tcslen(pszRes)>=30) pszRes[30] = 0;
		//    lstrcpy(ms_ViewerRus, pszRes); lstrcat(ms_ViewerRus, L" ");
		//    pszRes = pszNext;
		//    if (*pszRes) {
		//        pszNext = pszRes + _tcslen(pszRes)+1;
		//        if (_tcslen(pszRes)>=31) pszRes[31] = 0;
		//        lstrcpy(ms_TempPanelRus, pszRes);
		//        pszRes = pszNext;
		//    }
		//}
	}

	mp_RCon->UpdateFarSettings(mp_RCon->mn_FarPID_PluginDetected);

	pOut = ExecuteNewCmd(pIn->hdr.nCmd, sizeof(CESERVER_REQ_HDR));

	return pOut;
}

CESERVER_REQ* CRealServer::cmdSetForeground(LPVOID pInst, CESERVER_REQ* pIn, UINT nDataSize)
{
	CESERVER_REQ* pOut = NULL;
	
	DEBUGSTRCMD(L"GUI recieved CECMD_SETFOREGROUNDWND\n");
	AllowSetForegroundWindow(pIn->hdr.nSrcPID);

	HWND hWnd = (HWND)pIn->qwData[0];
	DWORD nWndPID = 0; GetWindowThreadProcessId(hWnd, &nWndPID);
	if (nWndPID == GetCurrentProcessId())
	{
		// ���� ��� ���� �� hWndDC - ��������� ������� ����
		if (hWnd != ghWnd && GetParent(hWnd) == ghWnd)
			hWnd = ghWnd;
	}
	
	BOOL lbRc = apiSetForegroundWindow(hWnd);
	pOut = ExecuteNewCmd(pIn->hdr.nCmd, sizeof(CESERVER_REQ_HDR)+sizeof(DWORD));
	if (pOut)
		pOut->dwData[0] = lbRc;

	return pOut;
}

CESERVER_REQ* CRealServer::cmdFlashWindow(LPVOID pInst, CESERVER_REQ* pIn, UINT nDataSize)
{
	CESERVER_REQ* pOut = NULL;

	DEBUGSTRCMD(L"GUI recieved CECMD_FLASHWINDOW\n");	
	UINT nFlash = RegisterWindowMessage(CONEMUMSG_FLASHWINDOW);
	WPARAM wParam = 0;

	if (pIn->Flash.bSimple)
	{
		wParam = (pIn->Flash.bInvert ? 2 : 1) << 25;
	}
	else
	{
		wParam = ((pIn->Flash.dwFlags & 0xF) << 24) | (pIn->Flash.uCount & 0xFFFFFF);
	}

	PostMessage(ghWnd, nFlash, wParam, (LPARAM)pIn->Flash.hWnd.u);
	pOut = ExecuteNewCmd(pIn->hdr.nCmd, sizeof(CESERVER_REQ_HDR));

	return pOut;
}

CESERVER_REQ* CRealServer::cmdRegPanelView(LPVOID pInst, CESERVER_REQ* pIn, UINT nDataSize)
{
	CESERVER_REQ* pOut = NULL;
	
	DEBUGSTRCMD(L"GUI recieved CECMD_REGPANELVIEW\n");
	pOut = ExecuteNewCmd(pIn->hdr.nCmd, sizeof(CESERVER_REQ_HDR)+sizeof(pIn->PVI));
	pOut->PVI = pIn->PVI;

	if (pOut->PVI.cbSize != sizeof(pOut->PVI))
	{
		pOut->PVI.cbSize = 0; // ������ ������?
	}
	else if (!mp_RCon->mp_VCon->RegisterPanelView(&(pOut->PVI)))
	{
		pOut->PVI.cbSize = 0; // ������
	}

	return pOut;
}

CESERVER_REQ* CRealServer::cmdSetBackground(LPVOID pInst, CESERVER_REQ* pIn, UINT nDataSize)
{
	CESERVER_REQ* pOut = NULL;

	DEBUGSTRCMD(L"GUI recieved CECMD_SETBACKGROUND\n");	
	pOut = ExecuteNewCmd(pIn->hdr.nCmd, sizeof(CESERVER_REQ_HDR)+sizeof(CESERVER_REQ_SETBACKGROUNDRET));
	// Set background Image
	UINT nCalcSize = pIn->hdr.cbSize - sizeof(pIn->hdr);

	if (nCalcSize < sizeof(CESERVER_REQ_SETBACKGROUND))
	{
		_ASSERTE(nCalcSize >= sizeof(CESERVER_REQ_SETBACKGROUND));
		pOut->BackgroundRet.nResult = esbr_InvalidArg;
	}
	else
	{
		UINT nCalcBmSize = nCalcSize - (((LPBYTE)&pIn->Background.bmp) - ((LPBYTE)&pIn->Background));

		if (pIn->Background.bEnabled && nCalcSize < nCalcBmSize)
		{
			_ASSERTE(nCalcSize >= nCalcBmSize);
			pOut->BackgroundRet.nResult = esbr_InvalidArg;
		}
		else
		{
			pOut->BackgroundRet.nResult = mp_RCon->mp_VCon->SetBackgroundImageData(&pIn->Background);
		}
	}

	return pOut;
}

CESERVER_REQ* CRealServer::cmdActivateCon(LPVOID pInst, CESERVER_REQ* pIn, UINT nDataSize)
{
	CESERVER_REQ* pOut = NULL;
	
	DEBUGSTRCMD(L"GUI recieved CECMD_ACTIVATECON\n");
	pOut = ExecuteNewCmd(pIn->hdr.nCmd, sizeof(CESERVER_REQ_HDR)+sizeof(CESERVER_REQ_ACTIVATECONSOLE));
	// Activate current console
	_ASSERTE(mp_RCon->hConWnd == (HWND)pIn->ActivateCon.hConWnd);

	if (gpConEmu->Activate(mp_RCon->mp_VCon))
		pOut->ActivateCon.hConWnd = mp_RCon->hConWnd;
	else
		pOut->ActivateCon.hConWnd = NULL;

	return pOut;
}

CESERVER_REQ* CRealServer::cmdOnCreateProc(LPVOID pInst, CESERVER_REQ* pIn, UINT nDataSize)
{
	CESERVER_REQ* pOut = NULL;

	DEBUGSTRCMD(L"GUI recieved CECMD_ONCREATEPROC\n");	
	pOut = ExecuteNewCmd(pIn->hdr.nCmd, 
		sizeof(CESERVER_REQ_HDR)+sizeof(CESERVER_REQ_ONCREATEPROCESSRET)
		/*+MAX_PATH*6*/);
	
	BOOL lbDos = (pIn->OnCreateProc.nImageBits == 16)
		&& (pIn->OnCreateProc.nImageSubsystem == IMAGE_SUBSYSTEM_DOS_EXECUTABLE);

	if (ghOpWnd && gpSetCls->hDebug)
	{
		DebugLogShellActivity *shl = (DebugLogShellActivity*)calloc(sizeof(DebugLogShellActivity),1);
		shl->nParentPID = pIn->hdr.nSrcPID;
		shl->nParentBits = pIn->OnCreateProc.nSourceBits;
		wcscpy_c(shl->szFunction, pIn->OnCreateProc.sFunction);
		shl->pszAction = lstrdup(pIn->OnCreateProc.wsValue);
		shl->pszFile   = lstrdup(pIn->OnCreateProc.wsValue+pIn->OnCreateProc.nActionLen);
		shl->pszParam  = lstrdup(pIn->OnCreateProc.wsValue+pIn->OnCreateProc.nActionLen+pIn->OnCreateProc.nFileLen);
		shl->bDos = lbDos;
		shl->nImageBits = pIn->OnCreateProc.nImageBits;
		shl->nImageSubsystem = pIn->OnCreateProc.nImageSubsystem;
		shl->nShellFlags = pIn->OnCreateProc.nShellFlags;
		shl->nCreateFlags = pIn->OnCreateProc.nCreateFlags;
		shl->nStartFlags = pIn->OnCreateProc.nStartFlags;
		shl->nShowCmd = pIn->OnCreateProc.nShowCmd;
		shl->hStdIn = (DWORD)pIn->OnCreateProc.hStdIn;
		shl->hStdOut = (DWORD)pIn->OnCreateProc.hStdOut;
		shl->hStdErr = (DWORD)pIn->OnCreateProc.hStdErr;

		PostMessage(gpSetCls->hDebug, DBGMSG_LOG_ID, DBGMSG_LOG_SHELL_MAGIC, (LPARAM)shl);
	}
	
	if (pIn->OnCreateProc.nImageBits > 0)
	{
		TODO("!!! DosBox allowed?");
		_ASSERTE(lbDos==FALSE); //PRAGMA_ERROR("����� (lbDos && FALSE)?");
		
		if (gpSet->AutoBufferHeight // LongConsoleOutput
			|| (lbDos && FALSE)) // DosBox!!!
		{
			pOut->OnCreateProcRet.bContinue = TRUE;
			//pOut->OnCreateProcRet.bUnicode = TRUE;
			pOut->OnCreateProcRet.bForceBufferHeight = gpSet->AutoBufferHeight;
			
			TODO("!!! DosBox allowed?");
			pOut->OnCreateProcRet.bAllowDosbox = FALSE;
			//pOut->OnCreateProcRet.nFileLen = pIn->OnCreateProc.nFileLen;
			//pOut->OnCreateProcRet.nBaseLen = _tcslen(gpConEmu->ms_ConEmuBaseDir)+2; // +����+\0
			
			////_wcscpy_c(pOut->OnCreateProcRet.wsValue, MAX_PATH+1, pszFile);
			//_wcscpy_c(pOut->OnCreateProcRet.wsValue, MAX_PATH+1, gpConEmu->ms_ConEmuBaseDir);
			//_wcscat_c(pOut->OnCreateProcRet.wsValue, MAX_PATH+1, L"\\");
		}
	}
	
	return pOut;
}

//CESERVER_REQ* CRealServer::cmdNewConsole(LPVOID pInst, CESERVER_REQ* pIn, UINT nDataSize)
//{
//	CESERVER_REQ* pOut = NULL;
//
//	DEBUGSTRCMD(L"GUI recieved CECMD_NEWCONSOLE\n");		
//	pOut = ExecuteNewCmd(pIn->hdr.nCmd, sizeof(CESERVER_REQ_HDR)+sizeof(wchar_t));
//	pOut->wData[0] = 0;
//	
//	return pOut;
//}

CESERVER_REQ* CRealServer::cmdOnPeekReadInput(LPVOID pInst, CESERVER_REQ* pIn, UINT nDataSize)
{
	CESERVER_REQ* pOut = NULL;

	DEBUGSTRCMD(L"GUI recieved CECMD_PEEKREADINFO\n");
	
	if (ghOpWnd && gpSetCls->hDebug && gpSetCls->m_ActivityLoggingType == glt_Input)
	{
		if (nDataSize >= sizeof(CESERVER_REQ_PEEKREADINFO))
		{
			CESERVER_REQ_PEEKREADINFO* pCopy = (CESERVER_REQ_PEEKREADINFO*)malloc(nDataSize);
			if (pCopy)
			{
				memmove(pCopy, &pIn->PeekReadInfo, nDataSize);
				PostMessage(gpSetCls->hDebug, DBGMSG_LOG_ID, DBGMSG_LOG_INPUT_MAGIC, (LPARAM)pCopy);
			}
		}
	}
	
	pOut = ExecuteNewCmd(pIn->hdr.nCmd, sizeof(CESERVER_REQ_HDR));	
	return pOut;
}

CESERVER_REQ* CRealServer::cmdOnSetConsoleKeyShortcuts(LPVOID pInst, CESERVER_REQ* pIn, UINT nDataSize)
{
	CESERVER_REQ* pOut = NULL;

	DEBUGSTRCMD(L"GUI recieved CECMD_KEYSHORTCUTS\n");

	mp_RCon->m_ConsoleKeyShortcuts = pIn->Data[0] ? pIn->Data[1] : 0;
	gpConEmu->UpdateWinHookSettings();

	pOut = ExecuteNewCmd(pIn->hdr.nCmd, sizeof(CESERVER_REQ_HDR));	
	return pOut;
}

CESERVER_REQ* CRealServer::cmdLockDc(LPVOID pInst, CESERVER_REQ* pIn, UINT nDataSize)
{
	CESERVER_REQ* pOut = NULL;

	DEBUGSTRCMD(L"GUI recieved CECMD_LOCKDC\n");
	
	_ASSERTE(pIn->LockDc.hDcWnd == mp_RCon->mp_VCon->GetView());
	
	mp_RCon->mp_VCon->LockDcRect(pIn->LockDc.bLock, &pIn->LockDc.Rect);
	
	pOut = ExecuteNewCmd(pIn->hdr.nCmd, sizeof(CESERVER_REQ_HDR));	
	return pOut;
}

//CESERVER_REQ* CRealServer::cmdAssert(LPVOID pInst, CESERVER_REQ* pIn, UINT nDataSize)
//{
//	CESERVER_REQ* pOut = NULL;
//
//	DEBUGSTRCMD(L"GUI recieved CECMD_ASSERT\n");		
//	pOut = ExecuteNewCmd(pIn->hdr.nCmd, sizeof(CESERVER_REQ_HDR)+sizeof(wchar_t));
//	pOut->wData[0] = MessageBox(NULL, pIn->AssertInfo.szDebugInfo, pIn->AssertInfo.szTitle, MB_SETFOREGROUND|MB_SYSTEMMODAL|MB_RETRYCANCEL);
//	
//	return pOut;
//}

// ��� ������� ���� �� ���������!
//void CRealServer::ServerThreadCommand(HANDLE hPipe)
BOOL CRealServer::ServerCommand(LPVOID pInst, CESERVER_REQ* pIn, CESERVER_REQ* &ppReply, DWORD &pcbReplySize, DWORD &pcbMaxReplySize, LPARAM lParam)
{
	BOOL lbRc = FALSE;
	CRealServer* pRSrv = (CRealServer*)lParam;
	CRealConsole* pRCon = (CRealConsole*)pRSrv->mp_RCon;
	
	ExecuteFreeResult(ppReply);
	CESERVER_REQ *pOut = NULL;

//	CESERVER_REQ in= {{0}}, *pIn=NULL;
//	DWORD cbRead = 0, cbWritten = 0, dwErr = 0;
//	BOOL fSuccess = FALSE;
//#ifdef _DEBUG
//	HANDLE lhConEmuC = mh_ConEmuC;
//#endif
//	MCHKHEAP;
//	// Send a message to the pipe server and read the response.
//	fSuccess = ReadFile(
//	               hPipe,            // pipe handle
//	               &in,              // buffer to receive reply
//	               sizeof(in),       // size of read buffer
//	               &cbRead,          // bytes read
//	               NULL);            // not overlapped
//
//	if (!fSuccess && ((dwErr = GetLastError()) != ERROR_MORE_DATA))
//	{
//#ifdef _DEBUG
//		// ���� ������� ����������� - MonitorThread � ��������� ����� ��� ������
//		DEBUGSTRPROC(L"!!! ReadFile(pipe) failed - console in close?\n");
//		//DWORD dwWait = WaitForSingleObject ( mh_TermEvent, 0 );
//		//if (dwWait == WAIT_OBJECT_0) return;
//		//Sleep(1000);
//		//if (lhConEmuC != mh_ConEmuC)
//		//	dwWait = WAIT_OBJECT_0;
//		//else
//		//	dwWait = WaitForSingleObject ( mh_ConEmuC, 0 );
//		//if (dwWait == WAIT_OBJECT_0) return;
//		//_ASSERTE("ReadFile(pipe) failed"==NULL);
//#endif
//		//CloseHandle(hPipe);
//		return;
//	}

	if (pIn->hdr.nVersion != CESERVER_REQ_VER)
	{
		gpConEmu->ReportOldCmdVersion(pIn->hdr.nCmd, pIn->hdr.nVersion, pIn->hdr.nSrcPID==pRCon->GetServerPID() ? 1 : 0,
			pIn->hdr.nSrcPID, pIn->hdr.hModule, pIn->hdr.nBits);
		return FALSE;
	}

	if (pIn->hdr.cbSize < sizeof(CESERVER_REQ_HDR))
	{
		_ASSERTE(pIn->hdr.cbSize>=sizeof(CESERVER_REQ_HDR));
		//CloseHandle(hPipe);
		return FALSE;
	}

	DWORD dwTimeStart = timeGetTime();
	//gpSetCls->debugLogCommand(pIn, TRUE, timeGetTime(), 0, ms_VConServer_Pipe, NULL/*pOut*/);

	//if (in.hdr.cbSize <= cbRead)
	//{
	//	pIn = &in; // ��������� ������ �� ���������
	//}
	//else
	//{
	//	int nAllSize = in.hdr.cbSize;
	//	pIn = (CESERVER_REQ*)calloc(nAllSize,1);
	//	_ASSERTE(pIn!=NULL);
	//	memmove(pIn, &in, cbRead);
	//	_ASSERTE(pIn->hdr.nVersion==CESERVER_REQ_VER);
	//	LPBYTE ptrData = ((LPBYTE)pIn)+cbRead;
	//	nAllSize -= cbRead;

	//	while(nAllSize>0)
	//	{
	//		//_tprintf(TEXT("%s\n"), chReadBuf);

	//		// Break if TransactNamedPipe or ReadFile is successful
	//		if (fSuccess)
	//			break;

	//		// Read from the pipe if there is more data in the message.
	//		fSuccess = ReadFile(
	//		               hPipe,      // pipe handle
	//		               ptrData,    // buffer to receive reply
	//		               nAllSize,   // size of buffer
	//		               &cbRead,    // number of bytes read
	//		               NULL);      // not overlapped

	//		// Exit if an error other than ERROR_MORE_DATA occurs.
	//		if (!fSuccess && ((dwErr = GetLastError()) != ERROR_MORE_DATA))
	//			break;

	//		ptrData += cbRead;
	//		nAllSize -= cbRead;
	//	}

	//	TODO("����� ���������� ASSERT, ���� ������� ���� ������� � �������� ������");
	//	_ASSERTE(nAllSize==0);

	//	if (nAllSize>0)
	//	{
	//		//CloseHandle(hPipe);
	//		return; // ������� ������� �� ��� ������
	//	}
	//}

	int nDataSize = pIn->hdr.cbSize - sizeof(CESERVER_REQ_HDR);

	// ��� ������ �� ����� ��������, ������������ ������� � ���������� (���� �����) ���������

	//  //if (pIn->hdr.nCmd == CECMD_GETFULLINFO /*|| pIn->hdr.nCmd == CECMD_GETSHORTINFO*/) {
	//  if (pIn->hdr.nCmd == CECMD_GETCONSOLEINFO) {
	//  	_ASSERTE(pIn->hdr.nCmd != CECMD_GETCONSOLEINFO);
	//// ������ ���� �� �� � ����� �������. ����� �� �������, ���������� ������ ��������� PopPacket
	////if (!con.bInSetSize && !con.bBufferHeight && pIn->ConInfo.inf.sbi.dwSize.Y > 200) {
	////	_ASSERTE(con.bBufferHeight || pIn->ConInfo.inf.sbi.dwSize.Y <= 200);
	////}
	////#ifdef _DEBUG
	////wchar_t szDbg[255]; swprintf_c(szDbg, L"GUI recieved %s, PktID=%i, Tick=%i\n",
	////	(pIn->hdr.nCmd == CECMD_GETFULLINFO) ? L"CECMD_GETFULLINFO" : L"CECMD_GETSHORTINFO",
	////	pIn->ConInfo.inf.nPacketId, pIn->hdr.nCreateTick);
	//      //DEBUGSTRCMD(szDbg);
	////#endif
	//      ////ApplyConsoleInfo(pIn);
	//      //if (((LPVOID)&in)==((LPVOID)pIn)) {
	//      //    // ��� ������������� ������ - ���������� (in)
	//      //    _ASSERTE(in.hdr.cbSize>0);
	//      //    // ��� ��� ��������� ����� ������� ����� ������, ������� ��������� PopPacket
	//      //    pIn = (CESERVER_REQ*)calloc(in.hdr.cbSize,1);
	//      //    memmove(pIn, &in, in.hdr.cbSize);
	//      //}
	//      //PushPacket(pIn);
	//      //pIn = NULL;

	//  } else

	if (pIn->hdr.nCmd == CECMD_CMDSTARTSTOP)
		pOut = pRSrv->cmdStartStop(pInst, pIn, nDataSize);
	//else if (pIn->hdr.nCmd == CECMD_GETGUIHWND)
	//	pOut = pRSrv->cmdGetGuiHwnd(pInst, pIn, nDataSize);
	else if (pIn->hdr.nCmd == CECMD_TABSCHANGED)
		pOut = pRSrv->cmdTabsChanged(pInst, pIn, nDataSize);
	else if (pIn->hdr.nCmd == CECMD_GETOUTPUTFILE)
		pOut = pRSrv->cmdGetOutputFile(pInst, pIn, nDataSize);
	else if (pIn->hdr.nCmd == CECMD_GUIMACRO)
		pOut = pRSrv->cmdGuiMacro(pInst, pIn, nDataSize);
	else if (pIn->hdr.nCmd == CECMD_LANGCHANGE)
		pOut = pRSrv->cmdLangChange(pInst, pIn, nDataSize);
	else if (pIn->hdr.nCmd == CECMD_TABSCMD)
		pOut = pRSrv->cmdTabsCmd(pInst, pIn, nDataSize);
	else if (pIn->hdr.nCmd == CECMD_RESOURCES)
		pOut = pRSrv->cmdResources(pInst, pIn, nDataSize);
	else if (pIn->hdr.nCmd == CECMD_SETFOREGROUND)
		pOut = pRSrv->cmdSetForeground(pInst, pIn, nDataSize);
	else if (pIn->hdr.nCmd == CECMD_FLASHWINDOW)
		pOut = pRSrv->cmdFlashWindow(pInst, pIn, nDataSize);
	else if (pIn->hdr.nCmd == CECMD_REGPANELVIEW)
		pOut = pRSrv->cmdRegPanelView(pInst, pIn, nDataSize);
	else if (pIn->hdr.nCmd == CECMD_SETBACKGROUND)
		pOut = pRSrv->cmdSetBackground(pInst, pIn, nDataSize);
	else if (pIn->hdr.nCmd == CECMD_ACTIVATECON)
		pOut = pRSrv->cmdActivateCon(pInst, pIn, nDataSize);
	else if (pIn->hdr.nCmd == CECMD_ONCREATEPROC)
		pOut = pRSrv->cmdOnCreateProc(pInst, pIn, nDataSize);
	//else if (pIn->hdr.nCmd == CECMD_NEWCONSOLE)
	//	pOut = pRSrv->cmdNewConsole(pInst, pIn, nDataSize);
	else if (pIn->hdr.nCmd == CECMD_PEEKREADINFO)
		pOut = pRSrv->cmdOnPeekReadInput(pInst, pIn, nDataSize);
	else if (pIn->hdr.nCmd == CECMD_KEYSHORTCUTS)
		pOut = pRSrv->cmdOnSetConsoleKeyShortcuts(pInst, pIn, nDataSize);
	else if (pIn->hdr.nCmd == CECMD_ALIVE)
		pOut = ExecuteNewCmd(CECMD_ALIVE, sizeof(CESERVER_REQ_HDR));
	//else if (pIn->hdr.nCmd == CECMD_ASSERT)
	else if (pIn->hdr.nCmd == CECMD_LOCKDC)
		pOut = pRSrv->cmdLockDc(pInst, pIn, nDataSize);
	//else if (pIn->hdr.nCmd == CECMD_ASSERT)
	//	pOut = cmdAssert(pInst, pIn, nDataSize);
	else
	{
		// ����������� �������
		_ASSERTE(pIn->hdr.nCmd == CECMD_CMDSTARTSTOP);

		// ���� �� "������" ������� � ����� ������, � �� ������ (Pipe was closed) � ������� ���������
		// 0 - ����� assert-��� ������ �������������� �������
		pOut = ExecuteNewCmd(0/*pIn->hdr.nCmd*/, sizeof(CESERVER_REQ_HDR));
	}


	
	if (pOut != (CESERVER_REQ*)INVALID_HANDLE_VALUE)
	{
		if (pOut == NULL)
		{
			// ��� ��������, ������ ������ ���� ���������� �������� ���������
			_ASSERTE(pOut!=NULL);
			// ���� �� "������" ������� � ����� ������, � �� ������ (Pipe was closed) � ������� ���������
			pOut = ExecuteNewCmd(pIn->hdr.nCmd, sizeof(CESERVER_REQ_HDR));
		}

		DWORD dwDur = timeGetTime() - dwTimeStart;
		gpSetCls->debugLogCommand(pIn, TRUE, dwTimeStart, dwDur, pRCon->ms_VConServer_Pipe, pOut);

		ppReply = pOut;
		if (pOut)
		{
			pcbReplySize = pOut->hdr.cbSize;
			lbRc = TRUE;
		}
		
		//// ������ ����-������ �������� � ����, � �� ������ (Pipe was closed) � ������� ���������
		//fSuccess = WriteFile(hPipe, pOut, pOut->hdr.cbSize, &cbWritten, NULL);
		//ExecuteFreeResult(pOut);
	}
	else
	{
		DWORD dwDur = timeGetTime() - dwTimeStart;
		gpSetCls->debugLogCommand(pIn, TRUE, dwTimeStart, dwDur, pRCon->ms_VConServer_Pipe, NULL/*pOut*/);
		
		// Delayed write
		_ASSERTE(ppReply==NULL);
		ppReply = NULL;
		lbRc = TRUE;
	}
	

	//// ���������� ������
	//if (pIn && (LPVOID)pIn != (LPVOID)&in)
	//{
	//	free(pIn); pIn = NULL;
	//}

	MCHKHEAP;
	//CloseHandle(hPipe);
	return lbRc;
}

BOOL CRealServer::ServerThreadReady(LPVOID pInst, LPARAM lParam)
{
	CRealServer* pRSrv = (CRealServer*)lParam;

	// ����� ConEmuC ����, ��� ��������� ���� �����
	if (pRSrv && pRSrv->mh_GuiAttached)
	{
		SetEvent(pRSrv->mh_GuiAttached);
		SafeCloseHandle(pRSrv->mh_GuiAttached);
	}
	else
	{
		_ASSERTE(pRSrv && pRSrv->mh_GuiAttached);
	}

	return TRUE;
}

void CRealServer::ServerCommandFree(CESERVER_REQ* pReply, LPARAM lParam)
{
	ExecuteFreeResult(pReply);
}
