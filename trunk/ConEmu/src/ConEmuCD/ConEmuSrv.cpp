
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

#define SHOWDEBUGSTR

#undef TEST_REFRESH_DELAYED

#include "ConEmuC.h"
#include "../common/ConsoleAnnotation.h"
#include "../common/Execute.h"
#include "../common/MStrSafe.h"
#include "TokenHelper.h"
#include "SrvPipes.h"
#include "Queue.h"

//#define DEBUGSTRINPUTEVENT(s) //DEBUGSTR(s) // SetEvent(gpSrv->hInputEvent)
//#define DEBUGLOGINPUT(s) DEBUGSTR(s) // ConEmuC.MouseEvent(X=
//#define DEBUGSTRINPUTWRITE(s) DEBUGSTR(s) // *** ConEmuC.MouseEvent(X=
//#define DEBUGSTRINPUTWRITEALL(s) //DEBUGSTR(s) // *** WriteConsoleInput(Write=
//#define DEBUGSTRINPUTWRITEFAIL(s) DEBUGSTR(s) // ### WriteConsoleInput(Write=
//#define DEBUGSTRCHANGES(s) DEBUGSTR(s)

#define MAX_EVENTS_PACK 20

#ifdef _DEBUG
//#define ASSERT_UNWANTED_SIZE
#else
#undef ASSERT_UNWANTED_SIZE
#endif

extern BOOL gbTerminateOnExit; // ��� ���������
extern BOOL gbTerminateOnCtrlBreak;
extern OSVERSIONINFO gOSVer;

//BOOL ProcessInputMessage(MSG64 &msg, INPUT_RECORD &r);
//BOOL SendConsoleEvent(INPUT_RECORD* pr, UINT nCount);
//BOOL ReadInputQueue(INPUT_RECORD *prs, DWORD *pCount);
//BOOL WriteInputQueue(const INPUT_RECORD *pr);
//BOOL IsInputQueueEmpty();
//BOOL WaitConsoleReady(BOOL abReqEmpty); // ���������, ���� ���������� ����� ����� ������� ������� �����. ���������� FALSE, ���� ������ �����������!
//DWORD WINAPI InputThread(LPVOID lpvParam);
int CreateColorerHeader();


// ���������� ������ �����, ����� ����� ���� ���������� ���������� ������� GUI ����
void ServerInitFont()
{
	// ������ ������ � Lucida. ����������� ��� ���������� ������.
	if (gpSrv->szConsoleFont[0])
	{
		// ��������� ��������� ������� ������ ������!
		LOGFONT fnt = {0};
		lstrcpynW(fnt.lfFaceName, gpSrv->szConsoleFont, LF_FACESIZE);
		gpSrv->szConsoleFont[0] = 0; // ����� �������. ���� ����� ���� - ��� ����� ����������� � FontEnumProc
		HDC hdc = GetDC(NULL);
		EnumFontFamiliesEx(hdc, &fnt, (FONTENUMPROCW) FontEnumProc, (LPARAM)&fnt, 0);
		DeleteDC(hdc);
	}

	if (gpSrv->szConsoleFont[0] == 0)
	{
		lstrcpyW(gpSrv->szConsoleFont, L"Lucida Console");
		gpSrv->nConFontWidth = 3; gpSrv->nConFontHeight = 5;
	}

	if (gpSrv->nConFontHeight<5) gpSrv->nConFontHeight = 5;

	if (gpSrv->nConFontWidth==0 && gpSrv->nConFontHeight==0)
	{
		gpSrv->nConFontWidth = 3; gpSrv->nConFontHeight = 5;
	}
	else if (gpSrv->nConFontWidth==0)
	{
		gpSrv->nConFontWidth = gpSrv->nConFontHeight * 2 / 3;
	}
	else if (gpSrv->nConFontHeight==0)
	{
		gpSrv->nConFontHeight = gpSrv->nConFontWidth * 3 / 2;
	}

	if (gpSrv->nConFontHeight<5 || gpSrv->nConFontWidth <3)
	{
		gpSrv->nConFontWidth = 3; gpSrv->nConFontHeight = 5;
	}

	if (gbAttachMode && gbNoCreateProcess && gpSrv->dwRootProcess && gbAttachFromFar)
	{
		// ������ ����� ��� ����� �� Far �������. ��������� ���������� ����� � ������� ����� ������.
		wchar_t szPipeName[128];
		_wsprintf(szPipeName, SKIPLEN(countof(szPipeName)) CEPLUGINPIPENAME, L".", gpSrv->dwRootProcess);
		CESERVER_REQ In;
		ExecutePrepareCmd(&In, CMD_SET_CON_FONT, sizeof(CESERVER_REQ_HDR)+sizeof(CESERVER_REQ_SETFONT));
		In.Font.cbSize = sizeof(In.Font);
		In.Font.inSizeX = gpSrv->nConFontWidth;
		In.Font.inSizeY = gpSrv->nConFontHeight;
		lstrcpy(In.Font.sFontName, gpSrv->szConsoleFont);
		CESERVER_REQ *pPlgOut = ExecuteCmd(szPipeName, &In, 500, ghConWnd);

		if (pPlgOut) ExecuteFreeResult(pPlgOut);
	}
	else if (!gbAlienMode || gOSVer.dwMajorVersion >= 6)
	{
		if (gpLogSize) LogSize(NULL, ":SetConsoleFontSizeTo.before");

		SetConsoleFontSizeTo(ghConWnd, gpSrv->nConFontHeight, gpSrv->nConFontWidth, gpSrv->szConsoleFont, gnDefTextColors, gnDefPopupColors);

		if (gpLogSize) LogSize(NULL, ":SetConsoleFontSizeTo.after");
	}
}

BOOL LoadGuiSettings(ConEmuGuiMapping& GuiMapping)
{
	BOOL lbRc = FALSE;
	if (ghConEmuWnd && IsWindow(ghConEmuWnd))
	{
		DWORD dwGuiThreadId, dwGuiProcessId;
		MFileMapping<ConEmuGuiMapping> GuiInfoMapping;
		dwGuiThreadId = GetWindowThreadProcessId(ghConEmuWnd, &dwGuiProcessId);

		if (!dwGuiThreadId)
		{
			_ASSERTE(dwGuiProcessId);
		}
		else
		{
			GuiInfoMapping.InitName(CEGUIINFOMAPNAME, dwGuiProcessId);
			const ConEmuGuiMapping* pInfo = GuiInfoMapping.Open();

			if (pInfo
				&& pInfo->cbSize == sizeof(ConEmuGuiMapping)
				&& pInfo->nProtocolVersion == CESERVER_REQ_VER)
			{
				memmove(&GuiMapping, pInfo, pInfo->cbSize);
				lbRc = TRUE;
			}
		}
	}
	return lbRc;
}

BOOL ReloadGuiSettings(ConEmuGuiMapping* apFromCmd)
{
	BOOL lbRc = FALSE;
	if (apFromCmd)
	{
		DWORD cbSize = min(sizeof(gpSrv->guiSettings), apFromCmd->cbSize);
		memmove(&gpSrv->guiSettings, apFromCmd, cbSize);
		_ASSERTE(cbSize == apFromCmd->cbSize);
		gpSrv->guiSettings.cbSize = cbSize;
		lbRc = TRUE;
	}
	else
	{
		gpSrv->guiSettings.cbSize = sizeof(ConEmuGuiMapping);
		lbRc = LoadGuiSettings(gpSrv->guiSettings);
	}

	if (lbRc)
	{
		gbLogProcess = (gpSrv->guiSettings.nLoggingType == glt_Processes);

		UpdateComspec(&gpSrv->guiSettings.ComSpec);

		SetEnvironmentVariableW(ENV_CONEMUDIR_VAR_W, gpSrv->guiSettings.sConEmuDir);
		SetEnvironmentVariableW(ENV_CONEMUBASEDIR_VAR_W, gpSrv->guiSettings.sConEmuBaseDir);

		// �� ����� ������� ����, ��� ���������� ��������� Gui ��� ����� �������
		// ��������������, ���������� ����������� ���������
		//SetEnvironmentVariableW(L"ConEmuArgs", pInfo->sConEmuArgs);

		wchar_t szHWND[16]; _wsprintf(szHWND, SKIPLEN(countof(szHWND)) L"0x%08X", gpSrv->guiSettings.hGuiWnd.u);
		SetEnvironmentVariableW(ENV_CONEMUHWND_VAR_W, szHWND);

		if (gpSrv->pConsole)
		{
			// !!! Warning !!! ������� �����, ������� � CreateMapHeader() !!!
			
			gpSrv->pConsole->hdr.nLoggingType = gpSrv->guiSettings.nLoggingType;
			gpSrv->pConsole->hdr.bDosBox = gpSrv->guiSettings.bDosBox;
			gpSrv->pConsole->hdr.bUseInjects = gpSrv->guiSettings.bUseInjects;
			gpSrv->pConsole->hdr.bUseTrueColor = gpSrv->guiSettings.bUseTrueColor;
			gpSrv->pConsole->hdr.bProcessAnsi = gpSrv->guiSettings.bProcessAnsi;
			gpSrv->pConsole->hdr.bUseClink = gpSrv->guiSettings.bUseClink;

			// �������� ���� � ConEmu
			wcscpy_c(gpSrv->pConsole->hdr.sConEmuExe, gpSrv->guiSettings.sConEmuExe);
			wcscpy_c(gpSrv->pConsole->hdr.sConEmuBaseDir, gpSrv->guiSettings.sConEmuBaseDir);

			// � ��������� ���������� ����������
			gpSrv->pConsole->hdr.ComSpec = gpSrv->guiSettings.ComSpec;

			// ���������, ����� �� ������ ������
			gpSrv->pConsole->hdr.isHookRegistry = gpSrv->guiSettings.isHookRegistry;
			wcscpy_c(gpSrv->pConsole->hdr.sHiveFileName, gpSrv->guiSettings.sHiveFileName);
			gpSrv->pConsole->hdr.hMountRoot = gpSrv->guiSettings.hMountRoot;
			wcscpy_c(gpSrv->pConsole->hdr.sMountKey, gpSrv->guiSettings.sMountKey);

			// !!! Warning !!! ������� �����, ������� � CreateMapHeader() !!!
			
			UpdateConsoleMapHeader();
		}
		else
		{
			lbRc = FALSE;
		}
	}
	return lbRc;
}

// ���������� ��� ������� �������: (gbNoCreateProcess && (gbAttachMode || gpSrv->bDebuggerActive))
int AttachRootProcess()
{
	DWORD dwErr = 0;

	_ASSERTE((gpSrv->hRootProcess == NULL || gpSrv->hRootProcess == GetCurrentProcess()) && "Must not be opened yet");

	if (!gpSrv->bDebuggerActive && !IsWindowVisible(ghConWnd) && !(gpSrv->dwGuiPID || gbAttachFromFar))
	{
		PRINT_COMSPEC(L"Console windows is not visible. Attach is unavailable. Exiting...\n", 0);
		DisableAutoConfirmExit();
		//gpSrv->nProcessStartTick = GetTickCount() - 2*CHECK_ROOTSTART_TIMEOUT; // ������ nProcessStartTick �� �����. �������� ������ �� �������
		#ifdef _DEBUG
		xf_validate();
		xf_dump_chk();
		#endif
		return CERR_RUNNEWCONSOLE;
	}

	if (gpSrv->dwRootProcess == 0 && !gpSrv->bDebuggerActive)
	{
		// ����� ���������� ���������� PID ��������� ��������.
		// ������������ ����� ���� cmd (comspec, ���������� �� FAR)
		DWORD dwParentPID = 0, dwFarPID = 0;
		DWORD dwServerPID = 0; // ����� � ���� ������� ��� ���� ������?
		_ASSERTE(!gpSrv->bDebuggerActive);

		if (gpSrv->nProcessCount >= 2 && !gpSrv->bDebuggerActive)
		{
			HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);

			if (hSnap != INVALID_HANDLE_VALUE)
			{
				PROCESSENTRY32 prc = {sizeof(PROCESSENTRY32)};

				if (Process32First(hSnap, &prc))
				{
					do
					{
						for(UINT i = 0; i < gpSrv->nProcessCount; i++)
						{
							if (prc.th32ProcessID != gnSelfPID
							        && prc.th32ProcessID == gpSrv->pnProcesses[i])
							{
								if (lstrcmpiW(prc.szExeFile, L"conemuc.exe")==0
								        /*|| lstrcmpiW(prc.szExeFile, L"conemuc64.exe")==0*/)
								{
									CESERVER_REQ* pIn = ExecuteNewCmd(CECMD_ATTACH2GUI, 0);
									CESERVER_REQ* pOut = ExecuteSrvCmd(prc.th32ProcessID, pIn, ghConWnd);

									if (pOut) dwServerPID = prc.th32ProcessID;

									ExecuteFreeResult(pIn); ExecuteFreeResult(pOut);

									// ���� ������� ������� ��������� - �������
									if (dwServerPID)
										break;
								}

								if (!dwFarPID && IsFarExe(prc.szExeFile))
								{
									dwFarPID = prc.th32ProcessID;
								}

								if (!dwParentPID)
									dwParentPID = prc.th32ProcessID;
							}
						}

						// ���� ��� ��������� ������� � ������� - �������, ������� ������ �� �����
						if (dwServerPID)
							break;
					}
					while(Process32Next(hSnap, &prc));
				}

				CloseHandle(hSnap);

				if (dwFarPID) dwParentPID = dwFarPID;
			}
		}

		if (dwServerPID)
		{
			AllowSetForegroundWindow(dwServerPID);
			PRINT_COMSPEC(L"Server was already started. PID=%i. Exiting...\n", dwServerPID);
			DisableAutoConfirmExit(); // ������ ��� ����?
			// ������ nProcessStartTick �� �����. �������� ������ �� �������
			//gpSrv->nProcessStartTick = GetTickCount() - 2*CHECK_ROOTSTART_TIMEOUT;
			#ifdef _DEBUG
			xf_validate();
			xf_dump_chk();
			#endif
			return CERR_RUNNEWCONSOLE;
		}

		if (!dwParentPID)
		{
			_printf("Attach to GUI was requested, but there is no console processes:\n", 0, GetCommandLineW()); //-V576
			_ASSERTE(FALSE);
			return CERR_CARGUMENT;
		}

		// ����� ������� HANDLE ��������� ��������
		gpSrv->hRootProcess = OpenProcess(PROCESS_QUERY_INFORMATION|SYNCHRONIZE, FALSE, dwParentPID);

		if (!gpSrv->hRootProcess)
		{
			dwErr = GetLastError();
			wchar_t* lpMsgBuf = NULL;
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwErr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&lpMsgBuf, 0, NULL);
			_printf("Can't open process (%i) handle, ErrCode=0x%08X, Description:\n", //-V576
			        dwParentPID, dwErr, (lpMsgBuf == NULL) ? L"<Unknown error>" : lpMsgBuf);

			if (lpMsgBuf) LocalFree(lpMsgBuf);
			SetLastError(dwErr);

			return CERR_CREATEPROCESS;
		}

		gpSrv->dwRootProcess = dwParentPID;
		// ��������� ������ ����� ConEmuC ����������!
		wchar_t szSelf[MAX_PATH+1];
		wchar_t szCommand[MAX_PATH+100];

		if (!GetModuleFileName(NULL, szSelf, countof(szSelf)))
		{
			dwErr = GetLastError();
			_printf("GetModuleFileName failed, ErrCode=0x%08X\n", dwErr);
			SetLastError(dwErr);
			return CERR_CREATEPROCESS;
		}

		//if (wcschr(pszSelf, L' '))
		//{
		//	*(--pszSelf) = L'"';
		//	lstrcatW(pszSelf, L"\"");
		//}

		_wsprintf(szCommand, SKIPLEN(countof(szCommand)) L"\"%s\" /ATTACH %s/PID=%u", szSelf, gpSrv->bRequestNewGuiWnd ? L"/GHWND=NEW " : L"", dwParentPID);
		PROCESS_INFORMATION pi; memset(&pi, 0, sizeof(pi));
		STARTUPINFOW si; memset(&si, 0, sizeof(si)); si.cb = sizeof(si);
		PRINT_COMSPEC(L"Starting modeless:\n%s\n", pszSelf);
		// CREATE_NEW_PROCESS_GROUP - ����, ��������� �������� Ctrl-C
		// ��� ������ ������ ������� � ���� �������. � ������ ���� ������� �� �����
		BOOL lbRc = createProcess(TRUE, NULL, szCommand, NULL,NULL, TRUE,
		                           NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi);
		dwErr = GetLastError();

		if (!lbRc)
		{
			PrintExecuteError(szCommand, dwErr);
			SetLastError(dwErr);
			return CERR_CREATEPROCESS;
		}

		//delete psNewCmd; psNewCmd = NULL;
		AllowSetForegroundWindow(pi.dwProcessId);
		PRINT_COMSPEC(L"Modeless server was started. PID=%i. Exiting...\n", pi.dwProcessId);
		SafeCloseHandle(pi.hProcess); SafeCloseHandle(pi.hThread);
		DisableAutoConfirmExit(); // ������ ������� ������ ���������, ����� �� ����������� bat �����
		// ������ nProcessStartTick �� �����. �������� ������ �� �������
		//gpSrv->nProcessStartTick = GetTickCount() - 2*CHECK_ROOTSTART_TIMEOUT;
		#ifdef _DEBUG
		xf_validate();
		xf_dump_chk();
		#endif
		return CERR_RUNNEWCONSOLE;
	}
	else
	{
		int iAttachRc = AttachDebuggingProcess();
		if (iAttachRc != 0)
			return iAttachRc;
	}

	return 0; // OK
}

BOOL ServerInitConsoleMode()
{
	BOOL bConRc = FALSE;

	HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
	DWORD dwFlags = 0;
	bConRc = GetConsoleMode(h, &dwFlags);

	if (!gnConsoleModeFlags)
	{
		// ��������� (�������� /CINMODE= �� ������)
		dwFlags |= (ENABLE_QUICK_EDIT_MODE|ENABLE_EXTENDED_FLAGS|ENABLE_INSERT_MODE);
	}
	else
	{
		DWORD nMask = (gnConsoleModeFlags & 0xFFFF0000) >> 16;
		DWORD nOr   = (gnConsoleModeFlags & 0xFFFF);
		dwFlags &= ~nMask;
		dwFlags |= (nOr | ENABLE_EXTENDED_FLAGS);
	}

	bConRc = SetConsoleMode(h, dwFlags); //-V519

	return bConRc;
}

int ServerInitCheckExisting(bool abAlternative)
{
	int iRc = 0;
	CESERVER_CONSOLE_MAPPING_HDR test = {};

	BOOL lbExist = LoadSrvMapping(ghConWnd, test);
	if (abAlternative == FALSE)
	{
		_ASSERTE(gnRunMode==RM_SERVER);
		// �������� ������! ������� ������� �� ���� ������ ��� ���� �� ������!
		// ��� ������ ���� ������ - ������� ������� ������� ������� � ��� �� �������!
		if (lbExist)
		{
			CESERVER_REQ_HDR In; ExecutePrepareCmd(&In, CECMD_ALIVE, sizeof(CESERVER_REQ_HDR));
			CESERVER_REQ* pOut = ExecuteSrvCmd(test.nServerPID, (CESERVER_REQ*)&In, NULL);
			if (pOut)
			{
				_ASSERTE(test.nServerPID == 0);
				ExecuteFreeResult(pOut);
				wchar_t szErr[127];
				msprintf(szErr, countof(szErr), L"\nServer (PID=%u) already exist in console! Current PID=%u\n", test.nServerPID, GetCurrentProcessId());
				_wprintf(szErr);
				iRc = CERR_SERVER_ALREADY_EXISTS;
				goto wrap;
			}

			// ������ ������ ����, ���������� �����? ����� �����-�� �������������� �������������?
			_ASSERTE(test.nServerPID == 0 && "Server already exists");
		}
	}
	else
	{
		_ASSERTE(gnRunMode==RM_ALTSERVER);
		// �� ����, � ������� ������ ���� _�����_ ������.
		_ASSERTE(lbExist && test.nServerPID != 0);
		if (test.nServerPID == 0)
		{
			iRc = CERR_MAINSRV_NOT_FOUND;
			goto wrap;
		}
		else
		{
			gpSrv->dwMainServerPID = test.nServerPID;
			gpSrv->hMainServer = OpenProcess(SYNCHRONIZE|PROCESS_QUERY_INFORMATION, FALSE, test.nServerPID);
		}
	}

wrap:
	return iRc;
}

