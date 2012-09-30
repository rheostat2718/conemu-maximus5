#pragma once

/*
exception.cpp

��� ��� ����������
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

#include "plugins.hpp"

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                  \
	((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |   \
	 ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))
#endif

#define FLOG_SYSINFO     0x00000001 // ���������� � �������
#define FLOG_EXCEPTION   0x00000002 // ��� ����������
#define FLOG_PLUGIN      0x00000004 // ���������� � �������
#define FLOG_FARAREA     0x00000008 // "��� �� ������ ���������?"
#define FLOG_MACRO       0x00000010 // �������
#define FLOG_RAWDARA     0x00000020 // ������������ ������
#define FLOG_PLUGINSINFO 0x80000000 // ���������� � ��������
#define FLOG_ALL         0xFFFFFFFF

enum FARRECORDTYPE
{
	RTYPE_SYSINFO      =MAKEFOURCC('S','Y','S','T'),// ���������� � �������
	RTYPE_EXCEPTION    =MAKEFOURCC('E','X','C','T'),// ��� ����������
	RTYPE_PLUGIN       =MAKEFOURCC('C','P','L','G'),// ���������� � ������� �������
	RTYPE_FARAREA      =MAKEFOURCC('A','R','E','A'),// "��� �� ������ ���������?"
	RTYPE_MACRO        =MAKEFOURCC('M','A','C','R'),// �������
	RTYPE_RAWDARA      =MAKEFOURCC('R','A','W','D'),// ������������ ������
};

struct RECHEADER          // ��������� �������
{
	DWORD TypeRec;          // ��� ������
	DWORD SizeRec;          // ������ ���������
	RECHEADER *Next; // ��������� ������� � ������
	// Data                 // ������ �������� SizeRec
};

struct SYSINFOHEADER      // ���������� � �������
{
	DWORD TypeRec;          // ��� ������ = RTYPE_SYSINFO
	DWORD SizeRec;          // ������ ������ = sizeof(DUMPHEADER)-sizeof(WORD)*2
	RECHEADER *Next; // ��������� ������� � ������
	DWORD DumpFlags;        // �������������� ����� (���� =0)
	VersionInfo FARVersion;       // ������ FAR Manager � ������� FAR_VERSION
	SYSTEMTIME DumpTime;    // the system time is expressed in Coordinated Universal Time (UTC))
	OSVERSIONINFO WinVer;   // ������ ������
};

// ����� ��� ���� PluginItem.WorkFlags
enum EX_PLUGINITEMWORKFLAGS
{
	EXPIWF_CACHED        = 0x00000001, // ����������
	EXPIWF_PRELOADED     = 0x00000002, //
	EXPIWF_DONTLOADAGAIN = 0x00000004, // �� ��������� ������ �����, �������� �
	//   ���������� �������� ��������� ������ ����
	EXPIWF_DATALOADED    = 0x00000008, // LoadData ������� �����������
};


struct PLUGINRECORD       // ���������� � �������
{
	DWORD TypeRec;          // ��� ������ = RTYPE_PLUGIN
	DWORD SizeRec;          // ������
	DWORD Reserved1;

	DWORD WorkFlags;        // ������� ����� �������� �������
	DWORD FuncFlags;        // ������� ����� ����.������� ������� (��� ���� - ��� � �������)
	DWORD CallFlags;        // ������� ����� ������ ����.������� �������

	// DWORD SysID; GUID

	const wchar_t *ModuleName;

	DWORD Reserved2[2];    // ������ :-)

	DWORD SizeModuleName;
};

struct EXCEPTIONRECORD    // ��� ����������
{
	DWORD TypeRec;          // ��� ������ = RTYPE_EXCEPTION
	DWORD SizeRec;          // ������ ������
	RECHEADER *Next; // ��������� ������� � ������

	EXCEPTION_POINTERS *Exception;
};

struct MACRORECORD        // �������
{
	DWORD TypeRec;          // ��� ������ = RTYPE_MACRO
	DWORD SizeRec;          // ������
	RECHEADER *Next; // ��������� ������� � ������
	WORD  MacroStatus;      // 0 - �� � ������ �����, 1 - Recording, 2 - Executing
	WORD  MacroPos;         // ������� ������� � MacroKeyBuffer
	DWORD MacroFlags;       // ����� - ������� ����� = MACRO_AREA
	DWORD MacroKey;         // ����������� ������������
	DWORD MacroBufferSize;  // ������ ����������������������� MacroKeyBuffer
	// DWORD MacroKeyBuffer[0];// �����-������������������
};

struct FARAREARECORD      // "��� �� ������ ���������?"
{
	DWORD TypeRec;          // ��� ������ = RTYPE_FARAREA
	DWORD SizeRec;          // ������ ������
	RECHEADER *Next; // ��������� ������� � ������
	DWORD ObjectType;       // ��, ��� ���������� CtrlObject->Cp()->GetType()
	COORD ScrWH;            // ������� ������ - ������, ������
};

enum
{
	RAWTYPE_BINARY =0,
	RAWTYPE_STRING   =1,
};

struct RAWDARARECORD      // ������������ ������
{
	DWORD TypeRec;          // ��� ������ = RTYPE_RAWDARA
	DWORD SizeRec;          // ������ ������
	RECHEADER *Next; // ��������� ������� � ������
	DWORD RawFlags;         // �������������� ����� ��� ���������������� :-)
	DWORD RawType;          // ��� ������ = RAWTYPE_BINARY, RAWTYPE_STRING
	DWORD SizeData;         // ������ ������������ ������
	//BYTE Data[0];         // ���� SizeRec=0, �� ����� ���� ����
};


//�������� ��������� ��� �������� ������� � ������
//����� ��������� ������
struct COMBINE_RECORD
{
	RECHEADER Header;
	char      Data[ 1 /*SizeRec*/ ];
};
//�������� ��������� ��� ������� � ����������� ������
//����� ������
struct MACRORECORD_t : public MACRORECORD
{
	DWORD MacroKeyBuffer[ 1 /*MacroBufferSize*/ ];
};
struct PLUGINRECORD_t : public PLUGINRECORD
{
	char ModuleName[ 1 /*SizeModuleName*/ ];
};
struct RAWDARARECORD_t : public RAWDARARECORD
{
	LPBYTE RawDataPtr;  //The pointer to allocated raw data can be placed here
};

