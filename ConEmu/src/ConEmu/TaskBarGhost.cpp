
/*
Copyright (c) 2011 Maximus5
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
#include "header.h"
#include "TaskBarGhost.h"
#include "ConEmu.h"
#include "VirtualConsole.h"
#include "RealConsole.h"
#include "../common/ConEmuCheck.h"

//#ifdef __GNUC__
#include "DwmApi_Part.h"
//#endif

#define UPDATE_DELTA 1000

ATOM CTaskBarGhost::mh_Class = NULL;

CTaskBarGhost::CTaskBarGhost(CVirtualConsole* apVCon)
{
	mp_VCon = apVCon;
	
	mh_Ghost = NULL;
	m_TabSize.cx = m_TabSize.cy = 0;
	mh_Snap = NULL;
	mn_LastUpdate = 0;

	mb_SimpleBlack = TRUE;

	mn_MsgUpdateThumbnail = RegisterWindowMessage(L"ConEmu::TaskBarGhost");

	mh_SkipActivateEvent = NULL;
	mb_WasSkipActivate = false;
	ms_LastTitle[0] = 0;
}

CTaskBarGhost::~CTaskBarGhost()
{
	if (mh_Snap)
	{
		DeleteObject(mh_Snap);
		mh_Snap = NULL;
	}
	if (mh_Ghost && IsWindow(mh_Ghost))
	{
		//_ASSERTE(mh_Ghost==NULL);
		DestroyWindow(mh_Ghost);
	}

	if (mh_SkipActivateEvent)
	{
		CloseHandle(mh_SkipActivateEvent);
		mh_SkipActivateEvent = NULL;
	}
}

CTaskBarGhost* CTaskBarGhost::Create(CVirtualConsole* apVCon)
{
	_ASSERTE(gpConEmu->isMainThread());

	if (mh_Class == 0)
	{
		WNDCLASSEX wcex = {0};
		wcex.cbSize         = sizeof(wcex);
		wcex.lpfnWndProc    = GhostStatic;
		wcex.hInstance      = g_hInstance;
		wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
		wcex.lpszClassName  = VirtualConsoleClassGhost;

		mh_Class = ::RegisterClassEx(&wcex);
	}

	CTaskBarGhost* pGhost = new CTaskBarGhost(apVCon);

	pGhost->UpdateTabSnapshoot(TRUE);

#if 0
	if (!pGhost->CreateTabSnapshoot(...))
	{
		delete pGhost;
		return NULL;
	}
#endif

	DWORD dwStyle, dwStyleEx;
	if (IsWindows7)
	{
		dwStyleEx = WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE | WS_EX_ACCEPTFILES;
		dwStyle = WS_POPUP | WS_BORDER | WS_SYSMENU | WS_CAPTION;
	}
	else
	{
		dwStyleEx = WS_EX_NOACTIVATE | WS_EX_APPWINDOW | WS_EX_ACCEPTFILES;
		dwStyle = WS_OVERLAPPED | WS_BORDER | WS_SYSMENU | WS_CAPTION;
		if ((gpConEmu->IsVConValid(apVCon) > 0) /*&& (gpConEmu->GetVCon(1) == NULL)*/)
			dwStyle |= WS_VISIBLE;
	}

	pGhost->mh_Ghost = ::CreateWindowEx(dwStyleEx,
			VirtualConsoleClassGhost, pGhost->CheckTitle(),
			dwStyle,
            -32000, -32000, 10, 10,
            ghWnd, NULL, g_hInstance, (LPVOID)pGhost);

	if (!pGhost->mh_Ghost)
	{
		delete pGhost;
		return NULL;
	}

	return pGhost;
}

HWND CTaskBarGhost::GhostWnd()
{
	if (!this)
	{
		_ASSERTE(this!=NULL);
		return NULL;
	}
	return mh_Ghost;
}

