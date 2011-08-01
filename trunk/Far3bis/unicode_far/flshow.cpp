/*
flshow.cpp

�������� ������ - ����� �� �����
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
#include "colors.hpp"
#include "lang.hpp"
#include "filefilter.hpp"
#include "cmdline.hpp"
#include "filepanels.hpp"
#include "ctrlobj.hpp"
#include "syslog.hpp"
#include "interf.hpp"
#include "config.hpp"
#include "datetime.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "flink.hpp"
#include "panelmix.hpp"

extern PanelViewSettings ViewSettingsArray[];
extern int ColumnTypeWidth[];

static wchar_t OutCharacter[8]={0,0,0,0,0,0,0,0};

static int __FormatEndSelectedPhrase(int Count)
{
	int M_Fmt=MListFileSize;

	if (Count != 1)
	{
		char StrItems[32];
		_itoa(Count,StrItems,10);
		int LenItems=(int)strlen(StrItems);

		if (StrItems[LenItems-1] == '1' && Count != 11)
			M_Fmt=MListFilesSize1;
		else
			M_Fmt=MListFilesSize2;
	}

	return M_Fmt;
}


void FileList::DisplayObject()
{
	Height=Y2-Y1+!Opt.ShowColumnTitles-(Opt.ShowPanelStatus ? (GetPanelStatusHeight()+2) : 2);
	_OT(SysLog(L"[%p] FileList::DisplayObject()",this));

	if (UpdateRequired)
	{
		UpdateRequired=FALSE;
		Update(UpdateRequiredMode);
	}

	ProcessPluginCommand();
	ShowFileList(FALSE);
}


void FileList::ShowFileList(int Fast)
{
	if (Locked())
	{
		CorrectPosition();
		return;
	}

	string strTitle;
	string strInfoCurDir;
	int Length;
	OpenPanelInfo Info;

	if (PanelMode==PLUGIN_PANEL)
	{
		if (ProcessPluginEvent(FE_REDRAW,nullptr))
			return;

		CtrlObject->Plugins.GetOpenPanelInfo(hPlugin,&Info);
		strInfoCurDir=Info.CurDir;
	}

	bool CurFullScreen=IsFullScreen();
	PrepareViewSettings(ViewMode,&Info);
	CorrectPosition();

	if (CurFullScreen!=IsFullScreen())
	{
		CtrlObject->Cp()->SetScreenPosition();
		CtrlObject->Cp()->GetAnotherPanel(this)->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
	}

	SetScreen(X1+1,Y1+1,X2-1,Y2-1,L' ',ColorIndexToColor(COL_PANELTEXT));
	Box(X1,Y1,X2,Y2,ColorIndexToColor(COL_PANELBOX),DOUBLE_BOX);

	if (Opt.ShowColumnTitles)
	{
//    SetScreen(X1+1,Y1+1,X2-1,Y1+1,' ',COL_PANELTEXT);
		SetColor(COL_PANELTEXT); //???
		//GotoXY(X1+1,Y1+1);
		//FS<<fmt::Width(X2-X1-1)<<L"";
	}

	for (int I=0,ColumnPos=X1+1; I<ViewSettings.ColumnCount; I++)
	{
		if (ViewSettings.ColumnWidth[I]<0)
			continue;

		if (Opt.ShowColumnTitles)
		{
			string strTitle;
			int IDMessage=-1;

			switch (ViewSettings.ColumnType[I] & 0xff)
			{
				case NAME_COLUMN:
					IDMessage=MColumnName;
					break;
				case EXTENSION_COLUMN:
					IDMessage=MColumnExtension;
					break;
				case SIZE_COLUMN:
					IDMessage=MColumnSize;
					break;
				case PACKED_COLUMN:
					IDMessage=MColumnPacked;
					break;
				case DATE_COLUMN:
					IDMessage=MColumnDate;
					break;
				case TIME_COLUMN:
					IDMessage=MColumnTime;
					break;
				case WDATE_COLUMN:
						IDMessage=MColumnWrited;
					break;
				case CDATE_COLUMN:
					IDMessage=MColumnCreated;
					break;
				case ADATE_COLUMN:
					IDMessage=MColumnAccessed;
					break;
				case CHDATE_COLUMN:
					IDMessage=MColumnChanged;
					break;
				case ATTR_COLUMN:
					IDMessage=MColumnAttr;
					break;
				case DIZ_COLUMN:
					IDMessage=MColumnDescription;
					break;
				case OWNER_COLUMN:
					IDMessage=MColumnOwner;
					break;
				case NUMLINK_COLUMN:
					IDMessage=MColumnMumLinks;
					break;
				case NUMSTREAMS_COLUMN:
					IDMessage=MColumnNumStreams;
					break;
				case STREAMSSIZE_COLUMN:
					IDMessage=MColumnStreamsSize;
					break;
			}

			if (IDMessage != -1)
				strTitle=MSG(IDMessage);

			if (PanelMode==PLUGIN_PANEL && Info.PanelModesArray &&
			        ViewMode<static_cast<int>(Info.PanelModesNumber) &&
			        Info.PanelModesArray[ViewMode].ColumnTitles)
			{
				const wchar_t *NewTitle=Info.PanelModesArray[ViewMode].ColumnTitles[I];

				if (NewTitle)
					strTitle=NewTitle;
			}

			string strTitleMsg;
			CenterStr(strTitle,strTitleMsg,ViewSettings.ColumnWidth[I]);
			SetColor(COL_PANELCOLUMNTITLE);
			GotoXY(ColumnPos,Y1+1);
			FS<<fmt::Precision(ViewSettings.ColumnWidth[I])<<strTitleMsg;
		}

		if (I>=ViewSettings.ColumnCount-1)
			break;

		if (ViewSettings.ColumnWidth[I+1]<0)
			continue;

		SetColor(COL_PANELBOX);
		ColumnPos+=ViewSettings.ColumnWidth[I];
		GotoXY(ColumnPos,Y1);
		BoxText(BoxSymbols[BS_T_H2V1]);

		if (Opt.ShowColumnTitles)
		{
			GotoXY(ColumnPos,Y1+1);
			BoxText(BoxSymbols[BS_V1]);
		}

		if (!Opt.ShowPanelStatus)
		{
			GotoXY(ColumnPos,Y2);
			BoxText(BoxSymbols[BS_B_H2V1]);
		}

		ColumnPos++;
	}

	int NextX1=X1+1;

	if (Opt.ShowSortMode)
	{
		static int SortModes[]={UNSORTED,BY_NAME,BY_EXT,BY_MTIME,BY_CTIME,
		                        BY_ATIME,BY_CHTIME,BY_SIZE,BY_DIZ,BY_OWNER,
		                        BY_COMPRESSEDSIZE,BY_NUMLINKS,
		                        BY_NUMSTREAMS,BY_STREAMSSIZE,
		                        BY_FULLNAME,BY_CUSTOMDATA
		                       };
		static int SortStrings[]={MMenuUnsorted,MMenuSortByName,
		                          MMenuSortByExt,MMenuSortByWrite,MMenuSortByCreation,
		                          MMenuSortByAccess,MMenuSortByChange,MMenuSortBySize,MMenuSortByDiz,MMenuSortByOwner,
		                          MMenuSortByCompressedSize,MMenuSortByNumLinks,MMenuSortByNumStreams,MMenuSortByStreamsSize,
		                          MMenuSortByFullName,MMenuSortByCustomData
		                         };

		for (size_t I=0; I<ARRAYSIZE(SortModes); I++)
		{
			if (SortModes[I]==SortMode)
			{
				const wchar_t *SortStr=MSG(SortStrings[I]);
				const wchar_t *Ch=wcschr(SortStr,L'&');

				if (Ch)
				{
					if (Opt.ShowColumnTitles)
						GotoXY(NextX1,Y1+1);
					else
						GotoXY(NextX1,Y1);

					SetColor(COL_PANELCOLUMNTITLE);
					OutCharacter[0]=SortOrder==1 ? Lower(Ch[1]):Upper(Ch[1]);
					Text(OutCharacter);
					NextX1++;

					if (Filter && Filter->IsEnabledOnPanel())
					{
						OutCharacter[0]=L'*';
						Text(OutCharacter);
						NextX1++;
					}
				}

				break;
			}
		}
	}

	/* <������ ����������> */
	if (/*GetNumericSort() || GetCaseSensitiveSort() || GetSortGroups() || */GetSelectedFirstMode())
	{
		if (Opt.ShowColumnTitles)
			GotoXY(NextX1,Y1+1);
		else
			GotoXY(NextX1,Y1);

		SetColor(COL_PANELCOLUMNTITLE);
		wchar_t *PtrOutCharacter=OutCharacter;
		*PtrOutCharacter=0;

		//if (GetSelectedFirstMode())
			*PtrOutCharacter++=L'^';

		/*
		    if(GetNumericSort())
		      *PtrOutCharacter++=L'#';
		    if(GetSortGroups())
		      *PtrOutCharacter++=L'@';
		*/
		/*
		if(GetCaseSensitiveSort())
		{

		}
		*/
		*PtrOutCharacter=0;
		Text(OutCharacter);
		PtrOutCharacter[1]=0;
	}

	/* </������ ����������> */

	if (!Fast && GetFocus())
	{
		if (PanelMode==PLUGIN_PANEL)
			CtrlObject->CmdLine->SetCurDir(Info.CurDir);
		else
			CtrlObject->CmdLine->SetCurDir(strCurDir);

		CtrlObject->CmdLine->Show();
	}

	int TitleX2=Opt.Clock && !Opt.ShowMenuBar ? Min(ScrX-4,X2):X2;
	int TruncSize=TitleX2-X1-3;

	if (!Opt.ShowColumnTitles && Opt.ShowSortMode && Filter && Filter->IsEnabledOnPanel())
		TruncSize-=2;

	GetTitle(strTitle,TruncSize,2);//,(PanelMode==PLUGIN_PANEL?0:2));
	Length=(int)strTitle.GetLength();
	int ClockCorrection=FALSE;

	if ((Opt.Clock && !Opt.ShowMenuBar) && TitleX2==ScrX-4)
	{
		ClockCorrection=TRUE;
		TitleX2+=4;
	}

	int TitleX=X1+(TitleX2-X1+1-Length)/2;

	if (ClockCorrection)
	{
		int Overlap=TitleX+Length-TitleX2+5;

		if (Overlap > 0)
			TitleX-=Overlap;
	}

	if (TitleX <= X1)
		TitleX = X1+1;

	SetColor(Focus ? COL_PANELSELECTEDTITLE:COL_PANELTITLE);
	GotoXY(TitleX,Y1);
	Text(strTitle);

	if (!FileCount)
	{
		SetScreen(X1+1,Y2-1,X2-1,Y2-1,L' ',ColorIndexToColor(COL_PANELTEXT));
		SetColor(COL_PANELTEXT); //???
		//GotoXY(X1+1,Y2-1);
		//FS<<fmt::Width(X2-X1-1)<<L"";
	}

	if (PanelMode==PLUGIN_PANEL && FileCount>0 && (Info.Flags & OPIF_REALNAMES))
	{
		if (!strInfoCurDir.IsEmpty())
		{
			strCurDir = strInfoCurDir;
		}
		else
		{
			if (!TestParentFolderName(ListData[CurFile]->strName))
			{
				strCurDir=ListData[CurFile]->strName;
				size_t pos;

				if (FindLastSlash(pos,strCurDir))
				{
					if (pos)
					{
						if (strCurDir.At(pos-1)!=L':')
							strCurDir.SetLength(pos);
						else
							strCurDir.SetLength(pos+1);
					}
				}
			}
			else
			{
				strCurDir = strOriginalCurDir;
			}
		}

		if (GetFocus())
		{
			CtrlObject->CmdLine->SetCurDir(strCurDir);
			CtrlObject->CmdLine->Show();
		}
	}

	if ((Opt.ShowPanelTotals || Opt.ShowPanelFree) &&
	        (Opt.ShowPanelStatus || !SelFileCount))
	{
		//Maximus5: ��� ������ ���� �������� �������������������� ��������� (0xCCCCCCCCC)
		ShowTotalSize(Info);
	}

	string strLastDir;
	if (GetMode()!=PLUGIN_PANEL && IsColumnDisplayed(CUSTOM_COLUMN0))
	{
		apiGetCurrentDirectory(strLastDir);
		apiSetCurrentDirectory(strCurDir);
	}

	ShowList(FALSE,0);
	ShowSelectedSize();

	if (!strLastDir.IsEmpty())
		apiSetCurrentDirectory(strLastDir);

	if (Opt.ShowPanelScrollbar)
	{
		SetColor(COL_PANELSCROLLBAR);
		ScrollBarEx(X2,Y1+1+Opt.ShowColumnTitles,Height,Round(CurTopFile,Columns),Round(FileCount,Columns));
	}

	ShowScreensCount();

	if (!ProcessingPluginCommand && LastCurFile!=CurFile)
	{
		LastCurFile=CurFile;
		UpdateViewPanel();
	}

	if (PanelMode==PLUGIN_PANEL)
		CtrlObject->Cp()->RedrawKeyBar();
}


