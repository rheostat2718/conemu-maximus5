
/*
Copyright (c) 2010-2011 Maximus5
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

#include <windows.h>
#ifdef _DEBUG
#pragma warning( disable : 4995 )
#endif
#include "../common/pluginW1900.hpp" // Far3
#ifdef _DEBUG
#pragma warning( default : 4995 )
#endif
#include "../common/plugin_helper.h"
#include "ConEmuBg.h"
#include "../ConEmu/version.h"

//#define FCTL_GETPANELDIR FCTL_GETCURRENTDIRECTORY

//#define _ACTL_GETFARRECT 32

#ifdef _DEBUG
#define SHOW_DEBUG_EVENTS
#endif

struct PluginStartupInfo *InfoW1900 = NULL;
struct FarStandardFunctions *FSFW1900 = NULL;

GUID guid_ConEmuBg = { /* f02bbf1c-25af-4c90-bb5b-a4140227df40 */
    0xf02bbf1c,
    0x25af,
    0x4c90,
    {0xbb, 0x5b, 0xa4, 0x14, 0x02, 0x27, 0xdf, 0x40}
  };
GUID guid_ConEmuBgCfgDlg = { /* b7359b6b-662e-4b90-aaac-d1720a1d020b */
    0xb7359b6b,
    0x662e,
    0x4b90,
    {0xaa, 0xac, 0xd1, 0x72, 0x0a, 0x1d, 0x02, 0x0b}
  };
GUID guid_ConEmuBgPluginMenu = { /* 394c5ba1-268f-41fd-b0f3-6a4e1c716eca */
    0x394c5ba1,
    0x268f,
    0x41fd,
    {0xb0, 0xf3, 0x6a, 0x4e, 0x1c, 0x71, 0x6e, 0xca}
  };
GUID guid_ConEmuBgPluginConfig = { /* 513f9204-9368-4f4c-90d3-9b5e37244f31 */
    0x513f9204,
    0x9368,
    0x4f4c,
    {0x90, 0xd3, 0x9b, 0x5e, 0x37, 0x24, 0x4f, 0x31}
  };


void WINAPI GetGlobalInfoW(struct GlobalInfo *Info)
{
	//static wchar_t szTitle[16]; _wcscpy_c(szTitle, L"ConEmu");
	//static wchar_t szDescr[64]; _wcscpy_c(szTitle, L"ConEmu support for Far Manager");
	//static wchar_t szAuthr[64]; _wcscpy_c(szTitle, L"ConEmu.Maximus5@gmail.com");

	//Info->StructSize = sizeof(GlobalInfo);
	_ASSERTE(Info->StructSize >= sizeof(GlobalInfo));
	Info->MinFarVersion = FARMANAGERVERSION;

	// Build: YYMMDDX (YY - ��� ����� ����, MM - �����, DD - ����, X - 0 � ����-����� ���������)
	Info->Version = MAKEFARVERSION(MVV_1,MVV_2,MVV_3,((MVV_1 % 100)*100000) + (MVV_2*1000) + (MVV_3*10) + (MVV_4 % 10),VS_RELEASE);
	
	Info->Guid = guid_ConEmuBg;
	Info->Title = L"ConEmu Background";
	Info->Description = L"Paint Far panels background in the ConEmu window";
	Info->Author = L"ConEmu.Maximus5@gmail.com";
}

void GetPluginInfoW1900(void *piv)
{
	PluginInfo *pi = (PluginInfo*)piv;
	//memset(pi, 0, sizeof(PluginInfo));
	//pi->StructSize = sizeof(struct PluginInfo);
	_ASSERTE(pi->StructSize>0 && ((size_t)pi->StructSize >= sizeof(*pi)/*(size_t)(((LPBYTE)&pi->MacroFunctionNumber) - (LPBYTE)pi))*/));

	static wchar_t *szMenu[1], szMenu1[255];
	szMenu[0] = szMenu1;
	lstrcpynW(szMenu1, GetMsgW(CEPluginName), 240);

	pi->Flags = gbBackgroundEnabled?PF_PRELOAD:0;
	//pi->PluginMenu.Guids = &guid_ConEmuBgPluginMenu;
	//pi->PluginMenu.Strings = szMenu;
	//pi->PluginMenu.Count = 1;
	pi->PluginConfig.Guids = &guid_ConEmuBgPluginConfig;
	pi->PluginConfig.Strings = szMenu;
	pi->PluginConfig.Count = 1;
}

