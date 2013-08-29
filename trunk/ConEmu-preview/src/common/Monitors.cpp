
/*
Copyright (c) 2013 Maximus5
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
#include <windows.h>
#include "common.hpp"
#include "MAssert.h"
#include "MArray.h"
#include "Monitors.h"


BOOL CALLBACK FindMonitorsWorkspace(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	LPRECT lprc = (LPRECT)dwData;

	if (lprcMonitor->left < lprc->left)
		lprc->left = lprcMonitor->left;

	if (lprcMonitor->top < lprc->top)
		lprc->top = lprcMonitor->top;

	if (lprcMonitor->right > lprc->right)
		lprc->right = lprcMonitor->right;

	if (lprcMonitor->bottom > lprc->bottom)
		lprc->bottom = lprcMonitor->bottom;

	return TRUE;
}


RECT GetAllMonitorsWorkspace()
{
	RECT rcAllMonRect = {};

	if (!EnumDisplayMonitors(NULL, NULL, FindMonitorsWorkspace, (LPARAM)&rcAllMonRect) || IsRectEmpty(&rcAllMonRect))
	{
		#ifdef _DEBUG
		DWORD nErr = GetLastError();
        _ASSERTE(FALSE && "EnumDisplayMonitors fails");
		#endif

		rcAllMonRect.left = rcAllMonRect.right = 0;
		rcAllMonRect.right = GetSystemMetrics(SM_CXFULLSCREEN);
		rcAllMonRect.bottom = GetSystemMetrics(SM_CYFULLSCREEN);
	}

	return rcAllMonRect;
}

struct _FindPrimaryMonitor
{
	HMONITOR hMon;
	MONITORINFO mi;
};

BOOL CALLBACK FindPrimaryMonitor(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	MONITORINFO mi = {sizeof(mi)};

	if (GetMonitorInfo(hMonitor, &mi) && (mi.dwFlags & MONITORINFOF_PRIMARY))
	{
		_FindPrimaryMonitor* pMon = (_FindPrimaryMonitor*)dwData;
		pMon->hMon = hMonitor;
		pMon->mi = mi;
		// And stop enumeration
		return FALSE;
	}

	return TRUE;
}

bool GetMonitorInfoSafe(HMONITOR hMon, MONITORINFO& mi)
{
	if (!hMon)
	{
		return false;
	}

	mi.cbSize = sizeof(mi);
	BOOL bMonitor = GetMonitorInfo(hMon, &mi);
	if (!bMonitor)
	{
		GetPrimaryMonitorInfo(&mi);
	}
	
	return (bMonitor!=FALSE);
}

HMONITOR GetPrimaryMonitorInfo(MONITORINFO* pmi /*= NULL*/)
{
	_FindPrimaryMonitor m = {NULL};

	EnumDisplayMonitors(NULL, NULL, FindPrimaryMonitor, (LPARAM)&m);

	if (!m.hMon)
	{
		_ASSERTE(FALSE && "FindPrimaryMonitor fails");
		// ���� ����� � ���������� - ����� ������ �� ���������
		m.mi.cbSize = 0;
		SystemParametersInfo(SPI_GETWORKAREA, 0, &m.mi.rcWork, 0);
		m.mi.rcMonitor.left = m.mi.rcMonitor.top = 0;
		m.mi.rcMonitor.right = GetSystemMetrics(SM_CXFULLSCREEN);
		m.mi.rcMonitor.bottom = GetSystemMetrics(SM_CYFULLSCREEN);
		m.mi.dwFlags = 0;
	}

	if (pmi) *pmi = m.mi;
	return m.hMon;
}

