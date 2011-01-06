
/*
Copyright (c) 2010 Maximus5
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

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
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


//#include <windows.h>
//#include <TCHAR.H>
#include "headers.hpp"
#include "dizlist.hpp"
#include "config.hpp"
//#include "imports.hpp"

#ifndef ARRAYSIZE
	#define ARRAYSIZE(array) (sizeof(array)/sizeof(*array))
#endif

/* FAR */
PluginStartupInfo psi;
FarStandardFunctions fsf;

// Diz object
DizList diz;


BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
                     )
{
    return TRUE;
}


#if defined(__GNUC__)

extern
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
                     );

#ifdef __cplusplus
extern "C"{
#endif
  BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
  int WINAPI GetMinFarVersionW(void);
  void WINAPI SetStartupInfoW(const struct PluginStartupInfo *Info);
  void WINAPI GetPluginInfoW(struct PluginInfo *Info);
  int WINAPI GetCustomDataW(const wchar_t *FilePath, wchar_t **CustomData);
  void WINAPI FreeCustomDataW(wchar_t *CustomData);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  DllMain(hDll, dwReason, lpReserved);
  return TRUE;
}
#endif


int WINAPI GetMinFarVersionW(void)
{
    return MAKEFARVERSION(2,0,1588);
}

void WINAPI SetStartupInfoW(const struct PluginStartupInfo *Info)
{
    ::psi = *Info;
    ::fsf = *(Info->FSF);
    
//    ifn.Load();
    ReadConfig();
}

void WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
	Info->Flags = PF_PRELOAD;
}

int WINAPI GetCustomDataW(const wchar_t *FilePath, wchar_t **CustomData)
{
	*CustomData = NULL;
	
	const wchar_t* pszSlash = wcsrchr(FilePath, L'\\');
	if (!pszSlash || pszSlash <= FilePath) return FALSE;
	if (pszSlash[1] == 0) return FALSE; // ���� ����� ��� ������ ��� ����� - �� ����� ��� �����
	string  strPath(FilePath, pszSlash-FilePath);
	
	// ������������ ������ ���-����� ���������� ��� diz
	if (diz.Read(strPath) == 0)
	{
		// ���� ��� ������ - ����� �������
		return FALSE;
	}

	
	const wchar_t* pszDiz = diz.GetDizTextAddr(pszSlash+1, L"", 0/*???*/);
	//if (!pszDiz || pszDiz[0] == 0) -- ConvertNameToShort �������� ����� ����� �������
	//{
	//	string strShort;
	//	ConvertNameToShort(FilePath, strShort);
	//	pszDiz = diz.GetDizTextAddr(pszSlash+1, strShort, 0/*???*/);
	//}
	if (!pszDiz || pszDiz[0] == 0)
	{
		return FALSE;
	}
	
	size_t nLen = wcslen(pszDiz)+1;
	*CustomData = (wchar_t*)malloc(nLen*2);
	wcscpy(*CustomData, pszDiz);
	// �������� ��������� �������
	wchar_t* pszTab = wcspbrk(*CustomData, L"\t");
	while (pszTab)
	{
		*pszTab = L' ';
		pszTab = wcspbrk(pszTab+1, L"\t");
	}

	return TRUE;
}

void WINAPI FreeCustomDataW(wchar_t *CustomData)
{
	if (CustomData)
		free(CustomData);
}
