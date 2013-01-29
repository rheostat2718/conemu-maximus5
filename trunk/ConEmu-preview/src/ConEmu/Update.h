
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

struct ConEmuUpdateSettings;
class CConEmuUpdate;
class MSection;

extern CConEmuUpdate* gpUpd;

class CWinInet;

class CConEmuUpdate
{
protected:
	BOOL mb_InCheckProcedure;
	DWORD mn_CheckThreadId;
	HANDLE mh_CheckThread;
	
	//HANDLE mh_StopThread;

	CWinInet* wi;
	friend class CWinInet;
	bool mb_InetMode, mb_DroppedMode;
	HANDLE mh_Internet, mh_Connect, mh_SrcFile;
	DWORD mn_InternetContentLen, mn_InternetContentReady, mn_PackageSize;
	DWORD_PTR mn_Context;
	void CloseInternet(bool bFull);
	
	BOOL mb_ManualCallMode;
	ConEmuUpdateSettings* mp_Set;
	
	bool mb_InShowLastError;
	wchar_t* ms_LastErrorInfo;
	MSection* mp_LastErrorSC;

	wchar_t* mpsz_DeleteIniFile;
	wchar_t* mpsz_DeletePackageFile;
	wchar_t* mpsz_DeleteBatchFile;
	void DeleteBadTempFiles();

	wchar_t* mpsz_PendingPackageFile;
	wchar_t* mpsz_PendingBatchFile;

	DWORD mn_Timeout, mn_ConnTimeout, mn_DataTimeout, mn_FileTimeout;
	
	static DWORD WINAPI CheckThreadProc(LPVOID lpParameter);
	DWORD CheckProcInt();
	
	wchar_t* CreateTempFile(LPCWSTR asDir, LPCWSTR asFileNameTempl, HANDLE& hFile);
	wchar_t* CreateBatchFile(LPCWSTR asPackage);
	
	bool IsLocalFile(LPWSTR& asPathOrUrl);
	bool IsLocalFile(LPCWSTR& asPathOrUrl);

	bool bNeedRunElevation;
	bool NeedRunElevation();
	
	BOOL DownloadFile(LPCWSTR asSource, LPCWSTR asTarget, HANDLE hDstFile, DWORD& crc, BOOL abPackage = FALSE);
	BOOL ReadSource(LPCWSTR asSource, BOOL bInet, HANDLE hSource, BYTE* pData, DWORD cbData, DWORD* pcbRead);
	BOOL WriteTarget(LPCWSTR asTarget, HANDLE hTarget, const BYTE* pData, DWORD cbData);
	
	void ReportError(LPCWSTR asFormat, DWORD nErrCode);
	void ReportError(LPCWSTR asFormat, LPCWSTR asArg, DWORD nErrCode);
	void ReportError(LPCWSTR asFormat, LPCWSTR asArg1, LPCWSTR asArg2, DWORD nErrCode);

	void ReportErrorInt(wchar_t* asErrorInfo);

public:
	CConEmuUpdate();
	~CConEmuUpdate();
	
	void StartCheckProcedure(BOOL abShowMessages);
	void StopChecking();
	void ShowLastError();
	bool ShowConfirmation();
	bool StartLocalUpdate(LPCWSTR asDownloadedPackage);

	enum UpdateStep
	{
		us_NotStarted = 0,
		us_Check = 1,
		us_ConfirmDownload = 2,
		us_Downloading = 3,
		us_ConfirmUpdate = 4,
		us_ExitAndUpdate = 5,
	};
	UpdateStep InUpdate();

	short GetUpdateProgress();

protected:
	BOOL mb_RequestTerminate;
	UpdateStep m_UpdateStep;
	wchar_t ms_NewVersion[64], ms_CurVersion[64], ms_SkipVersion[64];
	wchar_t ms_DefaultTitle[128];
	bool QueryConfirmation(UpdateStep step, LPCWSTR asParm = NULL);
	bool QueryConfirmationInt(LPCWSTR asConfirmInfo);
	void WaitAllInstances();
	#if 0
	bool CanUpdateInstallation();
	#endif
};