void CTaskBarGhost::UpdateGhostSize()
{
	HWND hView = mp_VCon->GetView();
	RECT rcClient = {}; GetClientRect(hView, &rcClient);
	m_TabSize.cx = rcClient.right;
	m_TabSize.cy = rcClient.bottom;

	if (mh_Ghost)
	{
		int nShowWidth = 0, nShowHeight = 0;
		if (CalcThumbnailSize(200, 150, nShowWidth, nShowHeight))
		{
			RECT rcGhost = {}; GetWindowRect(mh_Ghost, &rcGhost);
			if ((rcGhost.right - rcGhost.left) != nShowWidth || (rcGhost.bottom - rcGhost.top) != nShowHeight
				|| rcGhost.left != -32000 || rcGhost.top != -32000)
			{
				SetWindowPos(mh_Ghost, NULL, -32000, -32000, nShowWidth, nShowHeight, SWP_NOZORDER|SWP_NOACTIVATE);
			}
		}
	}
}

BOOL CTaskBarGhost::UpdateTabSnapshoot(BOOL abSimpleBlack, BOOL abNoSnapshoot)
{
	if (!gpConEmu->isMainThread())
	{
		PostMessage(mh_Ghost, mn_MsgUpdateThumbnail, abSimpleBlack, abNoSnapshoot);
		return FALSE;
	}

	mb_SimpleBlack = abSimpleBlack;

	CheckTitle();

	UpdateGhostSize();

	if (!abNoSnapshoot && NeedSnapshootCache())
	{
		if (gpConEmu->isVisible(mp_VCon))
			CreateTabSnapshoot();
	}

	gpConEmu->DwmInvalidateIconicBitmaps(mh_Ghost);
	return TRUE;
}

BOOL CTaskBarGhost::CreateTabSnapshoot()
{
	CheckTitle();

	if (!gpConEmu->Taskbar_GhostSnapshootRequired())
		return FALSE; // ����� ������.

	if (!gpConEmu->isValid(mp_VCon))
		return FALSE;

	if (mh_Snap)
	{
		bool lbChanged = false;
		if ((m_TabSize.cx != (int)mp_VCon->Width || m_TabSize.cy != (int)mp_VCon->Height))
		{
			lbChanged = true;
		}
		else
		{
			BITMAP bi = {};
			GetObject(mh_Snap, sizeof(bi), &bi);
			if ((bi.bmWidth != (int)mp_VCon->Width) || (bi.bmHeight != (int)mp_VCon->Height))
				lbChanged = true;
		}

		if (lbChanged)
		{
			DeleteObject(mh_Snap); mh_Snap = NULL;
		}
		//abForce = TRUE;
	}
	
	//if (!abForce)
	//{
	//	DWORD dwDelta = GetTickCount() - mn_LastUpdate;
	//	if (dwDelta < UPDATE_DELTA)
	//		return FALSE;
	//}

	m_TabSize.cx = mp_VCon->Width;
	m_TabSize.cy = mp_VCon->Height;

    HDC hdcMem = CreateCompatibleDC(NULL);
    if (hdcMem != NULL)
    {
    	if (mh_Snap == NULL)
    	{
	        ZeroMemory(&mbmi_Snap.bmiHeader, sizeof(BITMAPINFOHEADER));
	        mbmi_Snap.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	        mbmi_Snap.bmiHeader.biWidth = m_TabSize.cx;
	        mbmi_Snap.bmiHeader.biHeight = -m_TabSize.cy;  // Use a top-down DIB
	        mbmi_Snap.bmiHeader.biPlanes = 1;
	        mbmi_Snap.bmiHeader.biBitCount = 32;

	        mh_Snap = CreateDIBSection(hdcMem, &mbmi_Snap, DIB_RGB_COLORS, (VOID**)&mpb_DS, NULL, NULL);
        }
        
        if (mh_Snap != NULL)
        {
			HBITMAP hOld = (HBITMAP)SelectObject(hdcMem, mh_Snap);

			//BitBlt(hdcMem, 0,0,m_TabSize.cx,m_TabSize.cy, hdcSrc, 0,0, SRCCOPY);
			if (NeedSnapshootCache())
			{
				HWND hView = mp_VCon->GetView();
				RECT rcSnap = {}; GetWindowRect(hView, &rcSnap);

				bool bLockSnapshoot = false;
				if (gpConEmu->IsDwm())
				{
					// ���� ��� ������ "DWM-���" ������ � ��������...
					HWND hFind = FindWindowEx(NULL, NULL, L"TaskListOverlayWnd", NULL);
					if (hFind)
					{
						RECT rcDwm = {}; GetWindowRect(hFind, &rcDwm);
						RECT rcInter = {};
						if (IntersectRect(&rcInter, &rcSnap, &rcDwm))
							bLockSnapshoot = true;
					}
				}

				if (!bLockSnapshoot)
				{
					// ����� ���� ������
					#if 1
					// ���� ����� ����� �����������, ��
					// - ���� ������� ������������� � ������ ����������� DWM-������
					//   �� �� (�������) �������� � ���� ��������� ���� DWM ��� ������
					HDC hdcScrn = GetDC(NULL);
					BitBlt(hdcMem, 0,0, m_TabSize.cx,m_TabSize.cy, hdcScrn, rcSnap.left,rcSnap.top, SRCCOPY);
					ReleaseDC(NULL, hdcScrn);
					#else
					HDC hdcScrn;
					// -- ���� ����� �� ����������� ���������� ������ ���� (PicView, GUI-apps, � �.�.)
					hdcScrn = GetDC(hView);
					//hdcScrn = GetDCEx(hView, NULL, 0);
					BitBlt(hdcMem, 0,0, m_TabSize.cx,m_TabSize.cy, hdcScrn, 0,0, SRCCOPY);
					ReleaseDC(hView, hdcScrn);
					#endif
				}
			}
			else
			{
				// ������ �������� "�������" �� ����� DC
				RECT rcPaint = {0,0,m_TabSize.cx,m_TabSize.cy};
				mp_VCon->Paint(hdcMem, rcPaint);
			}

			SelectObject(hdcMem, hOld);
        }

        if (hdcMem) DeleteDC(hdcMem);
    }

    mn_LastUpdate = GetTickCount();
	return (mh_Snap != NULL);
}

