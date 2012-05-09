
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
#include "DwmApi_Part.h"
//#include "TabBarClass.h"
//#include "FrameHolder.h"
#include "ConEmu.h"
#include "Options.h"





/* *************************** */

CDwmHelper::CDwmHelper(void)
{
	mb_EnableGlass = false;
	mb_EnableTheming = false;
	mn_DwmClientRectTopOffset = 0;
	InitDwm();
	DrawType();
}

CDwmHelper::~CDwmHelper(void)
{
	if (mh_DwmApi)
	{
		FreeLibrary(mh_DwmApi);
		mh_DwmApi = NULL;

		mb_DwmAllowed = false;
		_DwmIsCompositionEnabled = NULL;
		_DwmSetWindowAttribute = NULL;
		_DwmGetWindowAttribute = NULL;
		_DwmExtendFrameIntoClientArea = NULL;
	}
	if (mh_UxTheme)
	{
		if (mb_BufferedAllowed && _BufferedPaintUnInit)
		{
			_BufferedPaintUnInit();
		}

		FreeLibrary(mh_UxTheme);
		mh_UxTheme = NULL;
		
		_IsAppThemed = NULL;
		_IsThemeActive = NULL;
	    _OpenThemeData = NULL;
	    _CloseThemeData = NULL;
		_BufferedPaintInit = NULL;
		_BufferedPaintUnInit = NULL;
		_BeginBufferedPaint = NULL;
		_EndBufferedPaint = NULL;
		_DrawThemeTextEx = NULL;
		_DrawThemeBackground = NULL;
		_DrawThemeEdge = NULL;
		_GetThemeMargins = NULL;
		_GetThemePartSize = NULL;
		_GetThemePosition = NULL;
		_GetThemeSysSize = NULL;
		_GetThemeBackgroundContentRect = NULL;
		_SetThemeAppProperties = NULL;
	}
}

FrameDrawStyle CDwmHelper::DrawType()
{
	if (IsGlass())
	{
		m_DrawType = fdt_Aero;
	}
	else if (IsThemed())
	{
		m_DrawType = fdt_Themed;
	}
	else
	{
		m_DrawType = fdt_Win2k;
	}
	
	return m_DrawType;
}

