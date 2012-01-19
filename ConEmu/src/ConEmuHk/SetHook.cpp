
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

#define DROP_SETCP_ON_WIN2K3R2

// ����� �� ����������� GetConsoleAliases (���� �� ������ ���� �������� � Win2k)
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501

#define DEFINE_HOOK_MACROS

#ifdef _DEBUG
	//#define HOOK_ERROR_PROC
	#undef HOOK_ERROR_PROC
	#define HOOK_ERROR_NO ERROR_INVALID_DATA
#else
	#undef HOOK_ERROR_PROC
#endif

#include <windows.h>
#include <Tlhelp32.h>
#include "../common/common.hpp"
#include "../common/ConEmuCheck.h"
#include "../common/WinObjects.h"
//#include "../common/MArray.h"
#include "ShellProcessor.h"
#include "SetHook.h"

//#include <WinInet.h>
//#pragma comment(lib, "wininet.lib")

#define DebugString(x) // OutputDebugString(x)

#define LDR_IS_DATAFILE(hm)      ((((ULONG_PTR)(hm)) & (ULONG_PTR)1) == (ULONG_PTR)1)
#define LDR_IS_IMAGEMAPPING(hm)  ((((ULONG_PTR)(hm)) & (ULONG_PTR)2) == (ULONG_PTR)2)
#define LDR_IS_RESOURCE(hm)      (LDR_IS_IMAGEMAPPING(hm) || LDR_IS_DATAFILE(hm))

HMODULE ghHookOurModule = NULL; // ����� ����� dll'�� (����� ���� �� ��������)
DWORD   gnHookMainThreadId = 0;

extern BOOL gbDllStopCalled;

#ifdef _DEBUG
bool gbSuppressShowCall = false;
#endif

//extern CEFAR_INFO_MAPPING *gpFarInfo, *gpFarInfoMapping; //gpConsoleInfo;
//extern HANDLE ghFarAliveEvent;
//extern HWND ConEmuHwnd; // �������� ����� ���� ���������. ��� �������� ����.
//extern HWND FarHwnd;
//extern BOOL gbFARuseASCIIsort;
//extern DWORD gdwServerPID;
//extern BOOL gbShellNoZoneCheck;

//const wchar_t kernel32[] = L"kernel32.dll";
//const wchar_t user32[]   = L"user32.dll";
//const wchar_t shell32[]  = L"shell32.dll";
//HMODULE ghKernel32 = NULL, ghUser32 = NULL, ghShell32 = NULL;
//extern const wchar_t *kernel32;// = L"kernel32.dll";
//extern const wchar_t *user32  ;// = L"user32.dll";
//extern const wchar_t *shell32 ;// = L"shell32.dll";
//extern const wchar_t *advapi32;// = L"Advapi32.dll";
//extern HMODULE ghKernel32, ghUser32, ghShell32, ghAdvapi32;

const wchar_t *kernel32 = L"kernel32.dll",	*kernel32_noext = L"kernel32";
const wchar_t *user32   = L"user32.dll",	*user32_noext   = L"user32";
const wchar_t *gdi32    = L"gdi32.dll",		*gdi32_noext    = L"gdi32";
const wchar_t *shell32  = L"shell32.dll",	*shell32_noext  = L"shell32";
const wchar_t *advapi32 = L"advapi32.dll",	*advapi32_noext = L"advapi32";
const wchar_t *comdlg32 = L"comdlg32.dll",	*comdlg32_noext = L"comdlg32";
HMODULE ghKernel32 = NULL, ghUser32 = NULL, ghShell32 = NULL, ghAdvapi32 = NULL, ghComdlg32 = NULL;

//typedef LONG (WINAPI* RegCloseKey_t)(HKEY hKey);
RegCloseKey_t RegCloseKey_f = NULL;
//typedef LONG (WINAPI* RegOpenKeyEx_t)(HKEY hKey, LPCWSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult);
RegOpenKeyEx_t RegOpenKeyEx_f = NULL;
//typedef LONG (WINAPI* RegCreateKeyEx_t(HKEY hKey, LPCWSTR lpSubKey, DWORD Reserved, LPWSTR lpClass, DWORD dwOptions, REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition);
RegCreateKeyEx_t RegCreateKeyEx_f = NULL;

//typedef BOOL (WINAPI* OnChooseColorA_t)(LPCHOOSECOLORA lpcc);
OnChooseColorA_t ChooseColorA_f = NULL;
//typedef BOOL (WINAPI* OnChooseColorW_t)(LPCHOOSECOLORW lpcc);
OnChooseColorW_t ChooseColorW_f = NULL;

void CheckLoadedModule(LPCWSTR asModule)
{
	if (!asModule || !*asModule)
		return;

	struct PreloadFuncs {
		LPCSTR	sFuncName;
		void**	pFuncPtr;
	};
	struct {
		LPCWSTR      sModule, sModuleNoExt;
		HMODULE     *pModulePtr;
		PreloadFuncs Funcs[5];
	} Checks[] = {
		{user32,	user32_noext,	&ghUser32},
		{shell32,	shell32_noext,	&ghShell32},
		{advapi32,	advapi32_noext,	&ghAdvapi32,
			{{"RegOpenKeyExW", (void**)&RegOpenKeyEx_f},
			 {"RegCreateKeyExW", (void**)&RegCreateKeyEx_f},
			 {"RegCloseKey", (void**)&RegCloseKey_f}}
		},
		{comdlg32,	comdlg32_noext,	&ghComdlg32,
			{{"ChooseColorA", (void**)&ChooseColorA_f},
			 {"ChooseColorW", (void**)&ChooseColorW_f}}
		},
	};

	for (size_t m = 0; m < countof(Checks); m++)
	{
		if ((*Checks[m].pModulePtr) != NULL)
			continue;

		if (!lstrcmpiW(asModule, Checks[m].sModule) || !lstrcmpiW(asModule, Checks[m].sModuleNoExt))
		{
			*Checks[m].pModulePtr = LoadLibraryW(Checks[m].sModule); // LoadLibrary, �.�. � ��� �� ����� - ��������� �������
			if ((*Checks[m].pModulePtr) != NULL)
			{
				for (size_t f = 0; f < countof(Checks[m].Funcs); f++)
				{
					if (!Checks[m].Funcs[f].sFuncName)
						break;
					*Checks[m].Funcs[f].pFuncPtr = (void*)GetProcAddress(*Checks[m].pModulePtr, Checks[m].Funcs[f].sFuncName);
				}
			}
		}
	}
}

#define MAX_HOOKED_PROCS 255

// ������������ GetModuleFileName ��� CreateToolhelp32Snapshot �� ����� �������� ��������� ������
// ������, ������� ������ ������� �����
// 1. ��� ����, ����� �����, � ����� ������� ���� ��� ���������
// 2. ��� ����������, ����� �������� � ConEmu ���� ������������ ������� "Shell and processes log"
struct HkModuleInfo
{
	BOOL    bUsed;   // ������ ������
	int     Hooked;  // 1-������ ������������� (���� �����������), 2-���� �����
	HMODULE hModule; // �����
	wchar_t sModuleName[128]; // ������ �������������, � ��������� �� ���������
	HkModuleInfo* pNext;
	HkModuleInfo* pPrev;
	size_t nAdrUsed;
	struct StrAddresses
	{
		DWORD_PTR* ppAdr;
		DWORD_PTR  pOld;
		DWORD_PTR  pOur;
		union {
			BOOL bHooked;
			LPVOID Dummy;
		};
		#ifdef _DEBUG
		char sName[32];
		#endif
	} Addresses[MAX_HOOKED_PROCS];
};
WARNING("������ �� �������� ������ ��� gpHookedModules ����� VirtualProtect, ����� �������� �� �� ��������� ������� �����������");
HkModuleInfo *gpHookedModules = NULL, *gpHookedModulesLast = NULL;
size_t gnHookedModules = 0;
CRITICAL_SECTION *gpHookedModulesSection = NULL;
void InitializeHookedModules()
{
	_ASSERTE(gpHookedModules==NULL && gpHookedModulesSection==NULL);
	if (!gpHookedModulesSection)
	{
		//MessageBox(NULL, L"InitializeHookedModules", L"Hooks", MB_SYSTEMMODAL);

		gpHookedModulesSection = (CRITICAL_SECTION*)calloc(1,sizeof(*gpHookedModulesSection));
		InitializeCriticalSection(gpHookedModulesSection);

		//WARNING: "new" �������� �� DllStart ������! DllStart ���������� �� �� ������� ����,
		//WARNING: ������, ����� ������� ���� ��� �� ���� ��������. � �����, ���� ��� 
		//WARNING: ���������� ������� �� �������:
		//WARNING: runtime error R6030  - CRT not initialized
		// -- gpHookedModules = new MArray<HkModuleInfo>;
		// -- ������� ���� ����� ������
		//#ifdef _DEBUG
		//gnHookedModules = 16;
		//#else
		//gnHookedModules = 256;
		//#endif
		gpHookedModules = (HkModuleInfo*)calloc(sizeof(HkModuleInfo),1);
		if (!gpHookedModules)
		{
			_ASSERTE(gpHookedModules!=NULL);
		}
		gpHookedModulesLast = gpHookedModules;
	}
}
void FinalizeHookedModules()
{
	if (gpHookedModules)
	{
		if (gpHookedModulesSection)
			EnterCriticalSection(gpHookedModulesSection);

		HkModuleInfo *p = gpHookedModules;
		gpHookedModules = NULL;
		while (p)
		{
			HkModuleInfo *pNext = p->pNext;
			free(p);
			p = pNext;
		}

		if (gpHookedModulesSection)
			LeaveCriticalSection(gpHookedModulesSection);
	}
	if (gpHookedModulesSection)
	{
		DeleteCriticalSection(gpHookedModulesSection);
		free(gpHookedModulesSection);
		gpHookedModulesSection = NULL;
	}
}
HkModuleInfo* IsHookedModule(HMODULE hModule, LPWSTR pszName = NULL, size_t cchNameMax = 0)
{
	if (!gpHookedModulesSection)
		InitializeHookedModules();

	if (!gpHookedModules)
	{
		_ASSERTE(gpHookedModules!=NULL);
		return false;
	}

	//bool lbHooked = false;

	//_ASSERTE(gpHookedModules && gpHookedModulesSection);
	//if (bSection)
	//	EnterCriticalSection(gpHookedModulesSection);

	HkModuleInfo* p = gpHookedModules;
	while (p)
	{
		if (p->bUsed && (p->hModule == hModule))
		{
			_ASSERTE(p->Hooked == 1 || p->Hooked == 2);
			//lbHooked = true;
			
			// ���� ����� ������ ��� ������ (�� hModule)
			if (pszName && (cchNameMax > 0))
				lstrcpyn(pszName, p->sModuleName, (int)cchNameMax);
			break;
		}
		p = p->pNext;
	}

	//if (bSection)
	//	LeaveCriticalSection(gpHookedModulesSection);

	return p;
}
HkModuleInfo* AddHookedModule(HMODULE hModule, LPCWSTR sModuleName)
{
	if (!gpHookedModulesSection)
		InitializeHookedModules();

	_ASSERTE(gpHookedModules && gpHookedModulesSection);
	if (!gpHookedModules)
	{
		_ASSERTE(gpHookedModules!=NULL);
		return NULL;
	}

	HkModuleInfo* p = IsHookedModule(hModule);

	if (!p)
	{
		EnterCriticalSection(gpHookedModulesSection);
		
		p = gpHookedModules;
		while (p)
		{
			if (!p->bUsed)
			{
				gnHookedModules++;
				p->nAdrUsed = 0;
				p->Hooked = 1;
				p->hModule = hModule;
				lstrcpyn(p->sModuleName, sModuleName?sModuleName:L"", countof(p->sModuleName));
				// bUsed - ���������, ����� �� ���� ������� � ������� ��������
				p->bUsed = TRUE;
				goto done;
			}
			p = p->pNext;
		}
		
		p = (HkModuleInfo*)calloc(sizeof(HkModuleInfo),1);
		if (!p)
		{
			_ASSERTE(p!=NULL);
		}
		else
		{
			gnHookedModules++;
			p->bUsed = TRUE;   // ������ ������. ��� ����� ������, �.�. � ������� ��� �� ��������
			p->Hooked = 1; // ������ ������������� (���� �����������)
			p->hModule = hModule; // �����
			lstrcpyn(p->sModuleName, sModuleName?sModuleName:L"", countof(p->sModuleName));
			p->pNext = NULL;
			p->pPrev = gpHookedModulesLast;
			gpHookedModulesLast->pNext = p;
			gpHookedModulesLast = p;
		}
		
	done:
		LeaveCriticalSection(gpHookedModulesSection);
	}

	return p;
}
void RemoveHookedModule(HMODULE hModule)
{
	if (!gpHookedModulesSection)
		InitializeHookedModules();

	_ASSERTE(gpHookedModules && gpHookedModulesSection);
	if (!gpHookedModules)
	{
		_ASSERTE(gpHookedModules!=NULL);
		return;
	}

	HkModuleInfo* p = gpHookedModules;
	while (p)
	{
		if (p->bUsed && (p->hModule == hModule))
		{
			gnHookedModules--;
			// ������ � ����� ������������������, ����� � ������� �������� �� �������
			p->Hooked = 0;
			p->bUsed = FALSE;
			break;
		}
		p = p->pNext;
	}
}