bool CTaskBarGhost::CalcThumbnailSize(int nWidth, int nHeight, int &nShowWidth, int &nShowHeight)
{
	if (nWidth && nHeight && m_TabSize.cx && m_TabSize.cy)
	{
		if ((nWidth / (double)nHeight) > (m_TabSize.cx / (double)m_TabSize.cy))
		{
			nShowHeight = nHeight;
			nShowWidth = nHeight * m_TabSize.cx / m_TabSize.cy;
		}
		else
		{
			nShowWidth = nWidth;
			nShowHeight = nWidth * m_TabSize.cy / m_TabSize.cx;
		}
		return true;
	}
	return false;
}

// ���� � "�������" ���� ����������� ���� (GUI Application, PicView, MMView, PanelViews, � �.�.)
// �� ������ snapshoot �� ������ ������� ����������� - ����� "���������������" ���������� ����
bool CTaskBarGhost::NeedSnapshootCache()
{
	if (!gpConEmu->IsDwm())
	{
		return false;
	}

	HWND hView = mp_VCon->GetView();
	HWND hChild = FindWindowEx(hView, NULL, NULL, NULL);
	DWORD nCurPID = GetCurrentProcessId();
	DWORD nPID, nStyle;
	while (hChild)
	{
		if (GetWindowThreadProcessId(hChild, &nPID) && nPID != nCurPID)
		{
			nStyle = GetWindowLong(hChild, GWL_STYLE);
			if (nStyle & WS_VISIBLE)
				return true;
		}
		hChild = FindWindowEx(hView, hChild, NULL, NULL);
	}
	return false;
}