HMONITOR GetNearestMonitorInfo(MONITORINFO* pmi /*= NULL*/, HMONITOR hDefault /*= NULL*/, LPCRECT prcWnd /*= NULL*/, HWND hWnd /*= NULL*/)
{
	HMONITOR hMon = NULL;
	MONITORINFO mi = {0};

	if (hDefault)
	{
		mi.cbSize = sizeof(mi);
		if (GetMonitorInfo(hDefault, &mi))
		{
			hMon = hDefault;
		}
		else
		{
			_ASSERTE(FALSE && "GetMonitorInfo(hDefault) failed");
			mi.cbSize = 0;
		}
	}

	if (!hMon)
	{
		if (prcWnd)
		{
			hMon = MonitorFromRect(prcWnd, MONITOR_DEFAULTTONEAREST);
		}
		else if (hWnd)
		{
			hMon = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
		}

		if (hMon)
		{
			mi.cbSize = sizeof(mi);
			if (!GetMonitorInfo(hMon, &mi))
			{
				_ASSERTE(FALSE && "GetMonitorInfo(hDefault) failed");
				mi.cbSize = 0;
			}
		}
	}
	
	if (!hMon)
	{
		_ASSERTE(FALSE && "Nor RECT neither HWND was succeeded, defaulting to PRIMARY");
		hMon = GetPrimaryMonitorInfo(&mi);
	}

	if (pmi) *pmi = mi;
	return hMon;
}

struct _FindAllMonitorsItem
{
	HMONITOR hMon;
	RECT rcMon;
	POINT ptCenter;
};

struct _FindAllMonitors
{
	MArray<_FindAllMonitorsItem> MonArray;
};

BOOL CALLBACK EnumAllMonitors(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	_FindAllMonitors* p = (_FindAllMonitors*)dwData;
	_FindAllMonitorsItem Info = {hMonitor, *lprcMonitor};
	Info.ptCenter.x = (Info.rcMon.left + Info.rcMon.right) >> 1;
	Info.ptCenter.y = (Info.rcMon.top + Info.rcMon.bottom) >> 1;
	p->MonArray.push_back(Info);
	return TRUE;
}

int MonitorSortCallback(_FindAllMonitorsItem &e1, _FindAllMonitorsItem &e2)
{
	if (e1.ptCenter.x < e2.ptCenter.x)
		return -1;
	else if (e1.ptCenter.x > e2.ptCenter.x)
		return 1;
	else if (e1.ptCenter.y < e2.ptCenter.y)
		return -1;
	else if (e1.ptCenter.y > e2.ptCenter.y)
		return 1;
	return 0;
}

HMONITOR GetNextMonitorInfo(MONITORINFO* pmi, LPCRECT prcWnd, bool Next)
{
	_FindAllMonitors Monitors;
	
	EnumDisplayMonitors(NULL, NULL, EnumAllMonitors, (LPARAM)&Monitors);

	INT_PTR iMonCount = Monitors.MonArray.size();
	if (iMonCount < 2)
		return NULL;

	HMONITOR hFound = NULL;

	_ASSERTE(prcWnd!=NULL); // ����� ����� ������ �� Primary

	HMONITOR hNearest = prcWnd ? MonitorFromRect(prcWnd, MONITOR_DEFAULTTONEAREST) : GetPrimaryMonitorInfo(NULL);
	MONITORINFO mi; GetMonitorInfoSafe(hNearest, mi);

	TODO("���������� �������������� ���������?");
	Monitors.MonArray.sort(MonitorSortCallback);

	for (INT_PTR i = iMonCount; (i--) > 0;)
	{
		_FindAllMonitorsItem& Item = Monitors.MonArray[i];
		if (Item.hMon == hNearest)
		{
			//POINT ptNext = {
			//	Next ? (Item.rcMon.right + 100) : (Item.rcMon.left - 100),
			//	Item.ptCenter.y};
			//hFound = MonitorFromPoint(ptNext, MONITOR_DEFAULTTONEAREST);
			//if (hFound != hNearest)
			//	break; // ����� ���-�� ����������


			INT_PTR j;
			if (Next)
			{
				j = ((i + 1) < iMonCount) ? (i + 1) : 0;
			}
			else
			{
				j = (i > 0) ? (i - 1) : (iMonCount - 1);
			}
			hFound = Monitors.MonArray[j].hMon;
			break;
		}
	}

	if (!hFound)
	{
		_ASSERTE((hFound!=NULL) && "Can't find current monitor in monitors array");

		if (Next)
		{
			hFound = Monitors.MonArray[0].hMon;
		}
		else
		{
			hFound = Monitors.MonArray[iMonCount-1].hMon;
		}
	}

	if (hFound != NULL)
	{
		MONITORINFO mi = {sizeof(mi)};
		if (GetMonitorInfo(hFound, &mi))
		{
			if (pmi) *pmi = mi;
		}
		else
		{
			hFound = NULL;
		}
	}

	return hFound;
}
