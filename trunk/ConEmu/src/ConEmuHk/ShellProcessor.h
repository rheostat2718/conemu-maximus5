
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

#pragma once

enum CmdOnCreateType
{
	eShellExecute = 1,
	eCreateProcess,
	eInjectingHooks,
	eHooksLoaded,
};

class CShellProc
{
private:
	UINT mn_CP; // = AreFileApisANSI() ? CP_ACP : CP_OEMCP;
	
	// ��� ����������� ���������� Ansi ������� (�������� ����� Unicode ��� ����������)
	LPWSTR mpwsz_TempAction; // = str2wcs(asAction, nCP);
	LPWSTR mpwsz_TempFile; // = str2wcs(asFile, nCP);
	LPWSTR mpwsz_TempParam; // = str2wcs(asParam, nCP);
	
	LPSTR  mpsz_TempRetFile;
	LPSTR  mpsz_TempRetParam;
	LPWSTR mpwsz_TempRetFile;
	LPWSTR mpwsz_TempRetParam;
	
	// ����� ��� ShellExecuteEx - ������ �� ����� ������ ���� ������
	LPSHELLEXECUTEINFOA mlp_ExecInfoA, mlp_SaveExecInfoA;
	LPSHELLEXECUTEINFOW mlp_ExecInfoW, mlp_SaveExecInfoW;
	
	// ���������� � ����������� ��������
	DWORD mn_ImageSubsystem, mn_ImageBits;
	wchar_t ms_Executable[MAX_PATH+1];
	BOOL mb_WasSuspended; // ���� TRUE - ������ ��� ������ CreateProcessXXX ��� ��� ���� CREATE_SUSPENDED
	BOOL mb_NeedInjects;
	
	static int mn_InShellExecuteEx;
	BOOL mb_InShellExecuteEx;

private:
	wchar_t* str2wcs(const char* psz, UINT anCP);
	char* wcs2str(const wchar_t* pwsz, UINT anCP);
	BOOL PrepareExecuteParms(
				enum CmdOnCreateType aCmd,
				LPCWSTR asAction, LPCWSTR asFile, LPCWSTR asParam,
				DWORD* anShellFlags, DWORD* anCreateFlags, DWORD* anStartFlags, DWORD* anShowCmd, // ��� Shell & Create �����
				HANDLE* lphStdIn, HANDLE* lphStdOut, HANDLE* lphStdErr,
				LPWSTR* psFile, LPWSTR* psParam);
	BOOL ChangeExecuteParms(enum CmdOnCreateType aCmd,
				LPCWSTR asFile, LPCWSTR asParam, LPCWSTR asBaseDir,
				LPCWSTR asExeFile, int ImageBits, int ImageSubsystem,
				LPWSTR* psFile, LPWSTR* psParam);
public:
	static CESERVER_REQ* NewCmdOnCreate(enum CmdOnCreateType aCmd,
				LPCWSTR asAction, LPCWSTR asFile, LPCWSTR asParam,
				DWORD* anShellFlags, DWORD* anCreateFlags, DWORD* anStartFlags, DWORD* anShowCmd,
				int nImageBits, int nImageSubsystem,
				HANDLE hStdIn, HANDLE hStdOut, HANDLE hStdErr,
				wchar_t (&szBaseDir)[MAX_PATH+2]);
public:
	CShellProc();
	~CShellProc();
public:
	// ��� ������� ���������� TRUE, ���� ������� �������������� (����� ����� ������ Inject)
	BOOL OnShellExecuteA(LPCSTR* asAction, LPCSTR* asFile, LPCSTR* asParam, DWORD* anFlags, DWORD* anShowCmd);
	BOOL OnShellExecuteW(LPCWSTR* asAction, LPCWSTR* asFile, LPCWSTR* asParam, DWORD* anFlags, DWORD* anShowCmd);
	BOOL OnShellExecuteExA(LPSHELLEXECUTEINFOA* lpExecInfo);
	BOOL OnShellExecuteExW(LPSHELLEXECUTEINFOW* lpExecInfo);
	BOOL OnCreateProcessA(LPCSTR* asFile, LPCSTR* asCmdLine, DWORD* anCreationFlags, LPSTARTUPINFOA lpSI);
	BOOL OnCreateProcessW(LPCWSTR* asFile, LPCWSTR* asCmdLine, DWORD* anCreationFlags, LPSTARTUPINFOW lpSI);
	// ���������� ����� ��������� �������� ��������
	void OnCreateProcessFinished(BOOL abSucceeded, PROCESS_INFORMATION *lpPI);
	void OnShellFinished(BOOL abSucceeded, HINSTANCE ahInstApp, HANDLE ahProcess);
};

// Service functions
typedef DWORD (WINAPI* GetProcessId_t)(HANDLE Process);
extern GetProcessId_t gfGetProcessId;
