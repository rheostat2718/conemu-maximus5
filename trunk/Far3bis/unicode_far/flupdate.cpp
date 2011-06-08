/*
flupdate.cpp

�������� ������ - ������ ���� ������
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

#include "filelist.hpp"
#include "flink.hpp"
#include "colors.hpp"
#include "lang.hpp"
#include "filepanels.hpp"
#include "cmdline.hpp"
#include "filefilter.hpp"
#include "hilight.hpp"
#include "ctrlobj.hpp"
#include "manager.hpp"
#include "TPreRedrawFunc.hpp"
#include "syslog.hpp"
#include "TaskBar.hpp"
#include "cddrv.hpp"
#include "interf.hpp"
#include "keyboard.hpp"
#include "message.hpp"
#include "config.hpp"
#include "fileowner.hpp"
#include "delete.hpp"
#include "pathmix.hpp"
#include "network.hpp"
#include "dirmix.hpp"
#include "strmix.hpp"
#include "mix.hpp"

// ����� ��� ReadDiz()
enum ReadDizFlags
{
	RDF_NO_UPDATE         = 0x00000001UL,
};

void FileList::Update(int Mode)
{
	_ALGO(CleverSysLog clv(L"FileList::Update"));
	_ALGO(SysLog(L"(Mode=[%d/0x%08X] %s)",Mode,Mode,(Mode==UPDATE_KEEP_SELECTION?L"UPDATE_KEEP_SELECTION":L"")));

	if (EnableUpdate)
		switch (PanelMode)
		{
			case NORMAL_PANEL:
				ReadFileNames(Mode & UPDATE_KEEP_SELECTION, Mode & UPDATE_IGNORE_VISIBLE,Mode & UPDATE_DRAW_MESSAGE);
				break;
			case PLUGIN_PANEL:
			{
				OpenPanelInfo Info;
				CtrlObject->Plugins.GetOpenPanelInfo(hPlugin,&Info);
				ProcessPluginCommand();

				if (PanelMode!=PLUGIN_PANEL)
					ReadFileNames(Mode & UPDATE_KEEP_SELECTION, Mode & UPDATE_IGNORE_VISIBLE,Mode & UPDATE_DRAW_MESSAGE);
				else if ((Info.Flags & OPIF_REALNAMES) ||
				         CtrlObject->Cp()->GetAnotherPanel(this)->GetMode()==PLUGIN_PANEL ||
				         !(Mode & UPDATE_SECONDARY))
					UpdatePlugin(Mode & UPDATE_KEEP_SELECTION, Mode & UPDATE_IGNORE_VISIBLE);
			}
			ProcessPluginCommand();
			break;
		}

	LastUpdateTime=clock();
}

void FileList::UpdateIfRequired()
{
	if (UpdateRequired && !UpdateDisabled)
	{
		UpdateRequired = FALSE;
		Update(UpdateRequiredMode | UPDATE_IGNORE_VISIBLE);
	}
}

void ReadFileNamesMsg(const wchar_t *Msg)
{
	Message(0,0,MSG(MReadingTitleFiles),Msg);
	PreRedrawItem preRedrawItem=PreRedraw.Peek();
	preRedrawItem.Param.Param1=(void*)Msg;
	PreRedraw.SetParam(preRedrawItem.Param);
}

static void PR_ReadFileNamesMsg()
{
	PreRedrawItem preRedrawItem=PreRedraw.Peek();
	ReadFileNamesMsg((wchar_t *)preRedrawItem.Param.Param1);
}


// ��� ���� ����� ����� ��� ���������� ������������� Far Manager
// ��� ���������� �����������

void FileList::ReadFileNames(int KeepSelection, int IgnoreVisible, int DrawMessage)
{
	TPreRedrawFuncGuard preRedrawFuncGuard(PR_ReadFileNamesMsg);
	TaskBar TB(false);

	strOriginalCurDir=strCurDir;

	if (!IsVisible() && !IgnoreVisible)
	{
		UpdateRequired=TRUE;
		UpdateRequiredMode=KeepSelection;
		return;
	}

	UpdateRequired=FALSE;
	AccessTimeUpdateRequired=FALSE;
	DizRead=FALSE;
	FAR_FIND_DATA_EX fdata;
	FileListItem *CurPtr=0,**OldData=0;
	string strCurName, strNextCurName;
	int OldFileCount=0;
	CloseChangeNotification();

	if (this!=CtrlObject->Cp()->LeftPanel && this!=CtrlObject->Cp()->RightPanel)
		return;

	string strSaveDir;
	apiGetCurrentDirectory(strSaveDir);
	{
		string strOldCurDir = strCurDir;

		if (!SetCurPath())
		{
			FlushInputBuffer(); // ������� ������ �����, �.�. �� ��� ����� ���� � ������ �����...

			if (!StrCmp(strCurDir, strOldCurDir)) //?? i??
			{
				GetPathRoot(strOldCurDir,strOldCurDir);

				if (!apiIsDiskInDrive(strOldCurDir))
					IfGoHome(strOldCurDir.At(0));

				/* ��� ����� �������� ���� �� ��������� */
			}

			return;
		}
	}
	SortGroupsRead=FALSE;

	if (GetFocus())
		CtrlObject->CmdLine->SetCurDir(strCurDir);

	LastCurFile=-1;
	Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
	AnotherPanel->QViewDelTempName();
	int PrevSelFileCount=SelFileCount;
	SelFileCount=0;
	SelFileSize=0;
	TotalFileCount=0;
	TotalFileSize=0;
	CacheSelIndex=-1;
	CacheSelClearIndex=-1;

	if (Opt.ShowPanelFree)
	{
		unsigned __int64 TotalSize,TotalFree;

		if (!apiGetDiskSize(strCurDir,&TotalSize,&TotalFree,&FreeDiskSize))
			FreeDiskSize=0;
	}

	if (FileCount>0)
	{
		strCurName = ListData[CurFile]->strName;

		if (ListData[CurFile]->Selected)
		{
			for (int i=CurFile+1; i < FileCount; i++)
			{
				CurPtr = ListData[i];

				if (!CurPtr->Selected)
				{
					strNextCurName = CurPtr->strName;
					break;
				}
			}
		}
	}

	if (KeepSelection || PrevSelFileCount>0)
	{
		OldData=ListData;
		OldFileCount=FileCount;
	}
	else
		DeleteListData(ListData,FileCount);

	ListData=nullptr;
	int ReadOwners=IsColumnDisplayed(OWNER_COLUMN);
	int ReadPacked=IsColumnDisplayed(PACKED_COLUMN);
	int ReadNumLinks=IsColumnDisplayed(NUMLINK_COLUMN);
	int ReadNumStreams=IsColumnDisplayed(NUMSTREAMS_COLUMN);
	int ReadStreamsSize=IsColumnDisplayed(STREAMSSIZE_COLUMN);
	string strComputerName;

	if (ReadOwners)
	{
		CurPath2ComputerName(strCurDir, strComputerName);
		// ������� ��� SID`��
		SIDCacheFlush();
	}

	SetLastError(ERROR_SUCCESS);
	int AllocatedCount=0;
	FileListItem *NewPtr;
	// ���������� ��������� ��� �����
	wchar_t Title[2048];
	int TitleLength=Min((int)X2-X1-1,(int)(ARRAYSIZE(Title))-1);
	//wmemset(Title,0x0CD,TitleLength); //BUGBUG
	//Title[TitleLength]=0;
	MakeSeparator(TitleLength, Title, 9, nullptr);
	BOOL IsShowTitle=FALSE;
	BOOL NeedHighlight=Opt.Highlight && PanelMode != PLUGIN_PANEL;

	if (!Filter)
		Filter=new FileFilter(this,FFT_PANEL);

	//������ �������� ������� ��� ������� ����� ������� ��������
	Filter->UpdateCurrentTime();
	CtrlObject->HiFiles->UpdateCurrentTime();
	bool bCurDirRoot=IsLocalRootPath(strCurDir)||IsLocalPrefixRootPath(strCurDir)||IsLocalVolumeRootPath(strCurDir);

	FileCount = 0;
	::FindFile Find(strCurDir+L"\\"+L"*",true);
	DWORD FindErrorCode = ERROR_SUCCESS;
	bool UseFilter=Filter->IsEnabledOnPanel();
	bool ReadCustomData=IsColumnDisplayed(CUSTOM_COLUMN0)!=0 && (SortMode==BY_CUSTOMDATA || (ViewSettings.Flags&PVS_PRELOADC0DATA));

	DWORD StartTime = GetTickCount();

	while (Find.Get(fdata))
	{
		FindErrorCode = GetLastError();

		if ((Opt.ShowHidden || !(fdata.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM))) && (!UseFilter || Filter->FileInFilter(fdata)))
		{
			if (FileCount>=AllocatedCount)
			{
				AllocatedCount=AllocatedCount+256+AllocatedCount/4;
				FileListItem **pTemp;

				if (!(pTemp=(FileListItem **)xf_realloc(ListData,AllocatedCount*sizeof(*ListData))))
					break;

				ListData=pTemp;
			}

			ListData[FileCount] = new FileListItem;
			ListData[FileCount]->Clear();
			NewPtr=ListData[FileCount];
			NewPtr->FileAttr = fdata.dwFileAttributes;
			NewPtr->CreationTime = fdata.ftCreationTime;
			NewPtr->AccessTime = fdata.ftLastAccessTime;
			NewPtr->WriteTime = fdata.ftLastWriteTime;
			NewPtr->ChangeTime = fdata.ftChangeTime;
			NewPtr->UnpSize = fdata.nFileSize;
			NewPtr->strName = fdata.strFileName;
			NewPtr->strShortName = fdata.strAlternateFileName;
			NewPtr->Position=FileCount++;
			NewPtr->NumberOfLinks=1;

			if (fdata.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
			{
				NewPtr->ReparseTag=fdata.dwReserved0; //MSDN
			}

			if (!(fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				TotalFileSize += NewPtr->UnpSize;
				bool Compressed=false;

				if (ReadPacked && ((fdata.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED) || (fdata.dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE)))
				{
					if (apiGetCompressedFileSize(fdata.strFileName,NewPtr->PackSize))
					{
						Compressed=true;
					}
				}

				if (!Compressed)
					NewPtr->PackSize = fdata.nFileSize;

				if (ReadNumLinks)
					NewPtr->NumberOfLinks=GetNumberOfLinks(fdata.strFileName);
			}
			else
			{
				NewPtr->PackSize = 0;
			}

			NewPtr->SortGroup=DEFAULT_SORT_GROUP;

			if (ReadOwners)
			{
				string strOwner;
				GetFileOwner(strComputerName, NewPtr->strName,strOwner);
				NewPtr->strOwner = strOwner;
			}

			NewPtr->NumberOfStreams=NewPtr->FileAttr&FILE_ATTRIBUTE_DIRECTORY?0:1;
			NewPtr->StreamsSize=NewPtr->UnpSize;

			if (ReadNumStreams||ReadStreamsSize)
			{
				EnumStreams(TestParentFolderName(fdata.strFileName)?strCurDir:fdata.strFileName,NewPtr->StreamsSize,NewPtr->NumberOfStreams);
			}

			if (ReadCustomData)
				CtrlObject->Plugins.GetCustomData(NewPtr);

			if (NeedHighlight)
				CtrlObject->HiFiles->GetHiColor(&NewPtr,1);

			if (!(fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				TotalFileCount++;

			//memcpy(ListData+FileCount,&NewPtr,sizeof(NewPtr));
//      FileCount++;

			DWORD CurTime = GetTickCount();
			if (CurTime - StartTime > RedrawTimeout)
			{
				StartTime = CurTime;
				if (IsVisible())
				{
					string strReadMsg;

					if (!IsShowTitle)
					{
						if (!DrawMessage)
						{
							Text(X1+1,Y1,COL_PANELBOX,Title);
							IsShowTitle=TRUE;
							SetColor(Focus ? COL_PANELSELECTEDTITLE:COL_PANELTITLE);
						}
					}

					strReadMsg.Format(MSG(MReadingFiles),FileCount);

					if (DrawMessage)
					{
						ReadFileNamesMsg(strReadMsg);
					}
					else
					{
						TruncStr(strReadMsg,TitleLength-2);
						int MsgLength=(int)strReadMsg.GetLength();
						GotoXY(X1+1+(TitleLength-MsgLength-1)/2,Y1);
						FS<<L" "<<strReadMsg<<L" ";
					}
				}

				if (CheckForEsc())
				{
					break;
				}
			}
		}
	}

	if (!(FindErrorCode==ERROR_SUCCESS || FindErrorCode==ERROR_NO_MORE_FILES || FindErrorCode==ERROR_FILE_NOT_FOUND))
		Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MReadFolderError),MSG(MOk));
	/*
	int NetRoot=FALSE;
	if (strCurDir.At(0)==L'\\' && strCurDir.At(1)==L'\\')
	{
		const wchar_t *ChPtr=wcschr(strCurDir.CPtr()+2,'\\');
		if (!ChPtr || !wcschr(ChPtr+1,L'\\'))
			NetRoot=TRUE;
	}
	*/

	// ���� ����� �����������, �������� �� ���� � �� ����������.
	if (!bCurDirRoot) // && !NetRoot)
	{
		if (FileCount>=AllocatedCount)
		{
			FileListItem **pTemp;

			if ((pTemp=(FileListItem **)xf_realloc(ListData,(FileCount+1)*sizeof(*ListData))))
				ListData=pTemp;
		}

		if (ListData)
		{
			ListData[FileCount] = new FileListItem;

			string TwoDotsOwner;
			if (ReadOwners)
			{
				GetFileOwner(strComputerName,strCurDir,TwoDotsOwner);
			}

			FILETIME TwoDotsTimes[4]={0};
			if(apiGetFindDataEx(strCurDir,fdata))
			{
				TwoDotsTimes[0]=fdata.ftCreationTime;
				TwoDotsTimes[1]=fdata.ftLastAccessTime;
				TwoDotsTimes[2]=fdata.ftLastWriteTime;
				TwoDotsTimes[3]=fdata.ftChangeTime;
			}

			AddParentPoint(ListData[FileCount],FileCount,TwoDotsTimes,TwoDotsOwner);

			if (NeedHighlight)
				CtrlObject->HiFiles->GetHiColor(&ListData[FileCount],1);

			FileCount++;
		}
	}

	if (IsColumnDisplayed(DIZ_COLUMN))
		ReadDiz();

	if (AnotherPanel->GetMode()==PLUGIN_PANEL)
	{
		HANDLE hAnotherPlugin=AnotherPanel->GetPluginHandle();
		PluginPanelItem *PanelData=nullptr;
		string strPath;
		int PanelCount=0;
		strPath = strCurDir;
		AddEndSlash(strPath);

		if (CtrlObject->Plugins.GetVirtualFindData(hAnotherPlugin,&PanelData,&PanelCount,strPath))
		{
			FileListItem **pTemp;

			if ((pTemp=(FileListItem **)xf_realloc(ListData,(FileCount+PanelCount)*sizeof(*ListData))))
			{
				ListData=pTemp;

				for (int i=0; i < PanelCount; i++)
				{
					CurPtr = ListData[FileCount+i];
					PluginPanelItem &fdata=PanelData[i];
					PluginToFileListItem(&PanelData[i],CurPtr);
					CurPtr->Position=FileCount;
					TotalFileSize += fdata.FileSize;
					CurPtr->PrevSelected=CurPtr->Selected=0;
					CurPtr->ShowFolderSize=0;
					CurPtr->SortGroup=CtrlObject->HiFiles->GetGroup(CurPtr);

					if (!TestParentFolderName(fdata.FileName) && !(CurPtr->FileAttr & FILE_ATTRIBUTE_DIRECTORY))
						TotalFileCount++;
				}

				// �������� ������ ��������� � ����� �����, �� ���� ���
				CtrlObject->HiFiles->GetHiColor(&ListData[FileCount],PanelCount);
				FileCount+=PanelCount;
			}

			CtrlObject->Plugins.FreeVirtualFindData(hAnotherPlugin,PanelData,PanelCount);
		}
	}

	CreateChangeNotification(FALSE);
	CorrectPosition();

	if (KeepSelection || PrevSelFileCount>0)
	{
		MoveSelection(ListData,FileCount,OldData,OldFileCount);
		DeleteListData(OldData,OldFileCount);
	}

	if (SortGroups)
		ReadSortGroups(false);

	if (!KeepSelection && PrevSelFileCount>0)
	{
		SaveSelection();
		ClearSelection();
	}

	SortFileList(FALSE);

	if (CurFile>=FileCount || StrCmpI(ListData[CurFile]->strName,strCurName))
		if (!GoToFile(strCurName) && !strNextCurName.IsEmpty())
			GoToFile(strNextCurName);

	/* $ 13.02.2002 DJ
		SetTitle() - ������ ���� �� ������� �����!
	*/
	if (CtrlObject->Cp() == FrameManager->GetCurrentFrame())
		SetTitle();

	FarChDir(strSaveDir); //???
}