const FarColor FileList::GetShowColor(int Position, int ColorType)
{
	FarColor ColorAttr=ColorIndexToColor(COL_PANELTEXT);
	const PaletteColors PalColor[] = {COL_PANELTEXT,COL_PANELSELECTEDTEXT,COL_PANELCURSOR,COL_PANELSELECTEDCURSOR};

	if (ListData && Position < FileCount)
	{
		int Pos = HIGHLIGHTCOLOR_NORMAL;

		if (CurFile==Position && Focus && FileCount > 0)
		{
			Pos=ListData[Position]->Selected?HIGHLIGHTCOLOR_SELECTEDUNDERCURSOR:HIGHLIGHTCOLOR_UNDERCURSOR;
		}
		else if (ListData[Position]->Selected)
			Pos = HIGHLIGHTCOLOR_SELECTED;

		ColorAttr=ListData[Position]->Colors.Color[ColorType][Pos];

		if (!(ColorAttr.ForegroundColor || ColorAttr.BackgroundColor) || !Opt.Highlight)
			ColorAttr=ColorIndexToColor(PalColor[Pos]);
	}

	return ColorAttr;
}

void FileList::SetShowColor(int Position, int ColorType)
{
	SetColor(GetShowColor(Position,ColorType));
}

void FileList::ShowSelectedSize()
{
	int Length;
	string strSelStr, strFormStr;

	if (Opt.ShowPanelStatus)
	{
		SetColor(COL_PANELBOX);
		int StatusHeight = GetPanelStatusHeight();
		DrawSeparator(Y2-StatusHeight);

		for (int I=0,ColumnPos=X1+1; I<ViewSettings.ColumnCount-1; I++)
		{
			if (ViewSettings.ColumnWidth[I]<0 ||
			        (I==ViewSettings.ColumnCount-2 && ViewSettings.ColumnWidth[I+1]<0))
				continue;

			ColumnPos+=ViewSettings.ColumnWidth[I];
			GotoXY(ColumnPos,Y2-StatusHeight);
			BoxText(BoxSymbols[BS_B_H1V1]);
			ColumnPos++;
		}
	}

	if (SelFileCount)
	{
		InsertCommas(SelFileSize,strFormStr);
		strSelStr.Format(MSG(__FormatEndSelectedPhrase(SelFileCount)),strFormStr.CPtr(),SelFileCount);
		TruncStr(strSelStr,X2-X1-1);
		Length=(int)strSelStr.GetLength();
		SetColor(COL_PANELSELECTEDINFO);
		GotoXY(X1+(X2-X1+1-Length)/2,Y2-GetPanelStatusHeight());
		Text(strSelStr);
	}
}