struct FARExceptionState
{
	DWORD               StructSize;
	DWORD               Version;       // ������ "��������"

	//FAR additional error info
	char RecomendedDumpFileName[_MAX_PATH];
	const char         *RootKey;

	//FAR error context
	RECHEADER   *Head;
};

/* $ 17.10.2000 SVS
   ����������!
*/
enum ExceptFunctionsType
{
	EXCEPT_KERNEL=-1,
	EXCEPT_GETGLOBALINFO,
	EXCEPT_SETSTARTUPINFO,
	EXCEPT_GETVIRTUALFINDDATA,
	EXCEPT_OPEN,
	EXCEPT_OPENFILEPLUGIN,
	EXCEPT_CLOSEPANEL,
	EXCEPT_GETPLUGININFO,
	EXCEPT_GETOPENPANELINFO,
	EXCEPT_GETFINDDATA,
	EXCEPT_FREEFINDDATA,
	EXCEPT_FREEVIRTUALFINDDATA,
	EXCEPT_SETDIRECTORY,
	EXCEPT_GETFILES,
	EXCEPT_PUTFILES,
	EXCEPT_DELETEFILES,
	EXCEPT_MAKEDIRECTORY,
	EXCEPT_PROCESSHOSTFILE,
	EXCEPT_SETFINDLIST,
	EXCEPT_CONFIGURE,
	EXCEPT_EXITFAR,
	EXCEPT_PROCESSPANELINPUT,
	EXCEPT_PROCESSPANELEVENT,
	EXCEPT_PROCESSEDITOREVENT,
	EXCEPT_COMPARE,
	EXCEPT_PROCESSEDITORINPUT,
	EXCEPT_MINFARVERSION,
	EXCEPT_PROCESSVIEWEREVENT,
	EXCEPT_PROCESSVIEWERINPUT,
	EXCEPT_PROCESSDIALOGEVENT,
	EXCEPT_PROCESSSYNCHROEVENT,
	EXCEPT_ANALYSE,
	EXCEPT_GETCUSTOMDATA,
	EXCEPT_FREECUSTOMDATA,
	EXCEPT_CLOSEANALYSE,
#if defined(MANTIS_0000466)
	EXCEPT_PROCESSMACRO,
#endif
	EXCEPT_PROCESSCONSOLEINPUT,
};

typedef BOOL (WINAPI *FARPROCESSEVENT)(FARExceptionState * Context);

int WriteEvent(DWORD DumpType, // FLOG_*
               EXCEPTION_POINTERS *xp=nullptr,
               Plugin *Module=nullptr,
               void *RawData=nullptr,DWORD RawDataSize=0,
               DWORD RawDataFlags=0,DWORD RawType=RAWTYPE_BINARY);

DWORD WINAPI xfilter(
    int From,                 // ������: 0 = OpenPlugin, 1 = OpenFilePlugin
    EXCEPTION_POINTERS *xp,   // ������ ��������
    Plugin *Module,// ������, ��������� � ����������.
    DWORD Flags);             // �������������� ����� - ���� ������ ����
//        0x1 - ���������� ��� ��������?
