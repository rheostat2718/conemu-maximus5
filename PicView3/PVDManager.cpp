
/**************************************************************************
Copyright (c) 2009 Skakov Pavel
Copyright (c) 2010 Maimus5
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
**************************************************************************/

/*
������ ����� ������������ 

����� ������������� �������� (�� ��� ����� ���� �����������, preload, ����� ����� �������)
FD_JUMP, FD_JUMP_NEXT, FD_HOME_END (���������� �� ����� ������� ������������)

��������� �������������.
FD_REFRESH
�������� ��������
1. ����� ��������
2. ��������� �������� ����������� ����� ����

*/


#define PICVIEWDUMP


#include "PVDManager.h"
#include "Image.h"
#include "ImageInfo.h"
#include "DecoderHandle.h"
#include "PictureView_Lang.h"
#include "DecodeItem.h"
#include "DecoderHandle.h"
#include "DisplayHandle.h"
#include "PVDModuleBase.h"
#include "FileMap.h"


CPVDManager g_Manager;
LPCSTR szPVDManager = "CPVDManager";

std::vector<CModuleInfo*> CPVDManager::Plugins;
std::vector<CModuleInfo*> CPVDManager::Decoders;
std::vector<CModuleInfo*> CPVDManager::Displays;
bool CPVDManager::bCancelDecoding = false;
CModuleInfo* CPVDManager::pDefaultDisplay = NULL; // ��� ������� � ��������� ������� - ���� "Display module"
CModuleInfo* CPVDManager::pActiveDisplay = NULL;  // � ��� ��, ����� ��� ���� ����� ������ (����� ���� NULL)
std::vector<wchar_t*> CPVDManager::sWholeLog;
bool CPVDManager::bWholeLogChanged = false;

CPVDManager::CPVDManager()
{
#ifdef MHEAP_DEFINED
	xf_initialize();
#endif
	memset(&m_DecoderQueue, 0, sizeof(m_DecoderQueue));
	InitializeCriticalSection(&csDecoderQueue);
	mh_Thread = NULL; mn_ThreadId = 0; mn_AliveTick = 0;
	mh_Request = mh_Alive = NULL;
}

CPVDManager::~CPVDManager()
{
	DeleteCriticalSection(&csDecoderQueue);
}

// ������� true, ���� p2 ������ ���� ���� p1
bool CPVDManager::PluginCompare(CModuleInfo* p1, CModuleInfo* p2)
{
	//-- ����� ���� ��������, ��� ��� �� �������� // ������ ������� ��������� ������ - ����� ����
	//if (!p1->pPlugin && p2->pPlugin)
	//	return true; // p1 � ����� ��� ��� �����������
	if (!(p1->nCurrentFlags & PVD_IP_SUPPORTEDTYPES) && (p2->nCurrentFlags & PVD_IP_SUPPORTEDTYPES))
		return true; // p1 � ����� ��� ��� ����������� ���
	if (!(p1->nCurrentFlags & PVD_IP_DECODE) && (p2->nCurrentFlags & PVD_IP_DECODE))
		return true; // p2 ����� ��� �������
	if ((p1->nCurrentFlags & PVD_IP_DECODE) && !(p2->nCurrentFlags & PVD_IP_DECODE))
		return false; // p1 ����� ��� �������
	// ����. � ����� ����� ����� ���� ��������� �����
	//if ((p1->nCurrentFlags & PVD_IP_SUPPORTEDTYPES) > (p2->nCurrentFlags & PVD_IP_SUPPORTEDTYPES))
	//	return true; // ����� ��� ������ (p2), � �������� ������ �� ����
	if (p1->Priority() < p2->Priority())
		return true; // ����� ��� ������ (p2), � �������� ������ ���������
	else if (p1->Priority() > p2->Priority())
		return false; // ����� ��� ������ (p1), � �������� ������ ���������
	if (p1->pModuleFileName && p2->pModuleFileName && lstrcmpi(p1->pModuleFileName, p2->pModuleFileName) > 0)
		return true; // ����� ��������� �� ����� ������
	return false;
}

void CPVDManager::LoadPlugins2()
{
	MCHKHEAP;

	ScanFolder(g_SelfPath);

	MCHKHEAP;

	SortPlugins2();

	#ifdef _DEBUG
	SortPlugins2();
	#endif

	MCHKHEAP;
}

void CPVDManager::ScanFolder(const wchar_t* asFrom)
{
	wchar_t *pSearchPath = ConcatPath(asFrom, L"*");
	if (!pSearchPath) {
		_ASSERTE(pSearchPath);
		return;
	}

	WIN32_FIND_DATAW fnd;
	HANDLE hFind = FindFirstFileW(pSearchPath, &fnd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if ((fnd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) {
				if (fnd.cFileName[0] == L'.' 
					&& (fnd.cFileName[1] == 0 || (fnd.cFileName[1] == L'.' && fnd.cFileName[2] == 0))
					)
					continue;

				wchar_t *pSubFolder = ConcatPath(asFrom, fnd.cFileName);
				if (pSubFolder) {
					ScanFolder(pSubFolder);
					free(pSubFolder);
				} else {
					_ASSERTE(pSubFolder);
				}
				continue;
			}
			
			LoadPlugin(asFrom, fnd, false/*abShowErrors*/);

		} while (FindNextFileW(hFind, &fnd));
		FindClose(hFind);
	}

	free(pSearchPath);
}

CModuleInfo* CPVDManager::FindPlugin(const wchar_t* asFile, bool abShowErrors)
{
	if (!asFile || !*asFile) {
		return NULL;
	}
	
	int nLen = lstrlen(asFile);
	std::vector<CModuleInfo*>::iterator iter;
	
	// ������� �� ������� ����
	if (wcschr(asFile, L'\\')) {
		if (_wcsnicmp(g_SelfPath, asFile, lstrlen(g_SelfPath))) {
			// ������ ���� �� ��������� � �������� � �������
			return NULL;
		}
		
		// ���������� ������ ���� � �������
		for (iter = Plugins.begin(); iter != Plugins.end();iter++) {
			CModuleInfo* plug = *iter;
			
			if (!lstrcmpi(plug->pModulePath, asFile))
				return plug; // �����
		}
	} else {
		// �������� ����� ������ �� �����
		for (iter = Plugins.begin(); iter != Plugins.end();iter++) {
			CModuleInfo* plug = *iter;
			
			if (!lstrcmpi(plug->pModuleFileName, asFile))
				return plug;
		}
	}
	
	return NULL; // �� �����
}

CModuleInfo* CPVDManager::LoadPlugin(const wchar_t* asFrom, WIN32_FIND_DATAW& fnd, bool abShowErrors)
{
	int nFileNameLen = lstrlen(fnd.cFileName);
	if (nFileNameLen <= 4)
		return NULL;
	if (lstrcmpi(fnd.cFileName+nFileNameLen-4, L".pvd"))
		return NULL; // �� '*.pvd' ����

	CModuleInfo *plug = new CModuleInfo(szPVDManager);
	_ASSERTE(plug!=NULL);
	//memset(plug, 0, sizeof(CModuleInfo));
	plug->pModulePath = ConcatPath(asFrom, fnd.cFileName);
	plug->pModuleFileName = wcsrchr(plug->pModulePath, L'\\');
	if (plug->pModuleFileName) plug->pModuleFileName++; else plug->pModuleFileName = plug->pModulePath;
	plug->pRegPath = ConcatPath(g_RootKey, fnd.cFileName);
	plug->ftModified = fnd.ftLastWriteTime;

	// ��������� �� ������� ������, ��������� ���� ���������
	HKEY hkey = NULL; DWORD dwDisp = 0;
	bool lbPluginChanged = true;
	if (RegCreateKeyEx(HKEY_CURRENT_USER, plug->pRegPath, 0, 0, 0, KEY_ALL_ACCESS, 0, &hkey, &dwDisp)) {
		_ASSERTE(FALSE);
	} else {
		DWORD nVerSize = sizeof(DWORD), nVersion = 0, nModSize = sizeof(FILETIME), nType,
			nFlags = 0, nFlagSize = sizeof(DWORD), nCurrentFlags = 0, nCurrentFlagSize = sizeof(DWORD);
		FILETIME ftRegModified;
		if (!RegQueryValueEx(hkey, L"Version", 0, &(nType=REG_DWORD), (LPBYTE)&nVersion, &nVerSize)
			&& !RegQueryValueEx(hkey, L"Modified", 0, &(nType=REG_BINARY), (LPBYTE)&ftRegModified, &nModSize)
			&& !RegQueryValueEx(hkey, L"PluginFlags", 0, 0, (LPBYTE)&nFlags, &nFlagSize)
			&& !RegQueryValueEx(hkey, L"PluginCurrentFlags", 0, 0, (LPBYTE)&nCurrentFlags, &nCurrentFlagSize)
			)
		{
			// ���� ������ �� ��������� � ��������� ��������
			if (ftRegModified.dwHighDateTime == plug->ftModified.dwHighDateTime
				&& ftRegModified.dwLowDateTime == plug->ftModified.dwLowDateTime)
			{
				plug->nVersion = nVersion;
				plug->nCurrentFlags = nCurrentFlags;
				plug->nFlags = nFlags;
				plug->bPrepared = (nFlags & PVD_IP_SUPPORTEDTYPES) != 0;
				lbPluginChanged = false;
			}
		}
		RegCloseKey(hkey); hkey = NULL;
	}

	plug->Load(lbPluginChanged);

	plug->hPlugin = NULL; // ��������� ���������� ���������� CPVDModuleBase
	Plugins.push_back(plug);
	
	return plug;
}

bool CPVDManager::CreateVersion1(CModuleInfo* plug, bool lbForceWriteReg)
{
	_ASSERTE(plug->pPlugin == NULL);
	
	plug->nVersion = 1;
	plug->bPrepared = FALSE;

	plug->pPlugin = new CPVDModuleVer1(plug);

	if (!plug->pPlugin)
	{
		_ASSERTE(plug->pPlugin!=NULL);
	}
	else
	{
		plug->nPriority = plug->pPlugin->nDefPriority = 0;
		//_ASSERTE(!plug->pActive && !plug->pInactive && !plug->pForbidden);
		// ���������� ����� �������� �� ����������� ���� (unload:, reload:)
		SafeFree(plug->pActive); SafeFree(plug->pInactive); SafeFree(plug->pForbidden);

		if (plug->pPlugin->InitPrepare(plug->ftModified, lbForceWriteReg))
		{
			_ASSERTE(plug->pPlugin->nFlags == PVD_IP_DECODE);
			_ASSERTE(plug->nCurrentFlags == PVD_IP_DECODE);
			_ASSERTE(plug->pPlugin->nDefPriority > 0);
			// ���� ��������� ������ �� ���� �� ����������� � ������� - � plug->nPriority ����� ���� 0
			if (!plug->nPriority) plug->nPriority = max(1,plug->pPlugin->nDefPriority);
			_ASSERTE(plug->pPlugin->pDefActive || plug->pActive);
			if (!plug->pActive)
				plug->pActive = _wcsdup(plug->pPlugin->pDefActive ? plug->pPlugin->pDefActive : L"");
			if (!plug->pInactive)
				plug->pInactive = _wcsdup(plug->pPlugin->pDefInactive ? plug->pPlugin->pDefInactive : L"");
			if (!plug->pForbidden)
				plug->pForbidden = _wcsdup(plug->pPlugin->pDefForbidden ? plug->pPlugin->pDefForbidden : L"");
			//plug->nFlags = plug->pPlugin->nFlags;
			//plug->iPriority = plug->pPlugin->iPriority;
			plug->bPrepared = TRUE; // OK
		}
		else
		{
			delete plug->pPlugin;
			plug->pPlugin = NULL;
		}
	}

	return plug->bPrepared;
}

bool CPVDManager::CreateVersion2(CModuleInfo* plug, bool lbForceWriteReg)
{
	_ASSERTE(plug->pPlugin == NULL);
	
	plug->nVersion = 2;
	plug->bPrepared = FALSE;

	plug->pPlugin = new CPVDModuleVer2(plug);

	if (!plug->pPlugin) {
		_ASSERTE(plug->pPlugin!=NULL);
	} else {
		plug->nPriority = plug->pPlugin->nDefPriority = 0;
		//_ASSERTE(!plug->pActive && !plug->pInactive && !plug->pForbidden);
		// ���������� ����� �������� �� ����������� ���� (unload:, reload:)
		SafeFree(plug->pActive); SafeFree(plug->pInactive); SafeFree(plug->pForbidden);

		if (plug->pPlugin->InitPrepare(plug->ftModified, lbForceWriteReg)) {
			_ASSERTE(plug->pPlugin->nFlags != 0);
			_ASSERTE(plug->nCurrentFlags != 0);
			_ASSERTE(plug->pPlugin->nDefPriority > 0);
			// ���� ��������� ������ �� ���� �� ����������� � ������� - � plug->nPriority ����� ���� 0
			if (!plug->nPriority) plug->nPriority = max(1,plug->pPlugin->nDefPriority);
			if (plug->nCurrentFlags & PVD_IP_DECODE) {
				_ASSERTE(plug->pPlugin->pDefActive || plug->pActive);
				if (!plug->pActive)
					plug->pActive = _wcsdup(plug->pPlugin->pDefActive ? plug->pPlugin->pDefActive : L"");
				if (!plug->pInactive)
					plug->pInactive = _wcsdup(plug->pPlugin->pDefInactive ? plug->pPlugin->pDefInactive : L"");
				if (!plug->pForbidden)
					plug->pForbidden = _wcsdup(plug->pPlugin->pDefForbidden ? plug->pPlugin->pDefForbidden : L"");
			}
			//plug->nFlags = plug->pPlugin->nFlags;
			//plug->iPriority = plug->pPlugin->iPriority;
			plug->bPrepared = TRUE; // OK
		} else {
			delete plug->pPlugin;
			plug->pPlugin = NULL;
		}
	}

	return plug->bPrepared;
}