//Maximus5: ��� ������ ���� �������� �������������������� ��������� (0xCCCCCCCCC)
void FileList::ShowTotalSize(OpenPanelInfo &Info)
{
	if (!Opt.ShowPanelTotals && PanelMode==PLUGIN_PANEL && !(Info.Flags & OPIF_REALNAMES))
		return;

	string strTotalStr, strFormSize, strFreeSize;
	int Length;
	InsertCommas(TotalFileSize,strFormSize);

	if (Opt.ShowPanelFree && (PanelMode!=PLUGIN_PANEL || (Info.Flags & OPIF_REALNAMES)))
		InsertCommas(FreeDiskSize,strFreeSize);

	if (Opt.ShowPanelTotals)
	{
		if (!Opt.ShowPanelFree || strFreeSize.IsEmpty())
			strTotalStr.Format(MSG(__FormatEndSelectedPhrase(TotalFileCount)),strFormSize.CPtr(),TotalFileCount);
		else
		{
			wchar_t DHLine[4]={BoxSymbols[BS_H2],BoxSymbols[BS_H2],BoxSymbols[BS_H2],0};
			strTotalStr.Format(L" %s (%d) %s %s ",strFormSize.CPtr(),TotalFileCount,DHLine,strFreeSize.CPtr());

			if ((int)strTotalStr.GetLength()> X2-X1-1)
			{
				InsertCommas(FreeDiskSize>>20,strFreeSize);
				InsertCommas(TotalFileSize>>20,strFormSize);
				strTotalStr.Format(L" %s %s (%d) %s %s %s ",strFormSize.CPtr(),MSG(MListMb),TotalFileCount,DHLine,strFreeSize.CPtr(),MSG(MListMb));
			}
		}
	}
	else
		strTotalStr.Format(MSG(MListFreeSize), !strFreeSize.IsEmpty() ? strFreeSize.CPtr():L"???");

	SetColor(COL_PANELTOTALINFO);
	/* $ 01.08.2001 VVM
	  + �������� ������� ������, � �� ����� */
	TruncStrFromEnd(strTotalStr, X2-X1-1);
	Length=(int)strTotalStr.GetLength();
	GotoXY(X1+(X2-X1+1-Length)/2,Y2);
	const wchar_t *FirstBox=wcschr(strTotalStr,BoxSymbols[BS_H2]);
	int BoxPos=FirstBox ? (int)(FirstBox-strTotalStr.CPtr()):-1;
	int BoxLength=0;

	if (BoxPos!=-1)
		for (int I=0; strTotalStr.At(BoxPos+I)==BoxSymbols[BS_H2]; I++)
			BoxLength++;

	if (BoxPos==-1 || !BoxLength)
		Text(strTotalStr);
	else
	{
		FS<<fmt::Precision(BoxPos)<<strTotalStr;
		SetColor(COL_PANELBOX);
		FS<<fmt::Precision(BoxLength)<<strTotalStr.CPtr()+BoxPos;
		SetColor(COL_PANELTOTALINFO);
		Text(strTotalStr.CPtr()+BoxPos+BoxLength);
	}
}