BOOL gbHooksTemporaryDisabled = FALSE;
//BOOL gbInShellExecuteEx = FALSE;

//typedef VOID (WINAPI* OnLibraryLoaded_t)(HMODULE ahModule);
HMODULE ghOnLoadLibModule = NULL;
OnLibraryLoaded_t gfOnLibraryLoaded = NULL;
OnLibraryLoaded_t gfOnLibraryUnLoaded = NULL;

// Forward declarations of the hooks
HMODULE WINAPI OnLoadLibraryW(const WCHAR* lpFileName);
HMODULE WINAPI OnLoadLibraryA(const char* lpFileName);
HMODULE WINAPI OnLoadLibraryExW(const WCHAR* lpFileName, HANDLE hFile, DWORD dwFlags);
HMODULE WINAPI OnLoadLibraryExA(const char* lpFileName, HANDLE hFile, DWORD dwFlags);
BOOL WINAPI OnFreeLibrary(HMODULE hModule);
FARPROC WINAPI OnGetProcAddress(HMODULE hModule, LPCSTR lpProcName);
#ifdef HOOK_ERROR_PROC
DWORD WINAPI OnGetLastError();
VOID WINAPI OnSetLastError(DWORD dwErrCode);
#endif

HookItem *gpHooks = NULL;
size_t gnHookedFuncs = 0;

const char *szLoadLibraryA = "LoadLibraryA";
const char *szLoadLibraryW = "LoadLibraryW";
const char *szLoadLibraryExA = "LoadLibraryExA";
const char *szLoadLibraryExW = "LoadLibraryExW";
const char *szFreeLibrary = "FreeLibrary";
const char *szGetProcAddress = "GetProcAddress";
#ifdef HOOK_ERROR_PROC
const char *szGetLastError = "GetLastError";
const char *szSetLastError = "SetLastError";
#endif

bool InitHooksLibrary()
{
#ifndef HOOKS_SKIP_LIBRARY
	if (!gpHooks)
	{
		_ASSERTE(gpHooks!=NULL);
		return false;
	}
	if (gpHooks[0].NewAddress != NULL)
	{
		_ASSERTE(gpHooks[0].NewAddress==NULL);
		return false;
	}

	gnHookedFuncs = 0;
	#define ADDFUNC(pProc,szName,szDll) \
		gpHooks[gnHookedFuncs].NewAddress = pProc; \
		gpHooks[gnHookedFuncs].Name = szName; \
		gpHooks[gnHookedFuncs].DllName = szDll; \
		if (pProc) gnHookedFuncs++;
	/* ************************ */
	ADDFUNC((void*)OnLoadLibraryA,			szLoadLibraryA,			kernel32);
	ADDFUNC((void*)OnLoadLibraryW,			szLoadLibraryW,			kernel32);
	ADDFUNC((void*)OnLoadLibraryExA,		szLoadLibraryExA,		kernel32);
	ADDFUNC((void*)OnLoadLibraryExW,		szLoadLibraryExW,		kernel32);
	ADDFUNC((void*)OnFreeLibrary,			szFreeLibrary,			kernel32); // OnFreeLibrary ���� �����!
	ADDFUNC((void*)OnGetProcAddress,		szGetProcAddress,		kernel32);

	#ifdef HOOK_ERROR_PROC
	// ��� ������� ��������� ��������� ������
	ADDFUNC((void*)OnGetLastError,			szGetLastError,		kernel32);
	ADDFUNC((void*)OnSetLastError,			szSetLastError,		kernel32);
	#endif

	ADDFUNC(NULL,NULL,NULL);
	#undef ADDFUNC
	/* ************************ */

#endif
	return true;
}



#define MAX_EXCLUDED_MODULES 40
// Skip/ignore/don't set hooks in modules...
const wchar_t* ExcludedModules[MAX_EXCLUDED_MODULES] =
{
	L"ntdll.dll",
	L"kernelbase.dll",
	L"kernel32.dll",
	L"user32.dll",
	L"advapi32.dll",
//	L"shell32.dll", -- shell ����� ������������ �����������. �� ������� ���� � WinXP/Win2k3 (ShellExecute ������ ����� ��� CreateProcess)
	L"wininet.dll", // �����-�� �������� � ���� �����������?
//#ifndef _DEBUG
	L"mssign32.dll",
	L"crypt32.dll",
//#endif
	// � ����� ����������� ��� "API-MS-Win-..." � ������� IsModuleExcluded
	0
};

BOOL gbLogLibraries = FALSE;
DWORD gnLastLogSetChange = 0;


// ������������ � ��� ������, ���� ��������� ��������� ������������ �������, ��� ����� �������
// ������ � OnPeekConsoleInputW
void* __cdecl GetOriginalAddress(void* OurFunction, void* DefaultFunction, BOOL abAllowModified, HookItem** ph)
{
	if (gpHooks)
	{
		for (int i = 0; gpHooks[i].NewAddress; i++)
		{
			if (gpHooks[i].NewAddress == OurFunction)
			{
				*ph = &(gpHooks[i]);
				// �� ����, ExeOldAddress ������ ��������� � OldAddress, ���� ������� "Inject ConEmuHk"
				return (abAllowModified && gpHooks[i].ExeOldAddress) ? gpHooks[i].ExeOldAddress : gpHooks[i].OldAddress;
			}
		}
	}

	_ASSERT(FALSE); // ���� �� �������� �� ������
	return DefaultFunction;
}

CInFuncCall::CInFuncCall()
{
	mpn_Counter = NULL;
}
BOOL CInFuncCall::Inc(int* pnCounter)
{
	BOOL lbFirstCall = FALSE;
	mpn_Counter = pnCounter;

	if (mpn_Counter)
	{
		lbFirstCall = (*mpn_Counter) == 0;
		(*mpn_Counter)++;
	}

	return lbFirstCall;
}
CInFuncCall::~CInFuncCall()
{
	if (mpn_Counter && (*mpn_Counter)>0)(*mpn_Counter)--;
}


//extern HANDLE ghHookMutex;
MSection* gpHookCS = NULL;

// ��������� ���� HookItem.OldAddress (�������� ��������� �� ������� ���������)
// apHooks->Name && apHooks->DllName MUST be for a lifetime
bool __stdcall InitHooks(HookItem* apHooks)
{
	size_t i, j;
	bool skip;

	//if (!ghHookMutex)
	//{
	//	wchar_t* szMutexName = (wchar_t*)malloc(MAX_PATH*2);
	//	msprintf(szMutexName, MAX_PATH, CEHOOKLOCKMUTEX, GetCurrentProcessId());
	//	ghHookMutex = CreateMutexW(NULL, FALSE, szMutexName);
	//	_ASSERTE(ghHookMutex != NULL);
	//	free(szMutexName);
	//}
	if (!gpHookCS)
	{
		gpHookCS = new MSection;
	}

	//_ASSERTE(gpHookedModules!=NULL && gpHookedModulesSection!=NULL);
	//PRAGMA_ERROR("�������� gpHookedModules");
	//if (!gpHookedModules)
	//{
	//	gpHookedModulesSection = (CRITICAL_SECTION*)calloc(1,sizeof(*gpHookedModulesSection));
	//	gpHookedModules = new MArray<HkModuleInfo>;
	//}

	if (gpHooks == NULL)
	{
		gpHooks = (HookItem*)calloc(sizeof(HookItem),MAX_HOOKED_PROCS);
		if (!gpHooks)
			return false;
			
		if (!InitHooksLibrary())
			return false;
	}

	if (apHooks && gpHooks)
	{
		for (i = 0; apHooks[i].NewAddress; i++)
		{
			if (apHooks[i].Name==NULL || apHooks[i].DllName==NULL)
			{
				_ASSERTE(apHooks[i].Name!=NULL && apHooks[i].DllName!=NULL);
				break;
			}

			skip = false;

			for (j = 0; gpHooks[j].NewAddress; j++)
			{
				if (gpHooks[j].NewAddress == apHooks[i].NewAddress)
				{
					skip = true; break;
				}
			}

			if (skip) continue;

			j = 0; // using while, because of j

			while (gpHooks[j].NewAddress)
			{
				if (!lstrcmpiA(gpHooks[j].Name, apHooks[i].Name)
				        && !lstrcmpiW(gpHooks[j].DllName, apHooks[i].DllName))
				{
					// �� ������ ���� ������ - ������� ������ ������ �����������
					_ASSERTEX(lstrcmpiA(gpHooks[j].Name, apHooks[i].Name) && lstrcmpiW(gpHooks[j].DllName, apHooks[i].DllName));
					gpHooks[j].NewAddress = apHooks[i].NewAddress;
					if (j >= gnHookedFuncs)
						gnHookedFuncs = j+1;
					skip = true; break;
				}

				j++;
			}

			if (skip) continue;

			if (j >= 0)
			{
				if ((j+1) >= MAX_HOOKED_PROCS)
				{
					// ��������� ���������� ����������
					_ASSERTE((j+1) < MAX_HOOKED_PROCS);
					continue; // ����� ����� ������ ���� ������� ��������, � �� ��������
				}

				gpHooks[j].Name = apHooks[i].Name;
				gpHooks[j].DllName = apHooks[i].DllName;
				gpHooks[j].NewAddress = apHooks[i].NewAddress;
				_ASSERTEX(j >= gnHookedFuncs);
				gnHookedFuncs = j+1;
				gpHooks[j+1].Name = NULL; // �� ������
				gpHooks[j+1].NewAddress = NULL; // �� ������
			}
		}
	}

	// ��� ����������� � gpHooks ������� ���������� "������������" ����� ��������
	for (i = 0; gpHooks[i].NewAddress; i++)
	{
		if (!gpHooks[i].OldAddress)
		{
			// ������ - �� ���������
			HMODULE mod = GetModuleHandle(gpHooks[i].DllName);

			if (mod == NULL)
			{
				_ASSERTE(mod != NULL 
					// ����������, ������� ����� ���� �� ������������ �� ������
					|| (gpHooks[i].DllName == shell32 
						|| gpHooks[i].DllName == user32 
						|| gpHooks[i].DllName == gdi32 
						|| gpHooks[i].DllName == advapi32
						|| gpHooks[i].DllName == comdlg32 
						));
			}
			else
			{
				gpHooks[i].OldAddress = (void*)GetProcAddress(mod, gpHooks[i].Name);
				if (gpHooks[i].OldAddress == NULL)
				{
					_ASSERTE(gpHooks[i].OldAddress != NULL);
				}
				gpHooks[i].hDll = mod;
			}
		}
	}

	return true;
}

