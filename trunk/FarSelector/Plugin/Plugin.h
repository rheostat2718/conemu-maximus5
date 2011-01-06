#pragma once

#define SZ_PLUGIN_PIPE _T("\\\\.\\pipe\\FarSelectorPipe.%u")

#define PIPEBUFSIZE 4096

#define SafeCloseHandle(h) { if ((h)!=NULL) { HANDLE hh = (h); (h) = NULL; if (hh!=INVALID_HANDLE_VALUE) CloseHandle(hh); } }

typedef struct tag_SelectorStr {
	DWORD   nAllSize;
	DWORD   nCmd;       // 1 - �������� ����, 2 - �������� ��������� � �������� ����� ���
	BOOL    bActiveFar; // ������ ���������� TRUE, ���� ��� ��� ������� (�������� ������� � ConEmu, ��� ������� ������� �������)
	DWORD   hWnd;       // HWND ��� ����������� ����, ��� �������� ���� ConEmu
	// ����� - ������
	DWORD   nDataLen; // ���������� wchar_t
	wchar_t szData[66000];
} SELECTORSTR;