int FileList::ConvertName(const wchar_t *SrcName,string &strDest,int MaxLength,int RightAlign,int ShowStatus,DWORD FileAttr)
{
	wchar_t *lpwszDest = strDest.GetBuffer(MaxLength+1);
	wmemset(lpwszDest,L' ',MaxLength);
	int SrcLength=StrLength(SrcName);

	if (RightAlign)
	{
		if (SrcLength>MaxLength)
			wmemcpy(lpwszDest,SrcName+SrcLength-MaxLength,MaxLength);
		else
			wmemcpy(lpwszDest+MaxLength-SrcLength,SrcName,SrcLength);
		strDest.ReleaseBuffer(MaxLength);
		return (SrcLength>MaxLength);
	}

	const wchar_t *DotPtr;

	if (!ShowStatus &&
	        ((!(FileAttr&FILE_ATTRIBUTE_DIRECTORY) && (ViewSettings.Flags&PVS_ALIGNEXTENSIONS))
	         || ((FileAttr&FILE_ATTRIBUTE_DIRECTORY) && (ViewSettings.Flags&PVS_FOLDERALIGNEXTENSIONS)))
	        && SrcLength<=MaxLength &&
	        (DotPtr=wcsrchr(SrcName,L'.')) && DotPtr!=SrcName &&
	        (SrcName[0]!=L'.' || SrcName[2]) && !wcschr(DotPtr+1,L' '))
	{
		int DotLength=StrLength(DotPtr+1);
		int NameLength=DotLength?(int)(DotPtr-SrcName):SrcLength;
		int DotPos=MaxLength-Max(DotLength,3);

		if (DotPos<=NameLength)
			DotPos=NameLength+1;

		if (DotPos>0 && NameLength>0 && SrcName[NameLength-1]==L' ')
			lpwszDest[NameLength]=L'.';

		wmemcpy(lpwszDest,SrcName,NameLength);
		wmemcpy(lpwszDest+DotPos,DotPtr+1,DotLength);
	}
	else
	{
		wmemcpy(lpwszDest,SrcName,Min(SrcLength, MaxLength));
	}

	strDest.ReleaseBuffer(MaxLength);
	return(SrcLength>MaxLength);
}


void FileList::PrepareViewSettings(int ViewMode,OpenPanelInfo *PlugInfo)
{
	OpenPanelInfo Info={0};

	if (PanelMode==PLUGIN_PANEL)
	{
		if (!PlugInfo)
			CtrlObject->Plugins.GetOpenPanelInfo(hPlugin,&Info);
		else
			Info=*PlugInfo;
	}

	ViewSettings=ViewSettingsArray[ViewMode];

	if (PanelMode==PLUGIN_PANEL)
	{
		if (Info.PanelModesArray && ViewMode<static_cast<int>(Info.PanelModesNumber) &&
		        Info.PanelModesArray[ViewMode].ColumnTypes &&
		        Info.PanelModesArray[ViewMode].ColumnWidths)
		{
			TextToViewSettings(Info.PanelModesArray[ViewMode].ColumnTypes,
			                   Info.PanelModesArray[ViewMode].ColumnWidths,
			                   false,ViewSettings.ColumnType,ViewSettings.ColumnWidth,
			                   ViewSettings.ColumnWidthType,ViewSettings.ColumnCount);

			if (Info.PanelModesArray[ViewMode].StatusColumnTypes &&
			        Info.PanelModesArray[ViewMode].StatusColumnWidths)
			{
				TextToViewSettings(Info.PanelModesArray[ViewMode].StatusColumnTypes,
				                   Info.PanelModesArray[ViewMode].StatusColumnWidths,
				                   true,ViewSettings.StatusColumnType,ViewSettings.StatusColumnWidth,
				                   ViewSettings.StatusColumnWidthType,ViewSettings.StatusColumnCount);
			}
			else if (Info.PanelModesArray[ViewMode].Flags&PMFLAGS_DETAILEDSTATUS)
			{
				ViewSettings.StatusColumnType[0]=COLUMN_RIGHTALIGN|NAME_COLUMN;
				ViewSettings.StatusColumnType[1]=SIZE_COLUMN;
				ViewSettings.StatusColumnType[2]=DATE_COLUMN;
				ViewSettings.StatusColumnType[3]=TIME_COLUMN;
				ViewSettings.StatusColumnWidth[0]=0;
				ViewSettings.StatusColumnWidth[1]=8;
				ViewSettings.StatusColumnWidth[2]=0;
				ViewSettings.StatusColumnWidth[3]=5;
				ViewSettings.StatusColumnCount=4;
			}
			else
			{
				ViewSettings.StatusColumnType[0]=COLUMN_RIGHTALIGN|NAME_COLUMN;
				ViewSettings.StatusColumnWidth[0]=0;
				ViewSettings.StatusColumnCount=1;
			}

			if (Info.PanelModesArray[ViewMode].Flags&PMFLAGS_FULLSCREEN)
				ViewSettings.Flags|=PVS_FULLSCREEN;
			else
				ViewSettings.Flags&=~PVS_FULLSCREEN;

			if (Info.PanelModesArray[ViewMode].Flags&PMFLAGS_ALIGNEXTENSIONS)
				ViewSettings.Flags|=PVS_ALIGNEXTENSIONS;
			else
				ViewSettings.Flags&=~PVS_ALIGNEXTENSIONS;

			if (!(Info.PanelModesArray[ViewMode].Flags&PMFLAGS_CASECONVERSION))
				ViewSettings.Flags&=~(PVS_FOLDERUPPERCASE|PVS_FILELOWERCASE|PVS_FILEUPPERTOLOWERCASE);
		}
		else
		{
			for (int I=0; I<ViewSettings.ColumnCount; I++)
			{
				if ((ViewSettings.ColumnType[I] & 0xff)==NAME_COLUMN)
				{
					if (Info.Flags & OPIF_SHOWNAMESONLY)
						ViewSettings.ColumnType[I]|=COLUMN_NAMEONLY;

					if (Info.Flags & OPIF_SHOWRIGHTALIGNNAMES)
						ViewSettings.ColumnType[I]|=COLUMN_RIGHTALIGN;

					if (Info.Flags & OPIF_SHOWPRESERVECASE)
						ViewSettings.Flags&=~(PVS_FOLDERUPPERCASE|PVS_FILELOWERCASE|PVS_FILEUPPERTOLOWERCASE);
				}
			}
		}
	}

	Columns=PreparePanelView(&ViewSettings);
	Height=Y2-Y1-(Opt.ShowPanelStatus ? (GetPanelStatusHeight()+2) : 2);

	if (!Opt.ShowColumnTitles)
		Height++;

	if (!Opt.ShowPanelStatus)
		Height+=GetPanelStatusHeight(); //Maximus5: BUGBUG: ???
}


