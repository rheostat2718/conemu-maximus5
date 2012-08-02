
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
#include "Header.h"
#include "GuiServer.h"
#include "RealConsole.h"
#include "VConGroup.h"
#include "VirtualConsole.h"
#include "ConEmu.h"
#include "../common/PipeServer.h"

#ifdef _DEBUG
//	#define ALLOW_WINE_MSG
#else
	#undef ALLOW_WINE_MSG
#endif

CGuiServer::CGuiServer()
{
	//mn_GuiServerThreadId = 0; mh_GuiServerThread = NULL; mh_GuiServerThreadTerminate = NULL;
	mp_GuiServer = (PipeServer<CESERVER_REQ>*)calloc(1, sizeof(*mp_GuiServer));
	mp_GuiServer->SetMaxCount(2);
	ms_ServerPipe[0] = 0;
}

CGuiServer::~CGuiServer()
{
	Stop(true);
}

bool CGuiServer::Start()
{
	// ��������� ��������� ����
	// 120122 - ������ ����� PipeServer
	_wsprintf(ms_ServerPipe, SKIPLEN(countof(ms_ServerPipe)) CEGUIPIPENAME, L".", (DWORD)ghWnd); //-V205
	
	mp_GuiServer->SetOverlapped(true);
	mp_GuiServer->SetLoopCommands(false);
	mp_GuiServer->SetDummyAnswerSize(sizeof(CESERVER_REQ_HDR));

	PipeServer<CESERVER_REQ>::PipeServerConnected_t lpfnOnConnected = NULL;
#ifdef _DEBUG
	lpfnOnConnected = CGuiServer::OnGuiServerConnected;
#endif
	
	if (!mp_GuiServer->StartPipeServer(ms_ServerPipe, NULL, LocalSecurity(), GuiServerCommand, GuiServerFree, lpfnOnConnected, NULL))
	{
		// ������ ��� ��������
		return false;
	}
	//mh_GuiServerThreadTerminate = CreateEvent(NULL, TRUE, FALSE, NULL);
	//if (mh_GuiServerThreadTerminate) ResetEvent(mh_GuiServerThreadTerminate);
	//mh_GuiServerThread = CreateThread(NULL, 0, GuiServerThread, (LPVOID)this, 0, &mn_GuiServerThreadId);

	return true;
}

void CGuiServer::Stop(bool abDeinitialize/*=false*/)
{
	//120122 - ������ ����� PipeServer
	if (mp_GuiServer)
	{
		ShutdownGuiStep(L"mp_GuiServer->StopPipeServer");

		mp_GuiServer->StopPipeServer();

		if (abDeinitialize)
		{
			free(mp_GuiServer);
			mp_GuiServer = NULL;
		}

		ShutdownGuiStep(L"mp_GuiServer->StopPipeServer - done");
	}
}

