
/*
Copyright (c) 2009-2011 Maximus5
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

#ifdef _DEBUG
// ������� ���������. ��������� ��������, ������� ����� ����� ����������, ...
//#define DEBUG_PAINT
#endif
#ifdef DEBUG_PAINT
#define DEBUG_SLEEP(n) Sleep(n)
#else
#define DEBUG_SLEEP(n)
#endif

#define SHOWDEBUGSTR
//#define MCHKHEAP
#define DEBUGSTRMENU(s) DEBUGSTR(s)
#define DEBUGSTRCTRL(s) DEBUGSTR(s)

#undef USE_DEBUG_LOGS
//#define USE_DEBUG_LOGS

#include <windows.h>
#include "ConEmuTh.h"
#include "../common/RgnDetect.h"
#include "resource.h"
#include "ImgCache.h"

#ifdef CONEMU_LOG_FILES
	#define DEBUGSTRPAINT(s) if (gpLogPaint) {gpLogPaint->LogString(s);} // DEBUGSTR(s)
#else
	#define DEBUGSTRPAINT(s)
#endif

#ifdef _DEBUG
#undef _ASSERTE
static bool gbIgnoreAsserts = false;
#define _ASSERTE(expr) if (!(expr) && !gbIgnoreAsserts) { gbIgnoreAsserts = true; _ASSERT_EXPR((expr), _CRT_WIDE(#expr)); gbIgnoreAsserts = false; }
#endif


static ATOM hClass = NULL;
//LRESULT CALLBACK DisplayWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
//DWORD WINAPI DisplayThread(LPVOID lpvParam);
//void Paint(HWND hwnd, PAINTSTRUCT& ps, RECT& rc, CeFullPanelInfo* pi);
const wchar_t gsDisplayClassName[] = L"ConEmuPanelView";
HANDLE ghCreateEvent = NULL;
//extern HICON ghUpIcon;
int gnCreateViewError = 0;
DWORD gnWin32Error = 0;
//ITEMIDLIST DesktopID = {{0}};
//IShellFolder *gpDesktopFolder = NULL;
BOOL gbCancelAll = FALSE;
//extern COLORREF /*gcrActiveColors[16], gcrFadeColors[16],*/ *gcrCurColors;
//extern bool gbFadeColors;
//extern bool gbFarPanelsReady;
UINT gnConEmuFadeMsg = 0, gnConEmuSettingsMsg = 0;
//extern CRgnDetect *gpRgnDetect;
//extern CEFAR_INFO_MAPPING gFarInfo;
//extern DWORD gnRgnDetectFlags;

#ifdef CONEMU_LOG_FILES
MFileLog* gpLogLoad = NULL;
MFileLog* gpLogPaint = NULL;
#endif

#define WINDOW_LONG_FADE (16*sizeof(DWORD))

void ResetUngetBuffer();
int ShowLastError();

#define MSG_CREATE_VIEW (WM_USER+101)
#define CREATE_WND_TIMEOUT 5000


HWND CeFullPanelInfo::CreateView()
{
	gnCreateViewError = 0;
	gnWin32Error = 0;

#ifdef CONEMU_LOG_FILES

	if (!gpLogLoad)
	{
		gpLogLoad = new MFileLog(L"ConEmuTh_Load");

		if (gpLogLoad->CreateLogFile())
		{
			delete gpLogLoad;
			gpLogLoad = NULL;
		}
	}

	if (!gpLogPaint)
	{
		gpLogPaint = new MFileLog(L"ConEmuTh_Draw");

		if (gpLogPaint->CreateLogFile())
		{
			delete gpLogPaint;
			gpLogPaint = NULL;
		}
	}

#endif // #ifdef CONEMU_LOG_FILES

	HWND lhView = this->hView; // (this->bLeftPanel) ? ghLeftView : ghRightView;

	if (lhView)
	{
		_ASSERTE(lhView==NULL);

		if (IsWindow(lhView))
		{
			_ASSERTE(lhView == this->hView);
			return lhView;
		}

		lhView = NULL;
	}

	this->hView = NULL;
	this->cbSize = sizeof(*this);

	if (!gnDisplayThreadId || !ghDisplayThread)
	{
		if (ghDisplayThread)
		{
			CloseHandle(ghDisplayThread); ghDisplayThread = NULL;
		}

		// ������� ����� ��������� ���� (�� ��� �� ���������)
		HANDLE hReady = CreateEvent(NULL,FALSE,FALSE,NULL);
		ghDisplayThread = CreateThread(NULL,0,DisplayThread,(LPVOID)hReady,0,&gnDisplayThreadId); //-V513

		if (!ghDisplayThread)
		{
			gnWin32Error = GetLastError();
			gnCreateViewError = CECreateThreadFailed;
			return NULL;
		}

		// ���������, ���� ���� ������
		DWORD nTimeout = CREATE_WND_TIMEOUT;
#ifdef _DEBUG

		if (IsDebuggerPresent())
			nTimeout = 60000;

#endif
		HANDLE hEvents[2] = {ghDisplayThread,hReady};
		DWORD dwWait = WaitForMultipleObjects(2, hEvents, FALSE, nTimeout);
		CloseHandle(hReady); hReady = NULL;

		if (dwWait != (WAIT_OBJECT_0+1))
		{
			gnWin32Error = GetLastError();
			gnCreateViewError = CEThreadActivationFailed;
			return NULL;
		}

		// � ��� ���� ���������, � �� ��������� "Invalid thread identifier"
		Sleep(50);
	}

	if (GetCurrentThreadId() != gnDisplayThreadId)
	{
		ghCreateEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
		BOOL lbRc = PostThreadMessage(gnDisplayThreadId, MSG_CREATE_VIEW, (WPARAM)ghCreateEvent, (LPARAM)this);

		if (!lbRc)
		{
			gnWin32Error = GetLastError();
			gnCreateViewError = CEPostThreadMessageFailed;
			CloseHandle(ghCreateEvent); ghCreateEvent = NULL;
		}
		else
		{
			DWORD nTimeout = CREATE_WND_TIMEOUT;
#ifdef _DEBUG

			if (IsDebuggerPresent())
				nTimeout = 60000;

#endif
			HANDLE hEvents[2] = {ghDisplayThread,ghCreateEvent};
			DWORD dwWait = WaitForMultipleObjects(2, hEvents, FALSE, nTimeout);
			CloseHandle(ghCreateEvent); ghCreateEvent = NULL;

			if (dwWait == (WAIT_OBJECT_0))
			{
				// ���� �����������
				gnCreateViewError = CEDisplayThreadTerminated;
				GetExitCodeThread(ghDisplayThread, &gnWin32Error);
			}
			else if (dwWait == (WAIT_OBJECT_0+1))
			{
				lhView = this->hView;
			}
		}

		return lhView;
	}

	if (!hClass)
	{
		WNDCLASS wc = {0};
		wc.style = CS_OWNDC|CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW;
		wc.lpfnWndProc = DisplayWndProc;
		// 16 + <Disabled> + 3 ���������������
		// <Disabled> - ��� ����� ������ ������ ���������� ������ (�� ���� �� ������������� ����������)
		wc.cbWndExtra = 20*sizeof(DWORD);
		wc.hInstance = (HINSTANCE)ghPluginModule;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
		wc.lpszClassName = gsDisplayClassName;
		hClass = RegisterClass(&wc);

		if (!hClass)
		{
			gnWin32Error = GetLastError();
			gnCreateViewError = CERegisterClassFailed;
			return NULL;
		}
	}

	this->cbSize = sizeof(*this);
	wchar_t szTitle[128];
	_wsprintf(szTitle, SKIPLEN(countof(szTitle)) L"ConEmu.%sPanelView.%i", (this->bLeftPanel) ? L"Left" : L"Right", gnSelfPID);
	lhView = CreateWindow(gsDisplayClassName, szTitle, WS_CHILD|WS_CLIPSIBLINGS, 0,0,0,0,
	                      ghConEmuRoot, NULL, (HINSTANCE)ghPluginModule, (LPVOID)this);
#ifdef _DEBUG
	HWND hParent = GetParent(lhView);
#endif
	this->hView = lhView;

	if (!lhView)
	{
		gnWin32Error = GetLastError();
		gnCreateViewError = CECreateWindowFailed;
	}

	//else
	//{
	//	if (this->bLeftPanel)
	//		ghLeftView = lhView;
	//	else
	//		ghRightView = lhView;
	//}
	return lhView;
}

