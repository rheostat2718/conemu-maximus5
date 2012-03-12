/*
dirmix.cpp

Misc functions for working with directories
*/
/*
Copyright � 1996 Eugene Roshal
Copyright � 2000 Far Group
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

#include "headers.hpp"
#pragma hdrstop

#include "dirmix.hpp"
#include "cvtname.hpp"
#include "message.hpp"
#include "language.hpp"
#include "lasterror.hpp"
#include "flink.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "treelist.hpp"
#include "config.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "interf.hpp"
#include "elevation.hpp"

BOOL FarChDir(const wchar_t *NewDir, BOOL ChangeDir)
{
	if (!NewDir || !*NewDir)
		return FALSE;

	BOOL rc=FALSE;
	string Drive(L"=A:");
	string strCurDir;

	if (*NewDir && NewDir[1]==L':' && !NewDir[2]) // ���� ������� ������
	{                                             // ����� �����, �� ����
		Drive.Replace(1, Upper(*NewDir));         // ������� �� ����������

		if (!apiGetEnvironmentVariable(Drive, strCurDir))
		{
			strCurDir = NewDir;
			AddEndSlash(strCurDir);
			ReplaceSlashToBSlash(strCurDir);
		}

		//*CurDir=toupper(*CurDir); ����!
		if (ChangeDir)
		{
			rc=apiSetCurrentDirectory(strCurDir);
		}
	}
	else
	{
		if (ChangeDir)
		{
			strCurDir = NewDir;

			if (!StrCmp(strCurDir,L"\\"))
				apiGetCurrentDirectory(strCurDir); // ����� ����� ������

			ReplaceSlashToBSlash(strCurDir);
			ConvertNameToFull(NewDir,strCurDir);
			PrepareDiskPath(strCurDir,false); // TRUE ???
			rc=apiSetCurrentDirectory(strCurDir);
		}
	}

	if (rc || !ChangeDir)
	{
		if ((!ChangeDir || apiGetCurrentDirectory(strCurDir)) &&
		        strCurDir.At(0) && strCurDir.At(1)==L':')
		{
			Drive.Replace(1, Upper(strCurDir.At(0)));
			SetEnvironmentVariable(Drive,strCurDir);
		}
	}

	return rc;
}

/*
  ������� TestFolder ���������� ���� ��������� ������������ ��������:

    TSTFLD_NOTEMPTY   (2) - �� �����
    TSTFLD_EMPTY      (1) - �����
    TSTFLD_NOTFOUND   (0) - ��� ������
    TSTFLD_NOTACCESS (-1) - ��� �������
    TSTFLD_ERROR     (-2) - ������ (������ ��������� ��� ��������� ������ ��� ��������� ������������� �������)
*/
int TestFolder(const wchar_t *Path)
{
	if (!(Path && *Path)) // �������� �� ��������
		return TSTFLD_ERROR;

	string strFindPath = Path;
	// ��������� ����� ��� ������.
	AddEndSlash(strFindPath);
	strFindPath += L"*";

	// ������ �������� - ��-���� ������� �����?
	FAR_FIND_DATA_EX fdata;
	FindFile Find(strFindPath);

	bool bFind = false;
	if(Find.Get(fdata))
	{
		return TSTFLD_NOTEMPTY;
	}

	if (!bFind)
	{
		GuardLastError gle;
		DWORD LastError = GetLastError();
		if (LastError == ERROR_FILE_NOT_FOUND || LastError == ERROR_NO_MORE_FILES)
			return TSTFLD_EMPTY;

		if (LastError == ERROR_PATH_NOT_FOUND)
			return TSTFLD_NOTFOUND;

		// ����������... �� ����, ��� ���� �� ������, �.�. �� ������ ����� � ����� ���� ���� "."
		// ������� ��������� �� Root
		GetPathRoot(Path,strFindPath);

		if (!StrCmp(Path,strFindPath))
		{
			// �������� ��������� ������������� ������ - ��� ���� BugZ#743 ��� ������ ������ �����.
			if (apiGetFileAttributes(strFindPath)!=INVALID_FILE_ATTRIBUTES)
			{
				if (LastError == ERROR_ACCESS_DENIED)
					return TSTFLD_NOTACCESS;

				return TSTFLD_EMPTY;
			}
		}

		strFindPath = Path;

		if (CheckShortcutFolder(&strFindPath,FALSE,TRUE))
		{
			if (StrCmp(Path,strFindPath))
				return TSTFLD_NOTFOUND;
		}

		{
			DisableElevation de;

			DWORD Attr=apiGetFileAttributes(strFindPath);

			if (Attr!=INVALID_FILE_ATTRIBUTES && !(Attr&FILE_ATTRIBUTE_DIRECTORY))
				return TSTFLD_ERROR;
		}
		return TSTFLD_NOTACCESS;
	}


	// ���������� ������� ����
	return TSTFLD_EMPTY;
}