/*$ 22.06.2001 SKV
  �������� �������� ��� ������ ����� ���������� �������.
*/
int FileList::UpdateIfChanged(int UpdateMode)
{
	//_SVS(SysLog(L"CurDir='%s' Opt.AutoUpdateLimit=%d <= FileCount=%d",CurDir,Opt.AutoUpdateLimit,FileCount));
	if (!Opt.AutoUpdateLimit || static_cast<DWORD>(FileCount) <= Opt.AutoUpdateLimit)
	{
		/* $ 19.12.2001 VVM
		  ! ������ ����������. ��� Force ���������� ������! */
		if ((IsVisible() && (clock()-LastUpdateTime>2000)) || (UpdateMode != UIC_UPDATE_NORMAL))
		{
			if (UpdateMode == UIC_UPDATE_NORMAL)
				ProcessPluginEvent(FE_IDLE,nullptr);

			/* $ 24.12.2002 VVM
			  ! �������� ������ ���������� �������. */
			if (// ���������� ������, �� ��� ����������� ����������� � ���� ������
			    (PanelMode==NORMAL_PANEL && hListChange!=INVALID_HANDLE_VALUE && WaitForSingleObject(hListChange,0)==WAIT_OBJECT_0) ||
			    // ��� ���������� ������, �� ��� ����������� � �� ��������� �������� ����� UPDATE_FORCE
			    (PanelMode==NORMAL_PANEL && hListChange==INVALID_HANDLE_VALUE && UpdateMode==UIC_UPDATE_FORCE) ||
			    // ��� ��������� ������ � ��������� ����� UPDATE_FORCE
			    (PanelMode!=NORMAL_PANEL && UpdateMode==UIC_UPDATE_FORCE)
			)
			{
				Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);

				if (AnotherPanel->GetType()==INFO_PANEL)
				{
					AnotherPanel->Update(UPDATE_KEEP_SELECTION);

					if (UpdateMode==UIC_UPDATE_NORMAL)
						AnotherPanel->Redraw();
				}

				Update(UPDATE_KEEP_SELECTION);

				if (UpdateMode==UIC_UPDATE_NORMAL)
					Show();

				return TRUE;
			}
		}
	}

	return FALSE;
}

