/*
flink.cpp

���� ������ ������� �� ��������� Link`�� - Hard&Sym
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

#include "copy.hpp"
#include "flink.hpp"
#include "imports.hpp"
#include "cddrv.hpp"
#include "config.hpp"
#include "pathmix.hpp"
#include "drivemix.hpp"
#include "panelmix.hpp"
#include "privilege.hpp"
#include "message.hpp"
#include "lang.hpp"
#include "language.hpp"
#include "dirmix.hpp"
#include "treelist.hpp"
#include "elevation.hpp"

bool WINAPI CreateVolumeMountPoint(const wchar_t *TargetVolume, const wchar_t *Object)
{
	bool Result=false;
	string strBuf;

	if (apiGetVolumeNameForVolumeMountPoint(TargetVolume,strBuf))
	{
		if (SetVolumeMountPoint(Object,strBuf))
		{
			Result=true;
		}
	}

	return Result;
}

bool FillREPARSE_DATA_BUFFER(PREPARSE_DATA_BUFFER rdb,LPCWSTR PrintName,size_t PrintNameLength,LPCWSTR SubstituteName,size_t SubstituteNameLength)
{
	bool Result=false;
	rdb->Reserved=0;

	switch (rdb->ReparseTag)
	{
		case IO_REPARSE_TAG_MOUNT_POINT:
			rdb->MountPointReparseBuffer.SubstituteNameOffset=0;
			rdb->MountPointReparseBuffer.SubstituteNameLength=static_cast<WORD>(SubstituteNameLength*sizeof(wchar_t));
			rdb->MountPointReparseBuffer.PrintNameOffset=rdb->MountPointReparseBuffer.SubstituteNameLength+2;
			rdb->MountPointReparseBuffer.PrintNameLength=static_cast<WORD>(PrintNameLength*sizeof(wchar_t));
			rdb->ReparseDataLength=FIELD_OFFSET(REPARSE_DATA_BUFFER,MountPointReparseBuffer.PathBuffer)+rdb->MountPointReparseBuffer.PrintNameOffset+rdb->MountPointReparseBuffer.PrintNameLength+1*sizeof(wchar_t)-REPARSE_DATA_BUFFER_HEADER_SIZE;

			if (rdb->ReparseDataLength+REPARSE_DATA_BUFFER_HEADER_SIZE<=static_cast<USHORT>(MAXIMUM_REPARSE_DATA_BUFFER_SIZE/sizeof(wchar_t)))
			{
				wmemcpy(&rdb->MountPointReparseBuffer.PathBuffer[rdb->MountPointReparseBuffer.SubstituteNameOffset/sizeof(wchar_t)],SubstituteName,SubstituteNameLength+1);
				wmemcpy(&rdb->MountPointReparseBuffer.PathBuffer[rdb->MountPointReparseBuffer.PrintNameOffset/sizeof(wchar_t)],PrintName,PrintNameLength+1);
				Result=true;
			}

			break;
		case IO_REPARSE_TAG_SYMLINK:
			rdb->SymbolicLinkReparseBuffer.PrintNameOffset=0;
			rdb->SymbolicLinkReparseBuffer.PrintNameLength=static_cast<WORD>(PrintNameLength*sizeof(wchar_t));
			rdb->SymbolicLinkReparseBuffer.SubstituteNameOffset=rdb->MountPointReparseBuffer.PrintNameLength;
			rdb->SymbolicLinkReparseBuffer.SubstituteNameLength=static_cast<WORD>(SubstituteNameLength*sizeof(wchar_t));
			rdb->ReparseDataLength=FIELD_OFFSET(REPARSE_DATA_BUFFER,SymbolicLinkReparseBuffer.PathBuffer)+rdb->SymbolicLinkReparseBuffer.SubstituteNameOffset+rdb->SymbolicLinkReparseBuffer.SubstituteNameLength-REPARSE_DATA_BUFFER_HEADER_SIZE;

			if (rdb->ReparseDataLength+REPARSE_DATA_BUFFER_HEADER_SIZE<=static_cast<USHORT>(MAXIMUM_REPARSE_DATA_BUFFER_SIZE/sizeof(wchar_t)))
			{
				wmemcpy(&rdb->SymbolicLinkReparseBuffer.PathBuffer[rdb->SymbolicLinkReparseBuffer.SubstituteNameOffset/sizeof(wchar_t)],SubstituteName,SubstituteNameLength);
				wmemcpy(&rdb->SymbolicLinkReparseBuffer.PathBuffer[rdb->SymbolicLinkReparseBuffer.PrintNameOffset/sizeof(wchar_t)],PrintName,PrintNameLength);
				Result=true;
			}

			break;
	}

	return Result;
}

bool SetREPARSE_DATA_BUFFER(const wchar_t *Object,PREPARSE_DATA_BUFFER rdb)
{
	bool Result=false;
	if (IsReparseTagValid(rdb->ReparseTag))
	{
		Privilege CreateSymlinkPrivilege(SE_CREATE_SYMBOLIC_LINK_NAME);
		File fObject;

		bool ForceElevation=false;

		DWORD Attributes = apiGetFileAttributes(Object);
		if(Attributes&FILE_ATTRIBUTE_READONLY)
		{
			apiSetFileAttributes(Object, Attributes&~FILE_ATTRIBUTE_READONLY);
		}
		for(size_t i=0;i<2;i++)
		{
			if (fObject.Open(Object,GENERIC_WRITE,0,nullptr,OPEN_EXISTING,FILE_FLAG_OPEN_REPARSE_POINT,nullptr,ForceElevation))
			{
				DWORD dwBytesReturned;
				if (fObject.IoControl(FSCTL_SET_REPARSE_POINT,rdb,rdb->ReparseDataLength+REPARSE_DATA_BUFFER_HEADER_SIZE,nullptr,0,&dwBytesReturned))
				{
					Result=true;
				}
				fObject.Close();
				// Open() success, but IoControl() fails. We can't handle this automatically :(
				if(!i && !Result && ElevationRequired(ELEVATION_MODIFY_REQUEST))
				{
					ForceElevation=true;
					continue;
				}
				break;
			}
		}
		if(Attributes&FILE_ATTRIBUTE_READONLY)
		{
			apiSetFileAttributes(Object, Attributes);
		}

	}

	return Result;
}

bool WINAPI CreateReparsePoint(const wchar_t *Target, const wchar_t *Object,ReparsePointTypes Type)
{
	bool Result=false;

	if (Object && *Object && Target && *Target)
	{
		switch (Type)
		{
			case RP_HARDLINK:
				break;
			case RP_EXACTCOPY:
				Result=DuplicateReparsePoint(Target,Object);
				break;
			case RP_SYMLINK:
			case RP_SYMLINKFILE:
			case RP_SYMLINKDIR:
				{
					DWORD ObjectAttributes = apiGetFileAttributes(Object);
					bool ObjectExist = ObjectAttributes!=INVALID_FILE_ATTRIBUTES;
					if(Type == RP_SYMLINK)
					{
						DWORD Attr = apiGetFileAttributes(Target);
						Type = ((Attr != INVALID_FILE_ATTRIBUTES) && (Attr&FILE_ATTRIBUTE_DIRECTORY)? RP_SYMLINKDIR : RP_SYMLINKFILE);
					}
					if (ifn.CreateSymbolicLinkPresent() && !ObjectExist)
					{
						Result=apiCreateSymbolicLink(Object,Target,Type==RP_SYMLINKDIR?SYMBOLIC_LINK_FLAG_DIRECTORY:0);
					}
					else
					{
						bool ObjectCreated=false;
						if (Type==RP_SYMLINKDIR)
						{
							ObjectCreated= (ObjectExist && (ObjectAttributes&FILE_ATTRIBUTE_DIRECTORY)) || apiCreateDirectory(Object,nullptr)!=FALSE;
						}
						else
						{
							if(ObjectExist)
							{
								ObjectCreated = !(ObjectAttributes&FILE_ATTRIBUTE_DIRECTORY);
							}
							else
							{
								File file;
								if(file.Open(Object,0,0,nullptr,CREATE_NEW))
								{
									ObjectCreated=true;
									file.Close();
								}
							}
						}

						if (ObjectCreated)
						{
							LPBYTE szBuff=new BYTE[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
							if(szBuff)
							{
								PREPARSE_DATA_BUFFER rdb=reinterpret_cast<PREPARSE_DATA_BUFFER>(szBuff);
								rdb->ReparseTag=IO_REPARSE_TAG_SYMLINK;
								string strPrintName=Target,strSubstituteName=Target;

								if (IsAbsolutePath(Target))
								{
									strSubstituteName=L"\\??\\";
									strSubstituteName+=(strPrintName.CPtr()+(HasPathPrefix(strPrintName)?4:0));
									rdb->SymbolicLinkReparseBuffer.Flags=0;
							}
								else
								{
									rdb->SymbolicLinkReparseBuffer.Flags=SYMLINK_FLAG_RELATIVE;
								}

								if (FillREPARSE_DATA_BUFFER(rdb,strPrintName,strPrintName.GetLength(),strSubstituteName,strSubstituteName.GetLength()))
								{
									Result=SetREPARSE_DATA_BUFFER(Object,rdb);
								}
								else
								{
									SetLastError(ERROR_INSUFFICIENT_BUFFER);
								}
								delete[] szBuff;
							}
						}
					}
				}
				break;
			case RP_JUNCTION:
			case RP_VOLMOUNT:
			{
				string strPrintName,strSubstituteName;
				ConvertNameToFull(Target,strPrintName);
				strSubstituteName=L"\\??\\";
				strSubstituteName+=(strPrintName.CPtr()+(HasPathPrefix(strPrintName)?4:0));
				LPBYTE szBuff = new BYTE[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
				PREPARSE_DATA_BUFFER rdb=reinterpret_cast<PREPARSE_DATA_BUFFER>(szBuff);
				rdb->ReparseTag=IO_REPARSE_TAG_MOUNT_POINT;

				if (FillREPARSE_DATA_BUFFER(rdb,strPrintName,strPrintName.GetLength(),strSubstituteName,strSubstituteName.GetLength()))
				{
					Result=SetREPARSE_DATA_BUFFER(Object,rdb);
				}
				else
				{
					SetLastError(ERROR_INSUFFICIENT_BUFFER);
				}
				delete[] szBuff;
			}
			break;
		}
	}

	return Result;
}

bool WINAPI DeleteReparsePoint(const wchar_t *Object)
{
	bool Result=false;
	DWORD ReparseTag;
	string strTmp;
	GetReparsePointInfo(Object,strTmp,&ReparseTag);
	File fObject;
	if (fObject.Open(Object, GENERIC_READ|GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT))
	{
		REPARSE_GUID_DATA_BUFFER rgdb={ReparseTag};
		DWORD dwBytes;
		Result=fObject.IoControl(FSCTL_DELETE_REPARSE_POINT,&rgdb,REPARSE_GUID_DATA_BUFFER_HEADER_SIZE,nullptr,0,&dwBytes);
		fObject.Close();
	}

	return Result;
}

bool GetREPARSE_DATA_BUFFER(const wchar_t *Object,PREPARSE_DATA_BUFFER rdb)
{
	bool Result=false;
	const DWORD FileAttr = apiGetFileAttributes(Object);

	if (FileAttr!=INVALID_FILE_ATTRIBUTES && (FileAttr&FILE_ATTRIBUTE_REPARSE_POINT))
	{
		File fObject;
		if(fObject.Open(Object,0,0,nullptr,OPEN_EXISTING,FILE_FLAG_OPEN_REPARSE_POINT))
		{
			DWORD dwBytesReturned;
			if(fObject.IoControl(FSCTL_GET_REPARSE_POINT, nullptr, 0, rdb, MAXIMUM_REPARSE_DATA_BUFFER_SIZE, &dwBytesReturned) && IsReparseTagValid(rdb->ReparseTag))
			{
				Result=true;
			}
			fObject.Close();
		}
	}

	return Result;
}

DWORD WINAPI GetReparsePointInfo(const wchar_t *Object, string &strDestBuff,LPDWORD lpReparseTag)
{
	WORD NameLength=0;
	LPBYTE szBuff=new BYTE[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
	if(szBuff)
	{
		PREPARSE_DATA_BUFFER rdb = reinterpret_cast<PREPARSE_DATA_BUFFER>(szBuff);

		if (GetREPARSE_DATA_BUFFER(Object,rdb))
		{
			const wchar_t *PathBuffer;

			if (lpReparseTag)
				*lpReparseTag=rdb->ReparseTag;

			if (rdb->ReparseTag == IO_REPARSE_TAG_SYMLINK)
			{
				NameLength = rdb->SymbolicLinkReparseBuffer.PrintNameLength/sizeof(wchar_t);

				if (NameLength)
				{
					PathBuffer = &rdb->SymbolicLinkReparseBuffer.PathBuffer[rdb->SymbolicLinkReparseBuffer.PrintNameOffset/sizeof(wchar_t)];
				}
				else
				{
					NameLength = rdb->SymbolicLinkReparseBuffer.SubstituteNameLength/sizeof(wchar_t);
					PathBuffer = &rdb->SymbolicLinkReparseBuffer.PathBuffer[rdb->SymbolicLinkReparseBuffer.SubstituteNameOffset/sizeof(wchar_t)];
				}
			}
			else
			{
				NameLength = rdb->MountPointReparseBuffer.PrintNameLength/sizeof(wchar_t);

				if (NameLength)
				{
					PathBuffer = &rdb->MountPointReparseBuffer.PathBuffer[rdb->MountPointReparseBuffer.PrintNameOffset/sizeof(wchar_t)];
				}
				else
				{
					NameLength = rdb->MountPointReparseBuffer.SubstituteNameLength/sizeof(wchar_t);
					PathBuffer = &rdb->MountPointReparseBuffer.PathBuffer[rdb->MountPointReparseBuffer.SubstituteNameOffset/sizeof(wchar_t)];
				}
			}

			wchar_t *lpwszDestBuff=strDestBuff.GetBuffer(NameLength+1);
			wcsncpy(lpwszDestBuff,PathBuffer,NameLength);
			strDestBuff.ReleaseBuffer(NameLength);
		}
		delete[] szBuff;
	}
	return NameLength;
}

int WINAPI GetNumberOfLinks(const wchar_t *Name)
{
	int NumberOfLinks=1;
	File file;
	if(file.Open(Name, 0, FILE_SHARE_READ|FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT))
	{
		BY_HANDLE_FILE_INFORMATION bhfi;
		if (file.GetInformation(bhfi))
		{
			NumberOfLinks=bhfi.nNumberOfLinks;
		}
		file.Close();
	}
	return NumberOfLinks;
}


int WINAPI MkHardLink(const wchar_t *ExistingName,const wchar_t *NewName)
{
	return apiCreateHardLink(NewName,ExistingName,nullptr)!=FALSE;
}

bool EnumStreams(const wchar_t *FileName,UINT64 &StreamsSize,DWORD &StreamsCount)
{
	bool Result=false;
	WIN32_FIND_STREAM_DATA fsd;
	HANDLE hFind=apiFindFirstStream(FileName,FindStreamInfoStandard,&fsd);

	if (hFind!=INVALID_HANDLE_VALUE)
	{
		StreamsCount=1;
		StreamsSize=fsd.StreamSize.QuadPart;

		while (apiFindNextStream(hFind,&fsd))
		{
			StreamsCount++;
			StreamsSize+=fsd.StreamSize.QuadPart;
		}

		apiFindStreamClose(hFind);
		Result=true;
	}

	return Result;
}

bool DelSubstDrive(const wchar_t *DeviceName)
{
	bool Result=false;
	string strTargetPath;

	if (GetSubstName(DRIVE_NOT_INIT,DeviceName,strTargetPath))
	{
		strTargetPath=(string)L"\\??\\"+strTargetPath;
		Result=(DefineDosDevice(DDD_RAW_TARGET_PATH|DDD_REMOVE_DEFINITION|DDD_EXACT_MATCH_ON_REMOVE,DeviceName,strTargetPath)==TRUE);
	}

	return Result;
}

bool GetSubstName(int DriveType,const wchar_t *DeviceName, string &strTargetPath)
{
	bool Ret=false;
	/*
	+ ��������� � ����������� �� Opt.SubstNameRule
	������� �����:
	0 - ���� ����������, �� ���������� ������� �����
	1 - ���� ����������, �� ���������� ��� ���������
	*/
	bool DriveRemovable = (DriveType==DRIVE_REMOVABLE || DriveType==DRIVE_CDROM);

	if (DriveType==DRIVE_NOT_INIT || (((Opt.SubstNameRule & 1) || !DriveRemovable) && ((Opt.SubstNameRule & 2) || DriveRemovable)))
	{
		if (IsLocalPath(DeviceName))
		{
			string Name;
			if (apiQueryDosDevice(DeviceName, Name))
			{
				if (Name.IsSubStrAt(0, L"\\??\\"))
				{
					strTargetPath=Name.SubStr(4);
					Ret=true;
				}
			}
		}
	}

	return Ret;
}