/*
   �������� ���� ��� ����-����� �� �������������
   ���� ���� �������� ���� (IsHostFile=FALSE), �� �����
   ����������� ������� ����� ��������� ����. ��������� �������
   ������������ � ���������� TestPath.

   Return: 0 - ����.
           1 - ���!,
          -1 - ����� ��� ���, �� ProcessPluginEvent ������ TRUE
   TestPath ����� ���� ������, ����� ������ �������� ProcessPluginEvent()

*/
int CheckShortcutFolder(string *pTestPath,int IsHostFile, BOOL Silent)
{
	if (pTestPath && !pTestPath->IsEmpty() && apiGetFileAttributes(*pTestPath) == INVALID_FILE_ATTRIBUTES)
	{
		int FoundPath=0;
		string strTarget = *pTestPath;
		TruncPathStr(strTarget, ScrX-16);

		if (IsHostFile)
		{
			SetLastError(ERROR_FILE_NOT_FOUND);

			if (!Silent)
				Message(MSG_WARNING | MSG_ERRORTYPE, 1, MSG(MError), strTarget, MSG(MOk));
		}
		else // ������� �����!
		{
			SetLastError(ERROR_PATH_NOT_FOUND);

			if (Silent || !Message(MSG_WARNING | MSG_ERRORTYPE, 2, MSG(MError), strTarget, MSG(MNeedNearPath), MSG(MHYes),MSG(MHNo)))
			{
				string strTestPathTemp = *pTestPath;

				for (;;)
				{
					if (!CutToSlash(strTestPathTemp,true))
						break;

					if (apiGetFileAttributes(strTestPathTemp) != INVALID_FILE_ATTRIBUTES)
					{
						int ChkFld=TestFolder(strTestPathTemp);

						if (ChkFld > TSTFLD_ERROR && ChkFld < TSTFLD_NOTFOUND)
						{
							if (!(pTestPath->At(0) == L'\\' && pTestPath->At(1) == L'\\' && !strTestPathTemp.At(1)))
							{
								*pTestPath = strTestPathTemp;

								if (pTestPath->GetLength() == 2) // ��� ������ "C:", ����� ������� � ������� ������� ����� C:
									AddEndSlash(*pTestPath);

								FoundPath=1;
							}

							break;
						}
					}
				}
			}
		}

		if (!FoundPath)
			return 0;
	}

	if (CtrlObject->Cp()->ActivePanel->ProcessPluginEvent(FE_CLOSE,nullptr))
		return -1;

	return 1;
}

void CreatePath(string &strPath, bool Simple)
{
	wchar_t *ChPtr = strPath.GetBuffer();
	wchar_t *DirPart = ChPtr;
	BOOL bEnd = FALSE;

	for (;;)
	{
		if (!*ChPtr || IsSlash(*ChPtr))
		{
			if (!*ChPtr)
				bEnd = TRUE;

			*ChPtr = 0;

			bool Exist = apiGetFileAttributes(strPath) != INVALID_FILE_ATTRIBUTES;
			if (!Simple && Opt.CreateUppercaseFolders && !IsCaseMixed(DirPart) && !Exist)  //BUGBUG
				CharUpper(DirPart);

			//Maximus: �������� (����������) ��������������� ������� "A:", "A:\4", "A:\4\" (���������-�� ������?)
			//Maximus: BUGBUG: ������������ ������ �������� ��������!
			if(!Exist && apiCreateDirectory(strPath, nullptr) && !Simple)
				TreeList::AddTreeName(strPath);

			if (bEnd)
				break;

			*ChPtr = L'\\';
			DirPart = ChPtr+1;
		}

		ChPtr++;
	}

	strPath.ReleaseBuffer();
}