void FileList::CreateChangeNotification(int CheckTree)
{
	wchar_t RootDir[4]=L" :\\";
	DWORD DriveType=DRIVE_REMOTE;
	CloseChangeNotification();

	if (IsLocalPath(strCurDir))
	{
		RootDir[0]=strCurDir.At(0);
		DriveType=FAR_GetDriveType(RootDir);
	}

	if (Opt.AutoUpdateRemoteDrive || (!Opt.AutoUpdateRemoteDrive && DriveType != DRIVE_REMOTE))
	{
		hListChange=FindFirstChangeNotification(strCurDir,CheckTree,
		                                        FILE_NOTIFY_CHANGE_FILE_NAME|
		                                        FILE_NOTIFY_CHANGE_DIR_NAME|
		                                        FILE_NOTIFY_CHANGE_ATTRIBUTES|
		                                        FILE_NOTIFY_CHANGE_SIZE|
		                                        FILE_NOTIFY_CHANGE_LAST_WRITE);
	}
}


void FileList::CloseChangeNotification()
{
	if (hListChange!=INVALID_HANDLE_VALUE)
	{
		DWORD nTID;
		HANDLE hCloseThread = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)FindCloseChangeNotification, hListChange, 0, &nTID);

		if (hCloseThread)
			CloseHandle(hCloseThread);
		else
			FindCloseChangeNotification(hListChange);

		hListChange=INVALID_HANDLE_VALUE;
	}
}

