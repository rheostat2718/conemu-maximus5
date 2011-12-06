
#include <windows.h>
#include "ConEmuC.h"
#include "RegPrepare.h"
#include "TokenHelper.h"
#include "PrivilegesHelper.h"


// 0 - SUCCEEDED, otherwise - error code
int WINAPI MountVirtualHive(LPCWSTR asHive, PHKEY phKey, LPCWSTR asXPMountName, wchar_t* pszErrInfo, int cchErrInfoMax, BOOL* pbKeyMounted)
{
	int lRc = -1;
	
	if (pszErrInfo && cchErrInfoMax)
		*pszErrInfo = 0; // ���� �������� ����� ��� ������ - ����� ��� ���������

	*pbKeyMounted = FALSE;

	OSVERSIONINFO osv = {sizeof(OSVERSIONINFO)};
	GetVersionEx(&osv);
	HMODULE hAdvapi32 = LoadLibrary(L"advapi32.dll");
	//LPCWSTR pszKeyName = NULL;
	LPCWSTR ppszKeys[] =
	{
		L"HKCU",
		L"HKCU\\Software",
		L"HKLM",
		L"HKLM\\Software",
		L"HKLM64",
		L"HKLM64\\Software"
	};
	size_t nRootKeys = IsWindows64() ? countof(ppszKeys) : (countof(ppszKeys) - 2);

	if (!hAdvapi32)
	{
		if (pszErrInfo && cchErrInfoMax)
			msprintf(pszErrInfo, cchErrInfoMax,
				L"LoadLibrary(advapi32.dll) failed, code=0x%08X!\n", GetLastError());
		lRc = -2;
		goto wrap;
	}
	
	if (osv.dwMajorVersion >= 6)
	{
		// Vista+
		typedef LONG (WINAPI* RegLoadAppKey_t)(LPCWSTR lpFile, PHKEY phkResult, REGSAM samDesired, DWORD dwOptions, DWORD Reserved);
		RegLoadAppKey_t RegLoadAppKey_f = (RegLoadAppKey_t)GetProcAddress(hAdvapi32, "RegLoadAppKeyW");
		if (!RegLoadAppKey_f)
		{
			if (pszErrInfo && cchErrInfoMax)
				msprintf(pszErrInfo, cchErrInfoMax,
					L"RegLoadAppKeyW not found, code=0x%08X!\n", GetLastError());
			lRc = -3;
			goto wrap;
		}
		else
		{
			if ((lRc = RegLoadAppKey_f(asHive, phKey, KEY_ALL_ACCESS, 0, 0)) != 0)
			{
				if ((lRc = RegLoadAppKey_f(asHive, phKey, KEY_READ, 0, 0)) != 0)
				{
					if (pszErrInfo && cchErrInfoMax)
						msprintf(pszErrInfo, cchErrInfoMax,
							L"RegLoadAppKey failed, code=0x%08X!", (DWORD)lRc);
					lRc = -4; //-V112
					goto wrap;
				}
			}
			*pbKeyMounted = TRUE;
		}
	}
	else if (!asXPMountName || !*asXPMountName)
	{
		lRc = -7;
		if (pszErrInfo && cchErrInfoMax)
			lstrcpyn(pszErrInfo, L"XPMountName is empty!", cchErrInfoMax);
		goto wrap;
	}
	else
	{
		CBackupPrivileges se;
		if (!se.BackupPrivilegesAcuire(TRUE))
		{
			if (pszErrInfo && cchErrInfoMax)
				msprintf(pszErrInfo, cchErrInfoMax,
					L"Aquiring SE_BACKUP_NAME/SE_RESTORE_NAME failed, code=0x%08X!\nYou must be Administrator or Backup operator", GetLastError());
			lRc = -5;
			goto wrap;
		}
		
		//_wcscpy_c(rsXPMountName, cchXPMountMax, VIRTUAL_REGISTRY_GUID);
		//WARNING("###: �������� � ����� ���-��� ����������, �������� CRC ���� � hive");
		
		// Hive ��� ��� ���� ��������� ������ ������ ConEmu.
		TODO("��� ������ - ����� ���������� ��������? ��� ������ ������� RegUnloadKey...");
		
		if ((lRc = RegOpenKeyEx(HKEY_USERS, asXPMountName, 0, KEY_ALL_ACCESS, phKey)) == 0)
		{
			goto wrap; // ������� - hive ��� ���������
		}
		else if ((lRc = RegOpenKeyEx(HKEY_USERS, asXPMountName, 0, KEY_READ, phKey)) == 0)
		{
			goto wrap; // ������� - hive ��� ��������� (ReadOnly)
		}
		
		// Hive ��� �� ��� ���������
		if ((lRc = RegLoadKey(HKEY_USERS, asXPMountName, asHive)) != 0)
		{
			if (pszErrInfo && cchErrInfoMax)
				msprintf(pszErrInfo, cchErrInfoMax,
					L"RegLoadKey failed, code=0x%08X!", (DWORD)lRc);
			lRc = -6;
			goto wrap;
		}
		// ���� �����������, ����� ��� ����� ������������� ��� ������
		*pbKeyMounted = TRUE;

		if ((lRc = RegOpenKeyEx(HKEY_USERS, asXPMountName, 0, KEY_ALL_ACCESS, phKey)) == 0)
		{
			goto wrap; // ������� - hive ��� ���������
		}
		else if ((lRc = RegOpenKeyEx(HKEY_USERS, asXPMountName, 0, KEY_READ, phKey)) == 0)
		{
			goto wrap; // ������� - hive ��� ��������� (ReadOnly)
		}
	}
	
	// ����� ���������, ����� �� �������/������� ����������� �����
	for (UINT i = 0; i < nRootKeys; i++)
	{
		HKEY hTest = NULL;
		LPCWSTR pszKeyName = ppszKeys[i];
		lRc = RegCreateKeyEx(*phKey, pszKeyName, 0,0,0, KEY_ALL_ACCESS, 0, &hTest, 0);
		if (lRc != 0)
			lRc = RegCreateKeyEx(*phKey, pszKeyName, 0,0,0, KEY_READ, 0, &hTest, 0);
		if (lRc != 0)
		{
			if (pszErrInfo && cchErrInfoMax)
				msprintf(pszErrInfo, cchErrInfoMax,
					L"RegCreateKeyEx(%s) failed, code=0x%08X!", pszKeyName, (DWORD)lRc);
			RegCloseKey(*phKey);
			*phKey = NULL;
			if (asXPMountName && *asXPMountName)
				UnMountVirtualHive(asXPMountName, NULL, 0);
			lRc = -8;
			goto wrap;
		}
	}

wrap:
	if (hAdvapi32)
		FreeLibrary(hAdvapi32); // Decrease counter
	return lRc;
}