int ServerInitConsoleSize()
{
	if ((gbParmVisibleSize || gbParmBufSize) && gcrVisibleSize.X && gcrVisibleSize.Y)
	{
		SMALL_RECT rc = {0};
		SetConsoleSize(gnBufferHeight, gcrVisibleSize, rc, ":ServerInit.SetFromArg"); // ����� ����������? ���� ����� ��� �������
	}
	else
	{
		HANDLE hOut = (HANDLE)ghConOut;
		CONSOLE_SCREEN_BUFFER_INFO lsbi = {{0,0}}; // ���������� �������� ��������� ���

		if (GetConsoleScreenBufferInfo(hOut, &lsbi))
		{
			gpSrv->crReqSizeNewSize = lsbi.dwSize;
			_ASSERTE(gpSrv->crReqSizeNewSize.X!=0);
			gcrVisibleSize.X = lsbi.dwSize.X;

			if (lsbi.dwSize.Y > lsbi.dwMaximumWindowSize.Y)
			{
				// �������� �����
				gcrVisibleSize.Y = (lsbi.srWindow.Bottom - lsbi.srWindow.Top + 1);
				gnBufferHeight = lsbi.dwSize.Y;
			}
			else
			{
				// ����� ��� ���������!
				gcrVisibleSize.Y = lsbi.dwSize.Y;
				gnBufferHeight = 0;
			}
		}
	}

	return 0;
}

int ServerInitAttach2Gui()
{
	int iRc = 0;

	// ���� Refresh �� ������ ���� ��������, ����� � ������� ����� ������� ������ �� �������
	// �� ����, ��� ���������� ������ (��� ������, ������� ������ ���������� GUI ��� ������)
	_ASSERTE(gpSrv->dwRefreshThread==0);
	HWND hDcWnd = NULL;

	while (true)
	{
		hDcWnd = Attach2Gui(ATTACH2GUI_TIMEOUT);

		if (hDcWnd)
			break; // OK

		if (MessageBox(NULL, L"Available ConEmu GUI window not found!", L"ConEmu",
		              MB_RETRYCANCEL|MB_SYSTEMMODAL|MB_ICONQUESTION) != IDRETRY)
			break; // �����
	}

	// 090719 ��������� � ������� ��� ������ ������. ����� �������� � GUI - TID ���� �����
	//// ���� ��� �� ����� ������� (-new_console) � �� /ATTACH ��� ������������ �������
	//if (!gbNoCreateProcess)
	//	SendStarted();

	if (!hDcWnd)
	{
		//_printf("Available ConEmu GUI window not found!\n"); -- �� ����� ������ � �������
		gbAlwaysConfirmExit = TRUE; gbInShutdown = TRUE;
		iRc = CERR_ATTACHFAILED; goto wrap;
	}

wrap:
	return iRc;
}

// ������� ConEmu, ����� �� ����� HWND ���� ���������
// (!gbAttachMode && !gpSrv->bDebuggerActive)
int ServerInitGuiTab()
{
	int iRc = 0;
	DWORD dwRead = 0, dwErr = 0; BOOL lbCallRc = FALSE;
	HWND hConEmuWnd = FindConEmuByPID();

	if (hConEmuWnd == NULL)
	{
		if (gnRunMode == RM_SERVER || gnRunMode == RM_ALTSERVER)
		{
			// ���� ����������� ������ - �� �� ������ ����� ����� ���� ConEmu � ������� ��� ������
			_ASSERTEX((hConEmuWnd!=NULL));
			return CERR_ATTACH_NO_GUIWND;
		}
		else
		{
			_ASSERTEX(gnRunMode == RM_SERVER || gnRunMode == RM_ALTSERVER);
		}
	}
	else
	{
		//UINT nMsgSrvStarted = RegisterWindowMessage(CONEMUMSG_SRVSTARTED);
		//DWORD_PTR nRc = 0;
		//SendMessageTimeout(hConEmuWnd, nMsgSrvStarted, (WPARAM)ghConWnd, gnSelfPID,
		//	SMTO_BLOCK, 500, &nRc);
		_ASSERTE(ghConWnd!=NULL);
		wchar_t szServerPipe[MAX_PATH];
		_wsprintf(szServerPipe, SKIPLEN(countof(szServerPipe)) CEGUIPIPENAME, L".", (DWORD)hConEmuWnd); //-V205
		CESERVER_REQ In, Out;
		ExecutePrepareCmd(&In, CECMD_SRVSTARTSTOP, sizeof(CESERVER_REQ_HDR)+sizeof(DWORD)*2);
		In.dwData[0] = 1; // ������� ������
		In.dwData[1] = (DWORD)ghConWnd; //-V205

		DWORD nInitTick = GetTickCount();

		while (true)
		{
			#ifdef _DEBUG
			DWORD nStartTick = GetTickCount();
			#endif

			lbCallRc = CallNamedPipe(szServerPipe, &In, In.hdr.cbSize, &Out, sizeof(Out), &dwRead, EXECUTE_CONNECT_GUI_CALL_TIMEOUT);

			DWORD dwErr = GetLastError(), nEndTick = GetTickCount();
			DWORD nConnectDelta = nEndTick - nInitTick;

			#ifdef _DEBUG
			DWORD nDelta = nEndTick - nStartTick;
			if (lbCallRc && (nDelta >= EXECUTE_CMD_WARN_TIMEOUT))
			{
				if (!IsDebuggerPresent())
				{
					//_ASSERTE(nDelta <= EXECUTE_CMD_WARN_TIMEOUT || (pIn->hdr.nCmd == CECMD_CMDSTARTSTOP && nDelta <= EXECUTE_CMD_WARN_TIMEOUT2));
					_ASSERTEX(nDelta <= EXECUTE_CMD_WARN_TIMEOUT);
				}
			}
			#endif

			if (lbCallRc || (nConnectDelta > EXECUTE_CONNECT_GUI_TIMEOUT))
				break;

			if (!lbCallRc)
			{
				_ASSERTE(lbCallRc && (dwErr==ERROR_FILE_NOT_FOUND) && "GUI was not initialized yet?");
				Sleep(250);
			}

			UNREFERENCED_PARAMETER(dwErr);
		}


		if (!lbCallRc || !Out.StartStopRet.hWndDc || !Out.StartStopRet.hWndBack)
		{
			dwErr = GetLastError();
			#ifdef _DEBUG
			if (gbIsWine)
			{
				wchar_t szDbgMsg[512], szTitle[128];
				GetModuleFileName(NULL, szDbgMsg, countof(szDbgMsg));
				msprintf(szTitle, countof(szTitle), L"%s: PID=%u", PointToName(szDbgMsg), gnSelfPID);
				msprintf(szDbgMsg, countof(szDbgMsg), L"CallNamedPipe(%s)\nFailed, code=%u, RC=%u, hWndDC=x%08X, hWndBack=x%08X", szServerPipe, dwErr, lbCallRc, (DWORD)Out.StartStopRet.hWndDc, (DWORD)Out.StartStopRet.hWndBack);
				MessageBox(NULL, szDbgMsg, szTitle, MB_SYSTEMMODAL);
			}
			else
			{
				_ASSERTEX(lbCallRc && Out.StartStopRet.hWndDc && Out.StartStopRet.hWndBack);
			}
			SetLastError(dwErr);
			#endif
		}
		else
		{
			ghConEmuWnd = Out.StartStop.hWnd;
			SetConEmuWindows(Out.StartStopRet.hWndDc, Out.StartStopRet.hWndBack);
			gpSrv->dwGuiPID = Out.StartStopRet.dwPID;
			#ifdef _DEBUG
			DWORD nGuiPID; GetWindowThreadProcessId(ghConEmuWnd, &nGuiPID);
			_ASSERTEX(Out.hdr.nSrcPID==nGuiPID);
			#endif
			gpSrv->bWasDetached = FALSE;
			UpdateConsoleMapHeader();
		}
	}

	DWORD nWaitRc = 99;
	if (gpSrv->hConEmuGuiAttached)
	{
		DEBUGTEST(DWORD t1 = GetTickCount());

		nWaitRc = WaitForSingleObject(gpSrv->hConEmuGuiAttached, 500);

		#ifdef _DEBUG
		DWORD t2 = GetTickCount(), tDur = t2-t1;
		if (tDur > GUIATTACHEVENT_TIMEOUT)
		{
			_ASSERTE((tDur <= GUIATTACHEVENT_TIMEOUT) && "GUI tab creation take more than 250ms");
		}
		#endif
	}

	CheckConEmuHwnd();

	return iRc;
}

bool AltServerWasStarted(DWORD nPID, HANDLE hAltServer, bool ForceThaw)
{
	_ASSERTE(nPID!=0);

	if (hAltServer == NULL)
	{
		hAltServer = OpenProcess(MY_PROCESS_ALL_ACCESS, FALSE, nPID);
		if (hAltServer == NULL)
		{
			hAltServer = OpenProcess(SYNCHRONIZE|PROCESS_QUERY_INFORMATION, FALSE, nPID);
			if (hAltServer == NULL)
			{
				return false;
			}
		}
	}

	if (gpSrv->dwAltServerPID && (gpSrv->dwAltServerPID != nPID))
	{
		// ���������� ������ (�������) ������
		CESERVER_REQ* pFreezeIn = ExecuteNewCmd(CECMD_FREEZEALTSRV, sizeof(CESERVER_REQ_HDR)+2*sizeof(DWORD));
		if (pFreezeIn)
		{
			pFreezeIn->dwData[0] = 1;
			pFreezeIn->dwData[1] = nPID;
			CESERVER_REQ* pFreezeOut = ExecuteSrvCmd(gpSrv->dwAltServerPID, pFreezeIn, ghConWnd);
			ExecuteFreeResult(pFreezeIn);
			ExecuteFreeResult(pFreezeOut);
		}

		// ���� ��� nPID �� ���� �������� "�����������" ����.�������
		if (!gpSrv->AltServers.Get(nPID, NULL))
		{
			// ����� ��������� ��������� ����� ����������� (����� ���� � ������)
			AltServerInfo info = {nPID};
			info.hPrev = gpSrv->hAltServer; // may be NULL
			info.nPrevPID = gpSrv->dwAltServerPID; // may be 0
			gpSrv->AltServers.Set(nPID, info);
		}
	}


	// ��������� ���� �������� � ����� �������� ���������� AltServer, ���������������� gpSrv->dwAltServerPID, gpSrv->hAltServer

	//if (gpSrv->hAltServer && (gpSrv->hAltServer != hAltServer))
	//{
	//	gpSrv->dwAltServerPID = 0;
	//	SafeCloseHandle(gpSrv->hAltServer);
	//}

	gpSrv->hAltServer = hAltServer;
	gpSrv->dwAltServerPID = nPID;

	if (gpSrv->hAltServerChanged && (GetCurrentThreadId() != gpSrv->dwRefreshThread))
	{
		// � RefreshThread �������� ���� � ��������� (100��), �� ����� �����������
		SetEvent(gpSrv->hAltServerChanged);
	}


	if (ForceThaw)
	{
		// ��������� ����� ������ (������� ������ �������������)
		CESERVER_REQ* pFreezeIn = ExecuteNewCmd(CECMD_FREEZEALTSRV, sizeof(CESERVER_REQ_HDR)+2*sizeof(DWORD));
		if (pFreezeIn)
		{
			pFreezeIn->dwData[0] = 0;
			pFreezeIn->dwData[1] = 0;
			CESERVER_REQ* pFreezeOut = ExecuteSrvCmd(gpSrv->dwAltServerPID, pFreezeIn, ghConWnd);
			ExecuteFreeResult(pFreezeIn);
			ExecuteFreeResult(pFreezeOut);
		}
	}

	return (hAltServer != NULL);
}

DWORD WINAPI SetOemCpProc(LPVOID lpParameter)
{
	UINT nCP = (UINT)(DWORD_PTR)lpParameter;
	SetConsoleCP(nCP);
	SetConsoleOutputCP(nCP);
	return 0;
}