static int _cdecl SortSearchList(const void *el1,const void *el2)
{
	FileListItem **SPtr1=(FileListItem **)el1,**SPtr2=(FileListItem **)el2;
	return StrCmp(SPtr1[0]->strName,SPtr2[0]->strName);
}

void FileList::MoveSelection(FileListItem **ListData,long FileCount,
                             FileListItem **OldData,long OldFileCount)
{
	FileListItem **OldPtr;
	SelFileCount=0;
	SelFileSize=0;
	CacheSelIndex=-1;
	CacheSelClearIndex=-1;
	far_qsort(OldData,OldFileCount,sizeof(*OldData),SortSearchList);

	while (FileCount--)
	{
		OldPtr=(FileListItem **)bsearch(ListData,(void *)OldData,
		                                OldFileCount,sizeof(*ListData),SortSearchList);

		if (OldPtr)
		{
			if (OldPtr[0]->ShowFolderSize)
			{
				ListData[0]->ShowFolderSize=2;
				ListData[0]->UnpSize=OldPtr[0]->UnpSize;
				ListData[0]->PackSize=OldPtr[0]->PackSize;
			}

			Select(ListData[0],OldPtr[0]->Selected);
			ListData[0]->PrevSelected=OldPtr[0]->PrevSelected;
		}

		ListData++;
	}
}

