#include <CRT/crt.hpp>
#include <initguid.h>
#include "Network.hpp"
#include "version.hpp"

//-----------------------------------------------------------------------------
#if defined(__GNUC__)
#ifdef __cplusplus
extern "C"
{
#endif
	BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
	(void) hDll;
	(void) dwReason;
	(void) lpReserved;
	return TRUE;
}
#endif

void WINAPI GetGlobalInfoW(GlobalInfo *Info)
{
	Info->StructSize=sizeof(GlobalInfo);
	Info->MinFarVersion=FARMANAGERVERSION;
	Info->Version=PLUGIN_VERSION;
	Info->Guid=MainGuid;
	Info->Title=PLUGIN_NAME;
	Info->Description=PLUGIN_DESC;
	Info->Author=PLUGIN_AUTHOR;
}

//-----------------------------------------------------------------------------
HANDLE WINAPI OpenW(const OpenInfo *OInfo)
{
	HANDLE hPlugin=new NetBrowser;

	if (hPlugin==NULL)
		return nullptr;

	NetBrowser *Browser=(NetBrowser *)hPlugin;

	if (OInfo->OpenFrom==OPEN_COMMANDLINE)
	{
		wchar_t Path[MAX_PATH] = L"\\\\";
		int I=0;
		wchar_t *cmd=(wchar_t *)OInfo->Data;
		wchar_t *p=wcschr(cmd, L':');

		if (!p || !*p)
		{
			delete Browser;
			return nullptr;
		}

		*p++ = L'\0';
		bool netg;

		if (!lstrcmpi(cmd, L"netg"))
			netg = true;
		else if (!lstrcmpi(cmd, L"net"))
			netg = false;
		else
		{
			delete Browser;
			return nullptr;
		}

		cmd = p;

		if (lstrlen(FSF.Trim(cmd)))
		{
			if (cmd [0] == L'/')
				cmd [0] = L'\\';

			if (cmd [1] == L'/')
				cmd [1] = L'\\';

			if (!netg && !Opt.NavigateToDomains)
			{
				if (cmd[0] == L'\\' && cmd[1] != L'\\')
					I=1;
				else if (cmd[0] != L'\\' && cmd[1] != L'\\')
					I=2;
			}

			lstrcpy(Path+I, cmd);
			FSF.Unquote(Path);
			// Expanding environment variables.
			{
				wchar_t PathCopy[MAX_PATH];
				lstrcpy(PathCopy, Path);
				ExpandEnvironmentStrings(PathCopy, Path, ARRAYSIZE(Path));
			}
			Browser->SetOpenFromCommandLine(Path);
		}
	}
	/* The line below is an UNDOCUMENTED and UNSUPPORTED EXPERIMENTAL
	    mechanism supported ONLY in FAR 1.70 beta 3. It will NOT be supported
	    in later versions. Please DON'T use it in your plugins. */
	else if (OInfo->OpenFrom == OPEN_FILEPANEL)
	{
		if (!Browser->SetOpenFromFilePanel((wchar_t *) OInfo->Data))
		{
			// we don't support upwards browsing from NetWare shares -
			// it doesn't work correctly
			delete Browser;
			return nullptr;
		}
	}
	#if 1
	//Maximus: ���� ��� ������� ������� �� ������� Far ������������ �� ������ - �� ������ Network ������������ � �������
	else if (OInfo->OpenFrom == OPEN_FROMMACRO)
	{
		BOOL Succeeded=FALSE;
		OpenMacroInfo* FromMacro=(OpenMacroInfo*)OInfo->Data;
		if (FromMacro->StructSize>=sizeof(*FromMacro) && FromMacro->Count==2)
		{
			if (FromMacro->Values[0].Type==FMVT_STRING && FromMacro->Values[1].Type==FMVT_STRING)
			{
				if (lstrcmpi(FromMacro->Values[0].String, L"connect")==0)
				{
					wchar_t* Resource=_wcsdup(FromMacro->Values[1].String);
					if (Browser->SetOpenFromFilePanel(Resource))
					{
						if (Browser->SetDirectory(FromMacro->Values[1].String, 0)
							&& Browser->SetDirectory(FromMacro->Values[1].String, 0)
							//&& Browser->GetFindData(NULL,NULL,0)
							)
						{
							Succeeded=TRUE;
						}
					}
					free(Resource);
				}
			}
		}
		delete Browser;
		return (HANDLE)Succeeded;
	}
	#endif
	else
	{
		if (IsFirstRun && Opt.LocalNetwork)
			Browser->GotoLocalNetwork();
	}

	IsFirstRun = FALSE;
	wchar_t szCurrDir[MAX_PATH];

	if (GetCurrentDirectory(ARRAYSIZE(szCurrDir), szCurrDir))
	{
		if (*szCurrDir == L'\\' && GetSystemDirectory(szCurrDir, ARRAYSIZE(szCurrDir)))
		{
			szCurrDir[2] = L'\0';
			SetCurrentDirectory(szCurrDir);
		}
	}

	return(hPlugin);
}

//-----------------------------------------------------------------------------
void WINAPI ClosePanelW(const ClosePanelInfo* Info)
{
	delete(NetBrowser *)Info->hPanel;
}

//-----------------------------------------------------------------------------
int WINAPI GetFindDataW(GetFindDataInfo *Info)
{
	NetBrowser *Browser=(NetBrowser *)Info->hPanel;
	return(Browser->GetFindData(&Info->PanelItem,&Info->ItemsNumber,Info->OpMode));
}

//-----------------------------------------------------------------------------
void WINAPI FreeFindDataW(const FreeFindDataInfo *Info)
{
	NetBrowser *Browser=(NetBrowser *)Info->hPanel;
	Browser->FreeFindData(Info->PanelItem,(int)Info->ItemsNumber);
}

//-----------------------------------------------------------------------------
void WINAPI GetOpenPanelInfoW(OpenPanelInfo *Info)
{
	NetBrowser *Browser=(NetBrowser *)Info->hPanel;
	Browser->GetOpenPanelInfo(Info);
}

//-----------------------------------------------------------------------------
int WINAPI SetDirectoryW(const struct SetDirectoryInfo *Info)
{
	NetBrowser *Browser=(NetBrowser *)Info->hPanel;
	return(Browser->SetDirectory(Info->Dir,Info->OpMode));
}

//-----------------------------------------------------------------------------
int WINAPI DeleteFilesW(const struct DeleteFilesInfo *Info)
{
	NetBrowser *Browser=(NetBrowser *)Info->hPanel;
	return(Browser->DeleteFiles(Info->PanelItem,(int)Info->ItemsNumber,Info->OpMode));
}

//-----------------------------------------------------------------------------
int WINAPI ProcessPanelInputW(const ProcessPanelInputInfo *Info)
{
	NetBrowser *Browser=(NetBrowser *)Info->hPanel;
	return(Browser->ProcessKey(&Info->Rec));
}

//-----------------------------------------------------------------------------
int WINAPI ProcessPanelEventW(const ProcessPanelEventInfo *Info)
{
	NetBrowser *Browser=(NetBrowser *)Info->hPanel;
	return Browser->ProcessEvent(Info->Event, Info->Param);
}

//-----------------------------------------------------------------------------