// ������� ����������� ������� � ����
int ServerInit(int anWorkMode/*0-Server,1-AltServer,2-Reserved*/)
{
	int iRc = 0;
	DWORD dwErr = 0;

	if (anWorkMode == 0)
	{
		gpSrv->dwMainServerPID = GetCurrentProcessId();
		gpSrv->hMainServer = GetCurrentProcess();

		_ASSERTE(gnRunMode==RM_SERVER);

		if (gbIsDBCS)
		{
			UINT nOemCP = GetOEMCP();
			UINT nConCP = GetConsoleOutputCP();
			if (nConCP != nOemCP)
			{
				DWORD nTID;
				HANDLE h = CreateThread(NULL, 0, SetOemCpProc, (LPVOID)nOemCP, 0, &nTID);
				if (h && (h != INVALID_HANDLE_VALUE))
				{
					DWORD nWait = WaitForSingleObject(h, 5000);
					if (nWait != WAIT_OBJECT_0)
					{
						_ASSERTE(nWait==WAIT_OBJECT_0 && "SetOemCpProc HUNGS!!!");
						TerminateThread(h, 100);
					}
					CloseHandle(h);
				}
			}
		}
	}


	gpSrv->osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&gpSrv->osv);

	// ������ ����� �� �����, ��� �������� "�������" ������� ����� "������������ ������� �������
	// ������������� ������� �� ��������, ������� ���� ������ � �������� ��������
	//InitializeConsoleInputSemaphore();

	if (gpSrv->osv.dwMajorVersion == 6 && gpSrv->osv.dwMinorVersion == 1)
		gpSrv->bReopenHandleAllowed = FALSE;
	else
		gpSrv->bReopenHandleAllowed = TRUE;

	if (anWorkMode == 0)
	{
		if (!gnConfirmExitParm)
		{
			gbAlwaysConfirmExit = TRUE; gbAutoDisableConfirmExit = TRUE;
		}
	}
	else
	{
		_ASSERTE(anWorkMode==1 && gnRunMode==RM_ALTSERVER);
		// �� ����, �������� ���� �� ������, �� ��������
		_ASSERTE(!gbAlwaysConfirmExit);
		gbAutoDisableConfirmExit = FALSE; gbAlwaysConfirmExit = FALSE;
	}

	// ����� � ������� ����� ������ � ����� ������, ����� ����� ���� �������� � ���������� ������� �������
	if ((anWorkMode == 0) && !gpSrv->bDebuggerActive && !gbNoCreateProcess)
		//&& (!gbNoCreateProcess || (gbAttachMode && gbNoCreateProcess && gpSrv->dwRootProcess))
		//)
	{
		ServerInitFont();

		//bool bMovedBottom = false;

		// Minimized ������ ����� ����������!
		// �� ����� ��� �����, ��������, ���-�� � ������ �������...
		if (IsIconic(ghConWnd))
		{
			//WINDOWPLACEMENT wplGui = {sizeof(wplGui)};
			//// �� ����, HWND ��� ��� ��� ������ ���� �������� (������� ����������)
			//if (gpSrv->hGuiWnd)
			//	GetWindowPlacement(gpSrv->hGuiWnd, &wplGui);
			//SendMessage(ghConWnd, WM_SYSCOMMAND, SC_RESTORE, 0);
			WINDOWPLACEMENT wplCon = {sizeof(wplCon)};
			GetWindowPlacement(ghConWnd, &wplCon);
			//wplCon.showCmd = SW_SHOWNA;
			////RECT rc = {wplGui.rcNormalPosition.left+3,wplGui.rcNormalPosition.top+3,wplCon.rcNormalPosition.right-wplCon.rcNormalPosition.left,wplCon.rcNormalPosition.bottom-wplCon.rcNormalPosition.top};
			//// �.�. ���� ��� ����� �������� "SetWindowPos(ghConWnd, NULL, 0, 0, ..." - ����� ��������� ��������
			//RECT rc = {-30000,-30000,-30000+wplCon.rcNormalPosition.right-wplCon.rcNormalPosition.left,-30000+wplCon.rcNormalPosition.bottom-wplCon.rcNormalPosition.top};
			//wplCon.rcNormalPosition = rc;
			////SetWindowPos(ghConWnd, HWND_BOTTOM, 0, 0, 0,0, SWP_NOSIZE|SWP_NOMOVE);
			//SetWindowPlacement(ghConWnd, &wplCon);
			wplCon.showCmd = SW_RESTORE;
			SetWindowPlacement(ghConWnd, &wplCon);
			//bMovedBottom = true;
		}

		if (!gbVisibleOnStartup && IsWindowVisible(ghConWnd))
		{
			ShowWindow(ghConWnd, SW_HIDE);
			//if (bMovedBottom)
			//{
			//	SetWindowPos(ghConWnd, HWND_TOP, 0, 0, 0,0, SWP_NOSIZE|SWP_NOMOVE);
			//}
		}

		// -- ����� �� ��������� �������� �� ��������� �������� � ����������������� -> {0,0}
		// Issue 274: ���� �������� ������� ��������������� � ��������� �����
		SetWindowPos(ghConWnd, NULL, 0, 0, 0,0, SWP_NOSIZE|SWP_NOZORDER);
	}

	// �� �����, ��������. OnTop ���������� ��������� ������,
	// ��� ������ ������� ����� Ctrl+Win+Alt+Space
	// �� ��� ���� ������� ��� ������, � ��� "/root", �����
	// ���������� ��������� ���� ������� ���� "OnTop"
	if (!gbNoCreateProcess && !gbDebugProcess && !gbIsWine /*&& !gnDefPopupColors*/)
	{
		//if (!gbVisibleOnStartup)
		//	ShowWindow(ghConWnd, SW_HIDE);
		SetWindowPos(ghConWnd, HWND_TOPMOST, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE);
	}
	//if (!gbVisibleOnStartup && IsWindowVisible(ghConWnd))
	//{
	//	ShowWindow(ghConWnd, SW_HIDE);
	//}

	// ����������� ����� ��� �������� ������
	// RM_SERVER - ������� � ������� ������� ���������� �������
	// RM_ALTSERVER - ������ ������� (�� ����� - ����������� �������� ���������� � RM_SERVER)
	_ASSERTE(gnRunMode==RM_SERVER || gnRunMode==RM_ALTSERVER);
	CmdOutputStore(true/*abCreateOnly*/);
	#if 0
	_ASSERTE(gpcsStoredOutput==NULL && gpStoredOutput==NULL);
	if (!gpcsStoredOutput)
	{
		gpcsStoredOutput = new MSection;
	}
	#endif


	// �������� �� ��������� ��������� �����
	if ((anWorkMode == 0) && !gbNoCreateProcess && gbConsoleModeFlags /*&& !(gbParmBufferSize && gnBufferHeight == 0)*/)
	{
		ServerInitConsoleMode();
	}

	//2009-08-27 ������� �����
	if (!gpSrv->hConEmuGuiAttached && (!gbDebugProcess || gpSrv->dwGuiPID || gpSrv->hGuiWnd))
	{
		wchar_t szTempName[MAX_PATH];
		_wsprintf(szTempName, SKIPLEN(countof(szTempName)) CEGUIRCONSTARTED, (DWORD)ghConWnd); //-V205

		gpSrv->hConEmuGuiAttached = CreateEvent(gpLocalSecurity, TRUE, FALSE, szTempName);

		_ASSERTE(gpSrv->hConEmuGuiAttached!=NULL);
		//if (gpSrv->hConEmuGuiAttached) ResetEvent(gpSrv->hConEmuGuiAttached); -- ����. ����� ��� ���� �������/����������� � GUI
	}

	// ���� 10, ����� �� ������������� ������� ��� �� ������� ���������� ("dir /s" � �.�.)
	gpSrv->nMaxFPS = 100;

	#ifdef _DEBUG
	if (ghFarInExecuteEvent)
		SetEvent(ghFarInExecuteEvent);
	#endif

	_ASSERTE(anWorkMode!=2); // ��� StandAloneGui - ���������

	if (ghConEmuWndDC == NULL)
	{
		// � AltServer ������ HWND ��� ������ ���� ��������
		_ASSERTE((anWorkMode==0) || ghConEmuWndDC!=NULL);
	}
	else
	{
		iRc = ServerInitCheckExisting((anWorkMode==1));
		if (iRc != 0)
			goto wrap;
	}

	// ������� MapFile ��� ��������� (�����!!!) � ������ ��� ������ � ���������
	iRc = CreateMapHeader();

	if (iRc != 0)
		goto wrap;

	_ASSERTE((ghConEmuWndDC==NULL) || (gpSrv->pColorerMapping!=NULL));

	gpSrv->csAltSrv = new MSection();
	gpSrv->csProc = new MSection();
	gpSrv->nMainTimerElapse = 10;
	gpSrv->nTopVisibleLine = -1; // ���������� ��������� �� ��������
	// ������������� ���� ������
	_wsprintf(gpSrv->szPipename, SKIPLEN(countof(gpSrv->szPipename)) CESERVERPIPENAME, L".", gnSelfPID);
	_wsprintf(gpSrv->szInputname, SKIPLEN(countof(gpSrv->szInputname)) CESERVERINPUTNAME, L".", gnSelfPID);
	_wsprintf(gpSrv->szQueryname, SKIPLEN(countof(gpSrv->szQueryname)) CESERVERQUERYNAME, L".", gnSelfPID);
	_wsprintf(gpSrv->szGetDataPipe, SKIPLEN(countof(gpSrv->szGetDataPipe)) CESERVERREADNAME, L".", gnSelfPID);
	_wsprintf(gpSrv->szDataReadyEvent, SKIPLEN(countof(gpSrv->szDataReadyEvent)) CEDATAREADYEVENT, gnSelfPID);
	gpSrv->nMaxProcesses = START_MAX_PROCESSES; gpSrv->nProcessCount = 0;
	gpSrv->pnProcesses = (DWORD*)calloc(START_MAX_PROCESSES, sizeof(DWORD));
	gpSrv->pnProcessesGet = (DWORD*)calloc(START_MAX_PROCESSES, sizeof(DWORD));
	gpSrv->pnProcessesCopy = (DWORD*)calloc(START_MAX_PROCESSES, sizeof(DWORD));
	MCHKHEAP;

	if (gpSrv->pnProcesses == NULL || gpSrv->pnProcessesGet == NULL || gpSrv->pnProcessesCopy == NULL)
	{
		_printf("Can't allocate %i DWORDS!\n", gpSrv->nMaxProcesses);
		iRc = CERR_NOTENOUGHMEM1; goto wrap;
	}

	CheckProcessCount(TRUE); // ������� ������� ����

	// � ��������, ��������� ����� ����� ���� ������ �� ����, ����� ����������� � GUI.
	// ������ ���� ��������� � ������� ������ ����� ����, ��������, ��� �� ���������
	// ���������� conemuc.exe, �� �������� ���� ������� ����������.
	_ASSERTE(gpSrv->bDebuggerActive || (gpSrv->nProcessCount<=2) || ((gpSrv->nProcessCount>2) && gbAttachMode && gpSrv->dwRootProcess));
	// ��������� ���� ��������� ������� (����������, ����, � ��.)
	gpSrv->hInputEvent = CreateEvent(NULL,FALSE,FALSE,NULL);

	if (gpSrv->hInputEvent) gpSrv->hInputThread = CreateThread(
		        NULL,              // no security attribute
		        0,                 // default stack size
		        InputThread,       // thread proc
		        NULL,              // thread parameter
		        0,                 // not suspended
		        &gpSrv->dwInputThread);      // returns thread ID

	if (gpSrv->hInputEvent == NULL || gpSrv->hInputThread == NULL)
	{
		dwErr = GetLastError();
		_printf("CreateThread(InputThread) failed, ErrCode=0x%08X\n", dwErr);
		iRc = CERR_CREATEINPUTTHREAD; goto wrap;
	}

	SetThreadPriority(gpSrv->hInputThread, THREAD_PRIORITY_ABOVE_NORMAL);
	
	gpSrv->InputQueue.Initialize(CE_MAX_INPUT_QUEUE_BUFFER, gpSrv->hInputEvent);
	//gpSrv->nMaxInputQueue = CE_MAX_INPUT_QUEUE_BUFFER;
	//gpSrv->pInputQueue = (INPUT_RECORD*)calloc(gpSrv->nMaxInputQueue, sizeof(INPUT_RECORD));
	//gpSrv->pInputQueueEnd = gpSrv->pInputQueue+gpSrv->nMaxInputQueue;
	//gpSrv->pInputQueueWrite = gpSrv->pInputQueue;
	//gpSrv->pInputQueueRead = gpSrv->pInputQueueEnd;

	// ��������� ���� ��������� ������� (����������, ����, � ��.)
	if (!InputServerStart())
	{
		dwErr = GetLastError();
		_printf("CreateThread(InputServerStart) failed, ErrCode=0x%08X\n", dwErr);
		iRc = CERR_CREATEINPUTTHREAD; goto wrap;
	}

	// ���� �������� ����������� �������
	if (!DataServerStart())
	{
		dwErr = GetLastError();
		_printf("CreateThread(DataServerStart) failed, ErrCode=0x%08X\n", dwErr);
		iRc = CERR_CREATEINPUTTHREAD; goto wrap;
	}


	if (!gbAttachMode && !gpSrv->bDebuggerActive)
	{
		iRc = ServerInitGuiTab();
		if (iRc != 0)
			goto wrap;
	}

	// ���� "��������" ������� ������� ������� �� ���� (����� ��� �����)
	// �� ����� � ���� "�����������" (������� HANDLE ��������)
	if (gbNoCreateProcess && (gbAttachMode || (gpSrv->bDebuggerActive && (gpSrv->hRootProcess == NULL))))
	{
		iRc = AttachRootProcess();
		if (iRc != 0)
			goto wrap;
	}


	gpSrv->hAllowInputEvent = CreateEvent(NULL, TRUE, TRUE, NULL);

	if (!gpSrv->hAllowInputEvent) SetEvent(gpSrv->hAllowInputEvent);

	

	_ASSERTE(gpSrv->pConsole!=NULL);
	gpSrv->pConsole->hdr.bConsoleActive = TRUE;
	gpSrv->pConsole->hdr.bThawRefreshThread = TRUE;

	// ���� ������� ��������� (gbParmBufferSize && gcrVisibleSize.X && gcrVisibleSize.Y) - ���������� ������
	// ����� - �������� ������� ������� �� ����������� ����
	ServerInitConsoleSize();

	//// Minimized ������ ����� ����������!
	//if (IsIconic(ghConWnd))
	//{
	//	WINDOWPLACEMENT wplCon = {sizeof(wplCon)};
	//	GetWindowPlacement(ghConWnd, &wplCon);
	//	wplCon.showCmd = SW_RESTORE;
	//	SetWindowPlacement(ghConWnd, &wplCon);
	//}

	// ����� �������� ������� ��������� �������
	ReloadFullConsoleInfo(TRUE);
	
	//
	gpSrv->hRefreshEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	if (!gpSrv->hRefreshEvent)
	{
		dwErr = GetLastError();
		_printf("CreateEvent(hRefreshEvent) failed, ErrCode=0x%08X\n", dwErr);
		iRc = CERR_REFRESHEVENT; goto wrap;
	}

	gpSrv->hRefreshDoneEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	if (!gpSrv->hRefreshDoneEvent)
	{
		dwErr = GetLastError();
		_printf("CreateEvent(hRefreshDoneEvent) failed, ErrCode=0x%08X\n", dwErr);
		iRc = CERR_REFRESHEVENT; goto wrap;
	}

	gpSrv->hDataReadyEvent = CreateEvent(gpLocalSecurity,FALSE,FALSE,gpSrv->szDataReadyEvent);
	if (!gpSrv->hDataReadyEvent)
	{
		dwErr = GetLastError();
		_printf("CreateEvent(hDataReadyEvent) failed, ErrCode=0x%08X\n", dwErr);
		iRc = CERR_REFRESHEVENT; goto wrap;
	}

	// !! Event ����� ��������� � ���������� ����� !!
	gpSrv->hReqSizeChanged = CreateEvent(NULL,TRUE,FALSE,NULL);
	if (!gpSrv->hReqSizeChanged)
	{
		dwErr = GetLastError();
		_printf("CreateEvent(hReqSizeChanged) failed, ErrCode=0x%08X\n", dwErr);
		iRc = CERR_REFRESHEVENT; goto wrap;
	}
	gpSrv->pReqSizeSection = new MSection();

	if ((gnRunMode == RM_SERVER) && gbAttachMode)
	{
		iRc = ServerInitAttach2Gui();
		if (iRc != 0)
			goto wrap;
	}

	// ��������� ���� ���������� �� ��������
	gpSrv->hRefreshThread = CreateThread(NULL, 0, RefreshThread, NULL, 0, &gpSrv->dwRefreshThread);
	if (gpSrv->hRefreshThread == NULL)
	{
		dwErr = GetLastError();
		_printf("CreateThread(RefreshThread) failed, ErrCode=0x%08X\n", dwErr);
		iRc = CERR_CREATEREFRESHTHREAD; goto wrap;
	}

	//#ifdef USE_WINEVENT_SRV
	////gpSrv->nMsgHookEnableDisable = RegisterWindowMessage(L"ConEmuC::HookEnableDisable");
	//// The client thread that calls SetWinEventHook must have a message loop in order to receive events.");
	//gpSrv->hWinEventThread = CreateThread(NULL, 0, WinEventThread, NULL, 0, &gpSrv->dwWinEventThread);
	//if (gpSrv->hWinEventThread == NULL)
	//{
	//	dwErr = GetLastError();
	//	_printf("CreateThread(WinEventThread) failed, ErrCode=0x%08X\n", dwErr);
	//	iRc = CERR_WINEVENTTHREAD; goto wrap;
	//}
	//#endif

	// ��������� ���� ��������� ������
	if (!CmdServerStart())
	{
		dwErr = GetLastError();
		_printf("CreateThread(CmdServerStart) failed, ErrCode=0x%08X\n", dwErr);
		iRc = CERR_CREATESERVERTHREAD; goto wrap;
	}


	// �������� �������, ��� ������� � ������ ������
	gpSrv->pConsole->hdr.bDataReady = TRUE;
	//gpSrv->pConsoleMap->SetFrom(&(gpSrv->pConsole->hdr));
	UpdateConsoleMapHeader();

	if (!gpSrv->bDebuggerActive)
		SendStarted();

	CheckConEmuHwnd();
	
	// �������� ���������� ��������� � ������� ������� (�� ConEmuGuiMapping)
	// �.�. � ������ CreateMapHeader ghConEmu ��� ��� ����������
	ReloadGuiSettings(NULL);

	// ���� �� ������� ������������ GUI ������
	if (gbNoCreateProcess && gbAttachMode && gpSrv->hRootProcessGui)
	{
		// ��� ����� �������, ����� ���������������� ���� ������ �� ������� ConEmu
		CESERVER_REQ* pIn = ExecuteNewCmd(CECMD_ATTACHGUIAPP, sizeof(CESERVER_REQ_HDR)+sizeof(CESERVER_REQ_ATTACHGUIAPP));
		_ASSERTE(((DWORD)gpSrv->hRootProcessGui)!=0xCCCCCCCC);
		_ASSERTE(IsWindow(ghConEmuWnd));
		_ASSERTE(IsWindow(ghConEmuWndDC));
		_ASSERTE(IsWindow(ghConEmuWndBack));
		_ASSERTE(IsWindow(gpSrv->hRootProcessGui));
		pIn->AttachGuiApp.hConEmuWnd = ghConEmuWnd;
		pIn->AttachGuiApp.hConEmuDc = ghConEmuWndDC;
		pIn->AttachGuiApp.hConEmuBack = ghConEmuWndBack;
		pIn->AttachGuiApp.hAppWindow = gpSrv->hRootProcessGui;
		pIn->AttachGuiApp.hSrvConWnd = ghConWnd;
		wchar_t szPipe[MAX_PATH];
		_ASSERTE(gpSrv->dwRootProcess!=0);
		_wsprintf(szPipe, SKIPLEN(countof(szPipe)) CEHOOKSPIPENAME, L".", gpSrv->dwRootProcess);
		CESERVER_REQ* pOut = ExecuteCmd(szPipe, pIn, GUIATTACH_TIMEOUT, ghConWnd);
		if (!pOut 
			|| (pOut->hdr.cbSize < (sizeof(CESERVER_REQ_HDR)+sizeof(DWORD)))
			|| (pOut->dwData[0] != (DWORD)gpSrv->hRootProcessGui))
		{
			iRc = CERR_ATTACH_NO_GUIWND;
		}
		ExecuteFreeResult(pOut);
		ExecuteFreeResult(pIn);
		if (iRc != 0)
			goto wrap;
	}

wrap:
	return iRc;
}

// ��������� ��� ���� � ������� �����������
void ServerDone(int aiRc, bool abReportShutdown /*= false*/)
{
	gbQuit = true;
	gbTerminateOnExit = FALSE;

	#ifdef _DEBUG
	// ���������, �� ������� �� Assert-�� � ������ �������
	MyAssertShutdown();
	#endif

	// �� ������ ������ - �������� �������
	if (ghExitQueryEvent)
	{
		_ASSERTE(gbTerminateOnCtrlBreak==FALSE);
		if (!nExitQueryPlace) nExitQueryPlace = 10+(nExitPlaceStep);

		SetTerminateEvent(ste_ServerDone);
	}

	if (ghQuitEvent) SetEvent(ghQuitEvent);

	if (ghConEmuWnd && IsWindow(ghConEmuWnd))
	{
		if (gpSrv->pConsole && gpSrv->pConsoleMap)
		{
			gpSrv->pConsole->hdr.nServerInShutdown = GetTickCount();
			//gpSrv->pConsoleMap->SetFrom(&(gpSrv->pConsole->hdr));
			UpdateConsoleMapHeader();
		}

		#ifdef _DEBUG
		int nCurProcCount = gpSrv->nProcessCount;
		DWORD nCurProcs[20];
		memmove(nCurProcs, gpSrv->pnProcesses, min(nCurProcCount,20)*sizeof(DWORD));
		_ASSERTE(nCurProcCount <= 1);
		#endif

		wchar_t szServerPipe[MAX_PATH];
		_wsprintf(szServerPipe, SKIPLEN(countof(szServerPipe)) CEGUIPIPENAME, L".", (DWORD)ghConEmuWnd); //-V205
		CESERVER_REQ In, Out; DWORD dwRead = 0;
		ExecutePrepareCmd(&In, CECMD_SRVSTARTSTOP, sizeof(CESERVER_REQ_HDR)+sizeof(DWORD)*2);
		In.dwData[0] = 101;
		In.dwData[1] = (DWORD)ghConWnd; //-V205
		// ������� � GUI �����������, ��� ������ �����������
		CallNamedPipe(szServerPipe, &In, In.hdr.cbSize, &Out, sizeof(Out), &dwRead, 1000);
	}

	// ���������� ��������, ����� ������������ ������� ���� ����������
	if (gpSrv->bDebuggerActive)
	{
		if (pfnDebugActiveProcessStop) pfnDebugActiveProcessStop(gpSrv->dwRootProcess);

		gpSrv->bDebuggerActive = FALSE;
	}



	if (gpSrv->hInputThread)
	{
		// �������� ��������, ���� ���� ���� ����������
		WARNING("�� �����������?");

		if (WaitForSingleObject(gpSrv->hInputThread, 500) != WAIT_OBJECT_0)
		{
			gbTerminateOnExit = gpSrv->bInputTermination = TRUE;
			#ifdef _DEBUG
				// ���������, �� ������� �� Assert-�� � ������ �������
				MyAssertShutdown();
			#endif

			#ifndef __GNUC__
			#pragma warning( push )
			#pragma warning( disable : 6258 )
			#endif
			TerminateThread(gpSrv->hInputThread, 100);    // ��� ��������� �� �����...
			#ifndef __GNUC__
			#pragma warning( pop )
			#endif
		}

		SafeCloseHandle(gpSrv->hInputThread);
		//gpSrv->dwInputThread = 0; -- �� ����� ������� ��, ��� �������
	}

	// ���� ����������� �����
	if (gpSrv)
		gpSrv->InputServer.StopPipeServer();

	//SafeCloseHandle(gpSrv->hInputPipe);
	SafeCloseHandle(gpSrv->hInputEvent);

	if (gpSrv)
		gpSrv->DataServer.StopPipeServer();

	if (gpSrv)
		gpSrv->CmdServer.StopPipeServer();

	if (gpSrv->hRefreshThread)
	{
		if (WaitForSingleObject(gpSrv->hRefreshThread, 250)!=WAIT_OBJECT_0)
		{
			_ASSERT(FALSE);
			gbTerminateOnExit = gpSrv->bRefreshTermination = TRUE;
			#ifdef _DEBUG
				// ���������, �� ������� �� Assert-�� � ������ �������
				MyAssertShutdown();
			#endif

			#ifndef __GNUC__
			#pragma warning( push )
			#pragma warning( disable : 6258 )
			#endif
			TerminateThread(gpSrv->hRefreshThread, 100);
			#ifndef __GNUC__
			#pragma warning( pop )
			#endif
		}

		SafeCloseHandle(gpSrv->hRefreshThread);
	}

	SafeCloseHandle(gpSrv->hAltServerChanged);

	SafeCloseHandle(gpSrv->hRefreshEvent);

	SafeCloseHandle(gpSrv->hRefreshDoneEvent);

	SafeCloseHandle(gpSrv->hDataReadyEvent);

	//if (gpSrv->hChangingSize) {
	//    SafeCloseHandle(gpSrv->hChangingSize);
	//}
	// ��������� ��� ����
	//gpSrv->bWinHookAllow = FALSE; gpSrv->nWinHookMode = 0;
	//HookWinEvents ( -1 );

	SafeDelete(gpSrv->pStoredOutputItem);
	SafeDelete(gpSrv->pStoredOutputHdr);
	#if 0
	{
		MSectionLock CS; CS.Lock(gpcsStoredOutput, TRUE);
		SafeFree(gpStoredOutput);
		CS.Unlock();
		SafeDelete(gpcsStoredOutput);
	}
	#endif

	SafeFree(gpSrv->pszAliases);

	//if (gpSrv->psChars) { free(gpSrv->psChars); gpSrv->psChars = NULL; }
	//if (gpSrv->pnAttrs) { free(gpSrv->pnAttrs); gpSrv->pnAttrs = NULL; }
	//if (gpSrv->ptrLineCmp) { free(gpSrv->ptrLineCmp); gpSrv->ptrLineCmp = NULL; }
	//DeleteCriticalSection(&gpSrv->csConBuf);
	//DeleteCriticalSection(&gpSrv->csChar);
	//DeleteCriticalSection(&gpSrv->csChangeSize);
	SafeCloseHandle(gpSrv->hAllowInputEvent);
	SafeCloseHandle(gpSrv->hRootProcess);
	SafeCloseHandle(gpSrv->hRootThread);

	SafeDelete(gpSrv->csAltSrv);
	SafeDelete(gpSrv->csProc);

	SafeFree(gpSrv->pnProcesses);

	SafeFree(gpSrv->pnProcessesGet);

	SafeFree(gpSrv->pnProcessesCopy);

	//SafeFree(gpSrv->pInputQueue);
	gpSrv->InputQueue.Release();

	CloseMapHeader();

	//SafeCloseHandle(gpSrv->hColorerMapping);
	SafeDelete(gpSrv->pColorerMapping);

	SafeCloseHandle(gpSrv->hReqSizeChanged);
	SafeDelete(gpSrv->pReqSizeSection);
}

