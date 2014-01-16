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

template <class T> struct PipeServer;
//#include "../common/PipeServer.h"

class CGuiServer
{
public:
	CGuiServer();
	~CGuiServer();
	bool Start();
	void Stop(bool abDeinitialize=false);
protected:	
	PipeServer<CESERVER_REQ>* mp_GuiServer;
	//static DWORD CALLBACK GuiServerThread(LPVOID lpvParam);
	//void GuiServerThreadCommand(HANDLE hPipe);
	//DWORD mn_GuiServerThreadId; HANDLE mh_GuiServerThread, mh_GuiServerThreadTerminate;
	wchar_t ms_ServerPipe[MAX_PATH];
	static BOOL WINAPI GuiServerCommand(LPVOID pInst, CESERVER_REQ* pIn, CESERVER_REQ* &ppReply, DWORD &pcbReplySize, DWORD &pcbMaxReplySize, LPARAM lParam);
	static void WINAPI GuiServerFree(CESERVER_REQ* pReply, LPARAM lParam);
#ifdef _DEBUG
	static BOOL WINAPI OnGuiServerConnected(LPVOID pInst, LPARAM lParam);
#endif
};