HBITMAP CTaskBarGhost::CreateThumbnail(int nWidth, int nHeight)
{
    HBITMAP hbm = NULL;
    HDC hdcMem = CreateCompatibleDC(NULL);
    if (hdcMem != NULL)
    {
		int nShowWidth = nWidth, nShowHeight = nHeight;
		int nX = 0, nY = 0;

		if (CalcThumbnailSize(nWidth, nHeight, nShowWidth, nShowHeight))
		{
			#if 0
			nX = (nWidth - nShowWidth) / 2;
			nY = (nHeight - nShowHeight) / 2;
			#else
			nWidth = nShowWidth;
			nHeight = nShowHeight;
			#endif
		}

        BITMAPINFO bmi;
        ZeroMemory(&bmi.bmiHeader, sizeof(BITMAPINFOHEADER));
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = nWidth;
        bmi.bmiHeader.biHeight = -nHeight;  // Use a top-down DIB
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;

        PBYTE pbDS = NULL;
        hbm = CreateDIBSection(hdcMem, &bmi, DIB_RGB_COLORS, (VOID**)&pbDS, NULL, NULL);
        if (hbm != NULL)
        {
			HBITMAP hOldMem = (HBITMAP)SelectObject(hdcMem, hbm);

			if (nWidth && nHeight && m_TabSize.cx && m_TabSize.cy)
			{
				memset(pbDS, 0, nWidth*nHeight*4);

				if (!mb_SimpleBlack)
				{
					if (NeedSnapshootCache())
					{
						// ��� ������� �������� GUI ���� - ��������, snapshoot-� ����� ����� ������ ����� ���� ������
						if (gpConEmu->isVisible(mp_VCon))
							CreateTabSnapshoot();

						if (mh_Snap)
						{
							HDC hdcSrc = CreateCompatibleDC(NULL);
							BITMAP bi = {}; GetObject(mh_Snap, sizeof(bi), &bi);
							HBITMAP hOldSrc = (HBITMAP)SelectObject(hdcSrc, mh_Snap);
							SetStretchBltMode(hdcMem, HALFTONE);
							StretchBlt(hdcMem, nX,nY,nShowWidth,nShowHeight, hdcSrc, 0,0,bi.bmWidth,bi.bmHeight, SRCCOPY);
							SelectObject(hdcSrc, hOldSrc);
							DeleteDC(hdcSrc);
						}
					}
					else
					{
						if (!gpConEmu->isActive(mp_VCon))
							mp_VCon->Update(true);
						mp_VCon->StretchPaint(hdcMem, nX, nY, nShowWidth, nShowHeight);
					}
				}

				#if 0
				if (mh_Snap)
				{
					HDC hdcSheet = CreateCompatibleDC(NULL);
					HBITMAP hOld = (HBITMAP)SelectObject(hdcSheet, mh_Snap);
					SetStretchBltMode(hdcMem, HALFTONE);
					StretchBlt(hdcMem, nX,nY,nShowWidth,nShowHeight, hdcSheet, 0,0,m_TabSize.cx, m_TabSize.cy, SRCCOPY);
					SelectObject(hdcSheet, hOld);
					DeleteDC(hdcSheet);
				}
				#endif

				// Apply Alpha Channel
				PBYTE pbRow = NULL;
				if (nWidth > nShowWidth)
				{
					for (int y = 0; y < nShowHeight; y++)
					{
						pbRow = pbDS + (y*nWidth+nX)*4;
						for (int x = 0; x < nShowWidth; x++)
						{
							pbRow[3] = 255;
							pbRow += 4;
						}
					}
				}
				else
				{
					for (int y = 0; y < nShowHeight; y++)
					{
						pbRow = pbDS + (y+nY)*4*nWidth;
						for (int x = 0; x < nWidth; x++)
						{
							pbRow[3] = 255;
							pbRow += 4;
						}
					}
				}
			}
			else
			{
				RECT rc = {0,0,nShowWidth,nShowHeight};
				FillRect(hdcMem, &rc, (HBRUSH)GetSysColorBrush(COLOR_WINDOW));
			}

			SelectObject(hdcMem, hOldMem);
        }

        DeleteDC(hdcMem);
    }

	return hbm;
}

LPCWSTR CTaskBarGhost::CheckTitle(BOOL abSkipValidation /*= FALSE*/)
{
	LPCWSTR pszTitle = NULL;
	TODO("�������� �� ����� �������");
	if (mp_VCon && (abSkipValidation || gpConEmu->isValid(mp_VCon)) && mp_VCon->RCon())
		pszTitle = mp_VCon->RCon()->GetTitle();
	if (!pszTitle)
	{
		pszTitle = gpConEmu->GetDefaultTitle();
		_ASSERTE(pszTitle!=NULL);
	}
	
	if (mh_Ghost)
	{
		if (wcsncmp(ms_LastTitle, pszTitle, countof(ms_LastTitle)) != 0)
		{
			lstrcpyn(ms_LastTitle, pszTitle, countof(ms_LastTitle));
			SetWindowText(mh_Ghost, pszTitle);
		}
	}

#ifdef _DEBUG
	wchar_t szInfo[1024];
	getWindowInfo(mh_Ghost, szInfo);
#endif
	
	return pszTitle;
}