// ������� ����� �������, ��� �������� ������� ����� ������������� ���������� �����.
// MAX_CONREAD_SIZE ��������� ����������������
BOOL MyReadConsoleOutput(HANDLE hOut, CHAR_INFO *pData, COORD& bufSize, SMALL_RECT& rgn)
{
	BOOL lbRc = FALSE;
	static bool bDBCS = false, bDBCS_Checked = false;
	if (!bDBCS_Checked)
	{
		bDBCS = (GetSystemMetrics(SM_DBCSENABLED) != 0);
		bDBCS_Checked = true;
	}

	bool   bDBCS_CP = bDBCS;
	LPCSTR szLeads = NULL;
	UINT   MaxCharSize = 0;
	DWORD  nCP = GetConsoleOutputCP();

	if (bDBCS)
	{
		szLeads = GetCpInfoLeads(nCP, &MaxCharSize);
		if (!szLeads || !*szLeads || MaxCharSize < 2)
		{
			bDBCS_CP = false;
		}
	}

	size_t nBufWidth = bufSize.X;
	int nWidth = (rgn.Right - rgn.Left + 1);
	int nHeight = (rgn.Bottom - rgn.Top + 1);
	int nCurSize = nWidth * nHeight;

	_ASSERTE(bufSize.X >= nWidth);
	_ASSERTE(bufSize.Y >= nHeight);
	_ASSERTE(rgn.Top>=0 && rgn.Bottom>=rgn.Top);
	_ASSERTE(rgn.Left>=0 && rgn.Right>=rgn.Left);

	MSectionLock RCS;
	if (gpSrv->pReqSizeSection && !RCS.Lock(gpSrv->pReqSizeSection, TRUE, 30000))
	{
		_ASSERTE(FALSE);
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;

	}

	COORD bufCoord = {0,0};
	DWORD dwErrCode = 0;

	if (!bDBCS_CP && (nCurSize <= MAX_CONREAD_SIZE))
	{
		if (ReadConsoleOutputW(hOut, pData, bufSize, bufCoord, &rgn))
			lbRc = TRUE;
	}

	if (!lbRc)
	{
		// �������� ������ ���������
		
		// ������������ - ����� � �������, �� ������� ����� ���������, � �����
		// ���� ����������, ���� ������� "������". ������� ���������...

		//bufSize.X = TextWidth;
		bufSize.Y = 1;
		bufCoord.X = 0; bufCoord.Y = 0;
		//rgn = gpSrv->sbi.srWindow;

		int Y1 = rgn.Top;
		int Y2 = rgn.Bottom;

		CHAR_INFO* pLine = pData;
		if (!bDBCS_CP)
		{
			for (int y = Y1; y <= Y2; y++, rgn.Top++, pLine+=nBufWidth)
			{
				rgn.Bottom = rgn.Top;
				lbRc = ReadConsoleOutputW(hOut, pLine, bufSize, bufCoord, &rgn);
				if (!lbRc)
				{
					dwErrCode = GetLastError();
					_ASSERTE(FALSE && "ReadConsoleOutputW failed in MyReadConsoleOutput");
					break;
				}
			}
		}
		else
		{
			DWORD nAttrsMax = bufSize.X;
			DWORD nCharsMax = nAttrsMax/* *4 */; // -- �������� ��� ����� �� ��������� CP - 4 �����
			wchar_t* pszChars = (wchar_t*)malloc(nCharsMax*sizeof(*pszChars));
			char* pszCharsA = (char*)malloc(nCharsMax*sizeof(*pszCharsA));
			WORD* pnAttrs = (WORD*)malloc(bufSize.X*sizeof(*pnAttrs));
			if (pszChars && pszCharsA && pnAttrs)
			{
				COORD crRead = {rgn.Left,Y1};
				DWORD nChars, nAttrs, nCharsA;
				CHAR_INFO* pLine = pData;
				for (; crRead.Y <= Y2; crRead.Y++, pLine+=nBufWidth)
				{
					rgn.Bottom = rgn.Top;
					
					lbRc = ReadConsoleOutputCharacterA(hOut, pszCharsA, nCharsMax, crRead, &nCharsA)
						&& ReadConsoleOutputAttribute(hOut, pnAttrs, bufSize.X, crRead, &nAttrs);

					if (!lbRc)
					{
						dwErrCode = GetLastError();
						_ASSERTE(FALSE && "ReadConsoleOutputAttribute failed in MyReadConsoleOutput");

						CHAR_INFO* p = pLine;
						for (size_t i = 0; i < nAttrsMax; ++i, ++p)
						{
							p->Attributes = 4; // red on black
							p->Char.UnicodeChar = 0xFFFE; // not a character
						}

						break;
					}
					else
					{
						nChars = MultiByteToWideChar(nCP, 0, pszCharsA, nCharsA, pszChars, nCharsMax);
						CHAR_INFO* p = pLine;
						wchar_t* psz = pszChars;
						WORD* pn = pnAttrs;
						//int i = nAttrsMax;
						//while ((i--) > 0)
						//{
						//	p->Attributes = *(pn++);
						//	p->Char.UnicodeChar = *(psz++);
						//	p++;
						//}
						size_t x1 = min(nChars,nAttrsMax);
						size_t x2 = nAttrsMax;
						for (size_t i = 0; i < x1; ++i, ++p)
						{
							p->Attributes = *pn;
							p->Char.UnicodeChar = *psz;

							WARNING("�����������! pn ����� ��������� �� ������ ����� DBCS/QBCS");
							pn++; // += MaxCharSize;
							psz++;
						}
						WORD nLastAttr = pnAttrs[max(0,(int)nAttrs-1)];
						for (size_t i = x1; i < x2; ++i, ++p)
						{
							p->Attributes = nLastAttr;
							p->Char.UnicodeChar = L' ';
						}
					}
				}
			}
			SafeFree(pszChars);
			SafeFree(pszCharsA);
			SafeFree(pnAttrs);
		}
	}

	return lbRc;
}

// ������� ����� �������, ��� �������� ������� ����� ������������� ���������� �����.
// MAX_CONREAD_SIZE ��������� ����������������
BOOL MyWriteConsoleOutput(HANDLE hOut, CHAR_INFO *pData, COORD& bufSize, COORD& crBufPos, SMALL_RECT& rgn)
{
	BOOL lbRc = FALSE;

	size_t nBufWidth = bufSize.X;
	int nWidth = (rgn.Right - rgn.Left + 1);
	int nHeight = (rgn.Bottom - rgn.Top + 1);
	int nCurSize = nWidth * nHeight;

	_ASSERTE(bufSize.X >= nWidth);
	_ASSERTE(bufSize.Y >= nHeight);
	_ASSERTE(rgn.Top>=0 && rgn.Bottom>=rgn.Top);
	_ASSERTE(rgn.Left>=0 && rgn.Right>=rgn.Left);

	COORD bufCoord = crBufPos;

	if (nCurSize <= MAX_CONREAD_SIZE)
	{
		if (WriteConsoleOutputW(hOut, pData, bufSize, bufCoord, &rgn))
			lbRc = TRUE;
	}

	if (!lbRc)
	{
		// �������� ������ ���������
		
		// ������������ - ����� � �������, �� ������� ����� ���������, � �����
		// ���� ����������, ���� ������� "������". ������� ���������...

		//bufSize.X = TextWidth;
		bufSize.Y = 1;
		bufCoord.X = 0; bufCoord.Y = 0;
		//rgn = gpSrv->sbi.srWindow;

		int Y1 = rgn.Top;
		int Y2 = rgn.Bottom;

		CHAR_INFO* pLine = pData;
		for (int y = Y1; y <= Y2; y++, rgn.Top++, pLine+=nBufWidth)
		{
			rgn.Bottom = rgn.Top;
			lbRc = WriteConsoleOutputW(hOut, pLine, bufSize, bufCoord, &rgn);
		}
	}

	return lbRc;
}


bool CmdOutputOpenMap(CONSOLE_SCREEN_BUFFER_INFO& lsbi, CESERVER_CONSAVE_MAPHDR*& pHdr, CESERVER_CONSAVE_MAP*& pData)
{
	pHdr = NULL;
	pData = NULL;

	// � Win7 �������� ����������� � ������ �������� - ��������� ���������� ����� ���������!!!
	// � �����, ����� ������ telnet'� ������������!
	if (gpSrv->bReopenHandleAllowed)
	{
		ghConOut.Close();
	}


	// !!! ��� ���������� �������� ��������� ��� � �������,
	//     � �� ����������������� �������� MyGetConsoleScreenBufferInfo
	if (!GetConsoleScreenBufferInfo(ghConOut, &lsbi))
	{
		//CS.RelockExclusive();
		//SafeFree(gpStoredOutput);
		return false; // �� ������ �������� ���������� � �������...
	}


	if (!gpSrv->pStoredOutputHdr)
	{
		gpSrv->pStoredOutputHdr = new MFileMapping<CESERVER_CONSAVE_MAPHDR>;
		gpSrv->pStoredOutputHdr->InitName(CECONOUTPUTNAME, (DWORD)ghConWnd); //-V205
		if (!(pHdr = gpSrv->pStoredOutputHdr->Create()))
		{
			_ASSERTE(FALSE && "Failed to create mapping: CESERVER_CONSAVE_MAPHDR");
			gpSrv->pStoredOutputHdr->CloseMap();
			return false;
		}

		ExecutePrepareCmd(&pHdr->hdr, 0, sizeof(*pHdr));
	}
	else
	{
		if (!(pHdr = gpSrv->pStoredOutputHdr->Ptr()))
		{
			_ASSERTE(FALSE && "Failed to get mapping Ptr: CESERVER_CONSAVE_MAPHDR");
			gpSrv->pStoredOutputHdr->CloseMap();
			return false;
		}
	}

	WARNING("� ��� ��� ����� �� ������ � RefreshThread!!!");
	DEBUGSTR(L"--- CmdOutputStore begin\n");

	//MSectionLock CS; CS.Lock(gpcsStoredOutput, FALSE);



	COORD crMaxSize = MyGetLargestConsoleWindowSize(ghConOut);
	DWORD cchOneBufferSize = lsbi.dwSize.X * lsbi.dwSize.Y; // ������ ��� ������� �������!
	DWORD cchMaxBufferSize = max(pHdr->MaxCellCount,(DWORD)(lsbi.dwSize.Y * lsbi.dwSize.X));


	bool lbNeedRecreate = false; // ��������� ����� ��� �������, ��� �������� ������ (������ � ������ �������)
	bool lbNeedReopen = (gpSrv->pStoredOutputItem == NULL);
	// Warning! ������� ��� ��� ���� ������ ������ ��������.
	if (!pHdr->CurrentIndex || (pHdr->MaxCellCount < cchOneBufferSize))
	{
		pHdr->CurrentIndex++;
		lbNeedRecreate = true;
	}
	int nNewIndex = pHdr->CurrentIndex;

	// ���������, ���� ������� ��� ���������� �����, ����� ��� ����� ����������� - �������� ������ (������ � ������ �������)
	if (!lbNeedRecreate && gpSrv->pStoredOutputItem)
	{
		if (!gpSrv->pStoredOutputItem->IsValid()
			|| (nNewIndex != gpSrv->pStoredOutputItem->Ptr()->CurrentIndex))
		{
			lbNeedReopen = lbNeedRecreate = true;
		}
	}

	if (lbNeedRecreate
		|| (!gpSrv->pStoredOutputItem)
		|| (pHdr->MaxCellCount < cchOneBufferSize))
	{
		if (!gpSrv->pStoredOutputItem)
		{
			gpSrv->pStoredOutputItem = new MFileMapping<CESERVER_CONSAVE_MAP>;
			_ASSERTE(lbNeedReopen);
			lbNeedReopen = true;
		}

		if (!lbNeedRecreate && pHdr->MaxCellCount)
		{
			_ASSERTE(pHdr->MaxCellCount >= cchOneBufferSize);
			cchMaxBufferSize = pHdr->MaxCellCount;
		}

		if (lbNeedReopen || lbNeedRecreate || !gpSrv->pStoredOutputItem->IsValid())
		{
			LPCWSTR pszName = gpSrv->pStoredOutputItem->InitName(CECONOUTPUTITEMNAME, (DWORD)ghConWnd, nNewIndex); //-V205
			DWORD nMaxSize = sizeof(*pData) + cchMaxBufferSize * sizeof(pData->Data[0]);

			if (!(pData = gpSrv->pStoredOutputItem->Create(nMaxSize)))
			{
				_ASSERTE(FALSE && "Failed to create item mapping: CESERVER_CONSAVE_MAP");
				gpSrv->pStoredOutputItem->CloseMap();
				pHdr->sCurrentMap[0] = 0; // �����, ���� ��� ����� ��������
				return false;
			}

			ExecutePrepareCmd(&pData->hdr, 0, nMaxSize);
			// Save current mapping
			pData->CurrentIndex = nNewIndex;
			pData->MaxCellCount = cchMaxBufferSize;
			pHdr->MaxCellCount = cchMaxBufferSize;
			wcscpy_c(pHdr->sCurrentMap, pszName);

			goto wrap;
		}
	}

	if (!(pData = gpSrv->pStoredOutputItem->Ptr()))
	{
		_ASSERTE(FALSE && "Failed to get item mapping Ptr: CESERVER_CONSAVE_MAP");
		gpSrv->pStoredOutputItem->CloseMap();
		return false;
	}

wrap:
	if (!pData || (pData->hdr.nVersion != CESERVER_REQ_VER) || (pData->hdr.cbSize <= sizeof(CESERVER_CONSAVE_MAP)))
	{
		_ASSERTE(pData && (pData->hdr.nVersion == CESERVER_REQ_VER) && (pData->hdr.cbSize > sizeof(CESERVER_CONSAVE_MAP)));
		gpSrv->pStoredOutputItem->CloseMap();
		return false;
	}

	return (pData != NULL);
}

// ��������� ������ ���� ������� � gpStoredOutput
void CmdOutputStore(bool abCreateOnly /*= false*/)
{
	CONSOLE_SCREEN_BUFFER_INFO lsbi = {{0,0}};
	CESERVER_CONSAVE_MAPHDR* pHdr = NULL;
	CESERVER_CONSAVE_MAP* pData = NULL;

	if (!CmdOutputOpenMap(lsbi, pHdr, pData))
		return;

	if (!pHdr || !pData)
	{
		_ASSERTE(pHdr && pData);
		return;
	}

	if (!pHdr->info.dwSize.Y || !abCreateOnly)
		pHdr->info = lsbi;
	if (!pData->info.dwSize.Y || !abCreateOnly)
		pData->info = lsbi;

	if (abCreateOnly)
		return;

	//// ���� ��������� ���������� ������� ���������� ������
	//if (gpStoredOutput)
	//{
	//	if (gpStoredOutput->hdr.cbMaxOneBufferSize < (DWORD)nOneBufferSize)
	//	{
	//		CS.RelockExclusive();
	//		SafeFree(gpStoredOutput);
	//	}
	//}

	//if (gpStoredOutput == NULL)
	//{
	//	CS.RelockExclusive();
	//	// �������� ������: ��������� + ����� ������ (�� �������� ������)
	//	gpStoredOutput = (CESERVER_CONSAVE*)calloc(sizeof(CESERVER_CONSAVE_HDR)+nOneBufferSize,1);
	//	_ASSERTE(gpStoredOutput!=NULL);

	//	if (gpStoredOutput == NULL)
	//		return; // �� ������ �������� ������

	//	gpStoredOutput->hdr.cbMaxOneBufferSize = nOneBufferSize;
	//}

	//// ��������� sbi
	////memmove(&gpStoredOutput->hdr.sbi, &lsbi, sizeof(lsbi));
	//gpStoredOutput->hdr.sbi = lsbi;

	// ������ ������ ������
	COORD BufSize = {lsbi.dwSize.X, lsbi.dwSize.Y};
	SMALL_RECT ReadRect = {0, 0, lsbi.dwSize.X-1, lsbi.dwSize.Y-1};

	// ���������/�������� sbi
	pData->info = lsbi;

	pData->Succeeded = MyReadConsoleOutput(ghConOut, pData->Data, BufSize, ReadRect);

#if 0
	// [Roman Kuzmin]
	// In FAR Manager source code this is mentioned as "fucked method". Yes, it is.
	// Functions ReadConsoleOutput* fail if requested data size exceeds their buffer;
	// MSDN says 64K is max but it does not say how much actually we can request now.
	// Experiments show that this limit is floating and it can be much less than 64K.
	// The solution below is not optimal when a user sets small font and large window,
	// but it is safe and practically optimal, because most of users set larger fonts
	// for large window and ReadConsoleOutput works OK. More optimal solution for all
	// cases is not that difficult to develop but it will be increased complexity and
	// overhead often for nothing, not sure that we really should use it.

	if ((nReadLen > MAX_CONREAD_SIZE)
		!ReadConsoleOutput(ghConOut, pData->Data, BufSize, coord, &nbActuallyRead)
		|| (nbActuallyRead != nReadLen))
	{
		DEBUGSTR(L"--- Full block read failed: read line by line\n");
		wchar_t* ConCharNow = gpStoredOutput->Data;
		nReadLen = lsbi.dwSize.X;

		for(int y = 0; y < (int)lsbi.dwSize.Y; y++, coord.Y++)
		{
			ReadConsoleOutputCharacter(ghConOut, ConCharNow, nReadLen, coord, &nbActuallyRead);
			ConCharNow += lsbi.dwSize.X;
		}
	}
#endif

	DEBUGSTR(L"--- CmdOutputStore end\n");
}

void CmdOutputRestore()
{
	WARNING("����������/�������� CmdOutputRestore");
#if 0
	CONSOLE_SCREEN_BUFFER_INFO lsbi = {{0,0}};
	CESERVER_CONSAVE_MAPHDR* pHdr = NULL;
	CESERVER_CONSAVE_MAP* pData = NULL;
	if (!CmdOutputOpenMap(lsbi, pHdr, pData))
		return;

	if (lsbi.srWindow.Top > 0)
	{
		_ASSERTE(lsbi.srWindow.Top == 0 && "Upper left corner of window expected");
		return;
	}

	if (lsbi.dwSize.Y <= (lsbi.srWindow.Bottom - lsbi.srWindow.Top + 1))
	{
		// There is no scroller in window
		// Nothing to do
		return;
	}

	CHAR_INFO chrFill = {};
	chrFill.Attributes = lsbi.wAttributes;
	chrFill.Char.UnicodeChar = L' ';

	SMALL_RECT rcTop = {0,0, lsbi.dwSize.X-1, lsbi.srWindow.Bottom};
	COORD crMoveTo = {0, lsbi.dwSize.Y - 1 - lsbi.srWindow.Bottom};
	if (!ScrollConsoleScreenBuffer(ghConOut, &rcTop, NULL, crMoveTo, &chrFill))
	{
		return;
	}

	SMALL_RECT rcBottom = {0, crMoveTo.Y, lsbi.srWindow.Right, lsbi.dwSize.Y-1};
	SetConsoleWindowInfo(ghConOut, TRUE, &rcBottom);

	COORD crNewPos = {lsbi.dwCursorPosition.X, lsbi.dwCursorPosition.Y + crMoveTo.Y};
	SetConsoleCursorPosition(ghConOut, crNewPos);

#if 0
	MSectionLock CS; CS.Lock(gpcsStoredOutput, TRUE);
	if (gpStoredOutput)
	{
		
		// ������, ��� ������ ������� ����� ���������� �� ������� ���������� ���������� �������.
		// ������ � ��� � ������� ����� ������� ����� ���������� ������� ����������� ������ (����������� FAR).
		// 1) ���� ������� ����� �������
		// 2) ����������� � ������ ����� ������� (�� ������� ����������� ���������� �������)
		// 3) ���������� ������� �� ���������� ������� (���� �� ������ ��� ����������� ������ ������)
		// 4) ������������ ���������� ����� �������. ������, ��� ��� �����
		//    ��������� ��������� ������� ��� � ������ ������-�� ��� ��������� ������...
	}
#endif

	// ������������ ����� ������� (������������ �����) ����� �������
	// ������, ��� ������ ������� ����� ���������� �� ������� ���������� ���������� �������.
	// ������ � ��� � ������� ����� ������� ����� ���������� ������� ����������� ������ (����������� FAR).
	// 1) ���� ������� ����� �������
	// 2) ����������� � ������ ����� ������� (�� ������� ����������� ���������� �������)
	// 3) ���������� ������� �� ���������� ������� (���� �� ������ ��� ����������� ������ ������)
	// 4) ������������ ���������� ����� �������. ������, ��� ��� �����
	//    ��������� ��������� ������� ��� � ������ ������-�� ��� ��������� ������...


	WARNING("���������� ��������� �� ������, ������� �� ����� �������� � �������");
	// ��-�� ��������� ������� ����� �����, ��������� ������ ����� ������ �����.


	CONSOLE_SCREEN_BUFFER_INFO storedSbi = pData->info;

	SHORT nStoredHeight = min(storedSbi.srWindow.Top,rcBottom.Top);
	if (nStoredHeight < 1)
	{
		// Nothing to restore?
		return;
	}

	COORD crOldBufSize = pData->info.dwSize; // ����� ���� ���� ��� ��� ��� ������� �������!
	SMALL_RECT rcWrite = {0,rcBottom.Top-nStoredHeight,min(crOldBufSize.X,lsbi.dwSize.X)-1,rcBottom.Top-1};
	COORD crBufPos = {0, storedSbi.srWindow.Top-nStoredHeight};

	MyWriteConsoleOutput(ghConOut, pData->Data, crOldBufSize, crBufPos, rcWrite);
#endif
}

static BOOL CALLBACK FindConEmuByPidProc(HWND hwnd, LPARAM lParam)
{
	DWORD dwPID;
	GetWindowThreadProcessId(hwnd, &dwPID);
	if (dwPID == gpSrv->dwGuiPID)
	{
		wchar_t szClass[128];
		if (GetClassName(hwnd, szClass, countof(szClass)))
		{
			if (lstrcmp(szClass, VirtualConsoleClassMain) == 0)
			{
				*(HWND*)lParam = hwnd;
				return FALSE;
			}
		}
	}
	return TRUE;
}