int FileList::PreparePanelView(PanelViewSettings *PanelView)
{
	for (int S=0; S<PanelView->StatusColumnCount;)
	{
		int Columns=PanelView->StatusColumnCount-S;
		int NextLine=PanelView->StatusColumnCount+1;
		for (int I=S; I<PanelView->StatusColumnCount; I++)
		{
			if ((PanelView->StatusColumnType[I]&0xFF)==LINEBREAK_COLUMN)
			{
				Columns=I-S;
				NextLine=S+Columns+1;
				break;
			}
		}
		if (Columns>0)
			PrepareColumnWidths(PanelView->StatusColumnType+S,PanelView->StatusColumnWidth+S,
				PanelView->StatusColumnWidthType+S,Columns,(PanelView->Flags&PVS_FULLSCREEN)==PVS_FULLSCREEN,true);
		S=NextLine; //���������� �������+<BR>
	}
	return(PrepareColumnWidths(PanelView->ColumnType,PanelView->ColumnWidth,PanelView->ColumnWidthType,
	                           PanelView->ColumnCount,(PanelView->Flags&PVS_FULLSCREEN)==PVS_FULLSCREEN,false));
}


int FileList::PrepareColumnWidths(unsigned int *ColumnTypes, int *ColumnWidths, int *ColumnWidthsTypes, int &ColumnCount, bool FullScreen, bool StatusLine)
{
	int TotalWidth,TotalPercentWidth,TotalPercentCount,ZeroLengthCount,EmptyColumns,I;
	ZeroLengthCount=EmptyColumns=0;
	TotalWidth=ColumnCount-1;
	TotalPercentCount=TotalPercentWidth=0;

	for (I=0; I<ColumnCount; I++)
	{
		if ((ColumnTypes[I]&0xFF)==LINEBREAK_COLUMN)
		{
			ColumnCount=I;
			break;
		}
		if (ColumnWidths[I]<0)
		{
			EmptyColumns++;
			continue;
		}

		int ColumnType=ColumnTypes[I] & 0xff;

		if (!ColumnWidths[I])
		{
			ColumnWidthsTypes[I] = COUNT_WIDTH; //manage all zero-width columns in same way
			ColumnWidths[I]=ColumnTypeWidth[ColumnType];

			if (ColumnType==WDATE_COLUMN || ColumnType==CDATE_COLUMN || ColumnType==ADATE_COLUMN || ColumnType==CHDATE_COLUMN)
			{
				if (ColumnTypes[I] & COLUMN_BRIEF)
					ColumnWidths[I]-=3;

				if (ColumnTypes[I] & COLUMN_MONTH)
					ColumnWidths[I]++;
			}
		}

		if (!ColumnWidths[I])
			ZeroLengthCount++;

		switch (ColumnWidthsTypes[I])
		{
			case COUNT_WIDTH:
				TotalWidth+=ColumnWidths[I];
				break;
			case PERCENT_WIDTH:
				TotalPercentWidth+=ColumnWidths[I];
				TotalPercentCount++;
				break;
		}
	}

	TotalWidth-=EmptyColumns;
	int PanelTextWidth=X2-X1-1;
	//Maximus5: BUGBUG: ��� ���������� "-1" ��� ������!

	if (FullScreen)
		PanelTextWidth=ScrX-1;

	int ExtraWidth=PanelTextWidth-TotalWidth;

	if (TotalPercentCount>0)
	{
		int ExtraPercentWidth=(TotalPercentWidth>100 || !ZeroLengthCount)?ExtraWidth:ExtraWidth*TotalPercentWidth/100;
		int TempWidth=0;

		for (I=0; I<ColumnCount && TotalPercentCount>0; I++)
			if (ColumnWidthsTypes[I]==PERCENT_WIDTH)
			{
				int PercentWidth = (TotalPercentCount>1)?(ExtraPercentWidth*ColumnWidths[I]/TotalPercentWidth):(ExtraPercentWidth-TempWidth);

				if (PercentWidth<1)
					PercentWidth=1;

				TempWidth+=PercentWidth;
				ColumnWidths[I]=PercentWidth;
				ColumnWidthsTypes[I] = COUNT_WIDTH;
				TotalPercentCount--;
			}

		ExtraWidth-=TempWidth;
	}

	for (I=0; I<ColumnCount && ZeroLengthCount>0; I++)
		if (!ColumnWidths[I])
		{
			int AutoWidth=ExtraWidth/ZeroLengthCount;

			if (AutoWidth<1)
				AutoWidth=1;

			ColumnWidths[I]=AutoWidth;
			ExtraWidth-=AutoWidth;
			ZeroLengthCount--;
		}

	while (1)
	{
		int LastColumn=ColumnCount-1;
		TotalWidth=LastColumn-EmptyColumns;

		for (I=0; I<ColumnCount; I++)
			if (ColumnWidths[I]>0)
				TotalWidth+=ColumnWidths[I];

		if (TotalWidth<=PanelTextWidth)
			break;

		if (ColumnCount<=1)
		{
			ColumnWidths[0]=PanelTextWidth;
			break;
		}
		else if (PanelTextWidth>=TotalWidth-ColumnWidths[LastColumn])
		{
			ColumnWidths[LastColumn]=PanelTextWidth-(TotalWidth-ColumnWidths[LastColumn]);
			break;
		}
		else
			ColumnCount--;
	}

	ColumnsInGlobal = 1;
	int GlobalColumns=0;
	bool UnEqual;
	int Remainder;

	for (int i = 0; i < ViewSettings.ColumnCount; i++)
	{
		UnEqual = false;
		Remainder = ViewSettings.ColumnCount%ColumnsInGlobal;
		GlobalColumns = ViewSettings.ColumnCount/ColumnsInGlobal;

		if (!Remainder)
		{
			for (int k = 0; k < GlobalColumns-1; k++)
			{
				for (int j = 0; j < ColumnsInGlobal; j++)
				{
					if ((ViewSettings.ColumnType[k*ColumnsInGlobal+j] & 0xFF) !=
					        (ViewSettings.ColumnType[(k+1)*ColumnsInGlobal+j] & 0xFF))
						UnEqual = true;
				}
			}

			if (!UnEqual)
				break;
		}

		ColumnsInGlobal++;
	}

	return(GlobalColumns);
}


extern void GetColor(int PaletteIndex);