void ShutdownHooks()
{
	UnsetAllHooks();
	
	//// ��������� ������ � ��������
	//DoneHooksReg();

	// ���������� ��������� ��������
	if (ghKernel32)
	{
		FreeLibrary(ghKernel32);
		ghKernel32 = NULL;
	}
	if (ghUser32)
	{
		FreeLibrary(ghUser32);
		ghUser32 = NULL;
	}
	if (ghShell32)
	{
		FreeLibrary(ghShell32);
		ghShell32 = NULL;
	}
	if (ghAdvapi32)
	{
		FreeLibrary(ghAdvapi32);
		ghAdvapi32 = NULL;
	}
	if (ghComdlg32)
	{
		FreeLibrary(ghComdlg32);
		ghComdlg32 = NULL;
	}
	
	if (gpHookCS)
	{
		MSection *p = gpHookCS;
		gpHookCS = NULL;
		delete p;
	}

	FinalizeHookedModules();
}

void __stdcall SetLoadLibraryCallback(HMODULE ahCallbackModule, OnLibraryLoaded_t afOnLibraryLoaded, OnLibraryLoaded_t afOnLibraryUnLoaded)
{
	ghOnLoadLibModule = ahCallbackModule;
	gfOnLibraryLoaded = afOnLibraryLoaded;
	gfOnLibraryUnLoaded = afOnLibraryUnLoaded;
}

bool __stdcall SetHookCallbacks(const char* ProcName, const wchar_t* DllName, HMODULE hCallbackModule,
                                HookItemPreCallback_t PreCallBack, HookItemPostCallback_t PostCallBack,
                                HookItemExceptCallback_t ExceptCallBack)
{
	if (!ProcName|| !DllName)
	{
		_ASSERTE(ProcName!=NULL && DllName!=NULL);
		return false;
	}
	_ASSERTE(ProcName[0]!=0 && DllName[0]!=0);

	bool bFound = false;

	if (gpHooks)
	{
		for (int i = 0; i<MAX_HOOKED_PROCS && gpHooks[i].NewAddress; i++)
		{
			if (!lstrcmpA(gpHooks[i].Name, ProcName) && !lstrcmpW(gpHooks[i].DllName,DllName))
			{
				gpHooks[i].hCallbackModule = hCallbackModule;
				gpHooks[i].PreCallBack = PreCallBack;
				gpHooks[i].PostCallBack = PostCallBack;
				gpHooks[i].ExceptCallBack = ExceptCallBack;
				bFound = true;
				//break; // ���������� ����� ���� ����� ������ (������� ����� �� exe/dll)
			}
		}
	}

	return bFound;
}

// ���������, ������� �� ������?
bool IsModuleValid(HMODULE module)
{
	if ((module == NULL) || (module == INVALID_HANDLE_VALUE))
		return false;
	if (LDR_IS_RESOURCE(module))
		return false;

#ifdef USE_SEH
	bool lbValid = true;
	IMAGE_DOS_HEADER dos;
	IMAGE_NT_HEADERS nt;

	SAFETRY
	{
		memmove(&dos, (void*)module, sizeof(dos));
		if (dos.e_magic != IMAGE_DOS_SIGNATURE /*'ZM'*/)
		{
			lbValid = false;
		}
		else
		{
			memmove(&nt, (IMAGE_NT_HEADERS*)((char*)module + ((IMAGE_DOS_HEADER*)module)->e_lfanew), sizeof(nt));
			if (nt.Signature != 0x004550)
				lbValid = false;
		}
	}
	SAFECATCH
	{
		lbValid = false;
	}

	return lbValid;
#else
	if (IsBadReadPtr((void*)module, sizeof(IMAGE_DOS_HEADER)))
		return false;

	if (((IMAGE_DOS_HEADER*)module)->e_magic != IMAGE_DOS_SIGNATURE /*'ZM'*/)
		return false;

	IMAGE_NT_HEADERS* nt_header = (IMAGE_NT_HEADERS*)((char*)module + ((IMAGE_DOS_HEADER*)module)->e_lfanew);
	if (IsBadReadPtr(nt_header, sizeof(IMAGE_NT_HEADERS)))
		return false;

	if (nt_header->Signature != 0x004550)
		return false;

	return true;
#endif
}

bool FindModuleFileName(HMODULE ahModule, LPWSTR pszName, size_t cchNameMax)
{
	bool lbFound = false;
	if (pszName && cchNameMax)
	{
		//*pszName = 0;
#ifdef _WIN64
		msprintf(pszName, cchNameMax, L"<HMODULE=0x%08X%08X> ",
			(DWORD)((((u64)ahModule) & 0xFFFFFFFF00000000) >> 32), //-V112
			(DWORD)(((u64)ahModule) & 0xFFFFFFFF)); //-V112
#else
		msprintf(pszName, cchNameMax, L"<HMODULE=0x%08X> ", (DWORD)ahModule);
#endif

		INT_PTR nLen = lstrlen(pszName);
		pszName += nLen;
		cchNameMax -= nLen;
		_ASSERTE(cchNameMax>0);
	}

	//TH32CS_SNAPMODULE - ����� �������� ��� ������� �� LoadLibrary/FreeLibrary.
	lbFound = (IsHookedModule(ahModule, pszName, cchNameMax) != NULL);

/*
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, 0);

	if (snapshot != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 module = {sizeof(MODULEENTRY32)};

		for (BOOL res = Module32First(snapshot, &module); res; res = Module32Next(snapshot, &module))
		{
			if (module.hModule == ahModule)
			{
				if (pszName)
					_wcscpyn_c(pszName, cchNameMax, module.szModule, cchNameMax);
				lbFound = true;
				break;
			}
		}

		CloseHandle(snapshot);
	}
*/

	return lbFound;
}

bool IsModuleExcluded(HMODULE module, LPCSTR asModuleA, LPCWSTR asModuleW)
{
	if (module == ghHookOurModule)
		return true;

	BOOL lbResource = LDR_IS_RESOURCE(module);
	if (lbResource)
		return true;

	// ��������, ����� ����� ������������ ��������� ���������� ����
    // API-MS-Win-Core-Util-L1-1-0.dll
	if (asModuleA)
	{
		char szTest[12]; lstrcpynA(szTest, asModuleA, 12);
		if (lstrcmpiA(szTest, "API-MS-Win-") == 0)
			return true;
	}
	else if (asModuleW)
	{
		wchar_t szTest[12]; lstrcpynW(szTest, asModuleW, 12);
		if (lstrcmpiW(szTest, L"API-MS-Win-") == 0)
			return true;
	}

#if 1
	for (int i = 0; ExcludedModules[i]; i++)
		if (module == GetModuleHandle(ExcludedModules[i]))
			return true;
#else
	wchar_t szModule[MAX_PATH*2]; szModule[0] = 0;
	DWORD nLen = GetModuleFileNameW(module, szModule, countof(szModule));
	if ((nLen == 0) || (nLen >= countof(szModule)))
	{
		//_ASSERTE(nLen>0 && nLen<countof(szModule));
		return true; // ���-�� � ������� �� ��...
	}
	LPCWSTR pszName = wcsrchr(szModule, L'\\');
	if (pszName) pszName++; else pszName = szModule;
	for (int i = 0; ExcludedModules[i]; i++)
	{
		if (lstrcmpi(ExcludedModules[i], pszName) == 0)
			return true; // ������ � �����������
	}
#endif

	return false;
}


#define GetPtrFromRVA(rva,pNTHeader,imageBase) (PVOID)((imageBase)+(rva))

extern BOOL gbInCommonShutdown;

bool LockHooks(HMODULE Module, LPCWSTR asAction, MSectionLock* apCS)
{
	//while (nHookMutexWait != WAIT_OBJECT_0)
	BOOL lbLockHooksSection = FALSE;
	while (!(lbLockHooksSection = apCS->Lock(gpHookCS, TRUE, 10000)))
	{
#ifdef _DEBUG

		if (!IsDebuggerPresent())
		{
			_ASSERTE(lbLockHooksSection);
		}

#endif

		if (gbInCommonShutdown)
			return false;

		wchar_t* szTrapMsg = (wchar_t*)calloc(1024,2);
		wchar_t* szName = (wchar_t*)calloc((MAX_PATH+1),2);

		if (!FindModuleFileName(Module, szName, MAX_PATH+1)) szName[0] = 0;

		DWORD nTID = GetCurrentThreadId(); DWORD nPID = GetCurrentProcessId();
		msprintf(szTrapMsg, 1024, 
			L"Can't %s hooks in module '%s'\nCurrent PID=%u, TID=%i\nCan't lock hook section\nPress 'Retry' to repeat locking",
			asAction, szName, nPID, nTID);

		int nBtn = 
#ifdef CONEMU_MINIMAL
			GuiMessageBox
#else
			MessageBoxW
#endif
			(GetConEmuHWND(TRUE), szTrapMsg, L"ConEmu", MB_RETRYCANCEL|MB_ICONSTOP|MB_SYSTEMMODAL);

		free(szTrapMsg);
		free(szName);

		if (nBtn != IDRETRY)
			return false;

		//nHookMutexWait = WaitForSingleObject(ghHookMutex, 10000);
		//continue;
	}

	return true;
}