void SetStartupInfoW1900(void *aInfo)
{
	INIT_FAR_PSI(::InfoW1900, ::FSFW1900, (PluginStartupInfo*)aInfo);
	
	//_ASSERTE(FPS_SHOWSTATUSLINE == 0x00000040);
	//_ASSERTE(FPS_SHOWCOLUMNTITLES == 0x00000020);

#ifdef _DEBUG
	INT_PTR i;
	wchar_t szPath[MAX_PATH];
	lstrcpy(szPath, L"C:\\Windows\\TEMP\\fff.log");
	i = FSFW1900->ProcessName(L"*\\TEMP\\*", szPath, 0, PN_CMPNAME);
	i = FSFW1900->ProcessName(L"TEMP", szPath, 0, PN_CMPNAME);
	i = FSFW1900->ProcessName(L"*TEMP*", szPath, 0, PN_CMPNAME);
	lstrcpy(szPath, L"C:\\vc\\.svn\\fff.log");
	i = FSFW1900->ProcessName(L"*\\.SVN\\*", szPath, 0, PN_CMPNAME);
	lstrcpy(szPath, L"C:\\vc\\.svn\\");
	i = FSFW1900->ProcessName(L"*\\.SVN\\*", szPath, 0, PN_CMPNAME);
#endif

	VersionInfo FarVer = {0};
	if (InfoW1900->AdvControl(&guid_ConEmuBg, ACTL_GETFARMANAGERVERSION, 0, &FarVer))
	{
		if (FarVer.Major == 3)
		{
			gFarVersion.dwBuild = FarVer.Build;
			_ASSERTE(FarVer.Major<=0xFFFF && FarVer.Minor<=0xFFFF)
			gFarVersion.dwVerMajor = (WORD)FarVer.Major;
			gFarVersion.dwVerMinor = (WORD)FarVer.Minor;
		}
		else
		{
			_ASSERTE(FarVer.Major == 3);
		}
	}
	
	//int nLen = lstrlenW(InfoW1900->RootKey)+16;

	//if (gszRootKey) free(gszRootKey);

	//gszRootKey = (wchar_t*)calloc(nLen,2);
	//lstrcpyW(gszRootKey, InfoW1900->RootKey);
	//WCHAR* pszSlash = gszRootKey+lstrlenW(gszRootKey)-1;

	//if (*pszSlash != L'\\') *(++pszSlash) = L'\\';

	//lstrcpyW(pszSlash+1, L"ConEmuTh\\");
}

extern BOOL gbInfoW_OK;
HANDLE WINAPI OpenW(const struct OpenInfo *Info)
{
	if (!gbInfoW_OK)
		return INVALID_HANDLE_VALUE;

	return OpenPluginWcmn(Info->OpenFrom, Info->Data);
}

void ExitFARW1900(void)
{
	if (InfoW1900)
	{
		free(InfoW1900);
		InfoW1900=NULL;
	}

	if (FSFW1900)
	{
		free(FSFW1900);
		FSFW1900=NULL;
	}
}

LPCWSTR GetMsgW1900(int aiMsg)
{
	if (!InfoW1900 || !InfoW1900->GetMsg)
		return L"";

	return InfoW1900->GetMsg(&guid_ConEmuBg,aiMsg);
}

#define FAR_UNICODE 1867
#include "Configure.h"

int ConfigureW1900(int ItemNumber)
{
	if (!InfoW1900)
		return false;

	return ConfigureProc(ItemNumber);
}