//// ��� ������� ���� �� ���������!
//void CGuiServer::GuiServerThreadCommand(HANDLE hPipe)
BOOL CGuiServer::GuiServerCommand(LPVOID pInst, CESERVER_REQ* pIn, CESERVER_REQ* &ppReply, DWORD &pcbReplySize, DWORD &pcbMaxReplySize, LPARAM lParam)
{
	BOOL lbRc = FALSE;
	CGuiServer* pGSrv = (CGuiServer*)lParam;

	
	gpSetCls->debugLogCommand(pIn, TRUE, timeGetTime(), 0, pGSrv->ms_ServerPipe);

	#ifdef _DEBUG
	UINT nDataSize = pIn->hdr.cbSize - sizeof(CESERVER_REQ_HDR);
	#endif
	// ��� ������ �� ����� ��������, ������������ ������� � ���������� (���� �����) ���������

	#ifdef ALLOW_WINE_MSG
	if (gbIsWine)
	{
		wchar_t szMsg[128];
		msprintf(szMsg, countof(szMsg), L"CGuiServer::GuiServerCommand.\nGUI TID=%u\nSrcPID=%u, SrcTID=%u, Cmd=%u",
			GetCurrentThreadId(), pIn->hdr.nSrcPID, pIn->hdr.nSrcThreadId, pIn->hdr.nCmd);
		MessageBox(szMsg, MB_ICONINFORMATION);
	}
	#endif

	switch (pIn->hdr.nCmd)
	{
		case CECMD_NEWCMD:
		{
			// �������� �� ������ ����� ConEmu.exe, ����� ��� �������� � ������ /single, /showhide, /showhideTSA
			DEBUGSTR(L"GUI recieved CECMD_NEWCMD\n");

			//if (gpConEmu->isIconic())
			//	SendMessage(ghWnd, WM_SYSCOMMAND, SC_RESTORE, 0);
			//apiSetForegroundWindow(ghWnd);

			gpConEmu->OnMinimizeRestore((pIn->NewCmd.ShowHide == sih_None) ? sih_SetForeground : pIn->NewCmd.ShowHide);

			// ����� ���� �����
			if ((pIn->NewCmd.ShowHide == sih_None) && pIn->NewCmd.szCommand[0])
			{
				RConStartArgs *pArgs = new RConStartArgs;
				pArgs->pszSpecialCmd = lstrdup(pIn->NewCmd.szCommand);
				if (pIn->NewCmd.szCurDir[0] == 0)
				{
					_ASSERTE(pIn->NewCmd.szCurDir[0] != 0);
				}
				else
				{
					pArgs->pszStartupDir = lstrdup(pIn->NewCmd.szCurDir);
				}

				gpConEmu->PostCreateCon(pArgs);
			}
			else
			{
				_ASSERTE(pIn->NewCmd.ShowHide==sih_ShowMinimize || pIn->NewCmd.ShowHide==sih_ShowHideTSA || pIn->NewCmd.ShowHide==sih_Show);
			}

			pcbReplySize = sizeof(CESERVER_REQ_HDR)+sizeof(BYTE);
			lbRc = ExecuteNewCmd(ppReply, pcbMaxReplySize, pIn->hdr.nCmd, pcbReplySize);
			if (lbRc)
			{
				ppReply->Data[0] = TRUE;
			}

			break;
		} //CECMD_NEWCMD

		case CECMD_TABSCMD:
		{
			// 0: ��������/�������� ����, 1: ������� �� ���������, 2: ������� �� ����������, 3: commit switch
			DEBUGSTR(L"GUI recieved CECMD_TABSCMD\n");
			_ASSERTE(nDataSize>=1);
			DWORD nTabCmd = pIn->Data[0];
			gpConEmu->TabCommand((ConEmuTabCommand)nTabCmd);

			pcbReplySize = sizeof(CESERVER_REQ_HDR)+sizeof(BYTE);
			if (ExecuteNewCmd(ppReply, pcbMaxReplySize, pIn->hdr.nCmd, pcbReplySize))
			{
				lbRc = TRUE;
				ppReply->Data[0] = TRUE;
			}
			break;
		} // CECMD_TABSCMD

		#if 0
		case CECMD_GETALLTABS:
		{
			int nConCount = gpConEmu->GetConCount();
			int nActiveCon = gpConEmu->ActiveConNum();
			size_t cchMax = nConCount*16;
			size_t cchCount = 0;
			CVirtualConsole* pVCon;
			CESERVER_REQ_GETALLTABS::TabInfo* pTabs = (CESERVER_REQ_GETALLTABS::TabInfo*)calloc(cchMax, sizeof(*pTabs));
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
				wchar_t szModified[4];
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

					pTabs[cchCount].ActiveConsole == (V == nActiveCon);
					pTabs[cchCount].ActiveTab == (tab.Current != 0);
					pTabs[cchCount].Disabled = ((tab.Type & fwt_Disabled) == fwt_Disabled);
					pTabs[cchCount].ConsoleIdx = V;
					pTabs[cchCount].TabIdx = T;

					// Text
					wcscpy_c(szModified, tab.Modified ? L" * " : L"   ");
					if (V == nActiveCon)
					{
						if (T < 9)
							_wsprintf(pTabs[cchCount].Title, SKIPLEN(countof(pTabs[cchCount].Title)) L"[%i/&%i]%s", V+1, T+1, szModified);
						else if (T == 9)
							_wsprintf(pTabs[cchCount].Title, SKIPLEN(countof(pTabs[cchCount].Title)) L"[%i/1&0]%s", V+1, szModified);
						else
							_wsprintf(pTabs[cchCount].Title, SKIPLEN(countof(pTabs[cchCount].Title)) L"[%i/%i]%s", V+1, T+1, szModified);
					}
					else
					{
						_wsprintf(pTabs[cchCount].Title, SKIPLEN(countof(pTabs[cchCount].Title)) L"[%i/%i]%s", V+1, T+1, szModified);
					}

					cchCount++;
				}
			}
			if (cchCount && pTabs)
			{
				pcbReplySize = sizeof(CESERVER_REQ_HDR)+sizeof(CESERVER_REQ_GETALLTABS)+((cchCount-1)*sizeof(CESERVER_REQ_GETALLTABS::TabInfo));
				if (ExecuteNewCmd(ppReply, pcbMaxReplySize, pIn->hdr.nCmd, pcbReplySize))
				{
					lbRc = TRUE;
					ppReply->GetAllTabs.Count = cchCount;
					memmove(ppReply->GetAllTabs.Tabs, pTabs, cchCount*sizeof(*pTabs));
				}
			}
			SafeFree(pTabs);
			break;
		} // CECMD_GETALLTABS

		case CECMD_ACTIVATETAB:
		{
			BOOL lbTabOk = FALSE;
			CVirtualConsole *pVCon = gpConEmu->GetVCon(pIn->dwData[0]);
			if (pVCon && pVCon->RCon())
			{
				lbTabOk = pVCon->RCon()->ActivateFarWindow(pIn->dwData[1]);
			}

			pcbReplySize = sizeof(CESERVER_REQ_HDR)+sizeof(DWORD);
			if (ExecuteNewCmd(ppReply, pcbMaxReplySize, pIn->hdr.nCmd, pcbReplySize))
			{
				lbRc = TRUE;
				ppReply->dwData[0] = lbTabOk;
			}
			break;
		} // CECMD_ACTIVATETAB
		#endif

		case CECMD_ATTACH2GUI:
		{
			// ������� ������ �� Attach �� �������
			pcbReplySize = sizeof(CESERVER_REQ_HDR)+sizeof(CESERVER_REQ_STARTSTOPRET);
			if (!ExecuteNewCmd(ppReply, pcbMaxReplySize, pIn->hdr.nCmd, pcbReplySize))
				goto wrap;
			//CESERVER_REQ* pOut = ExecuteNewCmd(CECMD_ATTACH2GUI, sizeof(CESERVER_REQ_HDR)+sizeof(CESERVER_REQ_STARTSTOPRET));

			gpConEmu->AttachRequested(pIn->StartStop.hWnd, &(pIn->StartStop), &(ppReply->StartStopRet));
			lbRc = TRUE;
			//ExecuteFreeResult(pOut);
			break;
		} // CECMD_ATTACH2GUI

		case CECMD_SRVSTARTSTOP:
		{
			pcbReplySize = sizeof(CESERVER_REQ_HDR)+sizeof(CESERVER_REQ_STARTSTOPRET);
			if (!ExecuteNewCmd(ppReply, pcbMaxReplySize, pIn->hdr.nCmd, pcbReplySize))
				goto wrap;

			if (pIn->dwData[0] == 1)
			{
				// ������� ������� �������
				HWND hConWnd = (HWND)pIn->dwData[1];
				_ASSERTE(hConWnd && IsWindow(hConWnd));

				DWORD nStartTick = timeGetTime();

				//LRESULT l = 0;
				//DWORD_PTR dwRc = 0;
				//2010-05-21 ��������� ��� �������� - ����� ����� �� �����, ���� ����� ���� DeadLock?
				//l = SendMessageTimeout(ghWnd, gpConEmu->mn_MsgSrvStarted, (WPARAM)hConWnd, pIn->hdr.nSrcPID,
				//	SMTO_BLOCK, 5000, &dwRc);
				//111002 - ������� ������ HWND ���� ��������� (�������� ���� ConEmu)
				MsgSrvStartedArg arg = {hConWnd, pIn->hdr.nSrcPID, nStartTick};
				HWND hWndDC = (HWND)SendMessage(ghWnd, gpConEmu->mn_MsgSrvStarted, 0, (LPARAM)&arg);
				_ASSERTE(hWndDC!=NULL);

				#ifdef _DEBUG
				DWORD dwErr = GetLastError(), nEndTick = timeGetTime(), nDelta = nEndTick - nStartTick;
				if (hWndDC && nDelta >= EXECUTE_CMD_WARN_TIMEOUT)
				{
					if (!IsDebuggerPresent())
					{
						//_ASSERTE(nDelta <= EXECUTE_CMD_WARN_TIMEOUT || (pIn->hdr.nCmd == CECMD_CMDSTARTSTOP && nDelta <= EXECUTE_CMD_WARN_TIMEOUT2));
						_ASSERTEX(nDelta <= EXECUTE_CMD_WARN_TIMEOUT);
					}
				}
				#endif

				//pIn->dwData[0] = (DWORD)ghWnd; //-V205
				//pIn->dwData[1] = (DWORD)dwRc; //-V205
				//pIn->dwData[0] = (l == 0) ? 0 : 1;
				ppReply->StartStopRet.hWnd = ghWnd;
				ppReply->StartStopRet.hWndDC = hWndDC;
				ppReply->StartStopRet.dwPID = GetCurrentProcessId();
			}
			else if (pIn->dwData[0] == 101)
			{
				// ������� ������� �����������
				CRealConsole* pRCon = NULL;
				CVConGuard VCon;

				for (size_t i = 0;; i++)
				{
					if (!CVConGroup::GetVCon(i, &VCon))
						break;

					pRCon = VCon->RCon();
					if (pRCon && (pRCon->GetServerPID(true) == pIn->hdr.nSrcPID || pRCon->GetServerPID(false) == pIn->hdr.nSrcPID))
					{
						break;
					}
					pRCon = NULL;
				}

				if (pRCon)
					pRCon->OnServerClosing(pIn->hdr.nSrcPID);

				//pIn->dwData[0] = 1;
			}
			else
			{
				_ASSERTE((pIn->dwData[0] == 1) || (pIn->dwData[0] == 101));
			}

			lbRc = TRUE;
			//// ����������
			//fSuccess = WriteFile(
			//               hPipe,        // handle to pipe
			//               pOut,         // buffer to write from
			//               pOut->hdr.cbSize,  // number of bytes to write
			//               &cbWritten,   // number of bytes written
			//               NULL);        // not overlapped I/O

			//ExecuteFreeResult(pOut);
			break;
		} // CECMD_SRVSTARTSTOP

		case CECMD_ASSERT:
		{
			DWORD nBtn = MessageBox(NULL, pIn->AssertInfo.szDebugInfo, pIn->AssertInfo.szTitle, pIn->AssertInfo.nBtns);

			pcbReplySize = sizeof(CESERVER_REQ_HDR)+sizeof(DWORD);
			if (ExecuteNewCmd(ppReply, pcbMaxReplySize, pIn->hdr.nCmd, pcbReplySize))
			{
				lbRc = TRUE;
				ppReply->dwData[0] = nBtn;
			}

			//ExecutePrepareCmd(&pIn->hdr, CECMD_ASSERT, sizeof(CESERVER_REQ_HDR) + sizeof(DWORD));
			//pIn->dwData[0] = nBtn;
			//// ����������
			//fSuccess = WriteFile(
			//               hPipe,        // handle to pipe
			//               pIn,         // buffer to write from
			//               pIn->hdr.cbSize,  // number of bytes to write
			//               &cbWritten,   // number of bytes written
			//               NULL);        // not overlapped I/O
			break;
		} // CECMD_ASSERT

		case CECMD_ATTACHGUIAPP:
		{
			pcbReplySize = sizeof(CESERVER_REQ_HDR)+sizeof(CESERVER_REQ_ATTACHGUIAPP);
			if (!ExecuteNewCmd(ppReply, pcbMaxReplySize, pIn->hdr.nCmd, pcbReplySize))
				goto wrap;
			ppReply->AttachGuiApp = pIn->AttachGuiApp;

			//CESERVER_REQ Out;
			//ExecutePrepareCmd(&Out.hdr, CECMD_ATTACHGUIAPP, sizeof(CESERVER_REQ_HDR)+sizeof(Out.AttachGuiApp));
			//Out.AttachGuiApp = pIn->AttachGuiApp;
			
			#ifdef SHOW_GUIATTACH_START
			if (pIn->AttachGuiApp.hWindow == NULL)
			{
				wchar_t szDbg[1024];
				_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"AttachGuiApp requested from:\n%s\nPID=%u", pIn->AttachGuiApp.sAppFileName, pIn->AttachGuiApp.nPID);
				//MBoxA(szDbg);
				MessageBox(NULL, szDbg, L"ConEmu", MB_SYSTEMMODAL);
			}
			#endif

			// ��������� ��������� �������
			CRealConsole* pRCon = gpConEmu->AttachRequestedGui(pIn->AttachGuiApp.sAppFileName, pIn->AttachGuiApp.nPID);
			if (pRCon)
			{
				RECT rcPrev = ppReply->AttachGuiApp.rcWindow;
				HWND hView = pRCon->GetView();
				// ������ ������ ���� ��������� �� ����������� ������� ��������� � VCon
				GetWindowRect(hView, &ppReply->AttachGuiApp.rcWindow);
				ppReply->AttachGuiApp.rcWindow.right -= ppReply->AttachGuiApp.rcWindow.left;
				ppReply->AttachGuiApp.rcWindow.bottom -= ppReply->AttachGuiApp.rcWindow.top;
				ppReply->AttachGuiApp.rcWindow.left = ppReply->AttachGuiApp.rcWindow.top = 0;
				//MapWindowPoints(NULL, hView, (LPPOINT)&ppReply->AttachGuiApp.rcWindow, 2);
				pRCon->CorrectGuiChildRect(ppReply->AttachGuiApp.nStyle, ppReply->AttachGuiApp.nStyleEx, ppReply->AttachGuiApp.rcWindow);
				
				// ��������� RCon � ConEmuC, ��� ��� ����������
				// ���������� ��� ����. ������ (��� ������� exe) ahGuiWnd==NULL, ������ - ����� ������������ �������� ����
				pRCon->SetGuiMode(pIn->AttachGuiApp.nFlags, pIn->AttachGuiApp.hAppWindow, pIn->AttachGuiApp.nStyle, pIn->AttachGuiApp.nStyleEx, pIn->AttachGuiApp.sAppFileName, pIn->AttachGuiApp.nPID, rcPrev);

				ppReply->AttachGuiApp.nFlags = agaf_Success;
				ppReply->AttachGuiApp.nPID = pRCon->GetServerPID();
				ppReply->AttachGuiApp.hConEmuWndDC = pRCon->GetView();
				ppReply->AttachGuiApp.hConEmuWnd = ghWnd;
				ppReply->AttachGuiApp.hAppWindow = pIn->AttachGuiApp.hAppWindow;
				ppReply->AttachGuiApp.hSrvConWnd = pRCon->ConWnd();
				ppReply->AttachGuiApp.hkl = (DWORD)(LONG)(LONG_PTR)GetKeyboardLayout(gpConEmu->mn_MainThreadId);
			}
			else
			{
				ppReply->AttachGuiApp.nFlags = agaf_Fail;
			}

			lbRc = TRUE;

			//// ����������
			//fSuccess = WriteFile(
			//               hPipe,        // handle to pipe
			//               &Out,         // buffer to write from
			//               Out.hdr.cbSize,  // number of bytes to write
			//               &cbWritten,   // number of bytes written
			//               NULL);        // not overlapped I/O
			break;
		} // CECMD_ATTACHGUIAPP
	}

	//// ���������� ������
	//if (pIn && (LPVOID)pIn != (LPVOID)&in)
	//{
	//	free(pIn); pIn = NULL;
	//}
wrap:
	return lbRc;
}

void CGuiServer::GuiServerFree(CESERVER_REQ* pReply, LPARAM lParam)
{
	ExecuteFreeResult(pReply);
}

#ifdef _DEBUG
BOOL CGuiServer::OnGuiServerConnected(LPVOID pInst, LPARAM lParam)
{
	#ifdef ALLOW_WINE_MSG
	if (gbIsWine)
	{
		wchar_t szMsg[128];
		msprintf(szMsg, countof(szMsg), L"CGuiServer::OnGuiServerConnected.\nGUI TID=%u", GetCurrentThreadId());
		MessageBox(szMsg, MB_ICONINFORMATION);
	}
	#endif
	return TRUE;
}
#endif
