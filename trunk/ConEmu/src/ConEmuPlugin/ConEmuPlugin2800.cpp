
/*
Copyright (c) 2009-2013 Maximus5
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
#include "../common/common.hpp"
#ifdef _DEBUG
#pragma warning( disable : 4995 )
#endif
#include "../common/pluginW2800.hpp" // Far3
#ifdef _DEBUG
#pragma warning( default : 4995 )
#endif
#include "../common/plugin_helper.h"
#include "PluginHeader.h"
#include "../ConEmu/version.h"
#include "../common/farcolor3.hpp"
#include "../common/ConEmuColors3.h"
#include "../ConEmuHk/SetHook.h"

#ifdef _DEBUG
//#define SHOW_DEBUG_EVENTS
#endif

GUID guid_ConEmu = { /* 4b675d80-1d4a-4ea9-8436-fdc23f2fc14b */
	0x4b675d80,
	0x1d4a,
	0x4ea9,
	{0x84, 0x36, 0xfd, 0xc2, 0x3f, 0x2f, 0xc1, 0x4b}
};
GUID guid_ConEmuPluginItems = { /* 3836ad1f-5130-4a13-93d8-15fefbdc3185 */
	0x3836ad1f,
	0x5130,
	0x4a13,
	{0x93, 0xd8, 0x15, 0xfe, 0xfb, 0xdc, 0x31, 0x85}
};
GUID guid_ConEmuPluginMenu = { /* 830d40da-ccf3-417b-b378-87f9441c4c95 */
	0x830d40da,
	0xccf3,
	0x417b,
	{0xb3, 0x78, 0x87, 0xf9, 0x44, 0x1c, 0x4c, 0x95}
};
GUID guid_ConEmuGuiMacroDlg = { /* a0484f91-a800-4e3a-abac-aed4485da79d */
	0xa0484f91,
	0xa800,
	0x4e3a,
	{0xab, 0xac, 0xae, 0xd4, 0x48, 0x5d, 0xa7, 0x9d}
};
GUID guid_ConEmuWaitEndSynchro = { /* e93fba92-d7de-4651-9be1-c9b064254f65 */
	0xe93fba92,
	0xd7de,
	0x4651,
	{0x9b, 0xe1, 0xc9, 0xb0, 0x64, 0x25, 0x4f, 0x65}
};

struct PluginStartupInfo *InfoW2800=NULL;
struct FarStandardFunctions *FSFW2800=NULL;

void WaitEndSynchroW2800();

static wchar_t* GetPanelDir(HANDLE hPanel)
{
	wchar_t* pszDir = NULL;
	size_t nSize;
	
	PanelInfo pi = {sizeof(pi)};
	nSize = InfoW2800->PanelControl(hPanel, FCTL_GETPANELINFO, 0, &pi);

	nSize = InfoW2800->PanelControl(hPanel, FCTL_GETPANELDIRECTORY, 0, 0);

	if (nSize)
	{
		FarPanelDirectory* pDir = (FarPanelDirectory*)calloc(nSize, 1);
		if (pDir)
		{
			pDir->StructSize = sizeof(*pDir);
			nSize = InfoW2800->PanelControl(hPanel, FCTL_GETPANELDIRECTORY, nSize, pDir);
			pszDir = lstrdup(pDir->Name);
			free(pDir);
		}
	}
	// ��������� �� ����� �������� ����, ���� ��� ��� ��������
	//_ASSERTE(nSize>0 || (pi.Flags & PFLAGS_PLUGIN));

	return pszDir;
}

void GetPluginInfoW2800(void *piv)
{
	PluginInfo *pi = (PluginInfo*)piv;
	//memset(pi, 0, sizeof(PluginInfo));
	//pi->StructSize = sizeof(struct PluginInfo);
	_ASSERTE(pi->StructSize>0 && ((size_t)pi->StructSize >= sizeof(*pi)/*(size_t)(((LPBYTE)&pi->MacroFunctionNumber) - (LPBYTE)pi))*/));

	static wchar_t *szMenu[1], szMenu1[255];
	szMenu[0]=szMenu1; //lstrcpyW(szMenu[0], L"[&\x2560] ConEmu"); -> 0x2584
	lstrcpynW(szMenu1, GetMsgW(CEPluginName), 240);

	//static WCHAR *szMenu[1], szMenu1[255];
	//szMenu[0]=szMenu1; //lstrcpyW(szMenu[0], L"[&\x2560] ConEmu"); -> 0x2584
	//szMenu[0][1] = L'&';
	//szMenu[0][2] = 0x2560;
	// ���������, �� ���������� �� ������� ������� �������, � ���� �� - ����������� �������
	//IsKeyChanged(TRUE); -- � FAR2 ��������, ���������� Synchro
	//if (gcPlugKey) szMenu1[0]=0; else lstrcpyW(szMenu1, L"[&\x2584] ");
	//lstrcpynW(szMenu1+lstrlenW(szMenu1), GetMsgW(2), 240);
	//lstrcpynW(szMenu1, GetMsgW(CEPluginName), 240);
	//_ASSERTE(pi->StructSize = sizeof(struct PluginInfo));
	pi->Flags = PF_EDITOR | PF_VIEWER | PF_DIALOG | PF_PRELOAD;
	//pi->DiskMenuStrings = NULL;
	//pi->DiskMenuNumbers = 0;
	pi->PluginMenu.Guids = &guid_ConEmuPluginMenu;
	pi->PluginMenu.Strings = szMenu;
	pi->PluginMenu.Count = 1;
	//pi->PluginConfigStrings = NULL;
	//pi->PluginConfigStringsNumber = 0;
	pi->CommandPrefix = L"ConEmu";
	//pi->Reserved = ConEmu_SysID; // 'CEMU'
}


