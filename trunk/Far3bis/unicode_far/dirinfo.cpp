/*
dirinfo.cpp

GetDirInfo & GetPluginDirInfo
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

#include "dirinfo.hpp"
#include "plugapi.hpp"
#include "keys.hpp"
#include "scantree.hpp"
#include "savescr.hpp"
#include "lang.hpp"
#include "RefreshFrameManager.hpp"
#include "TPreRedrawFunc.hpp"
#include "ctrlobj.hpp"
#include "filefilter.hpp"
#include "interf.hpp"
#include "message.hpp"
#include "TaskBar.hpp"
#include "constitle.hpp"
#include "keyboard.hpp"
#include "flink.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "wakeful.hpp"

static void DrawGetDirInfoMsg(const wchar_t *Title,const wchar_t *Name,const UINT64* Size)
{
	string strSize;
	FileSizeToStr(strSize,*Size,8,COLUMN_FLOATSIZE|COLUMN_COMMAS);
	RemoveLeadingSpaces(strSize);
	Message(0,0,Title,MSG(MScanningFolder),Name,strSize);
	PreRedrawItem preRedrawItem=PreRedraw.Peek();
	preRedrawItem.Param.Param1=Title;
	preRedrawItem.Param.Param2=Name;
	preRedrawItem.Param.Param3=Size;
	PreRedraw.SetParam(preRedrawItem.Param);
}

static void PR_DrawGetDirInfoMsg()
{
	PreRedrawItem preRedrawItem=PreRedraw.Peek();
	DrawGetDirInfoMsg(
		static_cast<const wchar_t*>(preRedrawItem.Param.Param1),
		static_cast<const wchar_t*>(preRedrawItem.Param.Param2),
		static_cast<const UINT64*>(preRedrawItem.Param.Param3)
	);
}

int GetDirInfo(const wchar_t *Title,
               const wchar_t *DirName,
               unsigned long &DirCount,
               unsigned long &FileCount,
               unsigned __int64 &FileSize,
               unsigned __int64 &CompressedFileSize,
               unsigned __int64 &RealSize,
               unsigned long &ClusterSize,
               clock_t MsgWaitTime,
               FileFilter *Filter,
               DWORD Flags)
{
	string strFullDirName, strDriveRoot;
	string strFullName, strCurDirName, strLastDirName;
	ConvertNameToFull(DirName, strFullDirName);
	SaveScreen SaveScr;
	UndoGlobalSaveScrPtr UndSaveScr(&SaveScr);
	TPreRedrawFuncGuard preRedrawFuncGuard(PR_DrawGetDirInfoMsg);
	TaskBar TB(MsgWaitTime!=-1);
	wakeful W;
	ScanTree ScTree(FALSE,TRUE,(Flags&GETDIRINFO_SCANSYMLINKDEF?(DWORD)-1:(Flags&GETDIRINFO_SCANSYMLINK)));
	FAR_FIND_DATA_EX FindData;
	clock_t StartTime=clock();
	SetCursorType(FALSE,0);
	GetPathRoot(strFullDirName,strDriveRoot);
	/* $ 20.03.2002 DJ
	   ��� . - ������� ��� ������������� ��������
	*/
	const wchar_t *ShowDirName = DirName;

	if (DirName[0] == L'.' && !DirName[1])
	{
		const wchar_t *p = LastSlash(strFullDirName);

		if (p)
			ShowDirName = p + 1;
	}

	ConsoleTitle OldTitle;
	RefreshFrameManager frref(ScrX,ScrY,MsgWaitTime,Flags&GETDIRINFO_DONTREDRAWFRAME);
	DWORD SectorsPerCluster=0,BytesPerSector=0,FreeClusters=0,Clusters=0;

	if (GetDiskFreeSpace(strDriveRoot,&SectorsPerCluster,&BytesPerSector,&FreeClusters,&Clusters))
		ClusterSize=SectorsPerCluster*BytesPerSector;

	// ��������� ��������� ��� ���������
	strLastDirName.Clear();
	strCurDirName.Clear();
	DirCount=FileCount=0;
	FileSize=CompressedFileSize=RealSize=0;
	ScTree.SetFindPath(DirName,L"*");

	while (ScTree.GetNextName(&FindData,strFullName))
	{
		if (!CtrlObject->Macro.IsExecuting())
		{
			INPUT_RECORD rec;

			switch (PeekInputRecord(&rec))
			{
				case 0:
				case KEY_IDLE:
					break;
				case KEY_NONE:
				case KEY_ALT:
				case KEY_CTRL:
				case KEY_SHIFT:
				case KEY_RALT:
				case KEY_RCTRL:
					GetInputRecord(&rec);
					break;
				case KEY_ESC:
				case KEY_BREAK:
					GetInputRecord(&rec);
					return 0;
				default:

					if (Flags&GETDIRINFO_ENHBREAK)
					{
						return -1;
					}

					GetInputRecord(&rec);
					break;
			}
		}

		clock_t CurTime=clock();

		if (MsgWaitTime!=-1 && CurTime-StartTime > MsgWaitTime)
		{
			StartTime=CurTime;
			MsgWaitTime=500;
			OldTitle << MSG(MScanningFolder) << L" " << ShowDirName << fmt::Flush(); // ������� ��������� �������
			SetCursorType(FALSE,0);
			DrawGetDirInfoMsg(Title,ShowDirName,&FileSize);
		}

		if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// ������� ��������� ���������� ������ ���� �� ������� ������,
			// � ��������� ������ ��� ����� ������ � �������� ���������� ������
			if (!(Flags&GETDIRINFO_USEFILTER))
				DirCount++;
			else
			{
				// ���� ������� �� �������� ��� ������ �� ��� ���� ���������
				// ���������� - ����� ��� ���������� �������� total
				// �� ������ (mantis 551)
				if (!Filter->FileInFilter(FindData))
					ScTree.SkipDir();
			}
		}
		else
		{
			/* $ 17.04.2005 KM
			   �������� ��������� ����� � ������� ������
			*/
			if ((Flags&GETDIRINFO_USEFILTER))
			{
				if (!Filter->FileInFilter(FindData))
					continue;
			}

			// ���������� ������� ��������� ��� ���������� ������� ������ �����,
			// ����� � ����� �������� ������ ����, ��������������� ��������
			// �������.
			if ((Flags&GETDIRINFO_USEFILTER))
			{
				strCurDirName = strFullName;
				CutToSlash(strCurDirName); //???

				if (StrCmpI(strCurDirName,strLastDirName))
				{
					DirCount++;
					strLastDirName = strCurDirName;
				}
			}

			FileCount++;
			unsigned __int64 CurSize = FindData.nFileSize;
			FileSize+=CurSize;

			if (FindData.dwFileAttributes & (FILE_ATTRIBUTE_COMPRESSED|FILE_ATTRIBUTE_SPARSE_FILE))
			{
				UINT64 Size=0;

				if (apiGetCompressedFileSize(strFullName,Size))
				{
					CurSize=Size;
				}
			}

			CompressedFileSize+=CurSize;

			if (ClusterSize>0)
			{
				RealSize+=CurSize;
				int Slack=(__int32)(CurSize%ClusterSize);

				if (Slack>0)
					RealSize+=ClusterSize-Slack;
			}
		}
	}

	return 1;
}


int GetPluginDirInfo(HANDLE hPlugin,const wchar_t *DirName,unsigned long &DirCount,
                     unsigned long &FileCount,unsigned __int64 &FileSize,
                     unsigned __int64 &CompressedFileSize)
{
	PluginPanelItem *PanelItem=nullptr;
	size_t ItemsNumber=0;
	int ExitCode;
	DirCount=FileCount=0;
	FileSize=CompressedFileSize=0;
	PluginHandle *ph = (PluginHandle*)hPlugin;

	if ((ExitCode=FarGetPluginDirList((INT_PTR)ph->pPlugin, ph->hPlugin, DirName, &PanelItem,&ItemsNumber))==TRUE) //INT_PTR - BUGBUG
	{
		for (size_t I=0; I<ItemsNumber; I++)
		{
			if (PanelItem[I].FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				DirCount++;
			}
			else
			{
				FileCount++;
				FileSize+=PanelItem[I].FileSize;
				CompressedFileSize+=PanelItem[I].PackSize?PanelItem[I].PackSize:PanelItem[I].FileSize;
			}
		}
	}

	if (PanelItem)
		FarFreePluginDirList(PanelItem, ItemsNumber);

	return(ExitCode);
}