bool GetVHDName(const wchar_t *DeviceName, string &strVolumePath)
{
	bool Result=false;
	File Device;
	if(Device.Open(DeviceName, FILE_READ_ATTRIBUTES,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, nullptr, OPEN_EXISTING))
	{
		ULONG Size = 1024;
		PSTORAGE_DEPENDENCY_INFO StorageDependencyInfo = static_cast<PSTORAGE_DEPENDENCY_INFO>(xf_malloc(Size));
		if(StorageDependencyInfo)
		{
			StorageDependencyInfo->Version = STORAGE_DEPENDENCY_INFO_VERSION_2;
			DWORD Used = 0;
			Result = Device.GetStorageDependencyInformation(GET_STORAGE_DEPENDENCY_FLAG_HOST_VOLUMES, Size, StorageDependencyInfo, &Used);
			if(!Result && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
				StorageDependencyInfo = static_cast<PSTORAGE_DEPENDENCY_INFO>(xf_realloc(StorageDependencyInfo, Used));
				if(StorageDependencyInfo)
				{
					Result = Device.GetStorageDependencyInformation(GET_STORAGE_DEPENDENCY_FLAG_HOST_VOLUMES, Used, StorageDependencyInfo, &Used);
				}
			}
			if(Result)
			{
				if(StorageDependencyInfo->NumberEntries)
				{
					strVolumePath = StorageDependencyInfo->Version2Entries[0].HostVolumeName;
					strVolumePath += StorageDependencyInfo->Version2Entries[0].DependentVolumeRelativePath;
				}
			}
		}
	}
	return Result;
}