void FileList::ShowList(int ShowStatus,int StartColumn)
{
	string strDateStr, strTimeStr;
	int StatusShown=FALSE;
	int MaxLeftPos=0,MinLeftPos=FALSE;
	int ColumnCount=ShowStatus ? ViewSettings.StatusColumnCount:ViewSettings.ColumnCount;
	int StatusHeight=GetPanelStatusHeight();
	unsigned int *ColumnTypes=ShowStatus ? ViewSettings.StatusColumnType:ViewSettings.ColumnType;
	int *ColumnWidths=ShowStatus ? ViewSettings.StatusColumnWidth:ViewSettings.ColumnWidth;
	int K0=0;
	int StatusAlign=0;
	

	if (ShowStatus)
	{
		if ((ColumnTypes[K0]&0xFF)==LINEBREAK_COLUMN)
			K0++;
		// �������� ������ ������ ������� ������������� �� ������ ShowStatus (1 based)
		for (int S=1; S<ShowStatus; S++)
		{
			while (K0<ColumnCount)
			{
				if ((ColumnTypes[K0++]&0xFF)==LINEBREAK_COLUMN)
					break;
			}
		}
		if (K0 && (ColumnTypes[K0-1]&0xFF)==LINEBREAK_COLUMN)
			StatusAlign=ColumnTypes[K0-1] & (COLUMN_CENTERALIGN|COLUMN_RIGHTALIGN);
		// ��������� ���������� ������� � ���� ������
		int K1=K0;
		while (K1<ColumnCount && (ColumnTypes[K1]&0xFF)!=LINEBREAK_COLUMN)
			K1++;
		ColumnCount=K1-K0;
		if (K0)
		{
			ColumnTypes+=K0;
			ColumnWidths+=K0;
		}
	}

	for (int I=Y1+1+Opt.ShowColumnTitles,J=CurTopFile; I<Y2-StatusHeight; I++,J++)
	{
		int CurColumn=StartColumn;

		if (ShowStatus)
		{
			SetColor(COL_PANELTEXT);
			GotoXY(X1+1,Y2-StatusHeight+ShowStatus);
		}
		else
		{
			SetShowColor(J);
			GotoXY(X1+1,I);
		}

		int StatusLine=FALSE;
		int Level = 1;

		if (ShowStatus)
		{
			//Maximus: TODO: ��������� ���� N-�� ������ �������
		}
		
		FormatString strLine;
				
		for (int K=0; K<ColumnCount; K++)
		{
			int ListPos=J+CurColumn*Height;

			if (ShowStatus)
			{
				if (CurFile!=ListPos)
				{
					CurColumn++;
					continue;
				}
				else
					StatusLine=TRUE;
			}

			int CurX=WhereX();
			int CurY=WhereY();
			int ShowDivider=TRUE;
			int ColumnType=ColumnTypes[K] & 0xff;
			int ColumnWidth=ColumnWidths[K];

			if (ColumnWidth<0)
			{
				if (!ShowStatus && K==ColumnCount-1)
				{
					SetColor(COL_PANELBOX);
					GotoXY(CurX-1,CurY);
					BoxText(CurX-1==X2 ? BoxSymbols[BS_V2]:L' ');
				}

				continue;
			}

			if (ListPos<FileCount)
			{
				if (!ShowStatus && !StatusShown && CurFile==ListPos && Opt.ShowPanelStatus)
				{
					for (int S=1; S<StatusHeight; S++)
						ShowList(S,CurColumn);
					GotoXY(CurX,CurY);
					StatusShown=TRUE;
					SetShowColor(ListPos);
				}

				if (!ShowStatus)
					SetShowColor(ListPos);

				if (ColumnType>=CUSTOM_COLUMN0 && ColumnType<=CUSTOM_COLUMN9)
				{
					size_t ColumnNumber=ColumnType-CUSTOM_COLUMN0;
					const wchar_t *ColumnData=nullptr;

					if (ColumnNumber<ListData[ListPos]->CustomColumnNumber)
						ColumnData=ListData[ListPos]->CustomColumnData[ColumnNumber];

					if (!ColumnData)
					{
						if (PanelMode!=PLUGIN_PANEL && !ListData[ListPos]->CustomDataLoaded)
						{
							//Maximus: BUGBUG: ��������� ��������� ������� ����� ��� ������ (��� ����� ���� ��������� ������!)
							CtrlObject->Plugins.GetCustomData(ListData[ListPos]);
						}
						ColumnData=ListData[ListPos]->strCustomData;//L"";
					}

					int CurLeftPos=0;

					if (!ShowStatus && LeftPos>0)
					{
						int Length=StrLength(ColumnData);

						if (Length>ColumnWidth)
						{
							CurLeftPos=LeftPos;

							if (CurLeftPos>Length-ColumnWidth)
								CurLeftPos=Length-ColumnWidth;

							if (CurLeftPos>MaxLeftPos)
								MaxLeftPos=CurLeftPos;
						}
					}

					strLine<<fmt::LeftAlign()<<fmt::Width(ColumnWidth)<<fmt::Precision(ColumnWidth)<<ColumnData+CurLeftPos;
				}
				else
				{
					switch (ColumnType)
					{
						case NAME_COLUMN:
						{
							int Width=ColumnWidth;
							int ViewFlags=ColumnTypes[K];

							if ((ViewFlags & COLUMN_MARK) && Width>2)
							{
								const wchar_t *MarkPtr = ListData[ListPos]->Selected?L"\x221A ":L"  ";
								if (!ShowStatus)
									Text(MarkPtr);
								else
									strLine.Append(MarkPtr);
								Width-=2;
							}

							if (ListData[ListPos]->Colors.MarkChar && Opt.Highlight && Width>1)
							{
								Width--;
								OutCharacter[0]=(wchar_t)ListData[ListPos]->Colors.MarkChar;
								FarColor OldColor=GetColor();

								if (!ShowStatus)
								{
									SetShowColor(ListPos,HIGHLIGHTCOLORTYPE_MARKCHAR);
									Text(OutCharacter);
									SetColor(OldColor);
								}
								else
								{
									strLine.Append(OutCharacter);
								}
							}

							const wchar_t *NamePtr = ShowShortNames && !ListData[ListPos]->strShortName.IsEmpty() && !ShowStatus ? ListData[ListPos]->strShortName:ListData[ListPos]->strName;
							
							string strNameCopy;
							if (!(ListData[ListPos]->FileAttr & FILE_ATTRIBUTE_DIRECTORY) && (ViewFlags & COLUMN_NOEXTENSION))
							{
								const wchar_t *ExtPtr = PointToExt(NamePtr);
								if (ExtPtr)
								{
									strNameCopy.Copy(NamePtr, ExtPtr-NamePtr);
									NamePtr = strNameCopy;
								}
							}

							const wchar_t *NameCopy = NamePtr;

							if (ViewFlags & COLUMN_NAMEONLY)
							{
								//BUGBUG!!!
								// !!! �� ������, �� ��, ��� ������������ ������
								// ������������ ������ �������� - ����
								NamePtr=PointToFolderNameIfFolder(NamePtr);
							}

							int CurLeftPos=0;
							int RightAlign=(ViewFlags & COLUMN_RIGHTALIGN);
							int LeftBracket=FALSE,RightBracket=FALSE;

							if (!ShowStatus && LeftPos)
							{
								int Length = (int)wcslen(NamePtr);

								if (Length>Width)
								{
									if (LeftPos>0)
									{
										if (!RightAlign)
										{
											CurLeftPos=LeftPos;

											if (Length-CurLeftPos<Width)
												CurLeftPos=Length-Width;

											NamePtr += CurLeftPos;

											if (CurLeftPos>MaxLeftPos)
												MaxLeftPos=CurLeftPos;
										}
									}
									else if (RightAlign)
									{
										int CurRightPos=LeftPos;

										if (Length+CurRightPos<Width)
										{
											CurRightPos=Width-Length;
										}
										else
										{
											RightBracket=TRUE;
											LeftBracket=TRUE;
										}

										NamePtr += Length+CurRightPos-Width;
										RightAlign=FALSE;

										if (CurRightPos<MinLeftPos)
											MinLeftPos=CurRightPos;
									}
								}
							}

							string strName;
							int TooLong=ConvertName(NamePtr, strName, Width, RightAlign,ShowStatus,ListData[ListPos]->FileAttr);

							if (CurLeftPos)
								LeftBracket=TRUE;

							if (TooLong)
							{
								if (RightAlign)
									LeftBracket=TRUE;

								if (!RightAlign && StrLength(NamePtr)>Width)
									RightBracket=TRUE;
							}

							if (!ShowStatus)
							{
								if (ViewSettings.Flags&PVS_FILEUPPERTOLOWERCASE)
									if (!(ListData[ListPos]->FileAttr & FILE_ATTRIBUTE_DIRECTORY) && !IsCaseMixed(NameCopy))
										strName.Lower();

								if ((ViewSettings.Flags&PVS_FOLDERUPPERCASE) && (ListData[ListPos]->FileAttr & FILE_ATTRIBUTE_DIRECTORY))
									strName.Upper();

								if ((ViewSettings.Flags&PVS_FILELOWERCASE) && !(ListData[ListPos]->FileAttr & FILE_ATTRIBUTE_DIRECTORY))
									strName.Lower();
							}

							if (!ShowStatus)
								Text(strName);
							else
								strLine.Append(strName);

							if (!ShowStatus)
							{
								int NameX=WhereX();
								
								if (LeftBracket)
								{
									GotoXY(CurX-1,CurY);

									if (Level == 1)
										SetColor(COL_PANELBOX);

									Text(openBracket);
									SetShowColor(J);
								}

								if (RightBracket)
								{
									if (Level == ColumnsInGlobal)
										SetColor(COL_PANELBOX);

									GotoXY(NameX,CurY);
									Text(closeBracket);
									ShowDivider=FALSE;

									if (Level == ColumnsInGlobal)
										SetColor(COL_PANELTEXT);
									else
										SetShowColor(J);
								}
							}
						}
						break;
						case EXTENSION_COLUMN:
						{
							const wchar_t *ExtPtr = nullptr;
							if (!(ListData[ListPos]->FileAttr & FILE_ATTRIBUTE_DIRECTORY))
							{
								const wchar_t *NamePtr = ShowShortNames && !ListData[ListPos]->strShortName.IsEmpty() && !ShowStatus ? ListData[ListPos]->strShortName:ListData[ListPos]->strName;
								ExtPtr = PointToExt(NamePtr);
							}
							if (ExtPtr && *ExtPtr) ExtPtr++; else ExtPtr = L"";

							int ExtLen = (int)wcslen(ExtPtr);
							bool TooLong = ExtLen > ColumnWidth;

							strLine<<fmt::LeftAlign()<<fmt::Width(ColumnWidth)<<fmt::Precision(ColumnWidth)<<ExtPtr;

							if (!ShowStatus && TooLong)
							{
								Text(strLine);
								strLine.Clear();
								
								int NameX=WhereX();
								
								if (Level == ColumnsInGlobal)
									SetColor(COL_PANELBOX);

								GotoXY(NameX,CurY);
								Text(closeBracket);
								ShowDivider=FALSE;

								if (Level == ColumnsInGlobal)
									SetColor(COL_PANELTEXT);
								else
									SetShowColor(J);
							}
							break;
						}
						break;
						case SIZE_COLUMN:
						case PACKED_COLUMN:
						case STREAMSSIZE_COLUMN:
						{
                            strLine.Append(FormatStr_Size(
                            		ListData[ListPos]->UnpSize,
                            		ListData[ListPos]->PackSize,
                            		ListData[ListPos]->StreamsSize,
                            		ListData[ListPos]->strName,
                            		ListData[ListPos]->FileAttr,
                            		ListData[ListPos]->ShowFolderSize,
                            		ListData[ListPos]->ReparseTag,
                            		ColumnType,
                            		ColumnTypes[K],
                            		ColumnWidth).CPtr());
							break;
						}

						case DATE_COLUMN:
						case TIME_COLUMN:
						case WDATE_COLUMN:
						case CDATE_COLUMN:
						case ADATE_COLUMN:
						case CHDATE_COLUMN:
						{
							FILETIME *FileTime;

							switch (ColumnType)
							{
								case CDATE_COLUMN:
									FileTime=&ListData[ListPos]->CreationTime;
									break;
								case ADATE_COLUMN:
									FileTime=&ListData[ListPos]->AccessTime;
									break;
								case CHDATE_COLUMN:
									FileTime=&ListData[ListPos]->ChangeTime;
									break;
								case DATE_COLUMN:
								case TIME_COLUMN:
								case WDATE_COLUMN:
								default:
									FileTime=&ListData[ListPos]->WriteTime;
									break;
							}

							strLine<<FormatStr_DateTime(FileTime,ColumnType,ColumnTypes[K],ColumnWidth);
							break;
						}

						case ATTR_COLUMN:
						{
							strLine<<FormatStr_Attribute(ListData[ListPos]->FileAttr,ColumnWidth);
							break;
						}

						case DIZ_COLUMN:
						{
							int CurLeftPos=0;

							if (!ShowStatus && LeftPos>0)
							{
								int Length=ListData[ListPos]->DizText ? StrLength(ListData[ListPos]->DizText):0;

								if (Length>ColumnWidth)
								{
									CurLeftPos=LeftPos;

									if (CurLeftPos>Length-ColumnWidth)
										CurLeftPos=Length-ColumnWidth;

									if (CurLeftPos>MaxLeftPos)
										MaxLeftPos=CurLeftPos;
								}
							}

							string strDizText=ListData[ListPos]->DizText ? ListData[ListPos]->DizText+CurLeftPos:L"";
							size_t pos;

							if (strDizText.Pos(pos,L'\4'))
								strDizText.SetLength(pos);

							strLine<<fmt::LeftAlign()<<fmt::Width(ColumnWidth)<<fmt::Precision(ColumnWidth)<<strDizText;
							break;
						}

						case OWNER_COLUMN:
						{
							const wchar_t* Owner=ListData[ListPos]->strOwner;

							if (Owner && !(ColumnTypes[K]&COLUMN_FULLOWNER) && PanelMode!=PLUGIN_PANEL)
							{
								const wchar_t* SlashPos=FirstSlash(Owner);

								if (SlashPos)
									Owner=SlashPos+1;
							}
							else if(IsSlash(*Owner))
							{
								Owner++;
							}

							int CurLeftPos=0;

							if (!ShowStatus && LeftPos>0)
							{
								int Length=StrLength(Owner);

								if (Length>ColumnWidth)
								{
									CurLeftPos=LeftPos;

									if (CurLeftPos>Length-ColumnWidth)
										CurLeftPos=Length-ColumnWidth;

									if (CurLeftPos>MaxLeftPos)
										MaxLeftPos=CurLeftPos;
								}
							}

							strLine<<fmt::LeftAlign()<<fmt::Width(ColumnWidth)<<fmt::Precision(ColumnWidth)<<Owner+CurLeftPos;
							break;
						}

						case NUMLINK_COLUMN:
						{
							strLine<<fmt::Width(ColumnWidth)<<fmt::Precision(ColumnWidth)<<ListData[ListPos]->NumberOfLinks;
							break;
						}

						case NUMSTREAMS_COLUMN:
						{
							strLine<<fmt::Width(ColumnWidth)<<fmt::Precision(ColumnWidth)<<ListData[ListPos]->NumberOfStreams;
							break;
						}

					}
				}
			}
			else
			{
				strLine<<fmt::Width(ColumnWidth)<<L"";
			}
			
			if (!ShowStatus)
			{
		        Text(strLine);
		        strLine.Clear();
			}

			if (ShowDivider==FALSE)
				GotoXY(CurX+ColumnWidth+1,CurY);
			else
			{
				if (!ShowStatus)
				{
					SetShowColor(ListPos);

					if (Level == ColumnsInGlobal)
						SetColor(COL_PANELBOX);
				}

				if (K == ColumnCount-1)
					SetColor(COL_PANELBOX);

				GotoXY(CurX+ColumnWidth,CurY);

				if (K==ColumnCount-1)
					BoxText(CurX+ColumnWidth==X2 ? BoxSymbols[BS_V2]:L' ');
				else if (!ShowStatus)
					BoxText(BoxSymbols[BS_V1]);
				else
					strLine.Append(L" ");

				if (!ShowStatus)
					SetColor(COL_PANELTEXT);
			}

			if (!ShowStatus)
			{
				if (Level == ColumnsInGlobal)
				{
					Level = 0;
					CurColumn++;
				}

				Level++;
			}
		}
		
		if (ShowStatus && !strLine.IsEmpty())
		{
			GotoXY(X1+1,WhereY());
			SetColor(COL_PANELTEXT);
	        if (StatusAlign & COLUMN_CENTERALIGN)
		        RemoveExternalSpaces(strLine);
			else if (StatusAlign & COLUMN_RIGHTALIGN)
				RemoveTrailingSpaces(strLine);
			int LineLen = static_cast<int>(strLine.GetLength());
			if (LineLen<(X2-X1-1))
			{
				if (StatusAlign & COLUMN_CENTERALIGN)
					FS<<fmt::Width((X2-X1-1-LineLen)>>1)<<L"";
				else if (StatusAlign & COLUMN_RIGHTALIGN)
					FS<<fmt::Width(X2-X1-1-LineLen)<<L"";
			}
			else if (LineLen>(X2-X1-1))
			{
				LineLen = X2-X1-1;
			}
	        FS<<fmt::Width(LineLen)<<fmt::Precision(LineLen)<<strLine/*LinePtr*/;
        }

		if ((!ShowStatus || StatusLine) && WhereX()<X2)
		{
			SetColor(COL_PANELTEXT);
			FS<<fmt::Width(X2-WhereX())<<L"";
		}
	}

	if (!ShowStatus && !StatusShown && Opt.ShowPanelStatus)
	{
		SetScreen(X1+1,Y2-1,X2-1,Y2-1,L' ',ColorIndexToColor(COL_PANELTEXT));
		SetColor(COL_PANELTEXT); //???
		//GotoXY(X1+1,Y2-1);
		//FS<<fmt::Width(X2-X1-1)<<L"";
	}

	if (!ShowStatus)
	{
		if (LeftPos<0)
			LeftPos=MinLeftPos;

		if (LeftPos>0)
			LeftPos=MaxLeftPos;
	}
}