void CTaskBarGhost::ActivateTaskbar()
{
	gpConEmu->Taskbar_SetActiveTab(mh_Ghost);
#if 0
	// -- ����� �������� (owner) �� WinXP �� �����������
	if (!IsWindows7)
	{
		HWND hParent = GetParent(ghWnd);
		if (hParent != mh_Ghost)
			gpConEmu->SetParent(mh_Ghost);
	}
#endif
}
LRESULT CALLBACK CTaskBarGhost::GhostStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = 0;

    CTaskBarGhost *pWnd = (CTaskBarGhost*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
    if (pWnd == NULL && message == WM_NCCREATE)
    {
        LPCREATESTRUCTW lpcs = (LPCREATESTRUCTW)lParam;
        pWnd = (CTaskBarGhost*)lpcs->lpCreateParams;
        pWnd->mh_Ghost = hWnd;
        ::SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)pWnd);
        lResult = ::DefWindowProcW(hWnd, message, wParam, lParam);
    }
    else if (pWnd != NULL)
    {
        lResult = pWnd->GhostProc(message, wParam, lParam);
    }
    else
    {
        lResult = ::DefWindowProcW(hWnd, message, wParam, lParam);
    }

    return lResult;
}

LRESULT CTaskBarGhost::OnCreate()
{
	SetTimer(mh_Ghost, 101, 2500, NULL);

	wchar_t szEvtName[64];
	_wsprintf(szEvtName, SKIPLEN(countof(szEvtName)) CEGHOSTSKIPACTIVATE, (DWORD)mh_Ghost);
	SafeCloseHandle(mh_SkipActivateEvent);
	mh_SkipActivateEvent = CreateEvent(NULL, FALSE, FALSE, szEvtName);

	UpdateGhostSize();

	// Set DWM window attributes to indicate we'll provide the iconic bitmap, and
	// to always render the thumbnail using the iconic bitmap.
	gpConEmu->ForceSetIconic(mh_Ghost);

	// Win7 � ����!
	if (IsWindows7)
	{
		// Tell the taskbar about this tab window
		gpConEmu->Taskbar_RegisterTab(mh_Ghost, gpConEmu->isActive(mp_VCon));
	}

	#if 0
	// -- ����� �������� (owner) �� WinXP �� �����������, � ��� ������ ������ (Vista+) ������ �� ���������
	else if (gpConEmu->isActive(mp_VCon))
	{
		HWND hParent = GetParent(ghWnd);
		if (hParent != mh_Ghost)
			gpConEmu->SetParent(mh_Ghost);
	}
	#endif

	gpConEmu->OnGhostCreated(mp_VCon, mh_Ghost);

	OnDwmSendIconicThumbnail(200,150);

	return 0;
}

LRESULT CTaskBarGhost::OnTimer(WPARAM TimerID)
{
	if (!gpConEmu->isValid(mp_VCon))
	{
		DestroyWindow(mh_Ghost);
	}
	else
	{
		if (gpConEmu->isVisible(mp_VCon) && NeedSnapshootCache())
		{
			UpdateTabSnapshoot(FALSE, FALSE);
		}
	}
	return 0;
}

LRESULT CTaskBarGhost::OnActivate(WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = 0;
	HRESULT hr = S_FALSE;
	// The taskbar will activate this window, so pass along the activation
	// to the tab window outer frame.
	if (LOWORD(wParam) == WA_ACTIVE)
	{
		mb_WasSkipActivate = false;
		if (!IsWindows7 && mh_SkipActivateEvent)
		{
			// ����� �� ���� ������ ��� Alt-Tab, Alt-Tab
			DWORD nSkipActivate = WaitForSingleObject(mh_SkipActivateEvent, 0);
			if (nSkipActivate == WAIT_OBJECT_0)
			{
				mb_WasSkipActivate = true;
				return 0;
			}
		}

		if (gpConEmu->isIconic())
			SendMessage(ghWnd, WM_SYSCOMMAND, SC_RESTORE, 0);

		apiSetForegroundWindow(ghWnd);

		// Update taskbar
		hr = gpConEmu->Taskbar_SetActiveTab(mh_Ghost);
		//hr = gpConEmu->DwmInvalidateIconicBitmaps(mh_Ghost); -- need?

		// Activate tab.
		// ����� ��� ����� �� ��������� ���� �� ������������� (gpSet->isMultiIterate)
		// ������� ��������� �������� ���
		if (gpConEmu->ActiveCon() != mp_VCon)
			gpConEmu->Activate(mp_VCon);

		// Forward message
		SendMessage(ghWnd, WM_ACTIVATE, wParam, lParam);
	}
	return lResult;
}