HWND FindConEmuByPID()
{
	HWND hConEmuWnd = NULL;
	DWORD dwGuiThreadId = 0, dwGuiProcessId = 0;

	// � ����������� ������� PID GUI ������� ����� ���������
	if (gpSrv->dwGuiPID == 0)
	{
		// GUI ����� ��� "������" � �������� ��� � ���������, ��� ��� ������� � ����� Snapshoot
		HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);

		if (hSnap != INVALID_HANDLE_VALUE)
		{
			PROCESSENTRY32 prc = {sizeof(PROCESSENTRY32)};

			if (Process32First(hSnap, &prc))
			{
				do
				{
					if (prc.th32ProcessID == gnSelfPID)
					{
						gpSrv->dwGuiPID = prc.th32ParentProcessID;
						break;
					}
				}
				while(Process32Next(hSnap, &prc));
			}

			CloseHandle(hSnap);
		}
	}

	if (gpSrv->dwGuiPID)
	{
		HWND hGui = NULL;

		while ((hGui = FindWindowEx(NULL, hGui, VirtualConsoleClassMain, NULL)) != NULL)
		{
			dwGuiThreadId = GetWindowThreadProcessId(hGui, &dwGuiProcessId);

			if (dwGuiProcessId == gpSrv->dwGuiPID)
			{
				hConEmuWnd = hGui;
				break;
			}
		}

		// ���� "� ���" �� ����� ������ ������ �� ����� - �������
		// ����� ���� �������� ��� �������� ��������
		if (hConEmuWnd == NULL)
		{
			HWND hDesktop = GetDesktopWindow();
			EnumChildWindows(hDesktop, FindConEmuByPidProc, (LPARAM)&hConEmuWnd);
		}
	}

	return hConEmuWnd;
}

void SetConEmuWindows(HWND hDcWnd, HWND hBackWnd)
{
	_ASSERTE(ghConEmuWnd!=NULL || (hDcWnd==NULL && hBackWnd==NULL));
	ghConEmuWndDC = hDcWnd;
	ghConEmuWndBack = hBackWnd;
}

void CheckConEmuHwnd()
{
	WARNING("����������, ��� ������� ����� ������� ��� ������ �������");

	//HWND hWndFore = GetForegroundWindow();
	//HWND hWndFocus = GetFocus();
	DWORD dwGuiThreadId = 0;

	if (gpSrv->bDebuggerActive)
	{
		HWND  hDcWnd = NULL;
		ghConEmuWnd = FindConEmuByPID();

		if (ghConEmuWnd)
		{
			GetWindowThreadProcessId(ghConEmuWnd, &gpSrv->dwGuiPID);
			// ������ ��� ����������
			hDcWnd = FindWindowEx(ghConEmuWnd, NULL, VirtualConsoleClass, NULL);
		}
		else
		{
			hDcWnd = NULL;
		}

		UNREFERENCED_PARAMETER(hDcWnd);

		return;
	}

	if (ghConEmuWnd == NULL)
	{
		SendStarted(); // �� � ���� ��������, � ��������� �������� � ������ ������� �������������
	}

	// GUI ����� ��� "������" � �������� ��� � ���������, ��� ��� ������� � ����� Snapshoot
	if (ghConEmuWnd == NULL)
	{
		ghConEmuWnd = FindConEmuByPID();
	}

	if (ghConEmuWnd == NULL)
	{
		// ���� �� ������ �� �������...
		ghConEmuWnd = GetConEmuHWND(TRUE/*abRoot*/);
	}

	if (ghConEmuWnd)
	{
		if (!ghConEmuWndDC)
		{
			// ghConEmuWndDC �� ���� ��� ������ ���� ������� �� GUI ����� �����
			_ASSERTE(ghConEmuWndDC!=NULL);
			HWND hBack = NULL, hDc = NULL;
			wchar_t szClass[128];
			while (!ghConEmuWndDC)
			{
				hBack = FindWindowEx(ghConEmuWnd, hBack, VirtualConsoleClassBack, NULL);
				if (!hBack)
					break;
				if (GetWindowLong(hBack, 0) == (LONG)(DWORD)ghConWnd)
				{
					hDc = (HWND)(DWORD)GetWindowLong(hBack, 4);
					if (IsWindow(hDc) && GetClassName(hDc, szClass, countof(szClass) && !lstrcmp(szClass, VirtualConsoleClass)))
					{
						SetConEmuWindows(hDc, hBack);
						break;
					}
				}
			}
			_ASSERTE(ghConEmuWndDC!=NULL);
		}

		// ���������� ���������� ����� � ������������ ����
		SetConEmuEnvVar(ghConEmuWnd);
		dwGuiThreadId = GetWindowThreadProcessId(ghConEmuWnd, &gpSrv->dwGuiPID);
		AllowSetForegroundWindow(gpSrv->dwGuiPID);

		//if (hWndFore == ghConWnd || hWndFocus == ghConWnd)
		//if (hWndFore != ghConEmuWnd)

		if (GetForegroundWindow() == ghConWnd)
			apiSetForegroundWindow(ghConEmuWnd); // 2009-09-14 ������-�� ���� ���� ghConWnd ?
	}
	else
	{
		// �� � ��� ����. ��� ����� ������ ���, ��� gui ���������
		//_ASSERTE(ghConEmuWnd!=NULL);
	}
}

bool TryConnect2Gui(HWND hGui, HWND& hDcWnd, CESERVER_REQ* pIn)
{
	bool bConnected = false;

	//if (lbNeedSetFont) {
	//	lbNeedSetFont = FALSE;
	//
	//    if (gpLogSize) LogSize(NULL, ":SetConsoleFontSizeTo.before");
	//    SetConsoleFontSizeTo(ghConWnd, gpSrv->nConFontHeight, gpSrv->nConFontWidth, gpSrv->szConsoleFont);
	//    if (gpLogSize) LogSize(NULL, ":SetConsoleFontSizeTo.after");
	//}
	// ���� GUI ������� �� �� ����� ������ - �� �� ���������� ��� �������
	// ������� ���������� �������� �������. ����� ����� ��� ������.
	pIn->StartStop.hServerProcessHandle = NULL;

	if (pIn->StartStop.bUserIsAdmin)
	{
		DWORD  nGuiPid = 0;

		if (GetWindowThreadProcessId(hGui, &nGuiPid) && nGuiPid)
		{
			HANDLE hGuiHandle = OpenProcess(PROCESS_DUP_HANDLE, FALSE, nGuiPid);

			if (hGuiHandle)
			{
				HANDLE hDupHandle = NULL;

				if (DuplicateHandle(GetCurrentProcess(), GetCurrentProcess(),
				                   hGuiHandle, &hDupHandle, PROCESS_QUERY_INFORMATION|SYNCHRONIZE,
				                   FALSE, 0)
				        && hDupHandle)
				{
					pIn->StartStop.hServerProcessHandle = (u64)hDupHandle;
				}

				CloseHandle(hGuiHandle);
			}
		}
	}

	wchar_t szPipe[64];
	_wsprintf(szPipe, SKIPLEN(countof(szPipe)) CEGUIPIPENAME, L".", (DWORD)hGui); //-V205
	CESERVER_REQ *pOut = ExecuteCmd(szPipe, pIn, GUIATTACH_TIMEOUT, ghConWnd);

	if (!pOut)
	{
		_ASSERTE(pOut!=NULL);
	}
	else
	{
		//ghConEmuWnd = hGui;
		ghConEmuWnd = pOut->StartStopRet.hWnd;
		hDcWnd = pOut->StartStopRet.hWndDc;
		SetConEmuWindows(pOut->StartStopRet.hWndDc, pOut->StartStopRet.hWndBack);
		gpSrv->dwGuiPID = pOut->StartStopRet.dwPID;
		_ASSERTE(gpSrv->pConsoleMap != NULL); // ������� ��� ������ ���� ������,
		_ASSERTE(gpSrv->pConsole != NULL); // � ��������� ����� ����
		//gpSrv->pConsole->info.nGuiPID = pOut->StartStopRet.dwPID;
		CESERVER_CONSOLE_MAPPING_HDR *pMap = gpSrv->pConsoleMap->Ptr();
		if (pMap)
		{
			pMap->nGuiPID = pOut->StartStopRet.dwPID;
			pMap->hConEmuRoot = ghConEmuWnd;
			pMap->hConEmuWndDc = ghConEmuWndDC;
			pMap->hConEmuWndBack = ghConEmuWndBack;
			_ASSERTE(pMap->hConEmuRoot==NULL || pMap->nGuiPID!=0);
		}

		//DisableAutoConfirmExit();

		// � ��������, ������� ����� ������������� ����������� �������. � ���� ������ �� �������� �� �����
		// �� ������ �����, ������� ���������� ��� ������� � Win7 ����� ���������� ��������
		// 110807 - ���� gbAttachMode, ���� ������� ����� ��������
		if (gbForceHideConWnd || gbAttachMode)
			apiShowWindow(ghConWnd, SW_HIDE);

		// ���������� ����� � �������
		if (pOut->StartStopRet.Font.cbSize == sizeof(CESERVER_REQ_SETFONT))
		{
			lstrcpy(gpSrv->szConsoleFont, pOut->StartStopRet.Font.sFontName);
			gpSrv->nConFontHeight = pOut->StartStopRet.Font.inSizeY;
			gpSrv->nConFontWidth = pOut->StartStopRet.Font.inSizeX;
			ServerInitFont();
		}

		COORD crNewSize = {(SHORT)pOut->StartStopRet.nWidth, (SHORT)pOut->StartStopRet.nHeight};
		//SMALL_RECT rcWnd = {0,pIn->StartStop.sbi.srWindow.Top};
		SMALL_RECT rcWnd = {0};
		SetConsoleSize((USHORT)pOut->StartStopRet.nBufferHeight, crNewSize, rcWnd, "Attach2Gui:Ret");
		// ���������� ���������� ����� � ������������ ����
		SetConEmuEnvVar(ghConEmuWnd);
		CheckConEmuHwnd();
		ExecuteFreeResult(pOut);
		
		bConnected = true;
	}

	return bConnected;
}

HWND Attach2Gui(DWORD nTimeout)
{
	if (isTerminalMode())
	{
		_ASSERTE(FALSE && "Attach is not allowed in telnet");
		return NULL;
	}

	// ���� Refresh �� ������ ���� ��������, ����� � ������� ����� ������� ������ �� �������
	// �� ����, ��� ���������� ������ (��� ������, ������� ������ ���������� GUI ��� ������)
	_ASSERTE(gpSrv->dwRefreshThread==0 || gpSrv->bWasDetached);
	HWND hGui = NULL, hDcWnd = NULL;
	//UINT nMsg = RegisterWindowMessage(CONEMUMSG_ATTACH);
	BOOL bNeedStartGui = FALSE;
	DWORD dwErr = 0;
	DWORD dwStartWaitIdleResult = -1;
	// ����� ������������ ������
	gpSrv->bWasDetached = FALSE;

	if (!gpSrv->pConsoleMap)
	{
		_ASSERTE(gpSrv->pConsoleMap!=NULL);
	}
	else
	{
		// ����� GUI �� ������� ������� ���������� �� ������� �� ���������� ������ (�� ��������� ��������)
		gpSrv->pConsoleMap->Ptr()->bDataReady = FALSE;
	}

	if (gpSrv->bRequestNewGuiWnd && !gpSrv->dwGuiPID && !gpSrv->hGuiWnd)
	{
		bNeedStartGui = TRUE;
		hGui = (HWND)-1;
	}
	else if (gpSrv->dwGuiPID && gpSrv->hGuiWnd)
	{
		wchar_t szClass[128];
		GetClassName(gpSrv->hGuiWnd, szClass, countof(szClass));
		if (lstrcmp(szClass, VirtualConsoleClassMain) == 0)
			hGui = gpSrv->hGuiWnd;
		else
			gpSrv->hGuiWnd = NULL;
	}
	else if (gpSrv->hGuiWnd)
	{
		_ASSERTE(gpSrv->hGuiWnd==NULL);
		gpSrv->hGuiWnd = NULL;
	}

	if (!hGui)
		hGui = FindWindowEx(NULL, hGui, VirtualConsoleClassMain, NULL);

	if (!hGui)
	{
		DWORD dwGuiPID = 0;
		HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);

		if (hSnap != INVALID_HANDLE_VALUE)
		{
			PROCESSENTRY32 prc = {sizeof(PROCESSENTRY32)};

			if (Process32First(hSnap, &prc))
			{
				do
				{
					for(UINT i = 0; i < gpSrv->nProcessCount; i++)
					{
						if (lstrcmpiW(prc.szExeFile, L"conemu.exe")==0
							|| lstrcmpiW(prc.szExeFile, L"conemu64.exe")==0)
						{
							dwGuiPID = prc.th32ProcessID;
							break;
						}
					}

					if (dwGuiPID) break;
				}
				while(Process32Next(hSnap, &prc));
			}

			CloseHandle(hSnap);

			if (!dwGuiPID) bNeedStartGui = TRUE;
		}
	}

	if (bNeedStartGui)
	{
		wchar_t szSelf[MAX_PATH+100];
		wchar_t* pszSelf = szSelf+1, *pszSlash = NULL;

		if (!GetModuleFileName(NULL, pszSelf, MAX_PATH))
		{
			dwErr = GetLastError();
			_printf("GetModuleFileName failed, ErrCode=0x%08X\n", dwErr);
			return NULL;
		}

		pszSlash = wcsrchr(pszSelf, L'\\');

		if (!pszSlash)
		{
			_printf("Invalid GetModuleFileName, backslash not found!\n", 0, pszSelf); //-V576
			return NULL;
		}

		bool bExeFound = false;

		for (int s = 0; s <= 1; s++)
		{
			if (s)
			{
				// �� ����� ���� �� ������� ����
				*pszSlash = 0;
				pszSlash = wcsrchr(pszSelf, L'\\');
				if (!pszSlash)
					break;
			}

			if (IsWindows64())
			{
				lstrcpyW(pszSlash+1, L"ConEmu64.exe");
				if (FileExists(pszSelf))
				{
					bExeFound = true;
					break;
				}
			}

			lstrcpyW(pszSlash+1, L"ConEmu.exe");
			if (FileExists(pszSelf))
			{
				bExeFound = true;
				break;
			}
		}

		if (!bExeFound)
		{
			_printf("ConEmu.exe not found!\n");
			return NULL;
		}

		if (wcschr(pszSelf, L' '))
		{
			*(--pszSelf) = L'"';
			//lstrcpyW(pszSlash, L"ConEmu.exe\"");
			lstrcatW(pszSlash, L"\"");
		}

		//else
		//{
		//	lstrcpyW(pszSlash, L"ConEmu.exe");
		//}
		lstrcatW(pszSelf, L" /detached");
		PROCESS_INFORMATION pi; memset(&pi, 0, sizeof(pi));
		STARTUPINFOW si; memset(&si, 0, sizeof(si)); si.cb = sizeof(si);
		PRINT_COMSPEC(L"Starting GUI:\n%s\n", pszSelf);
		// CREATE_NEW_PROCESS_GROUP - ����, ��������� �������� Ctrl-C
		// ������ GUI (conemu.exe), ���� ���-�� �� �����
		BOOL lbRc = createProcess(TRUE, NULL, pszSelf, NULL,NULL, TRUE,
		                           NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi);
		dwErr = GetLastError();

		if (!lbRc)
		{
			PrintExecuteError(pszSelf, dwErr);
			return NULL;
		}

		//delete psNewCmd; psNewCmd = NULL;
		AllowSetForegroundWindow(pi.dwProcessId);
		PRINT_COMSPEC(L"Detached GUI was started. PID=%i, Attaching...\n", pi.dwProcessId);
		dwStartWaitIdleResult = WaitForInputIdle(pi.hProcess, INFINITE); // ��� nTimeout, ������ ����� �����������
		SafeCloseHandle(pi.hProcess); SafeCloseHandle(pi.hThread);
		//if (nTimeout > 1000) nTimeout = 1000;
	}

	DWORD dwStart = GetTickCount(), dwDelta = 0, dwCur = 0;
	CESERVER_REQ *pIn = NULL;
	_ASSERTE(sizeof(CESERVER_REQ_STARTSTOP) >= sizeof(CESERVER_REQ_STARTSTOPRET));
	DWORD nInSize = sizeof(CESERVER_REQ_HDR)+sizeof(CESERVER_REQ_STARTSTOP)+(gpszRunCmd ? lstrlen(gpszRunCmd) : 0)*sizeof(wchar_t);
	pIn = ExecuteNewCmd(CECMD_ATTACH2GUI, nInSize);
	pIn->StartStop.nStarted = sst_ServerStart;
	pIn->StartStop.hWnd = ghConWnd;
	pIn->StartStop.dwPID = gnSelfPID;
	pIn->StartStop.dwAID = gpSrv->dwGuiAID;
	//pIn->StartStop.dwInputTID = gpSrv->dwInputPipeThreadId;
	pIn->StartStop.nSubSystem = gnImageSubsystem;

	if (gbAttachFromFar)
		pIn->StartStop.bRootIsCmdExe = FALSE;
	else
		pIn->StartStop.bRootIsCmdExe = gbRootIsCmdExe || (gbAttachMode && !gbNoCreateProcess);
	
	pIn->StartStop.bRunInBackgroundTab = gbRunInBackgroundTab;

	if (gpszRunCmd && *gpszRunCmd)
	{
		BOOL lbNeedCutQuot = FALSE, lbRootIsCmd = FALSE, lbConfirmExit = FALSE, lbAutoDisable = FALSE;
		IsNeedCmd(gpszRunCmd, NULL, &lbNeedCutQuot, pIn->StartStop.sModuleName, lbRootIsCmd, lbConfirmExit, lbAutoDisable);
		lstrcpy(pIn->StartStop.sCmdLine, gpszRunCmd);
	}

	// ���� GUI ������� �� �� ����� ������ - �� �� ���������� ��� �������
	// ������� ���������� �������� �������. ����� ����� ��� ������.
	pIn->StartStop.bUserIsAdmin = IsUserAdmin();
	HANDLE hOut = (HANDLE)ghConOut;

	if (!GetConsoleScreenBufferInfo(hOut, &pIn->StartStop.sbi))
	{
		_ASSERTE(FALSE);
	}
	else
	{
		gpSrv->crReqSizeNewSize = pIn->StartStop.sbi.dwSize;
		_ASSERTE(gpSrv->crReqSizeNewSize.X!=0);
	}

	pIn->StartStop.crMaxSize = MyGetLargestConsoleWindowSize(hOut);

//LoopFind:
	// � ������� "���������" ������ ����� ��� ����������, � ��� ������
	// ������� �������� - ����� ���-����� �������� �� ���������
	//BOOL lbNeedSetFont = TRUE;

	// ���� � ������� ���� �� ��������� (GUI ��� ��� �� �����������) ������� ���
	while (!hDcWnd && dwDelta <= nTimeout)
	{
		if (gpSrv->hGuiWnd)
		{
			if (TryConnect2Gui(gpSrv->hGuiWnd, hDcWnd, pIn) && hDcWnd)
				break; // OK
		}
		else
		{
			HWND hFindGui = NULL;

			while ((hFindGui = FindWindowEx(NULL, hFindGui, VirtualConsoleClassMain, NULL)) != NULL)
			{
				if (TryConnect2Gui(hFindGui, hDcWnd, pIn))
					break; // OK
			}
		}

		if (hDcWnd)
			break;

		dwCur = GetTickCount(); dwDelta = dwCur - dwStart;

		if (dwDelta > nTimeout)
			break;

		Sleep(500);
		dwCur = GetTickCount(); dwDelta = dwCur - dwStart;
	}

	return hDcWnd;
}