LRESULT CALLBACK CeFullPanelInfo::DisplayWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_CREATE:
		{
			CREATESTRUCT* pcr = (CREATESTRUCT*)lParam;
			CeFullPanelInfo* pi = (CeFullPanelInfo*)pcr->lpCreateParams;
			_ASSERTE(pi && pi->cbSize==sizeof(CeFullPanelInfo));
			_ASSERTE(pi == (&pviLeft) || pi == (&pviRight));
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pi); //-V107
			TODO("����� WINDOW_LONG_FADE �� ������������ �����?");
			SetWindowLong(hwnd, WINDOW_LONG_FADE, 1); // Fade == false
			return 0; //continue creation
		}
		case WM_PAINT:
		{
			CeFullPanelInfo* pi = (CeFullPanelInfo*)GetWindowLongPtr(hwnd, GWLP_USERDATA); //-V204
			_ASSERTE(pi && pi->cbSize==sizeof(CeFullPanelInfo));
			_ASSERTE(pi == (&pviLeft) || pi == (&pviRight));
			//BYTE nPanelColorIdx = pi->nFarColors[col_PanelText];
			//COLORREF nBackColor = GetWindowLong(hwnd, 4*((nPanelColorIdx & 0xF0)>>4));
			//#ifdef _DEBUG
			//COLORREF nForeColor = GetWindowLong(hwnd, 4*(nPanelColorIdx & 0xF));
			//#endif
			PAINTSTRUCT ps = {NULL};
			HDC hdc = BeginPaint(hwnd, &ps);

			if (hdc)
			{
				RECT rc; GetClientRect(hwnd, &rc);
				pi->Paint(hwnd, ps, rc);
				EndPaint(hwnd, &ps);
			}

			return 0;
		}
		case WM_ERASEBKGND:
		{
			CeFullPanelInfo* pi = (CeFullPanelInfo*)GetWindowLongPtr(hwnd, GWLP_USERDATA); //-V204
			_ASSERTE(pi && pi->cbSize==sizeof(CeFullPanelInfo));
			_ASSERTE(pi == (&pviLeft) || pi == (&pviRight));
			BYTE nPanelColorIdx = pi->nFarColors[col_PanelText];
			COLORREF nBackColor = gcrCurColors[((nPanelColorIdx & 0xF0)>>4)]; //GetWindowLong(hwnd, 4*((nPanelColorIdx & 0xF0)>>4));
			RECT rc; GetClientRect(hwnd, &rc);
			HBRUSH hbr = CreateSolidBrush(nBackColor);
			_ASSERTE(wParam!=0);
			FillRect((HDC)wParam, &rc, hbr);
			DeleteObject(hbr);
			return TRUE;
		}
		case WM_MOUSEMOVE:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_LBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
		case WM_XBUTTONDOWN:
		case WM_XBUTTONUP:
		case WM_XBUTTONDBLCLK:
		case WM_MOUSEWHEEL:
		case WM_MOUSEHWHEEL:
		{
			if (!gbFarPanelsReady)
			{
				POINT pt = {LOWORD(lParam),HIWORD(lParam)};

				if (uMsg != WM_MOUSEWHEEL && uMsg != WM_MOUSEHWHEEL)
					MapWindowPoints(hwnd, ghConEmuRoot, &pt, 1);

				PostMessage(ghConEmuRoot, uMsg, wParam, MAKELPARAM(pt.x,pt.y));
				return 0;
			}

			CeFullPanelInfo* pi = (CeFullPanelInfo*)GetWindowLongPtr(hwnd, GWLP_USERDATA); //-V204
			_ASSERTE(pi && pi->cbSize==sizeof(CeFullPanelInfo));
			_ASSERTE(pi == (&pviLeft) || pi == (&pviRight));
			INT_PTR nIndex; COORD crCon;

			if (!pi->GetIndexFromWndCoord(LOWORD(lParam), HIWORD(lParam), nIndex))
			{
				if ((uMsg == WM_LBUTTONDOWN || uMsg == WM_RBUTTONDOWN) && (!pi->Focus))
				{
					pi->RequestSetPos(-1, -1, TRUE/*abSetFocus*/);
					//ConEmuThSynchroArg *pCmd = (ConEmuThSynchroArg*)LocalAlloc(LPTR, sizeof(ConEmuThSynchroArg)+128);
					//pCmd->bValid = 1; pCmd->bExpired = 0; pCmd->nCommand = ConEmuThSynchroArg::eExecuteMacro;
					//wsprintfW((wchar_t*)pCmd->Data, L"$If (%cPanel.Left) Tab $End",
					//	pi->bLeftPanel ? L'P' : L'A');
					//ExecuteInMainThread(pCmd);
				}
			}
			else
			{
				if (uMsg == WM_LBUTTONDOWN || uMsg == WM_RBUTTONDOWN)
				{
					pi->RequestSetPos(nIndex, pi->OurTopPanelItem, TRUE/*abSetFocus*/);
					//if (!pi->Focus)
					//{
					//	ConEmuThSynchroArg *pCmd = (ConEmuThSynchroArg*)LocalAlloc(LPTR, sizeof(ConEmuThSynchroArg)+128);
					//	pCmd->bValid = 1; pCmd->bExpired = 0; pCmd->nCommand = ConEmuThSynchroArg::eExecuteMacro;
					//	wsprintfW((wchar_t*)pCmd->Data, L"$If (%cPanel.Left) Tab $End",
					//		pi->bLeftPanel ? L'P' : L'A');
					//	ExecuteInMainThread(pCmd);
					//}

					// RClick ���������� ������
					if (uMsg != WM_RBUTTONDOWN)
					{
						return 0;
					}
				}

				//else if (uMsg == WM_LBUTTONUP || uMsg == WM_RBUTTONUP)
				//{
				//	//if (nIndex != pi->CurrentItem)
				//	//	pi->RequestSetPos(nIndex, pi->OurTopPanelItem);
				//	return 0;
				//}

				if (!pi->GetConCoordFromIndex(nIndex, crCon))
				{
					//	if (uMsg == WM_LBUTTONDOWN || uMsg == WM_RBUTTONDOWN)
					//	{
					//		pi->RequestSetPos(nIndex, pi->TopPanelItem);
					//		//ConEmuThSynchroArg *pCmd = (ConEmuThSynchroArg*)LocalAlloc(LPTR, sizeof(ConEmuThSynchroArg)+128);
					//		//pCmd->bValid = 1; pCmd->bExpired = 0; pCmd->nCommand = ConEmuThSynchroArg::eExecuteMacro;
					//		//wsprintfW((wchar_t*)pCmd->Data, L"panel.setposidx(%i,%i)",
					//		//	pi->Focus ? 0 : 1, nIndex+1);
					//		//ExecuteInMainThread(pCmd);
					//	}
					return 0;
				}

				RECT rcClient = {0};

				if (!GetClientRect(pi->hView, &rcClient) || !rcClient.right || !rcClient.bottom)
					return 0;

				POINT pt;
				pt.x = (crCon.X - pi->WorkRect.left) * rcClient.right / (pi->WorkRect.right - pi->WorkRect.left + 1);
				pt.y = (crCon.Y - pi->WorkRect.top) * rcClient.bottom / (pi->WorkRect.bottom - pi->WorkRect.top + 1);
				MapWindowPoints(hwnd, (uMsg==WM_MOUSEWHEEL || uMsg==WM_MOUSEHWHEEL) ? NULL : ghConEmuRoot, &pt, 1);
				pt.x++; pt.y++;
				PostMessage(ghConEmuRoot, uMsg, wParam, MAKELPARAM(pt.x,pt.y));
			}

			return 0;
		}
		case WM_CLOSE:
		{
			if (pviLeft.hView == hwnd)
				pviLeft.hView = NULL;
			else if (pviRight.hView == hwnd)
				pviRight.hView = NULL;

			DestroyWindow(hwnd);

			if (!pviLeft.hView && !pviRight.hView)
				PostThreadMessage(gnDisplayThreadId, WM_QUIT, 0, 0);

			return 0;
		}
		case WM_DESTROY:
		{
			if (!pviLeft.hView && !pviRight.hView)
				PostThreadMessage(gnDisplayThreadId, WM_QUIT, 0, 0);

			return 0;
		}
		default:

			if (uMsg == gnConEmuFadeMsg)
			{
				TODO("����� WINDOW_LONG_FADE �� ������������ �����?");
				SetWindowLong(hwnd, WINDOW_LONG_FADE, (DWORD)lParam);

				if (gbFadeColors != (lParam == 2))
				{
					gbFadeColors = (lParam == 2);
					//gcrCurColors = gbFadeColors ? gcrFadeColors : gcrActiveColors;
					gcrCurColors = gbFadeColors ? gThSet.crFadePalette : gThSet.crPalette;
					//Inva lidateRect(hwnd, NULL, FALSE); -- �� ���������
				}
			}
			else if (uMsg == gnConEmuSettingsMsg)
			{
				WARNING("gnConEmuSettingsMsg");
				//MFileMapping<PanelViewSetMapping> ThSetMap;
				//_ASSERTE(ghConEmuRoot!=NULL);

				//DWORD nGuiPID;
				//#ifdef _DEBUG
				//GetWindowThreadProcessId(ghConEmuRoot, &nGuiPID);
				//_ASSERTE(nGuiPID == wParam);
				//#endif
				//nGuiPID = (DWORD)wParam;

				//ThSetMap.InitName(CECONVIEWSETNAME, nGuiPID);
				//if (!ThSetMap.Open()) {
				//	MessageBox(NULL, ThSetMap.GetErrorText(), L"ConEmuTh", MB_ICONSTOP|MB_SETFOREGROUND|MB_SYSTEMMODAL);
				//} else {
				//	ThSetMap.GetTo(&gThSet);
				//	ThSetMap.CloseMap();
				if (LoadThSet((DWORD)wParam))
				{
					gpImgCache->Reset();

					// � ��������
					if (pviLeft.hView)
						pviLeft.OnSettingsChanged(TRUE);

					if (pviRight.hView)
						pviRight.OnSettingsChanged(TRUE);
				}
			}
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

DWORD WINAPI CeFullPanelInfo::DisplayThread(LPVOID lpvParam)
{
	MSG msg;
	HANDLE hReady = (HANDLE)lpvParam;
	CoInitialize(NULL);
	_ASSERTE(gpImgCache);
	gnConEmuFadeMsg = RegisterWindowMessage(CONEMUMSG_PNLVIEWFADE);
	gnConEmuSettingsMsg = RegisterWindowMessage(CONEMUMSG_PNLVIEWSETTINGS);

	// ���������� �������, ��� ���� ������
	if (hReady)
	{
		SetEvent(hReady);
	}
	else
	{
		_ASSERTE(hReady!=NULL);
	}

	while(GetMessage(&msg,0,0,0))
	{
		if (msg.message == MSG_CREATE_VIEW)
		{
			HANDLE hEvent = (HANDLE)msg.wParam;
			CeFullPanelInfo* pi = (CeFullPanelInfo*)msg.lParam;

			if (hEvent != ghCreateEvent)
			{
				_ASSERTE(hEvent == ghCreateEvent);
			}
			else
			{
				HWND lhView = pi->CreateView();
				_ASSERTE(pi->hView == lhView);
				pi->hView = lhView;
				SetEvent(hEvent);
			}

			continue;
		}

		// default
		DispatchMessage(&msg);
	}

	if (hClass)
	{
		UnregisterClass(gsDisplayClassName, (HINSTANCE)ghPluginModule);
		hClass = NULL;
	}

	//CloseHandle(ghDisplayThread); ghDisplayThread = NULL;
	gnDisplayThreadId = 0;
	// 26.04.2010 Maks - delete ������! �� ����� ��������� gdiplus.dll �� ������ �� ����. �������...
	//delete gpImgCache;
	//gpImgCache = NULL;
	gpImgCache->Reset(); // ������ Reset.
	// ���������� ������
	pviLeft.FinalRelease();
	pviRight.FinalRelease();
	UpdateEnvVar(FALSE);
	CoUninitialize();
	return 0;
}