bool SetHookPrep(LPCWSTR asModule, HMODULE Module, BOOL abForceHooks, bool bExecutable, IMAGE_IMPORT_DESCRIPTOR* Import, size_t ImportCount, bool (&bFnNeedHook)[MAX_HOOKED_PROCS], HkModuleInfo* p);
bool SetHookChange(LPCWSTR asModule, HMODULE Module, BOOL abForceHooks, bool (&bFnNeedHook)[MAX_HOOKED_PROCS], HkModuleInfo* p);
// ��������� ������������� ������� � ������ (Module)
//	���� (abForceHooks == FALSE) �� ���� �� ��������, ����
//  ����� ��������� ������, �� ����������� � ����������
//  ��� ��� ����, ����� �� ��������� ������������� ���� ��� ������������� LoadLibrary
bool SetHook(LPCWSTR asModule, HMODULE Module, BOOL abForceHooks)
{
	IMAGE_IMPORT_DESCRIPTOR* Import = NULL;
	DWORD Size = 0;
	HMODULE hExecutable = GetModuleHandle(0);

	if (!gpHooks)
		return false;

	if (!Module)
		Module = hExecutable;
		
	//#ifdef NDEBUG
	//	PRAGMA_ERROR("���� ������ ������������ ���� ���");
	//#endif

	// ���� �� ��� ������ - �� ��������� ������ ������
	HkModuleInfo* p = IsHookedModule(Module);
	if (p)
		return true;
	
	if (!IsModuleValid(Module))
		return false;
		
	bool bExecutable = (Module == hExecutable);
	IMAGE_DOS_HEADER* dos_header = (IMAGE_DOS_HEADER*)Module;
	IMAGE_NT_HEADERS* nt_header = NULL;

	// ���������� ������ �������� sizeof(IMAGE_DOS_HEADER) ����������� � IsModuleValid.
	if (dos_header->e_magic == IMAGE_DOS_SIGNATURE /*'ZM'*/)
	{
		nt_header = (IMAGE_NT_HEADERS*)((char*)Module + dos_header->e_lfanew);
		if (IsBadReadPtr(nt_header, sizeof(IMAGE_NT_HEADERS)))
			return false;

		if (nt_header->Signature != 0x004550)
			return false;
		else
		{
			Import = (IMAGE_IMPORT_DESCRIPTOR*)((char*)Module +
			                                    (DWORD)(nt_header->OptionalHeader.
			                                            DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].
			                                            VirtualAddress));
			Size = nt_header->OptionalHeader.
			       DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
		}
	}
	else
		return false;

	// if wrong module or no import table
	if (!Import)
		return false;

#ifdef _DEBUG
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(nt_header); //-V220
#endif
#ifdef _WIN64
	_ASSERTE(sizeof(DWORD_PTR)==8);
#else
	_ASSERTE(sizeof(DWORD_PTR)==4);
#endif
#ifdef _WIN64
#define TOP_SHIFT 60
#else
#define TOP_SHIFT 28
#endif
	//DWORD nHookMutexWait = WaitForSingleObject(ghHookMutex, 10000);

	MSectionLock CS;
	if (!LockHooks(Module, L"install", &CS))
		return false;

	if (!p)
	{
		p = AddHookedModule(Module, asModule);
		if (!p)
			return false;
	}
		
//	//while (nHookMutexWait != WAIT_OBJECT_0)
//	while (!CS.Lock(gpHookCS, TRUE, 10000))
//	{
//#ifdef _DEBUG
//
//		if (!IsDebuggerPresent())
//		{
//			_ASSERTE(nHookMutexWait == WAIT_OBJECT_0);
//		}
//
//#endif
//
//		if (gbInCommonShutdown)
//			return false;
//
//		wchar_t* szTrapMsg = (wchar_t*)calloc(1024,2);
//		wchar_t* szName = (wchar_t*)calloc((MAX_PATH+1),2);
//
//		if (!GetModuleFileNameW(Module, szName, MAX_PATH+1)) szName[0] = 0;
//
//		DWORD nTID = GetCurrentThreadId(); DWORD nPID = GetCurrentProcessId();
//		msprintf(szTrapMsg, 1024, L"Can't install hooks in module '%s'\nCurrent PID=%u, TID=%i\nCan't lock hook mutex\nPress 'Retry' to repeat locking",
//		          szName, nPID, nTID);
//
//		int nBtn = 
//			#ifdef CONEMU_MINIMAL
//				GuiMessageBox
//			#else
//				MessageBoxW
//			#endif
//			(GetConEmuHWND(TRUE), szTrapMsg, L"ConEmu", MB_RETRYCANCEL|MB_ICONSTOP|MB_SYSTEMMODAL);
//		
//		free(szTrapMsg);
//		free(szName);
//		
//		if (nBtn != IDRETRY)
//			return false;
//
//		//nHookMutexWait = WaitForSingleObject(ghHookMutex, 10000);
//		//continue;
//	}

	TODO("!!! ��������� ORDINAL �������� !!!");
	bool res = false, bHooked = false;
	//INT_PTR i;
	INT_PTR nCount = Size / sizeof(IMAGE_IMPORT_DESCRIPTOR);
	bool bFnNeedHook[MAX_HOOKED_PROCS] = {};

	// � ��������� �������, �.�. __try
	res = SetHookPrep(asModule, Module, abForceHooks, bExecutable, Import, nCount, bFnNeedHook, p);

	// � ��������� �������, �.�. __try
	bHooked = SetHookChange(asModule, Module, abForceHooks, bFnNeedHook, p);
	
#ifdef _DEBUG

	if (bHooked)
	{
		wchar_t* szDbg = (wchar_t*)calloc(MAX_PATH*3, 2);
		wchar_t* szModPath = (wchar_t*)calloc(MAX_PATH*2, 2);
		FindModuleFileName(Module, szModPath, MAX_PATH*2);
		_wcscpy_c(szDbg, MAX_PATH*3, L"  ## Hooks was set by conemu: ");
		_wcscat_c(szDbg, MAX_PATH*3, szModPath);
		_wcscat_c(szDbg, MAX_PATH*3, L"\n");
		DebugString(szDbg);
		free(szDbg);
		free(szModPath);
	}

#endif
	//ReleaseMutex(ghHookMutex);
	CS.Unlock();

	// ������ ConEmu ����� ��������� �������������� ��������
	if (gfOnLibraryLoaded)
	{
		gfOnLibraryLoaded(Module);
	}

	return res;
}

