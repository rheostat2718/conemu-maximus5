
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

#include "ConEmuC.h"
#include "../common/ConsoleAnnotation.h"
#include "../common/Execute.h"
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
		if (ghLogSize) LogSize(NULL, ":SetConsoleFontSizeTo.before");

		SetConsoleFontSizeTo(ghConWnd, gpSrv->nConFontHeight, gpSrv->nConFontWidth, gpSrv->szConsoleFont);

		if (ghLogSize) LogSize(NULL, ":SetConsoleFontSizeTo.after");
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

		SetEnvironmentVariableW(L"ConEmuDir", gpSrv->guiSettings.sConEmuDir);
		SetEnvironmentVariableW(L"ConEmuBaseDir", gpSrv->guiSettings.sConEmuBaseDir);

		// �� ����� ������� ����, ��� ���������� ��������� Gui ��� ����� �������
		// ��������������, ���������� ����������� ���������
		//SetEnvironmentVariableW(L"ConEmuArgs", pInfo->sConEmuArgs);

		wchar_t szHWND[16]; _wsprintf(szHWND, SKIPLEN(countof(szHWND)) L"0x%08X", gpSrv->guiSettings.hGuiWnd.u);
		SetEnvironmentVariableW(L"ConEmuHWND", szHWND);

		if (gpSrv->pConsole)
		{
			// !!! Warning !!! ������� �����, ������� � CreateMapHeader() !!!
			
			gpSrv->pConsole->hdr.nLoggingType = gpSrv->guiSettings.nLoggingType;
			gpSrv->pConsole->hdr.bDosBox = gpSrv->guiSettings.bDosBox;
			gpSrv->pConsole->hdr.bUseInjects = gpSrv->guiSettings.bUseInjects;
			gpSrv->pConsole->hdr.bUseTrueColor = gpSrv->guiSettings.bUseTrueColor;
		
			// �������� ���� � ConEmu
			wcscpy_c(gpSrv->pConsole->hdr.sConEmuExe, gpSrv->guiSettings.sConEmuExe);
			wcscpy_c(gpSrv->pConsole->hdr.sConEmuBaseDir, gpSrv->guiSettings.sConEmuBaseDir);

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

								if (!dwFarPID && lstrcmpiW(prc.szExeFile, L"far.exe")==0)
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
		wchar_t szSelf[MAX_PATH+100];
		wchar_t* pszSelf = szSelf+1;

		if (!GetModuleFileName(NULL, pszSelf, MAX_PATH))
		{
			dwErr = GetLastError();
			_printf("GetModuleFileName failed, ErrCode=0x%08X\n", dwErr);
			SetLastError(dwErr);
			return CERR_CREATEPROCESS;
		}

		if (wcschr(pszSelf, L' '))
		{
			*(--pszSelf) = L'"';
			lstrcatW(pszSelf, L"\"");
		}

		wsprintf(pszSelf+lstrlen(pszSelf), L" /ATTACH /PID=%i", dwParentPID);
		PROCESS_INFORMATION pi; memset(&pi, 0, sizeof(pi));
		STARTUPINFOW si; memset(&si, 0, sizeof(si)); si.cb = sizeof(si);
		PRINT_COMSPEC(L"Starting modeless:\n%s\n", pszSelf);
		// CREATE_NEW_PROCESS_GROUP - ����, ��������� �������� Ctrl-C
		// ��� ������ ������ ������� � ���� �������. � ������ ���� ������� �� �����
		BOOL lbRc = createProcess(TRUE, NULL, pszSelf, NULL,NULL, TRUE,
		                           NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi);
		dwErr = GetLastError();

		if (!lbRc)
		{
			PrintExecuteError(pszSelf, dwErr);
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
		// ����� ������� HANDLE ��������� ��������
		DWORD dwFlags = PROCESS_QUERY_INFORMATION|SYNCHRONIZE;

		if (gpSrv->bDebuggerActive)
			dwFlags |= PROCESS_VM_READ;

		CAdjustProcessToken token;
		token.Enable(1, SE_DEBUG_NAME);

		// PROCESS_ALL_ACCESS may fails on WinXP!
		gpSrv->hRootProcess = OpenProcess((STANDARD_RIGHTS_REQUIRED|SYNCHRONIZE|0xFFF), FALSE, gpSrv->dwRootProcess);
		if (!gpSrv->hRootProcess)
			gpSrv->hRootProcess = OpenProcess(dwFlags, FALSE, gpSrv->dwRootProcess);

		token.Release();

		if (!gpSrv->hRootProcess)
		{
			dwErr = GetLastError();
			wchar_t* lpMsgBuf = NULL;
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwErr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&lpMsgBuf, 0, NULL);
			_printf("Can't open process (%i) handle, ErrCode=0x%08X, Description:\n", //-V576
			        gpSrv->dwRootProcess, dwErr, (lpMsgBuf == NULL) ? L"<Unknown error>" : lpMsgBuf);

			if (lpMsgBuf) LocalFree(lpMsgBuf);
			SetLastError(dwErr);

			return CERR_CREATEPROCESS;
		}

		if (gpSrv->bDebuggerActive)
		{
			wchar_t szTitle[64];
			_wsprintf(szTitle, SKIPLEN(countof(szTitle)) L"Debugging PID=%u, Debugger PID=%u", gpSrv->dwRootProcess, GetCurrentProcessId());
			SetConsoleTitleW(szTitle);
		}
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

int ServerInitCheckExisting(BOOL abAlternative)
{
	int iRc = 0;
	CESERVER_CONSOLE_MAPPING_HDR test = {};

	BOOL lbExist = LoadSrvMapping(ghConWnd, test);
	if (abAlternative == FALSE)
	{
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
		// �� ����, � ������� ������ ���� _�����_ ������.
		_ASSERTE(lbExist && test.nServerPID != 0);
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

	while(true)
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

		#ifdef _DEBUG
		DWORD nStartTick = timeGetTime();
		#endif

		lbCallRc = CallNamedPipe(szServerPipe, &In, In.hdr.cbSize, &Out, sizeof(Out), &dwRead, 1000);

		#ifdef _DEBUG
		DWORD dwErr = GetLastError(), nEndTick = timeGetTime(), nDelta = nEndTick - nStartTick;
		if (!lbCallRc || nDelta >= EXECUTE_CMD_WARN_TIMEOUT)
		{
			if (!IsDebuggerPresent())
			{
				//_ASSERTE(nDelta <= EXECUTE_CMD_WARN_TIMEOUT || (pIn->hdr.nCmd == CECMD_CMDSTARTSTOP && nDelta <= EXECUTE_CMD_WARN_TIMEOUT2));
				_ASSERTEX(nDelta <= EXECUTE_CMD_WARN_TIMEOUT);
			}
		}
		#endif


		if (!lbCallRc || !Out.StartStopRet.hWndDC)
		{
			dwErr = GetLastError();
			#ifdef _DEBUG
			_ASSERTEX(lbCallRc && Out.StartStopRet.hWndDC);
			SetLastError(dwErr);
			#endif
		}
		else
		{
			ghConEmuWnd = Out.StartStop.hWnd;
			ghConEmuWndDC = Out.StartStopRet.hWndDC;
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
		DEBUGTEST(DWORD t1 = timeGetTime());

		nWaitRc = WaitForSingleObject(gpSrv->hConEmuGuiAttached, 500);

		#ifdef _DEBUG
		DWORD t2 = timeGetTime(), tDur = t2-t1;
		if (tDur > GUIATTACHEVENT_TIMEOUT)
		{
			_ASSERTE(tDur <= GUIATTACHEVENT_TIMEOUT);
		}
		#endif
	}

	CheckConEmuHwnd();

	return iRc;
}


// ������� ����������� ������� � ����
int ServerInit(BOOL abAlternative/*=FALSE*/)
{
	int iRc = 0;
	DWORD dwErr = 0;
	// ����� ���������� ��������� ���� ������� ���� "OnTop"
	SetWindowPos(ghConWnd, HWND_TOPMOST, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE);
	gpSrv->osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&gpSrv->osv);

	// ������ ����� �� �����, ��� �������� "�������" ������� ����� "������������ ������� �������
	// ������������� ������� �� ��������, ������� ���� ������ � �������� ��������
	//InitializeConsoleInputSemaphore();

	if (gpSrv->osv.dwMajorVersion == 6 && gpSrv->osv.dwMinorVersion == 1)
		gpSrv->bReopenHandleAllowed = FALSE;
	else
		gpSrv->bReopenHandleAllowed = TRUE;

	if (!gnConfirmExitParm)
	{
		gbAlwaysConfirmExit = TRUE; gbAutoDisableConfirmExit = TRUE;
	}

	_ASSERTE(gpcsStoredOutput==NULL && gpStoredOutput==NULL);
	if (!gpcsStoredOutput)
	{
		gpcsStoredOutput = new MSection;
	}

	// ����� � ������� ����� ������ � ����� ������, ����� ����� ���� �������� � ���������� ������� �������
	if (!gpSrv->bDebuggerActive && !gbNoCreateProcess)
		//&& (!gbNoCreateProcess || (gbAttachMode && gbNoCreateProcess && gpSrv->dwRootProcess))
		//)
	{
		ServerInitFont();
		// -- ����� �� ��������� �������� �� ��������� �������� � ����������������� -> {0,0}
		// Issue 274: ���� �������� ������� ��������������� � ��������� �����
		SetWindowPos(ghConWnd, NULL, 0, 0, 0,0, SWP_NOSIZE|SWP_NOZORDER);
	}

	// �������� �� ��������� ��������� �����
	if (!gbNoCreateProcess && gbConsoleModeFlags /*&& !(gbParmBufferSize && gnBufferHeight == 0)*/)
		ServerInitConsoleMode();

	//2009-08-27 ������� �����
	if (!gpSrv->hConEmuGuiAttached)
	{
		wchar_t szTempName[MAX_PATH];
		_wsprintf(szTempName, SKIPLEN(countof(szTempName)) CEGUIRCONSTARTED, (DWORD)ghConWnd); //-V205
		//gpSrv->hConEmuGuiAttached = OpenEvent(EVENT_ALL_ACCESS, FALSE, szTempName);
		//if (gpSrv->hConEmuGuiAttached == NULL)
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

	if (ghConEmuWndDC == NULL)
	{
		_ASSERTE(!abAlternative || ghConEmuWndDC!=NULL);
	}
	else
	{
		iRc = ServerInitCheckExisting(abAlternative);
		if (iRc != 0)
			goto wrap;
	}

	// ������� MapFile ��� ��������� (�����!!!) � ������ ��� ������ � ���������
	iRc = CreateMapHeader();

	if (iRc != 0)
		goto wrap;

	_ASSERTE((ghConEmuWndDC==NULL) || (gpSrv->pColorerMapping!=NULL));

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
	gpSrv->nMaxInputQueue = 255;
	gpSrv->pInputQueue = (INPUT_RECORD*)calloc(gpSrv->nMaxInputQueue, sizeof(INPUT_RECORD));
	gpSrv->pInputQueueEnd = gpSrv->pInputQueue+gpSrv->nMaxInputQueue;
	gpSrv->pInputQueueWrite = gpSrv->pInputQueue;
	gpSrv->pInputQueueRead = gpSrv->pInputQueueEnd;
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
	if (gbNoCreateProcess && (gbAttachMode || gpSrv->bDebuggerActive))
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

	// Minimized ������ ����� ����������!
	if (IsIconic(ghConWnd))
	{
		WINDOWPLACEMENT wplCon = {sizeof(wplCon)};
		GetWindowPlacement(ghConWnd, &wplCon);
		wplCon.showCmd = SW_RESTORE;
		SetWindowPlacement(ghConWnd, &wplCon);
	}

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

	if (gbAttachMode)
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
		if (!nExitQueryPlace) nExitQueryPlace = 10+(nExitPlaceStep+nExitPlaceThread);

		SetEvent(ghExitQueryEvent);
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

	//// ������ ������� ����� �� ��� ����, � ����� ����� �����
	//#ifdef USE_WINEVENT_SRV
	//if (gpSrv->dwWinEventThread && gpSrv->hWinEventThread)
	//	PostThreadMessage(gpSrv->dwWinEventThread, WM_QUIT, 0, 0);
	//#endif

	//if (gpSrv->dwInputThreadId && gpSrv->hInputThread)
	//	PostThreadMessage(gpSrv->dwInputThreadId, WM_QUIT, 0, 0);
	// ����������� ���� ��������� ����
	//HANDLE hPipe = CreateFile(gpSrv->szPipename,GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);

	//if (hPipe == INVALID_HANDLE_VALUE)
	//{
	//	DEBUGSTR(L"All pipe instances closed?\n");
	//}

	//// ����������� ���� �����
	//if (gpSrv->hInputPipe && gpSrv->hInputPipe != INVALID_HANDLE_VALUE)
	//{
	//	//DWORD dwSize = 0;
	//	//BOOL lbRc = WriteFile(gpSrv->hInputPipe, &dwSize, sizeof(dwSize), &dwSize, NULL);
	//	/*BOOL lbRc = DisconnectNamedPipe(gpSrv->hInputPipe);
	//	CloseHandle(gpSrv->hInputPipe);
	//	gpSrv->hInputPipe = NULL;*/
	//	TODO("������ �� ��������, ������ ����� �� Overlapped ����������");
	//}

	//HANDLE hInputPipe = CreateFile(gpSrv->szInputname,GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
	//if (hInputPipe == INVALID_HANDLE_VALUE) {
	//	DEBUGSTR(L"Input pipe was not created?\n");
	//} else {
	//	MSG msg = {NULL}; msg.message = 0xFFFF; DWORD dwOut = 0;
	//	WriteFile(hInputPipe, &msg, sizeof(msg), &dwOut, 0);
	//}

	//#ifdef USE_WINEVENT_SRV
	//// ��������� ����������� � �������
	//if (/*gpSrv->dwWinEventThread &&*/ gpSrv->hWinEventThread)
	//{
	//	// �������� ��������, ���� ���� ���� ����������
	//	if (WaitForSingleObject(gpSrv->hWinEventThread, 500) != WAIT_OBJECT_0)
	//	{
	//		gbTerminateOnExit = gpSrv->bWinEventTermination = TRUE;
	//		#ifdef _DEBUG
	//			// ���������, �� ������� �� Assert-�� � ������ �������
	//			MyAssertShutdown();
	//		#endif

	//		#ifndef __GNUC__
	//		#pragma warning( push )
	//		#pragma warning( disable : 6258 )
	//		#endif
	//		TerminateThread(gpSrv->hWinEventThread, 100);    // ��� ��������� �� �����...
	//		#ifndef __GNUC__
	//		#pragma warning( pop )
	//		#endif
	//	}

	//	SafeCloseHandle(gpSrv->hWinEventThread);
	//	//gpSrv->dwWinEventThread = 0; -- �� ����� ������� ��, ��� �������
	//}
	//#endif

	if (gpSrv->hInputThread)
	{
		// �������� ��������, ���� ���� ���� ����������
		WARNING("�� �����������");

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
	//if (gpSrv->hInputPipeThread)
	//{
	//	// �������� ��������, ���� ���� ���� ����������
	//	if (WaitForSingleObject(gpSrv->hInputPipeThread, 50) != WAIT_OBJECT_0)
	//	{
	//		gbTerminateOnExit = gpSrv->bInputPipeTermination = TRUE;
	//		#ifdef _DEBUG
	//			// ���������, �� ������� �� Assert-�� � ������ �������
	//			MyAssertShutdown();
	//		#endif
	//#ifndef __GNUC__
	//#pragma warning( push )
	//#pragma warning( disable : 6258 )
	//#endif
	//		TerminateThread(gpSrv->hInputPipeThread, 100);    // ��� ��������� �� �����...
	//#ifndef __GNUC__
	//#pragma warning( pop )
	//#endif
	//	}
	//
	//	SafeCloseHandle(gpSrv->hInputPipeThread);
	//	//gpSrv->dwInputPipeThreadId = 0; -- �� ����� ������� ��, ��� �������
	//}

	//SafeCloseHandle(gpSrv->hInputPipe);
	SafeCloseHandle(gpSrv->hInputEvent);

	if (gpSrv)
		gpSrv->DataServer.StopPipeServer();
	//if (gpSrv->hGetDataPipeThread)
	//{
	//	// �������� ��������, ���� ���� ���� ����������
	//	TODO("���� ���� �� ����������. ����� ���� ������ overlapped, ���� ���-�� ������ � ����");
	//	//if (WaitForSingleObject(gpSrv->hGetDataPipeThread, 50) != WAIT_OBJECT_0)
	//	{
	//		gbTerminateOnExit = gpSrv->bGetDataPipeTermination = TRUE;
	//		#ifdef _DEBUG
	//			// ���������, �� ������� �� Assert-�� � ������ �������
	//			MyAssertShutdown();
	//		#endif
	//#ifndef __GNUC__
	//#pragma warning( push )
	//#pragma warning( disable : 6258 )
	//#endif
	//		TerminateThread(gpSrv->hGetDataPipeThread, 100);    // ��� ��������� �� �����...
	//#ifndef __GNUC__
	//#pragma warning( pop )
	//#endif
	//	}
	//	SafeCloseHandle(gpSrv->hGetDataPipeThread);
	//	gpSrv->dwGetDataPipeThreadId = 0;
	//}

	if (gpSrv)
		gpSrv->CmdServer.StopPipeServer();
	//if (gpSrv->hServerThread)
	//{
	//	// �������� ��������, ���� ���� ���� ����������
	//	if (WaitForSingleObject(gpSrv->hServerThread, 500) != WAIT_OBJECT_0)
	//	{
	//		gbTerminateOnExit = gpSrv->bServerTermination = TRUE;
	//		#ifdef _DEBUG
	//			// ���������, �� ������� �� Assert-�� � ������ �������
	//			MyAssertShutdown();
	//		#endif
	//#ifndef __GNUC__
	//#pragma warning( push )
	//#pragma warning( disable : 6258 )
	//#endif
	//		TerminateThread(gpSrv->hServerThread, 100);    // ��� ��������� �� �����...
	//#ifndef __GNUC__
	//#pragma warning( pop )
	//#endif
	//	}
	//	SafeCloseHandle(gpSrv->hServerThread);
	//}

	//SafeCloseHandle(hPipe);

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

	if (gpSrv->hRefreshEvent)
	{
		SafeCloseHandle(gpSrv->hRefreshEvent);
	}

	if (gpSrv->hRefreshDoneEvent)
	{
		SafeCloseHandle(gpSrv->hRefreshDoneEvent);
	}

	if (gpSrv->hDataReadyEvent)
	{
		SafeCloseHandle(gpSrv->hDataReadyEvent);
	}

	//if (gpSrv->hChangingSize) {
	//    SafeCloseHandle(gpSrv->hChangingSize);
	//}
	// ��������� ��� ����
	//gpSrv->bWinHookAllow = FALSE; gpSrv->nWinHookMode = 0;
	//HookWinEvents ( -1 );

	{
		MSectionLock CS; CS.Lock(gpcsStoredOutput, TRUE);
		SafeFree(gpStoredOutput);
		CS.Unlock();
		SafeDelete(gpcsStoredOutput);
	}

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

	if (gpSrv->csProc)
	{
		//WARNING("����������! ������ �������� ��� �������� �������");
		//�������� new & delete, ���������
		delete gpSrv->csProc;
		gpSrv->csProc = NULL;
	}

	if (gpSrv->pnProcesses)
	{
		free(gpSrv->pnProcesses); gpSrv->pnProcesses = NULL;
	}

	if (gpSrv->pnProcessesGet)
	{
		free(gpSrv->pnProcessesGet); gpSrv->pnProcessesGet = NULL;
	}

	if (gpSrv->pnProcessesCopy)
	{
		free(gpSrv->pnProcessesCopy); gpSrv->pnProcessesCopy = NULL;
	}

	if (gpSrv->pInputQueue)
	{
		free(gpSrv->pInputQueue); gpSrv->pInputQueue = NULL;
	}

	CloseMapHeader();

	//SafeCloseHandle(gpSrv->hColorerMapping);
	if (gpSrv->pColorerMapping)
	{
		delete gpSrv->pColorerMapping;
		gpSrv->pColorerMapping = NULL;
	}
}



// ��������� ������ ���� ������� � gpStoredOutput
void CmdOutputStore()
{
	// � Win7 �������� ����������� � ������ �������� - ��������� ���������� ����� ���������!!!
	// � �����, ����� ������ telnet'� ������������!
	if (gpSrv->bReopenHandleAllowed)
	{
		ghConOut.Close();
	}

	WARNING("� ��� ��� ����� �� ������ � RefreshThread!!!");
	DEBUGSTR(L"--- CmdOutputStore begin\n");
	CONSOLE_SCREEN_BUFFER_INFO lsbi = {{0,0}};

	MSectionLock CS; CS.Lock(gpcsStoredOutput, FALSE);

	// !!! ��� ���������� �������� ��������� ��� � �������,
	//     � �� ����������������� �������� MyGetConsoleScreenBufferInfo
	if (!GetConsoleScreenBufferInfo(ghConOut, &lsbi))
	{
		CS.RelockExclusive();
		SafeFree(gpStoredOutput);

		return; // �� ������ �������� ���������� � �������...
	}

	int nOneBufferSize = lsbi.dwSize.X * lsbi.dwSize.Y * 2; // ������ ��� ������� �������!

	// ���� ��������� ���������� ������� ���������� ������
	if (gpStoredOutput)
	{
		if (gpStoredOutput->hdr.cbMaxOneBufferSize < (DWORD)nOneBufferSize)
		{
			CS.RelockExclusive();
			SafeFree(gpStoredOutput);
		}
	}

	if (gpStoredOutput == NULL)
	{
		CS.RelockExclusive();
		// �������� ������: ��������� + ����� ������ (�� �������� ������)
		gpStoredOutput = (CESERVER_CONSAVE*)calloc(sizeof(CESERVER_CONSAVE_HDR)+nOneBufferSize,1);
		_ASSERTE(gpStoredOutput!=NULL);

		if (gpStoredOutput == NULL)
			return; // �� ������ �������� ������

		gpStoredOutput->hdr.cbMaxOneBufferSize = nOneBufferSize;
	}

	// ��������� sbi
	//memmove(&gpStoredOutput->hdr.sbi, &lsbi, sizeof(lsbi));
	gpStoredOutput->hdr.sbi = lsbi;
	// ������ ������ ������
	COORD coord = {0,0};
	DWORD nbActuallyRead = 0;
	DWORD nReadLen = lsbi.dwSize.X * lsbi.dwSize.Y;

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

	if (!ReadConsoleOutputCharacter(ghConOut, gpStoredOutput->Data, nReadLen, coord, &nbActuallyRead)
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

	DEBUGSTR(L"--- CmdOutputStore end\n");
}

void CmdOutputRestore()
{
	MSectionLock CS; CS.Lock(gpcsStoredOutput, TRUE);
	if (gpStoredOutput)
	{
		TODO("������������ ����� ������� (������������ �����) ����� �������");
		// ������, ��� ������ ������� ����� ���������� �� ������� ���������� ���������� �������.
		// ������ � ��� � ������� ����� ������� ����� ���������� ������� ����������� ������ (����������� FAR).
		// 1) ���� ������� ����� �������
		// 2) ����������� � ������ ����� ������� (�� ������� ����������� ���������� �������)
		// 3) ���������� ������� �� ���������� ������� (���� �� ������ ��� ����������� ������ ������)
		// 4) ������������ ���������� ����� �������. ������, ��� ��� �����
		//    ��������� ��������� ������� ��� � ������ ������-�� ��� ��������� ������...
	}
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

void CheckConEmuHwnd()
{
	WARNING("����������, ��� ������� ����� ������� ��� ������ �������");

	//HWND hWndFore = GetForegroundWindow();
	//HWND hWndFocus = GetFocus();
	DWORD dwGuiThreadId = 0;

	if (gpSrv->bDebuggerActive)
	{
		ghConEmuWnd = FindConEmuByPID();

		if (ghConEmuWnd)
		{
			GetWindowThreadProcessId(ghConEmuWnd, &gpSrv->dwGuiPID);
			// ������ ��� ����������
			ghConEmuWndDC = FindWindowEx(ghConEmuWnd, NULL, VirtualConsoleClassMain, NULL);
		}
		else
		{
			ghConEmuWndDC = NULL;
		}

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
			ghConEmuWndDC = FindWindowEx(ghConEmuWnd, NULL, VirtualConsoleClassMain, NULL);
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

HWND Attach2Gui(DWORD nTimeout)
{
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
						if (lstrcmpiW(prc.szExeFile, L"conemu.exe")==0)
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

		lstrcpyW(pszSlash+1, L"ConEmu.exe");

		if (!FileExists(pszSelf))
		{
			// �� ����� ���� �� ������� ����
			*pszSlash = 0;
			pszSlash = wcsrchr(pszSelf, L'\\');
			lstrcpyW(pszSlash+1, L"ConEmu.exe");

			if (!FileExists(pszSelf))
			{
				_printf("ConEmu.exe not found!\n");
				return NULL;
			}
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
		IsNeedCmd(gpszRunCmd, &lbNeedCutQuot, pIn->StartStop.sModuleName, lbRootIsCmd, lbConfirmExit, lbAutoDisable);
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

	pIn->StartStop.crMaxSize = GetLargestConsoleWindowSize(hOut);

//LoopFind:
	// � ������� "���������" ������ ����� ��� ����������, � ��� ������
	// ������� �������� - ����� ���-����� �������� �� ���������
	//BOOL lbNeedSetFont = TRUE;
	// ����� ��������. ����� ��� ������...
	hGui = NULL;

	// ���� � ������� ���� �� ��������� (GUI ��� ��� �� �����������) ������� ���
	while(!hDcWnd && dwDelta <= nTimeout)
	{
		while((hGui = FindWindowEx(NULL, hGui, VirtualConsoleClassMain, NULL)) != NULL)
		{
			//if (lbNeedSetFont) {
			//	lbNeedSetFont = FALSE;
			//
			//    if (ghLogSize) LogSize(NULL, ":SetConsoleFontSizeTo.before");
			//    SetConsoleFontSizeTo(ghConWnd, gpSrv->nConFontHeight, gpSrv->nConFontWidth, gpSrv->szConsoleFont);
			//    if (ghLogSize) LogSize(NULL, ":SetConsoleFontSizeTo.after");
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
				ghConEmuWndDC = hDcWnd = pOut->StartStopRet.hWndDC;
				gpSrv->dwGuiPID = pOut->StartStopRet.dwPID;
				_ASSERTE(gpSrv->pConsoleMap != NULL); // ������� ��� ������ ���� ������,
				_ASSERTE(gpSrv->pConsole != NULL); // � ��������� ����� ����
				//gpSrv->pConsole->info.nGuiPID = pOut->StartStopRet.dwPID;
				CESERVER_CONSOLE_MAPPING_HDR *pMap = gpSrv->pConsoleMap->Ptr();
				if (pMap)
				{
					pMap->nGuiPID = pOut->StartStopRet.dwPID;
					pMap->hConEmuRoot = ghConEmuWnd;
					pMap->hConEmuWnd = ghConEmuWndDC;
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
				break;
			}
		}

		if (hDcWnd) break;

		dwCur = GetTickCount(); dwDelta = dwCur - dwStart;

		if (dwDelta > nTimeout) break;

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
	COORD crMax = GetLargestConsoleWindowSize(h);

	if (crMax.X < 80 || crMax.Y < 25)
	{
#ifdef _DEBUG
		DWORD dwErr = GetLastError();
		_ASSERTE(crMax.X >= 80 && crMax.Y >= 25);
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
	gpSrv->pConsole->hdr.nLogLevel = (ghLogSize!=NULL) ? 1 : 0;
	gpSrv->pConsole->hdr.crMaxConSize = crMax;
	gpSrv->pConsole->hdr.bDataReady = FALSE;
	gpSrv->pConsole->hdr.hConWnd = ghConWnd; _ASSERTE(ghConWnd!=NULL);
	gpSrv->pConsole->hdr.nServerPID = GetCurrentProcessId();
	gpSrv->pConsole->hdr.nGuiPID = gpSrv->dwGuiPID;
	gpSrv->pConsole->hdr.hConEmuRoot = ghConEmuWnd;
	gpSrv->pConsole->hdr.hConEmuWnd = ghConEmuWndDC;
	_ASSERTE(gpSrv->pConsole->hdr.hConEmuRoot==NULL || gpSrv->pConsole->hdr.nGuiPID!=0);
	gpSrv->pConsole->hdr.bConsoleActive = TRUE; // ���� - TRUE (��� �� ������ �������)
	gpSrv->pConsole->hdr.nServerInShutdown = 0;
	gpSrv->pConsole->hdr.bThawRefreshThread = TRUE; // ���� - TRUE (��� �� ������ �������)
	gpSrv->pConsole->hdr.nProtocolVersion = CESERVER_REQ_VER;
	gpSrv->pConsole->hdr.nActiveFarPID = gpSrv->nActiveFarPID; // PID ���������� ��������� ����

	// �������� ���������� ��������� (����� ConEmuGuiMapping)
	if (ghConEmuWnd) // ���� ��� �������� - ����� �����
		ReloadGuiSettings(NULL);

	//// � ������ Create GuiSettings ������ ����� ��� �� ���������, ����� ��������� � ReloadGuiSettings()
	//if (gpSrv->guiSettings.cbSize)
	//{
	//	// !!! Warning !!! ������� �����, ������� � ReloadGuiSettings() !!!
	//	gpSrv->pConsole->hdr.nLoggingType = gpSrv->guiSettings.nLoggingType;
	//	gpSrv->pConsole->hdr.bDosBox = gpSrv->guiSettings.bDosBox;
	//	gpSrv->pConsole->hdr.bUseInjects = gpSrv->guiSettings.bUseInjects;

	//	gpSrv->pConsole->hdr.bHookRegistry = gpSrv->guiSettings.bHookRegistry;
	//	wcscpy_c(gpSrv->pConsole->hdr.sHiveFileName, gpSrv->guiSettings.sHiveFileName);
	//	//gpSrv->pConsole->hdr.hMountRoot = gpSrv->guiSettings.hMountRoot;
	//	wcscpy_c(gpSrv->pConsole->hdr.sMountKey, gpSrv->guiSettings.sMountKey);
	//
	//	wcscpy_c(gpSrv->pConsole->hdr.sConEmuExe, gpSrv->guiSettings.sConEmuExe);
	//	wcscpy_c(gpSrv->pConsole->hdr.sConEmuBaseDir, gpSrv->guiSettings.sConEmuBaseDir);
	//	// !!! Warning !!! ������� �����, ������� � ReloadGuiSettings() !!!
	//}
	//else
	//{
	//	_ASSERTE(gpSrv->guiSettings.cbSize!=0);
	//	// -- calloc, �� ���������
	//	//gpSrv->pConsole->hdr.sConEmuExe[0] = gpSrv->pConsole->hdr.sConEmuBaseDir[0] = 0;
	//}

	//gpSrv->pConsole->hdr.hConEmuWnd = ghConEmuWnd; -- ��������� UpdateConsoleMapHeader
	//WARNING! � ������ ��������� info ���� CESERVER_REQ_HDR ��� ���������� ������� ����� �����
	gpSrv->pConsole->info.cmd.cbSize = sizeof(gpSrv->pConsole->info); // ���� ��� - ������ ������ ���������
	gpSrv->pConsole->info.hConWnd = ghConWnd; _ASSERTE(ghConWnd!=NULL);
	//gpSrv->pConsole->info.nGuiPID = gpSrv->dwGuiPID;
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

	if (!gpSrv->pConsoleMap->Create())
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
	if (gpSrv && gpSrv->pConsole)
	{
		if (gnRunMode == RM_SERVER)
		{
			if (ghConEmuWndDC && (!gpSrv->pColorerMapping || (gpSrv->pConsole->hdr.hConEmuWnd != ghConEmuWndDC)))
			{
				if (gpSrv->pColorerMapping && (gpSrv->pConsole->hdr.hConEmuWnd != ghConEmuWndDC))
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
		gpSrv->pConsole->hdr.hConEmuWnd = ghConEmuWndDC;
		_ASSERTE(gpSrv->pConsole->hdr.hConEmuRoot==NULL || gpSrv->pConsole->hdr.nGuiPID!=0);
		gpSrv->pConsole->hdr.nActiveFarPID = gpSrv->nActiveFarPID;

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

	//COORD crMaxSize = GetLargestConsoleWindowSize(GetStdHandle(STD_OUTPUT_HANDLE));
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
		_ASSERTE(pHdr->locked == 0 && pHdr->flushCounter == 0);
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






static BOOL ReadConsoleInfo()
{
	BOOL lbRc = TRUE;
	BOOL lbChanged = gpSrv->pConsole->bDataChanged; // ���� ���-�� ��� �� �������� - ����� TRUE
	//CONSOLE_SELECTION_INFO lsel = {0}; // GetConsoleSelectionInfo
	CONSOLE_CURSOR_INFO lci = {0}; // GetConsoleCursorInfo
	DWORD ldwConsoleCP=0, ldwConsoleOutputCP=0, ldwConsoleMode=0;
	CONSOLE_SCREEN_BUFFER_INFO lsbi = {{0,0}}; // MyGetConsoleScreenBufferInfo
	HANDLE hOut = (HANDLE)ghConOut;

	if (hOut == INVALID_HANDLE_VALUE)
		hOut = GetStdHandle(STD_OUTPUT_HANDLE);

	// ����� ��������� �������� ��� �������� ComSpec � ���������� ������ ������
	MCHKHEAP;

	if (!GetConsoleCursorInfo(hOut, &lci))
	{
		gpSrv->dwCiRc = GetLastError(); if (!gpSrv->dwCiRc) gpSrv->dwCiRc = -1;
	}
	else
	{
		if (gpSrv->bTelnetActive) lci.dwSize = 15;  // telnet "������" ��� ������� Ins - ������ ������ ���� ����� ����� Ctrl ��������

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

		gpSrv->dwSbiRc = GetLastError(); if (!gpSrv->dwSbiRc) gpSrv->dwSbiRc = -1;

		lbRc = FALSE;
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

			if (ghLogSize) LogSize(NULL, ":ReadConsoleInfo");

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
				_ASSERTE(gpSrv->sbi.dwSize.Y <= 200);
				DEBUGLOGSIZE(L"!!! gpSrv->sbi.dwSize.Y > 200 !!! in ConEmuC.ReloadConsoleInfo\n");
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
	gpSrv->pConsole->hdr.nServerPID = GetCurrentProcessId();
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
	return lbChanged;
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
	COORD bufSize, bufCoord;
	SMALL_RECT rgn;
	DWORD nCurSize, nHdrSize;
	// -- �������� ���������� �������������� ���������
	_ASSERTE(gpSrv->sbi.srWindow.Left == 0); // ���� ���� �������
	//_ASSERTE(gpSrv->sbi.srWindow.Right == (gpSrv->sbi.dwSize.X - 1));
	DWORD nCurScroll = (gnBufferHeight ? rbs_Vert : 0) | (gnBufferWidth ? rbs_Horz : 0);
	DWORD nNewScroll = 0;
	int TextWidth = 0, TextHeight = 0;
	BOOL bSuccess = ::GetConWindowSize(gpSrv->sbi, gcrVisibleSize.X, gcrVisibleSize.Y, nCurScroll, &TextWidth, &TextHeight, &nNewScroll);
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

	if (nCurSize <= MAX_CONREAD_SIZE)
	{
		bufSize.X = TextWidth; bufSize.Y = TextHeight;
		bufCoord.X = 0; bufCoord.Y = 0;
		//rgn = gpSrv->sbi.srWindow;

		if (ReadConsoleOutput(hOut, gpSrv->pConsoleDataCopy, bufSize, bufCoord, &rgn))
			lbRc = TRUE;
	}

	if (!lbRc)
	{
		// �������� ������ ���������
		bufSize.X = TextWidth; bufSize.Y = 1;
		bufCoord.X = 0; bufCoord.Y = 0;
		//rgn = gpSrv->sbi.srWindow;
		CHAR_INFO* pLine = gpSrv->pConsoleDataCopy;

		for(int y = 0; y < (int)TextHeight; y++, rgn.Top++, pLine+=TextWidth)
		{
			rgn.Bottom = rgn.Top;
			ReadConsoleOutput(hOut, pLine, bufSize, bufCoord, &rgn);
		}
	}

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

	if (ReadConsoleInfo())
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

	return lbChanged;
}




//#ifdef USE_WINEVENT_SRV
//DWORD WINAPI WinEventThread(LPVOID lpvParam)
//{
//	//DWORD dwErr = 0;
//	//HANDLE hStartedEvent = (HANDLE)lpvParam;
//	// �� ������ ������
//	gpSrv->dwWinEventThread = GetCurrentThreadId();
//	// �� ��������� - ����� ������ StartStop.
//	// ��� ��������� � ������� FAR'� - ������� ��� �������
//	//gpSrv->bWinHookAllow = TRUE; gpSrv->nWinHookMode = 1;
//	//HookWinEvents ( TRUE );
//	_ASSERTE(gpSrv->hWinHookStartEnd==NULL);
//	// "�����" (Start/End)
//	gpSrv->hWinHookStartEnd = SetWinEventHook(
//	                           //EVENT_CONSOLE_LAYOUT, -- � ���������, EVENT_CONSOLE_LAYOUT ����� ������� �� ����� ������
//	                           //                      -- "�����" � ��� scroll & resize & focus, � �� ����� ���� ����� �����
//	                           //                      -- ��� ��� ��� �������������� ����� ������ ��� �� ��������
//	                           EVENT_CONSOLE_START_APPLICATION,
//	                           EVENT_CONSOLE_END_APPLICATION,
//	                           NULL, (WINEVENTPROC)WinEventProc, 0,0, WINEVENT_OUTOFCONTEXT /*| WINEVENT_SKIPOWNPROCESS ?*/);
//
//	if (!gpSrv->hWinHookStartEnd)
//	{
//		PRINT_COMSPEC(L"!!! HookWinEvents(StartEnd) FAILED, ErrCode=0x%08X\n", GetLastError());
//		return 1; // �� ������� ���������� ���, ������ � ���� ���� ���, �������
//	}
//
//	PRINT_COMSPEC(L"WinEventsHook(StartEnd) was enabled\n", 0);
//	//
//	//SetEvent(hStartedEvent); hStartedEvent = NULL; // ����� �� ����� �� ���������
//	MSG lpMsg;
//
//	//while (GetMessage(&lpMsg, NULL, 0, 0)) -- ������� �� Peek ����� ���������� ���� ��������
//	while(TRUE)
//	{
//		if (!PeekMessage(&lpMsg, 0,0,0, PM_REMOVE))
//		{
//			Sleep(10);
//			continue;
//		}
//
//		// 	if (lpMsg.message == gpSrv->nMsgHookEnableDisable) {
//		// 		gpSrv->bWinHookAllow = (lpMsg.wParam != 0);
//		//HookWinEvents ( gpSrv->bWinHookAllow ? gpSrv->nWinHookMode : 0 );
//		// 		continue;
//		// 	}
//		MCHKHEAP;
//
//		if (lpMsg.message == WM_QUIT)
//		{
//			//          lbQuit = TRUE;
//			break;
//		}
//
//		TranslateMessage(&lpMsg);
//		DispatchMessage(&lpMsg);
//		MCHKHEAP;
//	}
//
//	// ������� ���
//	//HookWinEvents ( FALSE );
//	if (/*abEnabled == -1 &&*/ gpSrv->hWinHookStartEnd)
//	{
//		UnhookWinEvent(gpSrv->hWinHookStartEnd); gpSrv->hWinHookStartEnd = NULL;
//		PRINT_COMSPEC(L"WinEventsHook(StartEnd) was disabled\n", 0);
//	}
//
//	MCHKHEAP;
//	return 0;
//}
//#endif

//#ifdef USE_WINEVENT_SRV
////Minimum supported client Windows 2000 Professional
////Minimum supported server Windows 2000 Server
//void WINAPI WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD anEvent, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
//{
//	if (hwnd != ghConWnd)
//	{
//		// ���� ��� �� ���� ���� - �������
//		return;
//	}
//
//	//BOOL bNeedConAttrBuf = FALSE;
//	//CESERVER_CHAR ch = {{0,0}};
//#ifdef _DEBUG
//	WCHAR szDbg[128];
//#endif
//	nExitPlaceThread = 1000;
//
//	switch(anEvent)
//	{
//		case EVENT_CONSOLE_START_APPLICATION:
//			//A new console process has started.
//			//The idObject parameter contains the process identifier of the newly created process.
//			//If the application is a 16-bit application, the idChild parameter is CONSOLE_APPLICATION_16BIT and idObject is the process identifier of the NTVDM session associated with the console.
//#ifdef _DEBUG
//#ifndef WIN64
//			_ASSERTE(CONSOLE_APPLICATION_16BIT==1);
//#endif
//			_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"EVENT_CONSOLE_START_APPLICATION(PID=%i%s)\n", idObject,
//			          (idChild == CONSOLE_APPLICATION_16BIT) ? L" 16bit" : L"");
//			DEBUGSTR(szDbg);
//#endif
//
//			if ((((DWORD)idObject) != gnSelfPID) && !nExitQueryPlace)
//			{
//				CheckProcessCount(TRUE);
//				/*
//				EnterCriticalSection(&gpSrv->csProc);
//				gpSrv->nProcesses.push_back(idObject);
//				LeaveCriticalSection(&gpSrv->csProc);
//				*/
//#ifndef WIN64
//				_ASSERTE(CONSOLE_APPLICATION_16BIT==1);
//
//				// �� �� ���� ������� ��������: (idChild == CONSOLE_APPLICATION_16BIT)
//				// ������ ��� �� �������� �����, ����� 16��� (��� DOS) ���������� �����
//				// ����������� ����� ������ �� ����� ������ ��������.
//				if (idChild == CONSOLE_APPLICATION_16BIT)
//				{
//					if (ghLogSize)
//					{
//						char szInfo[64]; _wsprintfA(szInfo, SKIPLEN(countof(szInfo)) "NTVDM started, PID=%i", idObject);
//						LogSize(NULL, szInfo);
//					}
//
//					gpSrv->bNtvdmActive = TRUE;
//					gpSrv->nNtvdmPID = idObject;
//					SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
//					// ��� ������ ������ ��� ������... ����� ������� �� ������� �� 16��� ����������...
//					// ��������� �������� ������ - 25/28/50 �����. ��� ������ - ������ 28
//					// ������ - ������ 80 ��������
//				}
//
//#endif
//				//
//				//HANDLE hIn = CreateFile(L"CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_READ,
//				//                  0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
//				//if (hIn != INVALID_HANDLE_VALUE) {
//				//  HANDLE hOld = ghConIn;
//				//  ghConIn = hIn;
//				//  SafeCloseHandle(hOld);
//				//}
//			}
//
//			nExitPlaceThread = 1000;
//			return; // ���������� ������ �� ���������
//		case EVENT_CONSOLE_END_APPLICATION:
//			//A console process has exited.
//			//The idObject parameter contains the process identifier of the terminated process.
//#ifdef _DEBUG
//			_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"EVENT_CONSOLE_END_APPLICATION(PID=%i%s)\n", idObject,
//			          (idChild == CONSOLE_APPLICATION_16BIT) ? L" 16bit" : L"");
//			DEBUGSTR(szDbg);
//#endif
//
//			if (((DWORD)idObject) != gnSelfPID)
//			{
//				CheckProcessCount(TRUE);
//#ifndef WIN64
//				_ASSERTE(CONSOLE_APPLICATION_16BIT==1);
//
//				if (idChild == CONSOLE_APPLICATION_16BIT)
//				{
//					if (ghLogSize)
//					{
//						char szInfo[64]; _wsprintfA(szInfo, SKIPLEN(countof(szInfo)) "NTVDM stopped, PID=%i", idObject);
//						LogSize(NULL, szInfo);
//					}
//
//					//DWORD ntvdmPID = idObject;
//					//dwActiveFlags &= ~CES_NTVDM;
//					gpSrv->bNtvdmActive = FALSE;
//					//TODO: �������� ����� ������� ������� NTVDM?
//					SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
//				}
//
//#endif
//				//
//				//HANDLE hIn = CreateFile(L"CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_READ,
//				//                  0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
//				//if (hIn != INVALID_HANDLE_VALUE) {
//				//  HANDLE hOld = ghConIn;
//				//  ghConIn = hIn;
//				//  SafeCloseHandle(hOld);
//				//}
//			}
//
//			nExitPlaceThread = 1000;
//			return; // ���������� ������ �� ���������
//		case EVENT_CONSOLE_LAYOUT: //0x4005
//		{
//			//The console layout has changed.
//			//EVENT_CONSOLE_LAYOUT, -- � ���������, EVENT_CONSOLE_LAYOUT ����� ������� �� ����� ������
//			//                      -- "�����" � ��� scroll & resize & focus, � �� ����� ���� ����� �����
//			//                      -- ��� ��� ��� �������������� ����� ������ ��� �� ��������
//#ifdef _DEBUG
//			DEBUGSTR(L"EVENT_CONSOLE_LAYOUT\n");
//#endif
//		}
//		nExitPlaceThread = 1000;
//		return; // ���������� �� ������� � ����
//	}
//
//	nExitPlaceThread = 1000;
//}
//#endif













DWORD WINAPI RefreshThread(LPVOID lpvParam)
{
	DWORD nWait = 0; //, dwConWait = 0;
	HANDLE hEvents[2] = {ghQuitEvent, gpSrv->hRefreshEvent};
	DWORD nDelta = 0;
	DWORD nLastReadTick = 0; //GetTickCount();
	DWORD nLastConHandleTick = GetTickCount();
	BOOL  /*lbEventualChange = FALSE,*/ /*lbForceSend = FALSE,*/ lbChanged = FALSE; //, lbProcessChanged = FALSE;
	DWORD dwTimeout = 10; // ������������� ������ ���������� �� ���� (��������, �������,...)
	//BOOL  bForceRefreshSetSize = FALSE; // ����� ��������� ������� ����� ����� ���������� ������� ��� ��������
	BOOL lbWasSizeChange = FALSE;

	while(TRUE)
	{
		nWait = WAIT_TIMEOUT;
		//lbForceSend = FALSE;
		MCHKHEAP;

		// Alwas update con handle, ������ �������
		if ((GetTickCount() - nLastConHandleTick) > UPDATECONHANDLE_TIMEOUT)
		{
			WARNING("!!! � Win7 �������� ����������� � ������ �������� - ��������� ���������� ����� ���������!!!");

			// � �����, ����� ������ telnet'� ������������!
			if (gpSrv->bReopenHandleAllowed)
			{
				ghConOut.Close();
				nLastConHandleTick = GetTickCount();
			}
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

		// �� ������ ���� �������� ������ �� ��������� ������� �������
		if (gpSrv->nRequestChangeSize)
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

		// ����� �� ������� ��������� ����������� ��������� ����, ����
		// ������ ��� �� ���� ����������� ��������� ������� �������
		if (!lbWasSizeChange
		        // �� ��������� �������������� �������������
		        && !gpSrv->bForceConsoleRead
		        // ������� �� �������
		        && (!gpSrv->pConsole->hdr.bConsoleActive
		            // ��� �������, �� ��� ConEmu GUI �� � ������
		            || (gpSrv->pConsole->hdr.bConsoleActive && !gpSrv->pConsole->hdr.bThawRefreshThread))
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
			ghConEmuWndDC = NULL;
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
		if (!gpSrv->bDebuggerActive)
		{
			if (pfnGetConsoleKeyboardLayoutName)
				CheckKeyboardLayout();
		}

		/* ****************** */
		/* ���������� ������� */
		/* ****************** */
		if (!gpSrv->bDebuggerActive)
		{
			lbChanged = ReloadFullConsoleInfo(gpSrv->bWasReattached/*lbForceSend*/);
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
