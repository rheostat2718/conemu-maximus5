#pragma once

/*
copy.hpp

class ShellCopy - ����������� ������
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

#include "dizlist.hpp"
#include "udlist.hpp"
#include "flink.hpp"

class Panel;

enum COPY_CODES
{
	COPY_CANCEL,
	COPY_NEXT,
	COPY_NOFILTER,                              // �� ������� �������, �.�. ���� �� ������ �� �������
	COPY_FAILURE,
	COPY_FAILUREREAD,
	COPY_SUCCESS,
	COPY_SUCCESS_MOVE,
	COPY_RETRY,
};

enum COPY_FLAGS
{
	FCOPY_COPYTONUL               = 0x00000001, // ������� ����������� � NUL
	FCOPY_CURRENTONLY             = 0x00000002, // ������ ������?
	FCOPY_ONLYNEWERFILES          = 0x00000004, // Copy only newer files
	FCOPY_OVERWRITENEXT           = 0x00000008, // Overwrite all
	FCOPY_LINK                    = 0x00000010, // �������� ������
	FCOPY_MOVE                    = 0x00000040, // �������/��������������
	FCOPY_DIZREAD                 = 0x00000080, //
	FCOPY_COPYSECURITY            = 0x00000100, // [x] Copy access rights
	FCOPY_NOSHOWMSGLINK           = 0x00000200, // �� ���������� ������ ��� ���������
	FCOPY_VOLMOUNT                = 0x00000400, // �������� ������������� ����
	FCOPY_STREAMSKIP              = 0x00000800, // ������
	FCOPY_STREAMALL               = 0x00001000, // ������
	FCOPY_SKIPSETATTRFLD          = 0x00002000, // ������ �� �������� ������� �������� ��� ��������� - ����� ������ Skip All
	FCOPY_COPYSYMLINKCONTENTS     = 0x00004000, // ���������� ���������� ������������ ������?
	FCOPY_COPYPARENTSECURITY      = 0x00008000, // ����������� ������������ �����, � ������ ���� �� �� �������� ����� �������
	FCOPY_LEAVESECURITY           = 0x00010000, // Move: [?] ������ �� ������ � ������� �������
	FCOPY_DECRYPTED_DESTINATION   = 0x00020000, // ��� ������������ ������ - ��������������...
	FCOPY_USESYSTEMCOPY           = 0x00040000, // ������������ ��������� ������� �����������
	FCOPY_COPYLASTTIME            = 0x10000000, // ��� ����������� � ��������� ��������� ��������������� ��� ����������.
	FCOPY_UPDATEPPANEL            = 0x80000000, // ���������� �������� ��������� ������
};

class ShellCopy
{
		DWORD Flags;
		Panel *SrcPanel,*DestPanel;
		int SrcPanelMode,DestPanelMode;
		int SrcDriveType,DestDriveType;
		string strSrcDriveRoot;
		string strDestDriveRoot;
		string strDestFSName;
		DizList DestDiz;
		string strDestDizPath;
		char *CopyBuffer;
		size_t CopyBufferSize;
		string strCopiedName;
		string strRenamedName;
		string strRenamedFilesPath;
		int OvrMode;
		int ReadOnlyOvrMode;
		int ReadOnlyDelMode;
		int SkipMode;          // ...��� �������� ��� ����������� ���������� ������.
		int SkipEncMode;
		int SkipDeleteMode;
		int SelectedFolderNameLength;
		UserDefinedList DestList;
		// ��� ������������ ������������.
		// ��� AltF6 ����� ��, ��� ������ ���� � �������,
		// � ��������� ������� - RP_EXACTCOPY - ��� � ���������
		ReparsePointTypes RPT;

		COPY_CODES CopyFileTree(const string&  Dest);
		COPY_CODES ShellCopyOneFile(const string&  Src,
		                            const FAR_FIND_DATA_EX &SrcData,
		                            string &strDest,
		                            int KeepPathPos, int Rename);
		COPY_CODES CheckStreams(const string& Src,const string& DestPath);
		int  ShellCopyFile(const string& SrcName,const FAR_FIND_DATA_EX &SrcData,
		                   string &strDestName,DWORD &DestAttr,int Append);
		int  ShellSystemCopy(const string& SrcName,const string& DestName,const FAR_FIND_DATA_EX &SrcData);
		int  DeleteAfterMove(const string& Name,DWORD Attr);
		void SetDestDizPath(const string& DestPath);
		int  AskOverwrite(const FAR_FIND_DATA_EX &SrcData,const string& SrcName,const string& DestName,
		                  DWORD DestAttr,int SameName,int Rename,int AskAppend,
		                  int &Append,string &strNewName,int &RetCode);
		int  GetSecurity(const string& FileName, FAR_SECURITY_DESCRIPTOR_EX& sd);
		int  SetSecurity(const string& FileName,const FAR_SECURITY_DESCRIPTOR_EX& sd);
		int  SetRecursiveSecurity(const string& FileName,const FAR_SECURITY_DESCRIPTOR_EX& sd);
		bool CalcTotalSize();
		bool ShellSetAttr(const string& Dest,DWORD Attr);
		void CheckUpdatePanel(); // ���������� ���� FCOPY_UPDATEPPANEL
	public:
		ShellCopy(Panel *SrcPanel,int Move,int Link,int CurrentOnly,int Ask,
		          int &ToPlugin, const wchar_t* PluginDestPath, bool ToSubdir=false);
		~ShellCopy();
};