// abCurrentItem  - ����� ���� TRUE ������ � �������� ������
// abSelectedItem - ������� ���������� �������
BOOL CeFullPanelInfo::PaintItem(
    HDC hdc, INT_PTR nIndex, int x, int y,
    CePluginPanelItem* pItem, CePluginPanelItemColor* pItemColor,
    BOOL abCurrentItem, BOOL abSelectedItem,
    /*COLORREF *nBackColor, COLORREF *nForeColor, HBRUSH *hBack,*/
    /*BOOL abAllowPreview,*/ HBRUSH hBackBrush, HBRUSH hPanelBrush, COLORREF crPanelColor)
{
	//const wchar_t* pszName = pItem->FindData.lpwszFileNamePart;
	//int nLen = lstrlen(pszName);
	//int nDrawRC = -1;
	//SHFILEINFO sfi = {NULL};
	//UINT cbSize = sizeof(sfi);
	//DWORD_PTR shRc = 0;
	//HICON hIcon = NULL;
	//HBITMAP hBmp = NULL;
	if (gbCancelAll)
		return FALSE;

	if (!pItem || !pItem->pszFullName)
	{
#ifdef _DEBUG
		_ASSERTE(pItem!=NULL);

		if (pItem) _ASSERTE(pItem->pszFullName != NULL);

#endif
		return FALSE;
	}

	RECT rcFull = {x, y, x+nWholeW, y+nWholeH};
	FillRect(hdc, &rcFull, hPanelBrush); //hBack[0]);

	if (gThSet.nPreviewFrame == 1)
	{
		// ������ ��� ���������. �� ����� ���������� �� ���� ������
		HBRUSH hOldBr = (HBRUSH)SelectObject(hdc, hBackBrush);
		Rectangle(hdc,
		          x+Spaces.nSpaceX1, y+Spaces.nSpaceY1,
		          x+Spaces.nSpaceX1+(Spaces.nImgSize+2*gThSet.nPreviewFrame), y+Spaces.nSpaceY1+(Spaces.nImgSize+2*gThSet.nPreviewFrame));
		/*
		Rectangle(hdc,
			x+gThSet.nHPadding, y+gThSet.nVPadding,
			x+gThSet.nHPadding+gThSet.nWidth, y+gThSet.nVPadding+gThSet.nHeight);
		*/
		SelectObject(hdc, hOldBr);
	}
	else if (gThSet.nPreviewFrame == 0)
	{
		RECT rcTmp = {x+Spaces.nSpaceX1, y+Spaces.nSpaceY1,
		              x+Spaces.nSpaceX1+(Spaces.nImgSize+2*gThSet.nPreviewFrame), y+Spaces.nSpaceY1+(Spaces.nImgSize+2*gThSet.nPreviewFrame)
		             };
		/*
		RECT rcTmp = {x+gThSet.nHPadding, y+gThSet.nVPadding,
			x+gThSet.nHPadding+gThSet.nWidth, y+gThSet.nVPadding+gThSet.nHeight};
		*/
		// ������ ��� ���������. �� ����� ���������� �� ���� ������
		FillRect(hdc, &rcTmp, hBackBrush);
	}
	else
	{
		_ASSERTE(gThSet.nPreviewFrame==0 || gThSet.nPreviewFrame==1);
		return FALSE;
	}

	const wchar_t* pszComments = NULL;

	BOOL bIgnoreFileDescription = FALSE;

	gpImgCache->PaintItem(hdc, x+gThSet.nPreviewFrame+Spaces.nSpaceX1, y+gThSet.nPreviewFrame+Spaces.nSpaceY1,
	                      Spaces.nImgSize, pItem, /*abAllowPreview,*/ &pszComments, &bIgnoreFileDescription);

	RECT rcClip = {0}, rcMaxText;
	COLORREF crBack = 0, crFore = 0;

	if (PVM == pvm_Thumbnails)
	{
		rcClip.left   = x+Spaces.nSpaceX1;
		rcClip.top    = y+Spaces.nSpaceY1+(Spaces.nImgSize+2*gThSet.nPreviewFrame); //+Spaces.nLabelSpacing;
		rcClip.right  = x+Spaces.nSpaceX1+(Spaces.nImgSize+2*gThSet.nPreviewFrame);
		rcClip.bottom = y+Spaces.nSpaceY1+(Spaces.nImgSize+2*gThSet.nPreviewFrame)+nFontHeight+2*Spaces.nLabelSpacing;
		//
		rcMaxText = rcClip;
	}
	else if (PVM == pvm_Tiles)
	{
		rcClip.left   = x+Spaces.nSpaceX1+(Spaces.nImgSize+2*gThSet.nPreviewFrame); //+Spaces.nLabelSpacing;
		rcClip.top    = y+Spaces.nSpaceY1;
		rcClip.right  = x+Spaces.nSpaceX1+(Spaces.nImgSize+2*gThSet.nPreviewFrame)+Spaces.nSpaceX2;
		rcClip.bottom = y+Spaces.nSpaceY1+(Spaces.nImgSize+2*gThSet.nPreviewFrame);
		//
		rcMaxText = rcClip;
		rcMaxText.top = y;
		rcMaxText.bottom = y+2*Spaces.nSpaceY1+(Spaces.nImgSize+2*gThSet.nPreviewFrame);
	}

	//int nIdx = ((pItem->Flags & (0x40000000/*PPIF_SELECTED*/)) ?
	//	((abCurrentItem/*nItem==nCurrentItem*/) ? 3 : 1) :
	//	((abCurrentItem/*nItem==nCurrentItem*/) ? 2 : 0));
	//crBack = nBackColor[nIdx];
	//crFore = nForeColor[nIdx];
	//if ((pItem->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	//	&& ((pItem->Flags & (0x40000000/*PPIF_SELECTED*/)) == 0))
	//	crFore = 0xFFFFFF;
	HBRUSH hTextBrush = GetItemColors(nIndex, pItem, pItemColor, abCurrentItem, crFore, crBack);
	RECT rcFrame = rcClip;
	// ���� � ������ Tiles ����� ����� ���������� ������� ���� - DrawItemText ���� ���������� �������������
	FillRect(hdc, &rcClip, hTextBrush); //hBack[nIdx]);
	SetTextColor(hdc, crFore);
	SetBkColor(hdc, crBack);

	//ExtTextOut(hdc, rcClip.left,rcClip.top, ETO_CLIPPED, &rcClip, pszName, nLen, NULL);
	if (PVM == pvm_Thumbnails)
	{
		if (Spaces.nLabelPadding && Spaces.nLabelPadding < ((rcClip.right-rcClip.left)/3))
		{
			rcClip.left += Spaces.nLabelPadding; rcMaxText.left = rcClip.left;
			rcClip.right -= Spaces.nLabelPadding; rcMaxText.right = rcClip.right;
		}
	}
	else if (PVM == pvm_Tiles)
	{
		if (Spaces.nLabelSpacing > 0 && Spaces.nLabelSpacing < ((rcClip.right-rcClip.left)/3))
		{
			rcClip.left += Spaces.nLabelSpacing; rcMaxText.left = rcClip.left;
		}

		if (Spaces.nLabelPadding && Spaces.nLabelPadding < ((rcClip.right-rcClip.left)/3))
		{
			rcClip.right -= Spaces.nLabelPadding; rcMaxText.right = rcClip.right;
		}
	}

	//DrawText(hdc, pszName, nLen, &rcClip, DT_END_ELLIPSIS|DT_NOPREFIX|DT_SINGLELINE|DT_VCENTER);
	DrawItemText(hdc, &rcClip, &rcMaxText, pItem, pszComments, hTextBrush/*hBack[nIdx]*/, bIgnoreFileDescription);

	if (abSelectedItem && gThSet.nSelectFrame == 1)
	{
		int nX = x, nY = y;
		int nW = 0, nH = 0;

		// ������� ����� ��� ���������� �������������
		if (PVM == pvm_Tiles)
		{
			rcFrame.top = rcClip.top; rcFrame.bottom = rcClip.bottom;
		}

		if (Spaces.nSpaceX1 > 1) nX += (Spaces.nSpaceX1-1);

		if (Spaces.nSpaceY1 > 1) nY += (Spaces.nSpaceY1-1);

		if (PVM == pvm_Thumbnails)
		{
			nW = rcFrame.right - nX; // + 1;
			nH = rcFrame.bottom - nY; // + 1;
		}
		else
		{
			nY = max(y,(rcFrame.top-1));
			nW = rcFrame.right - nX; // + 1;
			nH = rcFrame.bottom - nY;
		}

		if (nW && nH)
		{
			COLORREF crPen = 0;

			if (gThSet.crSelectFrame.UseIndex)
			{
				if (gThSet.crSelectFrame.ColorIdx <= 15)
					crPen = gcrCurColors[gThSet.crSelectFrame.ColorIdx];
				else
					crPen = gcrCurColors[7];
			}
			else
			{
				crPen = gThSet.crSelectFrame.ColorRGB;
			}

			HPEN hPen = CreatePen(PS_DOT, 1, crPen);
			HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
			SetBkColor(hdc, crPanelColor); //nBackColor[0]);
			MoveToEx(hdc, nX, nY, NULL);
			LineTo(hdc, nX+nW, nY);
			LineTo(hdc, nX+nW, nY+nH);
			LineTo(hdc, nX, nY+nH);
			LineTo(hdc, nX, nY);
			SelectObject(hdc, hOldPen);
			DeleteObject(hPen);
		}
	}

	return TRUE;
}

