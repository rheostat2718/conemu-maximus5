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

#pragma once

enum FrameDrawStyle
{
	fdt_Win2k = 1,
	fdt_Win2kSelf = 2,
	fdt_Themed = 3,
	fdt_Aero = 4,
	fdt_Win8 = 5,
};

struct PaintDC
{
	bool   bInternal;
	HDC    hDC;
	HANDLE hBuffered;

	// Internal use!
	RECT   rcTarget;
	void*  hInternal1; // HDC hdcTarget
	void*  hInternal2; // HBITMAP hbmp
	void*  hInternal3; // HBITMAP hOldBmp
	void*  hInternal4; // void* pPixels
};


class CDwmHelper
{
protected:
	bool    mb_DwmAllowed, mb_ThemeAllowed, mb_BufferedAllowed;
	bool    mb_EnableGlass, mb_EnableTheming;
	int     mn_DwmClientRectTopOffset;
	FrameDrawStyle m_DrawType;
	virtual void OnUseGlass(bool abEnableGlass) = 0;
	virtual void OnUseTheming(bool abEnableTheming) = 0;
	virtual void OnUseDwm(bool abEnableDwm) = 0;
public:
	CDwmHelper(void);
	virtual ~CDwmHelper(void);
public:
	bool IsDwm();
	bool IsDwmAllowed();
	bool IsGlass();
	bool IsThemed();
	void EnableGlass(bool abGlass);
	void EnableTheming(bool abTheme);
	void EnableBlurBehind(bool abBlurBehindClient);
	bool ExtendWindowFrame();
	bool ExtendWindowFrame(HWND hWnd, const RECT& rcMargins);
	void CheckGlassAttribute();
	int GetDwmClientRectTopOffset();
	BOOL DwmDefWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *plResult);
	HRESULT DwmGetWindowAttribute(HWND hwnd, DWORD dwAttribute, PVOID pvAttribute, DWORD cbAttribute);
	HANDLE/*HTHEME*/ OpenThemeData(HWND hwnd, LPCWSTR pszClassList);
	HRESULT CloseThemeData(HANDLE/*HTHEME*/ hTheme);
	HANDLE/*HPAINTBUFFER*/ BeginBufferedPaint(HDC hdcTarget, const RECT& rcTarget, PaintDC& dc);
	HRESULT BufferedPaintSetAlpha(const PaintDC& dc, const RECT *prc, BYTE alpha);
	HRESULT EndBufferedPaint(PaintDC& dc, BOOL fUpdateTarget);
	HRESULT DrawThemeTextEx(HANDLE/*HTHEME*/ hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwFlags, LPRECT pRect, const void/*DTTOPTS*/ *pOptions);
	HRESULT DrawThemeBackground(HANDLE/*HTHEME*/ hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, const RECT *pClipRect);
	HRESULT DrawThemeEdge(HANDLE/*HTHEME*/ hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT pDestRect, UINT uEdge, UINT uFlags, LPRECT pContentRect);
	HRESULT GetThemeMargins(HANDLE/*HTHEME*/ hTheme, HDC hdc, int iPartId, int iStateId, int iPropId, LPRECT prc, RECT *pMargins);
	HRESULT GetThemePartSize(HANDLE/*HTHEME*/ hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT prc, int/*THEMESIZE*/ eSize, SIZE *psz);
	HRESULT GetThemePosition(HANDLE/*HTHEME*/ hTheme, int iPartId, int iStateId, int iPropId, POINT *pPoint);
	int GetThemeSysSize(HANDLE/*HTHEME*/ hTheme, int iSizeID);
	HRESULT GetThemeBackgroundContentRect(HANDLE/*HTHEME*/ hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT pBoundingRect, LPRECT pContentRect);
	// Determine and store current draw type
	FrameDrawStyle DrawType();
	// Aero support
	void ForceSetIconic(HWND hWnd);
	HRESULT DwmSetIconicThumbnail(HWND hwnd, HBITMAP hbmp);
	HRESULT DwmSetIconicLivePreviewBitmap(HWND hwnd, HBITMAP hbmp, POINT *pptClient);
	HRESULT DwmInvalidateIconicBitmaps(HWND hwnd);
