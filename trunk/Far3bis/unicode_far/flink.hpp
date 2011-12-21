#pragma once

/*
flink.hpp

������������ ���� ��� ������ � Hard & SymLink
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

#include "plugin.hpp"

enum ReparsePointTypes
{
	RP_EXACTCOPY,   // ��� �����������/�������� ������, ����� �������������
	RP_HARDLINK,    // ������ ������
	RP_JUNCTION,    // �����
	RP_VOLMOUNT,    // ������������� ���
	RP_SYMLINK, // ������, NT>=6
	RP_SYMLINKFILE, // ����-������, NT>=6
	RP_SYMLINKDIR,  // �������-������, NT>=6
};

int   WINAPI MkHardLink(const wchar_t *ExistingName,const wchar_t *NewName);
BOOL  WINAPI FarMkLink(const wchar_t *Src,const wchar_t *Dest, LINK_TYPE Type, MKLINK_FLAGS Flags);

int   WINAPI GetNumberOfLinks(const string& Name);
bool WINAPI CreateVolumeMountPoint(const string& TargetVolume, const string& Object);

bool  WINAPI CreateReparsePoint(const string& Target, const string& Object,ReparsePointTypes Type=RP_JUNCTION);
bool  WINAPI DeleteReparsePoint(const string& Object);
bool ModifyReparsePoint(const string& Object,const string& NewData);

DWORD WINAPI GetReparsePointInfo(const string& Object, string &szDestBuff,LPDWORD lpReparseTag=nullptr);

bool GetSubstName(int DriveType,const string& DeviceName,string &strTargetPath);
bool GetVHDName(const string& DeviceName, string &strVolumePath);

bool DelSubstDrive(const string& DeviceName);
void GetPathRoot(const wchar_t *Path, string &strRoot);

// ������������� ��� EnumNTFSStreams
// � ��������� sid ���� cStreamName �� ���������, �.�. ������� ��� ������
//    ���������� � ��������� StreamName
//typedef BOOL (WINAPI *ENUMFILESTREAMS)(int Idx,const WCHAR *StreamName,const WIN32_STREAM_ID *sid);
//int WINAPI EnumNTFSStreams(const char *FileName,ENUMFILESTREAMS fpEnum,__int64 *SizeStreams);

bool EnumStreams(const string& FileName,UINT64 &StreamsSize,DWORD &StreamsCount);

bool DuplicateReparsePoint(const string& Src,const string& Dst);

void NormalizeSymlinkName(string &strLinkName);

int MkSymLink(const wchar_t *SelName,const wchar_t *Dest,ReparsePointTypes LinkType,DWORD Flags);