void CDwmHelper::InitDwm()
{
	BOOL lbDbg;
	
	mh_User32 = GetModuleHandle(L"User32.dll");
	_ChangeWindowMessageFilter = NULL;	
	mb_DwmAllowed = false;
	mh_DwmApi = NULL;
	_DwmIsCompositionEnabled = NULL;
	_DwmSetWindowAttribute = NULL;
	_DwmGetWindowAttribute = NULL;
	_DwmExtendFrameIntoClientArea = NULL;
	_DwmDefWindowProc = NULL;
	_DwmSetIconicThumbnail = NULL;
	_DwmSetIconicLivePreviewBitmap = NULL;
	_DwmInvalidateIconicBitmaps = NULL;
	mb_ThemeAllowed = false;
	mb_BufferedAllowed = false;
	mh_UxTheme = NULL;
	_IsAppThemed = NULL;
	_IsThemeActive = NULL;
    _OpenThemeData = NULL;
    _CloseThemeData = NULL;
	_BufferedPaintInit = NULL;
	_BufferedPaintUnInit = NULL;
	_BeginBufferedPaint = NULL;
	_EndBufferedPaint = NULL;
	_DrawThemeTextEx = NULL;
	_DrawThemeBackground = NULL;
	_DrawThemeEdge = NULL;
	_GetThemeMargins = NULL;
	_GetThemePartSize = NULL;
	_GetThemePosition = NULL;
	_GetThemeSysSize = NULL;
	_GetThemeBackgroundContentRect = NULL;
	_SetThemeAppProperties = NULL;

	//gOSVer.dwOSVersionInfoSize = sizeof(gOSVer);
	//GetVersionEx(&gOSVer);
	if (gOSVer.dwMajorVersion >= 6)
	{
		_ChangeWindowMessageFilter = (ChangeWindowMessageFilter_t)GetProcAddress(mh_User32, "ChangeWindowMessageFilter");
		if (_ChangeWindowMessageFilter)
		{
			lbDbg = _ChangeWindowMessageFilter(WM_DWMSENDICONICTHUMBNAIL, MSGFLT_ADD);
			lbDbg = _ChangeWindowMessageFilter(WM_DWMSENDICONICLIVEPREVIEWBITMAP, MSGFLT_ADD);
		}
		
		mh_DwmApi = LoadLibrary(_T("dwmapi.dll"));
		if (mh_DwmApi)
		{
			// Vista+
			_DwmIsCompositionEnabled = (DwmIsCompositionEnabled_t)GetProcAddress(mh_DwmApi, "DwmIsCompositionEnabled");
			_DwmSetWindowAttribute = (DwmSetWindowAttribute_t)GetProcAddress(mh_DwmApi, "DwmSetWindowAttribute");
			_DwmGetWindowAttribute = (DwmGetWindowAttribute_t)GetProcAddress(mh_DwmApi, "DwmGetWindowAttribute");
			_DwmExtendFrameIntoClientArea = (DwmExtendFrameIntoClientArea_t)GetProcAddress(mh_DwmApi, "DwmExtendFrameIntoClientArea");
			_DwmDefWindowProc = (DwmDefWindowProc_t)GetProcAddress(mh_DwmApi, "DwmDefWindowProc");
			_DwmSetIconicThumbnail = (DwmSetIconicThumbnail_t)GetProcAddress(mh_DwmApi, "DwmSetIconicThumbnail");
			_DwmSetIconicLivePreviewBitmap = (DwmSetIconicLivePreviewBitmap_t)GetProcAddress(mh_DwmApi, "DwmSetIconicLivePreviewBitmap");
			_DwmInvalidateIconicBitmaps = (DwmInvalidateIconicBitmaps_t)GetProcAddress(mh_DwmApi, "DwmInvalidateIconicBitmaps");

			mb_DwmAllowed = (_DwmIsCompositionEnabled != NULL)
				&& (_DwmGetWindowAttribute != NULL) && (_DwmSetWindowAttribute != NULL)
				&& (_DwmExtendFrameIntoClientArea != NULL)
				&& (_DwmDefWindowProc != NULL);
			if (mb_DwmAllowed)
				mb_EnableGlass = true;
		}
	}
	if (gOSVer.dwMajorVersion >= 6 || (gOSVer.dwMajorVersion == 5 && gOSVer.dwMinorVersion >= 1))
	{
		mh_UxTheme = LoadLibrary(_T("UxTheme.dll"));
		if (mh_UxTheme)
		{
			// XP+
	    	_IsAppThemed = (AppThemed_t)GetProcAddress(mh_UxTheme, "IsAppThemed");
    		_IsThemeActive = (AppThemed_t)GetProcAddress(mh_UxTheme, "IsThemeActive");
            _OpenThemeData = (OpenThemeData_t)GetProcAddress(mh_UxTheme, "OpenThemeData");
            _CloseThemeData = (CloseThemeData_t)GetProcAddress(mh_UxTheme, "CloseThemeData");
			_DrawThemeBackground = (DrawThemeBackground_t)GetProcAddress(mh_UxTheme, "DrawThemeBackground");
			_DrawThemeEdge = (DrawThemeEdge_t)GetProcAddress(mh_UxTheme, "DrawThemeEdge");
			_GetThemeMargins = (GetThemeMargins_t)GetProcAddress(mh_UxTheme, "GetThemeMargins");
			_GetThemePartSize = (GetThemePartSize_t)GetProcAddress(mh_UxTheme, "GetThemePartSize");
			_GetThemePosition = (GetThemePosition_t)GetProcAddress(mh_UxTheme, "GetThemePosition");
			_GetThemeSysSize = (GetThemeSysSize_t)GetProcAddress(mh_UxTheme, "GetThemeSysSize");
			_GetThemeBackgroundContentRect = (GetThemeBackgroundContentRect_t)GetProcAddress(mh_UxTheme, "GetThemeBackgroundContentRect");
			_SetThemeAppProperties = (SetThemeAppProperties_t)GetProcAddress(mh_UxTheme, "SetThemeAppProperties");
            // Vista+
			_BufferedPaintInit = (BufferedPaintInit_t)GetProcAddress(mh_UxTheme, "BufferedPaintInit");
			_BufferedPaintUnInit = (BufferedPaintInit_t)GetProcAddress(mh_UxTheme, "BufferedPaintUnInit");
			_BeginBufferedPaint = (BeginBufferedPaint_t)GetProcAddress(mh_UxTheme, "BeginBufferedPaint");
			_EndBufferedPaint = (EndBufferedPaint_t)GetProcAddress(mh_UxTheme, "EndBufferedPaint");
			_DrawThemeTextEx = (DrawThemeTextEx_t)GetProcAddress(mh_UxTheme, "DrawThemeTextEx");
    		
    		mb_ThemeAllowed = (_IsAppThemed != NULL) && (_IsThemeActive != NULL);
    		if (mb_ThemeAllowed)
    		{
    			mb_EnableTheming = true;
    			if (_BufferedPaintInit && _BufferedPaintUnInit)
    			{
    				HRESULT hr = _BufferedPaintInit();
    				mb_BufferedAllowed = SUCCEEDED(hr);
    			}
			}
		}
	}
}