bool SetHookPrep(LPCWSTR asModule, HMODULE Module, BOOL abForceHooks, bool bExecutable, IMAGE_IMPORT_DESCRIPTOR* Import, size_t ImportCount, bool (&bFnNeedHook)[MAX_HOOKED_PROCS], HkModuleInfo* p)
{
	bool res = false;
	size_t i;
	
	SAFETRY
	{
		//_ASSERTE(Size == (nCount * sizeof(IMAGE_IMPORT_DESCRIPTOR))); -- ����� ���� �� �������
		for (i = 0; i < ImportCount; i++)
		{
			if (Import[i].Name == 0)
				break;

			//DebugString( ToTchar( (char*)Module + Import[i].Name ) );
			#ifdef _DEBUG
			char* mod_name = (char*)Module + Import[i].Name;
			#endif
			DWORD_PTR rvaINT = Import[i].OriginalFirstThunk;
			DWORD_PTR rvaIAT = Import[i].FirstThunk; //-V101

			if (rvaINT == 0)      // No Characteristics field?
			{
				// Yes! Gotta have a non-zero FirstThunk field then.
				rvaINT = rvaIAT;

				if (rvaINT == 0)       // No FirstThunk field?  Ooops!!!
				{
					_ASSERTE(rvaINT!=0);
					break;
				}
			}

			//PIMAGE_IMPORT_BY_NAME pOrdinalName = NULL, pOrdinalNameO = NULL;
			PIMAGE_IMPORT_BY_NAME pOrdinalNameO = NULL;
			//IMAGE_IMPORT_BY_NAME** byname = (IMAGE_IMPORT_BY_NAME**)((char*)Module + rvaINT);
			//IMAGE_THUNK_DATA* thunk = (IMAGE_THUNK_DATA*)((char*)Module + rvaIAT);
			IMAGE_THUNK_DATA* thunk = (IMAGE_THUNK_DATA*)GetPtrFromRVA(rvaIAT, nt_header, (PBYTE)Module);
			IMAGE_THUNK_DATA* thunkO = (IMAGE_THUNK_DATA*)GetPtrFromRVA(rvaINT, nt_header, (PBYTE)Module);

			if (!thunk ||  !thunkO)
			{
				_ASSERTE(thunk && thunkO);
				continue;
			}

			size_t f, s, j;
			for (s = 0; s <= 1; s++)
			{
				for (f = 0;; thunk++, thunkO++, f++)
				{
					//111127 - ..\GIT\lib\perl5\site_perl\5.8.8\msys\auto\SVN\_Core\_Core.dll
					//         ������, � ���� ��� ������ ������� ��������
					#ifndef USE_SEH
					BOOL lbBadThunk = IsBadReadPtr(thunk, sizeof(*thunk));
					if (lbBadThunk)
					{
						_ASSERTEX(!lbBadThunk);
						break;
					}
					#endif
					
					if (!thunk->u1.Function)
						break;
					
					#ifndef USE_SEH
					BOOL lbBadThunkO = IsBadReadPtr(thunkO, sizeof(*thunkO));
					if (lbBadThunkO)
					{
						_ASSERTEX(!lbBadThunkO);
						break;
					}
					#endif

					const char* pszFuncName = NULL;
					//ULONGLONG ordinalO = -1;

					if (thunk->u1.Function != thunkO->u1.Function)
					{
						//// Ordinal � ��� ���� �� ������������
						////if (IMAGE_SNAP_BY_ORDINAL(thunkO->u1.Ordinal))
						////{
						////	WARNING("��� �� ORDINAL, ��� Hint!!!");
						////	ordinalO = IMAGE_ORDINAL(thunkO->u1.Ordinal);
						////	pOrdinalNameO = NULL;
						////}

						
						// ����� ������ ��� ������� �� ������ ��� EXE, �� � ��� ���� dll?
						if (s /*bExecutable*/)
						{
							if (!IMAGE_SNAP_BY_ORDINAL(thunkO->u1.Ordinal))
							{
								pOrdinalNameO = (PIMAGE_IMPORT_BY_NAME)GetPtrFromRVA(thunkO->u1.AddressOfData, nt_header, (PBYTE)Module);
								
								#ifdef USE_SEH
									pszFuncName = (LPCSTR)pOrdinalNameO->Name;
								#else
									//WARNING!!! ������������� ������ IsBad???Ptr ����� ������� � ���������
									BOOL lbValidPtr = !IsBadReadPtr(pOrdinalNameO, sizeof(IMAGE_IMPORT_BY_NAME));
									_ASSERTE(lbValidPtr);

									if (lbValidPtr)
									{
										lbValidPtr = !IsBadStringPtrA((LPCSTR)pOrdinalNameO->Name, 10);
										_ASSERTE(lbValidPtr);

										if (lbValidPtr)
											pszFuncName = (LPCSTR)pOrdinalNameO->Name;
									}
								#endif
							}
						}
					}

					// �������� ����� �������, � (�� ������ ����) ��� �������
					// ������ ����� ��������� (���� ����) ����� ���������
					for (j = 0; gpHooks[j].Name; j++)
					{
						_ASSERTEX(j<gnHookedFuncs && gnHookedFuncs<=MAX_HOOKED_PROCS);
					
						// ���� �� ������� ���������� ������������ ����� ��������� (kernel32/WriteConsoleOutputW, � �.�.)
						if (gpHooks[j].OldAddress == NULL)
						{
							continue;
						}
						
						// ���� ����� ������� � ������ ��� ��������� � ������� ����� �� ����� �������
						if (gpHooks[j].NewAddress == (void*)thunk->u1.Function)
						{
							res = true; // ��� ��� ��������
							break;
						}
						
						#ifdef _DEBUG
						const void* ptrNewAddress = gpHooks[j].NewAddress;
						const void* ptrOldAddress = (void*)thunk->u1.Function;
						#endif

						// ��������� ����� ��������������� �������
						if ((void*)thunk->u1.Function != gpHooks[j].OldAddress)
						{
							// pszFuncName ����������� �� ������ ����
							if (!pszFuncName /*|| !bExecutable*/)
							{
								continue; // ���� ��� ������� ���������� �� ������� - ����������
							}
							else
							{
								// ���������, ��� ������� ��������� � ����������������?
								if (lstrcmpA(pszFuncName, gpHooks[j].Name))
									continue;
							}

							if (!abForceHooks)
							{
								continue; // �������� ��������, ���� ������� ����� � ������ �� ��������� � ������������ ���������!
							}
							else if (bExecutable && !gpHooks[j].ExeOldAddress)
							{
								// OldAddress ��� ����� ���������� �� ������������� �������� ����������
								//// ��� ���������� �������� � PeekConsoleIntputW ��� ������� ������� Anamorphosis
								// ��� Anamorphosis ��������� ��������. ��� ���������� "Inject ConEmuHk"
								// ���� �������� ����� ��� ������� ��������.
								// ��, ������������, ���-�� ����� ������ ������, ��� ������ "Inject" ��������.
								gpHooks[j].ExeOldAddress = (void*)thunk->u1.Function;
							}
						}

						if (p->Addresses[j].ppAdr != NULL)
							break; // ��� ����������, ��������� ������
						
						//#ifdef _DEBUG
						//// ��� �� ORDINAL, ��� Hint!!!
						//if (gpHooks[j].nOrdinal == 0 && ordinalO != (ULONGLONG)-1)
						//	gpHooks[j].nOrdinal = (DWORD)ordinalO;
						//#endif


						_ASSERTE(sizeof(thunk->u1.Function)==sizeof(DWORD_PTR));
						
						if (thunk->u1.Function == (DWORD_PTR)gpHooks[j].NewAddress)
						{
							// ��������� �������� � ������ ����? ������ ���� �� ������, ����������� �������
							// �� ����� ���� �������� � ������� ���, ���� �� ��� ������ ���� ��������� ��� ������
							_ASSERTE(thunk->u1.Function != (DWORD_PTR)gpHooks[j].NewAddress);
						}
						else
						{
							bFnNeedHook[j] = true;
							p->Addresses[j].ppAdr = &thunk->u1.Function;
							p->Addresses[j].pOld = thunk->u1.Function;
							p->Addresses[j].pOur = (DWORD_PTR)gpHooks[j].NewAddress;
							#ifdef _DEBUG
							lstrcpynA(p->Addresses[j].sName, gpHooks[j].Name, countof(p->Addresses[j].sName));
							#endif
							
							_ASSERTEX(p->nAdrUsed < countof(p->Addresses));
							p->nAdrUsed++; //�������������
						}

						#ifdef _DEBUG
						if (bExecutable)
							gpHooks[j].ReplacedInExe = TRUE;
						#endif
							
						//DebugString( ToTchar( gpHooks[j].Name ) );
						res = true;

						break;
					} // for (j = 0; gpHooks[j].Name; j++)
				} // for (f = 0;; thunk++, thunkO++, f++)
			} // for (s = 0; s <= 1; s++)
		} // for (i = 0; i < nCount; i++)
	} SAFECATCH {
	}
	
	return res;
}

bool SetHookChange(LPCWSTR asModule, HMODULE Module, BOOL abForceHooks, bool (&bFnNeedHook)[MAX_HOOKED_PROCS], HkModuleInfo* p)
{
	bool bHooked = false;
	size_t j = 0;
	DWORD dwErr = (DWORD)-1;
	_ASSERTEX(j<gnHookedFuncs && gnHookedFuncs<=MAX_HOOKED_PROCS);
	
	SAFETRY
	{
		while (j < gnHookedFuncs)
		{
			// ����� ���� NULL, ���� ������������� �� ��� �������
			if (p->Addresses[j].ppAdr && bFnNeedHook[j])
			{
				if (*p->Addresses[j].ppAdr == p->Addresses[j].pOur)
				{
					// ��������� �������� � ������ ���� ��� ������
					_ASSERTEX(*p->Addresses[j].ppAdr != p->Addresses[j].pOur);
				}
				else
				{
					DWORD old_protect;
					if (!VirtualProtect(p->Addresses[j].ppAdr, sizeof(*p->Addresses[j].ppAdr),
					                   PAGE_READWRITE, &old_protect))
					{
						dwErr = GetLastError();
						_ASSERTEX(FALSE);
					}
					else
					{
						bHooked = true;
						
						*p->Addresses[j].ppAdr = p->Addresses[j].pOur;
						p->Addresses[j].bHooked = TRUE;
						
						VirtualProtect(p->Addresses[j].ppAdr, sizeof(*p->Addresses[j].ppAdr), old_protect, &old_protect);
					}

				}
			}
			
			j++;
		}
	} SAFECATCH {
		// ������ ����������
		p->Addresses[j].pOur = 0;
	}
	
	return bHooked;
}

DWORD GetMainThreadId()
{
	// ����� ID �������� ����
	if (!gnHookMainThreadId)
	{
		DWORD dwPID = GetCurrentProcessId();
		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, dwPID);

		if (snapshot != INVALID_HANDLE_VALUE)
		{
			THREADENTRY32 module = {sizeof(THREADENTRY32)};

			if (Thread32First(snapshot, &module))
			{
				while (!gnHookMainThreadId)
				{
					if (module.th32OwnerProcessID == dwPID)
					{
						gnHookMainThreadId = module.th32ThreadID;
						break;
					}

					if (!Thread32Next(snapshot, &module))
						break;
				}
			}

			CloseHandle(snapshot);
		}
	}

	_ASSERTE(gnHookMainThreadId!=0);
	return gnHookMainThreadId;
}

// ��������� ������������� ������� �� ���� ������� ��������, ����������� �� conemu.dll
// *aszExcludedModules - ������ ��������� �� ����������� �������� (program lifetime)
bool __stdcall SetAllHooks(HMODULE ahOurDll, const wchar_t** aszExcludedModules /*= NULL*/, BOOL abForceHooks)
{
	// �.�. SetAllHooks ����� ���� ������ �� ������ dll - ���������� ����������
	if (!ghHookOurModule) ghHookOurModule = ahOurDll;

	InitHooks(NULL);
	if (!gpHooks)
		return false;

#ifdef _DEBUG
	wchar_t szHookProc[128];
	for (int i = 0; gpHooks[i].NewAddress; i++)
	{
		msprintf(szHookProc, countof(szHookProc), L"## %S -> 0x%08X (exe: 0x%X)\n", gpHooks[i].Name, (DWORD)gpHooks[i].NewAddress, (DWORD)gpHooks[i].ExeOldAddress);
		DebugString(szHookProc);
	}

#endif

	// ��������� aszExcludedModules
	if (aszExcludedModules)
	{
		INT_PTR j;
		bool skip;

		for (INT_PTR i = 0; aszExcludedModules[i]; i++)
		{
			j = 0; skip = false;

			while (ExcludedModules[j])
			{
				if (lstrcmpi(ExcludedModules[j], aszExcludedModules[i]) == 0)
				{
					skip = true; break;
				}

				j++;
			}

			if (skip) continue;

			if (j > 0)
			{
				if ((j+1) >= MAX_EXCLUDED_MODULES)
				{
					// ��������� ���������� ����������
					_ASSERTE((j+1) < MAX_EXCLUDED_MODULES);
					continue;
				}

				ExcludedModules[j] = aszExcludedModules[i];
				ExcludedModules[j+1] = NULL; // �� ������
			}
		}
	}

	// ��� ������������ ����� ����� ���� ������ �������������� inject-� (��������� � FAR)
	#ifdef _DEBUG
	HMODULE hExecutable = GetModuleHandle(0);
	#endif
	HANDLE snapshot;

	// ����� ID �������� ����
	GetMainThreadId();
	_ASSERTE(gnHookMainThreadId!=0);
	//if (!gnHookMainThreadId)
	//{
	//	DWORD dwPID = GetCurrentProcessId();
	//	snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, dwPID);
	//	if (snapshot != INVALID_HANDLE_VALUE)
	//	{
	//		THREADENTRY32 module = {sizeof(THREADENTRY32)};
	//		if (Thread32First(snapshot, &module))
	//		{
	//			while (!gnHookMainThreadId)
	//			{
	//				if (module.th32OwnerProcessID == dwPID)
	//				{
	//					gnHookMainThreadId = module.th32ThreadID;
	//					break;
	//				}
	//				if (!Thread32Next(snapshot, &module))
	//					break;
	//			}
	//		}
	//		CloseHandle(snapshot);
	//	}
	//}

	// �������� ������ �� ���� ����������� (linked) �������
	snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, 0);

	if (snapshot != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 module = {sizeof(MODULEENTRY32)};

		for (BOOL res = Module32First(snapshot, &module); res; res = Module32Next(snapshot, &module))
		{
			if (module.hModule && !IsModuleExcluded(module.hModule, NULL, module.szModule))
			{
				DebugString(module.szModule);
				SetHook(module.szModule, module.hModule/*, (module.hModule == hExecutable)*/, abForceHooks);
			}
		}

		CloseHandle(snapshot);
	}

	return true;
}