/*
!! �� ��������� ���������������� ������ ������������ ��������� MAP ������ ��� ����� (Info), ���� �� ������
   ��������� � �������������� MAP, "ConEmuFileMapping.%08X.N", ��� N == nCurDataMapIdx
   ���������� ��� ��� ����, ����� ��� ���������� ������� ������� ����� ���� ������������� ��������� ����� ������
   � ����� ����������� ������ �� ���� �������

!! ��� ������, � ������� ����� �������� ������ � ������� ������ ������� �������. �.�. ��� � ������ ��������
   ��� ����� ��������� - ������� � ��������� ������� ���� �� ������.

1. ReadConsoleData ������ ����� �����������, ������ ������ ������ 30K? ���� ������ - �� � �� �������� ������ ������.
2. ReadConsoleInfo & ReadConsoleData ������ ���������� TRUE ��� ������� ��������� (��������� ������ ���������� ��������� ������ ��� ���������)
3. ReloadFullConsoleInfo �������� ����� ReadConsoleInfo & ReadConsoleData ������ ���� ���. ������� ������� ������ ��� ������� ���������. �� ����� �������� Tick

4. � RefreshThread ����� ������� hRefresh 10�� (������) ��� hExit.
-- ���� ���� ������ �� ��������� ������� -
   ������� � TRUE ��������� ���������� bChangingSize
   ������ ������ � ������ ����� �����
-- ����� ReloadFullConsoleInfo
-- ����� ���, ���� bChangingSize - ���������� Event hSizeChanged.
5. ����� ��� ����������� ������. ��� ������ ��� �� �����, �.�. ��� ������ ������� ����� � RefreshThread.
6. ������� ����� ������� �� ������ ���� ������ ������, � ���������� ������ � RefreshThread � ����� ������� hSizeChanged
*/