private:
	//OSVERSIONINFO m_OSVer;
	// Vista+
	HMODULE mh_User32;
	typedef BOOL (WINAPI* ChangeWindowMessageFilter_t)(UINT message, DWORD dwFlag);
	ChangeWindowMessageFilter_t _ChangeWindowMessageFilter;
	// Vista+ Aero
	HMODULE mh_DwmApi;
	typedef HRESULT (WINAPI* DwmIsCompositionEnabled_t)(BOOL *pfEnabled);
	DwmIsCompositionEnabled_t _DwmIsCompositionEnabled;
	typedef HRESULT (WINAPI* DwmSetWindowAttribute_t)(HWND hwnd, DWORD dwAttribute, LPCVOID pvAttribute, DWORD cbAttribute);
	DwmSetWindowAttribute_t _DwmSetWindowAttribute;
	typedef HRESULT (WINAPI* DwmExtendFrameIntoClientArea_t)(HWND hWnd, void* pMarInset);
	DwmExtendFrameIntoClientArea_t _DwmExtendFrameIntoClientArea;
	typedef BOOL (WINAPI* DwmDefWindowProc_t)(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *plResult);
	DwmDefWindowProc_t _DwmDefWindowProc;
	typedef HRESULT (WINAPI* DwmGetWindowAttribute_t)(HWND hwnd, DWORD dwAttribute, PVOID pvAttribute, DWORD cbAttribute);
	DwmGetWindowAttribute_t _DwmGetWindowAttribute;
	typedef HRESULT (WINAPI* DwmSetIconicThumbnail_t)(HWND hwnd, HBITMAP hbmp, DWORD dwSITFlags);
	DwmSetIconicThumbnail_t _DwmSetIconicThumbnail;
	typedef HRESULT (WINAPI* DwmSetIconicLivePreviewBitmap_t)(HWND hwnd, HBITMAP hbmp, POINT *pptClient, DWORD dwSITFlags);
	DwmSetIconicLivePreviewBitmap_t _DwmSetIconicLivePreviewBitmap;
	typedef HRESULT (WINAPI* DwmInvalidateIconicBitmaps_t)(HWND hwnd);
	DwmInvalidateIconicBitmaps_t _DwmInvalidateIconicBitmaps;
	typedef HRESULT (WINAPI* DwmEnableBlurBehindWindow_t)(HWND hWnd, const void/*DWM_BLURBEHIND*/ *pBlurBehind);
	DwmEnableBlurBehindWindow_t _DwmEnableBlurBehindWindow;
	// XP+ Theming
	HMODULE mh_UxTheme;
	typedef BOOL (WINAPI* AppThemed_t)();
	AppThemed_t _IsAppThemed; // XP
	AppThemed_t _IsThemeActive; // XP
	typedef HANDLE/*HTHEME*/ (WINAPI* OpenThemeData_t)(HWND hwnd, LPCWSTR pszClassList);
	OpenThemeData_t _OpenThemeData; // XP
	typedef HRESULT (WINAPI* CloseThemeData_t)(HANDLE/*HTHEME*/ hTheme);
	CloseThemeData_t _CloseThemeData; // XP
	typedef HRESULT (WINAPI* BufferedPaintInit_t)();
	BufferedPaintInit_t _BufferedPaintInit; // Vista
	BufferedPaintInit_t _BufferedPaintUnInit; // Vista
	typedef HANDLE/*HPAINTBUFFER*/ (WINAPI* BeginBufferedPaint_t)(HDC hdcTarget, const RECT *prcTarget, int/*BP_BUFFERFORMAT*/ dwFormat, void/*BP_PAINTPARAMS*/ *pPaintParams, HDC *phdc);
	BeginBufferedPaint_t _BeginBufferedPaint; // Vista
	typedef HRESULT (WINAPI* BufferedPaintSetAlpha_t)(HANDLE/*HPAINTBUFFER*/ hBufferedPaint, const RECT *prc, BYTE alpha);
	BufferedPaintSetAlpha_t _BufferedPaintSetAlpha; // Vista
	typedef HRESULT (WINAPI* EndBufferedPaint_t)(HANDLE/*HPAINTBUFFER*/ hBufferedPaint, BOOL fUpdateTarget);
	EndBufferedPaint_t _EndBufferedPaint; // Vista
	typedef HRESULT (WINAPI* DrawThemeTextEx_t)(HANDLE/*HTHEME*/ hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwFlags, LPRECT pRect, const void/*DTTOPTS*/ *pOptions);
	DrawThemeTextEx_t _DrawThemeTextEx; // Vista
	typedef HRESULT (WINAPI* DrawThemeBackground_t)(HANDLE/*HTHEME*/ hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, const RECT *pClipRect);
	DrawThemeBackground_t _DrawThemeBackground; // XP
	typedef HRESULT (WINAPI* DrawThemeEdge_t)(HANDLE/*HTHEME*/ hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT pDestRect, UINT uEdge, UINT uFlags, LPRECT pContentRect);
	DrawThemeEdge_t _DrawThemeEdge; // XP
	typedef HRESULT (WINAPI* GetThemeMargins_t)(HANDLE/*HTHEME*/ hTheme, HDC hdc, int iPartId, int iStateId, int iPropId, LPRECT prc, RECT *pMargins);
	GetThemeMargins_t _GetThemeMargins; // XP
	typedef HRESULT (WINAPI* GetThemePartSize_t)(HANDLE/*HTHEME*/ hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT prc, int/*THEMESIZE*/ eSize, SIZE *psz);
	GetThemePartSize_t _GetThemePartSize; // XP
	typedef HRESULT (WINAPI* GetThemePosition_t)(HANDLE/*HTHEME*/ hTheme, int iPartId, int iStateId, int iPropId, POINT *pPoint);
	GetThemePosition_t _GetThemePosition; // XP
	typedef int (WINAPI* GetThemeSysSize_t)(HANDLE/*HTHEME*/ hTheme, int iSizeID);
	GetThemeSysSize_t _GetThemeSysSize; // XP
	typedef HRESULT (WINAPI* GetThemeBackgroundContentRect_t)(HANDLE/*HTHEME*/ hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT pBoundingRect, LPRECT pContentRect);
	GetThemeBackgroundContentRect_t _GetThemeBackgroundContentRect; // XP
	typedef void (WINAPI* SetThemeAppProperties_t)(DWORD dwFlags);
	SetThemeAppProperties_t _SetThemeAppProperties; // XP
	typedef void (WINAPI* SetWindowThemeNonClientAttributes_t)(DWORD dwFlags);
	SetWindowThemeNonClientAttributes_t _SetWindowThemeNonClientAttributes; // Vista

	// some other private functions
	void InitDwm();
};