void FileList::UpdatePlugin(int KeepSelection, int IgnoreVisible)
{
	_ALGO(CleverSysLog clv(L"FileList::UpdatePlugin"));
	_ALGO(SysLog(L"(KeepSelection=%d, IgnoreVisible=%d)",KeepSelection,IgnoreVisible));

	if (!IsVisible() && !IgnoreVisible)
	{
		UpdateRequired=TRUE;
		UpdateRequiredMode=KeepSelection;
		return;
	}

	DizRead=FALSE;
	FileListItem *CurPtr, **OldData=0;
	string strCurName, strNextCurName;
	int OldFileCount=0;
	CloseChangeNotification();
	LastCurFile=-1;
	OpenPanelInfo Info;
	CtrlObject->Plugins.GetOpenPanelInfo(hPlugin,&Info);

	FreeDiskSize=0;
	if (Opt.ShowPanelFree)
	{
		if (Info.Flags & OPIF_REALNAMES)
		{
			unsigned __int64 TotalSize,TotalFree;
			if (!apiGetDiskSize(strCurDir,&TotalSize,&TotalFree,&FreeDiskSize))
				FreeDiskSize=0;
		}
		else if (Info.Flags & OPIF_USEFREESIZE)
			FreeDiskSize=Info.FreeSize;
	}

	PluginPanelItem *PanelData=nullptr;
	int PluginFileCount;

	if (!CtrlObject->Plugins.GetFindData(hPlugin,&PanelData,&PluginFileCount,0))
	{
		DeleteListData(ListData,FileCount);
		PopPlugin(TRUE);
		Update(KeepSelection);

		// WARP> ����� ���, �� ����� ������������ - ��������������� ������� �� ������ ��� ������ ������ ������.
		if (!PrevDataList.Empty())
			GoToFile((*PrevDataList.Last())->strPrevName);

		return;
	}

	int PrevSelFileCount=SelFileCount;
	SelFileCount=0;
	SelFileSize=0;
	TotalFileCount=0;
	TotalFileSize=0;
	CacheSelIndex=-1;
	CacheSelClearIndex=-1;
	strPluginDizName.Clear();

	if (FileCount>0)
	{
		CurPtr=ListData[CurFile];
		strCurName = CurPtr->strName;

		if (CurPtr->Selected)
		{
			for (int i=CurFile+1; i < FileCount; i++)
			{
				CurPtr = ListData[i];

				if (!CurPtr->Selected)
				{
					strNextCurName = CurPtr->strName;
					break;
				}
			}
		}
	}
	else if (Info.Flags & OPIF_ADDDOTS)
	{
		strCurName = L"..";
	}

	if (KeepSelection || PrevSelFileCount>0)
	{
		OldData=ListData;
		OldFileCount=FileCount;
	}
	else
	{
		DeleteListData(ListData,FileCount);
	}

	FileCount=PluginFileCount;
	ListData=(FileListItem**)xf_malloc(sizeof(FileListItem*)*(FileCount+1));

	if (!ListData)
	{
		FileCount=0;
		return;
	}

	if (!Filter)
		Filter=new FileFilter(this,FFT_PANEL);

	//������ �������� ������� ��� ������� ����� ������� ��������
	Filter->UpdateCurrentTime();
	CtrlObject->HiFiles->UpdateCurrentTime();
	int DotsPresent=FALSE;
	int FileListCount=0;
	bool UseFilter=Filter->IsEnabledOnPanel();

	for (int i=0; i < FileCount; i++)
	{
		ListData[FileListCount] = new FileListItem;
		FileListItem *CurListData=ListData[FileListCount];
		CurListData->Clear();

		if (UseFilter && !(Info.Flags & OPIF_DISABLEFILTER))
			//if (!(CurPanelData->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			if (!Filter->FileInFilter(PanelData[i]))
				continue;

		if (!Opt.ShowHidden && (PanelData[i].FileAttributes & (FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM)))
			continue;

		//memset(CurListData,0,sizeof(*CurListData));
		PluginToFileListItem(&PanelData[i],CurListData);
		CurListData->Position=i;

		if (!(Info.Flags & OPIF_DISABLESORTGROUPS)/* && !(CurListData->FileAttr & FILE_ATTRIBUTE_DIRECTORY)*/)
			CurListData->SortGroup=CtrlObject->HiFiles->GetGroup(CurListData);
		else
			CurListData->SortGroup=DEFAULT_SORT_GROUP;

		if (!CurListData->DizText)
		{
			CurListData->DeleteDiz=FALSE;
			//CurListData->DizText=nullptr;
		}

		if (TestParentFolderName(CurListData->strName))
		{
			DotsPresent=TRUE;
			CurListData->FileAttr|=FILE_ATTRIBUTE_DIRECTORY;
		}
		else if (!(CurListData->FileAttr & FILE_ATTRIBUTE_DIRECTORY))
		{
			TotalFileCount++;
		}

		TotalFileSize += CurListData->UnpSize;
		FileListCount++;
	}

	if (!(Info.Flags & OPIF_DISABLEHIGHLIGHTING) || (Info.Flags & OPIF_USEATTRHIGHLIGHTING))
		CtrlObject->HiFiles->GetHiColor(ListData,FileListCount,(Info.Flags&OPIF_USEATTRHIGHLIGHTING)!=0);

	FileCount=FileListCount;

	if ((Info.Flags & OPIF_ADDDOTS) && !DotsPresent)
	{
		ListData[FileCount] = new FileListItem;
		FileListItem *CurPtr = ListData[FileCount];
		CurPtr->Clear();
		AddParentPoint(CurPtr,FileCount);

		if (!(Info.Flags & OPIF_DISABLEHIGHLIGHTING) || (Info.Flags & OPIF_USEATTRHIGHLIGHTING))
			CtrlObject->HiFiles->GetHiColor(&CurPtr,1,(Info.Flags&OPIF_USEATTRHIGHLIGHTING)!=0);

		if (Info.HostFile && *Info.HostFile)
		{
			FAR_FIND_DATA_EX FindData;

			if (apiGetFindDataEx(Info.HostFile, FindData))
			{
				CurPtr->WriteTime=FindData.ftLastWriteTime;
				CurPtr->CreationTime=FindData.ftCreationTime;
				CurPtr->AccessTime=FindData.ftLastAccessTime;
				CurPtr->ChangeTime=FindData.ftChangeTime;
			}
		}

		FileCount++;
	}

	if (CurFile >= FileCount)
		CurFile = FileCount ? FileCount-1 : 0;

	/* $ 25.02.2001 VVM
	    ! �� ��������� �������� ������ ������ � ������ ������� */
	if (IsColumnDisplayed(DIZ_COLUMN))
		ReadDiz(PanelData,PluginFileCount,RDF_NO_UPDATE);

	CorrectPosition();
	CtrlObject->Plugins.FreeFindData(hPlugin,PanelData,PluginFileCount);

	if (KeepSelection || PrevSelFileCount>0)
	{
		MoveSelection(ListData,FileCount,OldData,OldFileCount);
		DeleteListData(OldData,OldFileCount);
	}

	if (!KeepSelection && PrevSelFileCount>0)
	{
		SaveSelection();
		ClearSelection();
	}

	SortFileList(FALSE);

	if (CurFile>=FileCount || StrCmpI(ListData[CurFile]->strName,strCurName))
		if (!GoToFile(strCurName) && !strNextCurName.IsEmpty())
			GoToFile(strNextCurName);

	SetTitle();
}