bool CPVDManager::IsSupportedExtension(const wchar_t *pFileName, LPVOID pFileData, size_t nFileDataSize)
{
	WARNING("�������� ��������� �����! �� ���� ����� '��������' ����� Shell ��������");

	CModuleInfo* p = NULL;
	UINT nCount = (UINT)Decoders.size();

	const wchar_t *pExt = NULL;
	const wchar_t *pS = wcsrchr(pFileName, '\\');
	if (!pS) pS = pFileName;
	if (((pExt = wcsrchr(pFileName, '.')) != NULL) && (pExt >= pS))
		pExt++;
	else
		pExt = NULL;

	// ��������, ����� ������ ����� �� ��������, � � "������" ������� ���������
	BYTE Data[128]; DWORD nDataRead = 0;
	bool bFileFailed = false;
	
	for (UINT i = 0; i < nCount; i++)
	{
		p = Decoders[i];
		if (p->pPlugin == NULL)
			continue; // ���� �� ���� ���� ��������� �� ��� ������ - ������ ������������� ����� ����������

		if (!pFileData && !bFileFailed && wcschr(pFileName, L'\\'))
		{
			if ((p->pPlugin->pData->pForbidden && wcschr(p->pPlugin->pData->pForbidden, L':'))
				|| (p->pPlugin->pData->pActive && wcschr(p->pPlugin->pData->pActive, L':'))
				|| (p->pPlugin->pData->pInactive && wcschr(p->pPlugin->pData->pInactive, L':')))
			{
				CUnicodeFileName FileName;
				FileName.Assign(pFileName); // ����������� � UNC �������
				HANDLE hFile = CreateFileW((LPCWSTR)FileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
				if (hFile == INVALID_HANDLE_VALUE)
				{
					bFileFailed = true;
				}
				else
				{
					if (!ReadFile(hFile, Data, sizeof(Data), &nDataRead, NULL) || !nDataRead)
					{
						bFileFailed = true;
					}
					else
					{
						pFileData = Data;
						nFileDataSize = nDataRead;
					}
					CloseHandle(hFile);
				}
			}
		}

		// ����������� - ��������� ������
		_ASSERTE(p->pForbidden && p->pPlugin->pData->pForbidden && (lstrcmp(p->pPlugin->pData->pForbidden,p->pForbidden)==0));
		//-- �� �� ����� �������� � IsAllowed
		//if (wcschr(p->pForbidden,L'*'))
		//	continue; // �� ����������� '*'

		// ���������� �������������� ����������� �������� ����� �����
		// 2009-12-13 ���� false/*abAllowAsterisk*/
		if (p->pPlugin->IsAllowed(pExt, true/*abAllowAsterisk*/, true/*abAllowInactive*/,
				false/*default for abAssumeActive*/,
				pFileData, nFileDataSize)) // ����� ����� ���� NULL
		{
			return true;
		}
	}

	return false;
}

void CPVDManager::SortPlugins2()
{
	std::vector<CModuleInfo*>::iterator i, j, liMin;
    bool liCmp, lbChanged = false;
    CModuleInfo *tmp, *pFind, *pNewDefaultDisplay;
    for (i=Plugins.begin(); i!=Plugins.end() && (i+1)!=Plugins.end(); i++)
	{
        liMin = i;
        for (j=i+1; j!=Plugins.end(); j++)
		{
            liCmp = PluginCompare(*liMin, *j);
            if (liCmp)
                liMin = j;
        }
        if (liMin!=i)
		{
			lbChanged = true;
			tmp = *liMin;
			*liMin = *i;
			*i = tmp;
        }
    }

	if (lbChanged)
	{
		UINT n = (int)Plugins.size() * 16;
		
		for (i=Plugins.begin(); i!=Plugins.end(); i++, n-=16)
		{
			CModuleInfo* p = *i;
			if (p->Priority() != n)
			{
				p->SetPriority(n);

				// ��������� ��������� ������ ���� �� �������� ������� ������� ��������
				//HKEY hkey = NULL; DWORD dwDisp = 0;
				//if (!RegCreateKeyEx(HKEY_CURRENT_USER, p->pRegPath, 0, 0, 0, KEY_ALL_ACCESS, 0, &hkey, &dwDisp)) {
				//	DWORD nDataSize = 0;
				//	RegSetValueEx(hkey, L"Priority", 0, REG_DWORD, (LPBYTE)&n, sizeof(n));
				//	RegCloseKey(hkey); hkey = NULL;
				//}
			}
		}
		_ASSERTE(n>=0);
	}
	
	
    for (i=Decoders.begin(); i!=Decoders.end(); i=Decoders.erase(i))
	{
    	tmp = *i;
    	tmp->Release(szPVDManager);
    }
    pFind = pDefaultDisplay;
	for (i=Displays.begin(); i!=Displays.end(); i=Displays.erase(i))
	{
		tmp = *i;
		if (pFind && pFind == tmp) pFind = NULL;
		tmp->Release(szPVDManager);
	}

	pNewDefaultDisplay = NULL;
	BOOL bTerminalSession = GetSystemMetrics(SM_REMOTESESSION); // TRUE - ��� "Terminal Services client session"
	#ifdef _DEBUG
	//bTerminalSession = TRUE;
	#endif

	WARNING("����� ����������� �������. � ������������ ������� ����������� ������������� DX");
    for (i=Plugins.begin(); i!=Plugins.end(); i++)
	{
        tmp = *i;
		bool bKnown = false;
		if ((tmp->nCurrentFlags & PVD_IP_DECODE) == PVD_IP_DECODE)
		{
			tmp->AddRef(szPVDManager);
			Decoders.push_back(tmp);
			bKnown = true;
		} 
		if ((tmp->nCurrentFlags & PVD_IP_DISPLAY) == PVD_IP_DISPLAY
			|| (tmp->pPlugin && (tmp->pPlugin->nFlags & PVD_IP_DISPLAY) == PVD_IP_DISPLAY))
		{
			tmp->AddRef(szPVDManager);
			Displays.push_back(tmp);
			bKnown = true;

			if (bTerminalSession) {
				if (tmp->nCurrentFlags & PVD_IP_NOTERMINAL) {
					tmp->nActiveFlags &= ~PVD_IP_DISPLAY;
					tmp->nCurrentFlags &= ~PVD_IP_DISPLAY;
				}
			}
			
			if ((tmp->nActiveFlags & PVD_IP_DISPLAY) && (tmp->nCurrentFlags & PVD_IP_PRIVATE)) {
				// Private Display - ������ �������� ��� ������������� ������ ������
				tmp->nActiveFlags &= ~PVD_IP_DISPLAY;
			} else if ((tmp->nActiveFlags & PVD_IP_DISPLAY) && !pNewDefaultDisplay) {
				if (tmp->pPlugin)
					pNewDefaultDisplay = tmp;
				else
					tmp->nActiveFlags &= ~PVD_IP_DISPLAY; // ������, ������ �� ������, � ���� ����������
			} else if (pNewDefaultDisplay && (tmp->nActiveFlags & PVD_IP_DISPLAY)) {
				tmp->nActiveFlags &= ~PVD_IP_DISPLAY; // ������� ���������� �������� ������ �����������
			}
		} 

		if (!bKnown)
		{
			#ifdef PICVIEWDUMP
			wchar_t* pszDbg = (wchar_t*)malloc(sizeof(wchar_t)*(lstrlenW(tmp->pModulePath)+64));
			wsprintfW(pszDbg, L"Unknown module type: %s\n", tmp->pModulePath);
			OutputDebugString(pszDbg);
			free(pszDbg);
			#endif
		}
	}

	if (!pNewDefaultDisplay && Displays.size()>0) {
		for (i=Displays.begin(); i!=Displays.end(); i++) {
			tmp = *i;
			if (!(tmp->nCurrentFlags & PVD_IP_DISPLAY))
				continue; // ���������� � ������� ������
			if ((tmp->nCurrentFlags & PVD_IP_DISPLAY) && (tmp->nCurrentFlags & PVD_IP_PRIVATE))
				continue; // Private Display - ������ �������� ��� ������������� ������ ������
			if (tmp->nCurrentFlags & PVD_IP_DIRECT) {
				pNewDefaultDisplay = tmp; break; // ������������ - �������� ������ ������
			}
		}
		if (!pNewDefaultDisplay) {
			for (i=Displays.begin(); i!=Displays.end(); i++) {
				tmp = *i;
				if (!(tmp->nCurrentFlags & PVD_IP_DISPLAY))
					continue; // ���������� � ������� ������
				if ((tmp->nCurrentFlags & PVD_IP_DISPLAY) && (tmp->nCurrentFlags & PVD_IP_PRIVATE))
					continue; // Private Display - ������ �������� ��� ������������� ������ ������
				pNewDefaultDisplay = tmp; break; // �� � ������ ��� ������� ����������...
			}
		}


		if (!pNewDefaultDisplay)
			pNewDefaultDisplay = Displays[0];

		pNewDefaultDisplay->nActiveFlags |= PVD_IP_DISPLAY;
	}
	pDefaultDisplay = pNewDefaultDisplay;
	_ASSERTE(pDefaultDisplay!=NULL);
}

// ���������� ��� ���������� ������ ������� (�� ExitFARW)
void CPVDManager::UnloadPlugins2()
{
	std::vector<CModuleInfo*>::iterator iter = Plugins.begin();
	while (iter != Plugins.end())
	{
		CModuleInfo* plug = *iter;
		plug->Release(szPVDManager);
		iter = Plugins.erase(iter);
	}
	iter = Decoders.begin();
	while (iter != Decoders.end())
	{
		CModuleInfo* plug = *iter;
		plug->Release(szPVDManager);
		iter = Decoders.erase(iter);
	}
	iter = Displays.begin();
	while (iter != Displays.end())
	{
		CModuleInfo* plug = *iter;
		plug->Release(szPVDManager);
		iter = Displays.erase(iter);
	}
	//
	std::vector<wchar_t*>::iterator iwc = sWholeLog.begin();
	while (iwc != sWholeLog.end())
	{
		wchar_t* psz = *iwc;
		free(psz);
		iwc = sWholeLog.erase(iwc);
	}
}

void CPVDManager::AddLogString(const wchar_t* asModule, const wchar_t* asMessage)
{
	if (!asMessage) asMessage = L"<NULL>";
	if (!asModule) asModule = L"";
	
	if (g_Plugin.sLogFile[0] && (*asModule || *asMessage)) {
		HANDLE hFile = CreateFile(g_Plugin.sLogFile, GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		if (hFile != INVALID_HANDLE_VALUE) {
			DWORD nWrite = 0;
			SetFilePointer(hFile, 0,0, FILE_END);
			if (*asModule) {
				WriteFile(hFile, asModule, lstrlen(asModule)*2, &nWrite, 0);
				WriteFile(hFile, L": ", 4, &nWrite, 0);
			}
			if (*asMessage) WriteFile(hFile, asMessage, lstrlen(asMessage)*2, &nWrite, 0);
			WriteFile(hFile, L"\r\n", 4, &nWrite, 0);
			CloseHandle(hFile);
		}
	}
	
	int nCurCount = (int)sWholeLog.size();
	if (nCurCount >= MAX_WHOLELOG_LINES) {
		std::vector<wchar_t*>::iterator iwc = sWholeLog.begin();
		while ((nCurCount >= MAX_WHOLELOG_LINES) 
			&& (iwc != sWholeLog.end()))
		{
			wchar_t* psz = *iwc;
			free(psz);
			iwc = sWholeLog.erase(iwc);
		}
	}

	MCHKHEAP;
	
	int nLen = lstrlen(asMessage)+1;
	if (nLen > 1024) nLen = 1024;
	#define LOGMODULEWIDTH 12
	wchar_t* pszNew = (wchar_t*)malloc((nLen+LOGMODULEWIDTH+3)*sizeof(wchar_t));
	if (pszNew) {
		lstrcpyn(pszNew, asModule, LOGMODULEWIDTH+1);
		int nCur = lstrlen(pszNew);
		while (nCur < LOGMODULEWIDTH) pszNew[nCur++] = L' ';
		pszNew[LOGMODULEWIDTH] = L'|';
		lstrcpyn(pszNew+LOGMODULEWIDTH+1, asMessage, nLen);
		
		sWholeLog.push_back(pszNew);
		bWholeLogChanged = true;
	}

	MCHKHEAP;
}

//// ����� ��������� �������� � ������������� ����� ��������� ����� ���������, ��������
//// ������ ����������� ����. ��� ��������� ��� ����, ����� ��� ��������� ��������
//// ������� (��� ���� ����� ��) ����� ���� ��������� �������� ����� ����������.
//// ��� ��������� �������� ������ ����� ���������� �������, ������� ������������
//// � ��������� ���.
//void CPVDManager::ResetProcessed(C_Image* apImage)
//{
//	MCHKHEAP;
//	if (apImage->mpb_Processed) {
//		int nCount = (int)Decoders.size();
//		_ASSERTE(nCount>0);
//		if (apImage->mn_ProcessedSize != nCount) {
//			free(apImage->mpb_Processed); apImage->mpb_Processed = NULL; apImage->mn_ProcessedSize = 0;
//		} else {
//			memset(apImage->mpb_Processed, 0, nCount*sizeof(bool));
//		}
//	}
//	MCHKHEAP;
//}

bool CPVDManager::OpenWith(
		CImage* apImage,
		CDecoderHandle** ppFile,
		CModuleInfo *pDecoder,
		const wchar_t *pFileName, i64 lFileSize,
		const u8 *pBuffer, uint lBuffer)
		// pvdInfoPage2 &InfoPage ������ � CImage
{
	// ������ ����������� � ������ ��������
	_ASSERTE(GetCurrentThreadId() == gnDecoderThreadId);

	if (!pDecoder->pPlugin)
	{
		_ASSERTE(pDecoder->pPlugin);
		return false;
	}
	
	TODO("��������� ��������, ����� ����������� ���������� ������ �������� ������������ ��� ���������");
	// � �������� ������ �� ������������� ������ ���������. ����� ����� ������� ����� apImage->pTempInfo,
	// ����� ��������� ������������� � ���� ������� ����� ������� �������� �� ������ ��� ��������������
	// ������, ��� ������������ ����������� �������� ����� ��������� � ���� ��������
	ImageInfo* pInfo = &apImage->Info;

	if (!*ppFile)
	{
		*ppFile = new CDecoderHandle(szPVDManager, apImage);

		#ifdef _DEBUG
		// ��� ����������
		(*ppFile)->FileName = (LPCWSTR)apImage->FileName;
		#endif
	}
	else
	{
		// ��� �� ����� ����, ������ ���� ����������� ����� ��������� (��� ������ ����������� ��� ��)
		// � ��� ���, �� ������ ���������� �������� ������ ���� ������!
		_ASSERTE((*ppFile)->IsReady()==FALSE);
		(*ppFile)->Close();
	}

	bool result = false;
	LPVOID pFileContext = NULL;
	pvdInfoImage2 InfoImage = {sizeof(InfoImage)};
	pvdInfoPage2  InfoPage = {sizeof(pvdInfoPage2)};
	
	MCHKHEAP;

#ifdef PICVIEWDUMP
	wchar_t szDbg[MAX_PATH*3], *pNamePart;
	int nCur = 0, nSize = sizeofarray(szDbg);
	lstrcpyn(szDbg+nCur, pDecoder->pPlugin->pName, 128); nCur += lstrlen(szDbg+nCur);
	lstrcpy(szDbg+nCur, L": Opening \""); nCur += lstrlen(szDbg+nCur);
	pNamePart = pFileName ? wcsrchr(pFileName, L'\\') : L"<NULL>";
	pNamePart = pNamePart ? (pNamePart+1) : (pFileName ? pFileName : L"<NULL>");
	lstrcpyn(szDbg+nCur, pNamePart, MAX_PATH+1); nCur += lstrlen(szDbg+nCur);
	lstrcpy(szDbg+nCur, L"\"� ");
	//wnsprintf(szDbg, sizeofarray(szDbg), L"%s: Opening image %s...", pDecoder->pPlugin->pName, pFileName ? pFileName : L"<NULL>");
	OutputDebugString(szDbg);
#endif

	//IN_APL ������ �� �������
	//if (lFileSize == (int)lBuffer)
	//	lFileSize = 0;
	
	// ��������� ��� �������� �����, ����� �������� ��� � ��������� ����
	//lstrcpyn(/*g_Plugin.ImageX*/ apImage->DecoderName, pDecoder->pPlugin->pName, sizeofarray(/*g_Plugin.ImageX*/ apImage->DecoderName));
	///*g_Plugin.ImageX*/ apImage->lWidth = /*g_Plugin.ImageX*/ apImage->lHeight = /*g_Plugin.ImageX*/ apImage->nBPP = 0;
	///*g_Plugin.ImageX*/ apImage->nPages = 0; /*g_Plugin.ImageX*/ apImage->Animation = false;
	pInfo->InitDecoder(pDecoder);
	
	//if (mp_Image && mp_Image == /*g_Plugin.ImageX*/ apImage && /*g_Plugin.ImageX*/ apImage == g_Plugin.Image[0])
	if (apImage->IsActive())
	{
		//TitleRepaint(true);
		//ExecuteInMainThread(FW_TITLEREPAINTD,0);
		WARNING("������ ���� ������������, � � ��������� �������������� ������ ����, � �� ���, ������� ��� �� ������!");
		g_Plugin.FlagsWork |= FW_TITLEREPAINTD;
	}
	else
	{
		TODO("OSD ��� ������������� �����������");
	}
		
	// �������
	pDecoder->pPlugin->szLastError[0] = 0;

	//if (apImage->mb_ImageOpened || apImage->mp_ImageContext) {
	//	_ASSERTE(!apImage->mb_ImageOpened && !apImage->mp_ImageContext);
	//}

	if (pDecoder->pPlugin->FileOpen2(pFileName, lFileSize, pBuffer, lBuffer, &InfoImage))
	{
		MCHKHEAP;
		
		//apImage->mb_ImageOpened = true;
		//apImage->mp_ImageContext = InfoImage.pImageContext;
		(*ppFile)->Assign(pDecoder, InfoImage.pImageContext, &InfoImage);
		
		pInfo->Assign(InfoImage);
		///*g_Plugin.ImageX*/ apImage->nPages = InfoImage.nPages;
		///*g_Plugin.ImageX*/ apImage->Animation = InfoImage.Flags & PVD_IIF_ANIMATED/* 1 */;
		///*g_Plugin.ImageX*/ apImage->FileName = pFileName;
		//if (!wcsncmp(pFileName,L"\\\\?\\",4)) pFileName+=4;
		//lstrcpynW(/*g_Plugin.ImageX*/ apImage->FileNameData, pFileName, sizeofarray(/*g_Plugin.ImageX*/ apImage->FileNameData));
		//memset(&apImage->Info, 0, sizeof(apImage->Info));
		//apImage->Info.cbSize = sizeof(apImage->Info);
		//apImage->Info.iPage = 0;
		
		InfoPage.iPage = 0;
		if (pDecoder->pPlugin->PageInfo2(InfoImage.pImageContext, &InfoPage))
		{
			MCHKHEAP;
			WARNING("!!! ����������, ��� ������ ���� CDecodeItem!");
			pInfo->Assign(InfoPage);
			
			(*ppFile)->Info.Assign(InfoPage);
			
			///*g_Plugin.ImageX*/ apImage->lWidth = apImage->Info.lWidth;
			///*g_Plugin.ImageX*/ apImage->lHeight = apImage->Info.lHeight;
			///*g_Plugin.ImageX*/ apImage->nBPP = apImage->Info.nBPP;
			//if (apImage->Info.pFormatName)
			//	lstrcpyn(/*g_Plugin.ImageX*/ apImage->FormatName, apImage->Info.pFormatName, sizeofarray(/*g_Plugin.ImageX*/ apImage->FormatName));
			//else if (InfoImage.pFormatName)
			//	lstrcpyn(/*g_Plugin.ImageX*/ apImage->FormatName, InfoImage.pFormatName, sizeofarray(/*g_Plugin.ImageX*/ apImage->FormatName));
			//else
			//	* /*g_Plugin.ImageX*/ apImage->FormatName = 0;
			//if (apImage->Info.pCompression)
			//	lstrcpyn(/*g_Plugin.ImageX*/ apImage->Compression, apImage->Info.pCompression, sizeofarray(/*g_Plugin.ImageX*/ apImage->Compression));
			//else if (InfoImage.pCompression)
			//	lstrcpyn(/*g_Plugin.ImageX*/ apImage->Compression, InfoImage.pCompression, sizeofarray(/*g_Plugin.ImageX*/ apImage->Compression));
			//else
			//	* /*g_Plugin.ImageX*/ apImage->Compression = 0;
			//if (InfoImage.pComments)
			//	lstrcpyn(/*g_Plugin.ImageX*/ apImage->Comments, InfoImage.pComments, sizeofarray(/*g_Plugin.ImageX*/ apImage->Comments));
			//else
			//	* /*g_Plugin.ImageX*/ apImage->Comments = 0;
			result = true;
		}
	}
	
	MCHKHEAP;

	#ifdef PICVIEWDUMP
	OutputDebugString(result ? L"Succeeded\n" : L"Failed!!!\n");
	#endif
	if (!result)
	{
		if (pDecoder->pPlugin->szLastError[0])
		{
			int nLen = lstrlen(szDbg);
			if ((nLen+10) < sizeofarray(szDbg))
			{
				lstrcpyn(szDbg+nLen, pDecoder->pPlugin->szLastError, sizeofarray(szDbg)-nLen-1);
			}
		}
		else
		{
			lstrcat(szDbg, L"Failed!!! ");
		}
		pDecoder->pPlugin->SetStatus(szDbg);
		#ifdef _DEBUG
		if ((GetKeyState(VK_CAPITAL) & 1) == 1)
		{
			Sleep(1000);
		}
		#endif

		(*ppFile)->Close();
	}
	MCHKHEAP;
	return result;
}

bool CPVDManager::Open(
		CImage* apImage,
		CDecoderHandle** ppFile,
		const wchar_t *pFileName,
		i64 lFileSize, const u8 *pBuffer, uint lBuffer,
		DecodeParams *apParams)
		// pvdInfoPage2 &InfoPage ������ � CImage
{
	if (Decoders.empty())
		return false;

	bool result = false;
	//int i, s, nFrom, nTo, nCount, c; // ������ �� ������!
	int s;
	
	//nCount = (int)Decoders.size();
	_ASSERTE(pFileName);

	TODO("������ ����� ����� ���� ���-���� ��������� � ������ ������ ��� �������������?");
	// ����� �� ������� �� �����, ������� ����� �� �������� �� �����������
    // �� � ��������, ��� ��� ����� ����� �� �������, ��� ������ FileMapping?

	DWORD nNextDecoder = (apParams->Flags & (eRenderNextDecoder|eRenderPrevDecoder));
	
	//int iFromDecoder = (nNextDecoder != 0) ? apImage->mi_DecoderIndex : 0;
	//CModuleInfo* pFromDecoder = apImage->GetDecoder();

	//if (iFromDecoder < 0 || iFromDecoder >= (int)Decoders.size())
	//{
	//	_ASSERTE(apImage->mi_DecoderIndex < (int)Decoders.size());
	//	iFromDecoder = 0;
	//}

	// ����� �� "������" ���� ��������� ��� ����� ���������
	// ��������� ������ (apImage->mpb_Processed) ���������, �������� ��������� ������� ����
	//apImage->CheckProcessedSize(nCount);
	
	//if (!apImage->mpb_Processed) {
	//	apImage->mn_ProcessedSize = nCount;
	//	apImage->mpb_Processed = (bool*)calloc(nCount,sizeof(bool));
	//}

	/* -- � ��� ��� ������ ������ (memset...), ����� �� ����������, ���� ������ ������ ������ ������
	else if (!nNextDecoder || !mps_FileName) { // ������� !nNextDecoder
		memset(mpb_Processed, 0, nCount*sizeof(bool));
	} else if (lstrcmp(pFileName, mps_FileName)) {
		memset(mpb_Processed, 0, nCount*sizeof(bool));
	}
	*/

	//// ��������� ��� ��������������� �����
	//bool bNewFile = (mps_FileName == NULL);
	//if (mps_FileName)
	//	bNewFile = lstrcmp(mps_FileName, pFileName)!=0;
	//if (mps_FileName && bNewFile) {
	//	free(mps_FileName); mps_FileName = NULL;
	//}
	//if (bNewFile)
	//	mps_FileName = _wcsdup(pFileName);



	const wchar_t *pExt = GetExtension(pFileName);
	//	wcsrchr(pFileName, L'\\');
	//if (!pExt) pExt = pFileName;
	//if (pExt = wcsrchr(pFileName, '.'))
	//	pExt++;
	//else
	//	pExt = L".";

	

	//for (; iSubDecoder < nPlugins && !result 
	//	&& pPluginsData[iSubDecoder].PluginInfo.Priority >= LowestAllowedPriority; 
	//	iSubDecoder++)
	CModuleInfo* p = NULL;
	// ���� � ��� ������� 
	// ��� ����� �� ������� - ���� ����������� �� ���� ����������� �������
	// �������� AltPgDn / AltPgUp - ���������� iSubDecoder �������� � ������� ��������� �����������
	// {{{
	// 1. iSubDecoder .. Count-1
	// 2. 0 .. iSubDecoder-1
	// }}} -- ��� ����� "����������" ������� ���������
	// 3. 0 .. Count-1, �� �� ���������� �����������
	// 4. 0 .. Count-1, �� � �� ���������� ���������� �������� ��� �� ��������
	for (s = 1; !result && s <= 4; s++)
	{
		if (s >= 4 && !(apParams->Flags & eRenderForceDecoder))
			break; // "��������� ������" ��� ����� �������������� ���������� ������ �� ������ ����� �����

		CModuleInfo* pFrom = (s == 1) ? apImage->GetDecoder() : NULL;

		//if (!nNextDecoder) // ������� �����. �� ����������� ���������� ������� �������
		//{
		//	c = 1;
		//	switch (s)
		//	{
		//	case 1: nFrom = iFromDecoder; nTo = nCount; break;
		//	case 2: nFrom = 0; nTo = iFromDecoder; break;
		//	case 3: case 4: nFrom = 0; nTo = nCount; break;
		//	}
		//}
		//else if (nNextDecoder & eRenderPrevDecoder)
		//{
		//	c = -1;
		//	switch (s)
		//	{
		//	case 1: nFrom = iFromDecoder-1; nTo = -1; break;
		//	case 2: nFrom = nCount-1; nTo = iFromDecoder; break;
		//	case 3: case 4: nFrom = nCount-1; nTo = -1; break;
		//	}
		//}
		//else if (nNextDecoder & eRenderNextDecoder)
		//{
		//	c = 1;
		//	switch (s)
		//	{
		//	case 1: nFrom = iFromDecoder+1; nTo = nCount; break;
		//	case 2: nFrom = 0; nTo = iFromDecoder+1; break;
		//	case 3: case 4: nFrom = 0; nTo = nCount; break;
		//	}
		//}
		
		// (i>=0 && i<nCount) - �� ������ ������, ����� � ����������� �� ���������
		//for (i = nFrom; !result && i != nTo && (i>=0 && i<nCount); i+=c)
		while ((p = GetNextDecoder(pFrom, ((nNextDecoder & eRenderPrevDecoder) == 0))) != NULL)
		{
			pFrom = p; // ����� ��������, ��� ���������� �����

			if ((g_Plugin.FlagsWork & FW_TERMINATE))
			{
				return false;
			}
		
			TODO("�� ��������� ������� ReOpen ���� �� ��������?");
			//if (i < apImage->mn_ProcessedSize && apImage->WasDecoderFailed(i)) continue;
			if (apImage->WasDecoderFailed(p))
				continue;

			//p = Decoders[i];
			if (p->pPlugin == NULL)
			{
				_ASSERTE(p->pPlugin!=NULL); // ��? ��� ����� ����� ����?
				continue;
			}

			//��� �� �������. ������������ �� ���������� // �� ���� 3 ���� �� ���������� ��������� ������ �� ���������� - 
			//// ������� ����� ������, ������ �� ������ �� ������������ ����������
			//if ((s == 3) && wcschr(p->pPlugin->pIgnored,L'*')) // �� ����������� '*'
			_ASSERTE(p->pForbidden);
			// ������ - ������ (����������� ����������)
			if (/*wcschr(p->pForbidden, L'*') ||*/ ExtensionMatch(p->pForbidden, pExt, pBuffer, lBuffer))
			{
				// ��� ���������� ������ ��������� ��� ����� ��������
				apImage->SetDecoderFailed(p); // ����� �������� �� ������������
				continue;
			}

			_ASSERTE(p->pInactive && p->pActive);
			// �������� ��������/���������� ����������
			if (p->pPlugin->IsAllowed(
					pExt, true/*abAllowAsterisk*/, (s >= 3)/*abAllowInactive*/, 
					(s == 4)/*abAssumeActive*/,
					pBuffer, lBuffer))
			{
				//if (i < apImage->mn_ProcessedSize)
				//	apImage->mpb_Processed[i] = true; // ����� �������� �� ������������

				TRY
				{
					if (OpenWith(apImage, ppFile, p, pFileName, lFileSize, pBuffer, lBuffer))
					{
						// ��������, ����� ��������� ������� ������� ���� (� ������������� ���� ���� �� ����)
						apImage->SetDecoder(p);
						//apImage->mi_DecoderIndex = i;
						result = true;
						break;
					}
					else
					{
						// ��������, ��� ���� ��������� ������� �� �������
						apImage->SetDecoderFailed(p);
					}
				}CATCH{
				}
			}
		}
		if (result)
			break;
	}

	return result;
}

CModuleInfo* CPVDManager::GetNextDecoder(const CModuleInfo* apDecoder, bool abForward)
{
	CModuleInfo* pNext = NULL;

	if (Decoders.empty())
	{
		_ASSERTE(!Decoders.empty());
		return NULL;
	}
	else if (abForward)
	{
		std::vector<CModuleInfo*>::iterator i;
		for (i = Decoders.begin(); i != Decoders.end(); ++i)
		{
			if (!apDecoder)
			{
				pNext = *i;
				break;
			}
			else if ((*i) == apDecoder)
			{
				++i;
				if (i != Decoders.end())
					pNext = *i;
				break;
			}
		}
		// -- ������ ������� NULL, ���� �������� ����� ������
		//if (!pNext)
		//{
		//	i = Decoders.begin();
		//	pNext = *i;
		//}
	}
	else
	{
		std::vector<CModuleInfo*>::reverse_iterator i;
		for (i = Decoders.rbegin(); i != Decoders.rend(); ++i)
		{
			if (!apDecoder)
			{
				pNext = *i;
				break;
			}
			else if ((*i) == apDecoder)
			{
				++i;
				if (i != Decoders.rend())
					pNext = *i;
				break;
			}
		}
		// -- ������ ������� NULL, ���� �������� ������ ������
		//if (!pNext)
		//{
		//	i = Decoders.rbegin();
		//	pNext = *i;
		//}
	}

	//_ASSERTE(pNext!=NULL); -- NULL ��������, ����� ��������
	return pNext;
}

// ������� �������������� ������ (PageFree2)
void CPVDManager::CloseData(CDecodeItem* apItem)
{
	if (!apItem)
	{
		_ASSERTE(apItem!=NULL);
		return;
	}
	
	if (!gnDecoderThreadId || GetCurrentThreadId() == gnDecoderThreadId)
	{
		if (apItem->pFile)
		{
			if (apItem->pFile && apItem->pFile->Decoder() && apItem->pFile->Decoder()->pPlugin)
			{
				apItem->pFile->Decoder()->pPlugin->PageFree2(apItem->pFile->Context(), /*(pvdInfoDecode2*)*/ &apItem->Data);
			}
			else
			{
				_ASSERTE(apItem->pFile && apItem->pFile->Decoder() && apItem->pFile->Decoder()->pPlugin);
			}
			apItem->Data.pImage = NULL;
		}
	}
	else
	{
		_ASSERTE(gnDecoderThreadId && GetCurrentThreadId() == gnDecoderThreadId);
	}
}

// ������� ���������� ��������
void CPVDManager::CloseDecoder(CDecodeItem* apItem)
{
	if (!apItem)
	{
		_ASSERTE(apItem!=NULL);
		return;
	}
	if (!apItem->pFile)
	{
		return; // ����
	}
	
	if (!gnDecoderThreadId || GetCurrentThreadId() == gnDecoderThreadId)
	{
		// ������ �������� Close()
		SafeRelease(apItem->pFile,szPVDManager);
	}
	else
	{
		//_ASSERTE(gnDecoderThreadId && GetCurrentThreadId() == gnDecoderThreadId);
		EnterCriticalSection(&csDecoderQueue);
		apItem->pFile->RequestRelease();
		m_DecoderRelease.push_back(apItem->pFile);
		LeaveCriticalSection(&csDecoderQueue);
		apItem->pFile = NULL;
	}
	//ResetProcessed(); -- ����. ����� �������� ��� ������� ������ ��������
	// �������� �������, ����� ��������� ������������� �� ���� �������� �� ���� ������ �������,
	// � ���� ������ ���������� ���������� ������� ���������, ���� ��� �� ��������,
	// ��� ���� �� ���������� �����-�� ������ �������.
	//iSubDecoder = 0; // � ��� ��� �����. �� F5 ����� �������� ��������� ����������� ��������
}

// ������� ���������� �������
void CPVDManager::CloseDisplay(CDecodeItem* apItem)
{
	if (!apItem)
	{
		_ASSERTE(apItem!=NULL);
		return;
	}
	
	if (!gnDisplayThreadId || GetCurrentThreadId() == gnDisplayThreadId)
	{
		// ������ �������� Close()
		SafeRelease(apItem->pDraw,szPVDManager);
	}
	else
	{
		_ASSERTE(gnDisplayThreadId && GetCurrentThreadId() == gnDisplayThreadId);
	}
}

BOOL __stdcall CPVDManager::DecodeCallback2(
		void *pDecodeCallbackContext2, UINT32 iStep, UINT32 nSteps, pvdInfoDecodeStep2* pImagePart)
{
	// ����������� ������ �������������
	if (bCancelDecoding || (g_Plugin.FlagsWork & FW_TERMINATE))
		return FALSE;
	return TRUE;
}

bool CPVDManager::DecodePixels(CDecodeItem* apItem, bool abResetTick)
{
	WARNING("� ��������� ���������� �������� ������ ������������ ��������. ���� - ����������� apImage->Info.nPage");
	WARNING("��������� ������� �� ��� ==> CPVDManager::Decode  &&  CPVDManager::TransferToDisplay");
	WARNING("CPVDManager::TransferToDisplay ������ ���������� � ���� �������!");
	bool result = false;

	WARNING("apItem->pFile->Decoder() ��������� � ��������� ���������� � ����� AddRef/Release");

	//if (!CPVDManager::pDefaultDisplay || !CPVDManager::pDefaultDisplay->pPlugin) {
	//	OutputDebugString(L"CPVDManager::Decode==> PVDManager::pDefaultDisplay->pPlugin was not created!");
	//	return false;
	//}
	
	if (!apItem->pImage)
	{
		_ASSERTE(apItem->pImage!=NULL);
		#ifdef PICVIEWDUMP
		OutputDebugString(L"CPVDManager::DecodePixels==> apItem->pImage was not created!\n");
		#endif
		return false;
	}
	if (!apItem->pFile)
	{
		_ASSERTE(apItem->pFile!=NULL);
		#ifdef PICVIEWDUMP
		OutputDebugString(L"CPVDManager::DecodePixels==> apItem->pFile was not created!\n");
		#endif
		return false;
	}
	_ASSERTE(apItem->pFile && apItem->pFile->Decoder() && apItem->pFile->Decoder()->pPlugin);
	
	MCHKHEAP

	//CModuleInfo* lpDisplay = NULL;

	if (abResetTick) // ��� ������������� ��������� �������� �������� ��� (��� ������ ����� ��������)
		apItem->pImage->Info.lStartOpenTime = timeGetTime();
	DWORD t0 = apItem->pImage->Info.lStartOpenTime;
	DWORD t1 = timeGetTime(), t2=t1, t3=t1, t4=t1, t6=t1; //, t5=t1, t6=t1;

#ifdef PICVIEWDUMP
	wchar_t szLastError[128]; szLastError[0] = 0;
	wchar_t szDbg[MAX_PATH*3], *pNamePart;
	int nCur = 0, nSize = sizeofarray(szDbg);
	lstrcpyn(szDbg+nCur, apItem->pFile->Decoder()->pPlugin->pName, 128); nCur += lstrlen(szDbg+nCur);
	wsprintf(szDbg+nCur, L": Decoding page %i of image \"", apItem->pImage->Info.nPage); nCur += lstrlen(szDbg+nCur);
	const wchar_t* pFileName = (const wchar_t*)apItem->pImage->FileName;
	pNamePart = pFileName ? wcsrchr(pFileName, L'\\') : L"<NULL>";
	pNamePart = pNamePart ? (pNamePart+1) : (pFileName ? pFileName : L"<NULL>");
	lstrcpyn(szDbg+nCur, pNamePart, MAX_PATH); nCur += lstrlen(szDbg+nCur);
	lstrcpy(szDbg+nCur, L"\"� ");
	//wnsprintf(szDbg, sizeofarray(szDbg), L"%s: Decoding page %i of image %s...",
	//	pDecoder->pPlugin->pName, apItem->pImage->nPage, (const wchar_t*)apItem->pImage->FileName);
	OutputDebugString(szDbg);
#endif

	// �������
	apItem->pFile->Decoder()->pPlugin->szLastError[0] = 0;
	
	//pvdInfoPage2 InfoPage = {sizeof(pvdInfoPage2)};
	//TRY{
	
	bool bPageInfoOk = false;
	//if (apItem->pImage->Info.cbSize && apItem->pImage->Info.iPage == apItem->pImage->Info.nPage && apItem->pImage->Info.lWidth && apItem->pImage->Info.lHeight)
	if (apItem->pFile->Info.IsPageLoaded(apItem->pImage->Info.nPage))
	{
		bPageInfoOk = true;
	}
	else
	{
		pvdInfoPage2 InfoPage = {sizeof(pvdInfoPage2)};
		InfoPage.iPage = apItem->pImage->Info.nPage;
		bPageInfoOk = apItem->pFile->Decoder()->pPlugin->PageInfo2(apItem->pFile->Context(), &InfoPage);
		if (bPageInfoOk)
		{
			apItem->pImage->Info.Assign(InfoPage);
			//if (apItem->pImage->Info.pFormatName)
			//	lstrcpyn(apItem->pImage->FormatName, apItem->pImage->Info.pFormatName, sizeofarray(apItem->pImage->FormatName));
			//if (apItem->pImage->Info.pCompression)
			//	lstrcpyn(apItem->pImage->Compression, apItem->pImage->Info.pCompression, sizeofarray(apItem->pImage->FormatName));
		}
		if (bPageInfoOk && apItem->pImage->Info.nPages)
			apItem->pImage->CheckPages(apItem->pImage->Info.nPages);
	}

	if (bPageInfoOk)
	{
		t2 = timeGetTime();
		//pvdInfoDecode2 DecodeInfo = {sizeof(pvdInfoDecode2)};
		bool bNeedFree = true;
		//TODO("����������� ��������� 24 ������ ����������� ��� ��������� �� �������������. ��� ����� ������ ����� !PageDecode2!");
		//TODO("�������� ������� PageDecode2 � CreateWorkSurface. � ��������� ������� �� ������ �� ������ ��������� ����������� �������������� colormodel");
		//WARNING("24 ������ ����������� ������ �� �������� � Primary surface");

		if (apItem->Data.cbSize && apItem->Data.pImage)
		{
			_ASSERTE(!apItem->Data.cbSize || !apItem->Data.pImage);
			WARNING("������(?) ������ ����� ����������");
		}
		apItem->Data.cbSize = sizeof(apItem->Data);

		// Execute
		{
			t3 = timeGetTime();
			apItem->Data.iPage = apItem->pImage->Info.nPage;

			apItem->Data.Flags = (apItem->pFile->Decoder()->pPlugin == CPVDManager::pDefaultDisplay->pPlugin) ? PVD_IDF_ASDISPLAY : 0;
			apItem->Data.nBackgroundColor = g_Plugin.BackColor();

			MCHKHEAP

			SIZE scaledSize = {0,0};
			if (
			    (apItem->pFile->Decoder()->pPlugin->nFlags & (PVD_IP_CANDESCALE|PVD_IP_CANUPSCALE)) &&
				(g_Plugin.ZoomAuto || g_Plugin.AbsoluteZoom || g_Plugin.FlagsWork & FW_QUICK_VIEW)
			   )
			{
				//TODO("����� ���������� �������");

				SIZE imgSize = {apItem->pImage->Info.lWidth,apItem->pImage->Info.lHeight};
				SIZE renderSize = apItem->pImage->GetDefaultScaleSize();

				//if (g_Plugin.FlagsWork & FW_QUICK_VIEW)
				if (renderSize.cx && renderSize.cy && imgSize.cx && imgSize.cy) {
					BOOL lbCanScale = FALSE;

					if (renderSize.cx < imgSize.cx && renderSize.cy < imgSize.cy)
					{
						lbCanScale = (apItem->pFile->Decoder()->pPlugin->nFlags & PVD_IP_CANDESCALE);
					}
					else if (renderSize.cx > imgSize.cx && renderSize.cy > imgSize.cy)
					{
						lbCanScale = (apItem->pFile->Decoder()->pPlugin->nFlags & PVD_IP_CANUPSCALE);
					}

					if (lbCanScale)
					{
						double dr = (double)renderSize.cx / (double)renderSize.cy;
						double di = (double)imgSize.cx / (double)imgSize.cy;
						if (g_Plugin.ZoomAuto == ZA_FIT) {
							if (di > dr)
							{
								// ������� �� ������
								scaledSize.cx = (LONG)(renderSize.cy * di);
								scaledSize.cy = renderSize.cy;
							}
							else
							{
								// ������� �� ������
								scaledSize.cx = renderSize.cx;
								scaledSize.cy = (LONG)(renderSize.cx / di);
							}
						}
						else if (g_Plugin.ZoomAuto == ZA_FILL)
						{
							if (di < dr)
							{
								// ������� �� ������
								scaledSize.cx = (LONG)(renderSize.cy * di);
								scaledSize.cy = renderSize.cy;
							}
							else
							{
								// ������� �� ������
								scaledSize.cx = renderSize.cx;
								scaledSize.cy = (LONG)(renderSize.cx / di);
							}
						}
						else if (di > dr)
						{
							// ������� �� ������
							scaledSize.cx = (LONG)(renderSize.cy * di);
							scaledSize.cy = renderSize.cy;
						}
						else
						{
							// ������� �� ������
							scaledSize.cx = renderSize.cx;
							scaledSize.cy = (LONG)(renderSize.cx / di);
						}
						apItem->Data.lWidth = scaledSize.cx;
						apItem->Data.lHeight = scaledSize.cy;
					}
				}
			}

			MCHKHEAP

			//_ASSERTE(g_Plugin.hWnd);
			if (!g_Plugin.hWnd || !IsWindow(g_Plugin.hWnd))
			{
				lstrcpy(szLastError, L"PicView window was closed");
				result = false;
			}
			else if (apItem->pFile->Decoder()->pPlugin->PageDecode2(apItem->pFile->Context(),
					/*(pvdInfoDecode2*)*/ &apItem->Data, DecodeCallback2, this))
			{
				MCHKHEAP;

				// ����� �������� OK
				result = true;

				// �������� "��������" ���������� ��������
				if (apItem->pImage->mp_File != apItem->pFile)
				{
					SafeRelease(apItem->pImage->mp_File,NULL); // ��� ��� ������ ����?
					apItem->pFile->AddRef(szPVDManager);
					apItem->pImage->mp_File = apItem->pFile;
				}

				if (apItem->Data.Flags & PVD_IDF_ALPHA)
				{
					_ASSERTE(apItem->Data.ColorModel == PVD_CM_BGRA);
				}
				
				// �������� ���������� � ������� �����������!
				if (apItem->Data.lSrcWidth && apItem->Data.lSrcHeight)
				{
					apItem->pImage->Info.lWidth = apItem->Data.lSrcWidth;
					apItem->pImage->Info.lHeight = apItem->Data.lSrcHeight;
				}
				apItem->pImage->Info.lDecodedWidth = apItem->Data.lWidth;
				apItem->pImage->Info.lDecodedHeight = apItem->Data.lHeight;
				apItem->pImage->Info.nDecodedBPP = apItem->Data.nBPP;
				
				//apItem->pImage->Info.nBPP = apItem->Data.nBPP; -- ��� �� ����. ����������� ����� ���� � 8-������, � ������ ������������ � 32 ����
				//
				apItem->pImage->Info.Assign(apItem->Data, pNamePart);
				//if (apItem->Data.pFormatName)
				//	lstrcpyn(apItem->pImage->FormatName, apItem->Data.pFormatName, sizeofarray(apItem->pImage->FormatName));
				//if (apItem->Data.pCompression)
				//	lstrcpyn(apItem->pImage->Compression, apItem->Data.pCompression, sizeofarray(apItem->pImage->Compression));
				//if (!apItem->pImage->FormatName[0])
				//{
				//	const wchar_t* pszExt = wcsrchr(pNamePart, L'.');
				//	if (pszExt) {
				//		apItem->pImage->FormatName[0] = L'[';
				//		lstrcpyn(apItem->pImage->FormatName+1, pszExt+1, sizeofarray(apItem->pImage->FormatName)-3);
				//		lstrcat(apItem->pImage->FormatName, L"]");
				//	}
				//}
				
				MCHKHEAP

				t4 = timeGetTime();
				if (apItem->Data.nPages)
					apItem->pImage->CheckPages(apItem->Data.nPages);
				//DWORD nFlags = apItem->Data.Flags;
				//if (nFlags < 2) {
				//	//nFlags |= (apItem->Data.nBPP == 32) ? PVD_IDF_BGRA : PVD_IDF_BGR;
				//}
				TODO("����� �� 2-� ������ ����������");

				#if 0
				{
					//WARNING("�� ������������� � ��������������� ���������� ��� �� NULL");
					////_ASSERTE(*ppDrawContext == NULL);
					//if (*ppDrawContext) {
					//	// � ��� ������ ���. �� �������. � CImage ����� ���� ��������� DrawContext, ����� �� ������� ������...
					//	_ASSERTE(apItem->pImage->Display->pPlugin);
					//	apItem->pImage->Display->pPlugin->DisplayClose2(*ppDrawContext);
					//	*ppDrawContext = NULL;
					//}
					
					//result = dds->Blit(&apItem->Data, g_Plugin.BackColor);
					pvdInfoDisplayCreate2 DsCreate = {sizeof(pvdInfoDisplayCreate2)};
					#ifdef _DEBUG
						if (apItem->Data.ColorModel == PVD_CM_UNKNOWN)
						{
							if (lstrcmpi(apItem->pFile->Decoder()->pModuleFileName, L"GDIPlus.pvd"))
							{
								_ASSERTE(apItem->Data.ColorModel != PVD_CM_UNKNOWN);
							}
						}
					#endif
					// ��������� �������� �� ���������� ������� ��� ��������������� ��������.
					// ����� ���������� ������� �� ���������, � ����� ������� �� �������� ����
					// ������ - � ������ ����� apItem->Data
					pvdInfoDecode2 DecodeInfoCopy = apItem->Data;
					if (DecodeInfoCopy.nBPP<=8 && DecodeInfoCopy.ColorModel!=PVD_CM_PRIVATE)
					{
						if (!DecodeInfoCopy.pPalette)
						{
							DecodeInfoCopy.pPalette = g_Plugin.GetPalette(DecodeInfoCopy.nBPP, DecodeInfoCopy.ColorModel);
						}
					}
					DsCreate.pImage = &DecodeInfoCopy;
					DsCreate.BackColor = g_Plugin.BackColor();
					DsCreate.pDisplayContext = NULL;
					DsCreate.pFileName = wcsrchr(pFileName, L'\\');
					if (DsCreate.pFileName) DsCreate.pFileName++; else DsCreate.pFileName = pFileName;
					//DsCreate.nChessMateColor1 = 0xFFFFFF;
					//DsCreate.nChessMateColor2 = 0xCCCCCC;
					//DsCreate.nChessMateSize = 8;
					
					WARNING("��������� ���� PVD_IDF_PRIVATE_DISPLAY. ���� ���������� - ����� �������� ������ ����� ������ ��������");
					
					if ((apItem->Data.Flags & PVD_IDF_PRIVATE_DISPLAY) && (apItem->pFile->Decoder()->pPlugin->nCurrentFlags & PVD_IP_DISPLAY))
					{
						lpDisplay = apItem->pImage->mp_DecoderModule;
					}
					else
					{
						lpDisplay = CPVDManager::pDefaultDisplay;
					}
					CModuleInfo* pStartDisplay = lpDisplay;
					std::vector<CModuleInfo*>::iterator iDisp = CPVDManager::Displays.begin();
					BOOL lbFirstCheckDisplay = TRUE;

					// ���� g_Plugin.hWnd == 0 - ������ ���� ��� ����� Esc � ��� �����������
					while (g_Plugin.hWnd && iDisp != CPVDManager::Displays.end())
					{
						if (!lpDisplay->pPlugin) {
							_ASSERTE(lpDisplay->pPlugin);
							result = FALSE;
						} else {
							if (result = lpDisplay->pPlugin->DisplayCheck()) {
								MCHKHEAP
								if (!g_Plugin.hWnd) {
									result = false;
									break;
								}
								if (result = lpDisplay->pPlugin->DisplayCreate2(&DsCreate))
									break;
								wchar_t szMsg[128];
								lstrcpy(szMsg, L"Display module skipped: ");
								lstrcpy(szMsg+lstrlen(szMsg), lpDisplay->pPlugin->pName);
								CPVDManager::AddLogString(L"PVDManager", szMsg);
							}
						}

						if ((apItem->Data.Flags & PVD_IDF_PRIVATE_DISPLAY) || CPVDManager::Displays.size()<=1)
							break;

						if (!lbFirstCheckDisplay)
							iDisp++;
						else
							lbFirstCheckDisplay = FALSE;
						// ����� ��������� ������� �������
						while (iDisp != CPVDManager::Displays.end()
							&& ((pStartDisplay == *iDisp) 
							    || ((*iDisp)->pPlugin == NULL)
							    || ((*iDisp)->nCurrentFlags & PVD_IP_PRIVATE)))
							iDisp++;
						if (iDisp == CPVDManager::Displays.end()) break;

						// ��������� ����� �������
						lpDisplay = *iDisp;
						wchar_t szMsg[128];
						lstrcpy(szMsg, L"Switching to display module: ");
						lstrcpy(szMsg+lstrlen(szMsg), lpDisplay->pPlugin->pName);
						CPVDManager::AddLogString(L"PVDManager", szMsg);
					}

					if (result)
					{
						(*ppDraw)->Assign(lpDisplay, DsCreate.pDisplayContext);
						//*ppDrawContext = DsCreate.pDisplayContext;
					}

						//apItem->Data.pImage, apItem->Data.lImagePitch, apItem->Data.nBPP, g_Plugin.BackColor, 
						//nFlags, apItem->Data.nTransparentColor, apItem->Data.pPalette);
				}
				t5 = timeGetTime();
				bNeedFree = false;
				MCHKHEAP
				apItem->pFile->Decoder()->pPlugin->PageFree2(apItem->pFile->Context(), &apItem->Data);
				#endif
			}
		}
		if (!g_Plugin.hWnd || !IsWindow(g_Plugin.hWnd))
		{
			lstrcpy(szLastError, L"PicView window was closed");
			result = false;
		}
		MCHKHEAP
		if (result)
		{
			apItem->pImage->Info.lWidth = apItem->pImage->Info.lWidth;
			apItem->pImage->Info.lHeight = apItem->pImage->Info.lHeight;
			apItem->pImage->Info.nBPP = apItem->pImage->Info.nBPP;
			if (apItem->pImage->Info.Animation && !apItem->pImage->Info.lFrameTime)
				apItem->pImage->Info.lFrameTime = 100; // ����������� ������������� ����� ������
			apItem->pImage->Info.lFrameTime = apItem->pImage->Info.lFrameTime;
		}
	}
	t6 = timeGetTime();
	apItem->pImage->Info.lTimeOpen = t2-t0; //�������� ����� ����������� � ��������� ���������� � ��� (Open & PageInfo)
		_ASSERTE(apItem->pImage->Info.lTimeOpen>=0); if (apItem->pImage->Info.lTimeOpen<0) apItem->pImage->Info.lTimeOpen = 0;
	apItem->pImage->Info.lTimeDecode = (t4==t1) ? 0 : (t4-t3); // PageDecode
		_ASSERTE(apItem->pImage->Info.lTimeDecode>=0); if (apItem->pImage->Info.lTimeDecode<0) apItem->pImage->Info.lTimeDecode = 0;
	apItem->pImage->Info.lTimeTransfer = 0 ; //((t5==t1) ? 0 : max(0,t5-t4)) + ((t3==t1) ? 0 : max(0,t3-t2)); // CreateWorkSurface+Blit
	//	_ASSERTE(apItem->pImage->Info.lTimeTransfer>=0); if (apItem->pImage->Info.lTimeTransfer<0) apItem->pImage->Info.lTimeTransfer = 0;
	apItem->pImage->Info.lTimePaint = 0;
	apItem->pImage->Info.bTimePaint = FALSE;
	//wsprintf(apItem->pImage->OpenTimes, L"%i+%i+%i", 
	//	t2-t0, //�������� ����� ����������� � ��������� ���������� � ��� (Open & PageInfo)
	//	(t4==t1) ? 0 : t4-t3, // PageDecode
	//	((t5==t1) ? 0 : t5-t4) + ((t3==t1) ? 0 : t3-t2) // CreateWorkSurface+Blit
	//	);
	apItem->pImage->Info.lOpenTime = t6-apItem->pImage->Info.lStartOpenTime;
	//}CATCH{}

	MCHKHEAP;
	#ifdef PICVIEWDUMP
	OutputDebugString(result ? L"Succeeded\n" : L"Failed!!!\n");
	#endif
	if (result)
	{
		WARNING("������� ���� �������");
	}
	else
	{
		LPCWSTR pszError = NULL;
		if (szLastError[0])
		{
			pszError = szLastError;
		}
		else if (apItem->pFile->Decoder()->pPlugin->szLastError[0])
		{
			pszError = apItem->pFile->Decoder()->pPlugin->szLastError;
		}
		else
		{
			pszError = L"Failed!!! ";
		}
		if (pszError)
		{
			int nLen = lstrlen(szDbg);
			if ((nLen+10) < sizeofarray(szDbg))
			{
				lstrcpyn(szDbg+nLen, pszError, sizeofarray(szDbg)-nLen-1);
			}
		}
		apItem->pFile->Decoder()->pPlugin->SetStatus(szDbg);
	}

	return result;
}

bool CPVDManager::PixelsToDisplay(CDecodeItem* apItem)
{
	bool result = false;

	if (!apItem || !apItem->pImage || !apItem->pFile)
	{
		_ASSERTE(apItem && apItem->pImage && apItem->pFile);
		#ifdef PICVIEWDUMP
		OutputDebugString(L"CPVDManager::PixelsToDisplay==> (!apItem || !apItem->pImage || !apItem->pFile)\n");
		#endif
		return false;
	}

	if (!CPVDManager::pDefaultDisplay || !CPVDManager::pDefaultDisplay->pPlugin)
	{
		_ASSERTE(CPVDManager::pDefaultDisplay && CPVDManager::pDefaultDisplay->pPlugin);
		#ifdef PICVIEWDUMP
		OutputDebugString(L"CPVDManager::PixelsToDisplay==> PVDManager::pDefaultDisplay->pPlugin was not created!\n");
		#endif
		return false;
	}

	if (!apItem->pDraw)
	{
		apItem->pDraw = new CDisplayHandle(szPVDManager, apItem->pImage/*, apItem->pFile*/);
	}
	else
	{
		_ASSERTE(apItem->pDraw->Context() == NULL);
		apItem->pDraw->Close(); // ������� ������ �������� �������
	}

	MCHKHEAP
	
	DWORD t4 = timeGetTime(), t5=t4, t6=t4;
	
	//WARNING("�� ������������� � ��������������� ���������� ��� �� NULL");
	////_ASSERTE(*ppDrawContext == NULL);
	//if (*ppDrawContext) {
	//	// � ��� ������ ���. �� �������. � CImage ����� ���� ��������� DrawContext, ����� �� ������� ������...
	//	_ASSERTE(apItem->pImage->Display->pPlugin);
	//	apItem->pImage->Display->pPlugin->DisplayClose2(*ppDrawContext);
	//	*ppDrawContext = NULL;
	//}
	
	//TODO("����������� ��������� 24 ������ ����������� ��� ��������� �� �������������. ��� ����� ������ ����� !PageDecode2!");
	//TODO("�������� ������� PageDecode2 � CreateWorkSurface. � ��������� ������� �� ������ �� ������ ��������� ����������� �������������� colormodel");
	//WARNING("24 ������ ����������� ������ �� �������� � Primary surface");
	
	//result = dds->Blit(&DecodeInfo, g_Plugin.BackColor);
	pvdInfoDisplayCreate2 DsCreate = {sizeof(pvdInfoDisplayCreate2)};
	#ifdef _DEBUG
		if (apItem->Data.ColorModel == PVD_CM_UNKNOWN)
		{
			if (lstrcmpi(apItem->pFile->Decoder()->pModuleFileName, L"GDIPlus.pvd"))
			{
				_ASSERTE(apItem->Data.ColorModel != PVD_CM_UNKNOWN);
			}
		}
	#endif
	// ��������� �������� �� ���������� ������� ��� ��������������� ��������.
	// ����� ���������� ������� �� ���������, � ����� ������� �� �������� ����
	// ������ - � ������ ����� DecodeInfo
	pvdInfoDecode2 DecodeInfoCopy = apItem->Data;
	if (DecodeInfoCopy.nBPP<=8 && DecodeInfoCopy.ColorModel!=PVD_CM_PRIVATE)
	{
		if (!DecodeInfoCopy.pPalette)
		{
			DecodeInfoCopy.pPalette = g_Plugin.GetPalette(DecodeInfoCopy.nBPP, DecodeInfoCopy.ColorModel);
		}
	}
	DsCreate.pImage = &DecodeInfoCopy;
	DsCreate.BackColor = g_Plugin.BackColor();
	DsCreate.pDisplayContext = NULL;
	DsCreate.pFileName = apItem->pImage->pszFileNameOnly;
	//DsCreate.nChessMateColor1 = 0xFFFFFF;
	//DsCreate.nChessMateColor2 = 0xCCCCCC;
	//DsCreate.nChessMateSize = 8;
	
	WARNING("��������� ���� PVD_IDF_PRIVATE_DISPLAY. ���� ���������� - ����� �������� ������ ����� ������ ��������");
	
	CModuleInfo* lpDisplay = NULL;
	
	if ((apItem->Data.Flags & PVD_IDF_PRIVATE_DISPLAY) && (apItem->pFile->Decoder()->pPlugin->nCurrentFlags & PVD_IP_DISPLAY))
	{
		lpDisplay = apItem->pImage->mp_File->Decoder();
	}
	else
	{
		lpDisplay = CPVDManager::pDefaultDisplay;
	}
	CModuleInfo* pStartDisplay = lpDisplay;
	std::vector<CModuleInfo*>::iterator iDisp = CPVDManager::Displays.begin();
	BOOL lbFirstCheckDisplay = TRUE;

	// ���� g_Plugin.hWnd == 0 - ������ ���� ��� ����� Esc � ��� �����������
	while (g_Plugin.hWnd && iDisp != CPVDManager::Displays.end())
	{
		if (!lpDisplay->pPlugin)
		{
			_ASSERTE(lpDisplay->pPlugin);
			result = FALSE;
		}
		else
		{
			if (result = lpDisplay->pPlugin->DisplayCheck())
			{
				MCHKHEAP
				if (!g_Plugin.hWnd)
				{
					result = false;
					break;
				}
				if (result = lpDisplay->pPlugin->DisplayCreate2(&DsCreate))
					break;
				wchar_t szMsg[128];
				lstrcpy(szMsg, L"Display module skipped: ");
				lstrcpy(szMsg+lstrlen(szMsg), lpDisplay->pPlugin->pName);
				CPVDManager::AddLogString(L"PVDManager", szMsg);
			}
		}

		if ((apItem->Data.Flags & PVD_IDF_PRIVATE_DISPLAY) || CPVDManager::Displays.size()<=1)
			break;

		if (!lbFirstCheckDisplay)
			iDisp++;
		else
			lbFirstCheckDisplay = FALSE;
		// ����� ��������� ������� �������
		while (iDisp != CPVDManager::Displays.end()
			&& ((pStartDisplay == *iDisp) 
			    || ((*iDisp)->pPlugin == NULL)
			    || ((*iDisp)->nCurrentFlags & PVD_IP_PRIVATE)))
			iDisp++;
		if (iDisp == CPVDManager::Displays.end()) break;

		// ��������� ����� �������
		lpDisplay = *iDisp;
		wchar_t szMsg[128];
		lstrcpy(szMsg, L"Switching to display module: ");
		lstrcpy(szMsg+lstrlen(szMsg), lpDisplay->pPlugin->pName);
		CPVDManager::AddLogString(L"PVDManager", szMsg);
	}

	if (result)
	{
		apItem->pDraw->Assign(lpDisplay, DsCreate.pDisplayContext);
		//*ppDrawContext = DsCreate.pDisplayContext;

		// ����� ��������, ��� �����...
		apItem->pDraw->Params = apItem->Params;

		// ����������, � �� ��� ������ � pFile
		//_ASSERTE(apItem->Info.bPageInfoLoaded);
	}
	else
	{
		_ASSERTE("result==false" && 0);
		SafeRelease(apItem->pDraw,szPVDManager);
	}
	
	t5 = timeGetTime();

		//DecodeInfo.pImage, DecodeInfo.lImagePitch, DecodeInfo.nBPP, g_Plugin.BackColor, 
		//nFlags, DecodeInfo.nTransparentColor, DecodeInfo.pPalette);
		
	apItem->pImage->Info.lTimeTransfer = max(0,t5-t4); // ((t5==t4) ? 0 : max(0,t5-t4)) + ((t3==t1) ? 0 : max(0,t3-t2)); // CreateWorkSurface+Blit
		_ASSERTE(apItem->pImage->Info.lTimeTransfer>=0); if (apItem->pImage->Info.lTimeTransfer<0) apItem->pImage->Info.lTimeTransfer = 0;

	return result;
}

#if 0
bool CPVDManager::Decode(CImage *apImage, CDecoderHandle* pFile, CDisplayHandle** ppDraw, bool abResetTick)
{
	WARNING("� ��������� ���������� �������� ������ ������������ ��������. ���� - ����������� apImage->Info.nPage");
	WARNING("��������� ������� �� ��� ==> CPVDManager::Decode  &&  CPVDManager::TransferToDisplay");
	WARNING("CPVDManager::TransferToDisplay ������ ���������� � ���� �������!");
	bool result = false;

	if (!CPVDManager::pDefaultDisplay || !CPVDManager::pDefaultDisplay->pPlugin)
	{
		OutputDebugString(L"CPVDManager::Decode==> PVDManager::pDefaultDisplay->pPlugin was not created!");
		return false;
	}
	
	if (!pFile)
	{
		OutputDebugString(L"CPVDManager::Decode==> apImage->mp_File was not created!");
		return false;
	}
	_ASSERTE(pFile && pFile->Decoder() && pFile->Decoder()->pPlugin);
	
	if (!*ppDraw)
	{
		*ppDraw = new CDisplayHandle(apImage, pFile);
	}
	else
	{
		_ASSERTE((*ppDraw)->Context() == NULL);
		(*ppDraw)->Close();
	}
	
	MCHKHEAP

	CModuleInfo* lpDisplay = NULL;

	if (abResetTick) // ��� ������������� ��������� �������� �������� ��� (��� ������ ����� ��������)
		apImage->Info.lStartOpenTime = timeGetTime();
	DWORD t0 = apImage->Info.lStartOpenTime;
	DWORD t1 = timeGetTime(), t2=t1, t3=t1, t4=t1, t5=t1, t6=t1;

	wchar_t szLastError[128]; szLastError[0] = 0;
	wchar_t szDbg[MAX_PATH*3], *pNamePart;
	int nCur = 0, nSize = sizeofarray(szDbg);
	lstrcpyn(szDbg+nCur, pFile->Decoder()->pPlugin->pName, 128); nCur += lstrlen(szDbg+nCur);
	wsprintf(szDbg+nCur, L": Decoding page %i of image \"", apImage->Info.nPage); nCur += lstrlen(szDbg+nCur);
	const wchar_t* pFileName = (const wchar_t*)apImage->FileName;
	pNamePart = pFileName ? wcsrchr(pFileName, L'\\') : L"<NULL>";
	pNamePart = pNamePart ? (pNamePart+1) : (pFileName ? pFileName : L"<NULL>");
	lstrcpyn(szDbg+nCur, pNamePart, MAX_PATH); nCur += lstrlen(szDbg+nCur);
	lstrcpy(szDbg+nCur, L"\"� ");
	//wnsprintf(szDbg, sizeofarray(szDbg), L"%s: Decoding page %i of image %s...",
	//	pDecoder->pPlugin->pName, apImage->nPage, (const wchar_t*)apImage->FileName);
	OutputDebugString(szDbg);

	// �������
	pFile->Decoder()->pPlugin->szLastError[0] = 0;
	
	//pvdInfoPage2 InfoPage = {sizeof(pvdInfoPage2)};
	//TRY{
	
	bool bPageInfoOk = false;
	//if (apImage->Info.cbSize && apImage->Info.iPage == apImage->Info.nPage && apImage->Info.lWidth && apImage->Info.lHeight)
	if (pFile->Info.IsPageLoaded(apImage->Info.nPage))
	{
		bPageInfoOk = true;
	}
	else
	{
		pvdInfoPage2 InfoPage = {sizeof(pvdInfoPage2)};
		InfoPage.iPage = apImage->Info.nPage;
		bPageInfoOk = pFile->Decoder()->pPlugin->PageInfo2(pFile->Context(), &InfoPage);
		if (bPageInfoOk)
		{
			if (apImage->Info.pFormatName)
				lstrcpyn(apImage->FormatName, apImage->Info.pFormatName, sizeofarray(apImage->FormatName));
			if (apImage->Info.pCompression)
				lstrcpyn(apImage->Compression, apImage->Info.pCompression, sizeofarray(apImage->FormatName));
		}
		if (bPageInfoOk && apImage->Info.nPages)
			apImage->CheckPages(apImage->Info.nPages);
	}

	if (bPageInfoOk)
	{
		t2 = timeGetTime();
		pvdInfoDecode2 DecodeInfo = {sizeof(pvdInfoDecode2)};
		bool bNeedFree = true;
		TODO("����������� ��������� 24 ������ ����������� ��� ��������� �� �������������. ��� ����� ������ ����� !PageDecode2!");
		TODO("�������� ������� PageDecode2 � CreateWorkSurface. � ��������� ������� �� ������ �� ������ ��������� ����������� �������������� colormodel");
		WARNING("24 ������ ����������� ������ �� �������� � Primary surface");

		{
			t3 = timeGetTime();
			DecodeInfo.iPage = apImage->Info.nPage;

			DecodeInfo.Flags = (pFile->Decoder()->pPlugin == CPVDManager::pDefaultDisplay->pPlugin) ? PVD_IDF_ASDISPLAY : 0;
			DecodeInfo.nBackgroundColor = g_Plugin.BackColor();

			MCHKHEAP

			SIZE scaledSize = {0,0};
			if (
			    (pFile->Decoder()->pPlugin->nFlags & (PVD_IP_CANDESCALE|PVD_IP_CANUPSCALE)) &&
				(g_Plugin.ZoomAuto || g_Plugin.AbsoluteZoom || g_Plugin.FlagsWork & FW_QUICK_VIEW)
			   )
			{
				//TODO("����� ���������� �������");

				SIZE imgSize = {apImage->Info.lWidth,apImage->Info.lHeight};
				SIZE renderSize = apImage->GetDefaultScaleSize();

				//if (g_Plugin.FlagsWork & FW_QUICK_VIEW)
				if (renderSize.cx && renderSize.cy && imgSize.cx && imgSize.cy)
				{
					BOOL lbCanScale = FALSE;

					if (renderSize.cx < imgSize.cx && renderSize.cy < imgSize.cy)
					{
						lbCanScale = (pFile->Decoder()->pPlugin->nFlags & PVD_IP_CANDESCALE);
					}
					else if (renderSize.cx > imgSize.cx && renderSize.cy > imgSize.cy)
					{
						lbCanScale = (pFile->Decoder()->pPlugin->nFlags & PVD_IP_CANUPSCALE);
					}

					if (lbCanScale)
					{
						double dr = (double)renderSize.cx / (double)renderSize.cy;
						double di = (double)imgSize.cx / (double)imgSize.cy;
						if (g_Plugin.ZoomAuto == ZA_FIT)
						{
							if (di > dr)
							{
								// ������� �� ������
								scaledSize.cx = (LONG)(renderSize.cy * di);
								scaledSize.cy = renderSize.cy;
							}
							else
							{
								// ������� �� ������
								scaledSize.cx = renderSize.cx;
								scaledSize.cy = (LONG)(renderSize.cx / di);
							}
						}
						else if (g_Plugin.ZoomAuto == ZA_FILL)
						{
							if (di < dr)
							{
								// ������� �� ������
								scaledSize.cx = (LONG)(renderSize.cy * di);
								scaledSize.cy = renderSize.cy;
							}
							else
							{
								// ������� �� ������
								scaledSize.cx = renderSize.cx;
								scaledSize.cy = (LONG)(renderSize.cx / di);
							}
						}
						else if (di > dr)
						{
							// ������� �� ������
							scaledSize.cx = (LONG)(renderSize.cy * di);
							scaledSize.cy = renderSize.cy;
						}
						else
						{
							// ������� �� ������
							scaledSize.cx = renderSize.cx;
							scaledSize.cy = (LONG)(renderSize.cx / di);
						}
						DecodeInfo.lWidth = scaledSize.cx;
						DecodeInfo.lHeight = scaledSize.cy;
					}
				}
			}

			MCHKHEAP

			//_ASSERTE(g_Plugin.hWnd);
			if (!g_Plugin.hWnd || !IsWindow(g_Plugin.hWnd)) {
				lstrcpy(szLastError, L"PicView window was closed");
				result = false;
			} else
			if (pFile->Decoder()->pPlugin->PageDecode2(pFile->Context(), /*(pvdInfoDecode2*)*/ &DecodeInfo, DecodeCallback2, this))
			{
				MCHKHEAP

				if (DecodeInfo.Flags & PVD_IDF_ALPHA)
				{
					_ASSERTE(DecodeInfo.ColorModel == PVD_CM_BGRA);
				}
				
				// �������� ���������� � ������� �����������!
				if (DecodeInfo.lSrcWidth && DecodeInfo.lSrcHeight)
				{
					apImage->Info.lWidth = DecodeInfo.lSrcWidth;
					apImage->Info.lHeight = DecodeInfo.lSrcHeight;
				}
				apImage->Info.lDecodedWidth = DecodeInfo.lWidth;
				apImage->Info.lDecodedHeight = DecodeInfo.lHeight;
				apImage->Info.nDecodedBPP = DecodeInfo.nBPP;
				
				//apImage->Info.nBPP = DecodeInfo.nBPP; -- ��� �� ����. ����������� ����� ���� � 8-������, � ������ ������������ � 32 ����
				//
				if (DecodeInfo.pFormatName)
					lstrcpyn(apImage->FormatName, DecodeInfo.pFormatName, sizeofarray(apImage->FormatName));
				if (DecodeInfo.pCompression)
					lstrcpyn(apImage->Compression, DecodeInfo.pCompression, sizeofarray(apImage->Compression));
				if (!apImage->FormatName[0])
				{
					const wchar_t* pszExt = wcsrchr(pNamePart, L'.');
					if (pszExt)
					{
						apImage->FormatName[0] = L'[';
						lstrcpyn(apImage->FormatName+1, pszExt+1, sizeofarray(apImage->FormatName)-3);
						lstrcat(apImage->FormatName, L"]");
					}
				}
				
				MCHKHEAP

				t4 = timeGetTime();
				if (DecodeInfo.nPages)
					apImage->CheckPages(DecodeInfo.nPages);
				//DWORD nFlags = DecodeInfo.Flags;
				//if (nFlags < 2) {
				//	//nFlags |= (DecodeInfo.nBPP == 32) ? PVD_IDF_BGRA : PVD_IDF_BGR;
				//}
				TODO("����� �� 2-� ������ ����������");

				{
					//WARNING("�� ������������� � ��������������� ���������� ��� �� NULL");
					////_ASSERTE(*ppDrawContext == NULL);
					//if (*ppDrawContext) {
					//	// � ��� ������ ���. �� �������. � CImage ����� ���� ��������� DrawContext, ����� �� ������� ������...
					//	_ASSERTE(apImage->Display->pPlugin);
					//	apImage->Display->pPlugin->DisplayClose2(*ppDrawContext);
					//	*ppDrawContext = NULL;
					//}
					
					//result = dds->Blit(&DecodeInfo, g_Plugin.BackColor);
					pvdInfoDisplayCreate2 DsCreate = {sizeof(pvdInfoDisplayCreate2)};
					#ifdef _DEBUG
						if (DecodeInfo.ColorModel == PVD_CM_UNKNOWN)
						{
							if (lstrcmpi(pFile->Decoder()->pModuleFileName, L"GDIPlus.pvd"))
							{
								_ASSERTE(DecodeInfo.ColorModel != PVD_CM_UNKNOWN);
							}
						}
					#endif
					// ��������� �������� �� ���������� ������� ��� ��������������� ��������.
					// ����� ���������� ������� �� ���������, � ����� ������� �� �������� ����
					// ������ - � ������ ����� DecodeInfo
					pvdInfoDecode2 DecodeInfoCopy = DecodeInfo;
					if (DecodeInfoCopy.nBPP<=8 && DecodeInfoCopy.ColorModel!=PVD_CM_PRIVATE)
					{
						if (!DecodeInfoCopy.pPalette)
						{
							DecodeInfoCopy.pPalette = g_Plugin.GetPalette(DecodeInfoCopy.nBPP, DecodeInfoCopy.ColorModel);
						}
					}
					DsCreate.pImage = &DecodeInfoCopy;
					DsCreate.BackColor = g_Plugin.BackColor();
					DsCreate.pDisplayContext = NULL;
					DsCreate.pFileName = wcsrchr(pFileName, L'\\');
					if (DsCreate.pFileName) DsCreate.pFileName++; else DsCreate.pFileName = pFileName;
					//DsCreate.nChessMateColor1 = 0xFFFFFF;
					//DsCreate.nChessMateColor2 = 0xCCCCCC;
					//DsCreate.nChessMateSize = 8;
					
					WARNING("��������� ���� PVD_IDF_PRIVATE_DISPLAY. ���� ���������� - ����� �������� ������ ����� ������ ��������");
					
					if ((DecodeInfo.Flags & PVD_IDF_PRIVATE_DISPLAY) && (pFile->Decoder()->pPlugin->nCurrentFlags & PVD_IP_DISPLAY))
					{
						lpDisplay = apImage->mp_DecoderModule;
					}
					else
					{
						lpDisplay = CPVDManager::pDefaultDisplay;
					}
					CModuleInfo* pStartDisplay = lpDisplay;
					std::vector<CModuleInfo*>::iterator iDisp = CPVDManager::Displays.begin();
					BOOL lbFirstCheckDisplay = TRUE;

					// ���� g_Plugin.hWnd == 0 - ������ ���� ��� ����� Esc � ��� �����������
					while (g_Plugin.hWnd && iDisp != CPVDManager::Displays.end())
					{
						if (!lpDisplay->pPlugin)
						{
							_ASSERTE(lpDisplay->pPlugin);
							result = FALSE;
						}
						else
						{
							if (result = lpDisplay->pPlugin->DisplayCheck())
							{
								MCHKHEAP
								if (!g_Plugin.hWnd)
								{
									result = false;
									break;
								}
								if (result = lpDisplay->pPlugin->DisplayCreate2(&DsCreate))
									break;
								wchar_t szMsg[128];
								lstrcpy(szMsg, L"Display module skipped: ");
								lstrcpy(szMsg+lstrlen(szMsg), lpDisplay->pPlugin->pName);
								CPVDManager::AddLogString(L"PVDManager", szMsg);
							}
						}

						if ((DecodeInfo.Flags & PVD_IDF_PRIVATE_DISPLAY) || CPVDManager::Displays.size()<=1)
							break;

						if (!lbFirstCheckDisplay)
							iDisp++;
						else
							lbFirstCheckDisplay = FALSE;
						// ����� ��������� ������� �������
						while (iDisp != CPVDManager::Displays.end()
							&& ((pStartDisplay == *iDisp) 
							    || ((*iDisp)->pPlugin == NULL)
							    || ((*iDisp)->nCurrentFlags & PVD_IP_PRIVATE)))
						{
							iDisp++;
						}
						if (iDisp == CPVDManager::Displays.end())
							break;

						// ��������� ����� �������
						lpDisplay = *iDisp;
						wchar_t szMsg[128];
						lstrcpy(szMsg, L"Switching to display module: ");
						lstrcpy(szMsg+lstrlen(szMsg), lpDisplay->pPlugin->pName);
						CPVDManager::AddLogString(L"PVDManager", szMsg);
					}

					if (result)
					{
						(*ppDraw)->Assign(lpDisplay, DsCreate.pDisplayContext);
						//*ppDrawContext = DsCreate.pDisplayContext;
					}

						//DecodeInfo.pImage, DecodeInfo.lImagePitch, DecodeInfo.nBPP, g_Plugin.BackColor, 
						//nFlags, DecodeInfo.nTransparentColor, DecodeInfo.pPalette);
				}
				t5 = timeGetTime();
				bNeedFree = false;
				MCHKHEAP
				pFile->Decoder()->pPlugin->PageFree2(pFile->Context(), &DecodeInfo);
			}
		}
		if (!g_Plugin.hWnd || !IsWindow(g_Plugin.hWnd))
		{
			lstrcpy(szLastError, L"PicView window was closed");
			result = false;
		}
		MCHKHEAP
		if (result)
		{
			apImage->Info.lWidth = apImage->Info.lWidth;
			apImage->Info.lHeight = apImage->Info.lHeight;
			apImage->Info.nBPP = apImage->Info.nBPP;
			if (apImage->Info.Animation && !apImage->Info.lFrameTime)
				apImage->Info.lFrameTime = 100; // ����������� ������������� ����� ������
			apImage->Info.lFrameTime = apImage->Info.lFrameTime;
		}
	}
	t6 = timeGetTime();
	apImage->Info.lTimeOpen = t2-t0; //�������� ����� ����������� � ��������� ���������� � ��� (Open & PageInfo)
		_ASSERTE(apImage->Info.lTimeOpen>=0); if (apImage->Info.lTimeOpen<0) apImage->Info.lTimeOpen = 0;
	apImage->Info.lTimeDecode = (t4==t1) ? 0 : (t4-t3); // PageDecode
		_ASSERTE(apImage->Info.lTimeDecode>=0); if (apImage->Info.lTimeDecode<0) apImage->Info.lTimeDecode = 0;
	apImage->Info.lTimeTransfer = ((t5==t1) ? 0 : max(0,t5-t4)) + ((t3==t1) ? 0 : max(0,t3-t2)); // CreateWorkSurface+Blit
		_ASSERTE(apImage->Info.lTimeTransfer>=0); if (apImage->Info.lTimeTransfer<0) apImage->Info.lTimeTransfer = 0;
	apImage->Info.lTimePaint = 0;
	apImage->Info.bTimePaint = FALSE;
	//wsprintf(apImage->OpenTimes, L"%i+%i+%i", 
	//	t2-t0, //�������� ����� ����������� � ��������� ���������� � ��� (Open & PageInfo)
	//	(t4==t1) ? 0 : t4-t3, // PageDecode
	//	((t5==t1) ? 0 : t5-t4) + ((t3==t1) ? 0 : t3-t2) // CreateWorkSurface+Blit
	//	);
	apImage->Info.lOpenTime = t6-apImage->Info.lStartOpenTime;
	//}CATCH{}

	MCHKHEAP
	OutputDebugString(result ? L"Succeeded\n" : L"Failed!!!\n");
	if (!result) {
		LPCWSTR pszError = NULL;
		if (szLastError[0])
		{
			pszError = szLastError;
		}
		else if (pFile->Decoder()->pPlugin->szLastError[0])
		{
			pszError = pFile->Decoder()->pPlugin->szLastError;
		}
		else
		{
			pszError = L"Failed!!! ";
		}
		if (pszError)
		{
			int nLen = lstrlen(szDbg);
			if ((nLen+10) < sizeofarray(szDbg))
			{
				lstrcpyn(szDbg+nLen, pszError, sizeofarray(szDbg)-nLen-1);
			}
		}
		pFile->Decoder()->pPlugin->SetStatus(szDbg);
	}

	return result;
}
#endif

bool CPVDManager::DisplayAttach()
{
	if (!CPVDManager::pActiveDisplay)
		return FALSE;
	if (!CPVDManager::pActiveDisplay->pPlugin)
		return FALSE;

	return CPVDManager::pActiveDisplay->pPlugin->DisplayAttach();
}

bool CPVDManager::DisplayDetach(BOOL abErase/*=TRUE*/)
{
	bool lbRc = false;
	if (CPVDManager::pActiveDisplay && CPVDManager::pActiveDisplay->pPlugin)
	{
		 lbRc = CPVDManager::pActiveDisplay->pPlugin->DisplayDetach();
	}
	if (abErase)
		CPVDManager::DisplayErase();
	return lbRc;
}

void CPVDManager::DisplayExit()
{
	_ASSERTE(GetCurrentThreadId() == gnDisplayThreadId || gnDisplayThreadId == 0);

	CPVDManager::DisplayDetach(FALSE);

	std::vector<CModuleInfo*>::iterator i;
	CModuleInfo *tmp;

	// ������� ����������� ������� ��� ���� ���������
	g_Panel.CloseDisplayHandles();

	for (i=Displays.begin(); i!=Displays.end(); i++)
	{
		tmp = *i;
		if (!tmp->pPlugin) continue;
		if (tmp->pPlugin->bDisplayInitialized)
		{
			tmp->pPlugin->DisplayExit2();
		}
	}

	if (g_Plugin.hWnd)
		DisplayErase();
}

// � ����������������� ���������, ������� (Win7 �� ������� ����)
// �� �������� ������� ������ ������ �������, �� ������� ���������.
// http://forum.farmanager.com/viewtopic.php?p=63985#p63985
void CPVDManager::DisplayErase()
{
	if (g_Plugin.hConEmuWnd || (g_Plugin.FlagsWork & FW_QUICK_VIEW))
		return;

	if (g_Plugin.hWnd && IsWindow(g_Plugin.hWnd) && IsWindowVisible(g_Plugin.hWnd))
	{
		HDC hdc = GetDC(g_Plugin.hWnd);
		RECT rc; GetClientRect(g_Plugin.hWnd, &rc);
		FillRect(hdc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
		ReleaseDC(g_Plugin.hWnd, hdc);
	}
	#ifdef _DEBUG
	else
	{
		_ASSERTE(g_Plugin.hWnd && IsWindow(g_Plugin.hWnd));
	}
	#endif
}

void CPVDManager::DecoderExit()
{
	// ���������� ���� ��������, �������� ������� �������������
	_ASSERTE((g_Plugin.FlagsWork & FW_TERMINATE) == FW_TERMINATE);
	g_Plugin.FlagsWork |= FW_TERMINATE;


	if (gnDecoderThreadId && GetCurrentThreadId() == gnDecoderThreadId)
	{
		// �� � ���� ��������, ����� ��������� �����������!
		g_Panel.CloseImageHandles();
	}
	else 
	{
		// ��������� ����������� ������ - �������� ����� ���������...
		if (g_Manager.mh_Thread)
		{
			_ASSERTE(g_Manager.mh_Request!=NULL);
			SetEvent(g_Manager.mh_Request);
			DWORD nWait = 0;
			if ((nWait = WaitForSingleObject(g_Manager.mh_Thread, 30000)) == WAIT_TIMEOUT)
			{
				TerminateThread(g_Manager.mh_Thread, 100);
			}
			CloseHandle(g_Manager.mh_Thread); g_Manager.mh_Thread = NULL;
		}
	}

	memset(g_Manager.m_DecoderQueue, 0, sizeof(g_Manager.m_DecoderQueue));
}

void CPVDManager::OnTerminate()
{
	// ���������� ���� ��������, �������� ������� �������������
	// (g_Plugin.FlagsWork == 0), ���� ������ �� ���� �� �������������
	_ASSERTE(((g_Plugin.FlagsWork & FW_TERMINATE) == FW_TERMINATE) || g_Plugin.FlagsWork == 0);
	g_Plugin.FlagsWork |= FW_TERMINATE;


	//if (GetCurrentThreadId() == g_Manager.mn_ThreadId)
	//	return; // �� � ��� � ���� ��������!


	MCHKHEAP;
	DecoderExit();
	DisplayExit();
	MCHKHEAP;
}

void CPVDManager::OnFirstImageOpened(CImage *apImage)
{
	//if (!(g_Plugin.FlagsWork & FW_FIRST_CALL))
	//{
	//	_ASSERTE((g_Plugin.FlagsWork & FW_FIRST_CALL) == FW_FIRST_CALL);
	//	return;
	//}

	if ((g_Plugin.FlagsWork & FW_TERMINATE))
		return;

	// ���� ���� ��������� ��� �� ���� �������� - ������ ����� ��������
	// ��� �� �������� ������ ������� �����-�� ���������
	if (!g_Plugin.hWnd || !IsWindow(g_Plugin.hWnd))
	{
		_ASSERTE(g_Plugin.hWnd && IsWindow(g_Plugin.hWnd));
	}
	else
	{
		if (!IsWindowVisible(g_Plugin.hWnd) && !(g_Plugin.FlagsWork & FW_TERMINATE))
			ShowWindowAsync(g_Plugin.hWnd, SW_SHOWNORMAL);		
	}


	// ��� ������ �������� ����� - �(�)������� (���� ���������) �����������
	// ����������� ���������� PgUp/PgDn (������� �������� ��� �����)
	g_Plugin.bAutoPagingChanged = 0;
	if (!(g_Plugin.FlagsWork & FW_QUICK_VIEW) && g_Plugin.bAutoPagingSet)
	{
		BOOL bChanged = FALSE;
		SHORT CurState = GetKeyState(g_Plugin.nAutoPagingVK);
		if (!apImage->Info.Animation && apImage->Info.nPages>1) {
			// �������� ScrollLock (PgUp/PgDn ������� ��������)
			if ((CurState & 1) == 0)
				bChanged = TRUE;
		} else {
			// ��������� ScrollLock (PgUp/PgDn ������� �����)
			if ((CurState & 1) != 0)
				bChanged = TRUE;
		}
		if (bChanged) {
			g_Plugin.bAutoPagingChanged = ((CurState & 1) == 0) ? 1 : 2;
			keybd_event((BYTE)g_Plugin.nAutoPagingVK, (BYTE)MapVirtualKey(g_Plugin.nAutoPagingVK, 0), KEYEVENTF_EXTENDEDKEY, 0);
			keybd_event((BYTE)g_Plugin.nAutoPagingVK, (BYTE)MapVirtualKey(g_Plugin.nAutoPagingVK, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
		}
	}
}


/* ******************************* */
/* ��������� ������� ������������� */
/* ******************************* */

CImage* CPVDManager::GetDecodedImage(DecodeParams *apParams)
{
	CImage* pImage = GetImageFromParam(apParams);
	if (!pImage)
	{
		TODO("�������� ������?");
		RequestTerminate();
		return NULL;
	}

	BOOL result = IsImageReady(apParams);

	//WARNING("����� ����, ��� mp_File ��� ������, � ������� - �����������?");
	//if (pImage->mp_File->IsReady())
	//{
	//	CDisplayHandle* pDraw = pImage->GetDrawHandle(apParams);
	//	if (pDraw)
	//	{
	//		WARNING("��� ����� ����������/��������/Refresh � �.�. ����� ��������� ��������� �������������");
	//		_ASSERTE(pImage->mp_File->Params.Compare(apParams));

	//		SafeRelease(pDraw);
	//		result = true;
	//	}
	//}
	
	if (!result)
	{
		apParams->Priority = eCurrentImageCurrentPage;
		apParams->Flags |= eRenderWaitOpened;

		result = RequestDecodedImage(apParams);
	}

	return result ? pImage : NULL;
}

int CPVDManager::AppendDecoderQueue(DecodeParams *apParams)
{
	HANDLE hReadyEvent = NULL;

#if 0
	// �� �����, ��� ����� �������... �� ����� �����
	// ��������� � ������ �������� ����� �� ������ ������
	if (GetCurrentThreadId() == mn_ThreadId)
	{
		_ASSERTE(GetCurrentThreadId() != mn_ThreadId);
		RequestTerminate();
		return -1;
	}
#endif

	EnterCriticalSection(&csDecoderQueue);

	int idx = -1;

	// ���� ����� ������ ��� ������ - ������ �������� ��� ���������/�����?
	for (int i = 0; i < sizeofarray(m_DecoderQueue); i++)
	{
		if (m_DecoderQueue[i].Status != eItemEmpty && m_DecoderQueue[i].Status != eItemFailed)
		{
			if (m_DecoderQueue[i].Params.Compare(apParams))
			{
				if (m_DecoderQueue[i].Status == eItemRequest)
				{
					idx = i;
					// ����� ���� ���������� ���������
					m_DecoderQueue[i].Params.Flags = apParams->Flags;
					m_DecoderQueue[i].Params.Priority = apParams->Priority;
					LeaveCriticalSection(&csDecoderQueue);
					return idx;
				}				 
			}
		}
	}

	for (int s = eItemEmpty; (idx == -1) && (s <= eItemReady); s++)
	{
		for (int i = 0; i < sizeofarray(m_DecoderQueue); i++)
		{
			// ����� ������ ���������, ��� ����� ������������ ������
			if (m_DecoderQueue[i].Status == s
				&& (s != eItemRequest
					|| apParams->Priority < m_DecoderQueue[i].Params.Priority))
			{
				idx = i; break;
			}
		}
	}

	if (idx == -1)
	{
		_ASSERTE(idx != -1);
		LeaveCriticalSection(&csDecoderQueue);
		RequestTerminate();
		return -1;
	}

	if (m_DecoderQueue[idx].Status != eItemEmpty)
	{
		TODO("���� ����� - �����-�� �������� �� ������ �������� �������");
	}

	if (m_DecoderQueue[idx].hReady == NULL)
		m_DecoderQueue[idx].hReady = CreateEvent(NULL, TRUE, FALSE, NULL);

	hReadyEvent = m_DecoderQueue[idx].hReady;
	ResetEvent(hReadyEvent);

	m_DecoderQueue[idx].Status = eItemRequest;

	// 'operator =' ����������� ������ ������������ ����� �����!
	m_DecoderQueue[idx].Params = *apParams;
	
	// ���������, ������������ ������ CPVDManager ����� ��������� �������!
	m_DecoderQueue[idx].Params.pResult = apParams->pResult;
	_ASSERTE(m_DecoderQueue[idx].pBuf == NULL);
	m_DecoderQueue[idx].pBuf = NULL;
	m_DecoderQueue[idx].lBuf = 0;

	SetEvent(mh_Request);

	LeaveCriticalSection(&csDecoderQueue);

	return idx;
}

bool CPVDManager::IsImageReady(DecodeParams *apParams)
{
	bool lbReady = false;
	
	// ��������, ��� ����������� ��� ������������ � ���������� �����������
	CImage* pImage = GetImageFromParam(apParams);
	if (!pImage)
	{
		_ASSERTE(pImage!=NULL);
	}
	else
	{
		TODO("���������, � ������ ����������� ���� ������������ ����������� (��������, �������, �������)!");
		TODO("������-�� ���������� ������� ����� � ���� ������� ���������, ������� ��� ����� ��� �� ����");
		WARNING("�������� (pImage->nPage == apParams->nPage) ���������� - � pImage ����� ��������� ����� ����� ��������!");
		WARNING("������-�� ���������� ������� mp_Draw ����� � ���� ������� ���������, ������� ��� ����� ��� �� ����");

		WARNING("����� ����, ��� mp_File ��� ������, � ������� - �����������?");
		if (pImage->mp_File->IsReady() /*&& pImage->Info.nPage == apParams->nPage*/)
		{
			CDisplayHandlePtr rDraw;
			if (pImage->GetDrawHandle(rDraw, apParams))
			{
				WARNING("��� ����� ����������/��������/Refresh � �.�. ����� ��������� ��������� �������������");
				//_ASSERTE(pImage->mp_File->Params.Compare(apParams));
				_ASSERTE(rDraw->Params.Compare(apParams));

				lbReady = true;
			}
		}
	}
	
	return lbReady;
}

CImage* CPVDManager::GetImageFromParam(DecodeParams *apParams)
{
	if (!apParams)
	{
		return NULL;
	}

	CImage* pImage = NULL;

	// ���� ������ �� ����������� �������
	if (!(apParams->Flags & eRenderRelativeIndex))
	{
		_ASSERTE(apParams->nRawIndex!=-1); // ���������� ������� �������, �� ���� ��?
		
		// ���������� NULL, ���� ������� �� �������� (��� bPicDisabled)!
		pImage = g_Panel.GetImage(apParams->nRawIndex);
		// ������ �������. ������ ��� ������ ������������� �����,
		// ������� ���� �� �� ���������� ��������!
		_ASSERTE(pImage != NULL);
	}
	else //if ((apParams->Flags & eRenderRelativeIndex))
	{
		// ������ ������������ �������� (����� ��� ����)
		_ASSERTE((apParams->Flags & eRenderRelativeIndex) == eRenderRelativeIndex);
		_ASSERTE(apParams->iDirection == -1 || apParams->iDirection == 1);
		
		// ���������� ������ "���������" ��������, � ������� (bPicDisabled == false)
		int nNextRawIdx = g_Panel.GetNextItemRawIdx(apParams->iDirection, apParams->nFromRawIndex);
		if (nNextRawIdx >= 0)
		{
			if ((pImage = g_Panel.GetImage(nNextRawIdx)) != NULL)
			{
				// ��������� � ->nRawIndex - ����� ����������� ��������
				// � ->nFromRawIndex ���� �������� ������ �������� (�� ���� �������)
				apParams->nRawIndex = nNextRawIdx;
			}
		}
	}
	
	return pImage;
}

BOOL CPVDManager::RequestDecodedImage(DecodeParams *apParams)
{
	TODO("������ �� ������������ �����������");
	_ASSERTE((apParams->Flags & eRenderReleaseDescriptor)==0);

	if (!CheckDecodingThread())
	{
		// RequestTerminate() ��� ������
		return FALSE;
	}
		
	if ((apParams->Flags & eRenderRelativeIndex))
	{
		_ASSERTE(apParams->nRawIndex == -1);
		_ASSERTE(apParams->nFromRawIndex >= 0);
		if (apParams->iDirection == 0)
		{
			_ASSERTE(apParams->iDirection == -1 || apParams->iDirection == 1);
			RequestTerminate();
			return FALSE;
		}
		if (apParams->nFromRawIndex == -1)
			apParams->nFromRawIndex = g_Panel.GetActiveRawIdx();
		if (apParams->nRawIndex == -1)
			apParams->nRawIndex = g_Panel.GetNextItemRawIdx(apParams->iDirection, apParams->nFromRawIndex);
			//apParams->nRawIndex = g_Panel.GetActiveRawIdx();
		//apParams->nFromRawIndex = apParams->nRawIndex;
	}
	else if ((apParams->Flags & eRenderFirstAvailable))
	{
		_ASSERTE(apParams->nRawIndex >= 0);
		if (apParams->nRawIndex == -1)
			apParams->nRawIndex = g_Panel.GetActiveRawIdx();
		apParams->nFromRawIndex = apParams->nRawIndex;
	}
	
	if ((apParams->Flags & (eRenderFirstAvailable|eRenderRelativeIndex)) && apParams->iDirection == 0)
	{
		_ASSERTE((apParams->Flags & eRenderRelativeIndex)==0 || (apParams->iDirection!=0));
		apParams->Flags &= ~(eRenderFirstAvailable|eRenderRelativeIndex);
	}

	// ��� ������� �� ������ ��������� ����� �� ������!
	CImage* pImage = GetImageFromParam(apParams);
	if (!pImage && (apParams->Flags & eRenderFirstAvailable) && apParams->iDirection)
	{
		_ASSERTE(apParams->iDirection==-1 || apParams->iDirection==1);
		int iNext;
		while ((iNext = g_Panel.GetNextItemRawIdx(apParams->iDirection, apParams->nRawIndex)) != -1)
		{
			apParams->nRawIndex = iNext;
			pImage = GetImageFromParam(apParams);
			if (pImage)
				break;
		}
	}
	if (!pImage)
	{
		//_ASSERTE(pImage != NULL);
		//RequestTerminate(); -- ������� ����� ���� NULL, ����� ��������� �� ���������� ��������, � �������� Loop
		return FALSE;
	}
	

#ifdef PICVIEWDUMP
	wchar_t szDbg[MAX_PATH*3], *pNamePart;
	int nCur = 0, nSize = sizeofarray(szDbg);
	lstrcpy(szDbg+nCur, L"Image page requested: \""); nCur += lstrlen(szDbg+nCur);
	const wchar_t* pFileName = (const wchar_t*)pImage->FileName;
	pNamePart = pFileName ? wcsrchr(pFileName, L'\\') : L"<NULL>";
	pNamePart = pNamePart ? (pNamePart+1) : (pFileName ? pFileName : L"<NULL>");
	lstrcpyn(szDbg+nCur, pNamePart, MAX_PATH+1); nCur += lstrlen(szDbg+nCur);
	wsprintfW(szDbg+nCur, L"\" Page=%u, RawIdx=%u\n", apParams->nPage, apParams->nRawIndex);
	//wnsprintf(szDbg, sizeofarray(szDbg), L"%s: Opening image %s...", pDecoder->pPlugin->pName, pFileName ? pFileName : L"<NULL>");
	OutputDebugString(szDbg);
#endif

		
	// ��������, ��� ����������� ��� ������������ � ���������� �����������
	if (IsImageReady(apParams))
	{
		if (apParams->Flags & eRenderActivateOnReady)
			apParams->OnItemReady();

		return TRUE;
	}
	if (g_Plugin.FlagsWork & FW_TERMINATE)
	{
		// RequestTerminate() ��� ������
		return FALSE;
	}
	
	DecodingStatus result = eItemFailed;
	if ((apParams->Flags & eRenderWaitOpened))
	{
		apParams->pResult = &result;
	}

	int idx = AppendDecoderQueue(apParams);
	if (idx < 0)
	{
		_ASSERTE(idx >= 0);
		// RequestTerminate() ��� ������
		return FALSE;
	}

	BOOL lbRc = TRUE;

	if ((apParams->Flags & eRenderWaitOpened))
	{
		TODO("������������� ����������� ������ �������� / �������� �������������");
		HANDLE hReady = m_DecoderQueue[idx].hReady;
		DWORD nWait = WaitForSingleObject(hReady, 250);
		while (nWait == WAIT_TIMEOUT)
		{
			if ((g_Plugin.FlagsWork & FW_TERMINATE))
				break;
			nWait = WaitForSingleObject(hReady, 250);
		}
		// ��������� ���������
		lbRc = (nWait == WAIT_OBJECT_0) && (result == eItemReady);
			///*&& (m_DecoderQueue[idx].nRawPanelItem == aiRawIndex)*/
			//&& (m_DecoderQueue[idx].Status == eItemReady)
			//&& (m_DecoderQueue[idx].Params.nPage == apParams->nPage);
		//// � ����� �������� ������ ��� �� ������������
		//m_DecoderQueue[idx].Status = eItemEmpty;
	}

	return lbRc;
}

void CPVDManager::RequestPreCache(DecodeParams *apParams)
{
	if (!apParams)
	{
		_ASSERTE(apParams!=NULL);
		return;
	}

	if (!CheckDecodingThread())
	{
		// RequestTerminate() ��� ������
		return;
	}

	CImage* pImage = GetImageFromParam(apParams);

	// ����� ��������� � ������� �� ������������� ��������� ����� (Priority = eCurrentImageNextPage)!
	if (pImage)
	{
		// ��, ���� �� ���� �������� ������ ����. ��������� ����������...
		_ASSERTE(pImage->Info.nPages > 0);

		TODO("��� �� ���� ����������� �������� ������� ����������, ��� � �������");

		if (pImage->Info.nPages > 1)
		{
			// ��������� � ������� �� ������������� ��������� �����
			if ((apParams->nPage + 1) < pImage->Info.nPages)
			{
				DecodeParams next;
				next.Priority = eCurrentImageNextPage;
				next.nPage = apParams->nPage + 1; // ��������� �����
				next.nRawIndex = apParams->nRawIndex;
				
				// RequestDecodedImage - �������� ������ � ������� �������������,
				// GetDecodedImage - ���������� ���������� �������������
				g_Manager.RequestDecodedImage(&next);
			}

			// ... � ���������� �����
			if (apParams->nPage > 0)
			{
				DecodeParams next;
				next.Priority = eCurrentImageNextPage;
				next.nPage = apParams->nPage - 1; // ���������� ����� (����� ���� ��� ��������, �� ���� ������� �������� ����� - ����� � ���)
				next.nRawIndex = apParams->nRawIndex;
				
				// RequestDecodedImage - �������� ������ � ������� �������������,
				// GetDecodedImage - ���������� ���������� �������������
				g_Manager.RequestDecodedImage(&next);
			}

			TODO("������ �� ����������� ������������� ���, �� +1 ��� ������, � +2");
		}
	}
	
	// 
	bool lbNeedCache = (g_Panel.IsRealNames()) ? g_Plugin.bCachingRP : g_Plugin.bCachingVP;
	if (lbNeedCache)
	{
		// Next (��� �� ����������� �����������)
		{
			DecodeParams next;
			next.Priority = eDecodeNextImage; // ��������� ������, �� ������� �����������
			next.nPage = 0; // ������ �����
			next.nRawIndex = -1; // relative
			next.nFromRawIndex = apParams->nRawIndex;
			next.Flags = eRenderRelativeIndex;
			next.iDirection = (apParams->iDirection ? apParams->iDirection : 1);
			g_Manager.RequestDecodedImage(&next);
		}

		// Prev (��� ������ ����������� �����������)
		{
			DecodeParams next;
			next.Priority = eDecodeNextImage; // ��������� ������, �� ������� �����������
			next.nPage = 0; // ������ �����
			next.nRawIndex = -1; // relative
			next.nFromRawIndex = apParams->nRawIndex;
			next.Flags = eRenderRelativeIndex;
			next.iDirection = -(apParams->iDirection ? apParams->iDirection : 1);
			g_Manager.RequestDecodedImage(&next);
		}

		TODO("������ �� ����������� ������������� ���, �� +1 ��� ������, � +2");
	}
}

DWORD CPVDManager::DecodingThread(LPVOID)
{
	int iRc = 0;
	BOOL lbStepRc = FALSE, lbException = FALSE;
	
	gnDecoderThreadId = GetCurrentThreadId(); // �������������

	while (!(g_Plugin.FlagsWork & FW_TERMINATE))
	{
		if (g_Manager.mh_Alive) SetEvent(g_Manager.mh_Alive);
		g_Manager.mn_AliveTick = GetTickCount();
		WaitForSingleObject(g_Manager.mh_Request, 100);

		if ((g_Plugin.FlagsWork & FW_TERMINATE))
			break; // ���������� �������

		TRY
		{
			lbStepRc = g_Manager.DecodingThreadStep();
		}
		CATCH
		{
			lbException = TRUE;
		}

		if (lbException)
		{
			iRc = 100;
			CFunctionLogger::FunctionLogger(L"!!! Exception in CPVDManager::DecodingThread");
			RequestTerminate();
			MessageBox(NULL, L"!!! Exception in CPVDManager::DecodingThread\nPlease, restart FAR", L"PicView3", MB_OK|MB_ICONSTOP|MB_SETFOREGROUND|MB_SYSTEMMODAL);
			break;
		}

		if (!lbStepRc)
		{
			// RequestTerminate ��� ������ ���� ������
			iRc = 100;
			break;
		}
	}

	// �������� ��������� - ������ ����������� � ���� ����!
	CPVDManager::DecoderExit();
	
	// ���� ����, ��� ���� ��������� �����������
	gnDecoderThreadId = 0;
	
	return iRc;
}

BOOL CPVDManager::DecodingThreadStep()
{
	int idx = -1;
	EnterCriticalSection(&g_Manager.csDecoderQueue);
	
	int iActiveRawIndex = g_Panel.GetActiveRawIdx();


	// ���������� �������������� �����������
	std::vector<CDecoderHandle*>::iterator f = m_DecoderRelease.begin();
	while (f != m_DecoderRelease.end())
	{
		CDecoderHandle* p = *f;
		SafeRelease(p,szPVDManager);
		f = m_DecoderRelease.erase(f);
	}


	for (int p = eCurrentImageCurrentPage; (idx == -1) && (p <= eDecodeAny); p++)
	{
		for (int i = 0; i < sizeofarray(m_DecoderQueue); i++)
		{
			if (p <= eCurrentImageNextPage && iActiveRawIndex != m_DecoderQueue[i].Params.nRawIndex)
				continue; // ��� ��� �� ������� �����������
			
			if (m_DecoderQueue[i].Status == eItemRequest
				&& (m_DecoderQueue[i].Params.Priority == p || p == eDecodeAny))
			{
				idx = i; break;
			}
		}
	}

	// ������ ����� ������ ���������, � ������ ���������� �������������
	LeaveCriticalSection(&csDecoderQueue); 

	// ���� ���� �����-�� �������
	if (idx == -1)
		return TRUE; // ���������� ��������� � ����!

	TODO("������ �� ������������ �����������");
	_ASSERTE((m_DecoderQueue[idx].Params.Flags & eRenderReleaseDescriptor)==0);

	BOOL lbRc = TRUE;
	bool lbDecodeResult = false;

	CImage* pImage = GetImageFromParam(&m_DecoderQueue[idx].Params);
lTryAgain:
	if (!pImage)
	{
		_ASSERTE(pImage != NULL);
		m_DecoderQueue[idx].Status = eItemFailed;
		SetEvent(m_DecoderQueue[idx].hReady);
		RequestTerminate();
		lbRc = FALSE; // ���������� ����
	}
	else
	{

		// ������ �������� �����, ��� ����� ������ ���� ������� ��������.
		if (m_DecoderQueue[idx].Params.Flags & eRenderResetDecoder)
		{
			pImage->Close();
			//pImage->ResetProcessed(); ==> ResetDecoderFailed()
			pImage->ResetDecoderFailed();
		}
		else
		{
			// ����������� ��� ����� ���� ������������ � ���������� �����������
			if (IsImageReady(&m_DecoderQueue[idx].Params))
				lbDecodeResult = true;
		}

		// ���� ��������� ������������� (��� �� ���� ������� �����)
		if (!lbDecodeResult)
		{
			CFunctionLogger::FunctionLogger(L"DecodingThread->ImageOpen(%s)",(LPCWSTR)pImage->FileName);
			lbDecodeResult = ImageOpen(pImage, &m_DecoderQueue[idx].Params);
			//CImage* pRc = g_Manager.GetDecodedImage(&m_DecoderQueue[idx].Params);
			//lbDecodeResult = (pRc != NULL);
			CFunctionLogger::FunctionLogger(L"ImageOpen done");

			if (!lbDecodeResult)
				g_Panel.SetItemDisabled(pImage->PanelItemRaw());
			
			if (!lbDecodeResult && (m_DecoderQueue[idx].Params.Flags & eRenderFirstAvailable))
			{
				// �������� ������, "�� ���� �������"
				m_DecoderQueue[idx].Params.nFromRawIndex = m_DecoderQueue[idx].Params.nRawIndex;
				// ���������� ���� "������������ �������"
				m_DecoderQueue[idx].Params.Flags |= eRenderRelativeIndex;
				// � ��������� "�����������", ���� ����� - ������ "����"
				if (m_DecoderQueue[idx].Params.iDirection == 0) m_DecoderQueue[idx].Params.iDirection = 1;
				
				// ������� �������� ������ �� ��������� ���� (CImage)
				if ((pImage = GetImageFromParam(&m_DecoderQueue[idx].Params)) != NULL)
					goto lTryAgain;

				// ������ �� ������, lbDecodeResult ��������� false
			}
		}

		TODO("������� ���� �������, ����� �� (� ����) ������� ������ �� �������� � �������?");
		TODO("��� �����, ��������� � ����� ����, ������� ����� �������������� ��� �������");
		if (lbDecodeResult /*&& (m_DecoderQueue[idx].Params.Flags & eRenderActivateOnReady)*/)
		{
			m_DecoderQueue[idx].Params.OnItemReady();
		}
		else
		{
			SafeRelease(m_DecoderQueue[idx].Params.pDecodeItem,szPVDManager);
		}
		
		if (m_DecoderQueue[idx].Params.pResult && !(g_Plugin.FlagsWork & FW_TERMINATE))
			*m_DecoderQueue[idx].Params.pResult = lbDecodeResult ? eItemReady : eItemFailed;

		m_DecoderQueue[idx].Status = lbDecodeResult ? eItemReady : eItemFailed;
		SetEvent(m_DecoderQueue[idx].hReady);

		lbRc = TRUE; // ���������� ��������� � ����!
	}

	return lbRc;
}

bool CPVDManager::CheckDecodingThread()
{
	DWORD nWait;

	if (mh_Thread)
	{
		nWait = WaitForSingleObject(mh_Thread, 0);
		if (nWait == WAIT_OBJECT_0)
		{
			CloseHandle(mh_Thread); // ���� ���� ���������!
			mh_Thread = NULL;
		}
	}

	if (!mh_Request) mh_Request = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (!mh_Alive) mh_Alive = CreateEvent(NULL, TRUE, FALSE, NULL);

	ResetEvent(mh_Alive);

	if (mh_Thread == NULL)
	{
		mh_Thread = CreateThread(NULL, 0, DecodingThread, NULL, 0, &mn_ThreadId);
		if (!mh_Thread)
		{
			_ASSERTE(mh_Thread != NULL || (g_Plugin.FlagsWork & FW_TERMINATE));
			RequestTerminate();
			return false;
		}

		HANDLE hEvents[2] = {mh_Alive,mh_Thread};
		nWait = WaitForMultipleObjects(2, hEvents, FALSE, 10000);
		if (nWait != WAIT_OBJECT_0)
		{
			_ASSERTE(nWait == WAIT_OBJECT_0 || (g_Plugin.FlagsWork & FW_TERMINATE));
			RequestTerminate();
			return false;
		}
	} else {
		TODO("���������, �� ������� �� ���� �������������");
	}

	return TRUE;
}

bool CPVDManager::ImageOpen(CImage* apImage, DecodeParams *apParams)
{
	_ASSERTE(apImage && !apImage->FileName.IsEmpty() && apParams);

	//// ������ ���� ��� ������, �� ��������
	//g_Plugin.InitCMYK(FALSE); // ��������� ��� ����������

	i64 lFileSize;
	const unsigned char *buf = NULL; int lBuf = 0;
	FileMap Map((LPCWSTR)apImage->FileName);
	lFileSize = Map.lSize;
	if (Map.MapView())
	{
		buf = Map.pMapping;
		lBuf = (int)Min<i64>(Map.lSize, ~0u);
		//if (lBuf) -- IN_APL ����� �� �������
		//	lFileSize = 0; // ��� ������ ��� � ������
	}


	bool result = false;

	TODO("��������� � ������ �������");
	apImage->Info.lStartOpenTime = timeGetTime();

	if (apParams->pDecodeItem == NULL)
	{
		apParams->pDecodeItem = new CDecodeItem(szPVDManager, apImage);
		apParams->pDecodeItem->Params = *apParams;
	}

	// ���������� ������! �� ������ ����� ��������� � ������ AltPgUp �������� (����� ��������)
	//Decoder->mi_DecoderIndex = *piSubDecoder;
	
lReOpen2:
	//memset(&InfoPage, 0, sizeof(InfoPage)); // !!! cbSize �� ���������
	result = Open(apImage, &apParams->pDecodeItem->pFile, (LPCWSTR)apImage->FileName, lFileSize, buf, lBuf, apParams);

	if (!result || (g_Plugin.FlagsWork & FW_TERMINATE))
	{
		return false;
	}

	// ��� ������ �������� ����� - �(�)������� (���� ���������) �����������
	// ����������� ���������� PgUp/PgDn (������� �������� ��� �����)
	if (apParams->Flags & eRenderFirstImage)
	{
		OnFirstImageOpened(apImage);
		if ((g_Plugin.FlagsWork & FW_TERMINATE))
			result = false;
	}
		

	if (result)
	{
		// � �� ���?
		TODO("������ �� ��������� ImageDecode � ������ ���� Display. ���������� ������������� ����� ������ ��� �� �������� DX � �������");

		//if (!ImageDecode())
		//if (!Decode(apImage, apImage->mp_File, &apImage->mp_Draw, false/*abResetTick*/))
		
		WARNING("����������, ��������� �������");
		//CDecodeItem img(apImage);

		TODO("abResetTick=true - ��� ������������� ��������� �������� �������� ��� (��� ������ ����� ��������)");
		if (!DecodePixels(apParams->pDecodeItem, false/*abResetTick*/))
		{
			WARNING("���� ImageDecode ������������ - �� �� �������������, �.�. ImageOpen �������� ��� �� �������!");
			WARNING("� ����� ������ ����� �������������� �������? �� ��������� pPVDDecoder � ��� ��������� ���������� bProcessed");
			uint nDecoders = (uint)CPVDManager::Decoders.size();
			if (nDecoders <= 1)
				return false;

			if (g_Plugin.FlagsWork & FW_TERMINATE)
				return false; // ������ �����������

			// ��������, ��� �� ������� ������������, ��� ��������� ������ � �������
			apImage->SetDecoderFailed(apImage->GetDecoder());
			//apImage->SetDecoderFailed(apImage->mi_DecoderIndex);

			//apImage->mi_DecoderIndex++;
			//if (apImage->mi_DecoderIndex >= nDecoders)
			//	apImage->mi_DecoderIndex = 0;

			goto lReOpen2;
		}
		//else
		//{
		//	// �������� �������������. ����� ������ "������� �������� ��� ����� �����������".
		// ����!
		// �������� �������, ����� ��������� ������������� �� ���� �������� �� ���� ������ �������,
		// � ���� ������ ���������� ���������� ������� ���������, ���� ��� �� ��������,
		// ��� ���� �� ���������� �����-�� ������ �������.
		//	g_Manager.ResetProcessed(this);
		//}
	}
	
	return result;
}

// 1) ��������� ������� ������ �� ��������� �������� � �������� �������
// 2) ���� ����� - ����������� ���� ���������
bool CPVDManager::OnItemReady(CDecodeItem* apItem)
{
	if (GetCurrentThreadId() != gnDisplayThreadId)
	{
		_ASSERTE(GetCurrentThreadId() == gnDisplayThreadId);
		return false;
	}

	if (!apItem || !apItem->pImage)
	{
		_ASSERTE(apItem!=NULL && apItem->pImage!=NULL);
		return false;
	}

	// ���������� ������� ������ �� �������� � �������
	if (PixelsToDisplay(apItem))
	{
		apItem->pImage->StoreDecodedFrame(apItem);
	}

	if (!apItem->pDraw || !apItem->pDraw->IsReady())
	{
		_ASSERTE(apItem->pDraw && apItem->pDraw->IsReady());
		return false;
	}

	_ASSERTE(IsImageReady(&apItem->Params));

	if ((apItem->Params.Flags & eRenderFirstImage))
	{
		if (!IsWindowVisible(g_Plugin.hWnd))
			ShowWindow(g_Plugin.hWnd, SW_SHOWNORMAL);
		g_Plugin.FlagsDisplay |= FD_TITLE_REPAINT;
		Invalidate(g_Plugin.hWnd);
	}

	return true;
}