void ProcessDragFromW2800()
{
	if (!InfoW2800 || !InfoW2800->AdvControl)
		return;

	WindowInfo WInfo = {sizeof(WindowInfo)};
	WInfo.Pos = 0;
	_ASSERTE(GetCurrentThreadId() == gnMainThreadId);
	InfoW2800->AdvControl(&guid_ConEmu, ACTL_GETWINDOWINFO, 0, &WInfo);

	if (!(WInfo.Flags & WIF_CURRENT))
	{
		int ItemsCount=0;
		//WriteFile(hPipe, &ItemsCount, sizeof(int), &cout, NULL);
		OutDataAlloc(sizeof(ItemsCount));
		OutDataWrite(&ItemsCount,sizeof(ItemsCount));
		return;
	}

	PanelInfo PInfo = {sizeof(PInfo)};
	WCHAR *szCurDir = NULL;
	InfoW2800->PanelControl(PANEL_ACTIVE, FCTL_GETPANELINFO, NULL, &PInfo);

	if ((PInfo.PanelType == PTYPE_FILEPANEL || PInfo.PanelType == PTYPE_TREEPANEL) 
		&& (PInfo.Flags & PFLAGS_VISIBLE))
	{
		szCurDir = GetPanelDir(PANEL_ACTIVE);
		if (!szCurDir)
		{
			_ASSERTE(szCurDir!=NULL);
			int ItemsCount=0;
			OutDataWrite(&ItemsCount, sizeof(int));
			OutDataWrite(&ItemsCount, sizeof(int)); // ����� �������
			return;
		}
		int nDirLen=0, nDirNoSlash=0;

		if (szCurDir[0])
		{
			nDirLen = lstrlen(szCurDir);

			if (nDirLen>0)
				if (szCurDir[nDirLen-1]!=L'\\')
					nDirNoSlash=1;
		}

		// ��� ������ �������������� ������, ��� ������������� �� ����� ��������
		OutDataAlloc(sizeof(int)+PInfo.SelectedItemsNumber*((MAX_PATH+2)+sizeof(int)));
		//Maximus5 - ����� ������ ��������
		int nNull=0; // ItemsCount
		//WriteFile(hPipe, &nNull, sizeof(int), &cout, NULL);
		OutDataWrite(&nNull/*ItemsCount*/, sizeof(int));

		if (PInfo.SelectedItemsNumber<=0)
		{
			// �������� ����, ��� �� ����� �� ".."
			if (PInfo.CurrentItem == 0 && PInfo.ItemsNumber > 0)
			{
				if (!nDirNoSlash)
					szCurDir[nDirLen-1] = 0;
				else
					nDirLen++;

				int nWholeLen = nDirLen + 1;
				OutDataWrite(&nWholeLen, sizeof(int));
				OutDataWrite(&nDirLen, sizeof(int));
				OutDataWrite(szCurDir, sizeof(WCHAR)*nDirLen);
			}

			// Fin
			OutDataWrite(&nNull/*ItemsCount*/, sizeof(int));
		}
		else
		{
			PluginPanelItem **pi = (PluginPanelItem**)calloc(PInfo.SelectedItemsNumber, sizeof(PluginPanelItem*));
			bool *bIsFull = (bool*)calloc(PInfo.SelectedItemsNumber, sizeof(bool));
			INT_PTR ItemsCount=PInfo.SelectedItemsNumber, i;
			int nMaxLen=MAX_PATH+1, nWholeLen=1;

			// ������� ��������� ������������ ����� ������
			for(i=0; i<ItemsCount; i++)
			{
				size_t sz = InfoW2800->PanelControl(PANEL_ACTIVE, FCTL_GETSELECTEDPANELITEM, i, NULL);

				if (!sz)
					continue;

				pi[i] = (PluginPanelItem*)calloc(sz, 1); // ������ ������������ � ������

				FarGetPluginPanelItem gppi = {sizeof(gppi), sz, pi[i]};
				if (!InfoW2800->PanelControl(PANEL_ACTIVE, FCTL_GETSELECTEDPANELITEM, i, &gppi))
				{
					free(pi[i]); pi[i] = NULL;
					continue;
				}

				if (!pi[i]->FileName)
				{
					_ASSERTE(pi[i]->FileName!=NULL);
					free(pi[i]); pi[i] = NULL;
					continue;
				}

				if (i == 0
				        && ((pi[i]->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
				        && !lstrcmpW(pi[i]->FileName, L".."))
				{
					free(pi[i]); pi[i] = NULL;
					continue;
				}

				int nLen=nDirLen+nDirNoSlash;

				if ((pi[i]->FileName[0] == L'\\' && pi[i]->FileName[1] == L'\\') ||
				        (ISALPHA(pi[i]->FileName[0]) && pi[i]->FileName[1] == L':' && pi[i]->FileName[2] == L'\\'))
					{ nLen = 0; bIsFull[i] = TRUE; } // ��� ��� ������ ����!

				nLen += lstrlenW(pi[i]->FileName);

				if (nLen>nMaxLen)
					nMaxLen = nLen;

				nWholeLen += (nLen+1);
			}

			nMaxLen += nDirLen;

			//WriteFile(hPipe, &nWholeLen, sizeof(int), &cout, NULL);
			OutDataWrite(&nWholeLen, sizeof(int));
			WCHAR* Path = new WCHAR[nMaxLen+1];

			for (i=0; i<ItemsCount; i++)
			{
				//WCHAR Path[MAX_PATH+1];
				//ZeroMemory(Path, MAX_PATH+1);
				//Maximus5 - ������ � ������ ����� � ������������ overflow
				//StringCchPrintf(Path, countof(Path), L"%s\\%s", szCurDir, PInfo.SelectedItems[i]->FileName);
				Path[0]=0;

				if (!pi[i] || !pi[i]->FileName) continue;  //���� ������� �������� �� �������

				int nLen=0;

				if (nDirLen>0 && !bIsFull[i])
				{
					lstrcpy(Path, szCurDir);

					if (nDirNoSlash)
					{
						Path[nDirLen]=L'\\';
						Path[nDirLen+1]=0;
					}

					nLen = nDirLen+nDirNoSlash;
				}

				lstrcpy(Path+nLen, pi[i]->FileName);
				nLen += lstrlen(pi[i]->FileName);
				nLen++;
				//WriteFile(hPipe, &nLen, sizeof(int), &cout, NULL);
				OutDataWrite(&nLen, sizeof(int));
				//WriteFile(hPipe, Path, sizeof(WCHAR)*nLen, &cout, NULL);
				OutDataWrite(Path, sizeof(WCHAR)*nLen);
			}

			for(i=0; i<ItemsCount; i++)
			{
				if (pi[i]) free(pi[i]);
			}

			free(pi); pi = NULL;
			free(bIsFull);
			delete [] Path; Path=NULL;
			// ����� ������
			//WriteFile(hPipe, &nNull, sizeof(int), &cout, NULL);
			OutDataWrite(&nNull, sizeof(int));
		}

		SafeFree(szCurDir);
	}
	else
	{
		int ItemsCount=0;
		OutDataWrite(&ItemsCount, sizeof(int));
		OutDataWrite(&ItemsCount, sizeof(int)); // ����� �������
	}

	//free(szCurDir);
}

void ProcessDragToW2800()
{
	if (!InfoW2800 || !InfoW2800->AdvControl)
		return;

	WindowInfo WInfo = {sizeof(WindowInfo)};
	//WInfo.Pos = 0;
	WInfo.Pos = -1; // ��������� �������� � �������� � ���������
	_ASSERTE(GetCurrentThreadId() == gnMainThreadId);
	InfoW2800->AdvControl(&guid_ConEmu, ACTL_GETWINDOWINFO, 0, &WInfo);

	if (!(WInfo.Flags & WIF_CURRENT))
	{
		//InfoW2800->AdvControl(&guid_ConEmu, ACTL_FREEWINDOWINFO, 0, &WInfo);
		int ItemsCount=0;
		if (gpCmdRet==NULL)
			OutDataAlloc(sizeof(ItemsCount));
		OutDataWrite(&ItemsCount,sizeof(ItemsCount));
		return;
	}

	int nStructSize;

	if ((WInfo.Type == WTYPE_DIALOG) || (WInfo.Type == WTYPE_EDITOR))
	{
		// ��������� ���� � ���� ������
		ForwardedPanelInfo DlgInfo = {};
		DlgInfo.NoFarConsole = TRUE;
		nStructSize = sizeof(DlgInfo);
		if (gpCmdRet==NULL)
			OutDataAlloc(nStructSize+sizeof(nStructSize));
		OutDataWrite(&nStructSize, sizeof(nStructSize));
		OutDataWrite(&DlgInfo, nStructSize);
		return;
	}
	else if (WInfo.Type != WTYPE_PANELS)
	{
		// ����� - ���� �� ��������
		int ItemsCount=0;
		if (gpCmdRet==NULL)
			OutDataAlloc(sizeof(ItemsCount));
		OutDataWrite(&ItemsCount,sizeof(ItemsCount));
		return;
	}

	nStructSize = sizeof(ForwardedPanelInfo)+4; // ����� �������� �� ����� �����

	//InfoW2800->AdvControl(&guid_ConEmu, ACTL_FREEWINDOWINFO, 0, &WInfo);
	PanelInfo PAInfo = {sizeof(PAInfo)}, PPInfo = {sizeof(PPInfo)};
	ForwardedPanelInfo *pfpi=NULL;
	//ZeroMemory(&fpi, sizeof(fpi));
	BOOL lbAOK=FALSE, lbPOK=FALSE;
	WCHAR *szPDir = NULL;
	WCHAR *szADir = NULL;
	//if (!(lbAOK=InfoW2800->PanelControl(PANEL_ACTIVE, FCTL_GETPANELSHORTINFO, &PAInfo)))
	lbAOK=InfoW2800->PanelControl(PANEL_ACTIVE, FCTL_GETPANELINFO, 0, &PAInfo) &&
	      (szADir = GetPanelDir(PANEL_ACTIVE));

	if (lbAOK && szADir)
		nStructSize += (lstrlen(szADir))*sizeof(WCHAR);

	lbPOK=InfoW2800->PanelControl(PANEL_PASSIVE, FCTL_GETPANELINFO, 0, &PPInfo) &&
	      (szPDir = GetPanelDir(PANEL_PASSIVE));

	if (lbPOK && szPDir)
		nStructSize += (lstrlen(szPDir))*sizeof(WCHAR); // ������ WCHAR! �� TCHAR

	pfpi = (ForwardedPanelInfo*)calloc(nStructSize,1);

	if (!pfpi)
	{
		int ItemsCount=0;

		//WriteFile(hPipe, &ItemsCount, sizeof(int), &cout, NULL);
		if (gpCmdRet==NULL)
			OutDataAlloc(sizeof(ItemsCount));

		OutDataWrite(&ItemsCount,sizeof(ItemsCount));
		SafeFree(szADir);
		SafeFree(szPDir);
		return;
	}

	pfpi->ActivePathShift = sizeof(ForwardedPanelInfo);
	pfpi->pszActivePath = (WCHAR*)(((char*)pfpi)+pfpi->ActivePathShift);
	pfpi->PassivePathShift = pfpi->ActivePathShift+2; // ���� ActivePath ���������� - ��������

	if (lbAOK)
	{
		pfpi->ActiveRect=PAInfo.PanelRect;

		if (!(PAInfo.Flags & PFLAGS_PLUGIN)
			&& (PAInfo.PanelType == PTYPE_FILEPANEL || PAInfo.PanelType == PTYPE_TREEPANEL) 
			&& (PAInfo.Flags & PFLAGS_VISIBLE))
		{
			if (szADir[0])
			{
				lstrcpyW(pfpi->pszActivePath, szADir);
				pfpi->PassivePathShift += lstrlenW(pfpi->pszActivePath)*2;
			}
		}
	}

	pfpi->pszPassivePath = (WCHAR*)(((char*)pfpi)+pfpi->PassivePathShift);

	if (lbPOK)
	{
		pfpi->PassiveRect=PPInfo.PanelRect;

		if (!(PPInfo.Flags & PFLAGS_PLUGIN)
			&& (PPInfo.PanelType == PTYPE_FILEPANEL || PPInfo.PanelType == PTYPE_TREEPANEL) 
			&& (PPInfo.Flags & PFLAGS_VISIBLE))
		{
			if (szPDir[0])
				lstrcpyW(pfpi->pszPassivePath, szPDir);
		}
	}

	// ����������, ��������� ����������
	//WriteFile(hPipe, &nStructSize, sizeof(nStructSize), &cout, NULL);
	//WriteFile(hPipe, pfpi, nStructSize, &cout, NULL);
	if (gpCmdRet==NULL)
		OutDataAlloc(nStructSize+sizeof(nStructSize));

	OutDataWrite(&nStructSize, sizeof(nStructSize));
	OutDataWrite(pfpi, nStructSize);
	free(pfpi); pfpi=NULL;
	SafeFree(szADir);
	SafeFree(szPDir);
}

void SetStartupInfoW2800(void *aInfo)
{
	INIT_FAR_PSI(::InfoW2800, ::FSFW2800, (PluginStartupInfo*)aInfo);

	VersionInfo FarVer = {0};
	if (InfoW2800->AdvControl(&guid_ConEmu, ACTL_GETFARMANAGERVERSION, 0, &FarVer))
	{
		if (FarVer.Major == 3)
		{
			gFarVersion.dwBuild = FarVer.Build;
			_ASSERTE(FarVer.Major<=0xFFFF && FarVer.Minor<=0xFFFF)
			gFarVersion.dwVerMajor = (WORD)FarVer.Major;
			gFarVersion.dwVerMinor = (WORD)FarVer.Minor;
			gFarVersion.Bis = (FarVer.Stage==VS_BIS);
			InitRootKey();
		}
		else
		{
			_ASSERTE(FarVer.Major == 3);
		}
	}

	// ���� ������ ��������� � Far2, � �� menu_ShowTabsList ���� ����� ������������
	_ASSERTE(MACROAREA_SHELL==1 && MACROAREA_SEARCH==5 && MACROAREA_INFOPANEL==10 && MACROAREA_QVIEWPANEL==11);
	_ASSERTE(MACROAREA_TREEPANEL==12 && OPEN_FILEPANEL==7 && MACROAREA_EDITOR==3 && MACROAREA_VIEWER==2);
}

DWORD GetEditorModifiedStateW2800()
{
	EditorInfo ei;
	InfoW2800->EditorControl(-1/*Active editor*/, ECTL_GETINFO, 0, &ei);
#ifdef SHOW_DEBUG_EVENTS
	char szDbg[255];
	wsprintfA(szDbg, "Editor:State=%i\n", ei.CurState);
	OutputDebugStringA(szDbg);
#endif
	// ���� �� ��������, �� ��� �� �������������
	DWORD currentModifiedState = ((ei.CurState & (ECSTATE_MODIFIED|ECSTATE_SAVED)) == ECSTATE_MODIFIED) ? 1 : 0;
	return currentModifiedState;
}

//extern int lastModifiedStateW;

#ifdef __cplusplus
extern "C"
{
#endif

int WINAPI ProcessEditorEventW(int Event, void *Param);
int WINAPI ProcessViewerEventW(int Event, void *Param);
int WINAPI ProcessDialogEventW(int Event, void *Param);
int WINAPI ProcessSynchroEventW(int Event,void *Param);

#ifdef __cplusplus
};
#endif


INT_PTR WINAPI ProcessEditorEventW2800(void* p)
{
	const ProcessEditorEventInfo* Info = (const ProcessEditorEventInfo*)p;
	return ProcessEditorEventW(Info->Event, Info->Param);
}

INT_PTR WINAPI ProcessViewerEventW2800(void* p)
{
	const ProcessViewerEventInfo* Info = (const ProcessViewerEventInfo*)p;
	return ProcessViewerEventW(Info->Event, Info->Param);
}

INT_PTR WINAPI ProcessDialogEventW2800(void* p)
{
	const ProcessDialogEventInfo* Info = (const ProcessDialogEventInfo*)p;
	return ProcessDialogEventW(Info->Event, Info->Param);
}

INT_PTR WINAPI ProcessSynchroEventW2800(void* p)
{
	const ProcessSynchroEventInfo* Info = (const ProcessSynchroEventInfo*)p;
	return ProcessSynchroEventW(Info->Event, Info->Param);
}

// watch non-modified -> modified editor status change
int ProcessEditorInputW2800(LPCVOID aRec)
{
	if (!InfoW2800)
		return 0;

	const ProcessEditorInputInfo *apInfo = (const ProcessEditorInputInfo*)aRec;

	// only key events with virtual codes > 0 are likely to cause status change (?)
	if (!gbRequestUpdateTabs && (apInfo->Rec.EventType & 0xFF) == KEY_EVENT
	        && (apInfo->Rec.Event.KeyEvent.wVirtualKeyCode || apInfo->Rec.Event.KeyEvent.wVirtualScanCode || apInfo->Rec.Event.KeyEvent.uChar.UnicodeChar)
	        && apInfo->Rec.Event.KeyEvent.bKeyDown)
	{
		//if (!gbRequestUpdateTabs)
		gbNeedPostEditCheck = TRUE;
	}

	return 0;
}


bool UpdateConEmuTabsW2800(int anEvent, bool losingFocus, bool editorSave, void* Param/*=NULL*/)
{
	if (!InfoW2800 || !InfoW2800->AdvControl || gbIgnoreUpdateTabs)
		return false;

	BOOL lbCh = FALSE, lbDummy = FALSE;
	WindowInfo WInfo = {sizeof(WindowInfo)};
	wchar_t szWNameBuffer[CONEMUTABMAX];
	//WInfo.Name = szWNameBuffer;
	//WInfo.NameSize = CONEMUTABMAX;
	int windowCount = (int)InfoW2800->AdvControl(&guid_ConEmu, ACTL_GETWINDOWCOUNT, 0, NULL);
	if ((windowCount == 0) && !gpFarInfo->bFarPanelAllowed)
	{
		windowCount = 1; lbDummy = TRUE;
	}
	lbCh = (lastWindowCount != windowCount);

	if (!CreateTabs(windowCount))
		return false;

	int tabCount = 0;

	if (lbDummy)
	{
		AddTab(tabCount, false, false, WTYPE_PANELS, NULL, NULL, 1, 0, 0, 0);
		return (lbCh != FALSE);
	}

	//EditorInfo ei = {0};
	//if (editorSave)
	//{
	//	InfoW2800->EditorControl(-1/*Active editor*/, ECTL_GETINFO, &ei);
	//}
	ViewerInfo vi = {sizeof(ViewerInfo)};

	if (anEvent == 206)
	{
		if (Param)
			vi.ViewerID = *(int*)Param;

		InfoW2800->ViewerControl(-1/*Active viewer*/, VCTL_GETINFO, 0, &vi);
	}

	BOOL lbActiveFound = FALSE;

	_ASSERTE(GetCurrentThreadId() == gnMainThreadId);

	WindowInfo WActive = {sizeof(WActive)};
	WActive.Pos = -1;
	bool bActiveInfo = InfoW2800->AdvControl(&guid_ConEmu, ACTL_GETWINDOWINFO, 0, &WActive)!=0;
	// ���� ��� ������� � ������ "/e" (��� standalone ��������) - ����� ������ ��� ������ ������� 
	// ������� ���������� �� ���� (�������� ��� �� ������?, � ������� ������ ���)
	_ASSERTE(bActiveInfo && (WActive.Flags & WIF_CURRENT));
	static WindowInfo WLastActive;

	// ���������, ���� �� �������� ��������/������/������
	if (bActiveInfo && (WActive.Type == WTYPE_EDITOR || WActive.Type == WTYPE_VIEWER || WActive.Type == WTYPE_PANELS))
	{
		if (!(WActive.Flags & WIF_MODAL))
			WLastActive = WActive;
	}
	else
	{
		int nTabs = 0, nModalTabs = 0;
		bool bFound = false;
		WindowInfo WModal, WFirst;
		// ��������� � ����� ������� �� ������������ - ���� ��������� "��������" ����
		// �.�. ������������ ��� ���, ������� ��� ������� �����
		for (int i = 0; i < windowCount; i++)
		{
			WInfo.Pos = i;
			if (InfoW2800->AdvControl(&guid_ConEmu, ACTL_GETWINDOWINFO, 0, &WInfo)
				&& (WInfo.Type == WTYPE_EDITOR || WInfo.Type == WTYPE_VIEWER || WInfo.Type == WTYPE_PANELS))
			{
				if (!nTabs)
					WFirst = WInfo;
				nTabs++;
				if (WInfo.Flags & WIF_MODAL)
				{
					nModalTabs++;
					WModal = WInfo;
				}

				if (WLastActive.StructSize && (WInfo.Type == WLastActive.Type) && (WInfo.Id == WLastActive.Id))
				{
					bActiveInfo = bFound = true;
					WActive = WInfo;
				}
			}
		}

		if (!bFound)
		{
			if (nModalTabs)
			{
				bActiveInfo = true;
				WActive = WModal;
			}
			else if (nTabs)
			{
				bActiveInfo = true;
				WActive = WFirst;
			}
		}
	}

	for (int i = 0; i < windowCount; i++)
	{
		WInfo.Pos = i;
		InfoW2800->AdvControl(&guid_ConEmu, ACTL_GETWINDOWINFO, 0, &WInfo);

		if (WInfo.Type == WTYPE_EDITOR || WInfo.Type == WTYPE_VIEWER || WInfo.Type == WTYPE_PANELS)
		{
			WInfo.Pos = i;
			WInfo.Name = szWNameBuffer;
			WInfo.NameSize = CONEMUTABMAX;
			InfoW2800->AdvControl(&guid_ConEmu, ACTL_GETWINDOWINFO, 0, &WInfo);
			WARNING("��� ��������� ����� ����� ���������� ECTL_GETFILENAME");

			//// ���������, ���� ���...
			//_ASSERTE((WInfo.Flags & WIF_MODAL) == 0);

			if (WInfo.Type == WTYPE_EDITOR || WInfo.Type == WTYPE_VIEWER || WInfo.Type == WTYPE_PANELS)
			{
				if ((WInfo.Flags & WIF_CURRENT))
					lbActiveFound = TRUE;
				else if (bActiveInfo && (WInfo.Type == WActive.Type) && (WInfo.Id == WActive.Id))
				{
					WInfo.Flags |= WIF_CURRENT;
					lbActiveFound = TRUE;
				}

				TODO("����������� �� ���������/�������");
				lbCh |= AddTab(tabCount, losingFocus, editorSave,
				               WInfo.Type, WInfo.Name, /*editorSave ? ei.FileName :*/ NULL,
				               (WInfo.Flags & WIF_CURRENT), (WInfo.Flags & WIF_MODIFIED), (WInfo.Flags & WIF_MODAL),
							   0/*WInfo.Id?*/);
				//if (WInfo.Type == WTYPE_EDITOR && WInfo.Current) //2009-08-17
				//	lastModifiedStateW = WInfo.Modified;
			}

			//InfoW2800->AdvControl(&guid_ConEmu, ACTL_FREEWINDOWINFO, (void*)&WInfo);
		}
	}

	// Viewer � FAR 2 build 9xx �� �������� � ������ ���� ��� ������� VE_GOTFOCUS
	_ASSERTE(VE_GOTFOCUS==6);
	if (!losingFocus && !editorSave && tabCount == 0 && anEvent == (200+VE_GOTFOCUS))
	{
		lbActiveFound = TRUE;
		size_t cchMax = InfoW2800->ViewerControl(vi.ViewerID, VCTL_GETFILENAME, 0, NULL);
		if (cchMax)
		{
			wchar_t* pszFileName = (wchar_t*)calloc(cchMax, sizeof(*pszFileName));
			if (pszFileName)
			{
				cchMax = InfoW2800->ViewerControl(vi.ViewerID, VCTL_GETFILENAME, 0, pszFileName);
				if (cchMax && *pszFileName)
				{
					lbCh |= AddTab(tabCount, losingFocus, editorSave,
							   WTYPE_VIEWER, pszFileName, NULL,
							   1, 0, 0, vi.ViewerID);
				}
				free(pszFileName);
			}
		}
	}

	// ������ ����� ��� ��������� �������� (��� ������?)
	if (!lbActiveFound && !losingFocus)
	{
		_ASSERTE("Active window must be detected already!" && 0);
		WInfo.Pos = -1;

		_ASSERTE(GetCurrentThreadId() == gnMainThreadId);
		if (InfoW2800->AdvControl(&guid_ConEmu, ACTL_GETWINDOWINFO, 0, &WInfo))
		{
			// ���������, ���� ���...
			_ASSERTE((WInfo.Flags & WIF_MODAL) == 0);

			if (WInfo.Type == WTYPE_EDITOR || WInfo.Type == WTYPE_VIEWER)
			{
				WInfo.Pos = -1;
				WInfo.Name = szWNameBuffer;
				WInfo.NameSize = CONEMUTABMAX;
				InfoW2800->AdvControl(&guid_ConEmu, ACTL_GETWINDOWINFO, 0, &WInfo);

				if (WInfo.Type == WTYPE_EDITOR || WInfo.Type == WTYPE_VIEWER)
				{
					tabCount = 0;
					TODO("����������� �� ���������/�������");
					lbCh |= AddTab(tabCount, losingFocus, editorSave,
					               WInfo.Type, WInfo.Name, /*editorSave ? ei.FileName :*/ NULL,
					               (WInfo.Flags & WIF_CURRENT), (WInfo.Flags & WIF_MODIFIED), 1/*Modal*/,
								   0);
				}
			}
			else if (WInfo.Type == WTYPE_PANELS)
			{
				gpTabs->Tabs.CurrentType = WInfo.Type;
			}
		}

		//wchar_t* pszEditorFileName = NULL;
		//EditorInfo ei = {0};
		//ViewerInfo vi = {sizeof(ViewerInfo)};
		//BOOL bEditor = FALSE, bViewer = FALSE;
		//bViewer = InfoW2800->ViewerControl(VCTL_GETINFO, &vi);
		//if (InfoW2800->EditorControl(ECTL_GETINFO, &ei)) {
		//	int nLen = InfoW2800->EditorControl(ECTL_GETFILENAME, NULL);
		//	if (nLen > 0) {
		//		wchar_t* pszEditorFileName = (wchar_t*)calloc(nLen+1,2);
		//		if (pszEditorFileName) {
		//			if (InfoW2800->EditorControl(ECTL_GETFILENAME, pszEditorFileName)) {
		//				bEditor = true;
		//			}
		//		}
		//	}
		//}
		//if (bEditor && bViewer) {
		//	// ��������� �������� ���������� �� �������� ����, �� ��� ����� �������� � ���������� ��������� �������� ���2?
		//	WInfo.Pos = -1;
		//	InfoW2800->AdvControl(&guid_ConEmu, ACTL_GETWINDOWINFO, (void*)&WInfo);
		//}
		//if (bEditor) {
		//	tabCount = 0;
		//	lbCh |= AddTab(tabCount, losingFocus, editorSave,
		//		WTYPE_EDITOR, pszEditorFileName, NULL,
		//		1, (ei.CurState & (ECSTATE_MODIFIED|ECSTATE_SAVED)) == ECSTATE_MODIFIED);
		//} else if (bViewer) {
		//	tabCount = 0;
		//	lbCh |= AddTab(tabCount, losingFocus, editorSave,
		//		WTYPE_VIEWER, vi.FileName, NULL,
		//		1, 0);
		//}
		//if (pszEditorFileName) free(pszEditorFileName);
	}

	// 101224 - ����� ��������� ����������!
	gpTabs->Tabs.nTabCount = tabCount;
	//// 2009-08-17
	//if (gbHandleOneRedraw && gbHandleOneRedrawCh && lbCh) {
	//	gbHandleOneRedraw = false;
	//	gbHandleOneRedrawCh = false;
	//}
#ifdef _DEBUG
	//WCHAR szDbg[128]; StringCchPrintf(szDbg, countof(szDbg), L"Event: %i, count %i\n", anEvent, tabCount);
	//OutputDebugStringW(szDbg);
#endif
	//SendTabs(tabCount, lbCh && (gnReqCommand==(DWORD)-1));
	return (lbCh != FALSE);
}

void ExitFARW2800(void)
{
	ShutdownPluginStep(L"ExitFARW2800");

	WaitEndSynchroW2800();

	if (InfoW2800)
	{
		free(InfoW2800);
		InfoW2800=NULL;
	}

	if (FSFW2800)
	{
		free(FSFW2800);
		FSFW2800=NULL;
	}

	ShutdownPluginStep(L"ExitFARW2800 - done");
}

int ShowMessageW2800(LPCWSTR asMsg, int aiButtons, bool bWarning)
{
	if (!InfoW2800 || !InfoW2800->Message)
		return -1;

	GUID lguid_Msg = { /* aba0df6c-163f-4950-9029-a3f595c1c351 */
	    0xaba0df6c,
	    0x163f,
	    0x4950,
	    {0x90, 0x29, 0xa3, 0xf5, 0x95, 0xc1, 0xc3, 0x51}
	};
	return InfoW2800->Message(&guid_ConEmu, &lguid_Msg, FMSG_ALLINONE1900|FMSG_MB_OK|(bWarning ? FMSG_WARNING : 0), NULL,
	                         (const wchar_t * const *)asMsg, 0, aiButtons);
}

int ShowMessageW2800(int aiMsg, int aiButtons)
{
	if (!InfoW2800 || !InfoW2800->Message || !InfoW2800->GetMsg)
		return -1;

	return ShowMessageW2800(InfoW2800->GetMsg(&guid_ConEmu,aiMsg), aiButtons, true);
}

LPCWSTR GetMsgW2800(int aiMsg)
{
	if (!InfoW2800 || !InfoW2800->GetMsg)
		return L"";

	return InfoW2800->GetMsg(&guid_ConEmu,aiMsg);
}

//void ReloadMacroW2800()
//{
//	if (!InfoW2800 || !InfoW2800->AdvControl)
//		return;
//
//	ActlKeyMacro command;
//	command.Command=MCMD_LOADALL;
//	InfoW2800->AdvControl(&guid_ConEmu,ACTL_KEYMACRO,&command);
//}

void SetWindowW2800(int nTab)
{
	if (!InfoW2800 || !InfoW2800->AdvControl)
		return;

	if (InfoW2800->AdvControl(&guid_ConEmu, ACTL_SETCURRENTWINDOW, nTab, NULL))
		InfoW2800->AdvControl(&guid_ConEmu, ACTL_COMMIT, 0, 0);
}

DWORD WINAPI BackgroundMacroError(LPVOID lpParameter)
{
	wchar_t* pszMacroError = (wchar_t*)lpParameter;

	MessageBox(NULL, pszMacroError, L"ConEmu plugin", MB_ICONSTOP|MB_SYSTEMMODAL);

	SafeFree(pszMacroError);

	return 0;
}

// Warning, �������� �� ��������. ������������ "�����" PostMacro
void PostMacroW2800(const wchar_t* asMacro, INPUT_RECORD* apRec)
{
	if (!InfoW2800 || !InfoW2800->AdvControl)
		return;

	MacroSendMacroText mcr = {sizeof(MacroSendMacroText)};
	//mcr.Flags = 0; // �� ��������� - ����� �� ����� ��������

	if (*asMacro == L'@' && asMacro[1] && asMacro[1] != L' ')
	{
		mcr.Flags |= KMFLAGS_DISABLEOUTPUT;
		asMacro ++;
	}

	// This macro was not adopted to Lua?
	_ASSERTE(*asMacro && *asMacro != L'$');

	wchar_t* pszMacroCopy = NULL;

	//Far3 build 2576: ������ $Text
	//�.�. ������� � ��� ����-����������� - ����� ����� � ������
	pszMacroCopy = lstrdup(asMacro);
	CharUpperBuff(pszMacroCopy, lstrlen(pszMacroCopy));
	// ������ ������, ���� ��� ���������� ������ � ������ ������� - �� �� ��� ������ �� �������...
	// ������� � Far 3 build 2851 - ��� ������� ���������� �� Lua
	#if 0
	if (wcsstr(pszMacroCopy, L"$TEXT") && !InfoW2800->MacroControl(&guid_ConEmu, MCTL_SENDSTRING, MSSC_CHECK, &mcr))
	{
		SafeFree(pszMacroCopy);
		pszMacroCopy = (wchar_t*)calloc(lstrlen(asMacro)+1,sizeof(wchar_t)*2);
		wchar_t* psz = pszMacroCopy;
		while (*asMacro)
		{
			if (asMacro[0] == L'$'
				&& (asMacro[1] == L'T' || asMacro[1] == L't')
				&& (asMacro[2] == L'E' || asMacro[2] == L'e')
				&& (asMacro[3] == L'X' || asMacro[3] == L'x')
				&& (asMacro[4] == L'T' || asMacro[4] == L't'))
			{
				lstrcpy(psz, L"print("); psz += 6;

				// ���������� spasing-symbols
				asMacro += 5;
				while (*asMacro == L' ' || *asMacro == L'\t' || *asMacro == L'\r' || *asMacro == L'\n')
					asMacro++;
				// ���������� ������ ��� ����������
				if (*asMacro == L'@' && *(asMacro+1) == L'"')
				{
					*(psz++) = *(asMacro++); *(psz++) = *(asMacro++);
					while (*asMacro)
					{
						*(psz++) = *(asMacro++);
						if (*(asMacro-1) == L'"')
						{
							if (*asMacro != L'"')
								break;
							*(psz++) = *(asMacro++);
						}
					}
				}
				else if (*asMacro == L'"')
				{
					*(psz++) = *(asMacro++);
					while (*asMacro)
					{
						*(psz++) = *(asMacro++);
						if (*(asMacro-1) == L'\\' && *asMacro == L'"')
						{
							*(psz++) = *(asMacro++);
						}
						else if (*(asMacro-1) == L'"')
						{
							break;
						}
					}
				}
				else if (*asMacro == L'%')
				{
					*(psz++) = *(asMacro++);
					while (*asMacro)
					{
						if (wcschr(L" \t\r\n", *asMacro))
							break;
						*(psz++) = *(asMacro++);
					}
				}
				else
				{
					SafeFree(pszMacroCopy);
					break; // ������
				}
				// ������� ������
				*(psz++) = L')';
			}
			else
			{
				*(psz++) = *(asMacro++);
			}
		}

		// ���� ������� ���������� ������
		if (pszMacroCopy)
			asMacro = pszMacroCopy;
	}
	#endif

	mcr.SequenceText = asMacro;
	if (apRec)
		mcr.AKey = *apRec;

	mcr.Flags |= KMFLAGS_SILENTCHECK;

	if (!InfoW2800->MacroControl(&guid_ConEmu, MCTL_SENDSTRING, MSSC_CHECK, &mcr))
	{
		wchar_t* pszErrText = NULL;
		size_t iRcSize = InfoW2800->MacroControl(&guid_ConEmu, MCTL_GETLASTERROR, 0, NULL);
		MacroParseResult* Result = iRcSize ? (MacroParseResult*)calloc(iRcSize,1) : NULL;
		if (Result)
		{
			Result->StructSize = sizeof(*Result);
			_ASSERTE(FALSE && "Check MCTL_GETLASTERROR");
			InfoW2800->MacroControl(&guid_ConEmu, MCTL_GETLASTERROR, iRcSize, Result);
			
			size_t cchMax = (Result->ErrSrc ? lstrlen(Result->ErrSrc) : 0) + lstrlen(asMacro) + 255;
			pszErrText = (wchar_t*)malloc(cchMax*sizeof(wchar_t));
			_wsprintf(pszErrText, SKIPLEN(cchMax)
				L"Error in Macro. Far %u.%u build %u r%u\nCode: %u, Line: %u, Col: %u%s%s\n----------------------------------\n%s",
				gFarVersion.dwVerMajor, gFarVersion.dwVerMinor, gFarVersion.dwBuild, gFarVersion.Bis ? 1 : 0,
				Result->ErrCode, (UINT)(int)Result->ErrPos.Y+1, (UINT)(int)Result->ErrPos.X+1,
				Result->ErrSrc ? L", Hint: " : L"", Result->ErrSrc ? Result->ErrSrc : L"",
				asMacro);

			SafeFree(Result);
		}
		else
		{
			size_t cchMax = lstrlen(asMacro) + 255;
			pszErrText = (wchar_t*)malloc(cchMax*sizeof(wchar_t));
			_wsprintf(pszErrText, SKIPLEN(cchMax)
				L"Error in Macro. Far %u.%u build %u r%u\n----------------------------------\n%s",
				gFarVersion.dwVerMajor, gFarVersion.dwVerMinor, gFarVersion.dwBuild, gFarVersion.Bis ? 1 : 0,
				asMacro);
		}

		if (pszErrText)
		{
			DWORD nTID;
			HANDLE h = CreateThread(NULL, 0, BackgroundMacroError, pszErrText, 0, &nTID);
			SafeCloseHandle(h);
		}
	}
	else
	{
		//gFarVersion.dwBuild
		InfoW2800->MacroControl(&guid_ConEmu, MCTL_SENDSTRING, 0, &mcr);

		//FAR BUGBUG: ������ �� ����������� �� ����������, ���� ������ �� ������ :(
		//  ��� ���� ����� ����������� ��� ������ ���� �� RClick
		//  ���� ������ �� ������ ������, �� RClick ����� �� ���������
		//  �� �������� ��������� :(
		// ���������� � "�����" PostMacro
		////if (!mcr.Param.PlainText.Flags) {
		//INPUT_RECORD ir[2] = {{MOUSE_EVENT},{MOUSE_EVENT}};
		//if (isPressed(VK_CAPITAL))
		//	ir[0].Event.MouseEvent.dwControlKeyState |= CAPSLOCK_ON;
		//if (isPressed(VK_NUMLOCK))
		//	ir[0].Event.MouseEvent.dwControlKeyState |= NUMLOCK_ON;
		//if (isPressed(VK_SCROLL))
		//	ir[0].Event.MouseEvent.dwControlKeyState |= SCROLLLOCK_ON;
		//ir[0].Event.MouseEvent.dwEventFlags = MOUSE_MOVED;
		//ir[1].Event.MouseEvent.dwControlKeyState = ir[0].Event.MouseEvent.dwControlKeyState;
		//ir[1].Event.MouseEvent.dwEventFlags = MOUSE_MOVED;
		//ir[1].Event.MouseEvent.dwMousePosition.X = 1;
		//ir[1].Event.MouseEvent.dwMousePosition.Y = 1;
		//
		////2010-01-29 ��������� STD_OUTPUT
		////if (!ghConIn) {
		////	ghConIn  = CreateFile(L"CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_READ,
		////		0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		////	if (ghConIn == INVALID_HANDLE_VALUE) {
		////		#ifdef _DEBUG
		////		DWORD dwErr = GetLastError();
		////		_ASSERTE(ghConIn!=INVALID_HANDLE_VALUE);
		////		#endif
		////		ghConIn = NULL;
		////		return;
		////	}
		////}
		//HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
		//DWORD cbWritten = 0;
		//#ifdef _DEBUG
		//BOOL fSuccess =
		//#endif
		//WriteConsoleInput(hIn/*ghConIn*/, ir, 1, &cbWritten);
		//_ASSERTE(fSuccess && cbWritten==1);
		////}
		////InfoW2800->AdvControl(&guid_ConEmu,ACTL_REDRAWALL,NULL);
	}

	SafeFree(pszMacroCopy);
}

int ShowPluginMenuW2800(ConEmuPluginMenuItem* apItems, int Count)
{
	if (!InfoW2800)
		return -1;

	//FarMenuItem items[] =
	//{
	//	{ConEmuHwnd ? MIF_SELECTED : MIF_DISABLE,  InfoW2800->GetMsg(&guid_ConEmu,CEMenuEditOutput)},
	//	{ConEmuHwnd ? 0 : MIF_DISABLE,             InfoW2800->GetMsg(&guid_ConEmu,CEMenuViewOutput)},
	//	{MIF_SEPARATOR},
	//	{ConEmuHwnd ? 0 : MIF_DISABLE,             InfoW2800->GetMsg(&guid_ConEmu,CEMenuShowHideTabs)},
	//	{ConEmuHwnd ? 0 : MIF_DISABLE,             InfoW2800->GetMsg(&guid_ConEmu,CEMenuNextTab)},
	//	{ConEmuHwnd ? 0 : MIF_DISABLE,             InfoW2800->GetMsg(&guid_ConEmu,CEMenuPrevTab)},
	//	{ConEmuHwnd ? 0 : MIF_DISABLE,             InfoW2800->GetMsg(&guid_ConEmu,CEMenuCommitTab)},
	//	{MIF_SEPARATOR},
	//	{0,                                        InfoW2800->GetMsg(&guid_ConEmu,CEMenuGuiMacro)},
	//	{MIF_SEPARATOR},
	//	{ConEmuHwnd||IsTerminalMode() ? MIF_DISABLE : MIF_SELECTED,  InfoW2800->GetMsg(&guid_ConEmu,CEMenuAttach)},
	//	{MIF_SEPARATOR},
	//	//#ifdef _DEBUG
	//	//{0, L"&~. Raise exception"},
	//	//#endif
	//	{IsDebuggerPresent()||IsTerminalMode() ? MIF_DISABLE : 0,    InfoW2800->GetMsg(&guid_ConEmu,CEMenuDebug)},
	//};

	FarMenuItem* items = (FarMenuItem*)calloc(Count, sizeof(*items));
	for (int i = 0; i < Count; i++)
	{
		if (apItems[i].Separator)
		{
			items[i].Flags = MIF_SEPARATOR;
			continue;
		}
		items[i].Flags	= (apItems[i].Disabled ? MIF_DISABLE : 0)
						| (apItems[i].Selected ? MIF_SELECTED : 0)
						| (apItems[i].Checked  ? MIF_CHECKED : 0)
						;
		items[i].Text = apItems[i].MsgText ? apItems[i].MsgText : InfoW2800->GetMsg(&guid_ConEmu, apItems[i].MsgID);
	}

	GUID lguid_Menu = { /* 2dc6b821-fd8e-4165-adcf-a4eda7b44e8e */
	    0x2dc6b821,
	    0xfd8e,
	    0x4165,
	    {0xad, 0xcf, 0xa4, 0xed, 0xa7, 0xb4, 0x4e, 0x8e}
	};
	int nRc = InfoW2800->Menu(&guid_ConEmu, &lguid_Menu, -1,-1, 0,
	                         FMENU_AUTOHIGHLIGHT|FMENU_CHANGECONSOLETITLE|FMENU_WRAPMODE,
	                         InfoW2800->GetMsg(&guid_ConEmu,CEPluginName),
	                         NULL, NULL, NULL, NULL, (FarMenuItem*)items, Count);
	SafeFree(items);
	return nRc;
}

BOOL EditOutputW2800(LPCWSTR asFileName, BOOL abView)
{
	if (!InfoW2800)
		return FALSE;

	BOOL lbRc = FALSE;

	if (!abView)
	{
		int iRc =
		    InfoW2800->Editor(asFileName, InfoW2800->GetMsg(&guid_ConEmu,CEConsoleOutput), 0,0,-1,-1,
		                     EF_NONMODAL|EF_IMMEDIATERETURN|EF_DELETEONLYFILEONCLOSE|EF_ENABLE_F6|EF_DISABLEHISTORY,
		                     0, 1, 1200);
		lbRc = (iRc != EEC_OPEN_ERROR);
	}
	else
	{
#ifdef _DEBUG
		int iRc =
#endif
		    InfoW2800->Viewer(asFileName, InfoW2800->GetMsg(&guid_ConEmu,CEConsoleOutput), 0,0,-1,-1,
		                     VF_NONMODAL|VF_IMMEDIATERETURN|VF_DELETEONLYFILEONCLOSE|VF_ENABLE_F6|VF_DISABLEHISTORY,
		                     1200);
		lbRc = TRUE;
	}

	return lbRc;
}

BOOL ExecuteSynchroW2800()
{
	if (!InfoW2800)
		return FALSE;

	if (IS_SYNCHRO_ALLOWED)
	{
		if (gbSynchroProhibited)
		{
			_ASSERT(gbSynchroProhibited==false);
			return FALSE;
		}

		// ���������� ����� 2-�, ���� ��� � ������ ������ ���-�� ����� (��������� �������?)
		//_ASSERTE(gnSynchroCount<=3);
		gnSynchroCount++;
		InfoW2800->AdvControl(&guid_ConEmu, ACTL_SYNCHRO, 0, NULL);
		return TRUE;
	}

	return FALSE;
}

static HANDLE ghSyncDlg = NULL;

void WaitEndSynchroW2800()
{
	// �������, ��� � Far 3 ��������
#if 0
	if ((gnSynchroCount == 0) || !(IS_SYNCHRO_ALLOWED))
		return;

	FarDialogItem items[] =
	{
		{DI_DOUBLEBOX,  3,  1,  51, 3, {0}, 0, 0, 0, GetMsgW2800(CEPluginName)},

		{DI_BUTTON,     0,  2,  0,  0, {0},  0, 0, DIF_FOCUS|DIF_CENTERGROUP|DIF_DEFAULTBUTTON, GetMsgW2800(CEStopSynchroWaiting)},
	};
	
	//GUID ConEmuWaitEndSynchro = { /* d0f369dc-2800-4833-a858-43dd1c115370 */
	//	    0xd0f369dc,
	//	    0x2800,
	//	    0x4833,
	//	    {0xa8, 0x58, 0x43, 0xdd, 0x1c, 0x11, 0x53, 0x70}
	//	  };
	
	ghSyncDlg = InfoW2800->DialogInit(&guid_ConEmu, &guid_ConEmuWaitEndSynchro,
			-1,-1, 55, 5, NULL, items, countof(items), 0, 0, NULL, 0);

	if (ghSyncDlg == INVALID_HANDLE_VALUE)
	{
		// ������, ��� � ��������� ������ (��������� �������� ���� ��������)
		// � ���� ������, �� ����, Synchro ���������� ����� �� ������
		gnSynchroCount = 0; // ��� ��� ������ ������� �������
	}
	else
	{
		InfoW2800->DialogRun(ghSyncDlg);
		InfoW2800->DialogFree(ghSyncDlg);
	}

	ghSyncDlg = NULL;
#endif
}

void StopWaitEndSynchroW2800()
{
	if (ghSyncDlg)
	{
		InfoW2800->SendDlgMessage(ghSyncDlg, DM_CLOSE, -1, 0);
	}
}


// Param ������ ���� ������� � ����. ������ ������������� � ProcessSynchroEventW.
//BOOL CallSynchroW2800(SynchroArg *Param, DWORD nTimeout /*= 10000*/)
//{
//	if (!InfoW2800 || !Param)
//		return FALSE;
//
//	if (gFarVersion.dwVerMajor>1 && (gFarVersion.dwVerMinor>0 || gFarVersion.dwBuild>=1006)) {
//		// ������� ������ ���������� 0
//		if (Param->hEvent)
//			ResetEvent(Param->hEvent);
//
//		//Param->Processed = FALSE;
//
//		InfoW2800->AdvControl ( &guid_ConEmu, ACTL_SYNCHRO, Param);
//
//		HANDLE hEvents[2] = {ghServerTerminateEvent, Param->hEvent};
//		int nCount = Param->hEvent ? 2 : 1;
//		_ASSERTE(Param->hEvent != NULL);
//
//		DWORD nWait = 100;
//		nWait = WaitForMultipleObjects(nCount, hEvents, FALSE, nTimeout);
//		if (nWait != WAIT_OBJECT_0 && nWait != (WAIT_OBJECT_0+1)) {
//			_ASSERTE(nWait==WAIT_OBJECT_0);
//			if (nWait == (WAIT_OBJECT_0+1)) {
//				// �������, ��� ������� ������ ������ ����������, ����� ��� ���� ��������� �� ���������
//				Param->Obsolete = TRUE;
//			}
//		}
//
//		return (nWait == 0);
//	}
//
//	return FALSE;
//}

BOOL IsMacroActiveW2800()
{
	if (!InfoW2800) return FALSE;

	INT_PTR liRc = InfoW2800->MacroControl(&guid_ConEmu, MCTL_GETSTATE, 0, 0);

	if (liRc == MACROSTATE_NOMACRO)
		return FALSE;

	return TRUE;
}

int GetMacroAreaW2800()
{
	int nArea = (int)InfoW2800->MacroControl(&guid_ConEmu, MCTL_GETAREA, 0, 0);
	return nArea;
}


void RedrawAllW2800()
{
	if (!InfoW2800) return;

	InfoW2800->AdvControl(&guid_ConEmu, ACTL_REDRAWALL, 0, NULL);
}

bool LoadPluginW2800(wchar_t* pszPluginPath)
{
	if (!InfoW2800) return false;

	InfoW2800->PluginsControl(INVALID_HANDLE_VALUE,PCTL_LOADPLUGIN,PLT_PATH,pszPluginPath);
	return true;
}

bool RunExternalProgramW2800(wchar_t* pszCommand)
{
	wchar_t strTemp[MAX_PATH+1];
	wchar_t *pszExpand = NULL;

	if (!pszCommand || !*pszCommand)
	{
		lstrcpy(strTemp, L"cmd");
		GUID lguid_Input = { /* 78ba0189-7dd7-4cb9-aff8-c70bca9f9cb6 */
		    0x78ba0189,
		    0x7dd7,
		    0x4cb9,
		    {0xaf, 0xf8, 0xc7, 0x0b, 0xca, 0x9f, 0x9c, 0xb6}
		};
		if (!InfoW2800->InputBox(&guid_ConEmu, &lguid_Input, L"ConEmu", L"Start console program", L"ConEmu.CreateProcess",
		                       strTemp, strTemp, MAX_PATH, NULL, FIB_BUTTONS))
			return false;

		pszCommand = strTemp;
	}
	else if (wcschr(pszCommand, L'%'))
	{
		DWORD cchMax = countof(strTemp);
		pszExpand = strTemp;
		DWORD nExpLen = ExpandEnvironmentStrings(pszCommand, pszExpand, cchMax);
		if (nExpLen)
		{
			if (nExpLen > cchMax)
			{
				cchMax = nExpLen + 32;
				pszExpand = (wchar_t*)calloc(cchMax,sizeof(*pszExpand));
				nExpLen = ExpandEnvironmentStrings(pszCommand, pszExpand, cchMax);
			}
			
			if (nExpLen && (nExpLen <= cchMax))
			{
				pszCommand = pszExpand;
			}
		}
	}

	wchar_t *pszCurDir = GetPanelDir(INVALID_HANDLE_VALUE);

	if (!pszCurDir)
	{
		pszCurDir = (wchar_t*)malloc(10);
		if (!pszCurDir)
			return TRUE;
		lstrcpy(pszCurDir, L"C:\\");
	}

	bool bSilent = (wcsstr(pszCommand, L"-new_console") != NULL);
	
	if (!bSilent)
		InfoW2800->PanelControl(INVALID_HANDLE_VALUE,FCTL_GETUSERSCREEN,0,0);
		
	RunExternalProgramW(pszCommand, pszCurDir, bSilent);
	
	if (!bSilent)
		InfoW2800->PanelControl(INVALID_HANDLE_VALUE,FCTL_SETUSERSCREEN,0,0);
	InfoW2800->AdvControl(&guid_ConEmu,ACTL_REDRAWALL,0, 0);
	free(pszCurDir); //pszCurDir = NULL;
	if (pszExpand && (pszExpand != strTemp)) free(pszExpand);
	return TRUE;
}


bool ProcessCommandLineW2800(wchar_t* pszCommand)
{
	if (!InfoW2800 || !FSFW2800) return false;

	if (FSFW2800->LStrnicmp(pszCommand, L"run:", 4)==0)
	{
		RunExternalProgramW2800(pszCommand+4); //-V112
		return true;
	}

	return false;
}

//static void FarPanel2CePanel(PanelInfo* pFar, CEFAR_SHORT_PANEL_INFO* pCE)
//{
//	pCE->PanelType = pFar->PanelType;
//	pCE->Plugin = ((pFar->Flags & PFLAGS_PLUGIN) == PFLAGS_PLUGIN);
//	pCE->PanelRect = pFar->PanelRect;
//	pCE->ItemsNumber = pFar->ItemsNumber;
//	pCE->SelectedItemsNumber = pFar->SelectedItemsNumber;
//	pCE->CurrentItem = pFar->CurrentItem;
//	pCE->TopPanelItem = pFar->TopPanelItem;
//	pCE->Visible = ((pFar->Flags & PFLAGS_VISIBLE) == PFLAGS_VISIBLE);
//	pCE->Focus = ((pFar->Flags & PFLAGS_FOCUS) == PFLAGS_FOCUS);
//	pCE->ViewMode = pFar->ViewMode;
//	pCE->ShortNames = ((pFar->Flags & PFLAGS_ALTERNATIVENAMES) == PFLAGS_ALTERNATIVENAMES);
//	pCE->SortMode = pFar->SortMode;
//	pCE->Flags = pFar->Flags;
//}

void LoadFarColorsW2800(BYTE (&nFarColors)[col_LastIndex])
{
	INT_PTR nColorSize = InfoW2800->AdvControl(&guid_ConEmu, ACTL_GETARRAYCOLOR, 0, NULL);
	FarColor* pColors = (FarColor*)calloc(nColorSize, sizeof(*pColors));
	if (pColors)
		nColorSize = InfoW2800->AdvControl(&guid_ConEmu, ACTL_GETARRAYCOLOR, (int)nColorSize, pColors);
	WARNING("��������� ����� 4��� ������");
	if (pColors && nColorSize > 0)
	{
#ifdef _DEBUG
		INT_PTR nDefColorSize = COL_LASTPALETTECOLOR;
		_ASSERTE(nColorSize==nDefColorSize);
#endif
		nFarColors[col_PanelText] = FarColor_3_2(pColors[COL_PANELTEXT]);
		nFarColors[col_PanelSelectedCursor] = FarColor_3_2(pColors[COL_PANELSELECTEDCURSOR]);
		nFarColors[col_PanelSelectedText] = FarColor_3_2(pColors[COL_PANELSELECTEDTEXT]);
		nFarColors[col_PanelCursor] = FarColor_3_2(pColors[COL_PANELCURSOR]);
		nFarColors[col_PanelColumnTitle] = FarColor_3_2(pColors[COL_PANELCOLUMNTITLE]);
		nFarColors[col_PanelBox] = FarColor_3_2(pColors[COL_PANELBOX]);
		nFarColors[col_HMenuText] = FarColor_3_2(pColors[COL_HMENUTEXT]);
		nFarColors[col_WarnDialogBox] = FarColor_3_2(pColors[COL_WARNDIALOGBOX]);
		nFarColors[col_DialogBox] = FarColor_3_2(pColors[COL_DIALOGBOX]);
		nFarColors[col_CommandLineUserScreen] = FarColor_3_2(pColors[COL_COMMANDLINEUSERSCREEN]);
		nFarColors[col_PanelScreensNumber] = FarColor_3_2(pColors[COL_PANELSCREENSNUMBER]);
		nFarColors[col_KeyBarNum] = FarColor_3_2(pColors[COL_KEYBARNUM]);
	}
	else
	{
		_ASSERTE(pColors && nColorSize > 0);
		memset(nFarColors, 7, countof(nFarColors)*sizeof(*nFarColors));
	}
	SafeFree(pColors);
}

static int GetFarSetting(HANDLE h, size_t Root, LPCWSTR Name)
{
	int nValue = 0;
	FarSettingsItem fsi = {sizeof(fsi), Root, Name};
	if (InfoW2800->SettingsControl(h, SCTL_GET, 0, &fsi))
	{
		_ASSERTE(fsi.Type == FST_QWORD);
		nValue = (fsi.Number != 0);
	}
	else
	{
		_ASSERTE("InfoW2800->SettingsControl failed" && 0);
	}
	return nValue;
}

static void LoadFarSettingsW2800(CEFarInterfaceSettings* pInterface, CEFarPanelSettings* pPanel)
{
	_ASSERTE(GetCurrentThreadId() == gnMainThreadId);
	GUID FarGuid = {};
	FarSettingsCreate sc = {sizeof(FarSettingsCreate), FarGuid, INVALID_HANDLE_VALUE};
	if (InfoW2800->SettingsControl(INVALID_HANDLE_VALUE, SCTL_CREATE, 0, &sc))
	{
		if (pInterface)
		{
			memset(pInterface, 0, sizeof(*pInterface));
			pInterface->AlwaysShowMenuBar = GetFarSetting(sc.Handle, FSSF_INTERFACE, L"ShowMenuBar");
			pInterface->ShowKeyBar = GetFarSetting(sc.Handle, FSSF_SCREEN, L"KeyBar");
		}

		if (pPanel)
		{
			memset(pPanel, 0, sizeof(*pPanel));
			pPanel->ShowColumnTitles = GetFarSetting(sc.Handle, FSSF_PANELLAYOUT, L"ColumnTitles");
			pPanel->ShowStatusLine = GetFarSetting(sc.Handle, FSSF_PANELLAYOUT, L"StatusLine");
			pPanel->ShowSortModeLetter = GetFarSetting(sc.Handle, FSSF_PANELLAYOUT, L"SortMode");
		}

		InfoW2800->SettingsControl(sc.Handle, SCTL_FREE, 0, 0);
	}
}

BOOL ReloadFarInfoW2800(/*BOOL abFull*/)
{
	if (!InfoW2800 || !FSFW2800) return FALSE;

	if (!gpFarInfo)
	{
		_ASSERTE(gpFarInfo!=NULL);
		return FALSE;
	}

	// ��������� gpFarInfo->
	//BYTE nFarColors[0x100]; // ������ ������ ����
	//DWORD nFarInterfaceSettings;
	//DWORD nFarPanelSettings;
	//DWORD nFarConfirmationSettings;
	//BOOL  bFarPanelAllowed, bFarLeftPanel, bFarRightPanel;   // FCTL_CHECKPANELSEXIST, FCTL_GETPANELSHORTINFO,...
	//CEFAR_SHORT_PANEL_INFO FarLeftPanel, FarRightPanel;
	DWORD ldwConsoleMode = 0;	GetConsoleMode(/*ghConIn*/GetStdHandle(STD_INPUT_HANDLE), &ldwConsoleMode);
#ifdef _DEBUG
	static DWORD ldwDbgMode = 0;

	if (IsDebuggerPresent())
	{
		if (ldwDbgMode != ldwConsoleMode)
		{
			wchar_t szDbg[128]; _wsprintf(szDbg, SKIPLEN(countof(szDbg)) L"Far.ConEmuW: ConsoleMode(STD_INPUT_HANDLE)=0x%08X\n", ldwConsoleMode);
			OutputDebugStringW(szDbg);
			ldwDbgMode = ldwConsoleMode;
		}
	}

#endif
	gpFarInfo->nFarConsoleMode = ldwConsoleMode;

	LoadFarColorsW2800(gpFarInfo->nFarColors);

	//_ASSERTE(FPS_SHOWCOLUMNTITLES==0x20 && FPS_SHOWSTATUSLINE==0x40); //-V112
	//gpFarInfo->FarInterfaceSettings =
	//    (DWORD)InfoW2800->AdvControl(&guid_ConEmu, ACTL_GETINTERFACESETTINGS, 0, 0);
	//gpFarInfo->nFarPanelSettings =
	//    (DWORD)InfoW2800->AdvControl(&guid_ConEmu, ACTL_GETPANELSETTINGS, 0, 0);
	//gpFarInfo->nFarConfirmationSettings =
	//    (DWORD)InfoW2800->AdvControl(&guid_ConEmu, ACTL_GETCONFIRMATIONS, 0, 0);

	LoadFarSettingsW2800(&gpFarInfo->FarInterfaceSettings, &gpFarInfo->FarPanelSettings);

	gpFarInfo->bMacroActive = IsMacroActiveW2800();
	INT_PTR nArea = InfoW2800->MacroControl(&guid_ConEmu, MCTL_GETAREA, 0, 0);
	switch(nArea)
	{
		case MACROAREA_SHELL:
		case MACROAREA_INFOPANEL:
		case MACROAREA_QVIEWPANEL:
		case MACROAREA_TREEPANEL:
		case MACROAREA_SEARCH:
			gpFarInfo->nMacroArea = fma_Panels;
			break;
		case MACROAREA_VIEWER:
			gpFarInfo->nMacroArea = fma_Viewer;
			break;
		case MACROAREA_EDITOR:
			gpFarInfo->nMacroArea = fma_Editor;
			break;
		case MACROAREA_DIALOG:
		case MACROAREA_DISKS:
		case MACROAREA_FINDFOLDER:
		case MACROAREA_SHELLAUTOCOMPLETION:
		case MACROAREA_DIALOGAUTOCOMPLETION:
		case MACROAREA_MAINMENU:
		case MACROAREA_MENU:
		case MACROAREA_USERMENU:
			gpFarInfo->nMacroArea = fma_Dialog;
			break;
		default:
			gpFarInfo->nMacroArea = fma_Unknown;
	}
	    
	gpFarInfo->bFarPanelAllowed = InfoW2800->PanelControl(PANEL_NONE, FCTL_CHECKPANELSEXIST, 0, 0)!=0;
	gpFarInfo->bFarPanelInfoFilled = FALSE;
	gpFarInfo->bFarLeftPanel = FALSE;
	gpFarInfo->bFarRightPanel = FALSE;
	// -- ����, �� ��������� ������ � FAR ��� ����������� �������� ���������� � �������
	//if (FALSE == (gpFarInfo->bFarPanelAllowed)) {
	//	gpConMapInfo->bFarLeftPanel = FALSE;
	//	gpConMapInfo->bFarRightPanel = FALSE;
	//} else {
	//	PanelInfo piA = {}, piP = {};
	//	BOOL lbActive  = InfoW2800->PanelControl(PANEL_ACTIVE, FCTL_GETPANELINFO, 0, &piA);
	//	BOOL lbPassive = InfoW2800->PanelControl(PANEL_PASSIVE, FCTL_GETPANELINFO, 0, &piP);
	//	if (!lbActive && !lbPassive)
	//	{
	//		gpConMapInfo->bFarLeftPanel = FALSE;
	//		gpConMapInfo->bFarRightPanel = FALSE;
	//	} else {
	//		PanelInfo *ppiL = NULL;
	//		PanelInfo *ppiR = NULL;
	//		if (lbActive) {
	//			if (piA.Flags & PFLAGS_PANELLEFT) ppiL = &piA; else ppiR = &piA;
	//		}
	//		if (lbPassive) {
	//			if (piP.Flags & PFLAGS_PANELLEFT) ppiL = &piP; else ppiR = &piP;
	//		}
	//		gpConMapInfo->bFarLeftPanel = ppiL!=NULL;
	//		gpConMapInfo->bFarRightPanel = ppiR!=NULL;
	//		if (ppiL) FarPanel2CePanel(ppiL, &(gpConMapInfo->FarLeftPanel));
	//		if (ppiR) FarPanel2CePanel(ppiR, &(gpConMapInfo->FarRightPanel));
	//	}
	//}
	return TRUE;
}

void ExecuteQuitFarW2800()
{
	if (!InfoW2800 || !InfoW2800->AdvControl)
	{
		PostMessage(FarHwnd, WM_CLOSE, 0, 0);
		return;
	}

	InfoW2800->AdvControl(&guid_ConEmu, ACTL_QUIT, 0, NULL);
}

BOOL CheckBufferEnabledW2800()
{
	if (!InfoW2800 || !InfoW2800->AdvControl)
		return FALSE;

	static int siEnabled = 0;

	// ����� �������� ��������� ������ ���� ���.
	// �.�. ����� ����� ���� ������� �������, � ��� ��� ���-��� �����.
	if (siEnabled)
	{
		return (siEnabled == 1);
	}

	SMALL_RECT rcFar = {0};

	if (InfoW2800->AdvControl(&guid_ConEmu, ACTL_GETFARRECT, 0, &rcFar))
	{
		if (rcFar.Top > 0 && rcFar.Bottom > rcFar.Top)
		{
			siEnabled = 1;
			return TRUE;
		}
	}

	siEnabled = -1;
	return FALSE;
}

static void CopyPanelInfoW(PanelInfo* pInfo, PaintBackgroundArg::BkPanelInfo* pBk)
{
	pBk->bVisible = ((pInfo->Flags & PFLAGS_VISIBLE) == PFLAGS_VISIBLE);
	pBk->bFocused = ((pInfo->Flags & PFLAGS_FOCUS) == PFLAGS_FOCUS);
	pBk->bPlugin = ((pInfo->Flags & PFLAGS_PLUGIN) == PFLAGS_PLUGIN);
	pBk->nPanelType = (int)pInfo->PanelType;
	HANDLE hPanel = (pBk->bFocused) ? PANEL_ACTIVE : PANEL_PASSIVE;
	wchar_t* pszDir = GetPanelDir(hPanel);
	//InfoW2800->PanelControl(hPanel, FCTL_GETPANELDIR /* == FCTL_GETPANELDIR == 25*/, BkPanelInfo_CurDirMax, pBk->szCurDir);
	lstrcpyn(pBk->szCurDir, pszDir ? pszDir : L"", BkPanelInfo_CurDirMax);
	SafeFree(pszDir);

	if (pBk->bPlugin)
	{
		pBk->szFormat[0] = 0;
		INT_PTR iFRc = InfoW2800->PanelControl(hPanel, FCTL_GETPANELFORMAT, BkPanelInfo_FormatMax, pBk->szFormat);
		if (iFRc < 0 || iFRc > BkPanelInfo_FormatMax || !*pBk->szFormat)
		{
			InfoW2800->PanelControl(hPanel, FCTL_GETPANELPREFIX, BkPanelInfo_FormatMax, pBk->szFormat);
		}
		
		InfoW2800->PanelControl(hPanel, FCTL_GETPANELHOSTFILE, BkPanelInfo_HostFileMax, pBk->szHostFile);
	}
	else
	{
		pBk->szFormat[0] = 0;
		pBk->szHostFile[0] = 0;
	}

	pBk->rcPanelRect = pInfo->PanelRect;
}

void FillUpdateBackgroundW2800(struct PaintBackgroundArg* pFar)
{
	if (!InfoW2800 || !InfoW2800->AdvControl)
		return;

	LoadFarColorsW2800(pFar->nFarColors);

	LoadFarSettingsW2800(&pFar->FarInterfaceSettings, &pFar->FarPanelSettings);

	pFar->bPanelsAllowed = (0 != InfoW2800->PanelControl(INVALID_HANDLE_VALUE, FCTL_CHECKPANELSEXIST, 0, 0));

	if (pFar->bPanelsAllowed)
	{
		PanelInfo pasv = {sizeof(pasv)}, actv = {sizeof(actv)};
		InfoW2800->PanelControl(PANEL_ACTIVE, FCTL_GETPANELINFO, 0, &actv);
		InfoW2800->PanelControl(PANEL_PASSIVE, FCTL_GETPANELINFO, 0, &pasv);
		PanelInfo* pLeft = (actv.Flags & PFLAGS_PANELLEFT) ? &actv : &pasv;
		PanelInfo* pRight = (actv.Flags & PFLAGS_PANELLEFT) ? &pasv : &actv;
		CopyPanelInfoW(pLeft, &pFar->LeftPanel);
		CopyPanelInfoW(pRight, &pFar->RightPanel);
	}

	HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO scbi = {};
	GetConsoleScreenBufferInfo(hCon, &scbi);

	if (CheckBufferEnabledW2800())
	{
		SMALL_RECT rc = {0};
		InfoW2800->AdvControl(&guid_ConEmu, ACTL_GETFARRECT, 0, &rc);
		pFar->rcConWorkspace.left = rc.Left;
		pFar->rcConWorkspace.top = rc.Top;
		pFar->rcConWorkspace.right = rc.Right;
		pFar->rcConWorkspace.bottom = rc.Bottom;
	}
	else
	{
		pFar->rcConWorkspace.left = pFar->rcConWorkspace.top = 0;
		pFar->rcConWorkspace.right = scbi.dwSize.X - 1;
		pFar->rcConWorkspace.bottom = scbi.dwSize.Y - 1;
		//pFar->conSize = scbi.dwSize;
	}

	pFar->conCursor = scbi.dwCursorPosition;
	CONSOLE_CURSOR_INFO crsr = {0};
	GetConsoleCursorInfo(hCon, &crsr);

	if (!crsr.bVisible || crsr.dwSize == 0)
	{
		pFar->conCursor.X = pFar->conCursor.Y = -1;
	}
}

int GetActiveWindowTypeW2800()
{
	if (!InfoW2800 || !InfoW2800->AdvControl)
		return -1;

	//_ASSERTE(GetCurrentThreadId() == gnMainThreadId); -- ��� - ThreadSafe

	INT_PTR nArea = InfoW2800->MacroControl(&guid_ConEmu, MCTL_GETAREA, 0, 0);

	switch(nArea)
	{
		case MACROAREA_SHELL:
		case MACROAREA_INFOPANEL:
		case MACROAREA_QVIEWPANEL:
		case MACROAREA_TREEPANEL:
			return WTYPE_PANELS;
		case MACROAREA_VIEWER:
			return WTYPE_VIEWER;
		case MACROAREA_EDITOR:
			return WTYPE_EDITOR;
		case MACROAREA_DIALOG:
		case MACROAREA_SEARCH:
		case MACROAREA_DISKS:
		case MACROAREA_FINDFOLDER:
		case MACROAREA_SHELLAUTOCOMPLETION:
		case MACROAREA_DIALOGAUTOCOMPLETION:
			return WTYPE_DIALOG;
		case MACROAREA_HELP:
			return WTYPE_HELP;
		case MACROAREA_MAINMENU:
		case MACROAREA_MENU:
		case MACROAREA_USERMENU:
			return WTYPE_VMENU;
		case MACROAREA_OTHER: // Grabber
			return -1;
	}

	// ���� �� ������� �� ������, ��� ������������ ������ ���� ������ � switch
	_ASSERTE(nArea==MACROAREA_SHELL);
	return -1;
}

//static LONG_PTR WINAPI CallGuiMacroDlg(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2);
//HANDLE CreateGuiMacroDlg(int height, FarDialogItem *items, int nCount)
//{
//	static const GUID ConEmuCallGuiMacro = { /* 21f61504-b40f-45e3-9d27-84db16bc9c22 */
//		    0x21f61504,
//		    0xb40f,
//		    0x45e3,
//		    {0x9d, 0x27, 0x84, 0xdb, 0x16, 0xbc, 0x9c, 0x22}
//		  };
//
//	HANDLE hDlg = InfoW2800->DialogInitW2800(&guid_ConEmu, ConEmuCallGuiMacro,
//									-1, -1, 76, height,
//									NULL/*L"Configure"*/, items, nCount, 
//									0, 0/*Flags*/, (FARWINDOWPROC)CallGuiMacroDlg, 0);
//	return hDlg;
//}

#define FAR_UNICODE 1867
#include "Dialogs.h"
void GuiMacroDlgW2800()
{
	CallGuiMacroProc();
}

//GUID ConEmuGuid = { /* 374471b3-db1e-4276-bf9e-c486fcce4553 */
//						    0x374471b3,
//						    0xdb1e,
//						    0x4276,
//						    {0xbf, 0x9e, 0xc4, 0x86, 0xfc, 0xce, 0x45, 0x53}
//				  };

void WINAPI GetGlobalInfoW(struct GlobalInfo *Info)
{
	//static wchar_t szTitle[16]; _wcscpy_c(szTitle, L"ConEmu");
	//static wchar_t szDescr[64]; _wcscpy_c(szTitle, L"ConEmu support for Far Manager");
	//static wchar_t szAuthr[64]; _wcscpy_c(szTitle, L"ConEmu.Maximus5@gmail.com");
	
	//Info->StructSize = sizeof(GlobalInfo);
	_ASSERTE(Info->StructSize >= sizeof(GlobalInfo));
	if (gFarVersion.dwBuild >= FAR_Y2_VER)
		Info->MinFarVersion = FARMANAGERVERSION;
	else
		Info->MinFarVersion = MAKEFARVERSION(FARMANAGERVERSION_MAJOR,FARMANAGERVERSION_MINOR, FARMANAGERVERSION_REVISION, 2578, FARMANAGERVERSION_STAGE);

	// Build: YYMMDDX (YY - ��� ����� ����, MM - �����, DD - ����, X - 0 � ����-����� ���������)
	Info->Version = MAKEFARVERSION(MVV_1,MVV_2,MVV_3,((MVV_1 % 100)*100000) + (MVV_2*1000) + (MVV_3*10) + (MVV_4 % 10),VS_RELEASE);
	
	Info->Guid = guid_ConEmu;
	Info->Title = L"ConEmu";
	Info->Description = L"ConEmu support for Far Manager";
	Info->Author = L"ConEmu.Maximus5@gmail.com";
}

extern BOOL gbInfoW_OK;
HANDLE WINAPI OpenW2800(const void* apInfo)
{
	const struct OpenInfo *Info = (const struct OpenInfo*)apInfo;

	if (!gbInfoW_OK)
		return NULL;
	
	INT_PTR Item = Info->Data;

	if ((Info->OpenFrom & OPEN_FROM_MASK) == OPEN_FROMMACRO)
	{
		Item = 0; // ����� �����
		OpenMacroInfo* p = (OpenMacroInfo*)Info->Data;
		if (p->StructSize >= sizeof(*p))
		{
			if (p->Count > 0)
			{
				switch (p->Values[0].Type)
				{
				case FMVT_INTEGER:
					Item = (INT_PTR)p->Values[0].Integer; break;
				// Far 3 Lua macros uses Double instead of Int :(
				case FMVT_DOUBLE:
					Item = (INT_PTR)p->Values[0].Double; break;
				case FMVT_STRING:
					_ASSERTE(p->Values[0].String!=NULL);
					Item = (INT_PTR)p->Values[0].String; break;
				default:
					_ASSERTE(p->Values[0].Type==FMVT_INTEGER || p->Values[0].Type==FMVT_STRING);
				}
			}
		}
		else
		{
			_ASSERTE(p->StructSize >= sizeof(*p));
		}
	}
	else if (Info->OpenFrom == OPEN_COMMANDLINE)
	{
		OpenCommandLineInfo* p = (OpenCommandLineInfo*)Info->Data;
		Item = (INT_PTR)p->CommandLine;
	}

	HANDLE h = OpenPluginWcmn(Info->OpenFrom, Item, (Info->OpenFrom == OPEN_FROMMACRO));
	if ((Info->OpenFrom & OPEN_FROM_MASK) == OPEN_FROMMACRO)
	{
		h = (HANDLE)(h != NULL);
	}
	else if ((h == INVALID_HANDLE_VALUE) || (h == (HANDLE)-2))
	{
		if ((Info->OpenFrom & OPEN_FROM_MASK) == OPEN_ANALYSE)
			h = PANEL_STOP;
		else
			h = NULL;
	}

	return h;
}

INT_PTR WINAPI ProcessConsoleInputW2800(void* apInfo)
{
	struct ProcessConsoleInputInfo *Info = (struct ProcessConsoleInputInfo*)apInfo;

#if 0
	// ����� ����� ���� "���������" �������� � Far3 � ��� �����
	BOOL bMainThread = TRUE; // ��� ����� ����� API - ������ MainThread
	BOOL lbRc = FALSE;
	HANDLE hConsoleInput = GetStdHandle(STD_INPUT_HANDLE);
	const INPUT_RECORD* lpBuffer = Info->Rec;
	DWORD nLength = 1; LPDWORD lpNumberOfEventsRead = &nLength;
	SETARGS4(&lbRc,hConsoleInput,lpBuffer,nLength,lpNumberOfEventsRead);

	if (!OnConsoleReadInputWork(&args) || (nLength == 0))
		return 1;

	OnConsoleReadInputPost(&args);
#endif

	return 0;
}