void FileList::ReadDiz(PluginPanelItem *ItemList,int ItemLength,DWORD dwFlags)
{
	if (DizRead)
		return;

	DizRead=TRUE;
	Diz.Reset();

	if (PanelMode==NORMAL_PANEL)
	{
		Diz.Read(strCurDir);
	}
	else
	{
		PluginPanelItem *PanelData=nullptr;
		int PluginFileCount=0;
		OpenPanelInfo Info;
		CtrlObject->Plugins.GetOpenPanelInfo(hPlugin,&Info);

		if (!Info.DescrFilesNumber)
			return;

		int GetCode=TRUE;

		/* $ 25.02.2001 VVM
		    + ��������� ����� RDF_NO_UPDATE */
		if (!ItemList && !(dwFlags & RDF_NO_UPDATE))
		{
			GetCode=CtrlObject->Plugins.GetFindData(hPlugin,&PanelData,&PluginFileCount,0);
		}
		else
		{
			PanelData=ItemList;
			PluginFileCount=ItemLength;
		}

		if (GetCode)
		{
			for (size_t I=0; I<Info.DescrFilesNumber; I++)
			{
				PluginPanelItem *CurPanelData=PanelData;

				for (int J=0; J < PluginFileCount; J++, CurPanelData++)
				{
					string strFileName = CurPanelData->FileName;

					if (!StrCmpI(strFileName,Info.DescrFiles[I]))
					{
						string strTempDir, strDizName;

						if (FarMkTempEx(strTempDir) && apiCreateDirectory(strTempDir,nullptr))
						{
							if (CtrlObject->Plugins.GetFile(hPlugin,CurPanelData,strTempDir,strDizName,OPM_SILENT|OPM_VIEW|OPM_QUICKVIEW|OPM_DESCR))
							{
								strPluginDizName = Info.DescrFiles[I];
								Diz.Read(L"",strDizName);
								DeleteFileWithFolder(strDizName);
								I=Info.DescrFilesNumber;
								break;
							}

							apiRemoveDirectory(strTempDir);
							//ViewPanel->ShowFile(nullptr,FALSE,nullptr);
						}
					}
				}
			}

			/* $ 25.02.2001 VVM
			    + ��������� ����� RDF_NO_UPDATE */
			if (!ItemList && !(dwFlags & RDF_NO_UPDATE))
				CtrlObject->Plugins.FreeFindData(hPlugin,PanelData,PluginFileCount);
		}
	}

	for (int I=0; I<FileCount; I++)
	{
		if (!ListData[I]->DizText)
		{
			ListData[I]->DeleteDiz=FALSE;
			ListData[I]->DizText=(wchar_t*)Diz.GetDizTextAddr(ListData[I]->strName,ListData[I]->strShortName,ListData[I]->UnpSize);
		}
	}
}


void FileList::ReadSortGroups(bool UpdateFilterCurrentTime)
{
	if (!SortGroupsRead)
	{
		if (UpdateFilterCurrentTime)
		{
			CtrlObject->HiFiles->UpdateCurrentTime();
		}

		SortGroupsRead=TRUE;

		for (int i=0; i<FileCount; i++)
		{
			ListData[i]->SortGroup=CtrlObject->HiFiles->GetGroup(ListData[i]);
		}
	}
}

// �������� ������� CurPtr � ������� ���������������� ������ ��� �������� ".."
void FileList::AddParentPoint(FileListItem *CurPtr,long CurFilePos,FILETIME* Times,string Owner)
{
	CurPtr->Clear();
	CurPtr->FileAttr = FILE_ATTRIBUTE_DIRECTORY;
	CurPtr->strName = L"..";

	if (Times)
	{
		CurPtr->CreationTime = Times[0];
		CurPtr->AccessTime = Times[1];
		CurPtr->WriteTime = Times[2];
		CurPtr->ChangeTime = Times[3];
	}

	CurPtr->strOwner = Owner;
	CurPtr->Position = CurFilePos;
}