LRESULT CTaskBarGhost::OnSysCommand(WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = 0;
	// All syscommands except for close will be passed along to the tab window
	// outer frame. This allows functions such as move/size to occur properly.
	if (wParam != SC_CLOSE)
	{
		if (wParam == SC_RESTORE || wParam == SC_MINIMIZE || wParam == SC_MAXIMIZE)
		{
			if (gpConEmu->isIconic())
				wParam = (wParam == SC_MAXIMIZE) ? SC_MAXIMIZE : SC_MINIMIZE;
			else if (gpConEmu->isZoomed() || gpConEmu->isFullScreen())
				wParam = (wParam == SC_RESTORE) ? SC_RESTORE : SC_MINIMIZE;
			else
				wParam = (wParam == SC_MAXIMIZE) ? SC_MAXIMIZE : SC_MINIMIZE;
		}
		SendMessage(ghWnd, WM_SYSCOMMAND, wParam, lParam);
	}
	else
	{
		lResult = ::DefWindowProc(mh_Ghost, WM_SYSCOMMAND, wParam, lParam);
	}
	return lResult;
}

HICON CTaskBarGhost::OnGetIcon(WPARAM anIconType)
{
	HICON lResult = NULL;

	TODO("�������� ������ ��������� ���������� � �������");
	//lResult = SendMessage(ghWnd, message, wParam, lParam);
	if (anIconType == ICON_BIG)
		lResult = hClassIcon;
	else
		lResult = hClassIconSm;

	return lResult;
}

LRESULT CTaskBarGhost::OnClose()
{
	// The taskbar (or system menu) is asking this tab window to close. Ask the
	// tab window outer frame to destroy this tab.

	//SetWindowLongPtr(_hwnd, GWLP_USERDATA, 0);
	//DestroyWindow(mh_Ghost);

	mp_VCon->RCon()->CloseConsole(false, true);

	return 0;
}

LRESULT CTaskBarGhost::OnDwmSendIconicThumbnail(short anWidth, short anHeight)
{
	// This tab window is being asked to provide its iconic bitmap. This indicates
	// a thumbnail is being drawn.

	HRESULT hr = S_FALSE;
	HBITMAP hThumb = CreateThumbnail(anWidth, anHeight);
	if (hThumb)
	{
#ifdef _DEBUG
		BITMAP bi = {};
		GetObject(hThumb, sizeof(bi), &bi);
#endif
		hr = gpConEmu->DwmSetIconicThumbnail(mh_Ghost, hThumb);
		DeleteObject(hThumb);
	}

	return 0;
}