bool UnsetHookInt(HMODULE Module)
{
	bool bUnhooked = false, res = false;
	IMAGE_IMPORT_DESCRIPTOR* Import = 0;
	size_t Size = 0;
	_ASSERTE(Module!=NULL);
	IMAGE_DOS_HEADER* dos_header = (IMAGE_DOS_HEADER*)Module;

	SAFETRY
	{
		if (dos_header->e_magic == IMAGE_DOS_SIGNATURE /*'ZM'*/)
		{
			IMAGE_NT_HEADERS* nt_header = (IMAGE_NT_HEADERS*)((char*)Module + dos_header->e_lfanew);
		
			if (nt_header->Signature != 0x004550)
				goto wrap;
			else
			{
				Import = (IMAGE_IMPORT_DESCRIPTOR*)((char*)Module +
													(DWORD)(nt_header->OptionalHeader.
															DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].
															VirtualAddress));
				Size = nt_header->OptionalHeader.
					   DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
			}
		}
		else
			goto wrap;
		
		// if wrong module or no import table
		if (Module == INVALID_HANDLE_VALUE || !Import)
			goto wrap;
		
		size_t i, s;
		size_t nCount = Size / sizeof(IMAGE_IMPORT_DESCRIPTOR);
		
		//_ASSERTE(Size == (nCount * sizeof(IMAGE_IMPORT_DESCRIPTOR))); -- ����� ���� �� �������
		for (s = 0; s <= 1; s++)
		{
			for (i = 0; i < nCount; i++)
			{
				if (Import[i].Name == 0)
					break;
			
				#ifdef _DEBUG
				char* mod_name = (char*)Module + Import[i].Name;
				#endif
				DWORD_PTR rvaINT = Import[i].OriginalFirstThunk;
				DWORD_PTR rvaIAT = Import[i].FirstThunk; //-V101
			
				if (rvaINT == 0)      // No Characteristics field?
				{
					// Yes! Gotta have a non-zero FirstThunk field then.
					rvaINT = rvaIAT;
			
					if (rvaINT == 0)       // No FirstThunk field?  Ooops!!!
					{
						_ASSERTE(rvaINT!=0);
						break;
					}
				}
			
				//PIMAGE_IMPORT_BY_NAME pOrdinalName = NULL, pOrdinalNameO = NULL;
				PIMAGE_IMPORT_BY_NAME pOrdinalNameO = NULL;
				//IMAGE_IMPORT_BY_NAME** byname = (IMAGE_IMPORT_BY_NAME**)((char*)Module + rvaINT);
				//IMAGE_THUNK_DATA* thunk = (IMAGE_THUNK_DATA*)((char*)Module + rvaIAT);
				IMAGE_THUNK_DATA* thunk = (IMAGE_THUNK_DATA*)GetPtrFromRVA(rvaIAT, nt_header, (PBYTE)Module);
				IMAGE_THUNK_DATA* thunkO = (IMAGE_THUNK_DATA*)GetPtrFromRVA(rvaINT, nt_header, (PBYTE)Module);
			
				if (!thunk ||  !thunkO)
				{
					_ASSERTE(thunk && thunkO);
					continue;
				}
			
				int f = 0;
				for (f = 0 ;; thunk++, thunkO++, f++)
				{
					//110220 - something strange. ������� ��� ������ �� ��������� �������� (AddFont.exe)
					//         ����� � ���, ��� thunk ��������� �� �� �������� ������� ������.
					//         ������ ������� �������, ��� ��������� ���� �������� ������� ��������.
					//Issue 466: We must check every thunk, not first (perl.exe fails?)
					//111127 - ..\GIT\lib\perl5\site_perl\5.8.8\msys\auto\SVN\_Core\_Core.dll
					//         ������, � ���� ��� ������ ������� ��������
					#ifndef USE_SEH
					if (IsBadReadPtr(thunk, sizeof(IMAGE_THUNK_DATA)))
					{
						_ASSERTE(thunk && FALSE);
						break;
					}
					#endif

					if (!thunk->u1.Function)
					{
						break;
					}

					#ifndef USE_SEH
					if (IsBadReadPtr(thunkO, sizeof(IMAGE_THUNK_DATA)))
					{
						_ASSERTE(thunkO && FALSE);
						break;
					}
					#endif
			
					const char* pszFuncName = NULL;

					// ��� ������� ��������� �� ������ ����
					if (s && thunk->u1.Function != thunkO->u1.Function && !IMAGE_SNAP_BY_ORDINAL(thunkO->u1.Ordinal))
					{
						pOrdinalNameO = (PIMAGE_IMPORT_BY_NAME)GetPtrFromRVA(thunkO->u1.AddressOfData, nt_header, (PBYTE)Module);

						#ifdef USE_SEH
							pszFuncName = (LPCSTR)pOrdinalNameO->Name;
						#else
							//WARNING!!! ������������� ������ IsBad???Ptr ����� ������� � ���������
							BOOL lbValidPtr = !IsBadReadPtr(pOrdinalNameO, sizeof(IMAGE_IMPORT_BY_NAME));
							#ifdef _DEBUG
							static bool bFirstAssert = false;
							if (!lbValidPtr && !bFirstAssert)
							{
								bFirstAssert = true;
								_ASSERTE(lbValidPtr);
							}
							#endif
				
							if (lbValidPtr)
							{
								//WARNING!!! ������������� ������ IsBad???Ptr ����� ������� � ���������
								lbValidPtr = !IsBadStringPtrA((LPCSTR)pOrdinalNameO->Name, 10);
								_ASSERTE(lbValidPtr);
				
								if (lbValidPtr)
									pszFuncName = (LPCSTR)pOrdinalNameO->Name;
							}
						#endif
					}
			
					int j;
			
					for (j = 0; gpHooks[j].Name; j++)
					{
						if (!gpHooks[j].OldAddress)
							continue; // ��� ������� �� ������������ (���� ������ ����?)
			
						// ����� ����� ������� (thunk) � gpHooks ����� NewAddress ��� ���
						if ((void*)thunk->u1.Function != gpHooks[j].NewAddress)
						{
							if (!pszFuncName)
							{
								continue;
							}
							else
							{
								if (lstrcmpA(pszFuncName, gpHooks[j].Name))
									continue;
							}
			
							// OldAddress ��� ����� ���������� �� ������������� �������� ����������
							// ��� ���� ������� �������� ��� ����� ���
						}
			
						// ���� �� ����� ���� - ������ ������� ������� (��� �� ������ ��� �� �����)
						// BugBug: � ��������, ��� ������� ��� �������� � ������ ������ (��� ����� ���),
						// �� ����� ������� ������������, ��� ����� ���������
						DWORD old_protect;
						if (VirtualProtect(&thunk->u1.Function, sizeof(thunk->u1.Function),
									   PAGE_READWRITE, &old_protect))
						{
							// BugBug: ExeOldAddress ����� ���������� �� �������������, ���� ������� ���� ����������� �� ���
							//if (abExecutable && gpHooks[j].ExeOldAddress)
							//	thunk->u1.Function = (DWORD_PTR)gpHooks[j].ExeOldAddress;
							//else
							thunk->u1.Function = (DWORD_PTR)gpHooks[j].OldAddress;
							VirtualProtect(&thunk->u1.Function, sizeof(thunk->u1.Function), old_protect, &old_protect);
							bUnhooked = true;
						}
						//DebugString( ToTchar( gpHooks[j].Name ) );
						break; // ������� � ���������� thunk-�
					}
				}
			}
		}
	wrap:
		res = bUnhooked;
	} SAFECATCH {
	}

	return res;
}

// ��������� ������������� ������� � ������
bool UnsetHook(HMODULE Module)
{
	if (!gpHooks)
		return false;

	if (!IsModuleValid(Module))
		return false;

	MSectionLock CS;
	if (!LockHooks(Module, L"uninstall", &CS))
		return false;

	HkModuleInfo* p = IsHookedModule(Module);
	bool bUnhooked = false;
	DWORD dwErr = (DWORD)-1;
	
	if (!p)
	{
		// ���� ������ � �� ������������� ����, �� ����� ����������, ��� � ���� ���������������� �������
		// ����� � ��������� �������, �.�. __try
		bUnhooked = UnsetHookInt(Module);
	}
	else
	{
		if (p->Hooked == 1)
		{
			for (size_t i = 0; i < MAX_HOOKED_PROCS; i++)
			{
				if (p->Addresses[i].pOur == 0)
					continue; // ���� ����� �������� �� ������
			
				DWORD old_protect;
				if (!VirtualProtect(p->Addresses[i].ppAdr, sizeof(*p->Addresses[i].ppAdr),
								   PAGE_READWRITE, &old_protect))
				{
					dwErr = GetLastError();
					_ASSERTEX(dwErr==ERROR_INVALID_ADDRESS);
				}
				else
				{
					bUnhooked = true;
					// BugBug: ExeOldAddress ����� ���������� �� �������������, ���� ������� ���� ����������� ��� ���
					//if (abExecutable && gpHooks[j].ExeOldAddress)
					//	thunk->u1.Function = (DWORD_PTR)gpHooks[j].ExeOldAddress;
					//else
					*p->Addresses[i].ppAdr = p->Addresses[i].pOld;
					p->Addresses[i].bHooked = FALSE;
					VirtualProtect(p->Addresses[i].ppAdr, sizeof(*p->Addresses[i].ppAdr), old_protect, &old_protect);
				}
			}
			// ���� �����
			p->Hooked = 2;
		}
	}


	#ifdef _DEBUG
	if (bUnhooked && p)
	{
		wchar_t* szDbg = (wchar_t*)calloc(MAX_PATH*3, 2);
		lstrcpy(szDbg, L"  ## Hooks was UNset by conemu: ");
		lstrcat(szDbg, p->sModuleName);
		lstrcat(szDbg, L"\n");
		DebugString(szDbg);
		free(szDbg);
	}
	#endif
	
	return bUnhooked;
}

void __stdcall UnsetAllHooks()
{
	#ifdef _DEBUG
	HMODULE hExecutable = GetModuleHandle(0);
	#endif
	//Warning: TH32CS_SNAPMODULE - ����� �������� ��� ������� �� LoadLibrary/FreeLibrary.
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, 0);

	if (snapshot != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 module = {sizeof(module)};

		for (BOOL res = Module32First(snapshot, &module); res; res = Module32Next(snapshot, &module))
		{
			if (module.hModule && !IsModuleExcluded(module.hModule, NULL, module.szModule))
			{
				//WARNING!!! OutputDebugString must NOT be used from ConEmuHk::DllMain(DLL_PROCESS_DETACH). See Issue 465
				DebugString(module.szModule);
				UnsetHook(module.hModule/*, (module.hModule == hExecutable)*/);
			}
		}

		CloseHandle(snapshot);
	}