bool FileList::IsFullScreen()
{
	return (this->ViewSettings.Flags&PVS_FULLSCREEN)==PVS_FULLSCREEN;
}


bool FileList::IsModeFullScreen(int Mode)
{
	return (ViewSettingsArray[Mode].Flags&PVS_FULLSCREEN)==PVS_FULLSCREEN;
}


int FileList::IsDizDisplayed()
{
	return IsColumnDisplayed(DIZ_COLUMN);
}


int FileList::IsColumnDisplayed(int Type)
{

	for (int i=0; i<ViewSettings.ColumnCount; i++)
		if ((int)(ViewSettings.ColumnType[i] & 0xff)==Type)
			return TRUE;

	for (int i=0; i<ViewSettings.StatusColumnCount; i++)
		if ((int)(ViewSettings.StatusColumnType[i] & 0xff)==Type)
			return TRUE;

	return FALSE;
}

int FileList::GetPanelStatusHeight()
{
	if (!Opt.ShowPanelStatus)
		return 0;
	int Count=0;
	int CountMax = Y2-Y1-5;
	for (int I=0; I<ViewSettings.StatusColumnCount && Count<CountMax; I++)
	{
		if (I && (ViewSettings.StatusColumnType[I]&0xFF)==LINEBREAK_COLUMN)
		{
			Count++;
		}
	}
	return Count+2; // <Number of status lines> + 1 (delimiter line)
}

void FileList::ClearCustomData()
{
	if (GetType()==FILE_PANEL && GetMode()==NORMAL_PANEL && ListData)
	{
		for (int i=0; i < FileCount; i++)
		{
			ListData[i]->ClearCustomData();
		}
	}
}