bool CDwmHelper::IsDwm()
{
	if (!mb_DwmAllowed)
		return false;
	BOOL composition_enabled = FALSE;
	return _DwmIsCompositionEnabled(&composition_enabled) == S_OK &&
		composition_enabled;
}

bool CDwmHelper::IsDwmAllowed()
{
	return mb_DwmAllowed;
}

bool CDwmHelper::IsGlass()
{
	if (!mb_DwmAllowed || !mb_EnableGlass)
		return false;
	return IsDwm();
}

bool CDwmHelper::IsThemed()
{
	if (!mb_ThemeAllowed) // || !mb_EnableTheming)
		return false;
	return _IsAppThemed() && _IsThemeActive();
}

void CDwmHelper::EnableGlass(bool abGlass)
{
	mb_EnableGlass = abGlass && mb_DwmAllowed;
	OnUseGlass(mb_EnableGlass);
	CheckGlassAttribute();
	if (mb_DwmAllowed && !abGlass)
	{
		//TODO: ��� ���������� Aero � ���� �������� "�����" (���-�� �� �����-������)
	}
}

void CDwmHelper::EnableTheming(bool abTheme)
{
	mb_EnableTheming = abTheme && mb_ThemeAllowed;
	OnUseTheming(mb_EnableTheming);
	CheckGlassAttribute();
}