void GetPathRoot(const wchar_t *Path, string &strRoot)
{
	string RealPath;
	ConvertNameToReal(Path, RealPath);
	strRoot = ExtractPathRoot(RealPath);
}

bool ModifyReparsePoint(const wchar_t *Object,const wchar_t *NewData)
{
	bool Result=false;
	LPBYTE szBuff=new BYTE[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
	if(szBuff)
	{
		PREPARSE_DATA_BUFFER rdb=reinterpret_cast<PREPARSE_DATA_BUFFER>(szBuff);

		if (GetREPARSE_DATA_BUFFER(Object,rdb))
		{
			bool FillResult=false;

			switch (rdb->ReparseTag)
			{
				case IO_REPARSE_TAG_MOUNT_POINT:
				{
					string strPrintName,strSubstituteName;
					ConvertNameToFull(NewData,strPrintName);
					strSubstituteName=L"\\??\\";
					strSubstituteName+=(strPrintName.CPtr()+(HasPathPrefix(strPrintName)?4:0));
					FillResult=FillREPARSE_DATA_BUFFER(rdb,strPrintName,strPrintName.GetLength(),strSubstituteName,strSubstituteName.GetLength());
				}
				break;
				case IO_REPARSE_TAG_SYMLINK:
				{
					string strPrintName=NewData,strSubstituteName=NewData;

					if (IsAbsolutePath(NewData))
					{
						strSubstituteName=L"\\??\\";
						strSubstituteName+=(strPrintName.CPtr()+(HasPathPrefix(strPrintName)?4:0));
						rdb->SymbolicLinkReparseBuffer.Flags=0;
					}
					else
					{
						rdb->SymbolicLinkReparseBuffer.Flags=SYMLINK_FLAG_RELATIVE;
					}

					FillResult=FillREPARSE_DATA_BUFFER(rdb,strPrintName,strPrintName.GetLength(),strSubstituteName,strSubstituteName.GetLength());
				}
				break;
			}

			if (FillResult)
			{
				Result=SetREPARSE_DATA_BUFFER(Object,rdb);
			}
			else
			{
				SetLastError(ERROR_INSUFFICIENT_BUFFER);
			}
		}
		delete[] szBuff;
	}
	return Result;
}

bool DuplicateReparsePoint(const wchar_t *Src,const wchar_t *Dst)
{
	bool Result=false;
	LPBYTE szBuff=new BYTE[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
	if(szBuff)
	{
		PREPARSE_DATA_BUFFER rdb=reinterpret_cast<PREPARSE_DATA_BUFFER>(szBuff);
		if (GetREPARSE_DATA_BUFFER(Src,rdb) && SetREPARSE_DATA_BUFFER(Dst,rdb))
		{
			Result=true;
		}
		delete[] szBuff;
	}
	return Result;
}

BOOL WINAPI FarMkLink(const wchar_t *Src,const wchar_t *Dest, LINK_TYPE Type, MKLINK_FLAGS Flags)
{
	int Result=0;

	if (Src && *Src && Dest && *Dest)
	{
		switch (Type)
		{
			case LINK_HARDLINK:
				Result=MkHardLink(Src,Dest);
				break;
			case LINK_JUNCTION:
			case LINK_VOLMOUNT:
			case LINK_SYMLINKFILE:
			case LINK_SYMLINKDIR:
				{
					ReparsePointTypes LinkType=RP_JUNCTION;

					switch (Type)
					{
						case LINK_VOLMOUNT:
							LinkType=RP_VOLMOUNT;
							break;
						case LINK_SYMLINK:
							LinkType=RP_SYMLINK;
							break;
						case LINK_SYMLINKFILE:
							LinkType=RP_SYMLINKFILE;
							break;
						case LINK_SYMLINKDIR:
							LinkType=RP_SYMLINKDIR;
							break;
						default:
							break;
					}

					Result=MkSymLink(Src,Dest,LinkType,(Flags&MLF_SHOWERRMSG?0:FCOPY_NOSHOWMSGLINK));
				}
				break;
			default:
				break;
		}
	}

	if (Result && !(Flags&MLF_DONOTUPDATEPANEL))
		ShellUpdatePanels(nullptr,FALSE);

	return Result;
}

void NormalizeSymlinkName(string &strLinkName)
{
	if (!StrCmpN(strLinkName,L"\\??\\",4))
	{
		if (IsNetworkPath(strLinkName) || IsLocalVolumePath(strLinkName))
		{
			LPWSTR LinkName=strLinkName.GetBuffer();
			LinkName[1]=L'\\';
			strLinkName.ReleaseBuffer();
		}
		else
		{
			strLinkName.LShift(4);
		}
	}
}

// ����� ��� �������� SymLink ��� ���������.
int MkSymLink(const wchar_t *SelName,const wchar_t *Dest,ReparsePointTypes LinkType,DWORD Flags)
{
	if (SelName && *SelName && Dest && *Dest)
	{
		string strSrcFullName, strDestFullName, strSelOnlyName;
		string strMsgBuf, strMsgBuf2;
		// ������� ���
		strSelOnlyName = SelName;
		DeleteEndSlash(strSelOnlyName);
		const wchar_t *PtrSelName=LastSlash(strSelOnlyName);

		if (!PtrSelName)
			PtrSelName=strSelOnlyName;
		else
			++PtrSelName;

		if (SelName[1] == L':' && (!SelName[2] || (IsSlash(SelName[2]) && !SelName[3]))) // C: ��� C:/
		{
//      if(Flags&FCOPY_VOLMOUNT)
			{
				strSrcFullName = SelName;
				AddEndSlash(strSrcFullName);
			}
			/*
			  ��� ����� - �� ����� ����� ���������!
			  �.�. ���� � �������� SelName �������� "C:", �� � ���� ����� ����������
			  ��������� ���� ����� - � symlink`� �� volmount
			*/
			LinkType=RP_VOLMOUNT;
		}
		else
			ConvertNameToFull(SelName,strSrcFullName);

		ConvertNameToFull(Dest,strDestFullName);

		if (IsSlash(strDestFullName.At(strDestFullName.GetLength()-1)))
		{
			if (LinkType!=RP_VOLMOUNT)
				strDestFullName += PtrSelName;
			else
			{
				const wchar_t Tmp[]={L'D',L'i',L's',L'k',L'_',*SelName,L'\0'};
				strDestFullName+=Tmp;
			}
		}

		if (LinkType==RP_VOLMOUNT)
		{
			AddEndSlash(strSrcFullName);
			AddEndSlash(strDestFullName);
		}

		DWORD JSAttr=apiGetFileAttributes(strDestFullName);

		if (JSAttr != INVALID_FILE_ATTRIBUTES) // ���������� �����?
		{
			if ((JSAttr&FILE_ATTRIBUTE_DIRECTORY)!=FILE_ATTRIBUTE_DIRECTORY)
			{
				if (!(Flags&FCOPY_NOSHOWMSGLINK))
				{
					Message(MSG_WARNING,1,MSG(MError),
					        MSG(MCopyCannotCreateJunctionToFile),
					        strDestFullName,MSG(MOk));
				}

				return 0;
			}

			if (TestFolder(strDestFullName) == TSTFLD_NOTEMPTY) // � ������?
			{
				// �� ������, �� ��� ��, ����� ������� ������� dest\srcname
				AddEndSlash(strDestFullName);

				if (LinkType==RP_VOLMOUNT)
				{
					string strTmpName;
					strTmpName.Format(MSG(MCopyMountName),*SelName);
					strDestFullName += strTmpName;
					AddEndSlash(strDestFullName);
				}
				else
					strDestFullName += PtrSelName;

				JSAttr=apiGetFileAttributes(strDestFullName);

				if (JSAttr != INVALID_FILE_ATTRIBUTES) // � ����� ���� ����???
				{
					if (TestFolder(strDestFullName) == TSTFLD_NOTEMPTY) // � ������?
					{
						if (!(Flags&FCOPY_NOSHOWMSGLINK))
						{
							if (LinkType==RP_VOLMOUNT)
							{
								strMsgBuf.Format(MSG(MCopyMountVolFailed), SelName);
								strMsgBuf2.Format(MSG(MCopyMountVolFailed2), strDestFullName.CPtr());
								Message(MSG_WARNING,1,MSG(MError),
								        strMsgBuf,
								        strMsgBuf2,
								        MSG(MCopyFolderNotEmpty),
								        MSG(MOk));
							}
							else
								Message(MSG_WARNING,1,MSG(MError),
								        MSG(MCopyCannotCreateLink),strDestFullName,
								        MSG(MCopyFolderNotEmpty),MSG(MOk));
						}

						return 0; // ���������� � ����
					}
				}
				else // �������.
				{
					if (apiCreateDirectory(strDestFullName,nullptr))
						TreeList::AddTreeName(strDestFullName);
					else
						CreatePath(strDestFullName);
				}

				if (apiGetFileAttributes(strDestFullName) == INVALID_FILE_ATTRIBUTES) // ���, ��� ����� ���� �����.
				{
					if (!(Flags&FCOPY_NOSHOWMSGLINK))
					{
						Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),
						        MSG(MCopyCannotCreateFolder),
						        strDestFullName,MSG(MOk));
					}

					return 0;
				}
			}
		}
		else
		{
			if (LinkType==RP_SYMLINK || LinkType==RP_SYMLINKFILE || LinkType==RP_SYMLINKDIR)
			{
				// � ���� ������ ��������� ����, �� �� ��� �������
				string strPath=strDestFullName;

				if (CutToSlash(strPath))
				{
					if (apiGetFileAttributes(strPath)==INVALID_FILE_ATTRIBUTES)
						CreatePath(strPath);
				}
			}
			else
			{
				bool CreateDir=true;

				if (LinkType==RP_EXACTCOPY)
				{
					// � ���� ������ ��������� ��� �������, ��� ������ ����
					DWORD dwSrcAttr=apiGetFileAttributes(strSrcFullName);

					if (dwSrcAttr!=INVALID_FILE_ATTRIBUTES && !(dwSrcAttr&FILE_ATTRIBUTE_DIRECTORY))
						CreateDir=false;
				}

				if (CreateDir)
				{
					if (apiCreateDirectory(strDestFullName,nullptr))
						TreeList::AddTreeName(strDestFullName);
					else
						CreatePath(strDestFullName);
				}
				else
				{
					string strPath=strDestFullName;

					if (CutToSlash(strPath))
					{
						// ������
						if (apiGetFileAttributes(strPath)==INVALID_FILE_ATTRIBUTES)
							CreatePath(strPath);

						File file;
						if(file.Open(strDestFullName, 0, 0, 0, CREATE_NEW, apiGetFileAttributes(strSrcFullName)))
						{
							file.Close();
						}
					}
				}

				if (apiGetFileAttributes(strDestFullName) == INVALID_FILE_ATTRIBUTES) // ���. ��� ����� ���� �����.
				{
					if (!(Flags&FCOPY_NOSHOWMSGLINK))
					{
						Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),
						        MSG(MCopyCannotCreateLink),strDestFullName,MSG(MOk));
					}

					return 0;
				}
			}
		}

		if (LinkType!=RP_VOLMOUNT)
		{
			if (CreateReparsePoint(strSrcFullName,strDestFullName,LinkType))
			{
				return 1;
			}
			else
			{
				if (!(Flags&FCOPY_NOSHOWMSGLINK))
				{
					Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),
					        MSG(MCopyCannotCreateLink),strDestFullName,MSG(MOk));
				}

				return 0;
			}
		}
		else
		{
			if (CreateVolumeMountPoint(strSrcFullName,strDestFullName))
			{
				return 1;
			}
			else
			{
				if (!(Flags&FCOPY_NOSHOWMSGLINK))
				{
					strMsgBuf.Format(MSG(MCopyMountVolFailed),SelName);
					strMsgBuf2.Format(MSG(MCopyMountVolFailed2),strDestFullName.CPtr());
					Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),strMsgBuf,strMsgBuf2,MSG(MOk));
				}

				return 0;
			}
		}
	}

	return 2;
}