int CreateMapHeader()
{
	int iRc = 0;
	//wchar_t szMapName[64];
	//int nConInfoSize = sizeof(CESERVER_CONSOLE_MAPPING_HDR);
	_ASSERTE(gpSrv->pConsole == NULL);
	_ASSERTE(gpSrv->pConsoleMap == NULL);
	_ASSERTE(gpSrv->pConsoleDataCopy == NULL);
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD crMax = MyGetLargestConsoleWindowSize(h);

	if (crMax.X < 80 || crMax.Y < 25)
	{
#ifdef _DEBUG
		DWORD dwErr = GetLastError();
		if (gbIsWine)
		{
			wchar_t szDbgMsg[512], szTitle[128];
			szDbgMsg[0] = 0;
			GetModuleFileName(NULL, szDbgMsg, countof(szDbgMsg));
			msprintf(szTitle, countof(szTitle), L"%s: PID=%u", PointToName(szDbgMsg), GetCurrentProcessId());
			msprintf(szDbgMsg, countof(szDbgMsg), L"GetLargestConsoleWindowSize failed -> {%ix%i}, Code=%u", crMax.X, crMax.Y, dwErr);
			MessageBox(NULL, szDbgMsg, szTitle, MB_SYSTEMMODAL);
		}
		else
		{
			_ASSERTE(crMax.X >= 80 && crMax.Y >= 25);
		}
#endif

		if (crMax.X < 80) crMax.X = 80;

		if (crMax.Y < 25) crMax.Y = 25;
	}

	// ������ ������ ����� ���� ��� �� ��������? �������� ������ �� ���������?
	HMONITOR hMon = MonitorFromWindow(ghConWnd, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO mi = {sizeof(MONITORINFO)};

	if (GetMonitorInfo(hMon, &mi))
	{
		int x = (mi.rcWork.right - mi.rcWork.left) / 3;
		int y = (mi.rcWork.bottom - mi.rcWork.top) / 5;

		if (crMax.X < x || crMax.Y < y)
		{
			//_ASSERTE((crMax.X + 16) >= x && (crMax.Y + 32) >= y);
			if (crMax.X < x)
				crMax.X = x;

			if (crMax.Y < y)
				crMax.Y = y;
		}
	}

	//TODO("�������� � nConDataSize ������ ����������� ��� �������� crMax �����");
	int nTotalSize = 0;
	DWORD nMaxCells = (crMax.X * crMax.Y);
	//DWORD nHdrSize = ((LPBYTE)gpSrv->pConsoleDataCopy->Buf) - ((LPBYTE)gpSrv->pConsoleDataCopy);
	//_ASSERTE(sizeof(CESERVER_REQ_CONINFO_DATA) == (sizeof(COORD)+sizeof(CHAR_INFO)));
	int nMaxDataSize = nMaxCells * sizeof(CHAR_INFO); // + nHdrSize;
	gpSrv->pConsoleDataCopy = (CHAR_INFO*)calloc(nMaxDataSize,1);

	if (!gpSrv->pConsoleDataCopy)
	{
		_printf("ConEmuC: calloc(%i) failed, pConsoleDataCopy is null", nMaxDataSize);
		goto wrap;
	}

	//gpSrv->pConsoleDataCopy->crMaxSize = crMax;
	nTotalSize = sizeof(CESERVER_REQ_CONINFO_FULL) + (nMaxCells * sizeof(CHAR_INFO));
	gpSrv->pConsole = (CESERVER_REQ_CONINFO_FULL*)calloc(nTotalSize,1);

	if (!gpSrv->pConsole)
	{
		_printf("ConEmuC: calloc(%i) failed, pConsole is null", nTotalSize);
		goto wrap;
	}

	// !!! Warning !!! ������� �����, ������� � ReloadGuiSettings() !!!
	gpSrv->pConsole->cbMaxSize = nTotalSize;
	//gpSrv->pConsole->cbActiveSize = ((LPBYTE)&(gpSrv->pConsole->data)) - ((LPBYTE)gpSrv->pConsole);
	//gpSrv->pConsole->bChanged = TRUE; // Initially == changed
	gpSrv->pConsole->hdr.cbSize = sizeof(gpSrv->pConsole->hdr);
	gpSrv->pConsole->hdr.nLogLevel = (gpLogSize!=NULL) ? 1 : 0;
	gpSrv->pConsole->hdr.crMaxConSize = crMax;
	gpSrv->pConsole->hdr.bDataReady = FALSE;
	gpSrv->pConsole->hdr.hConWnd = ghConWnd; _ASSERTE(ghConWnd!=NULL);
	_ASSERTE(gpSrv->dwMainServerPID!=0);
	gpSrv->pConsole->hdr.nServerPID = gpSrv->dwMainServerPID;
	gpSrv->pConsole->hdr.nAltServerPID = (gnRunMode==RM_ALTSERVER) ? GetCurrentProcessId() : gpSrv->dwAltServerPID;
	gpSrv->pConsole->hdr.nGuiPID = gpSrv->dwGuiPID;
	gpSrv->pConsole->hdr.hConEmuRoot = ghConEmuWnd;
	gpSrv->pConsole->hdr.hConEmuWndDc = ghConEmuWndDC;
	gpSrv->pConsole->hdr.hConEmuWndBack = ghConEmuWndBack;
	_ASSERTE(gpSrv->pConsole->hdr.hConEmuRoot==NULL || gpSrv->pConsole->hdr.nGuiPID!=0);
	gpSrv->pConsole->hdr.bConsoleActive = TRUE; // ���� - TRUE (��� �� ������ �������)
	gpSrv->pConsole->hdr.nServerInShutdown = 0;
	gpSrv->pConsole->hdr.bThawRefreshThread = TRUE; // ���� - TRUE (��� �� ������ �������)
	gpSrv->pConsole->hdr.nProtocolVersion = CESERVER_REQ_VER;
	gpSrv->pConsole->hdr.nActiveFarPID = gpSrv->nActiveFarPID; // PID ���������� ��������� ����

	// �������� ���������� ��������� (����� ConEmuGuiMapping)
	if (ghConEmuWnd) // ���� ��� �������� - ����� �����
		ReloadGuiSettings(NULL);

	//WARNING! � ������ ��������� info ���� CESERVER_REQ_HDR ��� ���������� ������� ����� �����
	gpSrv->pConsole->info.cmd.cbSize = sizeof(gpSrv->pConsole->info); // ���� ��� - ������ ������ ���������
	gpSrv->pConsole->info.hConWnd = ghConWnd; _ASSERTE(ghConWnd!=NULL);
	gpSrv->pConsole->info.crMaxSize = crMax;
	
	// ���������, ����� �� ������ ������, ����� � ����� ServerInit
	
	//WARNING! ����� ������ ���� ������������ ����� ������ ����� ����� � GUI
	gpSrv->pConsole->bDataChanged = TRUE;
	
	gpSrv->pConsoleMap = new MFileMapping<CESERVER_CONSOLE_MAPPING_HDR>;

	if (!gpSrv->pConsoleMap)
	{
		_printf("ConEmuC: calloc(MFileMapping<CESERVER_CONSOLE_MAPPING_HDR>) failed, pConsoleMap is null", 0); //-V576
		goto wrap;
	}

	gpSrv->pConsoleMap->InitName(CECONMAPNAME, (DWORD)ghConWnd); //-V205

	BOOL lbCreated;
	if (gnRunMode == RM_SERVER)
		lbCreated = (gpSrv->pConsoleMap->Create() != NULL);
	else
		lbCreated = (gpSrv->pConsoleMap->Open() != NULL);

	if (!lbCreated)
	{
		_wprintf(gpSrv->pConsoleMap->GetErrorText());
		delete gpSrv->pConsoleMap; gpSrv->pConsoleMap = NULL;
		iRc = CERR_CREATEMAPPINGERR; goto wrap;
	}

	//gpSrv->pConsoleMap->SetFrom(&(gpSrv->pConsole->hdr));
	UpdateConsoleMapHeader();
wrap:
	return iRc;
}

void UpdateConsoleMapHeader()
{
	WARNING("***ALT*** �� ����� ��������� ������� ������������ � � ������� � � ����.�������");

	if (gpSrv && gpSrv->pConsole)
	{
		if (gnRunMode == RM_SERVER) // !!! RM_ALTSERVER - ���� !!!
		{
			if (ghConEmuWndDC && (!gpSrv->pColorerMapping || (gpSrv->pConsole->hdr.hConEmuWndDc != ghConEmuWndDC)))
			{
				if (gpSrv->pColorerMapping && (gpSrv->pConsole->hdr.hConEmuWndDc != ghConEmuWndDC))
				{
					// �� ����, �� ������ ���� ������������ TrueColor ��������
					_ASSERTE(gpSrv->pColorerMapping);
					delete gpSrv->pColorerMapping;
					gpSrv->pColorerMapping = NULL;
				}
				CreateColorerHeader();
			}
			gpSrv->pConsole->hdr.nServerPID = GetCurrentProcessId();
		}
		else if (gnRunMode == RM_ALTSERVER)
		{
			DWORD nCurServerInMap = 0;
			if (gpSrv->pConsoleMap && gpSrv->pConsoleMap->IsValid())
				nCurServerInMap = gpSrv->pConsoleMap->Ptr()->nServerPID;

			_ASSERTE(gpSrv->pConsole->hdr.nServerPID && (gpSrv->pConsole->hdr.nServerPID == gpSrv->dwMainServerPID));
			if (nCurServerInMap && (nCurServerInMap != gpSrv->dwMainServerPID))
			{
				if (IsMainServerPID(nCurServerInMap))
				{
					// �������, �������� ������ ��������?
					_ASSERTE((nCurServerInMap == gpSrv->dwMainServerPID) && "Main server was changed?");
					CloseHandle(gpSrv->hMainServer);
					gpSrv->dwMainServerPID = nCurServerInMap;
					gpSrv->hMainServer = OpenProcess(SYNCHRONIZE|PROCESS_QUERY_INFORMATION, FALSE, nCurServerInMap);
				}
			}

			gpSrv->pConsole->hdr.nServerPID = gpSrv->dwMainServerPID;
			gpSrv->pConsole->hdr.nAltServerPID = GetCurrentProcessId();
		}

		if (gnRunMode == RM_SERVER || gnRunMode == RM_ALTSERVER)
		{
			// ������ _�������_ �������. ���������� ����������� ��������� ������ ��� "�������".
			// ������ ����� ������ ������ ������������ �������� ���� ConEmu
			_ASSERTE(gcrVisibleSize.X>0 && gcrVisibleSize.X<=400 && gcrVisibleSize.Y>0 && gcrVisibleSize.Y<=300);
			gpSrv->pConsole->hdr.bLockVisibleArea = TRUE;
			gpSrv->pConsole->hdr.crLockedVisible = gcrVisibleSize;
			// ����� ��������� ���������. ���� - �����.
			gpSrv->pConsole->hdr.rbsAllowed = rbs_Any;
		}
		gpSrv->pConsole->hdr.nGuiPID = gpSrv->dwGuiPID;
		gpSrv->pConsole->hdr.hConEmuRoot = ghConEmuWnd;
		gpSrv->pConsole->hdr.hConEmuWndDc = ghConEmuWndDC;
		gpSrv->pConsole->hdr.hConEmuWndBack = ghConEmuWndBack;
		_ASSERTE(gpSrv->pConsole->hdr.hConEmuRoot==NULL || gpSrv->pConsole->hdr.nGuiPID!=0);
		gpSrv->pConsole->hdr.nActiveFarPID = gpSrv->nActiveFarPID;

		// ������ ����.������� ������� ������ - ���������
		if (gnRunMode != RM_SERVER /*&& gnRunMode != RM_ALTSERVER*/)
		{
			_ASSERTE(gnRunMode == RM_SERVER || gnRunMode == RM_ALTSERVER);
			return;
		}

		if (gpSrv->pConsoleMap)
		{
			gpSrv->pConsoleMap->SetFrom(&(gpSrv->pConsole->hdr));
		}
	}
}

int CreateColorerHeader()
{
	int iRc = -1;
	//wchar_t szMapName[64];
	DWORD dwErr = 0;
	//int nConInfoSize = sizeof(CESERVER_CONSOLE_MAPPING_HDR);
	//int nMapCells = 0;
	//DWORD nMapSize = 0;
	HWND lhConWnd = NULL;
	_ASSERTE(gpSrv->pColorerMapping == NULL);
	// 111101 - ���� "GetConEmuHWND(2)", �� GetConsoleWindow ������ ���������������.
	lhConWnd = ghConEmuWndDC; // GetConEmuHWND(2);

	if (!lhConWnd)
	{
		_ASSERTE(lhConWnd != NULL);
		dwErr = GetLastError();
		_printf("Can't create console data file mapping. ConEmu DC window is NULL.\n");
		//iRc = CERR_COLORERMAPPINGERR; -- ������ �� ����������� � �� ��������������
		return 0;
	}

	//COORD crMaxSize = MyGetLargestConsoleWindowSize(GetStdHandle(STD_OUTPUT_HANDLE));
	//nMapCells = max(crMaxSize.X,200) * max(crMaxSize.Y,200) * 2;
	//nMapSize = nMapCells * sizeof(AnnotationInfo) + sizeof(AnnotationHeader);

	if (gpSrv->pColorerMapping == NULL)
	{
		gpSrv->pColorerMapping = new MFileMapping<const AnnotationHeader>;
	}
	// ������ ��� ��� mapping, ���� ���� - ��� ������� CloseMap();
	gpSrv->pColorerMapping->InitName(AnnotationShareName, (DWORD)sizeof(AnnotationInfo), (DWORD)lhConWnd); //-V205

	//_wsprintf(szMapName, SKIPLEN(countof(szMapName)) AnnotationShareName, sizeof(AnnotationInfo), (DWORD)lhConWnd);
	//gpSrv->hColorerMapping = CreateFileMapping(INVALID_HANDLE_VALUE,
	//                                        gpLocalSecurity, PAGE_READWRITE, 0, nMapSize, szMapName);

	//if (!gpSrv->hColorerMapping)
	//{
	//	dwErr = GetLastError();
	//	_printf("Can't create colorer data mapping. ErrCode=0x%08X\n", dwErr, szMapName);
	//	iRc = CERR_COLORERMAPPINGERR;
	//}
	//else
	//{
	// ��������� �������� �������� ���������� � �������, ����� ���������!
	//AnnotationHeader* pHdr = (AnnotationHeader*)MapViewOfFile(gpSrv->hColorerMapping, FILE_MAP_ALL_ACCESS,0,0,0);
	// 111101 - ���� "Create(nMapSize);"
	const AnnotationHeader* pHdr = gpSrv->pColorerMapping->Open();

	if (!pHdr)
	{
		dwErr = GetLastError();
		// 111101 ������ �� ������ - ������� ����� ���� �������� � ConEmu
		//_printf("Can't map colorer data mapping. ErrCode=0x%08X\n", dwErr/*, szMapName*/);
		//_wprintf(gpSrv->pColorerMapping->GetErrorText());
		//_printf("\n");
		//iRc = CERR_COLORERMAPPINGERR;
		//CloseHandle(gpSrv->hColorerMapping); gpSrv->hColorerMapping = NULL;
		delete gpSrv->pColorerMapping;
		gpSrv->pColorerMapping = NULL;
	}
	else if (pHdr->struct_size != sizeof(AnnotationHeader))
	{
		_ASSERTE(pHdr->struct_size == sizeof(AnnotationHeader));
		delete gpSrv->pColorerMapping;
		gpSrv->pColorerMapping = NULL;
	}
	else
	{
		//pHdr->struct_size = sizeof(AnnotationHeader);
		//pHdr->bufferSize = nMapCells;
		_ASSERTE((gnRunMode == RM_ALTSERVER) || (pHdr->locked == 0 && pHdr->flushCounter == 0));
		//pHdr->locked = 0;
		//pHdr->flushCounter = 0;
		gpSrv->ColorerHdr = *pHdr;
		//// � ������� - ������ �� �����
		////UnmapViewOfFile(pHdr);
		//gpSrv->pColorerMapping->ClosePtr();

		// OK
		iRc = 0;
	}

	//}

	return iRc;
}

void CloseMapHeader()
{
	if (gpSrv->pConsoleMap)
	{
		//gpSrv->pConsoleMap->CloseMap(); -- �� ���������, ������� ����������
		delete gpSrv->pConsoleMap;
		gpSrv->pConsoleMap = NULL;
	}

	if (gpSrv->pConsole)
	{
		free(gpSrv->pConsole);
		gpSrv->pConsole = NULL;
	}

	if (gpSrv->pConsoleDataCopy)
	{
		free(gpSrv->pConsoleDataCopy);
		gpSrv->pConsoleDataCopy = NULL;
	}
}


// ���������� TRUE - ���� ������ ������ ������� ������� (��� ����� ��������� � �������)
BOOL CorrectVisibleRect(CONSOLE_SCREEN_BUFFER_INFO* pSbi)
{
	BOOL lbChanged = FALSE;
	_ASSERTE(gcrVisibleSize.Y<200); // ������ ������� �������
	// ���������� �������������� ���������
	SHORT nLeft = 0;
	SHORT nRight = pSbi->dwSize.X - 1;
	SHORT nTop = pSbi->srWindow.Top;
	SHORT nBottom = pSbi->srWindow.Bottom;

	if (gnBufferHeight == 0)
	{
		// ������ ��� ��� �� ������ ������������ �� ��������� ������ BufferHeight
		if (pSbi->dwMaximumWindowSize.Y < pSbi->dwSize.Y)
		{
			// ��� ���������� �������� �����, �.�. ������ ������ ������ ����������� ����������� ������� ����
			// ������ ���������� ��������. �������� VBinDiff ������� ������ ���� �����,
			// �������������� ��� ������ ���������, � ��� ������ ��������� ��...
			//_ASSERTE(pSbi->dwMaximumWindowSize.Y >= pSbi->dwSize.Y);
			gnBufferHeight = pSbi->dwSize.Y;
		}
	}

	// ���������� ������������ ��������� ��� �������� ������
	if (gnBufferHeight == 0)
	{
		nTop = 0;
		nBottom = pSbi->dwSize.Y - 1;
	}
	else if (gpSrv->nTopVisibleLine!=-1)
	{
		// � ��� '���������' ������ ������� ����� ���� �������������
		nTop = gpSrv->nTopVisibleLine;
		nBottom = min((pSbi->dwSize.Y-1), (gpSrv->nTopVisibleLine+gcrVisibleSize.Y-1)); //-V592
	}
	else
	{
		// ������ ������������ ������ ������ �� ������������� � GUI �������
		// ������ �� ��� ��������� ������� ���, ����� ������ ��� �����
		if (pSbi->dwCursorPosition.Y == pSbi->srWindow.Bottom)
		{
			// ���� ������ ��������� � ������ ������� ������ (������������, ��� ����� ���� ������������ ������� ������)
			nTop = pSbi->dwCursorPosition.Y - gcrVisibleSize.Y + 1; // ���������� ������� ����� �� �������
		}
		else
		{
			// ����� - ���������� ����� (��� ����) ����������, ����� ������ ���� �����
			if ((pSbi->dwCursorPosition.Y < pSbi->srWindow.Top) || (pSbi->dwCursorPosition.Y > pSbi->srWindow.Bottom))
			{
				nTop = pSbi->dwCursorPosition.Y - gcrVisibleSize.Y + 1;
			}
		}

		// ��������� �� ������ �� �������
		if (nTop<0) nTop = 0;

		// ������������ ������ ������� �� ������� + �������� ������ ������� �������
		nBottom = (nTop + gcrVisibleSize.Y - 1);

		// ���� �� ��������� ��� �������� �� ������� ������ (���� �� ������ ��?)
		if (nBottom >= pSbi->dwSize.Y)
		{
			// ������������ ���
			nBottom = pSbi->dwSize.Y - 1;
			// � ���� �� ��������� �������
			nTop = max(0, (nBottom - gcrVisibleSize.Y + 1));
		}
	}

#ifdef _DEBUG

	if ((pSbi->srWindow.Bottom - pSbi->srWindow.Top)>pSbi->dwMaximumWindowSize.Y)
	{
		_ASSERTE((pSbi->srWindow.Bottom - pSbi->srWindow.Top)<pSbi->dwMaximumWindowSize.Y);
	}

#endif

	if (nLeft != pSbi->srWindow.Left
	        || nRight != pSbi->srWindow.Right
	        || nTop != pSbi->srWindow.Top
	        || nBottom != pSbi->srWindow.Bottom)
		lbChanged = TRUE;

	return lbChanged;
}






static int ReadConsoleInfo()
{
	//int liRc = 1;
	BOOL lbChanged = gpSrv->pConsole->bDataChanged; // ���� ���-�� ��� �� �������� - ����� TRUE
	//CONSOLE_SELECTION_INFO lsel = {0}; // GetConsoleSelectionInfo
	CONSOLE_CURSOR_INFO lci = {0}; // GetConsoleCursorInfo
	DWORD ldwConsoleCP=0, ldwConsoleOutputCP=0, ldwConsoleMode=0;
	CONSOLE_SCREEN_BUFFER_INFO lsbi = {{0,0}}; // MyGetConsoleScreenBufferInfo
	HANDLE hOut = (HANDLE)ghConOut;
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	//HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
	//DWORD nConInMode = 0;

	if (hOut == INVALID_HANDLE_VALUE)
		hOut = hStdOut;

	// ����� ��������� �������� ��� �������� ComSpec � ���������� ������ ������
	MCHKHEAP;

	if (!GetConsoleCursorInfo(hOut, &lci))
	{
		gpSrv->dwCiRc = GetLastError(); if (!gpSrv->dwCiRc) gpSrv->dwCiRc = -1;
	}
	else
	{
		TODO("������ �� ��������� �� ����� ������� � cmd.exe, ������, GetConsoleMode ����� �������� ������ � cmd.exe");
		//if (gpSrv->bTelnetActive) lci.dwSize = 15;  // telnet "������" ��� ������� Ins - ������ ������ ���� ����� ����� Ctrl ��������
		//GetConsoleMode(hIn, &nConInMode);
		//GetConsoleMode(hOut, &nConInMode);
		//if (GetConsoleMode(hIn, &nConInMode) && !(nConInMode & ENABLE_INSERT_MODE) && (lci.dwSize < 50))
		//	lci.dwSize = 50;

		gpSrv->dwCiRc = 0;

		if (memcmp(&gpSrv->ci, &lci, sizeof(gpSrv->ci)))
		{
			gpSrv->ci = lci;
			lbChanged = TRUE;
		}
	}

	ldwConsoleCP = GetConsoleCP();

	if (gpSrv->dwConsoleCP!=ldwConsoleCP)
	{
		gpSrv->dwConsoleCP = ldwConsoleCP; lbChanged = TRUE;
	}

	ldwConsoleOutputCP = GetConsoleOutputCP();

	if (gpSrv->dwConsoleOutputCP!=ldwConsoleOutputCP)
	{
		gpSrv->dwConsoleOutputCP = ldwConsoleOutputCP; lbChanged = TRUE;
	}

	ldwConsoleMode = 0;
#ifdef _DEBUG
	BOOL lbConModRc =
#endif
	    GetConsoleMode(/*ghConIn*/GetStdHandle(STD_INPUT_HANDLE), &ldwConsoleMode);

	if (gpSrv->dwConsoleMode!=ldwConsoleMode)
	{
		gpSrv->dwConsoleMode = ldwConsoleMode; lbChanged = TRUE;
	}

	MCHKHEAP;

	if (!MyGetConsoleScreenBufferInfo(hOut, &lsbi))
	{
		DWORD dwErr = GetLastError();
		_ASSERTE(FALSE && "ReadConsole::MyGetConsoleScreenBufferInfo failed");

		gpSrv->dwSbiRc = dwErr; if (!gpSrv->dwSbiRc) gpSrv->dwSbiRc = -1;

		//liRc = -1;
		return -1;
	}
	else
	{
		DWORD nCurScroll = (gnBufferHeight ? rbs_Vert : 0) | (gnBufferWidth ? rbs_Horz : 0);
		DWORD nNewScroll = 0;
		int TextWidth = 0, TextHeight = 0;
		BOOL bSuccess = ::GetConWindowSize(lsbi, gcrVisibleSize.X, gcrVisibleSize.Y, nCurScroll, &TextWidth, &TextHeight, &nNewScroll);

		// ��������������� "�������" �����. ������� ������� ��, ��� ������������ � ConEmu
		if (bSuccess)
		{
			//rgn = gpSrv->sbi.srWindow;
			if (!(nNewScroll & rbs_Horz))
			{
				lsbi.srWindow.Left = 0;
				lsbi.srWindow.Right = lsbi.dwSize.X-1;
			}

			if (!(nNewScroll & rbs_Vert))
			{
				lsbi.srWindow.Top = 0;
				lsbi.srWindow.Bottom = lsbi.dwSize.Y-1;
			}
		}

		if (memcmp(&gpSrv->sbi, &lsbi, sizeof(gpSrv->sbi)))
		{
			_ASSERTE(lsbi.srWindow.Left == 0);
			/*
			//Issue 373: ��� ������� wmic ��������������� ������ ������ � 1500 ��������
			//           ���� ConEmu �� ������������ �������������� ��������� - �������,
			//           ����� ���������� ������ ������� ������� ����
			if (lsbi.srWindow.Right != (lsbi.dwSize.X - 1))
			{
				//_ASSERTE(lsbi.srWindow.Right == (lsbi.dwSize.X - 1)); -- �������� ���� �� �����
				lsbi.dwSize.X = (lsbi.srWindow.Right - lsbi.srWindow.Left + 1);
			}
			*/

			// ���������� ���������� ����� �������� ������ ������
			if (!NTVDMACTIVE)  // �� ��� ���������� 16������ ���������� - ��� �� ��� ������ ���������, ����� �������� ������ ��� �������� 16���
			{
				if ((lsbi.srWindow.Top == 0  // ��� ���� ������������ ������� ������
				        && lsbi.dwSize.Y == (lsbi.srWindow.Bottom - lsbi.srWindow.Top + 1)))
				{
					// ��� ������, ��� ��������� ���, � ���������� ���������� �������� ������ ������
					gnBufferHeight = 0; gcrVisibleSize = lsbi.dwSize;
				}

				if (lsbi.dwSize.X != gpSrv->sbi.dwSize.X
				        || (lsbi.srWindow.Bottom - lsbi.srWindow.Top) != (gpSrv->sbi.srWindow.Bottom - gpSrv->sbi.srWindow.Top))
				{
					// ��� ��������� ������� ������� ������� - ����������� ����������� ������
					gpSrv->pConsole->bDataChanged = TRUE;
				}
			}

#ifdef ASSERT_UNWANTED_SIZE
			COORD crReq = gpSrv->crReqSizeNewSize;
			COORD crSize = lsbi.dwSize;

			if (crReq.X != crSize.X && !gpSrv->dwDisplayMode && !IsZoomed(ghConWnd))
			{
				// ������ ���� �� ���� ��������� ��������� ������� �������!
				if (!gpSrv->nRequestChangeSize)
				{
					LogSize(NULL, ":ReadConsoleInfo(AssertWidth)");
					wchar_t /*szTitle[64],*/ szInfo[128];
					//_wsprintf(szTitle, SKIPLEN(countof(szTitle)) L"ConEmuC, PID=%i", GetCurrentProcessId());
					_wsprintf(szInfo, SKIPLEN(countof(szInfo)) L"Size req by server: {%ix%i},  Current size: {%ix%i}",
					          crReq.X, crReq.Y, crSize.X, crSize.Y);
					//MessageBox(NULL, szInfo, szTitle, MB_OK|MB_SETFOREGROUND|MB_SYSTEMMODAL);
					MY_ASSERT_EXPR(FALSE, szInfo, false);
				}
			}

#endif

			if (gpLogSize) LogSize(NULL, ":ReadConsoleInfo");

			gpSrv->sbi = lsbi;
			lbChanged = TRUE;
		}
	}

	if (!gnBufferHeight)
	{
		int nWndHeight = (gpSrv->sbi.srWindow.Bottom - gpSrv->sbi.srWindow.Top + 1);

		if (gpSrv->sbi.dwSize.Y > (max(gcrVisibleSize.Y,nWndHeight)+200)
		        || (gpSrv->nRequestChangeSize && gpSrv->nReqSizeBufferHeight))
		{
			// ���������� �������� ������ ������!

			if (!gpSrv->nReqSizeBufferHeight)
			{
				//#ifdef _DEBUG
				//EmergencyShow(ghConWnd);
				//#endif
				WARNING("###: ���������� �������� ������������ ������ ������");
				if (gpSrv->sbi.dwSize.Y > 200)
				{
					//_ASSERTE(gpSrv->sbi.dwSize.Y <= 200);
					DEBUGLOGSIZE(L"!!! gpSrv->sbi.dwSize.Y > 200 !!! in ConEmuC.ReloadConsoleInfo\n");
				}
				gpSrv->nReqSizeBufferHeight = gpSrv->sbi.dwSize.Y;
			}

			gnBufferHeight = gpSrv->nReqSizeBufferHeight;
		}

		//	Sleep(10);
		//} else {
		//	break; // OK
	}

	// ����� ������ ������, ����� ������ ���� �������������� ����������
	gpSrv->pConsole->hdr.hConWnd = gpSrv->pConsole->info.hConWnd = ghConWnd;
	_ASSERTE(gpSrv->dwMainServerPID!=0);
	gpSrv->pConsole->hdr.nServerPID = gpSrv->dwMainServerPID;
	gpSrv->pConsole->hdr.nAltServerPID = (gnRunMode==RM_ALTSERVER) ? GetCurrentProcessId() : gpSrv->dwAltServerPID;
	//gpSrv->pConsole->info.nInputTID = gpSrv->dwInputThreadId;
	gpSrv->pConsole->info.nReserved0 = 0;
	gpSrv->pConsole->info.dwCiSize = sizeof(gpSrv->ci);
	gpSrv->pConsole->info.ci = gpSrv->ci;
	gpSrv->pConsole->info.dwConsoleCP = gpSrv->dwConsoleCP;
	gpSrv->pConsole->info.dwConsoleOutputCP = gpSrv->dwConsoleOutputCP;
	gpSrv->pConsole->info.dwConsoleMode = gpSrv->dwConsoleMode;
	gpSrv->pConsole->info.dwSbiSize = sizeof(gpSrv->sbi);
	gpSrv->pConsole->info.sbi = gpSrv->sbi;
	// ���� ���� ����������� (WinXP+) - ������� �������� ������ ��������� �� �������
	//CheckProcessCount(); -- ��� ������ ���� ������� !!!
	//2010-05-26 ��������� � ������ ��������� �� ��������� � GUI �� ������ ���� � �������.
	_ASSERTE(gpSrv->pnProcesses!=NULL);

	if (!gpSrv->nProcessCount /*&& gpSrv->pConsole->info.nProcesses[0]*/)
	{
		_ASSERTE(gpSrv->nProcessCount); //CheckProcessCount(); -- ��� ������ ���� ������� !!!
		lbChanged = TRUE;
	}
	else if (memcmp(gpSrv->pnProcesses, gpSrv->pConsole->info.nProcesses,
	               sizeof(DWORD)*min(gpSrv->nProcessCount,countof(gpSrv->pConsole->info.nProcesses))))
	{
		// ������ ��������� ���������!
		lbChanged = TRUE;
	}

	GetProcessCount(gpSrv->pConsole->info.nProcesses, countof(gpSrv->pConsole->info.nProcesses));
	_ASSERTE(gpSrv->pConsole->info.nProcesses[0]);
	//if (memcmp(&(gpSrv->pConsole->hdr), gpSrv->pConsoleMap->Ptr(), gpSrv->pConsole->hdr.cbSize))
	//	gpSrv->pConsoleMap->SetFrom(&(gpSrv->pConsole->hdr));
	//if (lbChanged) {
	//	gpSrv->pConsoleMap->SetFrom(&(gpSrv->pConsole->hdr));
	//	//lbChanged = TRUE;
	//}
	//if (lbChanged) {
	//	//gpSrv->pConsole->bChanged = TRUE;
	//}
	return lbChanged ? 1 : 0;
}

// !! test test !!

// !!! ������� ������� ������ ������ ������� ��������, ��� ���� ������ �������� �������� �����
// !!! ������ 1000 ��� ������ ������ �������� 140x76 �������� 100��.
// !!! ������ 1000 ��� �� ������ (140x1) �������� 30��.
// !!! ������. �� ��������� ���������� ���� � ������ ��������� ������ ������ ��� ���������.
// !!! � ������� �� ���� ���������� ������ - ������������ � ������� ���� ������������� ������.


static BOOL ReadConsoleData()
{
	BOOL lbRc = FALSE, lbChanged = FALSE;
#ifdef _DEBUG
	CONSOLE_SCREEN_BUFFER_INFO dbgSbi = gpSrv->sbi;
#endif
	HANDLE hOut = NULL;
	//USHORT TextWidth=0, TextHeight=0;
	DWORD TextLen=0;
	COORD bufSize; //, bufCoord;
	SMALL_RECT rgn;
	DWORD nCurSize, nHdrSize;
	// -- �������� ���������� �������������� ���������
	_ASSERTE(gpSrv->sbi.srWindow.Left == 0); // ���� ���� �������
	//_ASSERTE(gpSrv->sbi.srWindow.Right == (gpSrv->sbi.dwSize.X - 1));
	DWORD nCurScroll = (gnBufferHeight ? rbs_Vert : 0) | (gnBufferWidth ? rbs_Horz : 0);
	DWORD nNewScroll = 0;
	int TextWidth = 0, TextHeight = 0;
	BOOL bSuccess = ::GetConWindowSize(gpSrv->sbi, gcrVisibleSize.X, gcrVisibleSize.Y, nCurScroll, &TextWidth, &TextHeight, &nNewScroll);
	UNREFERENCED_PARAMETER(bSuccess);
	//TextWidth  = gpSrv->sbi.dwSize.X;
	//TextHeight = (gpSrv->sbi.srWindow.Bottom - gpSrv->sbi.srWindow.Top + 1);
	TextLen = TextWidth * TextHeight;

	//rgn = gpSrv->sbi.srWindow;
	if (nNewScroll & rbs_Horz)
	{
		rgn.Left = gpSrv->sbi.srWindow.Left;
		rgn.Right = min(gpSrv->sbi.srWindow.Left+TextWidth,gpSrv->sbi.dwSize.X)-1;
	}
	else
	{
		rgn.Left = 0;
		rgn.Right = gpSrv->sbi.dwSize.X-1;
	}

	if (nNewScroll & rbs_Vert)
	{
		rgn.Top = gpSrv->sbi.srWindow.Top;
		rgn.Bottom = min(gpSrv->sbi.srWindow.Top+TextHeight,gpSrv->sbi.dwSize.Y)-1;
	}
	else
	{
		rgn.Top = 0;
		rgn.Bottom = gpSrv->sbi.dwSize.Y-1;
	}
	

	if (!TextWidth || !TextHeight)
	{
		_ASSERTE(TextWidth && TextHeight);
		goto wrap;
	}

	nCurSize = TextLen * sizeof(CHAR_INFO);
	nHdrSize = sizeof(CESERVER_REQ_CONINFO_FULL)-sizeof(CHAR_INFO);

	if (!gpSrv->pConsole || gpSrv->pConsole->cbMaxSize < (nCurSize+nHdrSize))
	{
		_ASSERTE(gpSrv->pConsole && gpSrv->pConsole->cbMaxSize >= (nCurSize+nHdrSize));
		TextHeight = (gpSrv->pConsole->info.crMaxSize.X * gpSrv->pConsole->info.crMaxSize.Y) / TextWidth;

		if (!TextHeight)
		{
			_ASSERTE(TextHeight);
			goto wrap;
		}

		TextLen = TextWidth * TextHeight;
		nCurSize = TextLen * sizeof(CHAR_INFO);
		// ���� MapFile ��� �� ����������, ��� ��� �������� ������ �������
		//if (!RecreateMapData())
		//{
		//	// ��� �� ������� ����������� MapFile - �� � ��������� �� �����...
		//	goto wrap;
		//}
		//_ASSERTE(gpSrv->nConsoleDataSize >= (nCurSize+nHdrSize));
	}

	gpSrv->pConsole->info.crWindow.X = TextWidth; gpSrv->pConsole->info.crWindow.Y = TextHeight;
	hOut = (HANDLE)ghConOut;

	if (hOut == INVALID_HANDLE_VALUE)
		hOut = GetStdHandle(STD_OUTPUT_HANDLE);

	lbRc = FALSE;

	//if (nCurSize <= MAX_CONREAD_SIZE)
	{
		bufSize.X = TextWidth; bufSize.Y = TextHeight;
		//bufCoord.X = 0; bufCoord.Y = 0;
		//rgn = gpSrv->sbi.srWindow;

		//if (ReadConsoleOutput(hOut, gpSrv->pConsoleDataCopy, bufSize, bufCoord, &rgn))
		if (MyReadConsoleOutput(hOut, gpSrv->pConsoleDataCopy, bufSize, rgn))
			lbRc = TRUE;
	}

	//if (!lbRc)
	//{
	//	// �������� ������ ���������
	//	bufSize.X = TextWidth; bufSize.Y = 1;
	//	bufCoord.X = 0; bufCoord.Y = 0;
	//	//rgn = gpSrv->sbi.srWindow;
	//	CHAR_INFO* pLine = gpSrv->pConsoleDataCopy;

	//	for(int y = 0; y < (int)TextHeight; y++, rgn.Top++, pLine+=TextWidth)
	//	{
	//		rgn.Bottom = rgn.Top;
	//		ReadConsoleOutput(hOut, pLine, bufSize, bufCoord, &rgn);
	//	}
	//}

	// ���� - �� ��� ���������� � ������������ ��������
	//gpSrv->pConsoleDataCopy->crBufSize.X = TextWidth;
	//gpSrv->pConsoleDataCopy->crBufSize.Y = TextHeight;

	if (memcmp(gpSrv->pConsole->data, gpSrv->pConsoleDataCopy, nCurSize))
	{
		memmove(gpSrv->pConsole->data, gpSrv->pConsoleDataCopy, nCurSize);
		gpSrv->pConsole->bDataChanged = TRUE; // TRUE ��� ����� ���� � �������� ����, �� ���������� � FALSE
		lbChanged = TRUE;
	}


	if (!lbChanged && gpSrv->pColorerMapping)
	{
		AnnotationHeader ahdr;
		if (gpSrv->pColorerMapping->GetTo(&ahdr, sizeof(ahdr)))
		{
			if (gpSrv->ColorerHdr.flushCounter != ahdr.flushCounter && !ahdr.locked)
			{
				gpSrv->ColorerHdr = ahdr;
				gpSrv->pConsole->bDataChanged = TRUE; // TRUE ��� ����� ���� � �������� ����, �� ���������� � FALSE
				lbChanged = TRUE;
			}
		}
	}


	// ���� - �� ��� ���������� � ������������ ��������
	//gpSrv->pConsoleData->crBufSize = gpSrv->pConsoleDataCopy->crBufSize;
wrap:
	//if (lbChanged)
	//	gpSrv->pConsole->bDataChanged = TRUE;
	return lbChanged;
}




// abForceSend ������������ � TRUE, ����� ��������������
// ����������� GUI �� �������� (�� ���� 1 ���).
BOOL ReloadFullConsoleInfo(BOOL abForceSend)
{
	BOOL lbChanged = abForceSend;
	BOOL lbDataChanged = abForceSend;
	DWORD dwCurThId = GetCurrentThreadId();

	// ������ ���������� ������ � ���� (RefreshThread)
	// ����� �������� ����������
	if (gpSrv->dwRefreshThread && dwCurThId != gpSrv->dwRefreshThread)
	{
		//ResetEvent(gpSrv->hDataReadyEvent);
		if (abForceSend)
			gpSrv->bForceConsoleRead = TRUE;

		ResetEvent(gpSrv->hRefreshDoneEvent);
		SetEvent(gpSrv->hRefreshEvent);
		// ��������, ���� ��������� RefreshThread
		HANDLE hEvents[2] = {ghQuitEvent, gpSrv->hRefreshDoneEvent};
		DWORD nWait = WaitForMultipleObjects(2, hEvents, FALSE, RELOAD_INFO_TIMEOUT);
		lbChanged = (nWait == (WAIT_OBJECT_0+1));

		if (abForceSend)
			gpSrv->bForceConsoleRead = FALSE;

		return lbChanged;
	}

#ifdef _DEBUG
	DWORD nPacketID = gpSrv->pConsole->info.nPacketId;
#endif

	if (gpSrv->hExtConsoleCommit)
	{
		WaitForSingleObject(gpSrv->hExtConsoleCommit, EXTCONCOMMIT_TIMEOUT);
	}

	if (abForceSend)
		gpSrv->pConsole->bDataChanged = TRUE;

	int iInfoRc = ReadConsoleInfo();
	if (iInfoRc == -1)
	{
		lbChanged = FALSE;
	}
	else
	{
		if (iInfoRc == 1)
			lbChanged = TRUE;

		if (ReadConsoleData())
			lbChanged = lbDataChanged = TRUE;

		if (lbChanged && !gpSrv->pConsole->hdr.bDataReady)
		{
			gpSrv->pConsole->hdr.bDataReady = TRUE;
		}

		if (memcmp(&(gpSrv->pConsole->hdr), gpSrv->pConsoleMap->Ptr(), gpSrv->pConsole->hdr.cbSize))
		{
			lbChanged = TRUE;
			//gpSrv->pConsoleMap->SetFrom(&(gpSrv->pConsole->hdr));
			UpdateConsoleMapHeader();
		}

		if (lbChanged)
		{
			// ��������� ������� � Tick
			//gpSrv->pConsole->bChanged = TRUE;
			//if (lbDataChanged)
			gpSrv->pConsole->info.nPacketId++;
			gpSrv->pConsole->info.nSrvUpdateTick = GetTickCount();

			if (gpSrv->hDataReadyEvent)
				SetEvent(gpSrv->hDataReadyEvent);

			//if (nPacketID == gpSrv->pConsole->info.nPacketId) {
			//	gpSrv->pConsole->info.nPacketId++;
			//	TODO("����� �������� �� multimedia tick");
			//	gpSrv->pConsole->info.nSrvUpdateTick = GetTickCount();
			//	//			gpSrv->nFarInfoLastIdx = gpSrv->pConsole->info.nFarInfoIdx;
			//}
		}
	}

	return lbChanged;
}






DWORD WINAPI RefreshThread(LPVOID lpvParam)
{
	DWORD nWait = 0, nAltWait = 0, nFreezeWait = 0;
	HANDLE hEvents[2] = {ghQuitEvent, gpSrv->hRefreshEvent};
	DWORD nDelta = 0;
	DWORD nLastReadTick = 0; //GetTickCount();
	DWORD nLastConHandleTick = GetTickCount();
	BOOL  /*lbEventualChange = FALSE,*/ /*lbForceSend = FALSE,*/ lbChanged = FALSE; //, lbProcessChanged = FALSE;
	DWORD dwTimeout = 10; // ������������� ������ ���������� �� ���� (��������, �������,...)
	DWORD dwAltTimeout = 100;
	//BOOL  bForceRefreshSetSize = FALSE; // ����� ��������� ������� ����� ����� ���������� ������� ��� ��������
	BOOL lbWasSizeChange = FALSE;
	BOOL bThaw = TRUE; // ���� FALSE - ������� �������� �� conhost
	BOOL bConsoleActive = TRUE;

	while (TRUE)
	{
		nWait = WAIT_TIMEOUT;
		//lbForceSend = FALSE;
		MCHKHEAP;

		if (gpSrv->hFreezeRefreshThread)
		{
			HANDLE hFreeze[2] = {gpSrv->hFreezeRefreshThread, ghQuitEvent};
			nFreezeWait = WaitForMultipleObjects(countof(hFreeze), hFreeze, FALSE, INFINITE);
			if (nFreezeWait == (WAIT_OBJECT_0+1))
				break; // ����������� ����������� ������
		}

		// �������� ��������������� �������
		if (gpSrv->hAltServer)
		{
			if (!gpSrv->hAltServerChanged)
				gpSrv->hAltServerChanged = CreateEvent(NULL, FALSE, FALSE, NULL);

			HANDLE hAltWait[3] = {gpSrv->hAltServer, gpSrv->hAltServerChanged, ghQuitEvent};
			nAltWait = WaitForMultipleObjects(countof(hAltWait), hAltWait, FALSE, dwAltTimeout);

			if ((nAltWait == (WAIT_OBJECT_0+0)) || (nAltWait == (WAIT_OBJECT_0+1)))
			{
				// ���� ��� �������� AltServer
				if ((nAltWait == (WAIT_OBJECT_0+0)))
				{
					MSectionLock CsAlt; CsAlt.Lock(gpSrv->csAltSrv, TRUE, 10000);

					if (gpSrv->hAltServer)
					{
						HANDLE h = gpSrv->hAltServer;
						gpSrv->hAltServer = NULL;
						gpSrv->hCloseAltServer = h;

						DWORD nAltServerWasStarted = 0;
						DWORD nAltServerWasStopped = gpSrv->dwAltServerPID;
						AltServerInfo info = {};
						if (gpSrv->dwAltServerPID)
						{
							// ��������� ������� ������ ����������� - �� ����� ������� PID (��� �������� ��� �� �����)
							gpSrv->dwAltServerPID = 0;
							// ��� "����������" ����.������?
							if (gpSrv->AltServers.Get(nAltServerWasStopped, &info, true/*Remove*/))
							{
								// ������������� �� "������" (���� ���)
								if (info.nPrevPID)
								{
									_ASSERTE(info.hPrev!=NULL);
									// ��������� ���� �������� � ������� �����, ������� gpSrv->hAltServer
									// ������������ �������������� ������ (��������), ��������� ��� ���� ������
									nAltServerWasStarted = info.nPrevPID;
									AltServerWasStarted(info.nPrevPID, info.hPrev, true);
								}
							}
						}

						CsAlt.Unlock();

						// ��������� ���
						CESERVER_REQ *pGuiIn = NULL, *pGuiOut = NULL;
						int nSize = sizeof(CESERVER_REQ_HDR)+sizeof(CESERVER_REQ_STARTSTOP);
						pGuiIn = ExecuteNewCmd(CECMD_CMDSTARTSTOP, nSize);

						if (!pGuiIn)
						{
							_ASSERTE(pGuiIn!=NULL && "Memory allocation failed");
						}
						else
						{
							pGuiIn->StartStop.dwPID = nAltServerWasStarted ? nAltServerWasStarted : nAltServerWasStopped;
							pGuiIn->StartStop.hServerProcessHandle = NULL; // ��� GUI ������ �� �����
							pGuiIn->StartStop.nStarted = nAltServerWasStarted ? sst_AltServerStart : sst_AltServerStop;

							pGuiOut = ExecuteGuiCmd(ghConWnd, pGuiIn, ghConWnd);

							_ASSERTE(pGuiOut!=NULL && "Can not switch GUI to alt server?"); // �������� ����������?
							ExecuteFreeResult(pGuiIn);
							ExecuteFreeResult(pGuiOut);
						}
					}
				}

				// ����� �������
				nAltWait = WAIT_OBJECT_0;
			}
			else if (nAltWait == (WAIT_OBJECT_0+2))
			{
				// ����������� ����������� ������
				break;
			}
			#ifdef _DEBUG
			else
			{
            	// ����������� ��������� WaitForMultipleObjects
				_ASSERTE(nAltWait==WAIT_OBJECT_0 || nAltWait==WAIT_TIMEOUT);
			}
			#endif
		}
		else
		{
			nAltWait = WAIT_OBJECT_0;
		}

		if (gpSrv->hCloseAltServer)
		{
			// ����� �� ��������� ����� �������� - ��������� ����� ������ �����
			if (gpSrv->hCloseAltServer != gpSrv->hAltServer)
			{
				SafeCloseHandle(gpSrv->hCloseAltServer);
			}
			else
			{
				gpSrv->hCloseAltServer = NULL;
			}
		}

		// Always update con handle, ������ �������
		// !!! � Win7 �������� ����������� � ������ �������� - ��������� ���������� ����� ���������. � �����, ����� ������ telnet'� ������������! !!!
		// 120507 - ���� �������� ����.������ - �� ������������
		if (gpSrv->bReopenHandleAllowed
			&& !nAltWait
			&& ((GetTickCount() - nLastConHandleTick) > UPDATECONHANDLE_TIMEOUT))
		{
			ghConOut.Close();
			nLastConHandleTick = GetTickCount();
		}

		//// ������� ��������� CECMD_SETCONSOLECP
		//if (gpSrv->hLockRefreshBegin)
		//{
		//	// ���� ������� ������� ���������� ���������� -
		//	// ����� ���������, ���� ��� (hLockRefreshBegin) ����� ����������
		//	SetEvent(gpSrv->hLockRefreshReady);
		//
		//	while(gpSrv->hLockRefreshBegin
		//	        && WaitForSingleObject(gpSrv->hLockRefreshBegin, 10) == WAIT_TIMEOUT)
		//		SetEvent(gpSrv->hLockRefreshReady);
		//}

		#ifdef _DEBUG
		if (nAltWait == WAIT_TIMEOUT)
		{
			// ���� �������� ����.������ - �� ������ �� ��������� ������� ���� ��������� �� ������
			_ASSERTE(!gpSrv->nRequestChangeSize);
		}
		#endif

		// �� ������ ���� �������� ������ �� ��������� ������� �������
		// 120507 - ���� �������� ����.������ - �� ������������
		if (!nAltWait && gpSrv->nRequestChangeSize)
		{
			// AVP ������... �� ����� � �� �����
			//DWORD dwSusp = 0, dwSuspErr = 0;
			//if (gpSrv->hRootThread) {
			//	WARNING("A 64-bit application can suspend a WOW64 thread using the Wow64SuspendThread function");
			//	// The handle must have the THREAD_SUSPEND_RESUME access right
			//	dwSusp = SuspendThread(gpSrv->hRootThread);
			//	if (dwSusp == (DWORD)-1) dwSuspErr = GetLastError();
			//}
			SetConsoleSize(gpSrv->nReqSizeBufferHeight, gpSrv->crReqSizeNewSize, gpSrv->rReqSizeNewRect, gpSrv->sReqSizeLabel);
			//if (gpSrv->hRootThread) {
			//	ResumeThread(gpSrv->hRootThread);
			//}
			// ������� �������� ����� ��������� ������������� �������
			lbWasSizeChange = TRUE;
			//SetEvent(gpSrv->hReqSizeChanged);
		}

		// ��������� ���������� ��������� � �������.
		// ������� �������� ghExitQueryEvent, ���� ��� �������� �����������.
		//lbProcessChanged = CheckProcessCount();
		// ������� ����������� ������ ����� �������� CHECK_PROCESSES_TIMEOUT (������ ������ �� ������ �������)
		// #define CHECK_PROCESSES_TIMEOUT 500
		CheckProcessCount();

		// ��������� ��������
		if (gpSrv->nMaxFPS>0)
		{
			dwTimeout = 1000 / gpSrv->nMaxFPS;

			// ���� 50, ����� �� ������������� ������� ��� �� ������� ���������� ("dir /s" � �.�.)
			if (dwTimeout < 10) dwTimeout = 10;
		}
		else
		{
			dwTimeout = 100;
		}

		// !!! ����� ������� ������ ���� �����������, �� ����� ��� ������� ���������
		//		HANDLE hOut = (HANDLE)ghConOut;
		//		if (hOut == INVALID_HANDLE_VALUE) hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		//		TODO("���������, � ���������� ��������� �� ��������� � �������?");
		//		-- �� ��������� (������) � ������� �� ���������
		//		dwConWait = WaitForSingleObject ( hOut, dwTimeout );
		//#ifdef _DEBUG
		//		if (dwConWait == WAIT_OBJECT_0) {
		//			DEBUGSTRCHANGES(L"STD_OUTPUT_HANDLE was set\n");
		//		}
		//#endif
		//
		if (lbWasSizeChange)
			nWait = (WAIT_OBJECT_0+1); // ��������� ���������� ������� ����� ��������� �������!
		else
			nWait = WaitForMultipleObjects(2, hEvents, FALSE, dwTimeout/*dwTimeout*/);

		if (nWait == WAIT_OBJECT_0)
		{
			break; // ����������� ���������� ����
		}// else if (nWait == WAIT_TIMEOUT && dwConWait == WAIT_OBJECT_0) {

		//	nWait = (WAIT_OBJECT_0+1);
		//}

		//lbEventualChange = (nWait == (WAIT_OBJECT_0+1))/* || lbProcessChanged*/;
		//lbForceSend = (nWait == (WAIT_OBJECT_0+1));

		WARNING("gpSrv->pConsole->hdr.bConsoleActive � gpSrv->pConsole->hdr.bThawRefreshThread ����� ���� �������������!");
		//if (gpSrv->pConsole->hdr.bConsoleActive && gpSrv->pConsoleMap)
		//{
		BOOL bNewThaw = TRUE;

		if (gpSrv->pConsoleMap->IsValid())
		{
			CESERVER_CONSOLE_MAPPING_HDR* p = gpSrv->pConsoleMap->Ptr();
			bNewThaw = p->bThawRefreshThread;
			bConsoleActive = p->bConsoleActive;
		}
		else
		{
			bNewThaw = bConsoleActive = TRUE;
		}
		//}

		if (bNewThaw != bThaw)
		{
			bThaw = bNewThaw;

			if (gpLogSize)
			{
				char szInfo[128];
				_wsprintfA(szInfo, SKIPLEN(countof(szInfo)) "ConEmuC: RefreshThread: Thaw changed, speed(%s)", bNewThaw ? "high" : "low");
				LogString(szInfo);
			}
		}


		// ����� �� ������� ��������� ����������� ��������� ����, ����
		// ������ ��� �� ���� ����������� ��������� ������� �������
		if (!lbWasSizeChange
		        // �� ��������� �������������� �������������
		        && !gpSrv->bForceConsoleRead
		        // ������� �� �������
		        && (!bConsoleActive
		            // ��� �������, �� ��� ConEmu GUI �� � ������
		            || (bConsoleActive && !bThaw))
		        // � �� ������� ������� gpSrv->hRefreshEvent
		        && (nWait != (WAIT_OBJECT_0+1))
				&& !gpSrv->bWasReattached)
		{
			DWORD nCurTick = GetTickCount();
			nDelta = nCurTick - nLastReadTick;

			// #define MAX_FORCEREFRESH_INTERVAL 500
			if (nDelta <= MAX_FORCEREFRESH_INTERVAL)
			{
				// ����� �� ������� ���������
				continue;
			}
		}

#ifdef _DEBUG

		if (nWait == (WAIT_OBJECT_0+1))
		{
			DEBUGSTR(L"*** hRefreshEvent was set, checking console...\n");
		}

#endif

		if (ghConEmuWnd && !IsWindow(ghConEmuWnd))
		{
			gpSrv->bWasDetached = TRUE;
			ghConEmuWnd = NULL;
			SetConEmuWindows(NULL, NULL);
			gpSrv->dwGuiPID = 0;
			UpdateConsoleMapHeader();
			EmergencyShow(ghConWnd);
		}

		// 17.12.2009 Maks - �������� ������
		//if (ghConEmuWnd && GetForegroundWindow() == ghConWnd) {
		//	if (lbFirstForeground || !IsWindowVisible(ghConWnd)) {
		//		DEBUGSTR(L"...apiSetForegroundWindow(ghConEmuWnd);\n");
		//		apiSetForegroundWindow(ghConEmuWnd);
		//		lbFirstForeground = FALSE;
		//	}
		//}

		// ���� ����� - �������� ������� ��������� � �������
		// 120507 - ���� �������� ����.������ - �� ������������
		if (!nAltWait && !gpSrv->bDebuggerActive)
		{
			if (pfnGetConsoleKeyboardLayoutName)
				CheckKeyboardLayout();
		}

		/* ****************** */
		/* ���������� ������� */
		/* ****************** */
		// 120507 - ���� �������� ����.������ - �� ������������
		if (!nAltWait && !gpSrv->bDebuggerActive)
		{
			bool lbReloadNow = true;
			#if defined(TEST_REFRESH_DELAYED)
			static DWORD nDbgTick = 0;
			const DWORD nMax = 1000;
			HANDLE hOut = (HANDLE)ghConOut;
			DWORD nWaitOut = WaitForSingleObject(hOut, 0);
			DWORD nCurTick = GetTickCount();
			if ((nWaitOut == WAIT_OBJECT_0) || (nCurTick - nDbgTick) >= nMax)
			{
				nDbgTick = nCurTick;
			}
			else
			{
				lbReloadNow = false;
				lbChanged = FALSE;
			}
			//ShutdownSrvStep(L"ReloadFullConsoleInfo begin");
			#endif
			
			if (lbReloadNow)
			{
				lbChanged = ReloadFullConsoleInfo(gpSrv->bWasReattached/*lbForceSend*/);
			}

			#if defined(TEST_REFRESH_DELAYED)
			//ShutdownSrvStep(L"ReloadFullConsoleInfo end (%u,%u)", (int)lbReloadNow, (int)lbChanged);
			#endif

			// ��� ���� ������ ������������� gpSrv->hDataReadyEvent
			if (gpSrv->bWasReattached)
			{
				_ASSERTE(lbChanged);
				_ASSERTE(gpSrv->pConsole && gpSrv->pConsole->bDataChanged);
				gpSrv->bWasReattached = FALSE;
			}
		}
		else
		{
			lbChanged = FALSE;
		}

		// ������� �������� ����� ��������� ������������� �������
		if (lbWasSizeChange)
		{
			SetEvent(gpSrv->hReqSizeChanged);
			lbWasSizeChange = FALSE;
		}

		if (nWait == (WAIT_OBJECT_0+1))
			SetEvent(gpSrv->hRefreshDoneEvent);

		// ��������� ��������� tick
		//if (lbChanged)
		nLastReadTick = GetTickCount();
		MCHKHEAP
	}

	return 0;
}



int MySetWindowRgn(CESERVER_REQ_SETWINDOWRGN* pRgn)
{
	if (!pRgn)
	{
		_ASSERTE(pRgn!=NULL);
		return 0; // Invalid argument!
	}

	if (pRgn->nRectCount == 0)
	{
		return SetWindowRgn((HWND)pRgn->hWnd, NULL, pRgn->bRedraw);
	}
	else if (pRgn->nRectCount == -1)
	{
		apiShowWindow((HWND)pRgn->hWnd, SW_HIDE);
		return SetWindowRgn((HWND)pRgn->hWnd, NULL, FALSE);
	}

	// ����� �������...
	HRGN hRgn = NULL, hSubRgn = NULL, hCombine = NULL;
	BOOL lbPanelVisible = TRUE;
	hRgn = CreateRectRgn(pRgn->rcRects->left, pRgn->rcRects->top, pRgn->rcRects->right, pRgn->rcRects->bottom);

	for(int i = 1; i < pRgn->nRectCount; i++)
	{
		RECT rcTest;

		// IntersectRect �� ��������, ���� ��� ���������?
		_ASSERTE(pRgn->rcRects->bottom != (pRgn->rcRects+i)->bottom);
		if (!IntersectRect(&rcTest, pRgn->rcRects, pRgn->rcRects+i))
			continue;

		// ��� ��������� �������������� �������� �� hRgn
		hSubRgn = CreateRectRgn(rcTest.left, rcTest.top, rcTest.right, rcTest.bottom);

		if (!hCombine)
			hCombine = CreateRectRgn(0,0,1,1);

		int nCRC = CombineRgn(hCombine, hRgn, hSubRgn, RGN_DIFF);

		if (nCRC)
		{
			// ������ ����������
			HRGN hTmp = hRgn; hRgn = hCombine; hCombine = hTmp;
			// � ���� ������ �� �����
			DeleteObject(hSubRgn); hSubRgn = NULL;
		}

		// ��������, ����� ������ ��� ����?
		if (nCRC == NULLREGION)
		{
			lbPanelVisible = FALSE; break;
		}
	}

	int iRc = 0;
	SetWindowRgn((HWND)pRgn->hWnd, hRgn, TRUE); hRgn = NULL;

	// ������
	if (hCombine) { DeleteObject(hCombine); hCombine = NULL; }

	return iRc;
}