#ifdef _DEBUG
	hExecutable = hExecutable;
#endif
}






/* **************************** *
 *                              *
 *  ����� ���� ���������� ����  *
 *                              *
 * **************************** */

void LoadModuleFailed(LPCSTR asModuleA, LPCWSTR asModuleW)
{
	DWORD dwErrCode = GetLastError();

	if (!gnLastLogSetChange)
	{
		CShellProc* sp = new CShellProc();
		if (sp)
		{
			gnLastLogSetChange = GetTickCount();
			gbLogLibraries = sp->LoadGuiMapping();
			delete sp;
		}
		SetLastError(dwErrCode);
	}

	if (!gbLogLibraries)
		return;


	CESERVER_REQ* pIn = NULL;
	wchar_t szModule[MAX_PATH+1]; szModule[0] = 0;
	wchar_t szErrCode[64]; szErrCode[0] = 0;
	msprintf(szErrCode, countof(szErrCode), L"ErrCode=0x%08X", dwErrCode);
	if (!asModuleA && !asModuleW)
	{
		wcscpy_c(szModule, L"<NULL>");
		asModuleW = szModule;
	}
	else if (asModuleA)
	{
		MultiByteToWideChar(AreFileApisANSI() ? CP_ACP : CP_OEMCP, 0, asModuleA, -1, szModule, countof(szModule));
		szModule[countof(szModule)-1] = 0;
		asModuleW = szModule;
	}
	pIn = ExecuteNewCmdOnCreate(eLoadLibrary, L"Fail", asModuleW, szErrCode, NULL, NULL, NULL, NULL,
		#ifdef _WIN64
		64
		#else
		32
		#endif
		, 0, NULL, NULL, NULL);
	if (pIn)
	{
		HWND hConWnd = GetConsoleWindow();
		CESERVER_REQ* pOut = ExecuteGuiCmd(hConWnd, pIn, hConWnd);
		ExecuteFreeResult(pIn);
		if (pOut) ExecuteFreeResult(pOut);
	}
	SetLastError(dwErrCode);
}

// � �������� �������� ������ (module) ����� ������������
// (���������� ��� �����������) � ������ ����������!
void CheckProcessModules(HMODULE hFromModule);

// �������� � ������ Module �������������� ������� �� ����������� �������� �������
// �� ���������, �.�. � Win32 ���������� shell32 ����� ���� ��������� ����� conemu.dll
//   ��� ������� ������������ �������� �������,
// � � Win64 �������� ������ ������ ���� 64�������, � ��������� ������ ������ ������ 32������ ��������

bool PrepareNewModule(HMODULE module, LPCSTR asModuleA, LPCWSTR asModuleW, BOOL abNoSnapshoot = FALSE)
{
	if (!module)
	{
		LoadModuleFailed(asModuleA, asModuleW);

		// � �������� �������� ������ (module) ����� ������������
		// (���������� ��� �����������) � ������ ����������!
		CheckProcessModules(module);
		return false;
	}

	// ��������� �� gpHookedModules � �� ��� �� ������ ��� ���������?
	if (IsHookedModule(module))
	{
		// ���� ������ ��� ���������!
		return false;
	}

	bool lbModuleOk = false;

	BOOL lbResource = LDR_IS_RESOURCE(module);

	CShellProc* sp = new CShellProc();
	if (sp != NULL)
	{
		if (!gnLastLogSetChange || ((GetTickCount() - gnLastLogSetChange) > 2000))
		{
			gnLastLogSetChange = GetTickCount();
			gbLogLibraries = sp->LoadGuiMapping();
		}

		if (gbLogLibraries)
		{
			CESERVER_REQ* pIn = NULL;
			LPCWSTR pszModuleW = asModuleW;
			wchar_t szModule[MAX_PATH+1]; szModule[0] = 0;
			if (!asModuleA && !asModuleW)
			{
				wcscpy_c(szModule, L"<NULL>");
				pszModuleW = szModule;
			}
			else if (asModuleA)
			{
				MultiByteToWideChar(AreFileApisANSI() ? CP_ACP : CP_OEMCP, 0, asModuleA, -1, szModule, countof(szModule));
				szModule[countof(szModule)-1] = 0;
				pszModuleW = szModule;
			}
			wchar_t szInfo[64]; szInfo[0] = 0;
			#ifdef _WIN64
			if ((DWORD)((DWORD_PTR)module >> 32))
				msprintf(szInfo, countof(szInfo), L"Module=0x%08X%08X",
					(DWORD)((DWORD_PTR)module >> 32), (DWORD)((DWORD_PTR)module & 0xFFFFFFFF)); //-V112
			else
				msprintf(szInfo, countof(szInfo), L"Module=0x%08X",
					(DWORD)((DWORD_PTR)module & 0xFFFFFFFF)); //-V112
			#else
			msprintf(szInfo, countof(szInfo), L"Module=0x%08X", (DWORD)module);
			#endif
			pIn = sp->NewCmdOnCreate(eLoadLibrary, NULL, pszModuleW, szInfo, NULL, NULL, NULL, NULL,
				#ifdef _WIN64
				64
				#else
				32
				#endif
				, 0, NULL, NULL, NULL);
			if (pIn)
			{
				HWND hConWnd = GetConsoleWindow();
				CESERVER_REQ* pOut = ExecuteGuiCmd(hConWnd, pIn, hConWnd);
				ExecuteFreeResult(pIn);
				if (pOut) ExecuteFreeResult(pOut);
			}
		}

		delete sp;
		sp = NULL;
	}


	if (!lbResource)
	{
		if (!abNoSnapshoot /*&& !lbResource*/)
		{
			// ��������� ��������������� ���������� ����� ����
			// �� ��������� �� ����� ��������� �������������
			// �������������� ��� ��� (���� ��� ���������) �����
			// �������� "������������" ������ ��������
			InitHooks(NULL);

			// � �������� �������� ������ (module) ����� ������������
			// (���������� ��� �����������) � ������ ����������!
			CheckProcessModules(module);
		}


		if (!IsModuleExcluded(module, asModuleA, asModuleW))
		{
			wchar_t szModule[128] = {};
			if (asModuleA)
			{
				LPCSTR pszNameA = strrchr(asModuleA, '\\');
				if (!pszNameA) pszNameA = asModuleA; else pszNameA++;
				MultiByteToWideChar(CP_ACP, 0, pszNameA, -1, szModule, countof(szModule)-1);				
			}
			else if (asModuleW)
			{
				LPCWSTR pszNameW = wcsrchr(asModuleW, L'\\');
				if (!pszNameW) pszNameW = asModuleW; else pszNameW++;
				lstrcpyn(szModule, pszNameW, countof(szModule));
			}
			
			lbModuleOk = true;
			// ������� ������������� ������� � module
			SetHook(szModule, module, FALSE);
		}
	}

	return lbModuleOk;
}

// � �������� �������� ������ (module) ����� ������������
// (���������� ��� �����������) � ������ ����������!
void CheckProcessModules(HMODULE hFromModule)
{
	WARNING("TH32CS_SNAPMODULE - ����� �������� ��� ������� �� LoadLibrary/FreeLibrary!!!");
	// �����, ����� ����� ��������� ������� ����, � ������� ��������� ��� ����������� ������?

	//Warning: TH32CS_SNAPMODULE - ����� �������� ��� ������� �� LoadLibrary/FreeLibrary.
	HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
	MODULEENTRY32 mi = {sizeof(mi)};
	if (h && h != INVALID_HANDLE_VALUE && Module32First(h, &mi))
	{
		BOOL lbAddMod = FALSE;
		do {
			//CheckLoadedModule(mi.szModule);
			//if (!ghUser32)
			//{
			//	// ���� �� ������ exe-����� user32 �� ������������� - ����� ��������� �� ���� ��������� ���������!
			//	if (*mi.szModule && (!lstrcmpiW(mi.szModule, L"user32.dll") || !lstrcmpiW(mi.szModule, L"user32")))
			//	{
			//		ghUser32 = LoadLibraryW(user32); // LoadLibrary, �.�. � ��� �� ����� - ��������� �������
			//		//InitHooks(NULL); -- ���� � ��� ����� ���������
			//	}
			//}
			//if (!ghShell32)
			//{
			//	// ���� �� ������ exe-����� shell32 �� ������������� - ����� ��������� �� ���� ��������� ���������!
			//	if (*mi.szModule && (!lstrcmpiW(mi.szModule, L"shell32.dll") || !lstrcmpiW(mi.szModule, L"shell32")))
			//	{
			//		ghShell32 = LoadLibraryW(shell32); // LoadLibrary, �.�. � ��� �� ����� - ��������� �������
			//		//InitHooks(NULL); -- ���� � ��� ����� ���������
			//	}
			//}
			//if (!ghAdvapi32)
			//{
			//	// ���� �� ������ exe-����� advapi32 �� ������������� - ����� ��������� �� ���� ��������� ���������!
			//	if (*mi.szModule && (!lstrcmpiW(mi.szModule, L"advapi32.dll") || !lstrcmpiW(mi.szModule, L"advapi32")))
			//	{
			//		ghAdvapi32 = LoadLibraryW(advapi32); // LoadLibrary, �.�. � ��� �� ����� - ��������� �������
			//		if (ghAdvapi32)
			//		{
			//			RegOpenKeyEx_f = (RegOpenKeyEx_t)GetProcAddress(ghAdvapi32, "RegOpenKeyExW");
			//			RegCreateKeyEx_f = (RegCreateKeyEx_t)GetProcAddress(ghAdvapi32, "RegCreateKeyExW");
			//			RegCloseKey_f = (RegCloseKey_t)GetProcAddress(ghAdvapi32, "RegCloseKey");
			//		}
			//		//InitHooks(NULL); -- ���� � ��� ����� ���������
			//	}
			//}

			if (lbAddMod)
			{
				if (PrepareNewModule(mi.hModule, NULL, mi.szModule, TRUE/*�� ����� CheckProcessModules*/))
					CheckLoadedModule(mi.szModule);
			}
			else if (mi.hModule == hFromModule)
			{
				lbAddMod = TRUE;
			}
		} while (Module32Next(h, &mi));
		CloseHandle(h);
	}
}


typedef HMODULE(WINAPI* OnLoadLibraryA_t)(const char* lpFileName);
HMODULE WINAPI OnLoadLibraryA(const char* lpFileName)
{
	ORIGINAL(LoadLibraryA);
	HMODULE module = F(LoadLibraryA)(lpFileName);

	if (gbHooksTemporaryDisabled)
		return module;

	if (PrepareNewModule(module, lpFileName, NULL))
	{
		if (ph && ph->PostCallBack)
		{
			SETARGS1(&module,lpFileName);
			ph->PostCallBack(&args);
		}
	}

	return module;
}