void CDwmHelper::CheckGlassAttribute()
{
	if (mb_DwmAllowed)
	{
		DWMNCRENDERINGPOLICY policy = mb_EnableGlass ? DWMNCRP_ENABLED : DWMNCRP_DISABLED;
		_DwmSetWindowAttribute(ghWnd, DWMWA_NCRENDERING_POLICY,
			&policy, sizeof(DWMNCRENDERINGPOLICY));

	    OnUseDwm(IsDwm());

		ExtendWindowFrame();
	}
	
	if (mb_ThemeAllowed)
	{
		//TODO: �������� ��� ��������� ���� XP+
		//_SetThemeAppProperties(mb_EnableTheming ? STAP_VALIDBITS : 0);
		//if (_SetWindowThemeNonClientAttributes)
		//	_SetWindowThemeNonClientAttributes(ghWnd, STAP_VALIDBITS, mb_EnableTheming ? STAP_VALIDBITS : 0);
		//static bool bRgnWasSet = false;
		//if (mb_EnableTheming)
		//{
		//	if (bRgnWasSet)
		//	{
		//		bRgnWasSet = false;
		//		SetWindowRgn(ghWnd, NULL, TRUE);
		//	}
		//}
		//else
		//{
		//	if (!bRgnWasSet)
		//	{
		//		RECT wr; GetWindowRect(ghWnd, &wr); OffsetRect(&wr, -wr.left, -wr.top);
		//		HRGN rgn = CreateRectRgnIndirect(&wr);
		//		SetWindowRgn(ghWnd, rgn, TRUE);
		//	}
		//}
	}
			
	SetWindowPos(ghWnd, NULL, 0, 0, 0, 0,
			SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
	RedrawWindow(ghWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}

void CDwmHelper::ExtendWindowFrame()
{
	if (!mb_DwmAllowed)
		return;

	MARGINS dwmm = {0};
	//dwmm.cxLeftWidth = 2;
	//dwmm.cxRightWidth = 2;
	dwmm.cyTopHeight = GetDwmClientRectTopOffset();
	//dwmm.cyBottomHeight = 0;
	_DwmExtendFrameIntoClientArea(ghWnd, &dwmm);

	//SetWindowThemeNonClientAttributes(hWnd, WTNCA_VALIDBITS, WTNCA_NODRAWCAPTION);
}

BOOL CDwmHelper::DwmDefWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *plResult)
{
	if (!mb_DwmAllowed)
		return FALSE;
	return _DwmDefWindowProc(hwnd, msg, wParam, lParam, plResult);
}

HRESULT CDwmHelper::DwmGetWindowAttribute(HWND hwnd, DWORD dwAttribute, PVOID pvAttribute, DWORD cbAttribute)
{
	if (!_DwmGetWindowAttribute)
		return E_NOINTERFACE;
	return _DwmGetWindowAttribute(hwnd, dwAttribute, pvAttribute, cbAttribute);
}

int CDwmHelper::GetDwmClientRectTopOffset()
{
	// GetFrameHeight(), GetCaptionDragHeight(), GetTabsHeight()
	if (gpSet->isTabs && gpSet->isTabsInCaption)
	{
		//mn_DwmClientRectTopOffset = 
		//	(GetSystemMetrics(SM_CYCAPTION)+(IsGlass() ? 8 : 0)
		//	+(IsZoomed(ghWnd)?(GetSystemMetrics(SM_CYFRAME)-1):(GetSystemMetrics(SM_CYCAPTION)/2)));
		mn_DwmClientRectTopOffset = 0
			//+ (IsGlass() ? 8 : 0)
			+ gpConEmu->GetFrameHeight() //+ 2
			+ gpConEmu->GetCaptionDragHeight()
			+ gpConEmu->GetTabsHeight();
	}
	else
	{
		mn_DwmClientRectTopOffset = 0;
			//GetSystemMetrics(SM_CYCAPTION)+(IsGlass() ? 8 : 0)
			//+(GetSystemMetrics(SM_CYFRAME)-1);
	}
	return mn_DwmClientRectTopOffset;
}

HANDLE/*HTHEME*/ CDwmHelper::OpenThemeData(HWND hwnd, LPCWSTR pszClassList)
{
	if (!_OpenThemeData)
		return NULL;
	return _OpenThemeData(hwnd, pszClassList);
}

HRESULT CDwmHelper::CloseThemeData(HANDLE/*HTHEME*/ hTheme)
{
	if (!_CloseThemeData)
		return E_NOINTERFACE;
	return _CloseThemeData(hTheme);
}

HANDLE/*HPAINTBUFFER*/ CDwmHelper::BeginBufferedPaint(HDC hdcTarget, const RECT *prcTarget, int/*BP_BUFFERFORMAT*/ dwFormat, void/*BP_PAINTPARAMS*/ *pPaintParams, HDC *phdc)
{
	if (!_BeginBufferedPaint)
		return NULL;
	return _BeginBufferedPaint(hdcTarget, prcTarget, dwFormat, (BP_PAINTPARAMS*)pPaintParams, phdc);
}

HRESULT CDwmHelper::EndBufferedPaint(HANDLE/*HPAINTBUFFER*/ hBufferedPaint, BOOL fUpdateTarget)
{
	if (!_EndBufferedPaint)
		return E_NOINTERFACE;
	return _EndBufferedPaint(hBufferedPaint, fUpdateTarget);
}

HRESULT CDwmHelper::DrawThemeTextEx(HANDLE/*HTHEME*/ hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwFlags, LPRECT pRect, const void/*DTTOPTS*/ *pOptions)
{
	if (!_DrawThemeTextEx)
		return E_NOINTERFACE;
	return _DrawThemeTextEx(hTheme, hdc, iPartId, iStateId, pszText, iCharCount, dwFlags, pRect, (const DTTOPTS*)pOptions);
}

HRESULT CDwmHelper::DrawThemeBackground(HANDLE/*HTHEME*/ hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, const RECT *pClipRect)
{
	if (!_DrawThemeBackground)
		return E_NOINTERFACE;
	return _DrawThemeBackground(hTheme, hdc, iPartId, iStateId, pRect, pClipRect);
}

HRESULT CDwmHelper::DrawThemeEdge(HANDLE/*HTHEME*/ hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT pDestRect, UINT uEdge, UINT uFlags, LPRECT pContentRect)
{
	if (!_DrawThemeEdge)
		return E_NOINTERFACE;
	return _DrawThemeEdge(hTheme, hdc, iPartId, iStateId, pDestRect, uEdge, uFlags, pContentRect);
}

HRESULT CDwmHelper::GetThemeMargins(HANDLE/*HTHEME*/ hTheme, HDC hdc, int iPartId, int iStateId, int iPropId, LPRECT prc, RECT *pMargins)
{
	if (!_GetThemeMargins)
		return E_NOINTERFACE;
	return _GetThemeMargins(hTheme, hdc, iPartId, iStateId, iPropId, prc, pMargins);
}

HRESULT CDwmHelper::GetThemePartSize(HANDLE/*HTHEME*/ hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT prc, int/*THEMESIZE*/ eSize, SIZE *psz)
{
	if (!_GetThemePartSize)
		return E_NOINTERFACE;
	return _GetThemePartSize(hTheme, hdc, iPartId, iStateId, prc, eSize, psz);
}

HRESULT CDwmHelper::GetThemePosition(HANDLE/*HTHEME*/ hTheme, int iPartId, int iStateId, int iPropId, POINT *pPoint)
{
	if (!_GetThemePosition)
		return E_NOINTERFACE;
	return _GetThemePosition(hTheme, iPartId, iStateId, iPropId, pPoint);
}

int CDwmHelper::GetThemeSysSize(HANDLE/*HTHEME*/ hTheme, int iSizeID)
{
	if (!_GetThemeSysSize)
		return -1;
	return _GetThemeSysSize(hTheme, iSizeID);
}

HRESULT CDwmHelper::GetThemeBackgroundContentRect(HANDLE/*HTHEME*/ hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT pBoundingRect, LPRECT pContentRect)
{
	if (!_GetThemeBackgroundContentRect)
		return E_NOINTERFACE;
	return _GetThemeBackgroundContentRect(hTheme, hdc, iPartId, iStateId, pBoundingRect, pContentRect);
}

void CDwmHelper::ForceSetIconic(HWND hWnd)
{
    BOOL bValue;
    
    if (_DwmSetWindowAttribute)
    {
    	bValue = TRUE;
		_DwmSetWindowAttribute(hWnd, DWMWA_FORCE_ICONIC_REPRESENTATION, &bValue, sizeof(bValue));
	}

	if (_DwmSetWindowAttribute)
	{
		bValue = TRUE;
		_DwmSetWindowAttribute(hWnd, DWMWA_HAS_ICONIC_BITMAP, &bValue, sizeof(bValue));
	}
}

HRESULT CDwmHelper::DwmSetIconicThumbnail(HWND hwnd, HBITMAP hbmp)
{
	HRESULT hr = E_NOINTERFACE;
	if (_DwmSetIconicThumbnail)
		hr = _DwmSetIconicThumbnail(hwnd, hbmp, 0);
	return hr;
}

HRESULT CDwmHelper::DwmSetIconicLivePreviewBitmap(HWND hwnd, HBITMAP hbmp, POINT *pptClient)
{
	HRESULT hr = E_NOINTERFACE;
	if (_DwmSetIconicLivePreviewBitmap)
		hr = _DwmSetIconicLivePreviewBitmap(hwnd, hbmp, pptClient, 0);
	return hr;
}

HRESULT CDwmHelper::DwmInvalidateIconicBitmaps(HWND hwnd)
{
	HRESULT hr = E_NOINTERFACE;
	if (_DwmInvalidateIconicBitmaps)
		hr = _DwmInvalidateIconicBitmaps(hwnd);
	return hr;
}