LRESULT CTaskBarGhost::OnDwmSendIconicLivePreviewBitmap()
{
	HRESULT hr = S_FALSE;

	// This tab window is being asked to provide a bitmap to show in live preview.
	// This indicates the tab's thumbnail in the taskbar is being previewed.
	//_SendLivePreviewBitmap();
	if (CreateTabSnapshoot())
	{
		POINT ptOffset = {0,0};
		HWND hView = mp_VCon->GetView();
		if (hView)
		{
			RECT rcMain = {0}; GetWindowRect(ghWnd, &rcMain);
			RECT rcView = {0}; GetWindowRect(hView, &rcView);
			TODO("���������, ����������� �� DWM � ������ ����� � ������� �����");
			ptOffset.x = rcView.left - rcMain.left;
			ptOffset.y = rcView.top - rcMain.top;
			if (!gpConEmu->isIconic())
			{
				ptOffset.x -= GetSystemMetrics(SM_CXFRAME);
				ptOffset.y -= GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFRAME);
			}
		}

		BITMAP bi = {};
		GetObject(mh_Snap, sizeof(bi), &bi);

		PBYTE pbRow = NULL;
		int nHeight = (bi.bmHeight < 0) ? -bi.bmHeight : bi.bmHeight;
		for (int y = 0; y < nHeight; y++)
		{
			pbRow = mpb_DS + (y*bi.bmWidth)*4;
			for (int x = 0; x < bi.bmWidth; x++)
			{
				pbRow[3] = 255;
				pbRow += 4;
			}
		}

#if 0
		//defined(_DEBUG)
		HDC hdc = GetDC(NULL);
		HDC hdcComp = CreateCompatibleDC(hdc);
		HBITMAP hOld = (HBITMAP)SelectObject(hdcComp, mh_Snap);
		BitBlt(hdc, 0,0,200,150, hdcComp, 0,0, SRCCOPY);
		SelectObject(hdcComp, hOld);
		DeleteDC(hdcComp);
		ReleaseDC(NULL, hdc);
#endif

		hr = gpConEmu->DwmSetIconicLivePreviewBitmap(mh_Ghost, mh_Snap, &ptOffset);
	}

	return 0;
}

LRESULT CTaskBarGhost::OnDestroy()
{
	KillTimer(mh_Ghost, 101);

	#if 0
	HWND hParent = GetParent(ghWnd);
	if (hParent == mh_Ghost)
	{
		gpConEmu->SetParent(NULL);
	}
	#endif

	if (IsWindows7)
	{
		gpConEmu->Taskbar_UnregisterTab(mh_Ghost);
	}

	return 0;
}

LRESULT CTaskBarGhost::GhostProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = 0;

#ifdef _DEBUG
	// ��� ���������� ������ � DebugLog
	if (message == WM_SETTEXT)
	{
		bool lbDbg = false;
	}
	else if (message == WM_WINDOWPOSCHANGING)
	{
		bool lbDbg = false;
	}
	else if (message != 0xAE/*WM_NCUAHDRAWCAPTION*/ && message != WM_GETTEXT && message != WM_GETMINMAXINFO
		&& message != WM_GETICON && message != WM_TIMER)
	{
		//wchar_t szDbg[127]; _wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"GhostProc(%i{x%03X},%i,%i)\n", message, message, (DWORD)wParam, (DWORD)lParam);
		//OutputDebugStringW(szDbg);
	}
#endif

    switch (message)
    {
        case WM_CREATE:
			lResult = OnCreate();
            break;

		case WM_TIMER:
			lResult = OnTimer(wParam);
			break;

        case WM_ACTIVATE:
			lResult = OnActivate(wParam, lParam);
            break;

        case WM_SYSCOMMAND:
			lResult = OnSysCommand(wParam, lParam);
            break;

		case WM_GETICON:
			lResult = (LRESULT)OnGetIcon(wParam);
			break;

        case WM_CLOSE:
			lResult = OnClose();
            break;

        case WM_DWMSENDICONICTHUMBNAIL:
			lResult = OnDwmSendIconicThumbnail((short)HIWORD(lParam), (short)LOWORD(lParam));
            break;

        case WM_DWMSENDICONICLIVEPREVIEWBITMAP:
			lResult = OnDwmSendIconicLivePreviewBitmap();
            break;

		case WM_DESTROY:
			lResult = OnDestroy();
			break;

		//case WM_SYSKEYUP:
		case WM_NCACTIVATE:
			//if (wParam == VK_MENU)
			if (wParam)
			{
				if (mb_WasSkipActivate)
				{
					ResetEvent(mh_SkipActivateEvent);
					mb_WasSkipActivate = false;
					OnActivate(WA_ACTIVE, NULL);
				}
			}
			lResult = ::DefWindowProcW(mh_Ghost, message, wParam, lParam);
			break;

        default:
			if (message == mn_MsgUpdateThumbnail)
			{
				lResult = UpdateTabSnapshoot(wParam!=0, lParam!=0);
			}
			else
			{
				lResult = ::DefWindowProcW(mh_Ghost, message, wParam, lParam);
			}
            break;
    }

    return lResult;
}