int CeFullPanelInfo::DrawItemText(HDC hdc, LPRECT prcText, LPRECT prcMaxText, CePluginPanelItem* pItem, LPCWSTR pszComments, HBRUSH hBr, BOOL bIgnoreFileDescription)
{
	int iRc = 0;
	const wchar_t* pszName = pItem->FindData.lpwszFileNamePart;
	int nLen = lstrlen(pszName);
	RECT rcClip = *prcText;

	if (PVM == pvm_Thumbnails)
	{
		const wchar_t* pszExt = NULL;

		// ������ ��� ������, ��� ����� "����������" �� �������, ��� ��� �� ����������.
		if ((pItem->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		{
			pszExt = wcsrchr(pszName, L'.');

			if (pszExt == pszName) pszExt = NULL;  // ���� ����� ����, � ��� ���������� � ���
		}

		BOOL  lbCorrected = FALSE;
		DWORD nDrawFlags = DT_END_ELLIPSIS|DT_NOPREFIX|DT_SINGLELINE|DT_VCENTER;

		if (pszExt)
		{
			RECT rcExt = rcClip;
			RECT rcName = rcClip;
			int nExtLen = lstrlen(pszExt);
			int nNameLen = (int)(pszExt - pszName);
			int iRc1 = DrawText(hdc, pszExt, nExtLen, &rcExt, DT_CALCRECT|DT_NOPREFIX|DT_SINGLELINE|DT_TOP|DT_LEFT);
			int iRc2 = DrawText(hdc, pszName, nNameLen, &rcName, DT_CALCRECT|DT_NOPREFIX|DT_SINGLELINE|DT_TOP|DT_LEFT);

			// ������ ���������� �������� ������ ���� ����� ���������� � �������� ������ �������� �� ����������� prcText
			if (iRc1 && iRc2)
			{
				if ((gThSet.nPreviewFrame == 0)
				        && ((rcExt.right-rcExt.left)+(rcName.right-rcName.left)) <= (rcClip.right-rcClip.left))
				{
					lbCorrected = FALSE; // ���� - �������� �� ������
				}
				else if ((rcExt.right-rcExt.left) <= ((rcClip.right-rcClip.left)/2 /*-(rcName.right-rcName.left)*/))
				{
					rcExt.left = rcClip.right-(rcExt.right-rcExt.left);
					rcExt.right = rcClip.right; rcExt.top = rcClip.top; rcExt.bottom = rcClip.bottom;
					iRc = DrawText(hdc, pszExt, nExtLen, &rcExt, DT_NOPREFIX|DT_SINGLELINE|DT_VCENTER);
					nLen = nNameLen;
					rcClip.right = rcExt.left;
					lbCorrected = TRUE; //  ����� ���� �� ������ �� �����������
				}
			}
		}
		else if (gThSet.nPreviewFrame == 0)
		{
			RECT rcName = rcClip;
			int iRc1 = DrawText(hdc, pszName, nLen, &rcName, DT_CALCRECT|DT_NOPREFIX|DT_SINGLELINE|DT_TOP|DT_LEFT);

			if (iRc1 && ((rcName.right-rcName.left) >= (rcClip.right-rcClip.left)))
				lbCorrected = TRUE; //  ����� ���� �� ������ �� �����������
		}

		if (!lbCorrected && gThSet.nPreviewFrame == 0)
		{
			nDrawFlags |= DT_CENTER; // ��� �������, ��� ���������� �����, ������� �� ������.
		}

		iRc = DrawText(hdc, pszName, nLen, &rcClip, nDrawFlags);
	}
	else
	{
		TODO("��������� pItem->pszInfo");
		wchar_t szFullInfo[MAX_PATH*4];
		wchar_t* psz = szFullInfo;
		// ���������
		StringCchCopyN(psz, MAX_PATH, pszName, MAX_PATH); psz[MAX_PATH] = 0;
		psz += lstrlen(psz);

		if (!(pszName[0] == L'.' && (pszName[1] == 0 || (pszName[1] == L'.' && pszName[2] == 0))))
		{
			*(psz++) = L'\n'; *psz = 0;

			if ((pItem->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
			{
				if ((pItem->NumberOfLinks > 1) || (pItem->FindData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
				{
					*(psz++) = L'<'; *psz = 0;
					lstrcpy(psz, gsSymLink);
					//gsJunction, gsSymLink, gsFolder, gsHardLink
					//else if (pItem->FindData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
					//	lstrcpy(psz, gsSymLink);
					//else
					//	lstrcpy(psz, gsFolder);
					psz += lstrlen(psz);
					*(psz++) = L'>'; *(psz++) = L' '; *psz = 0;
				}
			}

			SYSTEMTIME st; FileTimeToSystemTime(&pItem->FindData.ftLastWriteTime, &st);
			_wsprintf(psz, SKIPLEN(countof(szFullInfo)-(psz-szFullInfo)) L"%02i.%02i.%02i %i:%02i", st.wDay, st.wMonth, st.wYear % 100, st.wHour, st.wMinute);
			psz += lstrlen(psz);

			if (pItem->FindData.nFileSize
			        || ((pItem->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0))
			{
				*(psz++) = L' '; *(psz++) = L' '; *psz = 0;
				__int64 nSize = pItem->FindData.nFileSize;
				wchar_t cType = L'B';
				int n = 0;
				const wchar_t cTypeList[] = L"BKMGTP";
				int nMax = countof(cTypeList);

				while(nSize > 9999)
				{
					nSize = nSize >> 10; n++;
				}
				if (n < nMax) cType = cTypeList[n]; else cType = L'?';

				if (nSize > 999)
				{
					int nAll = (int)nSize;
					int nL = nAll % 1000;
					int nH = nAll / 1000;
					_wsprintf(psz, SKIPLEN(countof(szFullInfo)-(psz-szFullInfo)) L"%i,%i %c", nH, nL, cType);
				}
				else
				{
					_wsprintf(psz, SKIPLEN(countof(szFullInfo)-(psz-szFullInfo)) L"%i %c", (int)nSize, cType);
				}

				psz += lstrlen(psz);
			}

			*psz = 0; // \0 ended
		}

		if (pszComments && *pszComments)
		{
			*(psz++) = L'\n'; *psz = 0;
			lstrcpyn(psz, pszComments, MAX_PATH); psz += lstrlen(psz);
		}

		if (!bIgnoreFileDescription && pItem->pszDescription && *pItem->pszDescription)
		{
			*(psz++) = L'\n'; *psz = 0;
			lstrcpyn(psz, pItem->pszDescription, MAX_PATH); psz += lstrlen(psz);
		}

		*psz = 0; // \0 ended
		nLen = lstrlen(szFullInfo);
		DWORD nFlags = DT_WORD_ELLIPSIS|DT_NOPREFIX|DT_TOP|DT_LEFT;
		RECT rcText = rcClip; rcText.bottom = nFontHeight;
		// ���� ��� ������ �� ���������� - ����� ������� � �������� ����
		iRc = DrawText(hdc, szFullInfo, nLen, &rcText, DT_CALCRECT|nFlags);
		int nCellHeight = (rcClip.bottom-rcClip.top);
		int nTextHeight = (rcText.bottom-rcText.top);

		if (nTextHeight < nCellHeight)
		{
			rcClip.top = rcClip.top + ((nCellHeight - nTextHeight) >> 1);
		}
		else if (nTextHeight > nCellHeight)
		{
			RECT rcFull = *prcMaxText;
			int nFullHeight = rcFull.bottom-rcFull.top;

			if (nFullHeight > nCellHeight)
			{
				if (nTextHeight < nFullHeight)
				{
					rcClip.top = rcFull.top = rcFull.top + ((nFullHeight - nTextHeight) >> 1);
					rcClip.bottom = rcFull.bottom = rcFull.top + nTextHeight;
				}
				else
				{
					rcClip.top = rcFull.top;
					rcClip.bottom = rcFull.bottom;
				}

				FillRect(hdc, &rcFull, hBr);
			}
		}

		// ������ - ������
		iRc = DrawText(hdc, szFullInfo, nLen, &rcClip, nFlags); //-V519

		// ������� ����������� �������������
		if (rcClip.top < prcText->top) prcText->top = rcClip.top;

		if (rcClip.bottom > prcText->bottom) prcText->bottom = rcClip.bottom;
	}

	return iRc;
}

BOOL CeFullPanelInfo::GetIndexFromWndCoord(int x, int y, INT_PTR &rnIndex)
{
	if (!nWholeW || !nWholeH)
		return FALSE;

	//RECT rcClient = {0};
	//if (!GetClientRect(hView, &rcClient) || !rcClient.right || !rcClient.bottom)
	//	return FALSE;
	int xCell = x / nWholeW;
	int yCell = y / nWholeH;

	if (PVM == pvm_Thumbnails)
	{
		rnIndex = OurTopPanelItem + yCell * nXCountFull + xCell;
	}
	else if (PVM == pvm_Tiles)
	{
		rnIndex = OurTopPanelItem + xCell * nYCountFull + yCell;
	}
	else
	{
		_ASSERTE(PVM == pvm_Thumbnails || PVM == pvm_Tiles);
		return FALSE; // �� ��������������
	}

	if (rnIndex < 0 || rnIndex >= ItemsNumber)
		return FALSE;

	return TRUE;
}

BOOL CeFullPanelInfo::GetConCoordFromIndex(INT_PTR nIndex, COORD& rCoord)
{
	BOOL lbRc = FALSE;
	INT_PTR nCol0Index = nIndex - TopPanelItem;

	if (nCol0Index >= 0 && nCol0Index <= (WorkRect.bottom - WorkRect.top))
	{
		rCoord.X = (SHORT)WorkRect.left; rCoord.Y = (SHORT)(WorkRect.top+nCol0Index); lbRc = TRUE;
	}

	TODO("� ������� ����� ���� ����� ����� ������� N - ����� �� ����� ����������!");
	return lbRc;
}

void CeFullPanelInfo::LoadItemColors(INT_PTR nIndex, CePluginPanelItem* pItem, CePluginPanelItemColor* pItemColor, BOOL abCurrentItem, BOOL abStrictConsole)
{
	// ������ ���� ������� "������". ��� ��������� ����� ������ ������, � ���� ����� "�����".
	if (gbFarPanelsReady)
	{
		COORD crItem;

		if (GetConCoordFromIndex(nIndex, crItem))
		{
			wchar_t c; CharAttr a;
			//if (gpRgnDetect->GetCharAttr(WorkRect.left, WorkRect.top+nCol0Index, c, a)) {
			WARNING("���� � ���� ������� - ������, �� ��������� ����� ����� ������������!");

			if (gpRgnDetect->GetCharAttr(crItem.X, crItem.Y, c, a))
			{
				pItemColor->crBack = gcrCurColors[a.nBackIdx];
				pItemColor->crFore = gcrCurColors[a.nForeIdx];
				pItemColor->bItemColorLoaded = TRUE;
				return;
			}
		}
	}

	// ���� ����� �������� ��� ��� ���� �������, � ������� ������ "�����" ��-�� ���������.
	if (!pItemColor->bItemColorLoaded)
	{
		// ��������� �������� ����� ���� �������� � �������, �� ������ � PanelView.
		// ����������� �������� �� �� �����������, �� ���������
		bool bFound = false;
		DWORD nMasks[] =
		{
			0xFFFFFFFF,
			FILE_ATTRIBUTE_NORMAL|FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM,
			FILE_ATTRIBUTE_NORMAL|FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_HIDDEN,
			FILE_ATTRIBUTE_NORMAL|FILE_ATTRIBUTE_DIRECTORY,
		};

		for(int m = 0; !bFound && m < ARRAYSIZE(nMasks); m++)
		{
			DWORD nMask = nMasks[m];

			for(INT_PTR j = this->TopPanelItem; !bFound && j < this->ItemsNumber; j++)
			{
				CePluginPanelItem* pCmp = this->ppItems[j];
				CePluginPanelItemColor* pCmpColor = this->pItemColors+j;

				if (!pCmp)
				{
					_ASSERTE(this->ppItems[j]!=NULL);
					continue; // ������?
				}

				if (pCmp == pItem || !pCmpColor->bItemColorLoaded)
					continue;

				if (pCmp->Flags == pItem->Flags
				        && (nMask&pCmp->FindData.dwFileAttributes) == (nMask&pItem->FindData.dwFileAttributes)
				        && (!Focus || (j != this->CurrentItem))
				  )
				{
					pItemColor->crBack = pCmpColor->crBack;
					pItemColor->crFore = pCmpColor->crFore;
					bFound = true; // ����� ����������
					break;
				}
			}
		}

		if (!bFound)
		{
			// ���� �� ������� - ����� �� ��������� (��� �������� �����)
			int nIdx = ((pItem->Flags & (0x40000000/*PPIF_SELECTED*/)) ?
			            ((abCurrentItem/*nItem==nCurrentItem*/) ? col_PanelSelectedCursor : col_PanelSelectedText) :
				            ((abCurrentItem/*nItem==nCurrentItem*/) ? col_PanelCursor : col_PanelText));
			pItemColor->crBack = gcrCurColors[((nFarColors[nIdx] & 0xF0)>>4)];
			pItemColor->crFore = gcrCurColors[(nFarColors[nIdx] & 0xF)];
		}

		//// ��� ����� - ������ ����� �����
		//if (pItem->crBack != 0xFFFFFF
		//	&& (pItem->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		//	&& ((pItem->Flags & (0x40000000/*PPIF_SELECTED*/)) == 0))
		//{
		//	pItem->crFore = 0xFFFFFF;
		//}
	}
}

HBRUSH CeFullPanelInfo::GetItemColors(INT_PTR nIndex, CePluginPanelItem* pItem, CePluginPanelItemColor* pItemColor, BOOL abCurrentItem, COLORREF &crFore, COLORREF &crBack)
{
	//COORD crItem;
	//if (GetConCoordFromIndex(nIndex, crItem))
	////int nCol0Index = nIndex - TopPanelItem;
	////if (nCol0Index >= 0 && nCol0Index <= (WorkRect.bottom - WorkRect.top))
	//{
	//	wchar_t c; CharAttr a;
	//	//if (gpRgnDetect->GetCharAttr(WorkRect.left, WorkRect.top+nCol0Index, c, a)) {
	//	if (gpRgnDetect->GetCharAttr(crItem.X, crItem.Y, c, a)) {
	//		pItem->crBack = crBack = gcrCurColors[a.nBackIdx];
	//		pItem->crFore = crFore = gcrCurColors[a.nForeIdx];
	//		pItem->bItemColorLoaded = TRUE;
	//		goto ChkBrush;
	//	}
	//}
	//
	//if (pItem->bItemColorLoaded) {
	//	// ���� ����� �������� ��� ��� ���� �������, � ������� ������ "�����" ��-�� ���������.
	crBack = pItemColor->crBack;
	crFore = pItemColor->crFore;

	//} else {
	//	// ���� �� ������� - ����� �� ��������� (��� �������� �����)
	//	int nIdx = ((pItem->Flags & (0x40000000/*PPIF_SELECTED*/)) ?
	//		((abCurrentItem/*nItem==nCurrentItem*/) ? col_PanelSelectedCursor : col_PanelSelectedText) :
	//		((abCurrentItem/*nItem==nCurrentItem*/) ? col_PanelCursor : col_PanelText));
	//
	//	crBack = gcrCurColors[((nFarColors[nIdx] & 0xF0)>>4)];
	//	crFore = gcrCurColors[(nFarColors[nIdx] & 0xF)];
	//
	//	// ��� ����� - ������ ����� �����
	//	if (crBack != 0xFFFFFF
	//		&& (pItem->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	//		&& ((pItem->Flags & (0x40000000/*PPIF_SELECTED*/)) == 0))
	//	{
	//		crFore = 0xFFFFFF;
	//	}
	//}

//ChkBrush:
	if (!hLastBackBrush || crBack != crLastBackBrush)
	{
		if (hLastBackBrush) DeleteObject(hLastBackBrush);

		hLastBackBrush = CreateSolidBrush(crBack);
		crLastBackBrush = crBack;
	}

	return hLastBackBrush;
}

INT_PTR CeFullPanelInfo::CalcTopPanelItem(INT_PTR anCurrentItem, INT_PTR anTopItem)
{
	INT_PTR nTopItem = anTopItem;
	INT_PTR nItemCount = this->ItemsNumber;
	INT_PTR nCurrentItem = anCurrentItem;
	//CePluginPanelItem** ppItems = this->ppItems;

	if ((nTopItem + nXCountFull*(nYCountFull)) <= nCurrentItem)
	{
		TODO("������������ �� ������� nXCount");
		nTopItem = nCurrentItem - (nXCountFull*(nYCountFull)) + 1;

		//
		if (PVM == pvm_Thumbnails)
		{
			INT_PTR n = (nTopItem + (nXCount-1)) / nXCount;
			INT_PTR nNewTop = n * nXCount;
			nTopItem = min(nNewTop, (nItemCount-1));
			//int nMod = nTopItem % nXCount;
			//if (nMod) {
			//	nTopItem = max(0,nTopItem-nMod);
			//	//if (nTopItem > nCurrentItem)
			//	//	nTopItem = max(nCurrentItem,(nTopItem-nXCount));
			//}
		}
		else
		{
			INT_PTR n = (nTopItem + (nYCountFull-1)) / nYCountFull;
			INT_PTR nNewTop = n * nYCountFull;
			nTopItem = min(nNewTop, (nItemCount-1));
			//if (nTopItem > nCurrentItem)
			//	nTopItem = max(0,min((nTopItem-nYCountFull),nCurrentItem
		}
	}
	else
	{
		if (nTopItem > nCurrentItem)
			nTopItem = nCurrentItem;

		//
		if (PVM == pvm_Thumbnails)
		{
			INT_PTR nMod = nTopItem % nXCount;

			if (nMod)
			{
				nTopItem = max(0,nTopItem-nMod);
			}

			while((nTopItem + nXCountFull*(nYCountFull)) <= nCurrentItem)
			{
				nTopItem += nXCountFull;
			}
		}
		else
		{
			INT_PTR nMod = nTopItem % nYCountFull;

			if (nMod)
			{
				nTopItem = max(0,nTopItem-nMod);
			}

			while((nTopItem + nXCountFull*(nYCountFull)) <= nCurrentItem)
			{
				nTopItem += nYCountFull;
			}
		}
	}

	return nTopItem;
}

void CeFullPanelInfo::InvalidateAll()
{
	if (pviLeft.hView && IsWindow(pviLeft.hView))
		pviLeft.Invalidate();

	if (pviRight.hView && IsWindow(pviRight.hView))
		pviRight.Invalidate();
}

void CeFullPanelInfo::Invalidate()
{
	if (!this)
		return;

	if (!this->hView || !IsWindow(this->hView))
	{
		_ASSERTE(IsWindow(this->hView));

		if (this->hView)
		{
			this->Close();
		}

		return;
	}

	if (!IsWindowVisible(this->hView))
		return;

	DEBUGSTRCTRL(L"Invalidating panel\n");
	InvalidateRect(this->hView, NULL, FALSE);
}

void CeFullPanelInfo::Paint(HWND hwnd, PAINTSTRUCT& ps, RECT& rc)
{
	gbCancelAll = FALSE;
	DEBUGSTRPAINT(L"WM_PAINT\n");
	HDC hdc = ps.hdc;
	//for (int i=0; i<16; i++)
	//	gcrColors[i] = GetWindowLong(hwnd, 4*i);
	//BYTE nIndexes[4] = {
	//	this->nFarColors[col_PanelText],
	//	this->nFarColors[col_PanelSelectedText],
	//	this->nFarColors[col_PanelCursor],
	//	this->nFarColors[col_PanelSelectedCursor]
	//};
	//COLORREF nBackColor[4], nForeColor[4];
	//HBRUSH hBack[4];
	//int i;
	//for (i = 0; i < 4; i++) {
	//	nBackColor[i] = gcrCurColors[((nIndexes[i] & 0xF0)>>4)];
	//	nForeColor[i] = gcrCurColors[(nIndexes[i] & 0xF)];
	//	hBack[i] = CreateSolidBrush(nBackColor[i]);
	//}
	COLORREF crGray = gcrCurColors[8];
	COLORREF crPanel = gcrCurColors[((nFarColors[col_PanelText] & 0xF0)>>4)]; //nBackColor[0]; //gcrColors[15];
	COLORREF crBack = crPanel;

	if (gThSet.crBackground.UseIndex)
	{
		if (gThSet.crBackground.ColorIdx <= 15)
			crBack = gcrCurColors[gThSet.crBackground.ColorIdx];
	}
	else
	{
		crBack = gThSet.crBackground.ColorRGB;
	}

	if (gThSet.crPreviewFrame.UseIndex)
	{
		if (gThSet.crPreviewFrame.ColorIdx <= 15)
			crGray = gcrCurColors[gThSet.crPreviewFrame.ColorIdx];
	}
	else
	{
		crGray = gThSet.crPreviewFrame.ColorRGB;
	}

	HPEN hPen = CreatePen(PS_SOLID, 1, crGray);
	HBRUSH hPanelBrush = CreateSolidBrush(crPanel);
	HBRUSH hBackBrush = CreateSolidBrush(crBack);
	HFONT hFont = CreateFont(nFontHeight,0,0,0,400,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
	                         gThSet.nFontQuality,DEFAULT_PITCH,sFontName);
	// ����������� ����� �� ������� �����/������������� ��������
	gpImgCache->Init(crBack);

	if (PVM == pvm_Thumbnails)
	{
		if ((2*Spaces.nLabelSpacing+nFontHeight) > Spaces.nSpaceY2)
		{
			Spaces.nSpaceY2 = (2*Spaces.nLabelSpacing+nFontHeight);
		}
	}

	nWholeW = (Spaces.nImgSize+2*gThSet.nPreviewFrame)  + Spaces.nSpaceX2 + Spaces.nSpaceX1*2;
	nWholeH = (Spaces.nImgSize+2*gThSet.nPreviewFrame) + Spaces.nSpaceY2 + Spaces.nSpaceY1*2;

	if (!nWholeW || !nWholeH)
	{
		_ASSERTE(nWholeW && nWholeH);
		return;
	}

	nXCountFull = (rc.right/*+Spaces.nSpaceX2*/) / nWholeW; // ��� �����, ����� ������ �� �����������
	nYCountFull = (rc.bottom/*+Spaces.nSpaceY2*/) / nWholeH; // ��� �����, ����� ������ �� �����������

	if (PVM == pvm_Thumbnails)
	{
		nXCount = nXCountFull; // ����� ������ �� �����������.
		nYCount = (rc.bottom+(Spaces.nImgSize+2*gThSet.nPreviewFrame)+Spaces.nSpaceY2) / nWholeH; // � ��� ����������� ����������� ������� ����� ������
	}
	else
	{
		nXCount = (rc.right+Spaces.nSpaceX2) / nWholeW; // � ��� ����������� ����������� ����� ����� ������
		nYCount = nYCountFull; // �� ���� ��������� ������� - ����� �� ���� �� �����������.
	}

	if (nXCount < 1) nXCount = 1;

	if (nXCountFull < 1) nXCountFull = 1;

	if (nYCount < 1) nYCount = 1;

	if (nYCountFull < 1) nYCountFull = 1;

	//this->nXCount = nXCount; this->nYCountFull = nYCountFull; this->nYCount = nYCount;
	//int nTopItem = this->TopPanelItem;
	INT_PTR nItemCount = this->ItemsNumber;
	INT_PTR nCurrentItem = this->CurrentItem;
	INT_PTR nTopItem = this->TopPanelItem;
	// Issue 321: ������������ ��������� ��� ��������� ��������� ������, ������� �������� ����������� "���" Top ������
	//if (nCurrentItem == this->ReqCurrentItem && nTopItem < this->ReqTopPanelItem)
	nTopItem = this->ReqTopPanelItem;
	nTopItem = CalcTopPanelItem(nCurrentItem, nTopItem);
	this->OurTopPanelItem = nTopItem;

	// ���� ������ �� ���� ������ �� ����� ��������� ������� - ������� ���� �����������
	if (!bRequestItemSet && ReqTopPanelItem != OurTopPanelItem)
	{
		ReqTopPanelItem = OurTopPanelItem;
		ReqCurrentItem = CurrentItem;
	}

#ifdef _DEBUG
	wchar_t szDbg[512];
	_wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"CeFullPanelInfo::Paint(%s, Items=%i, Top=%i, Current=%i)\n",
	          this->bLeftPanel ? L"Left" : L"Right", ItemsNumber, OurTopPanelItem, CurrentItem);
	DEBUGSTRCTRL(szDbg);
#ifdef DEBUG_PAINT
	FillRect(hdc, &rc, (HBRUSH)GetStockObject(GRAY_BRUSH));
#endif
#endif
	int nMaxLen = (this->pszPanelDir ? lstrlen(this->pszPanelDir) : 0) + MAX_PATH+3;
	wchar_t* pszFull = (wchar_t*)malloc(nMaxLen*2);
	wchar_t* pszNamePtr = NULL;

	if (this->pszPanelDir && *this->pszPanelDir)
	{
		lstrcpy(pszFull, this->pszPanelDir);
		pszNamePtr = pszFull+lstrlen(pszFull);

		if (*(pszNamePtr-1) != L'\\')
		{
			*(pszNamePtr++) = L'\\';
			*pszNamePtr = 0;
		}
	}
	else
	{
		pszNamePtr = pszFull;
	}

	//int nXIcon = GetSystemMetrics(SM_CXICON);
	//int nYIcon = GetSystemMetrics(SM_CYICON);
	//int nXIconSpace = ((Spaces.nImgSize+2*gThSet.nPreviewFrame) - nXIcon) >> 1;
	//int nYIconSpace = ((Spaces.nImgSize+2*gThSet.nPreviewFrame) - nYIcon) >> 1;
	HDC hCompDC = CreateCompatibleDC(hdc);
	HBITMAP hCompBmp = CreateCompatibleBitmap(hdc, nWholeW, nWholeH);
	HBITMAP hOldCompBmp = (HBITMAP)SelectObject(hCompDC, hCompBmp);
	{
		RECT rcComp = {0,0,nWholeW,nWholeH};
		FillRect(hCompDC, &rcComp, hPanelBrush); //hBack[0]);
	}
	HPEN hOldPen = (HPEN)SelectObject(hCompDC, hPen);
	HBRUSH hOldBr = (HBRUSH)SelectObject(hCompDC, hPanelBrush);
	HFONT hOldFont = (HFONT)SelectObject(hCompDC,hFont);
	//if (!this->Focus)
	//	nCurrentItem = -1;
	MSectionLock CS;

	if (CS.Lock(pSection, FALSE, 1000))
	{
		// ���� - ������ � ��� ����
		// 0. ���������� ��������� (�������� ���� ������ �������� �� �������)
		// 1. ��������� ������ ������ (��� ������ SHGetFileInfo), �� ���� ����
		//    ����������� ������� ������� ���������.
		// -- 2. ��������� ��������� (��� ��� ���������, � ������� �� ������� ��������)
		for(int nStep = 0; !gbCancelAll && nStep <= 1; nStep++)
		{
			//if (nStep == 2)
			//{
			//	if ((gThSet.bLoadPreviews & PVM) == 0)
			//		continue; // ��� ����� ������ ��������� �� �������
			//}
			_ASSERTE(nTopItem>=0 && nTopItem<nItemCount && nItemCount>=0);
			INT_PTR nItem = nTopItem;
			int nYCoord = 0, nXCoord = 0;
			int X = -1, Y = -1;
			int iCount = (PVM == pvm_Thumbnails) ? nYCount : nXCount;
			int jCount = (PVM == pvm_Thumbnails) ? nXCount : nYCount;

			for(INT_PTR i = 0; !gbCancelAll && i < iCount && nItem < nItemCount; i++)
			{
				if (PVM == pvm_Thumbnails)
				{
					nXCoord = 0; Y++; X = -1;
					//nYCoord = Y * nWholeH;
				}
				else
				{
					nYCoord = 0; X++; Y = -1;
					//nXCoord = X * nWholeW;
				}

				for(INT_PTR j = 0; !gbCancelAll && j < jCount && nItem < nItemCount; j++, nItem++)
				{
					if (PVM == pvm_Thumbnails)
					{
						X++;
					}
					else
					{
						Y++;
					}

#ifdef DEBUG_PAINT
					StringCchPrintf(szDbg, countof(szDbg), L"  -- Step:%i, Item:%2i, Row:%i, Col:%i...", nStep, nItem, Y, X);
					DEBUGSTR(szDbg);
#endif
					_ASSERTE(this->ppItems!=NULL);
					_ASSERTE(this->pItemColors!=NULL);
					CePluginPanelItem* pItem = this->ppItems[nItem];
					CePluginPanelItemColor* pItemColor = this->pItemColors+nItem;

					if (!pItem || !pItemColor)
					{
						_ASSERTE(this->ppItems[nItem]!=NULL);
						continue; // ������?
					}

					// ����������� ��������� ��������� �� ������ ���� � �����
					const wchar_t* pszName = pItem->FindData.lpwszFileName;

					if (wcschr(pszName, L'\\'))
					{
						// ��� ��� ����� ���� ������ ���� (TempPanel?)
						pItem->pszFullName = pszName;
					}
					else
					{
						// ������ ���� ����� ������������
						lstrcpyn(pszNamePtr, pszName, MAX_PATH+1);
						pItem->pszFullName = pszFull;
					}

					if (nStep == 0 || (!pItemColor->bItemColorLoaded && nStep == 1))
					{
						// ������ ���������� ����� �� �������
						LoadItemColors(nItem, pItem, pItemColor, (Focus && nItem==nCurrentItem), (nStep == 0));
					}

					// ���������� � ������� ��������
					if (nStep == 0)
					{
						// �� ���� 0 - �������� ShellIcon, �� ���� 1 - Thumbnails
						// ����� ������� ���������� ������� ���������� ��������� ShellIcon
						// ��������� ������ � ���� 1
						//#ifdef _DEBUG
						//wchar_t szDbg[MAX_PATH+32];
						//lstrcpy(szDbg, L"Req: ");
						//#endif
						if (!pItem->PreviewLoaded)
						{
							//#ifdef _DEBUG
							//lstrcpy(szDbg, L"ReqS: "); lstrcpyn(szDbg+6, pItem->FindData.lpwszFileName, MAX_PATH);
							//DEBUGSTRPAINT(szDbg);
							//#endif
							gpImgCache->RequestItem(pItem, FALSE/*������ Shell*/);
						}

						if ((gThSet.bLoadPreviews & PVM) && !(pItem->PreviewLoaded & 2))
						{
							//#ifdef _DEBUG
							//lstrcpy(szDbg, L"ReqT: "); lstrcpyn(szDbg+6, pItem->FindData.lpwszFileName, MAX_PATH);
							//DEBUGSTRPAINT(szDbg);
							//#endif
							gpImgCache->RequestItem(pItem, TRUE/*Preview*/);
						}
					}

					if (nStep == 1)  // || (nStep == 2 && !pItem->bPreviewLoaded))
					{
						if (PaintItem(hCompDC, nItem, 0, 0, pItem, pItemColor,
						             (Focus && nItem==nCurrentItem), (nItem==nCurrentItem),
						             /*nBackColor, nForeColor, hBack, (nStep == 2),*/ hBackBrush, hPanelBrush, crPanel))
						{
							BitBlt(hdc, nXCoord, nYCoord, nWholeW, nWholeH, hCompDC, 0,0, SRCCOPY);
#ifdef DEBUG_PAINT
							GdiFlush();
							DEBUG_SLEEP(300);
#endif
						}
					}

					if (PVM == pvm_Thumbnails)
					{
						nXCoord += nWholeW;
					}
					else
					{
						nYCoord += nWholeH;
					}

#ifdef DEBUG_PAINT
					DEBUG_SLEEP(300);
					OutputDebugStringW(L"Done\n");
#endif
				} // for (int j = 0; !gbCancelAll && j < jCount && nItem < nItemCount; j++, nItem++)

				// ������� ��������� ������
				if (nStep == 1)
				{
					if (PVM == pvm_Thumbnails)
					{
						// �� ���� 1 - ������ ��������� ����� ������ ����
						if (nStep == 1 && nXCoord < rc.right)
						{
							RECT rcComp = {nXCoord,nYCoord,rc.right,nYCoord+nWholeH};
#ifdef DEBUG_PAINT
							StringCchPrintf(szDbg, countof(szDbg), L"  -- FillRect({%ix%i}-{%ix%i})...",rcComp.left,rcComp.top,rcComp.right,rcComp.bottom);
							DEBUGSTR(szDbg);
#endif
							FillRect(hdc, &rcComp, hPanelBrush); //hBack[0]);
#ifdef DEBUG_PAINT
							GdiFlush();
							DEBUG_SLEEP(300);
							OutputDebugStringW(L" Done\n");
#endif
						}

						//nYCoord += nWholeH;
					}
					else
					{
						// �� ���� 1 - ������ ��������� ����� ������ ����
						if (nStep == 1 && nYCoord < rc.bottom)
						{
							RECT rcComp = {nXCoord,nYCoord,rc.right,rc.bottom};
#ifdef DEBUG_PAINT
							StringCchPrintf(szDbg, countof(szDbg), L"  -- FillRect({%ix%i}-{%ix%i})...",rcComp.left,rcComp.top,rcComp.right,rcComp.bottom);
							DEBUGSTR(szDbg);
#endif
							FillRect(hdc, &rcComp, hPanelBrush); //hBack[0]);
#ifdef DEBUG_PAINT
							GdiFlush();
							DEBUG_SLEEP(300);
							OutputDebugStringW(L" Done\n");
#endif
						}

						//nXCoord += nWholeW;
					}
				}

				if (PVM == pvm_Thumbnails)
				{
					nYCoord += nWholeH;
				}
				else
				{
					nXCoord += nWholeW;
				}
			} // for (int i = 0; !gbCancelAll && i < iCount && nItem < nItemCount; i++)

			//// �� ���� 0 - ������ ��������� �����
			//if (nStep == 0)
			//{
			//	// ��������� �������� ����� ���� �������� � �������, �� ������ � PanelView.
			//	// ����������� �������� �� �� �����������, �� ���������
			//	_ASSERTE(nItem <= nItemCount);
			//	for (int i = nTopItem; i < nItem; i++) {
			//		CePluginPanelItem* pItem = this->ppItems[i];
			//		if (!pItem) {
			//			_ASSERTE(this->ppItems[i]!=NULL);
			//			continue; // ������?
			//		}
			//		if (pItem->bItemColorLoaded)
			//			continue; // � ���� ��������� ��� ��, �� ����� � �������

			//		bool bFound = false;
			//		DWORD nMasks[] = {
			//			0xFFFFFFFF,
			//			FILE_ATTRIBUTE_NORMAL|FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM,
			//			FILE_ATTRIBUTE_NORMAL|FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_HIDDEN,
			//			FILE_ATTRIBUTE_NORMAL|FILE_ATTRIBUTE_DIRECTORY,
			//		};
			//		for (int m = 0; !bFound && m < ARRAYSIZE(nMasks); m++)
			//		{
			//			DWORD nMask = nMasks[m];
			//			for (int j = nTopItem; !bFound && j < nItem; j++) {
			//				CePluginPanelItem* pCmp = this->ppItems[j];
			//				if (!pCmp) {
			//					_ASSERTE(this->ppItems[j]!=NULL);
			//					continue; // ������?
			//				}
			//				if (!pCmp->bItemColorLoaded)
			//					continue;
			//
			//				if (pCmp->Flags == pItem->Flags
			//					&& (nMask&pCmp->FindData.dwFileAttributes) == (nMask&pItem->FindData.dwFileAttributes)
			//					&& (!Focus || ((i==nCurrentItem) == (j==nCurrentItem)))
			//					)
			//				{
			//					pItem->crBack = pCmp->crBack;
			//					pItem->crFore = pCmp->crFore;
			//					bFound = true; // ����� ����������
			//					break;
			//				}
			//			}
			//		}
			//	}
			//}
			//else
			if (nStep == 1)  // �� ���� 1 - ������ ��������� ����� ������ ����
			{
				if (PVM == pvm_Thumbnails)
				{
					if (nYCoord < rc.bottom)
					{
						RECT rcComp = {0,(Y+1)*nWholeH,rc.right,rc.bottom};
#ifdef DEBUG_PAINT
						StringCchPrintf(szDbg, countof(szDbg), L"  -- FillRect({%ix%i}-{%ix%i})...",rcComp.left,rcComp.top,rcComp.right,rcComp.bottom);
						DEBUGSTR(szDbg);
#endif
						FillRect(hdc, &rcComp, hPanelBrush); //hBack[0]);
#ifdef DEBUG_PAINT
						GdiFlush();
						DEBUG_SLEEP(300);
						OutputDebugStringW(L" Done\n");
#endif
					}
				}
				else
				{
					if (nXCoord < rc.right)
					{
						RECT rcComp = {nXCoord,0,rc.right,rc.bottom};
						FillRect(hdc, &rcComp, hPanelBrush); //hBack[0]);
					}
				}
			} // if (nStep == 1) // �� ���� 1 - ������ ��������� ����� ������ ����

#ifdef DEBUG_PAINT
			GdiFlush();
			StringCchPrintf(szDbg, countof(szDbg), L"-- Step:%i finished\n");
			DEBUG_SLEEP(300);
#endif
		} // for (int nStep = 0; !gbCancelAll && nStep <= 2; nStep++)
	}

	free(pszFull);
	//SafeRelease(gpDesktopFolder);
	SelectObject(hCompDC, hOldPen);     DeleteObject(hPen);
	SelectObject(hCompDC, hOldBr);      DeleteObject(hBackBrush); DeleteObject(hPanelBrush);
	SelectObject(hCompDC, hOldFont);    DeleteObject(hFont);
	SelectObject(hCompDC, hOldCompBmp); DeleteObject(hCompBmp);

	if (hLastBackBrush) { DeleteObject(hLastBackBrush); hLastBackBrush = NULL; }

	DeleteDC(hCompDC);
	//for (i = 0; i < 4; i++) DeleteObject(hBack[i]);
//#ifdef _DEBUG
//	BitBlt(hdc, 0,0,rc.right,rc.bottom, gpImgCache->hField[0], 0,0, SRCCOPY);
//#endif
}


// ��� "����������" ������� ���������� �� �������� ����, ��� ����� ������� FAR Api
void CeFullPanelInfo::DisplayReloadPanel()
{
	_ASSERTE(GetCurrentThreadId()==gnMainThreadId);
	TODO("���������� �������� ����� �������� ������, � ���������� ������ ��");

	for(INT_PTR nItem = 0; nItem < this->ItemsNumber; nItem++)
	{
		// �������� ���������� �� �������� (���, ������������, � �.�.)
		LoadPanelItemInfo(this, nItem);
	}
}




// ��� "����������" ������� ���������� �� �������� ����, ��� ����� ������� FAR Api
int CeFullPanelInfo::RegisterPanelView()
{
	_ASSERTE(GetCurrentThreadId() == gnMainThreadId);

	// ��������� �� ����, ��� conemu.dll ����� ��������� (unload:...)
	if (!CheckConEmu() || !gfRegisterPanelView)
		return -1;

	//CeFullPanelInfo* pi = abLeft ? &pviLeft : &pviRight;
	PanelViewInit pvi = {sizeof(PanelViewInit)};
	pvi.bLeftPanel = this->bLeftPanel;
	pvi.bVisible = TRUE;
	pvi.nCoverFlags = PVI_COVER_NORMAL;
	pvi.tColumnTitle.bConAttr = this->nFarColors[col_PanelColumnTitle];
	pvi.tColumnTitle.nFlags = PVI_TEXT_CENTER|PVI_TEXT_SKIPSORTMODE;
	lstrcpyW(pvi.tColumnTitle.sText, (PVM==pvm_Thumbnails) ? gsTitleThumbs : gsTitleTiles);
	pvi.nFarInterfaceSettings = this->nFarInterfaceSettings;
	pvi.nFarPanelSettings = this->nFarPanelSettings;
	pvi.PanelRect = this->PanelRect;
	pvi.pfnPeekPreCall.f = OnPrePeekConsole;
	pvi.pfnPeekPostCall.f = OnPostPeekConsole;
	pvi.pfnReadPreCall.f = OnPreReadConsole;
	pvi.pfnReadPostCall.f = OnPostReadConsole;
	pvi.pfnWriteCall.f = OnPreWriteConsoleOutput;
	// ������������������ (��� �������� ���������)
	pvi.bRegister = TRUE;
	pvi.hWnd = this->hView;
	BOOL lbRc = FALSE;
	DWORD dwErr = 0;
	int nRc = gfRegisterPanelView(&pvi);
	gnCreateViewError = 0;

	// ���� ���� ��� ��
	if (nRc == 0)
	{
		LoadThSet();
		// ��������� ����������� ��� ������ ���� ���������!
		_ASSERTE(gThSet.crPalette[0]!=0 || gThSet.crPalette[1]!=0);
		//memset(&gThSet, 0, sizeof(gThSet));
		//if (!pvi.ThSet.cbSize || !pvi.ThSet.Thumbs.nImgSize || !pvi.ThSet.Tiles.nImgSize) {
		//	gnCreateViewError = CEInvalidSettingValues;
		//	nRc = -1000;
		//} else {
		//_ASSERTE(pvi.ThSet.cbSize == sizeof(gThSet));
		//memmove(&gThSet, &pvi.ThSet, min(pvi.ThSet.cbSize,sizeof(gThSet)));
		//// ����� "�������"
		//for (int i=0; i<16; i++) {
		//	gcrActiveColors[i] = pvi.crPalette[i];
		//	gcrFadeColors[i] = pvi.crFadePalette[i];
		//}
		this->WorkRect = pvi.WorkRect;
		// �� �������? �� ���� ������, ��� ��������� �������-��
		gbFadeColors = (pvi.bFadeColors!=FALSE);
		//gcrCurColors = gbFadeColors ? gcrFadeColors : gcrActiveColors;
		gcrCurColors = gbFadeColors ? gThSet.crFadePalette : gThSet.crPalette;
		TODO("����� WINDOW_LONG_FADE �� ������������ �����?");
		SetWindowLong(this->hView, WINDOW_LONG_FADE, gbFadeColors ? 2 : 1);
		// ����������� �������� ��������
		_ASSERTE(gpRgnDetect!=NULL);
		SMALL_RECT rcFarRect; GetFarRect(&rcFarRect);
		gpRgnDetect->SetFarRect(&rcFarRect);

		if (gpRgnDetect->InitializeSBI(gcrCurColors))
		{
			gpRgnDetect->PrepareTransparent(&gFarInfo, gcrCurColors);
		}

		// ����� ����������� ��������� ���������������� ������ � ����
		if (!OnSettingsChanged(FALSE))
			nRc = -1000;

		//if (PVM == pvm_Thumbnails) {
		//	Spaces = gThSet.Thumbs;
		//	lstrcpy(sFontName, gThSet.Thumbs.sFontName);
		//	nFontHeight = gThSet.Thumbs.nFontHeight;
		//} else if (PVM == pvm_Tiles) {
		//	Spaces = gThSet.Tiles;
		//	lstrcpy(sFontName, gThSet.Tiles.sFontName);
		//	nFontHeight = gThSet.Tiles.nFontHeight;
		//} else {
		//	_ASSERTE(PVM==pvm_Thumbnails || PVM==pvm_Tiles);
		//	gnCreateViewError = CEUnknownPanelMode;
		//	nRc = -1000;
		//}
		//}
	}

	// ���� ����� ���������� - ����������
	if (nRc == 0)
	{
		gnRgnDetectFlags = gpRgnDetect->GetFlags();
		//gbLastCheckWindow = true;
		UpdateEnvVar(FALSE/*abForceRedraw*/);

		if (pvi.bVisible)
		{
			if (!IsWindowVisible(this->hView))
			{
				lbRc = apiShowWindow(this->hView, SW_SHOWNORMAL);

				if (!lbRc)
					dwErr = GetLastError();
			}

			_ASSERTE(lbRc || IsWindowVisible(this->hView));
			//Inva lidateRect(this->hView, NULL, FALSE);
			Invalidate();
		}

		//RedrawWindow(this->hView, NULL, NULL, RDW_INTERNALPAINT|RDW_UPDATENOW);
	}

	// ���� GUI ��������� �� ������ (��� ��� ����� ��������� ������) - ����� �� �������
	if (nRc != 0)
	{
		// ������� ���� (�� WM_CLOSE ���� ���� ������ ������� WM_QUIT ���� ��� ������������)
		_ASSERTE(nRc == 0);

		// ���� ��� ������ ������������ - ����� ���������� ����������
		if (!pviLeft.hView || !pviRight.hView)
		{
			ResetUngetBuffer();
			SetEnvironmentVariable(TH_ENVVAR_NAME, NULL);
		}

		PostMessage(this->hView, WM_CLOSE, 0, 0);

		if (gnCreateViewError == 0)  // ���� ������ ��� �� ���������� - �������� �� GUI
			gnCreateViewError = CEGuiDontAcceptPanel;

		gnWin32Error = nRc;

		if (GetCurrentThreadId() == gnMainThreadId)
		{
			ShowLastError();
		}
	}

	return nRc;
}

void CeFullPanelInfo::Close()
{
	// ������������������
	UnregisterPanelView();
	_ASSERTE(hView == NULL);
	hView = NULL;
	// ��������� ���������
	DWORD dwMode = pvm_None;
	HKEY hk = NULL;

	SettingsSave(bLeftPanel ? L"LeftPanelView" : L"RightPanelView", &dwMode);
	//if (!RegCreateKeyExW(HKEY_CURRENT_USER, gszRootKey, 0, NULL, 0, KEY_WRITE, NULL, &hk, NULL))
	//{
	//	RegSetValueEx(hk, bLeftPanel ? L"LeftPanelView" : L"RightPanelView", 0,
	//	              REG_DWORD, (LPBYTE)&dwMode, sizeof(dwMode));
	//	RegCloseKey(hk);
	//}
}

// ����� ���������� �� ����� ����
// --- // ��� "����������" ������� ���������� �� �������� ����, ��� ����� ������� FAR Api
int CeFullPanelInfo::UnregisterPanelView()
{
	// ��������� �� ����, ��� conemu.dll ����� ��������� (unload:...)
	if (!CheckConEmu() || !gfRegisterPanelView)
	{
		// ������� ���� (�� WM_CLOSE ���� ���� ������ ������� WM_QUIT ���� ��� ������������)
		if (this->hView)
			PostMessage(this->hView, WM_CLOSE, 0, 0);

		return -1;
	}

	// ���� ��� ������ ������������ - ����� ���������� ����������
	if (!pviLeft.hView || !pviRight.hView)
	{
		ResetUngetBuffer();
		SetEnvironmentVariable(TH_ENVVAR_NAME, NULL);
	}

	//CeFullPanelInfo* pi = abLeft ? &pviLeft : &pviRight;
	PanelViewInit pvi = {sizeof(PanelViewInit)};
	pvi.bLeftPanel = this->bLeftPanel;
	pvi.nFarInterfaceSettings = this->nFarInterfaceSettings;
	pvi.nFarPanelSettings = this->nFarPanelSettings;
	pvi.PanelRect = this->PanelRect;
	// ������������������
	pvi.bRegister = FALSE;
	pvi.hWnd = this->hView;
	int nRc = gfRegisterPanelView(&pvi);
	// ������� ���� (�� WM_CLOSE ���� ���� ������ ������� WM_QUIT ���� ��� ������������)
	PostMessage(this->hView, WM_CLOSE, 0, 0);
	return nRc;
}

BOOL CeFullPanelInfo::OnSettingsChanged(BOOL bInvalidate)
{
	BOOL lbRc = TRUE;

	// ����� ����������� ��������� ���������������� ������ � ����
	if (PVM == pvm_Thumbnails)
	{
		Spaces = gThSet.Thumbs;
		lstrcpy(sFontName, gThSet.Thumbs.sFontName);
		nFontHeight = gThSet.Thumbs.nFontHeight;
	}
	else if (PVM == pvm_Tiles)
	{
		Spaces = gThSet.Tiles;
		lstrcpy(sFontName, gThSet.Tiles.sFontName);
		nFontHeight = gThSet.Tiles.nFontHeight;
	}
	else
	{
		_ASSERTE(PVM==pvm_Thumbnails || PVM==pvm_Tiles);
		gnCreateViewError = CEUnknownPanelMode;
		//nRc = -1000;
		lbRc = FALSE;
	}

	if (bInvalidate && IsWindowVisible(hView))
	{
		//Inva lidateRect(hView, NULL, FALSE);
		Invalidate();
	}

	return lbRc;
}

void CeFullPanelInfo::RequestSetPos(INT_PTR anCurrentItem, INT_PTR anTopItem, BOOL abSetFocus /*= FALSE*/)
{
	if (!this)
		return;

	if (anCurrentItem != -1 && anTopItem != -1)
	{
		// ������� ���������� ������ � ����� ��������
		this->ReqCurrentItem = anCurrentItem; this->ReqTopPanelItem = anTopItem;
		this->bRequestItemSet = true;

		if (!gbSynchoRedrawPanelRequested)
		{
			//gbWaitForKeySequenceEnd = true;
			UpdateEnvVar(FALSE);
			gbSynchoRedrawPanelRequested = true;
			ExecuteInMainThread(SYNCHRO_REDRAW_PANEL);
		}
	}

	if (abSetFocus)
	{
		ConEmuThSynchroArg *pCmd = (ConEmuThSynchroArg*)LocalAlloc(LPTR, sizeof(ConEmuThSynchroArg)+128);
		pCmd->bValid = 1; pCmd->bExpired = 0; pCmd->nCommand = ConEmuThSynchroArg::eExecuteMacro;
		wsprintfW((wchar_t*)pCmd->Data, L"$If (%cPanel.Left) Tab $End",
		          this->bLeftPanel ? L'P' : L'A');
		ExecuteInMainThread(pCmd);
	}
}



void CeFullPanelInfo::FinalRelease()
{
	if (ppItems)
	{
		for(int i=0; i<ItemsNumber; i++)
			if (ppItems[i]) free(ppItems[i]);

		free(ppItems);
		ppItems = NULL;
	}

	if (pItemColors)
	{
		free(pItemColors);
		pItemColors = NULL;
	}

	nMaxItemsNumber = 0;

	if (pszPanelDir)
	{
		free(pszPanelDir);
		pszPanelDir = NULL;
	}

	nMaxPanelDir = 0;

	//if (nFarColors)
	//{
	//	free(nFarColors);
	//	nFarColors = NULL;
	//}
	//nMaxFarColors = 0;

	if (pFarTmpBuf)
	{
		free(pFarTmpBuf);
		pFarTmpBuf = NULL;
	}

	nFarTmpBuf = 0;

	if (hView)
	{
		BOOL bValid = IsWindow(hView);
		_ASSERTE(bValid==FALSE);
		hView = NULL;
	}

	if (pSection)
	{
		delete pSection; pSection = NULL;
	}

#ifdef CONEMU_LOG_FILES
	if (gpLogLoad)
	{
		MFileLog *p = gpLogLoad; gpLogLoad = NULL;
		delete p;
	}

	if (gpLogPaint)
	{
		MFileLog *p = gpLogPaint; gpLogPaint = NULL;
		delete p;
	}
#endif
}

BOOL CeFullPanelInfo::ReallocItems(INT_PTR anCount)
{
	CePluginPanelItem** ppNew = NULL;
	CePluginPanelItemColor* pNewColors = NULL;

	if ((ppItems == NULL) || (nMaxItemsNumber < anCount))
	{
		MSectionLock CS;

		if (!CS.Lock(pSection, TRUE, 5000))
			return FALSE;

		INT_PTR nNewMax = anCount+255; // + �������� ��� �����
		ppNew = (CePluginPanelItem**)calloc(nNewMax, sizeof(LPVOID));
		pNewColors = (CePluginPanelItemColor*)calloc(nNewMax, sizeof(CePluginPanelItemColor));

		if (!ppNew || !pNewColors)
		{
			_ASSERTE(ppNew!=NULL && pNewColors!=NULL);
			return FALSE;
		}

		if (ppItems)
		{
			if (nMaxItemsNumber)
			{
				memmove(ppNew, ppItems, nMaxItemsNumber*sizeof(LPVOID));
			}

			free(ppItems);
		}

		if (pItemColors)
		{
			if (nMaxItemsNumber)
			{
				memmove(pNewColors, pItemColors, nMaxItemsNumber*sizeof(CePluginPanelItemColor));
			}

			free(pItemColors);
		}

		nMaxItemsNumber = nNewMax;
		ppItems = ppNew;
		pItemColors = pNewColors;
	}

	if (pItemColors == NULL)
	{
		_ASSERTE(pItemColors!=NULL);
		pItemColors = (CePluginPanelItemColor*)calloc(nMaxItemsNumber, sizeof(CePluginPanelItemColor));
	}

	return TRUE;
}

BOOL CeFullPanelInfo::FarItem2CeItem(INT_PTR anIndex,
                                     const wchar_t*   asName,
                                     const wchar_t*   asDesc,
                                     DWORD            dwFileAttributes,
                                     FILETIME         ftLastWriteTime,
                                     unsigned __int64 anFileSize,
                                     BOOL             abVirtualItem,
                                     DWORD_PTR        apUserData,
                                     unsigned __int64 anFlags,
                                     DWORD            anNumberOfLinks)
{
	_ASSERTE(pItemColors && ppItems);
	// ����������� ������ ������ ��� �������� ��������
	size_t nSize = sizeof(CePluginPanelItem)
	               +(lstrlen(asName)+1)*2
	               +((asDesc ? lstrlen(asDesc) : 0)+1)*2;

	// ��� ����� ���� �������� ���������� ������ ��� ���� �������
	if ((ppItems[anIndex] == NULL) || (ppItems[anIndex]->cbSize < (DWORD_PTR)nSize))
	{
		if (ppItems[anIndex]) free(ppItems[anIndex]);

		nSize += 32;
		ppItems[anIndex] = (CePluginPanelItem*)calloc(nSize, 1);

		if (ppItems[anIndex] == NULL)
		{
			_ASSERTE(ppItems[anIndex] != NULL);
			return FALSE;
		}

		ppItems[anIndex]->cbSize = (int)nSize;
	}

	// ��������� �� ����� ��� ����� ����� (�� ���� ����� �� ����������, ������ ��������)
	wchar_t* psz = (wchar_t*)(ppItems[anIndex]+1);

	if (ppItems[anIndex]->bIsCurrent != (CurrentItem == anIndex) ||
	        ppItems[anIndex]->bVirtualItem != abVirtualItem ||
	        ppItems[anIndex]->UserData != apUserData ||
	        ppItems[anIndex]->Flags != anFlags ||
	        ppItems[anIndex]->NumberOfLinks != anNumberOfLinks ||
	        ppItems[anIndex]->FindData.dwFileAttributes != dwFileAttributes ||
	        ppItems[anIndex]->FindData.ftLastWriteTime.dwLowDateTime != ftLastWriteTime.dwLowDateTime ||
	        ppItems[anIndex]->FindData.ftLastWriteTime.dwHighDateTime != ftLastWriteTime.dwHighDateTime ||
	        ppItems[anIndex]->FindData.nFileSize != anFileSize ||
	        lstrcmp(psz, asName))
	{
		// ����� ��������, ����� ����� �� ���������, �� � ���� � �������� ����� �����������, ����� �� ��������...
		memset(((LPBYTE)ppItems[anIndex])+sizeof(ppItems[anIndex]->cbSize), 0, ppItems[anIndex]->cbSize-sizeof(ppItems[anIndex]->cbSize));
		// ����� ����� "���������������" �����
		pItemColors[anIndex].bItemColorLoaded = FALSE;
	}

	// ��������
	ppItems[anIndex]->bIsCurrent = (CurrentItem == anIndex);
	ppItems[anIndex]->bVirtualItem = abVirtualItem;
	ppItems[anIndex]->UserData = apUserData;
	ppItems[anIndex]->Flags = anFlags;
	ppItems[anIndex]->NumberOfLinks = anNumberOfLinks;
	ppItems[anIndex]->FindData.dwFileAttributes = dwFileAttributes;
	ppItems[anIndex]->FindData.ftLastWriteTime = ftLastWriteTime;
	ppItems[anIndex]->FindData.nFileSize = anFileSize;
	lstrcpy(psz, asName);
	ppItems[anIndex]->FindData.lpwszFileName = psz;
	ppItems[anIndex]->FindData.lpwszFileNamePart = wcsrchr(psz, L'\\');

	if (ppItems[anIndex]->FindData.lpwszFileNamePart == NULL)
		ppItems[anIndex]->FindData.lpwszFileNamePart = psz;

	ppItems[anIndex]->FindData.lpwszFileExt = wcsrchr(ppItems[anIndex]->FindData.lpwszFileNamePart, L'.');
	// Description
	psz += lstrlen(psz)+1; //-V102

	if (asDesc)
		lstrcpy(psz, asDesc);
	else
		psz[0] = 0;

	ppItems[anIndex]->pszDescription = psz;
	return TRUE;
}