void SettingsLoadW1900()
{
	FarSettingsCreate sc = {sizeof(FarSettingsCreate), guid_ConEmuBg, INVALID_HANDLE_VALUE};
	FarSettingsItem fsi = {0};
	if (InfoW1900->SettingsControl(INVALID_HANDLE_VALUE, SCTL_CREATE, 0, &sc))
	{
		//BYTE cVal; DWORD nVal;

		for (ConEmuBgSettings *p = gSettings; p->pszValueName; p++)
		{
			if (p->nValueType == REG_BINARY)
			{
				_ASSERTE(p->nValueSize == 1);
				//cVal = (BYTE)*(BOOL*)p->pValue;
				fsi.Name = p->pszValueName;
				fsi.Type = FST_DATA;
				//fsi.Data.Size = 1; 
				//fsi.Data.Data = &cVal;
				if (InfoW1900->SettingsControl(sc.Handle, SCTL_GET, 0, &fsi) && (fsi.Data.Size == sizeof(BYTE)))
					*((BOOL*)p->pValue) = (*((BYTE*)fsi.Data.Data) != 0);
			}
			else if (p->nValueType == REG_DWORD)
			{
				_ASSERTE(p->nValueSize == 4);
				fsi.Name = p->pszValueName;
				fsi.Type = FST_DATA;
				//fsi.Data.Size = 4;
				//fsi.Data.Data = &nVal;
				if (InfoW1900->SettingsControl(sc.Handle, SCTL_GET, 0, &fsi) && (fsi.Data.Size == sizeof(DWORD)))
					*((DWORD*)p->pValue) = *((DWORD*)fsi.Data.Data);
			}
			else if (p->nValueType == REG_SZ)
			{
				fsi.Name = p->pszValueName;
				fsi.Type = FST_STRING;
				fsi.String = NULL;
				if (InfoW1900->SettingsControl(sc.Handle, SCTL_GET, 0, &fsi) && fsi.String)
					lstrcpyn((wchar_t*)p->pValue, fsi.String, p->nValueSize);
			}
		}

		InfoW1900->SettingsControl(sc.Handle, SCTL_FREE, 0, 0);
	}
}
void SettingsSaveW1900()
{
	if (!InfoW1900)
		return;

	FarSettingsCreate sc = {sizeof(FarSettingsCreate), guid_ConEmuBg, INVALID_HANDLE_VALUE};
	FarSettingsItem fsi = {0};
	if (InfoW1900->SettingsControl(INVALID_HANDLE_VALUE, SCTL_CREATE, 0, &sc))
	{
		BYTE cVal;
		for (ConEmuBgSettings *p = gSettings; p->pszValueName; p++)
		{
			if (p->nValueType == REG_BINARY)
			{
				_ASSERTE(p->nValueSize == 1);
				cVal = (BYTE)*(BOOL*)p->pValue;
				fsi.Name = p->pszValueName;
				fsi.Type = FST_DATA;
				fsi.Data.Size = 1; 
				fsi.Data.Data = &cVal;
				InfoW1900->SettingsControl(sc.Handle, SCTL_SET, 0, &fsi);
			}
			else if (p->nValueType == REG_DWORD)
			{
				_ASSERTE(p->nValueSize == 4);
				fsi.Name = p->pszValueName;
				fsi.Type = FST_DATA;
				fsi.Data.Size = 4;
				fsi.Data.Data = p->pValue;
				InfoW1900->SettingsControl(sc.Handle, SCTL_SET, 0, &fsi);
			}
			else if (p->nValueType == REG_SZ)
			{
				_ASSERTE(p->nValueSize == MAX_PATH);
				fsi.Name = p->pszValueName;
				fsi.Type = FST_STRING;
				fsi.String = (wchar_t*)p->pValue; // ASCIIZ
				InfoW1900->SettingsControl(sc.Handle, SCTL_SET, 0, &fsi);
			}
		}

		InfoW1900->SettingsControl(sc.Handle, SCTL_FREE, 0, 0);
	}
}

bool FMatchW1900(LPCWSTR asMask, LPWSTR asPath)
{
	INT_PTR iRc = FSFW1900->ProcessName(asMask, asPath, 0, PN_CMPNAME);
	return (iRc != 0);
}