typedef HMODULE(WINAPI* OnLoadLibraryW_t)(const wchar_t* lpFileName);
HMODULE WINAPI OnLoadLibraryW(const wchar_t* lpFileName)
{
	ORIGINAL(LoadLibraryW);
	HMODULE module = F(LoadLibraryW)(lpFileName);

	if (gbHooksTemporaryDisabled)
		return module;

#if 1
	#ifdef _WIN64
	DWORD dwErrCode = 0;	
	if (!module)
	{
		dwErrCode = GetLastError();
		//0x7E - module not found
		//0xC1 - module �� �������� ����������� Win32.
		if ((dwErrCode == ERROR_MOD_NOT_FOUND || dwErrCode == ERROR_BAD_EXE_FORMAT)
			&& lpFileName && (lstrcmpiW(lpFileName, L"extendedconsole.dll") == 0))
		{
			//_ASSERTE(module!=NULL); // ��� ���������� ��������� 2 ���� (����������� � �������� ��������)
			module = F(LoadLibraryW)(L"ExtendedConsole64.dll");
		}
	}
	#endif
#endif

	if (PrepareNewModule(module, NULL, lpFileName))
	{
		if (ph && ph->PostCallBack)
		{
			SETARGS1(&module,lpFileName);
			ph->PostCallBack(&args);
		}
	}

	return module;
}

typedef HMODULE(WINAPI* OnLoadLibraryExA_t)(const char* lpFileName, HANDLE hFile, DWORD dwFlags);
HMODULE WINAPI OnLoadLibraryExA(const char* lpFileName, HANDLE hFile, DWORD dwFlags)
{
	ORIGINAL(LoadLibraryExA);
	HMODULE module = F(LoadLibraryExA)(lpFileName, hFile, dwFlags);

	if (gbHooksTemporaryDisabled)
		return module;

	if (PrepareNewModule(module, lpFileName, NULL))
	{
		if (ph && ph->PostCallBack)
		{
			SETARGS3(&module,lpFileName,hFile,dwFlags);
			ph->PostCallBack(&args);
		}
	}

	return module;
}

typedef HMODULE(WINAPI* OnLoadLibraryExW_t)(const wchar_t* lpFileName, HANDLE hFile, DWORD dwFlags);
HMODULE WINAPI OnLoadLibraryExW(const wchar_t* lpFileName, HANDLE hFile, DWORD dwFlags)
{
	ORIGINAL(LoadLibraryExW);
	HMODULE module = F(LoadLibraryExW)(lpFileName, hFile, dwFlags);

	if (gbHooksTemporaryDisabled)
		return module;

	if (PrepareNewModule(module, NULL, lpFileName))
	{
		if (ph && ph->PostCallBack)
		{
			SETARGS3(&module,lpFileName,hFile,dwFlags);
			ph->PostCallBack(&args);
		}
	}

	return module;
}

FARPROC WINAPI OnGetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
	typedef FARPROC(WINAPI* OnGetProcAddress_t)(HMODULE hModule, LPCSTR lpProcName);
	ORIGINALFAST(GetProcAddress);
	FARPROC lpfn = NULL;

	#ifdef LOG_ORIGINAL_CALL
	char gszLastGetProcAddress[255];
	#endif

	if (gbHooksTemporaryDisabled)
	{
		TODO("!!!");
		#ifdef LOG_ORIGINAL_CALL
		msprintf(gszLastGetProcAddress, countof(gszLastGetProcAddress),
			(((DWORD_PTR)lpProcName) <= 0xFFFF) ? "   OnGetProcAddress(x%08X,%u)\n" : "   OnGetProcAddress(x%08X,%s)\n",
			(DWORD)hModule, lpProcName);
		#endif
	}
	else if (((DWORD_PTR)lpProcName) <= 0xFFFF)
	{
		TODO("!!! ������������ � ORDINAL values !!!");
		#ifdef LOG_ORIGINAL_CALL
		msprintf(gszLastGetProcAddress, countof(gszLastGetProcAddress), "   OnGetProcAddress(x%08X,%u)\n",
			(DWORD)hModule, (DWORD)lpProcName);
		#endif
	}
	else
	{
		#ifdef LOG_ORIGINAL_CALL
		msprintf(gszLastGetProcAddress, countof(gszLastGetProcAddress), "   OnGetProcAddress(x%08X,%s)\n",
			(DWORD)hModule, lpProcName);
		#endif

		if (gpHooks)
		{
			if (gbDllStopCalled)
			{
				//-- assert ������, �.�. ��� ��� ������������������!
				//_ASSERTE(ghHeap!=NULL);
				lpfn = NULL;
			}
			else
			{
				for (int i = 0; gpHooks[i].Name; i++)
				{
					// The spelling and case of a function name pointed to by lpProcName must be identical
					// to that in the EXPORTS statement of the source DLL's module-definition (.Def) file
					if (gpHooks[i].hDll == hModule
							&& lstrcmpA(gpHooks[i].Name, lpProcName) == 0)
					{
						lpfn = (FARPROC)gpHooks[i].NewAddress;
						break;
					}
				}
			}
		}
	}

	#ifdef LOG_ORIGINAL_CALL
	OutputDebugStringA(gszLastGetProcAddress);
	#endif

	if (!lpfn)
		lpfn = F(GetProcAddress)(hModule, lpProcName);

	return lpfn;
}

BOOL WINAPI OnFreeLibrary(HMODULE hModule)
{
	typedef BOOL (WINAPI* OnFreeLibrary_t)(HMODULE hModule);
	ORIGINALFAST(FreeLibrary);
	BOOL lbRc = FALSE;
	BOOL lbResource = LDR_IS_RESOURCE(hModule);
	// lbResource ���������� TRUE �������� ��� ������� �� version.dll
	//BOOL lbProcess = !lbResource;
	wchar_t szModule[MAX_PATH*2]; szModule[0] = 0;

	if (gbLogLibraries && !gbDllStopCalled)
	{
		CShellProc* sp = new CShellProc();
		if (sp->LoadGuiMapping())
		{
			CESERVER_REQ* pIn = NULL;
			szModule[0] = 0;
			wchar_t szHandle[32] = {};
			#ifdef _WIN64
				msprintf(szHandle, countof(szModule), L", <HMODULE=0x%08X%08X>",
					(DWORD)((((u64)hModule) & 0xFFFFFFFF00000000) >> 32), //-V112
					(DWORD)(((u64)hModule) & 0xFFFFFFFF)); //-V112
			#else
				msprintf(szHandle, countof(szModule), L", <HMODULE=0x%08X>", (DWORD)hModule);
			#endif
			
			// GetModuleFileName � ��������� ������� �������� O_O. �������, ���������� � ��������� ������� ��� ������������ ����� ������
			if (FindModuleFileName(hModule, szModule, countof(szModule)-lstrlen(szModule)-1))
				wcscat_c(szModule, szHandle);
			else
				wcscpy_c(szModule, szHandle+2);

			pIn = sp->NewCmdOnCreate(eFreeLibrary, NULL, szModule, NULL, NULL, NULL, NULL, NULL,
				#ifdef _WIN64
				64
				#else
				32
				#endif
				, 0, NULL, NULL, NULL);
			if (pIn)
			{
				HWND hConWnd = GetConsoleWindow();
				CESERVER_REQ* pOut = ExecuteGuiCmd(hConWnd, pIn, hConWnd);
				ExecuteFreeResult(pIn);
				if (pOut) ExecuteFreeResult(pOut);
			}
		}
		delete sp;
	}

#ifdef _DEBUG
	BOOL lbModulePre = IsModuleValid(hModule); // GetModuleFileName(hModule, szModule, countof(szModule));
#endif

	lbRc = F(FreeLibrary)(hModule);

	// ����� ������ ���� !LDR_IS_RESOURCE
	if (lbRc && !lbResource && !gbDllStopCalled)
	{
		// ��������� ����������, ������������� �� ������ ��������, ��� ������ ������� ����������
		BOOL lbModulePost = IsModuleValid(hModule); // GetModuleFileName(hModule, szModule, countof(szModule));
		#ifdef _DEBUG
		DWORD dwErr = lbModulePost ? 0 : GetLastError();
		#endif

		if (!lbModulePost)
		{
			RemoveHookedModule(hModule);

			if (ghOnLoadLibModule == hModule)
			{
				ghOnLoadLibModule = NULL;
				gfOnLibraryLoaded = NULL;
				gfOnLibraryUnLoaded = NULL;
			}

			if (gpHooks)
			{
				for (int i = 0; i<MAX_HOOKED_PROCS && gpHooks[i].NewAddress; i++)
				{
					if (gpHooks[i].hCallbackModule == hModule)
					{
						gpHooks[i].hCallbackModule = NULL;
						gpHooks[i].PreCallBack = NULL;
						gpHooks[i].PostCallBack = NULL;
						gpHooks[i].ExceptCallBack = NULL;
					}
				}
			}
			
			TODO("���� �� ���� ����������, ��� � CheckLoadedModule");

			if (gfOnLibraryUnLoaded)
			{
				gfOnLibraryUnLoaded(hModule);
			}

			if (ghUser32 && (hModule == ghUser32))
			{
				if (GetModuleHandle(user32) == NULL)
					Is_Window = NULL;
			}

			if (ghAdvapi32 && (hModule == ghAdvapi32))
			{
				if (GetModuleHandle(advapi32) == NULL)
				{
					RegOpenKeyEx_f = NULL;
					RegCreateKeyEx_f = NULL;
					RegCloseKey_f = NULL;
				}
			}
			
			if (ghComdlg32 && (hModule == ghComdlg32))
			{
				if (GetModuleHandle(comdlg32) == NULL)
				{
					ChooseColorA_f = NULL;
					ChooseColorW_f = NULL;
				}
			}
		}
	}

	return lbRc;
}


#ifdef HOOK_ERROR_PROC
DWORD WINAPI OnGetLastError()
{
	typedef DWORD (WINAPI* OnGetLastError_t)();
	SUPPRESSORIGINALSHOWCALL;
	ORIGINALFAST(GetLastError);
	DWORD nErr = 0;

	if (F(GetLastError))
		nErr = F(GetLastError)();

	if (nErr == HOOK_ERROR_NO)
	{
		nErr = HOOK_ERROR_NO;
	}
	return nErr;
}
VOID WINAPI OnSetLastError(DWORD dwErrCode)
{
	typedef DWORD (WINAPI* OnSetLastError_t)(DWORD dwErrCode);
	SUPPRESSORIGINALSHOWCALL;
	ORIGINALFAST(SetLastError);

	if (dwErrCode == HOOK_ERROR_NO)
	{
		dwErrCode = HOOK_ERROR_NO;
	}

	if (F(SetLastError))
		F(SetLastError)(dwErrCode);
}
#endif
