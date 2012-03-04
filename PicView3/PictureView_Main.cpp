
/**************************************************************************
               ������������� ����� � �������� ����� ����
**************************************************************************/

/**************************************************************************
Copyright (c) 2010 Maximus5
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
**************************************************************************/

#include "PictureView.h"
#include "PVDManager.h"
#include "Image.h"
#include "PictureView_Lang.h"
#include "PictureView_Display.h"
//
#include "ConEmuSupport.h"
#ifdef _DEBUG
#include <shlwapi.h>
#endif
#include <Tlhelp32.h>

BOOL ExecuteInMainThread(DWORD nCmd, int nPeekMsg)
{	
	BOOL lbResult = FALSE;
	if (!g_Plugin.hSynchroDone)
		return FALSE;

	CFunctionLogger::FunctionLogger(L"ExecuteInMainThread(%i)", nCmd);

	TODO("����������� ������ ���������� ������");
	EnterCriticalSection(&g_Plugin.csMainThread);

	_ASSERTE((g_Plugin.FlagsWork & nCmd) == 0);
	g_Plugin.FlagsWorkResult &= ~(nCmd);
	ResetEvent(g_Plugin.hSynchroDone);
	g_Plugin.FlagsWork |= nCmd;

	bool  lbBreak = false;
	DWORD nWait = -1;
	while (!lbBreak && (nWait = WaitForSingleObject(g_Plugin.hSynchroDone, 10)) != WAIT_OBJECT_0)
	{
		if ((g_Plugin.FlagsWork & nCmd) == 0) {
			// ������� ����� ���� ���������� ��� ����� ������������� WaitForSingleObject ��� TIMEOUT
			lbResult = (g_Plugin.FlagsWorkResult & nCmd) != 0;
			break;
		}
		//EscapePressed �� �����. ������� Esc � ����������� ��������� � �������� ����� ��������
		//if (EscapePressed()) g_Plugin.FlagsWork |= FW_TERMINATE;

		if (g_Plugin.FlagsWork & FW_TERMINATE)
		{
			g_Plugin.FlagsWork &= ~nCmd;
			break;
		}

		if (nPeekMsg)
		{
			MSG  Msg;
			// Note! PeekMessage �� ������� �� ����� ��������� ���������, �������� WM_PAINT
			while (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
			{
				// �� ����, ����� ��������� �� ������? ����� ��� ���� �������� ����?
				if (Msg.message == WM_QUIT || Msg.message == WM_DESTROY)
				{
					// -- // ������� ������� � �������
					//PostMessage(Msg.hwnd, Msg.message, Msg.wParam, Msg.lParam);

					// � �����
					lbBreak = true; break;
				}

				__try
				{
					if (nPeekMsg == 1)
						DefWindowProc(Msg.hwnd, Msg.message, Msg.wParam, Msg.lParam);
					else
						DispatchMessage(&Msg);
				}
				__except(EXCEPTION_EXECUTE_HANDLER)
				{
					CFunctionLogger::FunctionLogger(L"!!! Exception in ExecuteInMainThread.DefWindowProc(%i)",Msg.message);
				}

				// ���� ������� ����������� - ���������� ��������� (������ �������) ���������
				if (WaitForSingleObject(g_Plugin.hSynchroDone, 0) == WAIT_OBJECT_0)
				{
					lbBreak = true;
					lbResult = (g_Plugin.FlagsWorkResult & nCmd) != 0;
					break;
				}
			}
		}
	}

	if (nWait == WAIT_OBJECT_0)
	{
		lbResult = (g_Plugin.FlagsWorkResult & nCmd) != 0;
	}

	//_ASSERTE((g_Plugin.FlagsWork & nCmd) == 0);
	g_Plugin.FlagsWork &= ~nCmd;

	CFunctionLogger::FunctionLogger(L"~ExecuteInMainThread(%i)", nCmd);

	LeaveCriticalSection(&g_Plugin.csMainThread);

	return lbResult;
}

EFlagsMainProcess ProcessMainThread()
{
	EFlagsMainProcess nResult = FEM_EXIT_PLUGIN;

	DWORD dwWait = 0;
	//HANDLE hEvents[2] = {g_Plugin.hDisplayEvent, g_Plugin.hDisplayThread};
	//Wait ForSingleObject(g_Plugin.hDisplayEvent, INFINITE); -- ����. ���� ���� ���������� - �� ����� ��������

	_ASSERTE(GetCurrentThreadId() == gnMainThreadId);

	// ����������� �������� �������� �� ����, � ������� ���������� �������. � �� Display - ������ ReadConsoleInput
	// ��� ����� ��� ����, ����� FAR �� ��� ��� ������������ � ������ ���������� �� ����� ��������� �������� � �����
	// �� ����� ������, ��� ���� �������� ����/������ �� ������� ������ ��� ���?

	while (!(g_Plugin.FlagsWork & FW_TERMINATE)
		&& (dwWait = WaitForSingleObject(g_Plugin.hDisplayThread, 10)) == WAIT_TIMEOUT)
	{
		MCHKHEAP;

		if (g_Plugin.FlagsWork & FW_SHOW_HELP)
		{
			// �������� Help � �������� ����
			CFunctionLogger flog(L"FW_SHOW_HELP");
			wchar_t Title[0x40];
			lstrcpynW(Title, g_Plugin.pszPluginTitle, sizeofarray(Title) - 6);
			lstrcat(Title, L" - Far");
			// ������ ����������� � ���� Display, � ����� �����
			SetConsoleTitleW(Title);

			g_StartupInfo.ShowHelp(g_StartupInfo.ModuleName, _T("Controls"), 0);

			SetEvent(g_Plugin.hSynchroDone);
			g_Plugin.FlagsWorkResult |= FW_SHOW_HELP;
			g_Plugin.FlagsWork &= ~FW_SHOW_HELP;			

		}
		else if (g_Plugin.FlagsWork & (FW_SHOW_CONFIG | FW_SHOW_MODULES))
		{
			// �������� ���� ������� � �������� ����
			CFunctionLogger flog(L"FW_SHOW_CONFIG|FW_SHOW_MODULES");

			//TRUE, ���� ��������� ���� ��������
			const bool reconfig = ConfigureW(0);

			if (reconfig)
				g_Plugin.FlagsDisplay |= FD_REQ_REFRESH;
			SetEvent(g_Plugin.hSynchroDone);
			g_Plugin.FlagsWorkResult |= (g_Plugin.FlagsWork & (FW_SHOW_CONFIG|FW_SHOW_MODULES));
			g_Plugin.FlagsWork &= ~(FW_SHOW_CONFIG|FW_SHOW_MODULES);

		}
		else if (g_Plugin.FlagsWork & (FW_MARK_FILE | FW_UNMARK_FILE | FW_TOGGLEMARK_FILE))
		{
			CFunctionLogger flog(L"FW_MARK_FILE | FW_UNMARK_FILE | FW_TOGGLEMARK_FILE");

			//g_Plugin.Image[0]->bSelected = (g_Plugin.FlagsWork & FW_MARK_FILE) == FW_MARK_FILE;
			g_Plugin.SelectionChanged = true; // ����� ��� ������ �� ������� ����� ���� �������� ������, ������� ���������� ������
			//g_StartupInfo.PanelControl(INVALID_HANDLE_VALUE, FCTL_SETSELECTION, g_Plugin.Image[0]->PanelItemRaw(), g_Plugin.Image[0]->bSelected);
			//g_StartupInfo.PanelControl(INVALID_HANDLE_VALUE, FCTL_REDRAWPANEL, 0, 0);
			g_Panel.MarkUnmarkFile(
				(g_Plugin.FlagsWork & FW_TOGGLEMARK_FILE) ? CPicViewPanel::ema_Switch :
				(g_Plugin.FlagsWork & FW_MARK_FILE) ? CPicViewPanel::ema_Mark
				: CPicViewPanel::ema_Unmark);
			TitleRepaint();
			SetEvent(g_Plugin.hSynchroDone);
			g_Plugin.FlagsWorkResult |= (g_Plugin.FlagsWork & (FW_MARK_FILE | FW_UNMARK_FILE | FW_TOGGLEMARK_FILE));
			g_Plugin.FlagsWork &= ~(FW_MARK_FILE | FW_UNMARK_FILE | FW_TOGGLEMARK_FILE);

		}
		else if (g_Plugin.FlagsWork & (FW_TITLEREPAINT | FW_TITLEREPAINTD))
		{
			CFunctionLogger flog(L"FW_TITLEREPAINT | FW_TITLEREPAINTD");

			TitleRepaint((g_Plugin.FlagsWork & FW_TITLEREPAINTD)==FW_TITLEREPAINTD);
			SetEvent(g_Plugin.hSynchroDone);
			g_Plugin.FlagsWorkResult |= (g_Plugin.FlagsWork & (FW_TITLEREPAINT | FW_TITLEREPAINTD));
			g_Plugin.FlagsWork &= ~(FW_TITLEREPAINT | FW_TITLEREPAINTD);

		}
		else if (g_Plugin.FlagsWork & (FW_QVIEWREPAINT))
		{
			CFunctionLogger flog(L"FW_QVIEWREPAINT");

			QViewRepaint();
			SetEvent(g_Plugin.hSynchroDone);
			g_Plugin.FlagsWorkResult |= FW_QVIEWREPAINT;
			g_Plugin.FlagsWork &= ~FW_QVIEWREPAINT;

		}
		else if (g_Plugin.FlagsWork & (FW_RETRIEVE_FILE))
		{
			CFunctionLogger flog(L"FW_RETRIEVE_FILE");

			// ������ ���������� � FAR ����� ������� ���� �� ������
			bool lbMacroOk = g_Panel.StartItemExtraction();

			if (lbMacroOk)
				g_Plugin.FlagsWorkResult |= FW_RETRIEVE_FILE;
			SetEvent(g_Plugin.hSynchroDone);

			g_Plugin.FlagsWork &= ~FW_RETRIEVE_FILE;

			if (lbMacroOk)
			{
				nResult = FEM_RETRIEVE_FILE;
				g_Plugin.FlagsWork |= FW_IN_RETRIEVE;
				break;
			}
			else
			{
				_ASSERTE(lbMacroOk == true);
			}
		}

		if (g_Plugin.hWnd)
			ProcessConsoleInputs();
	}

	if (dwWait == WAIT_OBJECT_0 && !(g_Plugin.FlagsWork & FW_TERMINATE))
	{
		_ASSERTE(nResult != FEM_RETRIEVE_FILE);
		RequestTerminate(2);
	}

	CFunctionLogger::FunctionLogger(L"ProcessMainThread done");

	return nResult;
}



// EntryPoint
//		��� ����� ����� ����� ��� �������. ������ �������� �����!
// OpenFrom
//		0 - ������� ����� �� ������� (�������� "����� � ����" �� Enter/CtrlPgDn ��� ������������� ����������)
//		OPEN_BY_USER - ����� ����� �� �������. ��� ����� ���� ������� ��� F11. ��������, �� ������!
//		OPEN_VIEWER - ������ VE_READ, ���� ����� �������� ������ ��� ������ �������� �����
//		(OPEN_VIEWER|OPEN_BY_USER) - F11 �� �������
//		OPEN_EDITOR - ������ EE_READ, ���� ����� �������� ������ ��� ������ �������� �����
//		(OPEN_EDITOR|OPEN_BY_USER) - F11 �� ���������
// asFile
//		����������� ����������� ������ ���� (OpenFrom == 0)
//		����� - ����� ���� ����. ������� ���� ������ �������� ��� �����.
EFlagsResult EntryPoint(int OpenFrom, LPCWSTR asFile)
{
	WARNING("��������� ������ � TRY {} CATCH {}");

	EFlagsResult result = FE_PROCEEDED;
	EFlagsMainProcess r = FEM_EXIT_PLUGIN;
	DecodeParams parms;
	//BOOL lbNeedCache;
	CImage *pImage = NULL;

	_ASSERTE(g_Plugin.hWnd == NULL);

	g_Plugin.InitPlugin(); // InitHooks() ��� ������, �� InitPlugin ��� �� ������ ������ �������

	g_Plugin.SaveTitle();

	// ��� ������ �������� ����� - ���������� ���� FW_FIRST_CALL, �� �� FW_FORCE_DECODE,
	// �.�. ���� �� ����� ������� � ��� ��������� �������/���������, � ��� FW_FORCE_DECODE �� �����
	//g_Plugin.FlagsWork |= FW_FIRST_CALL; --> eRenderFirstImage
	//if ((OpenFrom & OPEN_BY_USER))
	//	g_Plugin.FlagsWork |= FW_FORCE_DECODE;
	// ����� ������������ ������, ����� �������� �� ����������� �������
	g_Plugin.FlagsWork &= ~FW_TERMINATE;

	parms.Flags = ((OpenFrom & OPEN_BY_USER) ? eRenderForceDecoder : 0) | eRenderFirstImage;

	// ����� ���� �������� ����� �������?
	g_Panel.CheckAllClosed();
	g_Panel.FreeAllItems();

	// ���� �������� �� ����� - �� ������� ��������, �������������� �� ��� ����������
	if (!(OpenFrom & OPEN_BY_USER))
	{
		//_ASSERTE((g_Plugin.FlagsWork & FW_FORCE_DECODE) == FW_FORCE_DECODE);
		if (!CPVDManager::IsSupportedExtension(asFile))
		{
			result = FE_UNKNOWN_EXTENSION;
			goto wrap;
		}
	}

	// ������������� ������ ������
	WARNING("�� ������� ������� ������ ��� �������� ��������, ����� ����� ��������������?");
	if (!g_Panel.Initialize(OpenFrom, asFile))
	{
		result = FE_OTHER_ERROR;
		g_Plugin.FlagsWork |= FW_TERMINATE;
		goto wrap;
	}


	// ��������, ����� �� �������� � ConEmu?
	_ASSERTE(g_Plugin.hConEmuCtrlPressed==NULL && g_Plugin.hConEmuShiftPressed==NULL);
	CheckConEmu();

	// �������� ���������� ������������� ����
	if (!g_Plugin.hConEmuWnd && (!IsWindow(g_Plugin.hFarWnd) || !IsWindowVisible(g_Plugin.hFarWnd)))
	{
		g_StartupInfo.Message(PluginNumberMsg, FMSG_ALLINONE|FMSG_MB_OK,
			NULL,
			(const wchar_t * const *)GetMsg(MIConEmuPluginWarning),
			0, 0);
		result = FE_SKIP_ERROR;
		g_Plugin.FlagsWork |= FW_TERMINATE;
		goto wrap;
	}

	MCHKHEAP;

	// ������� ��� �������������� CMYK -> RGB
	g_Plugin.InitCMYK(FALSE); // ��������� ��� ����������

	if (g_Plugin.hDisplayThread)
	{
		// ��������, ��� �� ����� �������
		if (WaitForSingleObject(g_Plugin.hDisplayThread, 0) == WAIT_OBJECT_0)
		{
			CloseHandle(g_Plugin.hDisplayThread); g_Plugin.hDisplayThread = NULL;
		}
		else
		{
			_ASSERTE(g_Plugin.hDisplayThread == NULL);
		}
	}
	if (!g_Plugin.hDisplayThread)
	{
		// ����� �������
		if (!g_Display.CreateDisplayThread())
		{
			result = FE_OTHER_ERROR;
			goto wrap;
		}
	}

	MCHKHEAP;

	parms.Priority = eCurrentImageCurrentPage;
	parms.nRawIndex = g_Panel.GetActiveRawIdx();
	// RequestDecodedImage - �������� ������ � ������� �������������
	// GetDecodedImage - ���������� ���������� �������������
	pImage = g_Manager.GetDecodedImage(&parms);
	if (!pImage)
	{
		result = FE_OPEN_FAILED;
		RequestTerminate(); // �� ������ ������
		goto wrap;
	}

	MCHKHEAP;

	if ((g_Plugin.FlagsWork & FW_TERMINATE) == FW_TERMINATE)
	{
		result = FE_OTHER_ERROR;
		goto wrap;
	}

	MCHKHEAP;

	if (!g_Plugin.hWnd || !IsWindow(g_Plugin.hWnd))
	{
		_ASSERTE(g_Plugin.hWnd && IsWindow(g_Plugin.hWnd));
		result = FE_OTHER_ERROR;
		goto wrap;
	}
	else
	{
		// ���� �������� ������� ������������ - ���� ����������� ����� ��������
		if (!IsWindowVisible(g_Plugin.hWnd))
		{
			// Async, �.�. ��� ��������� � ������ ������
			ShowWindowAsync(g_Plugin.hWnd, SW_SHOWNORMAL);
		}
	}
	
	// ����� ��������� � ������� �� ������������� ��������� ����� (Priority = eCurrentImageNextPage)!
	g_Manager.RequestPreCache(&parms);
#if 0
	if ((pImage->Info.nPages > 1) && ((pImage->Info.nPage + 1) < pImage->Info.nPages))
	{
		WARNING("��������� � ������� �� ������������� ��������� �����!");
		DecodeParams next;
		next.Priority = eCurrentImageNextPage;
		next.nPage = pImage->Info.nPage+1; // ��������� �����
		next.nRawIndex = parms.nRawIndex;
		
		// RequestDecodedImage - �������� ������ � ������� �������������,
		// GetDecodedImage - ���������� ���������� �������������
		g_Manager.RequestDecodedImage(&next);
	}
	
	// 
	lbNeedCache = (g_Panel.IsRealNames()) ? g_Plugin.bCachingRP : g_Plugin.bCachingVP;
	if (lbNeedCache)
	{
		DecodeParams next;
		next.Priority = eDecodeNextImage; // ��������� ������, �� ������� �����������
		next.nPage = 0; // ������ �����
		
		TODO("�������� �� eRenderRelativeIndex & iDirection & nRawIndex");
		
		// RequestDecodedImage - �������� ������ � ������� �������������,
		// GetDecodedImage - ���������� ���������� �������������

		// ��������� ����
		int nNext = next.nRawIndex = g_Panel.GetNextItemRawIdx(1);
		if ((next.nRawIndex != -1) && (next.nRawIndex != parms.nRawIndex))
			g_Manager.RequestDecodedImage(&next);

		// ���������� ����
		next.nRawIndex = g_Panel.GetNextItemRawIdx(-1);
		if ((next.nRawIndex != -1) && (next.nRawIndex != parms.nRawIndex) && (next.nRawIndex != nNext))
			g_Manager.RequestDecodedImage(&next);
	}
#endif

	MCHKHEAP;

	r = ProcessMainThread();

	if (r == FEM_RETRIEVE_FILE)
	{
		// ������� �� ���������� ������?
		_ASSERTE((r != FEM_RETRIEVE_FILE) || (g_Plugin.FlagsWork & FW_IN_RETRIEVE));
		result = FE_RETRIEVE_FILE;
	}
	else
	{
		result = FE_PROCEEDED;
	}

wrap:
	if (result != FE_RETRIEVE_FILE)
	{

		if (!(g_Plugin.FlagsWork & FW_TERMINATE))
		{
			_ASSERTE((g_Plugin.FlagsWork & FW_TERMINATE) || !(g_Plugin.FlagsWork & (FW_TERMINATE|FW_IN_RETRIEVE)));
			g_Plugin.FlagsWork |= FW_TERMINATE;
		}
	}

	if ((g_Plugin.FlagsWork & FW_TERMINATE))
	{
		OnFinalTerminate(result);
	}

	return result;
}

void OnFinalTerminate(EFlagsResult result)
{
	RequestTerminate(2);
	_ASSERTE((g_Plugin.FlagsWork & FW_TERMINATE) == FW_TERMINATE);

	g_Plugin.RestoreTitle();

	g_Display.OnTerminate();
	g_Manager.OnTerminate();
	g_Panel.OnTerminate();


	switch (result)
	{
		case FE_PROCEEDED:
			// ���� (g_Plugin.FlagsWork & FW_PLUGIN_CALL) �� � FAR ��� ���� ������ Ctrl-PgDn
			CFunctionLogger::FunctionLogger(L"OnFinalTerminate.FE_PROCEEDED");
			//hPlugin = (HANDLE)-2; // ����� FAR �� ������� ������� ���� ������ ��������
			break;
		case FE_SKIP_ERROR:
			CFunctionLogger::FunctionLogger(L"OnFinalTerminate.FE_SKIP_ERROR");
			break; // �� �������, �� ������ ���������� �� �����
		case FE_UNKNOWN_EXTENSION:
			// ����������� ����������
			//if (g_Plugin.FlagsWork & FW_FIRST_CALL) {
			//	g_StartupInfo.Message(PluginNumberMsg, FMSG_ALLINONE|FMSG_MB_OK,
			//		NULL, (const wchar_t * const *)GetMsg(MIOpenUnknownExtension), 0, 0);
			//}
			CFunctionLogger::FunctionLogger(L"OnFinalTerminate.FE_UNKNOWN_EXTENSION");
			break;
		case FE_OPEN_FAILED:
			// ������ �������� �����
			CFunctionLogger::FunctionLogger(L"OnFinalTerminate.FE_OPEN_FAILED");
			//if (g_Plugin.FlagsWork & FW_FIRST_CALL)
			{
				g_StartupInfo.Message(PluginNumberMsg, FMSG_ALLINONE|FMSG_MB_OK,
					NULL, (const wchar_t * const *)GetMsg(MIOpenFailedMsg), 0, 0);
			}
			break;
		case FE_NO_PANEL_ITEM:
			// ������ ��� ��������� �� ������?
			CFunctionLogger::FunctionLogger(L"OnFinalTerminate.FE_NO_PANEL_ITEM");
			break;
		case FE_VIEWER_ERROR:
			// ������ � ProcessViewerEventW
			CFunctionLogger::FunctionLogger(L"OnFinalTerminate.FE_VIEWER_ERROR");
			break;
	}
}
