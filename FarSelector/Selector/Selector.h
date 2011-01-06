#pragma once

#include "resource.h"

typedef struct tag_FarInstance
{
	DWORD dwPID;
	BOOL  bActiveFar; // ������ ���������� TRUE, ���� ��� ��� ������� (�������� ������� � ConEmu, ��� ������� ������� �������)
	HWND  hWnd;
	wchar_t* pszLeft;
	wchar_t* pszRight;
} FARINSTANCE;


//INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK SelectionProc(HWND, UINT, WPARAM, LPARAM);
BOOL FindInstances();
int NextArg(wchar_t** asCmdLine, wchar_t** rsArg/*[32768]*/);
void StartNewInstance(HWND hParent);
BOOL CallInstance(int iSel);
