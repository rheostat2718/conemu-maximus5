
#pragma once

extern RECT    grcConEmuClient;   // ��� ������ ������ ����
extern BOOL    gbAttachGuiClient; // ��� ������ ������ ����
extern BOOL    gbGuiClientAttached; // ��� ������ ������ ���� (������� ������������)
extern BOOL    gbGuiClientExternMode; // ���� ����� �������� Gui-���������� ��� ������� ConEmu
extern HWND    ghAttachGuiClient; // ����� ShowWindow �����������
extern DWORD   gnAttachGuiClientFlags; // enum ATTACHGUIAPP_FLAGS
extern DWORD   gnAttachGuiClientStyle, gnAttachGuiClientStyleEx;

bool CheckCanCreateWindow(LPCSTR lpClassNameA, LPCWSTR lpClassNameW, DWORD& dwStyle, DWORD& dwExStyle, HWND& hWndParent, BOOL& bAttachGui, BOOL& bStyleHidden);
void ReplaceGuiAppWindow(BOOL abStyleHidden);
void OnGuiWindowAttached(HWND hWindow, HMENU hMenu, LPCSTR asClassA, LPCWSTR asClassW, DWORD anStyle, DWORD anStyleEx, BOOL abStyleHidden, int anFromShowWindow=-1);
void OnShowGuiClientWindow(HWND hWnd, int &nCmdShow, BOOL &rbGuiAttach);
void OnPostShowGuiClientWindow(HWND hWnd, int nCmdShow);
bool OnSetGuiClientWindowPos(HWND hWnd, HWND hWndInsertAfter, int &X, int &Y, int &cx, int &cy, UINT uFlags);
void SetGuiExternMode(BOOL abUseExternMode, LPRECT prcOldPos = NULL);
void AttachGuiWindow(HWND hOurWindow);