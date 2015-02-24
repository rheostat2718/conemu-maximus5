﻿
/*
Copyright (c) 2009-2015 Maximus5
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

#include <windows.h>

struct CEStartupEnv;
struct MSectionSimple;

class MFileLog
{
	protected:
		wchar_t* ms_FilePathName;
		wchar_t* ms_FileName;
		wchar_t* ms_DefPath;
		HANDLE   mh_LogFile;
		HRESULT  InitFileName(LPCWSTR asName = NULL, DWORD anPID = 0);
		static void LogStartEnvInt(LPCWSTR asText, LPARAM lParam, bool bFirst, bool bNewLine);
	protected:
		MSectionSimple* mpcs_Lock;
	public:
		MFileLog(LPCWSTR asName, LPCWSTR asDir = NULL, DWORD anPID = 0);
		~MFileLog();
		void CloseLogFile();
		HRESULT CreateLogFile(LPCWSTR asName = NULL, DWORD anPID = 0, DWORD anLevel = 0); // Returns 0 if succeeded, otherwise - GetLastError() code
		LPCWSTR GetLogFileName();
		void LogString(LPCSTR asText, bool abWriteTime = true, LPCSTR asThreadName = NULL, bool abNewLine = true, UINT anCP = CP_ACP);
		void LogString(LPCWSTR asText, bool abWriteTime = true, LPCWSTR asThreadName = NULL, bool abNewLine = true);
		void LogStartEnv(CEStartupEnv* apStartEnv);
};