int WINAPI UnMountVirtualHive(LPCWSTR asXPMountName, wchar_t* pszErrInfo, int cchErrInfoMax)
{
	int lRc = -1;

	CBackupPrivileges se;
	HMODULE hAdvapi32 = LoadLibrary(L"advapi32.dll");
	
	if (!hAdvapi32)
	{
		if (pszErrInfo && cchErrInfoMax)
			msprintf(pszErrInfo, cchErrInfoMax,
				L"LoadLibrary(advapi32.dll) failed, code=0x%08X!\n", GetLastError());
		lRc = -2;
		goto wrap;
	}
	
	if (!se.BackupPrivilegesAcuire(TRUE))
	{
		if (pszErrInfo && cchErrInfoMax)
			msprintf(pszErrInfo, cchErrInfoMax,
				L"Aquiring SE_BACKUP_NAME/SE_RESTORE_NAME failed, code=0x%08X!\nYou must be Administrator or Backup operator", GetLastError());
		goto wrap;
	}
	
	if ((lRc = RegUnLoadKey(HKEY_USERS, asXPMountName)) != 0)
	{
		if (pszErrInfo && cchErrInfoMax)
			msprintf(pszErrInfo, cchErrInfoMax,
				L"RegUnLoadKey failed, code=0x%08X!", (DWORD)lRc);
	}
	
wrap:
	if (hAdvapi32)
		FreeLibrary(hAdvapi32); // Decrease counter
	return lRc;
}
