/*
copy.cpp

����������� ������
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
#include "lang.hpp"
#include "keys.hpp"
#include "colors.hpp"
#include "flink.hpp"
#include "dialog.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "filelist.hpp"
#include "foldtree.hpp"
#include "treelist.hpp"
#include "chgprior.hpp"
#include "scantree.hpp"
#include "constitle.hpp"
#include "filefilter.hpp"
#include "imports.hpp"
#include "fileview.hpp"
#include "TPreRedrawFunc.hpp"
#include "syslog.hpp"
#include "TaskBar.hpp"
#include "cddrv.hpp"
#include "interf.hpp"
#include "keyboard.hpp"
#include "palette.hpp"
#include "message.hpp"
#include "config.hpp"
#include "stddlg.hpp"
#include "fileattr.hpp"
#include "datetime.hpp"
#include "dirinfo.hpp"
#include "pathmix.hpp"
#include "drivemix.hpp"
#include "dirmix.hpp"
#include "strmix.hpp"
#include "panelmix.hpp"
#include "processname.hpp"
#include "mix.hpp"
#include "DlgGuid.hpp"
#include "console.hpp"
#include "wakeful.hpp"
#include "configdb.hpp"

/* ����� ����� �������� ������������ */
extern long WaitUserTime;
/* ��� ����, ��� �� ����� ��� �������� ������������ ������, � remaining/speed ��� */
static long OldCalcTime;

#define SDDATA_SIZE   64*1024

enum {COPY_BUFFER_SIZE  = 0x10000};

enum
{
	COPY_RULE_NUL    = 0x0001,
	COPY_RULE_FILES  = 0x0002,
};

enum COPYSECURITYOPTIONS
{
	CSO_MOVE_SETCOPYSECURITY       = 0x00000001,  // Move: �� ��������� ���������� ����� "Copy access rights"?
	CSO_MOVE_SETINHERITSECURITY    = 0x00000003,  // Move: �� ��������� ���������� ����� "Inherit access rights"?
	CSO_MOVE_SESSIONSECURITY       = 0x00000004,  // Move: ��������� ��������� "access rights" ������ ������?
	CSO_COPY_SETCOPYSECURITY       = 0x00000008,  // Copy: �� ��������� ���������� ����� "Copy access rights"?
	CSO_COPY_SETINHERITSECURITY    = 0x00000018,  // Copy: �� ��������� ���������� ����� "Inherit access rights"?
	CSO_COPY_SESSIONSECURITY       = 0x00000020,  // Copy: ��������� ��������� "access rights" ������ ������?
};


size_t TotalFiles,TotalFilesToProcess;

static clock_t CopyStartTime;

static int OrigScrX,OrigScrY;

static DWORD WINAPI CopyProgressRoutine(LARGE_INTEGER TotalFileSize,
                                        LARGE_INTEGER TotalBytesTransferred,LARGE_INTEGER StreamSize,
                                        LARGE_INTEGER StreamBytesTransferred,DWORD dwStreamNumber,
                                        DWORD dwCallbackReason,HANDLE /*hSourceFile*/,HANDLE /*hDestinationFile*/,
                                        LPVOID lpData);

static unsigned __int64 TotalCopySize, TotalCopiedSize; // ����� ��������� �����������
static unsigned __int64 CurCopiedSize;                  // ������� ��������� �����������
static unsigned __int64 TotalSkippedSize;               // ����� ������ ����������� ������
static unsigned __int64 TotalCopiedSizeEx;
static size_t   CountTarget;                    // ����� �����.
static int CopySecurityCopy=-1;
static int CopySecurityMove=-1;
static bool ShowTotalCopySize;
static string strTotalCopySizeText;

static FileFilter *Filter;
static int UseFilter=FALSE;

static BOOL ZoomedState,IconicState;

struct CopyDlgParam
{
	string strPluginFormat;
	ShellCopy *thisClass;
	int AltF10;
	int CopySecurity;
	int SelCount;
	DWORD FileAttr;
	bool FolderPresent;
	bool FilesPresent;
	bool AskRO;
};

enum enumShellCopy
{
	ID_SC_TITLE,
	ID_SC_TARGETTITLE,
	ID_SC_TARGETEDIT,
	ID_SC_SEPARATOR1,
	ID_SC_ACTITLE,
	ID_SC_ACLEAVE,
	ID_SC_ACCOPY,
	ID_SC_ACINHERIT,
	ID_SC_SEPARATOR2,
	ID_SC_COMBOTEXT,
	ID_SC_COMBO,
	ID_SC_COPYSYMLINK,
	ID_SC_MULTITARGET,
	ID_SC_SEPARATOR3,
	ID_SC_USEFILTER,
	ID_SC_SEPARATOR4,
	ID_SC_BTNCOPY,
	ID_SC_BTNTREE,
	ID_SC_BTNFILTER,
	ID_SC_BTNCANCEL,
	ID_SC_SOURCEFILENAME,
};

enum CopyMode
{
	CM_ASK,
	CM_OVERWRITE,
	CM_SKIP,
	CM_RENAME,
	CM_APPEND,
	CM_ONLYNEWER,
	CM_SEPARATOR,
	CM_ASKRO,
};

// CopyProgress start
// ����� ��� ������ � ��������� ���� ����� ������� ���� ���������� ���������� ������
class CopyProgress
{
		ConsoleTitle CopyTitle;
		TaskBar TB;
		wakeful W;
		SMALL_RECT Rect;
		wchar_t Bar[100];
		size_t BarSize;
		bool Move,Total,Time;
		bool BgInit,ScanBgInit;
		bool IsCancelled;
		FarColor Color;
		int Percents;
		DWORD LastWriteTime;
		string strSrc,strDst,strFiles,strTime;
		bool Timer();
		void Flush();
		void DrawNames();
		void CreateScanBackground();
		void SetProgress(bool TotalProgress,UINT64 CompletedSize,UINT64 TotalSize);
	public:
		CopyProgress(bool Move,bool Total,bool Time);
		void CreateBackground();
		bool Cancelled() {return IsCancelled;};
		void SetScanName(const wchar_t *Name);
		void SetNames(const wchar_t *Src,const wchar_t *Dst);
		void SetProgressValue(UINT64 CompletedSize,UINT64 TotalSize) {return SetProgress(false,CompletedSize,TotalSize);}
		void SetTotalProgressValue(UINT64 CompletedSize,UINT64 TotalSize) {return SetProgress(true,CompletedSize,TotalSize);}
};

static void GetTimeText(DWORD Time,string &strTimeText)
{
	DWORD Sec=Time;
	DWORD Min=Sec/60;
	Sec-=(Min*60);
	DWORD Hour=Min/60;
	Min-=(Hour*60);
	strTimeText.Format(L"%02u:%02u:%02u",Hour,Min,Sec);
}

bool CopyProgress::Timer()
{
	bool Result=false;
	DWORD Time=GetTickCount();

	if (!LastWriteTime||(Time-LastWriteTime>=RedrawTimeout))
	{
		LastWriteTime=Time;
		Result=true;
	}

	return Result;
}

void CopyProgress::Flush()
{
	if (Timer())
	{
		if (!IsCancelled)
		{
			if (CheckForEscSilent())
			{
				(*FrameManager)[0]->Lock();
				IsCancelled=ConfirmAbortOp()!=0;
				(*FrameManager)[0]->Unlock();
			}
		}

		if (Total || (TotalFilesToProcess==1))
		{
			CopyTitle << L"{" << (Total?ToPercent64(TotalCopiedSize>>8,TotalCopySize>>8):Percents) << L"%} " << MSG(Move? MCopyMovingTitle : MCopyCopyingTitle) << fmt::Flush();
		}
	}
}

CopyProgress::CopyProgress(bool Move,bool Total,bool Time):
	BarSize(52),
	Move(Move),
	Total(Total),
	Time(Time),
	BgInit(false),
	ScanBgInit(false),
	IsCancelled(false),
	Color(ColorIndexToColor(COL_DIALOGTEXT)),
	Percents(0),
	LastWriteTime(0)
{
}

void CopyProgress::SetScanName(const wchar_t *Name)
{
	if (!ScanBgInit)
	{
		CreateScanBackground();
	}

	GotoXY(Rect.Left+5,Rect.Top+3);
	FS<<fmt::LeftAlign()<<fmt::Width(Rect.Right-Rect.Left-9)<<fmt::Precision(Rect.Right-Rect.Left-9)<<Name;
	Flush();
}

void CopyProgress::CreateScanBackground()
{
	for (size_t i=0; i<BarSize; i++)
	{
		Bar[i]=L' ';
	}

	Bar[BarSize]=0;
	Message(MSG_LEFTALIGN,0,MSG(Move?MMoveDlgTitle:MCopyDlgTitle),MSG(MCopyScanning),Bar);
	int MX1,MY1,MX2,MY2;
	GetMessagePosition(MX1,MY1,MX2,MY2);
	Rect.Left=MX1;
	Rect.Right=MX2;
	Rect.Top=MY1;
	Rect.Bottom=MY2;
	ScanBgInit=true;
}

void CopyProgress::CreateBackground()
{
	for (size_t i=0; i<BarSize; i++)
	{
		Bar[i]=L' ';
	}

	Bar[BarSize]=0;

	if (!Total)
	{
		if (!Time)
		{
			Message(MSG_LEFTALIGN,0,MSG(Move?MMoveDlgTitle:MCopyDlgTitle),MSG(Move?MCopyMoving:MCopyCopying),L"",MSG(MCopyTo),L"",Bar,L"\x1",L"");
		}
		else
		{
			Message(MSG_LEFTALIGN,0,MSG(Move?MMoveDlgTitle:MCopyDlgTitle),MSG(Move?MCopyMoving:MCopyCopying),L"",MSG(MCopyTo),L"",Bar,L"\x1",L"",L"\x1",L"");
		}
	}
	else
	{
		string strTotalSeparator(L"\x1 ");
		strTotalSeparator+=MSG(MCopyDlgTotal);
		strTotalSeparator+=L": ";
		strTotalSeparator+=strTotalCopySizeText;
		strTotalSeparator+=L" ";

		if (!Time)
		{
			Message(MSG_LEFTALIGN,0,MSG(Move?MMoveDlgTitle:MCopyDlgTitle),MSG(Move?MCopyMoving:MCopyCopying),L"",MSG(MCopyTo),L"",Bar,strTotalSeparator,Bar,L"\x1",L"");
		}
		else
		{
			Message(MSG_LEFTALIGN,0,MSG(Move?MMoveDlgTitle:MCopyDlgTitle),MSG(Move?MCopyMoving:MCopyCopying),L"",MSG(MCopyTo),L"",Bar,strTotalSeparator,Bar,L"\x1",L"",L"\x1",L"");
		}
	}

	int MX1,MY1,MX2,MY2;
	GetMessagePosition(MX1,MY1,MX2,MY2);
	Rect.Left=MX1;
	Rect.Right=MX2;
	Rect.Top=MY1;
	Rect.Bottom=MY2;
	BgInit=true;
	DrawNames();
}

void CopyProgress::DrawNames()
{
	Text(Rect.Left+5,Rect.Top+3,Color,strSrc);
	Text(Rect.Left+5,Rect.Top+5,Color,strDst);
	Text(Rect.Left+5,Rect.Top+(Total?10:8),Color,strFiles);
}

void CopyProgress::SetNames(const wchar_t *Src,const wchar_t *Dst)
{
	if (!BgInit)
	{
		CreateBackground();
	}

	FormatString FString;
	FString<<fmt::LeftAlign()<<fmt::Width(Rect.Right-Rect.Left-9)<<fmt::Precision(Rect.Right-Rect.Left-9)<<Src;
	strSrc=FString;
	FString.Clear();
	FString<<fmt::LeftAlign()<<fmt::Width(Rect.Right-Rect.Left-9)<<fmt::Precision(Rect.Right-Rect.Left-9)<<Dst;
	strDst=FString;

	if (Total)
	{
		strFiles.Format(MSG(MCopyProcessedTotal),TotalFiles,TotalFilesToProcess);
	}
	else
	{
		strFiles.Format(MSG(MCopyProcessed),TotalFiles);
	}

	DrawNames();
	Flush();
}

void CopyProgress::SetProgress(bool TotalProgress,UINT64 CompletedSize,UINT64 TotalSize)
{
	if (!BgInit)
	{
		CreateBackground();
	}

	if (Total==TotalProgress)
	{
		TBC.SetProgressValue(CompletedSize,TotalSize);
	}

	UINT64 OldCompletedSize = CompletedSize;
	UINT64 OldTotalSize = TotalSize;
	CompletedSize>>=8;
	TotalSize>>=8;
	CompletedSize=Min(CompletedSize,TotalSize);
	COORD BarCoord={static_cast<SHORT>(Rect.Left+5),static_cast<SHORT>(Rect.Top+(TotalProgress?8:6))};
	size_t BarLength=Rect.Right-Rect.Left-9-5; //-5 ��� ���������
	size_t Length=TotalSize?static_cast<size_t>((TotalSize<1000000?CompletedSize:CompletedSize/100)*BarLength/(TotalSize<1000000?TotalSize:TotalSize/100)):BarLength;

	for (size_t i=0; i<BarLength; i++)
	{
		Bar[i]=BoxSymbols[BS_X_B0];
	}

	if (TotalSize)
	{
		for (size_t i=0; i<Length; i++)
		{
			Bar[i]=BoxSymbols[BS_X_DB];
		}
	}

	Bar[BarLength]=0;
	Percents=ToPercent64(CompletedSize,TotalSize);
	FormatString strPercents;
	strPercents<<fmt::Width(4)<<Percents<<L"%";
	Text(BarCoord.X,BarCoord.Y,Color,Bar);
	Text(static_cast<int>(BarCoord.X+BarLength),BarCoord.Y,Color,strPercents);

	if (Time&&(!Total||TotalProgress))
	{
		DWORD WorkTime=clock()-CopyStartTime;
		UINT64 SizeLeft=(OldTotalSize>OldCompletedSize)?(OldTotalSize-OldCompletedSize):0;
		long CalcTime=OldCalcTime;

		if (WaitUserTime!=-1) // -1 => ��������� � �������� �������� ������ �����
		{
			OldCalcTime=CalcTime=WorkTime-WaitUserTime;
		}

		WorkTime/=1000;
		CalcTime/=1000;

		if (!WorkTime)
		{
			strTime.Format(MSG(MCopyTimeInfo),L" ",L" ",L" ");
		}
		else
		{
			if (TotalProgress)
			{
				OldCompletedSize=OldCompletedSize-TotalSkippedSize;
			}

			UINT64 CPS=CalcTime?OldCompletedSize/CalcTime:0;
			DWORD TimeLeft=static_cast<DWORD>(CPS?SizeLeft/CPS:0);
			string strSpeed;
			FileSizeToStr(strSpeed,CPS,8,COLUMN_FLOATSIZE|COLUMN_COMMAS);
			string strWorkTimeStr,strTimeLeftStr;
			GetTimeText(WorkTime,strWorkTimeStr);
			GetTimeText(TimeLeft,strTimeLeftStr);
			if(strSpeed.At(0)==L' ' && strSpeed.At(strSpeed.GetLength()-1)>=L'0' && strSpeed.At(strSpeed.GetLength()-1)<=L'9')
			{
				strSpeed.LShift(1);
				strSpeed+=L" ";
			}
			strTime.Format(MSG(MCopyTimeInfo),strWorkTimeStr.CPtr(),strTimeLeftStr.CPtr(),strSpeed.CPtr());
		}

		Text(Rect.Left+5,Rect.Top+(Total?12:10),Color,strTime);
	}

	Flush();
}
// CopyProgress end

CopyProgress *CP;


/* $ 25.05.2002 IS
 + ������ �������� � ��������� _��������_ �������, � ���������� ����
   ������������� ��������, �����
   Src="D:\Program Files\filename"
   Dest="D:\PROGRA~1\filename"
   ("D:\PROGRA~1" - �������� ��� ��� "D:\Program Files")
   ���������, ��� ����� ���� ����������, � ������ ���������,
   ��� ��� ������ (������� �� �����, ��� � � ������, � �� ������ ������
   ���� ���� � ��� ��)
 ! ����������� - "���������" ������� �� DeleteEndSlash
 ! ������� ��� ���������������� �� �������� ���� � ������
   ��������� �� ������� �����, ������ ��� ��� ����� ������ ������ ���
   ��������������, � ������� ���������� � ��� ����������� ����.
   ������ ������� ������ 1, ��� ������ ���� src=path\filename,
   dest=path\filename (������ ���������� 2 - �.�. ������ �� ������).
*/

int CmpFullNames(const wchar_t *Src,const wchar_t *Dest)
{
	string strSrcFullName, strDestFullName;

	// ������� ������ ���� � ������ ������������� ������
	ConvertNameToReal(Src, strSrcFullName);
	ConvertNameToReal(Dest, strDestFullName);
	DeleteEndSlash(strSrcFullName);
	DeleteEndSlash(strDestFullName);

	// ��������� �� �������� ����
	ConvertNameToLong(strSrcFullName, strSrcFullName);
	ConvertNameToLong(strDestFullName, strDestFullName);

	return !StrCmpI(strSrcFullName,strDestFullName);
}

bool CheckNulOrCon(const wchar_t *Src)
{
	if (HasPathPrefix(Src))
		Src+=4;

	return (!StrCmpNI(Src,L"nul",3) || !StrCmpNI(Src,L"con",3)) && (IsSlash(Src[3])||!Src[3]);
}

string& GetParentFolder(const wchar_t *Src, string &strDest)
{
	ConvertNameToReal(Src,strDest);
	CutToSlash(strDest,true);
	return strDest;
}

int CmpFullPath(const wchar_t *Src, const wchar_t *Dest)
{
	string strSrcFullName, strDestFullName;

	GetParentFolder(Src, strSrcFullName);
	GetParentFolder(Dest, strDestFullName);
	DeleteEndSlash(strSrcFullName);
	DeleteEndSlash(strDestFullName);

	// ��������� �� �������� ����
	if (IsLocalPath(strSrcFullName))
		ConvertNameToLong(strSrcFullName, strSrcFullName);

	if (IsLocalPath(strDestFullName))
		ConvertNameToLong(strDestFullName, strDestFullName);

	return !StrCmpI(strSrcFullName, strDestFullName);
}

static void GenerateName(string &strName,const wchar_t *Path=nullptr)
{
	if (Path&&*Path)
	{
		string strTmp=Path;
		AddEndSlash(strTmp);
		strTmp+=PointToName(strName);
		strName=strTmp;
	}

	string strExt=PointToExt(strName);
	size_t NameLength=strName.GetLength()-strExt.GetLength();

	for (int i=1; apiGetFileAttributes(strName)!=INVALID_FILE_ATTRIBUTES; i++)
	{
		WCHAR Suffix[20]=L"_";
		_itow(i,Suffix+1,10);
		strName.SetLength(NameLength);
		strName+=Suffix;
		strName+=strExt;
	}
}

void PR_ShellCopyMsg()
{
	PreRedrawItem preRedrawItem=PreRedraw.Peek();

	if (preRedrawItem.Param.Param1)
	{
		((CopyProgress*)preRedrawItem.Param.Param1)->CreateBackground();
	}
}

BOOL CheckAndUpdateConsole(BOOL IsChangeConsole)
{
	HWND hWnd = Console.GetWindow();
	BOOL curZoomedState=IsZoomed(hWnd);
	BOOL curIconicState=IsIconic(hWnd);

	if (ZoomedState!=curZoomedState && IconicState==curIconicState)
	{
		ZoomedState=curZoomedState;
		ChangeVideoMode(ZoomedState);
		Frame *frame=FrameManager->GetBottomFrame();
		int LockCount=-1;

		while (frame->Locked())
		{
			LockCount++;
			frame->Unlock();
		}

		FrameManager->ResizeAllFrame();
		FrameManager->PluginCommit();

		while (LockCount > 0)
		{
			frame->Lock();
			LockCount--;
		}

		IsChangeConsole=TRUE;
	}

	return IsChangeConsole;
}

enum
{
	DM_CALLTREE = DM_USER+1,
	DM_SWITCHRO = DM_USER+2,
};

INT_PTR WINAPI CopyDlgProc(HANDLE hDlg,int Msg,int Param1,void* Param2)
{

	CopyDlgParam *DlgParam=(CopyDlgParam *)SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);

	switch (Msg)
	{
		case DN_INITDIALOG:
			SendDlgMessage(hDlg,DM_SETCOMBOBOXEVENT,ID_SC_COMBO,ToPtr(CBET_KEY|CBET_MOUSE));
			SendDlgMessage(hDlg,DM_SETMOUSEEVENTNOTIFY,TRUE,0);
			break;
		case DM_SWITCHRO:
		{
			FarListGetItem LGI={CM_ASKRO};
			SendDlgMessage(hDlg,DM_LISTGETITEM,ID_SC_COMBO,&LGI);

			if (LGI.Item.Flags&LIF_CHECKED)
				LGI.Item.Flags&=~LIF_CHECKED;
			else
				LGI.Item.Flags|=LIF_CHECKED;

			SendDlgMessage(hDlg,DM_LISTUPDATE,ID_SC_COMBO,&LGI);
			SendDlgMessage(hDlg,DM_REDRAW,0,0);
			return TRUE;
		}
		case DN_BTNCLICK:
		{
			if (Param1==ID_SC_USEFILTER) // "Use filter"
			{
				UseFilter=static_cast<int>(reinterpret_cast<INT_PTR>(Param2));
				return TRUE;
			}

			if (Param1 == ID_SC_BTNTREE) // Tree
			{
				SendDlgMessage(hDlg,DM_CALLTREE,0,0);
				return FALSE;
			}
			else if (Param1 == ID_SC_BTNCOPY)
			{
				SendDlgMessage(hDlg,DM_CLOSE,ID_SC_BTNCOPY,0);
			}
			/*
			else if(Param1 == ID_SC_ONLYNEWER && ((DlgParam->thisClass->Flags)&FCOPY_LINK))
			{
			  // ����������� ��� ����� �������� ������������ � ������ ����� :-))
			  		SendDlgMessage(hDlg,DN_EDITCHANGE,ID_SC_TARGETEDIT,0);
			}
			*/
			else if (Param1==ID_SC_BTNFILTER) // Filter
			{
				Filter->FilterEdit();
				return TRUE;
			}

			break;
		}
		case DN_CONTROLINPUT: // �� ������ ������!
		{
			const INPUT_RECORD* record=(const INPUT_RECORD *)Param2;
			if (record->EventType==KEY_EVENT)
			{
				int key = InputRecordToKey(record);
				if (key == KEY_ALTF10 || key == KEY_RALTF10 || key == KEY_F10 || key == KEY_SHIFTF10)
				{
					DlgParam->AltF10=(key == KEY_ALTF10 || key == KEY_RALTF10)?1:(key == KEY_SHIFTF10?2:0);
					SendDlgMessage(hDlg,DM_CALLTREE,DlgParam->AltF10,0);
					return TRUE;
				}

				if (Param1 == ID_SC_COMBO)
				{
					if (key==KEY_ENTER || key==KEY_NUMENTER || key==KEY_INS || key==KEY_NUMPAD0 || key==KEY_SPACE)
					{
						if (SendDlgMessage(hDlg,DM_LISTGETCURPOS,ID_SC_COMBO,0)==CM_ASKRO)
							return SendDlgMessage(hDlg,DM_SWITCHRO,0,0);
					}
				}
			}
		}
		break;

		case DN_LISTHOTKEY:
			if(Param1==ID_SC_COMBO)
			{
				if (SendDlgMessage(hDlg,DM_LISTGETCURPOS,ID_SC_COMBO,0)==CM_ASKRO)
				{
					SendDlgMessage(hDlg,DM_SWITCHRO,0,0);
					return TRUE;
				}
			}
			break;
		case DN_INPUT:

			if (SendDlgMessage(hDlg,DM_GETDROPDOWNOPENED,ID_SC_COMBO,0))
			{
				MOUSE_EVENT_RECORD *mer=&reinterpret_cast<INPUT_RECORD*>(Param2)->Event.MouseEvent;

				if (SendDlgMessage(hDlg,DM_LISTGETCURPOS,ID_SC_COMBO,0)==CM_ASKRO && mer->dwButtonState && !(mer->dwEventFlags&MOUSE_MOVED))
				{
					SendDlgMessage(hDlg,DM_SWITCHRO,0,0);
					return FALSE;
				}
			}

			break;
		case DM_CALLTREE:
		{
			/* $ 13.10.2001 IS
			   + ��� ����������������� ��������� ��������� � "������" ������� � ���
			     ������������� ������ ����� ����� � �������.
			   - ���: ��� ����������������� ��������� � "������" ������� ��
			     ���������� � �������, ���� �� �������� � �����
			     ����� �������-�����������.
			   - ���: ����������� �������� Shift-F10, ���� ������ ����� ���������
			     ���� �� �����.
			   - ���: ����������� �������� Shift-F10 ��� ����������������� -
			     ����������� �������� �������, ������ ������������ ����� ������ �������
			     � ������.
			*/
			BOOL MultiCopy=SendDlgMessage(hDlg,DM_GETCHECK,ID_SC_MULTITARGET,0)==BSTATE_CHECKED;
			string strOldFolder;
			int nLength;
			FarDialogItemData Data;
			nLength = (int)SendDlgMessage(hDlg, DM_GETTEXTLENGTH, ID_SC_TARGETEDIT, 0);
			Data.PtrData = strOldFolder.GetBuffer(nLength+1);
			Data.PtrLength = nLength;
			SendDlgMessage(hDlg,DM_GETTEXT,ID_SC_TARGETEDIT,&Data);
			strOldFolder.ReleaseBuffer();
			string strNewFolder;

			if (DlgParam->AltF10 == 2)
			{
				strNewFolder = strOldFolder;

				if (MultiCopy)
				{
					UserDefinedList DestList(0,0,ULF_UNIQUE);

					if (DestList.Set(strOldFolder))
					{
						DestList.Reset();
						const wchar_t *NamePtr=DestList.GetNext();

						if (NamePtr)
							strNewFolder = NamePtr;
					}
				}

				if (strNewFolder.IsEmpty())
					DlgParam->AltF10=-1;
				else // ������� ������ ����
					DeleteEndSlash(strNewFolder);
			}

			if (DlgParam->AltF10 != -1)
			{
				{
					string strNewFolder2;
					FolderTree Tree(strNewFolder2,
					                (DlgParam->AltF10==1?MODALTREE_PASSIVE:
					                 (DlgParam->AltF10==2?MODALTREE_FREE:
					                  MODALTREE_ACTIVE)),
					                FALSE,FALSE);
					strNewFolder = strNewFolder2;
				}

				if (!strNewFolder.IsEmpty())
				{
					AddEndSlash(strNewFolder);

					if (MultiCopy) // �����������������
					{
						// ������� �������, ���� ��� �������� �������� �������-�����������
						if (wcspbrk(strNewFolder,L";,"))
							InsertQuote(strNewFolder);

						if (strOldFolder.GetLength())
							strOldFolder += L";"; // ������� ����������� � ��������� ������

						strOldFolder += strNewFolder;
						strNewFolder = strOldFolder;
					}

					SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_SC_TARGETEDIT,const_cast<wchar_t*>(strNewFolder.CPtr()));
					SendDlgMessage(hDlg,DM_SETFOCUS,ID_SC_TARGETEDIT,0);
				}
			}

			DlgParam->AltF10=0;
			return TRUE;
		}
		case DN_CLOSE:
		{
			if (Param1==ID_SC_BTNCOPY)
			{
				FarListGetItem LGI={CM_ASKRO};
				SendDlgMessage(hDlg,DM_LISTGETITEM,ID_SC_COMBO,&LGI);

				if (LGI.Item.Flags&LIF_CHECKED)
					DlgParam->AskRO=TRUE;
			}
		}
		break;

		default:
			break;
	}

	return DefDlgProc(hDlg,Msg,Param1,Param2);
}

ShellCopy::ShellCopy(Panel *SrcPanel,        // �������� ������ (��������)
                     int Move,               // =1 - �������� Move
                     int Link,               // =1 - Sym/Hard Link
                     int CurrentOnly,        // =1 - ������ ������� ����, ��� ��������
                     int Ask,                // =1 - �������� ������?
                     int &ToPlugin,          // =?
                     const wchar_t *PluginDestPath,
                     bool ToSubdir):
	sddata(nullptr),
	CopyBuffer(nullptr),
	RPT(RP_EXACTCOPY)
{
	Filter=nullptr;
	DestList.SetParameters(0,0,ULF_UNIQUE);
	CopyDlgParam CDP={};
	if (!(CDP.SelCount=SrcPanel->GetSelCount()))
		return;

	string strSelName;

	if (CDP.SelCount==1)
	{
		SrcPanel->GetSelName(nullptr,CDP.FileAttr); //????
		SrcPanel->GetSelName(&strSelName,CDP.FileAttr);

		if (TestParentFolderName(strSelName))
			return;
	}

	ZoomedState=IsZoomed(Console.GetWindow());
	IconicState=IsIconic(Console.GetWindow());
	// �������� ������ �������
	Filter=new FileFilter(SrcPanel, FFT_COPY);
	sddata=new char[SDDATA_SIZE];
	// $ 26.05.2001 OT ��������� ����������� ������� �� ����� �����������
	_tran(SysLog(L"call (*FrameManager)[0]->LockRefresh()"));
	(*FrameManager)[0]->Lock();
	// ������ ������ ������� �� �������
	GeneralCfg->GetValue(L"System", L"CopyBufferSize", &CopyBufferSize, 0);
	CopyBufferSize=Max(CopyBufferSize,(int)COPY_BUFFER_SIZE);
	CDP.thisClass=this;
	CDP.AltF10=0;
	CDP.FolderPresent=false;
	CDP.FilesPresent=false;
	Flags=(Move?FCOPY_MOVE:0)|(Link?FCOPY_LINK:0)|(CurrentOnly?FCOPY_CURRENTONLY:0);
	ShowTotalCopySize=Opt.CMOpt.CopyShowTotal!=0;
	strTotalCopySizeText.Clear();
	SelectedFolderNameLength=0;
	int DestPlugin=ToPlugin;
	ToPlugin=FALSE;
	SrcDriveType=0;
	this->SrcPanel=SrcPanel;
	DestPanel=CtrlObject->Cp()->GetAnotherPanel(SrcPanel);
	DestPanelMode=DestPlugin ? DestPanel->GetMode():NORMAL_PANEL;
	SrcPanelMode=SrcPanel->GetMode();

	// ***********************************************************************
	// *** Prepare Dialog Controls
	// ***********************************************************************
	int DLG_HEIGHT=16, DLG_WIDTH=76;

	FarDialogItem CopyDlgData[]=
	{
		{DI_DOUBLEBOX,   3, 1,DLG_WIDTH-4,DLG_HEIGHT-2,0,nullptr,nullptr,0,MSG(MCopyDlgTitle)},
		{DI_TEXT,        5, 2, 0, 2,0,nullptr,nullptr,0,MSG(Link?MCMLTargetIN:MCMLTargetTO)},
		{DI_EDIT,        5, 3,70, 3,0,L"Copy",nullptr,DIF_FOCUS|DIF_HISTORY|DIF_EDITEXPAND|DIF_USELASTHISTORY|DIF_EDITPATH,L""},
		{DI_TEXT,        3, 4, 0, 4,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_TEXT,        5, 5, 0, 5,0,nullptr,nullptr,0,MSG(MCopySecurity)},
		{DI_RADIOBUTTON, 5, 5, 0, 5,0,nullptr,nullptr,DIF_GROUP,MSG(MCopySecurityLeave)},
		{DI_RADIOBUTTON, 5, 5, 0, 5,0,nullptr,nullptr,0,MSG(MCopySecurityCopy)},
		{DI_RADIOBUTTON, 5, 5, 0, 5,0,nullptr,nullptr,0,MSG(MCopySecurityInherit)},
		{DI_TEXT,        3, 6, 0, 6,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_TEXT,        5, 7, 0, 7,0,nullptr,nullptr,0,MSG(MCopyIfFileExist)},
		{DI_COMBOBOX,   29, 7,70, 7,0,nullptr,nullptr,DIF_DROPDOWNLIST|DIF_LISTNOAMPERSAND|DIF_LISTWRAPMODE,L""},
		{DI_CHECKBOX,    5, 8, 0, 8,0,nullptr,nullptr,0,MSG(MCopySymLinkContents)},
		{DI_CHECKBOX,    5, 9, 0, 9,0,nullptr,nullptr,0,MSG(MCopyMultiActions)},
		{DI_TEXT,        3,10, 0,10,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_CHECKBOX,    5,11, 0,11,UseFilter?BSTATE_CHECKED:BSTATE_UNCHECKED,nullptr,nullptr,DIF_AUTOMATION,MSG(MCopyUseFilter)},
		{DI_TEXT,        3,12, 0,12,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_BUTTON,      0,13, 0,13,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_CENTERGROUP,MSG(MCopyDlgCopy)},
		{DI_BUTTON,      0,13, 0,13,0,nullptr,nullptr,DIF_CENTERGROUP|DIF_BTNNOCLOSE,MSG(MCopyDlgTree)},
		{DI_BUTTON,      0,13, 0,13,0,nullptr,nullptr,DIF_CENTERGROUP|DIF_BTNNOCLOSE|DIF_AUTOMATION|(UseFilter?0:DIF_DISABLE),MSG(MCopySetFilter)},
		{DI_BUTTON,      0,13, 0,13,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MCopyDlgCancel)},
		{DI_TEXT,        5, 2, 0, 2,0,nullptr,nullptr,DIF_SHOWAMPERSAND,L""},
	};
	MakeDialogItemsEx(CopyDlgData,CopyDlg);
	CopyDlg[ID_SC_MULTITARGET].Selected=Opt.CMOpt.MultiCopy;
	{
		const wchar_t *Str = MSG(MCopySecurity);
		CopyDlg[ID_SC_ACLEAVE].X1 = CopyDlg[ID_SC_ACTITLE].X1 + StrLength(Str) - (wcschr(Str, L'&')?1:0) + 1;
		Str = MSG(MCopySecurityLeave);
		CopyDlg[ID_SC_ACCOPY].X1 = CopyDlg[ID_SC_ACLEAVE].X1 + StrLength(Str) - (wcschr(Str, L'&')?1:0) + 5;
		Str = MSG(MCopySecurityCopy);
		CopyDlg[ID_SC_ACINHERIT].X1 = CopyDlg[ID_SC_ACCOPY].X1 + StrLength(Str) - (wcschr(Str, L'&')?1:0) + 5;
	}

	if (Link)
	{
		CopyDlg[ID_SC_COMBOTEXT].strData=MSG(MLinkType);
		CopyDlg[ID_SC_COPYSYMLINK].Selected=0;
		CopyDlg[ID_SC_COPYSYMLINK].Flags|=DIF_DISABLE|DIF_HIDDEN;
		CDP.CopySecurity=1;
	}
	else if (Move) // ������ ��� �������
	{
		CopyDlg[ID_SC_MULTITARGET].Selected = 0;
		CopyDlg[ID_SC_MULTITARGET].Flags |= DIF_DISABLE;

		//   2 - Default
		//   1 - Copy access rights
		//   0 - Inherit access rights
		CDP.CopySecurity=2;

		// ������� ����� "Inherit access rights"?
		// CSO_MOVE_SETINHERITSECURITY - ���������� ����
		if ((Opt.CMOpt.CopySecurityOptions&CSO_MOVE_SETINHERITSECURITY) == CSO_MOVE_SETINHERITSECURITY)
			CDP.CopySecurity=0;
		else if (Opt.CMOpt.CopySecurityOptions&CSO_MOVE_SETCOPYSECURITY)
			CDP.CopySecurity=1;

		// ������ ���������� �����������?
		if (CopySecurityMove != -1 && (Opt.CMOpt.CopySecurityOptions&CSO_MOVE_SESSIONSECURITY))
			CDP.CopySecurity=CopySecurityMove;
		else
			CopySecurityMove=CDP.CopySecurity;
	}
	else // ������ ��� �����������
	{
		//   2 - Default
		//   1 - Copy access rights
		//   0 - Inherit access rights
		CDP.CopySecurity=2;

		// ������� ����� "Inherit access rights"?
		// CSO_COPY_SETINHERITSECURITY - ���������� ����
		if ((Opt.CMOpt.CopySecurityOptions&CSO_COPY_SETINHERITSECURITY) == CSO_COPY_SETINHERITSECURITY)
			CDP.CopySecurity=0;
		else if (Opt.CMOpt.CopySecurityOptions&CSO_COPY_SETCOPYSECURITY)
			CDP.CopySecurity=1;

		// ������ ���������� �����������?
		if (CopySecurityCopy != -1 && Opt.CMOpt.CopySecurityOptions&CSO_COPY_SESSIONSECURITY)
			CDP.CopySecurity=CopySecurityCopy;
		else
			CopySecurityCopy=CDP.CopySecurity;
	}

	// ��� ������ ����������
	if (CDP.CopySecurity)
	{
		if (CDP.CopySecurity == 1)
		{
			Flags|=FCOPY_COPYSECURITY;
			CopyDlg[ID_SC_ACCOPY].Selected=1;
		}
		else
		{
			Flags|=FCOPY_LEAVESECURITY;
			CopyDlg[ID_SC_ACLEAVE].Selected=1;
		}
	}
	else
	{
		Flags&=~(FCOPY_COPYSECURITY|FCOPY_LEAVESECURITY);
		CopyDlg[ID_SC_ACINHERIT].Selected=1;
	}

	string strCopyStr;

	if (CDP.SelCount==1)
	{
		if (SrcPanel->GetType()==TREE_PANEL)
		{
			string strNewDir(strSelName);
			size_t pos;

			if (FindLastSlash(pos,strNewDir))
			{
				strNewDir.SetLength(pos);

				if (!pos || strNewDir.At(pos-1)==L':')
					strNewDir += L"\\";

				FarChDir(strNewDir);
			}
		}

		string strSelNameShort = strSelName;
		strCopyStr=MSG(Move?MMoveFile:(Link?MLinkFile:MCopyFile));
		TruncPathStr(strSelNameShort,static_cast<int>(CopyDlg[ID_SC_TITLE].X2-CopyDlg[ID_SC_TITLE].X1-strCopyStr.GetLength()-7));
		strCopyStr+=L" "+strSelNameShort;

		// ���� �������� ��������� ����, �� ��������� ������������ ������
		if (!(CDP.FileAttr&FILE_ATTRIBUTE_DIRECTORY))
		{
			CopyDlg[ID_SC_USEFILTER].Selected=0;
			CopyDlg[ID_SC_USEFILTER].Flags|=DIF_DISABLE;
		}
	}
	else // �������� ���������!
	{
		int NOper=MCopyFiles;

		if (Move) NOper=MMoveFiles;
		else if (Link) NOper=MLinkFiles;

		// ��������� ����� - ��� ���������
		FormatString StrItems;
		StrItems<<CDP.SelCount;
		size_t LenItems=StrItems.GetLength();
		int NItems=MCMLItemsA;

		if (LenItems > 0)
		{
			if ((LenItems >= 2 && StrItems[LenItems-2] == '1') ||
			        StrItems[LenItems-1] >= '5' ||
			        StrItems[LenItems-1] == '0')
				NItems=MCMLItemsS;
			else if (StrItems[LenItems-1] == '1')
				NItems=MCMLItems0;
		}

		strCopyStr.Format(MSG(NOper),CDP.SelCount,MSG(NItems));
	}

	CopyDlg[ID_SC_SOURCEFILENAME].strData=strCopyStr;
	CopyDlg[ID_SC_TITLE].strData = MSG(Move?MMoveDlgTitle :(Link?MLinkDlgTitle:MCopyDlgTitle));
	CopyDlg[ID_SC_BTNCOPY].strData = MSG(Move?MCopyDlgRename:(Link?MCopyDlgLink:MCopyDlgCopy));

	if (DestPanelMode == PLUGIN_PANEL)
	{
		// ���� ��������������� ������ - ������, �� �������� OnlyNewer //?????
/*
		CDP.CopySecurity=2;
		CopyDlg[ID_SC_ACCOPY].Selected=0;
		CopyDlg[ID_SC_ACINHERIT].Selected=0;
		CopyDlg[ID_SC_ACLEAVE].Selected=1;
		CopyDlg[ID_SC_ACCOPY].Flags|=DIF_DISABLE;
		CopyDlg[ID_SC_ACINHERIT].Flags|=DIF_DISABLE;
		CopyDlg[ID_SC_ACLEAVE].Flags|=DIF_DISABLE;
*/
	}

	string strDestDir;
	DestPanel->GetCurDir(strDestDir);
	if(ToSubdir)
	{
		AddEndSlash(strDestDir);
		string strSubdir, strShort;
		DestPanel->GetCurName(strSubdir, strShort);
		strDestDir+=strSubdir;
	}
	string strSrcDir;
	SrcPanel->GetCurDir(strSrcDir);

	if (CurrentOnly)
	{
		//   ��� ����������� ������ �������� ��� �������� ����� ��� ��� � �������, ���� ��� �������� �����������.
		CopyDlg[ID_SC_TARGETEDIT].strData = strSelName;

		if (!Move && wcspbrk(CopyDlg[ID_SC_TARGETEDIT].strData,L",;"))
		{
			Unquote(CopyDlg[ID_SC_TARGETEDIT].strData);     // ������ ��� ������ �������
			InsertQuote(CopyDlg[ID_SC_TARGETEDIT].strData); // ������� � �������, �.�. ����� ���� �����������
		}
	}
	else
	{
		switch (DestPanelMode)
		{
			case NORMAL_PANEL:
			{
				if ((strDestDir.IsEmpty() || !DestPanel->IsVisible() || !StrCmpI(strSrcDir,strDestDir)) && CDP.SelCount==1)
					CopyDlg[ID_SC_TARGETEDIT].strData = strSelName;
				else
				{
					CopyDlg[ID_SC_TARGETEDIT].strData = strDestDir;
					AddEndSlash(CopyDlg[ID_SC_TARGETEDIT].strData);
				}

				/* $ 19.07.2003 IS
				   ���� ���� �������� �����������, �� ������� �� � �������, ���� �� ��������
				   ������ ��� F5, Enter � �������, ����� ������������ ������� MultiCopy
				*/
				if (!Move && wcspbrk(CopyDlg[ID_SC_TARGETEDIT].strData,L",;"))
				{
					Unquote(CopyDlg[ID_SC_TARGETEDIT].strData);     // ������ ��� ������ �������
					InsertQuote(CopyDlg[ID_SC_TARGETEDIT].strData); // ������� � �������, �.�. ����� ���� �����������
				}

				break;
			}

			case PLUGIN_PANEL:
			{
				OpenPanelInfo Info;
				DestPanel->GetOpenPanelInfo(&Info);
				string strFormat = Info.Format;
				CopyDlg[ID_SC_TARGETEDIT].strData = strFormat+L":";

				while (CopyDlg[ID_SC_TARGETEDIT].strData.GetLength()<2)
					CopyDlg[ID_SC_TARGETEDIT].strData += L":";

				CDP.strPluginFormat = CopyDlg[ID_SC_TARGETEDIT].strData;
				CDP.strPluginFormat.Upper();
				break;
			}
		}
	}

	string strInitDestDir = CopyDlg[ID_SC_TARGETEDIT].strData;
	// ��� �������
	FAR_FIND_DATA_EX fd;
	SrcPanel->GetSelName(nullptr,CDP.FileAttr);

	bool AddSlash=false;

	while (SrcPanel->GetSelName(&strSelName,CDP.FileAttr,nullptr,&fd))
	{
		if (UseFilter)
		{
			if (!Filter->FileInFilter(fd))
				continue;
		}

		if (CDP.FileAttr & FILE_ATTRIBUTE_DIRECTORY)
		{
			CDP.FolderPresent=true;
			AddSlash=true;
//      break;
		}
		else
		{
			CDP.FilesPresent=true;
		}
	}

	if (Link) // ������ �� ������ ������ (���������������!)
	{
		// ���������� ����� ��� ����������� �����.
		CopyDlg[ID_SC_ACTITLE].Flags|=DIF_DISABLE|DIF_HIDDEN;
		CopyDlg[ID_SC_ACCOPY].Flags|=DIF_DISABLE|DIF_HIDDEN;
		CopyDlg[ID_SC_ACINHERIT].Flags|=DIF_DISABLE|DIF_HIDDEN;
		CopyDlg[ID_SC_ACLEAVE].Flags|=DIF_DISABLE|DIF_HIDDEN;
		CopyDlg[ID_SC_SEPARATOR2].Flags|=DIF_HIDDEN;

		for(int i=ID_SC_SEPARATOR2;i<=ID_SC_COMBO;i++)
		{
			CopyDlg[i].Y1-=2;
			CopyDlg[i].Y2-=2;
		}
		for(int i=ID_SC_MULTITARGET;i<=ID_SC_BTNCANCEL;i++)
		{
			CopyDlg[i].Y1-=3;
			CopyDlg[i].Y2-=3;
		}
		CopyDlg[ID_SC_TITLE].Y2-=3;
		DLG_HEIGHT-=3;
	}

	// ����������� ������� " to"
	CopyDlg[ID_SC_TARGETTITLE].X1=CopyDlg[ID_SC_TARGETTITLE].X2=CopyDlg[ID_SC_SOURCEFILENAME].X1+(int)CopyDlg[ID_SC_SOURCEFILENAME].strData.GetLength();

	/* $ 15.06.2002 IS
	   ��������� ����������� ������ - � ���� ������ ������ �� ������������,
	   �� ���������� ��� ����� ����������������. ���� ���������� ���������
	   ���������� ������ �����, �� ������� ������.
	*/
	string strCopyDlgValue;
	if (!Ask)
	{
		strCopyDlgValue = CopyDlg[ID_SC_TARGETEDIT].strData;
		Unquote(strCopyDlgValue);
		InsertQuote(strCopyDlgValue);

		if (!DestList.Set(strCopyDlgValue))
			Ask=TRUE;
	}

	// ***********************************************************************
	// *** ����� � ��������� �������
	// ***********************************************************************
	if (Ask)
	{
		FarList ComboList;
		FarListItem LinkTypeItems[5]={},CopyModeItems[8]={};

		if (Link)
		{
			ComboList.ItemsNumber=ARRAYSIZE(LinkTypeItems);
			ComboList.Items=LinkTypeItems;
			ComboList.Items[0].Text=MSG(MLinkTypeHardlink);
			ComboList.Items[1].Text=MSG(MLinkTypeJunction);
			ComboList.Items[2].Text=MSG(MLinkTypeSymlink);
			ComboList.Items[3].Text=MSG(MLinkTypeSymlinkFile);
			ComboList.Items[4].Text=MSG(MLinkTypeSymlinkDirectory);

			if (CDP.FilesPresent)
				ComboList.Items[0].Flags|=LIF_SELECTED;
			else
				ComboList.Items[1].Flags|=LIF_SELECTED;
		}
		else
		{
			ComboList.ItemsNumber=ARRAYSIZE(CopyModeItems);
			ComboList.Items=CopyModeItems;
			ComboList.Items[CM_ASK].Text=MSG(MCopyAsk);
			ComboList.Items[CM_OVERWRITE].Text=MSG(MCopyOverwrite);
			ComboList.Items[CM_SKIP].Text=MSG(MCopySkipOvr);
			ComboList.Items[CM_RENAME].Text=MSG(MCopyRename);
			ComboList.Items[CM_APPEND].Text=MSG(MCopyAppend);
			ComboList.Items[CM_ONLYNEWER].Text=MSG(MCopyOnlyNewerFiles);
			ComboList.Items[CM_ASKRO].Text=MSG(MCopyAskRO);
			ComboList.Items[CM_ASK].Flags=LIF_SELECTED;
			ComboList.Items[CM_SEPARATOR].Flags=LIF_SEPARATOR;

			if (Opt.Confirm.RO)
			{
				ComboList.Items[CM_ASKRO].Flags=LIF_CHECKED;
			}
		}

		CopyDlg[ID_SC_COMBO].ListItems=&ComboList;
		Dialog Dlg(CopyDlg,ARRAYSIZE(CopyDlg),CopyDlgProc,&CDP);
		Dlg.SetHelp(Link?L"HardSymLink":L"CopyFiles");
		Dlg.SetId(Link?HardSymLinkId:(Move?MoveFilesId:CopyFilesId));
		Dlg.SetPosition(-1,-1,DLG_WIDTH,DLG_HEIGHT);
		Dlg.SetAutomation(ID_SC_USEFILTER,ID_SC_BTNFILTER,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
//    Dlg.Show();
		// $ 02.06.2001 IS + �������� ������ ����� � �������� �������, ���� �� �������� ������
		int DlgExitCode;

		for (;;)
		{
			Dlg.ClearDone();
			Dlg.Process();
			DlgExitCode=Dlg.GetExitCode();
			//������ �������� ������� ��� ������� ����� ����� ������ �� �������
			Filter->UpdateCurrentTime();

			if (DlgExitCode == ID_SC_BTNCOPY)
			{
				/* $ 03.08.2001 IS
				   �������� ������� �� ������� � �������� �� ������ � ����������� ��
				   ��������� ����� �����������������
				*/
				strCopyDlgValue = CopyDlg[ID_SC_TARGETEDIT].strData;
				if(!Move)
				{
					Opt.CMOpt.MultiCopy=CopyDlg[ID_SC_MULTITARGET].Selected;
				}

				if (!CopyDlg[ID_SC_MULTITARGET].Selected || !wcspbrk(strCopyDlgValue,L",;")) // ��������� multi*
				{
					// ������ ������ �������
					Unquote(strCopyDlgValue);
					// ������� �������, ����� "������" ������ ��������������� ���
					// ����������� �� ������� ������������ � ����
					InsertQuote(strCopyDlgValue);
				}

				if (DestList.Set(strCopyDlgValue) && !wcspbrk(strCopyDlgValue,ReservedFilenameSymbols))
				{
					// ��������� ������� ������������� �������. KM
					UseFilter=CopyDlg[ID_SC_USEFILTER].Selected;
					break;
				}
				else
				{
					Message(MSG_WARNING,1,MSG(MWarning),MSG(MCopyIncorrectTargetList), MSG(MOk));
				}
			}
			else
				break;
		}

		if (DlgExitCode == ID_SC_BTNCANCEL || DlgExitCode < 0 || (CopyDlg[ID_SC_BTNCOPY].Flags&DIF_DISABLE))
		{
			if (DestPlugin)
				ToPlugin=-1;

			return;
		}
	}

	// ***********************************************************************
	// *** ������ ���������� ������ ����� �������
	// ***********************************************************************
	Flags&=~FCOPY_COPYPARENTSECURITY;

	if (CopyDlg[ID_SC_ACCOPY].Selected)
	{
		Flags|=FCOPY_COPYSECURITY;
	}
	else if (CopyDlg[ID_SC_ACINHERIT].Selected)
	{
		Flags&=~(FCOPY_COPYSECURITY|FCOPY_LEAVESECURITY);
	}
	else
	{
		Flags|=FCOPY_LEAVESECURITY;
	}

	if (Opt.CMOpt.UseSystemCopy)
		Flags|=FCOPY_USESYSTEMCOPY;
	else
		Flags&=~FCOPY_USESYSTEMCOPY;

	if (!(Flags&(FCOPY_COPYSECURITY|FCOPY_LEAVESECURITY)))
		Flags|=FCOPY_COPYPARENTSECURITY;

	CDP.CopySecurity=Flags&FCOPY_COPYSECURITY?1:(Flags&FCOPY_LEAVESECURITY?2:0);

	// � ����� ������ ��������� ���������� ����������� (�� ��� Link, �.�. ��� Link ��������� ��������� - "������!")
	if (!Link)
	{
		if (Move)
			CopySecurityMove=CDP.CopySecurity;
		else
			CopySecurityCopy=CDP.CopySecurity;
	}

	ReadOnlyDelMode=ReadOnlyOvrMode=OvrMode=SkipEncMode=SkipMode=SkipDeleteMode=-1;

	if (Link)
	{
		switch (CopyDlg[ID_SC_COMBO].ListPos)
		{
			case 0:
				RPT=RP_HARDLINK;
				break;
			case 1:
				RPT=RP_JUNCTION;
				break;
			case 2:
				RPT=RP_SYMLINK;
				break;
			case 3:
				RPT=RP_SYMLINKFILE;
				break;
			case 4:
				RPT=RP_SYMLINKDIR;
				break;
		}
	}
	else
	{
		ReadOnlyOvrMode=CDP.AskRO?-1:1;

		switch (CopyDlg[ID_SC_COMBO].ListPos)
		{
			case CM_ASK:
				OvrMode=-1;
				break;
			case CM_OVERWRITE:
				OvrMode=1;
				break;
			case CM_SKIP:
				OvrMode=3;
				ReadOnlyOvrMode=CDP.AskRO?-1:3;
				break;
			case CM_RENAME:
				OvrMode=5;
				break;
			case CM_APPEND:
				OvrMode=7;
				break;
			case CM_ONLYNEWER:
				Flags|=FCOPY_ONLYNEWERFILES;
				break;
		}
	}

	Flags|=CopyDlg[ID_SC_COPYSYMLINK].Selected?FCOPY_COPYSYMLINKCONTENTS:0;

	if (DestPlugin && !StrCmp(CopyDlg[ID_SC_TARGETEDIT].strData,strInitDestDir))
	{
		ToPlugin=1;
		return;
	}

	if (DestPlugin==2)
	{
		if (PluginDestPath)
			strCopyDlgValue = PluginDestPath;

		return;
	}

	if ((Opt.Diz.UpdateMode==DIZ_UPDATE_IF_DISPLAYED && SrcPanel->IsDizDisplayed()) ||
	        Opt.Diz.UpdateMode==DIZ_UPDATE_ALWAYS)
	{
		CtrlObject->Cp()->LeftPanel->ReadDiz();
		CtrlObject->Cp()->RightPanel->ReadDiz();
	}

	CopyBuffer=new char[CopyBufferSize];
	DestPanel->CloseFile();
	strDestDizPath.Clear();
	SrcPanel->SaveSelection();
	// TODO: Posix - bugbug
	ReplaceSlashToBSlash(strCopyDlgValue);
	// ����� �� ���������� ����� �����������?
	// ***********************************************************************
	// **** ����� ��� ���������������� �������� ���������, ����� ����������
	// **** � �������� Copy/Move/Link
	// ***********************************************************************
	int NeedDizUpdate=FALSE;
	int NeedUpdateAPanel=FALSE;
	// ����! ������������� �������� ����������.
	// � ����������� ���� ���� ����� ������������ � ShellCopy::CheckUpdatePanel()
	Flags|=FCOPY_UPDATEPPANEL;
	/*
	   ���� ������� � �������� ����������� �����, �������� ';',
	   �� ����� ������� CopyDlgValue �� ������� MultiCopy �
	   �������� CopyFileTree ������ ���������� ���.
	*/
	{
		Flags&=~FCOPY_MOVE;

		if (DestList.Set(strCopyDlgValue)) // ���� ������ ������� "���������������"
		{
			const wchar_t *NamePtr;
			string strNameTmp;
			// ��������� ���������� �����.
			CountTarget=DestList.GetTotal();
			DestList.Reset();
			TotalFiles=0;
			TotalCopySize=TotalCopiedSize=TotalSkippedSize=0;

			// �������� ����� ������
			CopyStartTime = clock();
			WaitUserTime = OldCalcTime = 0;

			if (CountTarget > 1)
				Move=0;

			while (nullptr!=(NamePtr=DestList.GetNext()))
			{
				CurCopiedSize=0;
				strNameTmp = NamePtr;

				if ((strNameTmp.GetLength() == 2) && IsAlpha(strNameTmp.At(0)) && (strNameTmp.At(1) == L':'))
					PrepareDiskPath(strNameTmp);

				if (!StrCmp(strNameTmp,L"..") && IsLocalRootPath(strSrcDir))
				{
					if (!Message(MSG_WARNING,2,MSG(MError),MSG((!Move?MCannotCopyToTwoDot:MCannotMoveToTwoDot)),MSG(MCannotCopyMoveToTwoDot),MSG(MCopySkip),MSG(MCopyCancel)))
						continue;

					break;
				}

				if (CheckNulOrCon(strNameTmp))
					Flags|=FCOPY_COPYTONUL;
				else
					Flags&=~FCOPY_COPYTONUL;

				if (Flags&FCOPY_COPYTONUL)
				{
					Flags&=~FCOPY_MOVE;
					Move=0;
				}
				bool ShowCopyTime=(Opt.CMOpt.CopyTimeRule&((Flags&FCOPY_COPYTONUL)?COPY_RULE_NUL:COPY_RULE_FILES))!=0;

				if (CDP.SelCount==1 || (Flags&FCOPY_COPYTONUL))
					AddSlash=false; //???


				if (DestList.IsEmpty()) // ����� ������ ������� ��������� � ��������� Move.
				{
					Flags|=FCOPY_COPYLASTTIME|(Move?FCOPY_MOVE:0); // ������ ��� ��������� ��������
				}

				// ���� ���������� ��������� ������ 1 � ����� ��� ���� �������, �� ������
				// ������ ���, ����� �� ����� ��� '\\'
				// ������� ��� �� ������, � ������ ����� NameTmp �� �������� ������.
				if (AddSlash && !wcspbrk(strNameTmp,L"*?"))
					AddEndSlash(strNameTmp);

				if (CDP.SelCount==1 && !CDP.FolderPresent)
				{
					ShowTotalCopySize=false;
					TotalFilesToProcess=1;
				}

				if (Move) // ��� ����������� "�����" ��� �� ����������� ��� "���� �� �����"
				{
					if (CheckDisksProps(strSrcDir,strNameTmp,CHECKEDPROPS_ISSAMEDISK))
						ShowTotalCopySize=false;
					if (CDP.SelCount==1 && CDP.FolderPresent && CheckUpdateAnotherPanel(SrcPanel,strSelName))
					{
						NeedUpdateAPanel=TRUE;
					}
				}

				CP=new CopyProgress(Move!=0,ShowTotalCopySize,ShowCopyTime);
				// ������� ���� ��� ����
				strDestDizPath.Clear();
				Flags&=~FCOPY_DIZREAD;
				// �������� ���������
				SrcPanel->SaveSelection();
				strDestFSName.Clear();
				int OldCopySymlinkContents=Flags&FCOPY_COPYSYMLINKCONTENTS;
				// ���������� - ���� ������ �����������
				// Mantis#45: ���������� ������� ����������� ������ �� ����� � NTFS �� FAT � ����� ��������� ����
				{
					string strRootDir;
					ConvertNameToFull(strNameTmp,strRootDir);
					GetPathRoot(strRootDir, strRootDir);
					DWORD FileSystemFlags=0;

					if (apiGetVolumeInformation(strRootDir,nullptr,nullptr,nullptr,&FileSystemFlags,nullptr))
						if (!(FileSystemFlags&FILE_SUPPORTS_REPARSE_POINTS))
							Flags|=FCOPY_COPYSYMLINKCONTENTS;
				}
				PreRedraw.Push(PR_ShellCopyMsg);
				PreRedrawItem preRedrawItem=PreRedraw.Peek();
				preRedrawItem.Param.Param1=CP;
				PreRedraw.SetParam(preRedrawItem.Param);
				int I=CopyFileTree(strNameTmp);
				PreRedraw.Pop();

				if (OldCopySymlinkContents)
					Flags|=FCOPY_COPYSYMLINKCONTENTS;
				else
					Flags&=~FCOPY_COPYSYMLINKCONTENTS;

				if (I == COPY_CANCEL)
				{
					NeedDizUpdate=TRUE;
					break;
				}

				// ���� "���� ����� � ������������" - ����������� ���������
				if (!DestList.IsEmpty())
					SrcPanel->RestoreSelection();

				// ����������� � �����.
				if (!(Flags&FCOPY_COPYTONUL) && !strDestDizPath.IsEmpty())
				{
					string strDestDizName;
					DestDiz.GetDizName(strDestDizName);
					DWORD Attr=apiGetFileAttributes(strDestDizName);
					int DestReadOnly=(Attr!=INVALID_FILE_ATTRIBUTES && (Attr & FILE_ATTRIBUTE_READONLY));

					if (DestList.IsEmpty()) // ��������� ������ �� ����� ��������� Op.
						if (Move && !DestReadOnly)
							SrcPanel->FlushDiz();

					DestDiz.Flush(strDestDizPath);
				}
			}
		}
		_LOGCOPYR(else SysLog(L"Error: DestList.Set(CopyDlgValue) return FALSE"));
	}
	// ***********************************************************************
	// *** �������������� ������ ��������
	// *** ���������������/�����/��������
	// ***********************************************************************

	if (NeedDizUpdate) // ��� ����������������� ����� ���� �����, �� ��� ���
	{                 // ����� ����� ��������� ����!
		if (!(Flags&FCOPY_COPYTONUL) && !strDestDizPath.IsEmpty())
		{
			string strDestDizName;
			DestDiz.GetDizName(strDestDizName);
			DWORD Attr=apiGetFileAttributes(strDestDizName);
			int DestReadOnly=(Attr!=INVALID_FILE_ATTRIBUTES && (Attr & FILE_ATTRIBUTE_READONLY));

			if (Move && !DestReadOnly)
				SrcPanel->FlushDiz();

			DestDiz.Flush(strDestDizPath);
		}
	}

#if 1
	SrcPanel->Update(UPDATE_KEEP_SELECTION);

	if (CDP.SelCount==1 && !strRenamedName.IsEmpty())
		SrcPanel->GoToFile(strRenamedName);

#if 1

	if (NeedUpdateAPanel && CDP.FileAttr != INVALID_FILE_ATTRIBUTES && (CDP.FileAttr&FILE_ATTRIBUTE_DIRECTORY) && DestPanelMode != PLUGIN_PANEL)
	{
		string strTmpSrcDir;
		SrcPanel->GetCurDir(strTmpSrcDir);
		DestPanel->SetCurDir(strTmpSrcDir,FALSE);
	}

#else

	if (CDP.FileAttr != INVALID_FILE_ATTRIBUTES && (CDP.FileAttr&FILE_ATTRIBUTE_DIRECTORY) && DestPanelMode != PLUGIN_PANEL)
	{
		// ���� SrcDir ���������� � DestDir...
		string strTmpDestDir;
		string strTmpSrcDir;
		DestPanel->GetCurDir(strTmpDestDir);
		SrcPanel->GetCurDir(strTmpSrcDir);

		if (CheckUpdateAnotherPanel(SrcPanel,strTmpSrcDir))
			DestPanel->SetCurDir(strTmpDestDir,FALSE);
	}

#endif

	// �������� "��������" ������� ��������� ������
	if (Flags&FCOPY_UPDATEPPANEL)
	{
		DestPanel->SortFileList(TRUE);
		DestPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
	}

	if (SrcPanelMode == PLUGIN_PANEL)
		SrcPanel->SetPluginModified();

	CtrlObject->Cp()->Redraw();
#else
	SrcPanel->Update(UPDATE_KEEP_SELECTION);

	if (CDP.SelCount==1 && strRenamedName.IsEmpty())
		SrcPanel->GoToFile(strRenamedName);

	SrcPanel->Redraw();
	DestPanel->SortFileList(TRUE);
	DestPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
	DestPanel->Redraw();
#endif
}

ShellCopy::~ShellCopy()
{
	_tran(SysLog(L"[%p] ShellCopy::~ShellCopy(), CopyBufer=%p",this,CopyBuffer));

	if (CopyBuffer)
		delete[] CopyBuffer;

	// $ 26.05.2001 OT ��������� ����������� �������
	_tran(SysLog(L"call (*FrameManager)[0]->UnlockRefresh()"));
	(*FrameManager)[0]->Unlock();
	(*FrameManager)[0]->Refresh();

	if (sddata)
		delete[] sddata;

	if (Filter) // ��������� ������ �������
		delete Filter;

	if (CP)
	{
		delete CP;
		CP=nullptr;
	}
}




COPY_CODES ShellCopy::CopyFileTree(const wchar_t *Dest)
{
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
	//SaveScreen SaveScr;
	DWORD DestAttr=INVALID_FILE_ATTRIBUTES;
	string strSelName, strSelShortName;
	int Length;
	DWORD FileAttr;

	if (!(Length=StrLength(Dest)) || !StrCmp(Dest,L"."))
		return COPY_FAILURE; //????

	SetCursorType(FALSE,0);
	Flags&=~(FCOPY_STREAMSKIP|FCOPY_STREAMALL);

	// �������� ��������� ��������� � ����� ����������
	if (!(Flags&FCOPY_COPYTONUL))
	{
		//if (Length > 1 && Dest[Length-1]=='\\' && Dest[Length-2]!=':') //??????????
		{
			string strNewPath = Dest;

			if (!IsSlash(strNewPath.At(strNewPath.GetLength()-1)) &&
			        SrcPanel->GetSelCount()>1 &&
			        !wcspbrk(strNewPath,L"*?") &&
			        apiGetFileAttributes(strNewPath)==INVALID_FILE_ATTRIBUTES)
			{
				switch (Message(FMSG_WARNING,3,MSG(MWarning),strNewPath,MSG(MCopyDirectoryOrFile),MSG(MCopyDirectoryOrFileDirectory),MSG(MCopyDirectoryOrFileFile),MSG(MCancel)))
				{
					case 0:
						AddEndSlash(strNewPath);
						break;
					case -2:
					case -1:
					case 2:
						return COPY_CANCEL;
				}
			}

			size_t pos;

			if (FindLastSlash(pos,strNewPath))
			{
				strNewPath.SetLength(pos+1);

				if (Opt.CreateUppercaseFolders && !IsCaseMixed(strNewPath))
					strNewPath.Upper();

				DWORD Attr=apiGetFileAttributes(strNewPath);

				if (Attr==INVALID_FILE_ATTRIBUTES)
				{
					if (apiCreateDirectory(strNewPath,nullptr))
						TreeList::AddTreeName(strNewPath);
					else
						CreatePath(strNewPath); //Maximus: BUGBUG: ������������ ������ �������� ��������!
				}
				else if (!(Attr & FILE_ATTRIBUTE_DIRECTORY))
				{
					Message(MSG_WARNING,1,MSG(MError),MSG(MCopyCannotCreateFolder),strNewPath,MSG(MOk));
					return COPY_FAILURE;
				}
			}
		}

		DestAttr=apiGetFileAttributes(Dest);
	}

	// �������� ������� "��� �� ����"
	bool SameDisk=false;

	if (Flags&FCOPY_MOVE)
	{
		string strTmpSrcDir;
		SrcPanel->GetCurDir(strTmpSrcDir);
		SameDisk=(CheckDisksProps(strTmpSrcDir,Dest,CHECKEDPROPS_ISSAMEDISK))!=0;
	}

	string strDest = Dest;
	bool UseWildCards = wcspbrk(Dest,L"*?")!=nullptr;
	if (!(Flags&FCOPY_COPYTONUL))
	{
		SrcPanel->GetSelName(nullptr,FileAttr);
		SrcPanel->GetSelName(&strSelName,FileAttr,&strSelShortName);
		SelectedFolderNameLength = (FileAttr & FILE_ATTRIBUTE_DIRECTORY)?(int)strSelName.GetLength():0;
		if (UseWildCards)
		{
			ConvertWildcards(strSelName, strDest, SelectedFolderNameLength);
		}

		DestAttr=apiGetFileAttributes(strDest);

		// ������� ������ � ����� ����������
		if (strDestDriveRoot.IsEmpty())
		{
			GetPathRoot(strDest,strDestDriveRoot);
			DestDriveType=FAR_GetDriveType(wcschr(strDest,L'\\') ? strDestDriveRoot.CPtr():nullptr);
		}
	}

	// "�������" � ������ ���� ������� - �������� ������ �������, ���������� �� �����
	// (�� �� ��� ������ �������������� ����� �� ����)
	if ((DestDriveType == DRIVE_REMOTE || SrcDriveType == DRIVE_REMOTE) && StrCmpI(strSrcDriveRoot,strDestDriveRoot))
		Flags|=FCOPY_COPYSYMLINKCONTENTS;

	if (!TotalCopySize)
	{
		strTotalCopySizeText.Clear();

		//  ! �� ��������� �������� ��� �������� ������
		if (ShowTotalCopySize && !(Flags&FCOPY_LINK) && !CalcTotalSize())
			return COPY_FAILURE;
	}
	else
	{
		CurCopiedSize=0;
	}


	// �������� ���� ����������� ����� ������.
	SrcPanel->GetSelName(nullptr,FileAttr);
	while (SrcPanel->GetSelName(&strSelName,FileAttr,&strSelShortName))
	{
		SelectedFolderNameLength = (FileAttr & FILE_ATTRIBUTE_DIRECTORY)?(int)strSelName.GetLength():0;
		if (UseWildCards)
		{
			strDest = Dest;
			ConvertWildcards(strSelName, strDest, SelectedFolderNameLength);
		}

		string strDestPath = strDest;

		FAR_FIND_DATA_EX SrcData;
		int CopyCode=COPY_SUCCESS,KeepPathPos;
		Flags&=~FCOPY_OVERWRITENEXT;

		if (strSrcDriveRoot.IsEmpty() || StrCmpNI(strSelName,strSrcDriveRoot,(int)strSrcDriveRoot.GetLength()))
		{
			GetPathRoot(strSelName,strSrcDriveRoot);
			SrcDriveType=FAR_GetDriveType(wcschr(strSelName,L'\\') ? strSrcDriveRoot.CPtr():nullptr);
		}


		KeepPathPos=(int)(PointToName(strSelName)-strSelName.CPtr());

		if (RPT==RP_JUNCTION || RPT==RP_SYMLINK || RPT==RP_SYMLINKFILE || RPT==RP_SYMLINKDIR)
		{
			switch (MkSymLink(strSelName,strDest,RPT,Flags))
			{
				case 2:
					break;
				case 1:

					// ������� (Ins) ��������� ���������, ALT-F6 Enter - ��������� � ����� �� �������.
					if ((!(Flags&FCOPY_CURRENTONLY)) && (Flags&FCOPY_COPYLASTTIME))
						SrcPanel->ClearLastGetSelection();

					continue;
				case 0:
					return COPY_FAILURE;
			}
		}
		else
		{
			// �������� �� �������� ;-)
			if (!apiGetFindDataEx(strSelName,SrcData))
			{
				strDestPath = strSelName;
				CP->SetNames(strSelName,strDestPath);

				if (Message(MSG_WARNING,2,MSG(MError),MSG(MCopyCannotFind),
					          strSelName,MSG(MSkip),MSG(MCancel))==1)
				{
					return COPY_FAILURE;
				}

				continue;
			}
		}


		//KeepPathPos=PointToName(SelName)-SelName;

		// �����?
		if ((Flags&FCOPY_MOVE))
		{
			// ����, � ��� �� ���� "��� �� ����"?
			if (KeepPathPos && PointToName(strDest)==strDest)
			{
				strDestPath = strSelName;
				strDestPath.SetLength(KeepPathPos);
				strDestPath += strDest;
				SameDisk=true;
			}

			if ((UseFilter || !SameDisk) || ((SrcData.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT) && (Flags&FCOPY_COPYSYMLINKCONTENTS)))
			{
				CopyCode=COPY_FAILURE;
			}
			else
			{
				do
				{
					CopyCode=ShellCopyOneFile(strSelName,SrcData,strDestPath,KeepPathPos,1);
				}
				while (CopyCode==COPY_RETRY);

				if (CopyCode==COPY_SUCCESS_MOVE)
				{
					if (!strDestDizPath.IsEmpty())
					{
						if (!strRenamedName.IsEmpty())
						{
							DestDiz.DeleteDiz(strSelName,strSelShortName);
							SrcPanel->CopyDiz(strSelName,strSelShortName,strRenamedName,strRenamedName,&DestDiz);
						}
						else
						{
							if (strCopiedName.IsEmpty())
								strCopiedName = strSelName;

							SrcPanel->CopyDiz(strSelName,strSelShortName,strCopiedName,strCopiedName,&DestDiz);
							SrcPanel->DeleteDiz(strSelName,strSelShortName);
						}
					}

					continue;
				}

				if (CopyCode==COPY_CANCEL)
					return COPY_CANCEL;

				if (CopyCode==COPY_NEXT)
				{
					TotalCopiedSize = TotalCopiedSize - CurCopiedSize + SrcData.nFileSize;
					TotalSkippedSize = TotalSkippedSize + SrcData.nFileSize - CurCopiedSize;
					continue;
				}

				if (!(Flags&FCOPY_MOVE) || CopyCode==COPY_FAILURE)
					Flags|=FCOPY_OVERWRITENEXT;
			}
		}

		if (!(Flags&FCOPY_MOVE) || CopyCode==COPY_FAILURE)
		{
			string strCopyDest=strDest;

			do
			{
				CopyCode=ShellCopyOneFile(strSelName,SrcData,strCopyDest,KeepPathPos,0);
			}
			while (CopyCode==COPY_RETRY);

			Flags&=~FCOPY_OVERWRITENEXT;

			if (CopyCode==COPY_CANCEL)
				return COPY_CANCEL;

			if (CopyCode!=COPY_SUCCESS)
			{
				if (CopyCode != COPY_NOFILTER) //????
					TotalCopiedSize = TotalCopiedSize - CurCopiedSize +  SrcData.nFileSize;

				if (CopyCode == COPY_NEXT)
					TotalSkippedSize = TotalSkippedSize +  SrcData.nFileSize - CurCopiedSize;

				continue;
			}
		}

		if (CopyCode==COPY_SUCCESS && !(Flags&FCOPY_COPYTONUL) && !strDestDizPath.IsEmpty())
		{
			if (strCopiedName.IsEmpty())
				strCopiedName = strSelName;

			SrcPanel->CopyDiz(strSelName,strSelShortName,strCopiedName,strCopiedName,&DestDiz);
		}

#if 0

		// ���� [ ] Copy contents of symbolic links
		if ((SrcData.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT) && !(Flags&FCOPY_COPYSYMLINKCONTENTS))
		{
			//������� �������
			switch (MkSymLink(SelName,Dest,FCOPY_LINK/*|FCOPY_NOSHOWMSGLINK*/))
			{
				case 2:
					break;
				case 1:

					// ������� (Ins) ��������� ���������, ALT-F6 Enter - ��������� � ����� �� �������.
					if ((!(Flags&FCOPY_CURRENTONLY)) && (Flags&FCOPY_COPYLASTTIME))
						SrcPanel->ClearLastGetSelection();

					_LOGCOPYR(SysLog(L"%d continue;",__LINE__));
					continue;
				case 0:
					_LOGCOPYR(SysLog(L"return COPY_FAILURE -> %d",__LINE__));
					return COPY_FAILURE;
			}

			continue;
		}

#endif

		// Mantis#44 - ������ ������ ��� ����������� ������ �� �����
		// ���� ������� (��� ����� ���������� �������) - �������� ���������� ����������...
		if (RPT!=RP_SYMLINKFILE && (SrcData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
			      (
			          !(SrcData.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT) ||
			          ((SrcData.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT) && (Flags&FCOPY_COPYSYMLINKCONTENTS))
			      )
			  )
		{
			int SubCopyCode;
			string strSubName;
			string strFullName;
			ScanTree ScTree(TRUE,TRUE,Flags&FCOPY_COPYSYMLINKCONTENTS);
			strSubName = strSelName;
			strSubName += L"\\";

			if (DestAttr==INVALID_FILE_ATTRIBUTES)
				KeepPathPos=(int)strSubName.GetLength();

			int NeedRename=!((SrcData.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT) && (Flags&FCOPY_COPYSYMLINKCONTENTS) && (Flags&FCOPY_MOVE));
			ScTree.SetFindPath(strSubName,L"*",FSCANTREE_FILESFIRST);

			while (ScTree.GetNextName(&SrcData,strFullName))
			{
				if (UseFilter && (SrcData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					// ������ ���������� ������� ������������ - ���� ������� ������� �
					// ������� ��� ������������, �� ������� ���������� � ��� � �� ���
					// ����������.
					if (!Filter->FileInFilter(SrcData))
					{
						ScTree.SkipDir();
						continue;
					}
				}
				{
					int AttemptToMove=FALSE;

					if ((Flags&FCOPY_MOVE) && SameDisk && !(SrcData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					{
						AttemptToMove=TRUE;
						int Ret=COPY_SUCCESS;
						string strCopyDest=strDest;

						do
						{
							Ret=ShellCopyOneFile(strFullName,SrcData,strCopyDest,KeepPathPos,NeedRename);
						}
						while (Ret==COPY_RETRY);

						switch (Ret) // 1
						{
							case COPY_CANCEL:
								return COPY_CANCEL;
							case COPY_NEXT:
							{
								TotalCopiedSize = TotalCopiedSize - CurCopiedSize + SrcData.nFileSize;
								TotalSkippedSize = TotalSkippedSize + SrcData.nFileSize - CurCopiedSize;
								continue;
							}
							case COPY_SUCCESS_MOVE:
							{
								continue;
							}
							case COPY_SUCCESS:

								if (!NeedRename) // ������� ��� ����������� ����������� ������� � ������ "���������� ���������� ���..."
								{
									TotalCopiedSize = TotalCopiedSize - CurCopiedSize + SrcData.nFileSize;
									TotalSkippedSize = TotalSkippedSize + SrcData.nFileSize - CurCopiedSize;
									continue;     // ...  �.�. �� ��� �� ������, � �����������, �� ���, �� ���� �������� �������� � ���� ������
								}
						}
					}

					int SaveOvrMode=OvrMode;

					if (AttemptToMove)
						OvrMode=1;

					string strCopyDest=strDest;

					do
					{
						SubCopyCode=ShellCopyOneFile(strFullName,SrcData,strCopyDest,KeepPathPos,0);
					}
					while (SubCopyCode==COPY_RETRY);

					if (AttemptToMove)
						OvrMode=SaveOvrMode;
				}

				if (SubCopyCode==COPY_CANCEL)
					return COPY_CANCEL;

				if (SubCopyCode==COPY_NEXT)
				{
					TotalCopiedSize = TotalCopiedSize - CurCopiedSize + SrcData.nFileSize;
					TotalSkippedSize = TotalSkippedSize + SrcData.nFileSize - CurCopiedSize;
				}

				if (SubCopyCode==COPY_SUCCESS)
				{
					if (Flags&FCOPY_MOVE)
					{
						if (SrcData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
						{
							if (ScTree.IsDirSearchDone() ||
								      ((SrcData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) && !(Flags&FCOPY_COPYSYMLINKCONTENTS)))
							{
								if (SrcData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
									apiSetFileAttributes(strFullName,FILE_ATTRIBUTE_NORMAL);

								if (apiRemoveDirectory(strFullName))
									TreeList::DelTreeName(strFullName);
							}
						}
						// ����� ����� �������� �� FSCANTREE_INSIDEJUNCTION, �����
						// ��� ������� ����� �������� �����, ��� ������ �����������!
						else if (!ScTree.InsideJunction())
						{
							if (DeleteAfterMove(strFullName,SrcData.dwFileAttributes)==COPY_CANCEL)
								return COPY_CANCEL;
						}
					}
				}
			}

			if ((Flags&FCOPY_MOVE) && CopyCode==COPY_SUCCESS)
			{
				if (FileAttr & FILE_ATTRIBUTE_READONLY)
					apiSetFileAttributes(strSelName,FILE_ATTRIBUTE_NORMAL);

				if (apiRemoveDirectory(strSelName))
				{
					TreeList::DelTreeName(strSelName);

					if (!strDestDizPath.IsEmpty())
						SrcPanel->DeleteDiz(strSelName,strSelShortName);
				}
			}
		}
		else if ((Flags&FCOPY_MOVE) && CopyCode==COPY_SUCCESS)
		{
			int DeleteCode;

			if ((DeleteCode=DeleteAfterMove(strSelName,FileAttr))==COPY_CANCEL)
				return COPY_CANCEL;

			if (DeleteCode==COPY_SUCCESS && !strDestDizPath.IsEmpty())
				SrcPanel->DeleteDiz(strSelName,strSelShortName);
		}

		if ((!(Flags&FCOPY_CURRENTONLY)) && (Flags&FCOPY_COPYLASTTIME))
		{
			SrcPanel->ClearLastGetSelection();
		}
	}

	return COPY_SUCCESS; //COPY_SUCCESS_MOVE???
}



// ��������� ����������� �������. ������� ����� �������� �������� ���� �� �����. ���������� ASAP

COPY_CODES ShellCopy::ShellCopyOneFile(
    const wchar_t *Src,
    const FAR_FIND_DATA_EX &SrcData,
    string &strDest,
    int KeepPathPos,
    int Rename
)
{
	CurCopiedSize = 0; // �������� ������� ��������

	if (CP->Cancelled())
	{
		return(COPY_CANCEL);
	}

	if (UseFilter)
	{
		if (!Filter->FileInFilter(SrcData))
			return COPY_NOFILTER;
	}

	string strDestPath = strDest;
	const wchar_t *NamePtr=PointToName(strDestPath);
	DWORD DestAttr=INVALID_FILE_ATTRIBUTES;

	if (strDestPath.At(0)==L'\\' && strDestPath.At(1)==L'\\')
	{
		string strRoot;
		GetPathRoot(strDestPath, strRoot);
		DeleteEndSlash(strRoot);

		if (!StrCmp(strDestPath,strRoot))
			DestAttr=FILE_ATTRIBUTE_DIRECTORY;
	}

	if (!*NamePtr || TestParentFolderName(NamePtr))
		DestAttr=FILE_ATTRIBUTE_DIRECTORY;

	FAR_FIND_DATA_EX DestData={};
	if (DestAttr==INVALID_FILE_ATTRIBUTES && !(Flags&FCOPY_COPYTONUL))
	{
		if (apiGetFindDataEx(strDestPath,DestData))
			DestAttr=DestData.dwFileAttributes;
	}

	int SameName=0, Append=0;

	if (DestAttr!=INVALID_FILE_ATTRIBUTES && (DestAttr & FILE_ATTRIBUTE_DIRECTORY))
	{
		int CmpCode=CmpFullNames(Src,strDestPath);

		if(CmpCode && SrcData.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT && RPT==RP_EXACTCOPY && !(Flags&FCOPY_COPYSYMLINKCONTENTS))
		{
			CmpCode = 0;
		}

		if (CmpCode==1) // TODO: error check
		{
			SameName=1;

			if (Rename)
			{
				CmpCode=!StrCmp(PointToName(Src),PointToName(strDestPath));
			}

			if (CmpCode==1)
			{
				SetMessageHelp(L"ErrCopyItSelf");
				Message(MSG_WARNING,1,MSG(MError),MSG(MCannotCopyFolderToItself1),
				        Src,MSG(MCannotCopyFolderToItself2),MSG(MOk));
				return(COPY_CANCEL);
			}
		}

		if (!SameName)
		{
			int Length=(int)strDestPath.GetLength();

			if (!IsSlash(strDestPath.At(Length-1)) && strDestPath.At(Length-1)!=L':')
				strDestPath += L"\\";

			const wchar_t *PathPtr=Src+KeepPathPos;

			if (*PathPtr && !KeepPathPos && PathPtr[1]==L':')
				PathPtr+=2;

			if (IsSlash(*PathPtr))
				PathPtr++;

			strDestPath += PathPtr;

			if (!apiGetFindDataEx(strDestPath,DestData))
				DestAttr=INVALID_FILE_ATTRIBUTES;
			else
				DestAttr=DestData.dwFileAttributes;
		}
	}

	if (!(Flags&FCOPY_COPYTONUL) && StrCmpI(strDestPath,L"prn"))
		SetDestDizPath(strDestPath);

	CP->SetProgressValue(0,0);
	CP->SetNames(Src,strDestPath);

	int IsSetSecuty=FALSE;

	if (!(Flags&FCOPY_COPYTONUL))
	{
		// �������� ���������� ��������� �� ������
		switch (CheckStreams(Src,strDestPath))
		{
			case COPY_NEXT:
				return COPY_NEXT;
			case COPY_CANCEL:
				return COPY_CANCEL;
			default:
				break;
		}

		if (SrcData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY ||
		        (SrcData.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT && RPT==RP_EXACTCOPY && !(Flags&FCOPY_COPYSYMLINKCONTENTS)))
		{
			if (!Rename)
				strCopiedName = PointToName(strDestPath);

			if (DestAttr!=INVALID_FILE_ATTRIBUTES)
			{
				if ((DestAttr & FILE_ATTRIBUTE_DIRECTORY) && !SameName)
				{
					DWORD SetAttr=SrcData.dwFileAttributes;

					if (IsDriveTypeCDROM(SrcDriveType) && Opt.ClearReadOnly && (SetAttr & FILE_ATTRIBUTE_READONLY))
						SetAttr&=~FILE_ATTRIBUTE_READONLY;

					if (SetAttr!=DestAttr)
						ShellSetAttr(strDestPath,SetAttr);

					string strSrcFullName;
					ConvertNameToFull(Src,strSrcFullName);
					return(!StrCmp(strDestPath,strSrcFullName) ? COPY_NEXT:COPY_SUCCESS);
				}

				int Type=apiGetFileTypeByName(strDestPath);

				if (Type==FILE_TYPE_CHAR || Type==FILE_TYPE_PIPE)
					return(Rename ? COPY_NEXT:COPY_SUCCESS);
			}

			if (Rename)
			{
				string strSrcFullName,strDestFullName;
				ConvertNameToFull(Src,strSrcFullName);
				SECURITY_ATTRIBUTES sa;

				// ��� Move ��� ���������� ������ ������� ��������, ����� �������� ��� ���������
				if (!(Flags&(FCOPY_COPYSECURITY|FCOPY_LEAVESECURITY)))
				{
					IsSetSecuty=FALSE;

					if (CmpFullPath(Src,strDest)) // � �������� ������ �������� ������ �� ������
						IsSetSecuty=FALSE;
					else if (apiGetFileAttributes(strDest) == INVALID_FILE_ATTRIBUTES) // ���� �������� ���...
					{
						// ...�������� ��������� ��������
						if (GetSecurity(GetParentFolder(strDest,strDestFullName), sa))
							IsSetSecuty=TRUE;
					}
					else if (GetSecurity(strDest,sa)) // ����� �������� ��������� Dest`�
						IsSetSecuty=TRUE;
				}

				// �������� �������������, ���� �� �������
				for (;;)
				{
					BOOL SuccessMove=apiMoveFile(Src,strDestPath);

					if (SuccessMove)
					{
						if (IsSetSecuty)// && !strcmp(DestFSName,"NTFS"))
							SetRecursiveSecurity(strDestPath,sa);

						if (PointToName(strDestPath)==strDestPath.CPtr())
							strRenamedName = strDestPath;
						else
							strCopiedName = PointToName(strDestPath);

						ConvertNameToFull(strDest, strDestFullName);
						TreeList::RenTreeName(strSrcFullName,strDestFullName);
						return(SameName ? COPY_NEXT:COPY_SUCCESS_MOVE);
					}
					else
					{
						int MsgCode = Message(MSG_WARNING|MSG_ERRORTYPE,3,MSG(MError),
						                      MSG(MCopyCannotRenameFolder),Src,MSG(MCopyRetry),
						                      MSG(MCopyIgnore),MSG(MCopyCancel));

						switch (MsgCode)
						{
							case 0:  continue;
							case 1:
							{
								int CopySecurity = Flags&FCOPY_COPYSECURITY;
								SECURITY_ATTRIBUTES tmpsa;

								if ((CopySecurity) && !GetSecurity(Src,tmpsa))
									CopySecurity = FALSE;

								if (apiCreateDirectory(strDestPath,CopySecurity?&tmpsa:nullptr))
								{
									if (PointToName(strDestPath)==strDestPath.CPtr())
										strRenamedName = strDestPath;
									else
										strCopiedName = PointToName(strDestPath);

									TreeList::AddTreeName(strDestPath);
									return(COPY_SUCCESS);
								}
							}
							default:
								return (COPY_CANCEL);
						} /* switch */
					} /* else */
				} /* while */
			} // if (Rename)

			SECURITY_ATTRIBUTES sa;

			if ((Flags&FCOPY_COPYSECURITY) && !GetSecurity(Src,sa))
				return COPY_CANCEL;

			if (RPT!=RP_SYMLINKFILE && SrcData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
			{
				while (!apiCreateDirectoryEx(
					// CreateDirectoryEx preserves reparse points,
					// so we shouldn't use template when copying with content
					((SrcData.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT) && (Flags&FCOPY_COPYSYMLINKCONTENTS))? nullptr : Src,
					strDestPath,(Flags&FCOPY_COPYSECURITY) ? &sa:nullptr))
				{
					int MsgCode=Message(MSG_WARNING|MSG_ERRORTYPE,3,MSG(MError),
					                MSG(MCopyCannotCreateFolder),strDestPath,MSG(MCopyRetry),
					                MSG(MCopySkip),MSG(MCopyCancel));

					if (MsgCode)
						return((MsgCode==-2 || MsgCode==2) ? COPY_CANCEL:COPY_NEXT);
				}

				DWORD SetAttr=SrcData.dwFileAttributes;

				if (IsDriveTypeCDROM(SrcDriveType) && Opt.ClearReadOnly && (SetAttr & FILE_ATTRIBUTE_READONLY))
					SetAttr&=~FILE_ATTRIBUTE_READONLY;

				if ((SetAttr & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
				{
					// �� ����� ���������� ����������, ���� ������� � �������
					// � ������������ FILE_ATTRIBUTE_ENCRYPTED (� �� ��� ����� ��������� ����� CreateDirectory)
					// �.�. ���������� ������ ���.
					if (apiGetFileAttributes(strDestPath)&FILE_ATTRIBUTE_ENCRYPTED)
						SetAttr&=~FILE_ATTRIBUTE_COMPRESSED;

					if (SetAttr&FILE_ATTRIBUTE_COMPRESSED)
					{
						for (;;)
						{
							int MsgCode=ESetFileCompression(strDestPath,1,0,SkipMode);

							if (MsgCode)
							{
								if (MsgCode == SETATTR_RET_SKIP)
									Flags|=FCOPY_SKIPSETATTRFLD;
								else if (MsgCode == SETATTR_RET_SKIPALL)
								{
									Flags|=FCOPY_SKIPSETATTRFLD;
									SkipMode=SETATTR_RET_SKIP;
								}

								break;
							}

							if (MsgCode != SETATTR_RET_OK)
								return (MsgCode==SETATTR_RET_SKIP || MsgCode==SETATTR_RET_SKIPALL) ? COPY_NEXT:COPY_CANCEL;
						}
					}

					while (!ShellSetAttr(strDestPath,SetAttr))
					{
						int MsgCode=Message(MSG_WARNING|MSG_ERRORTYPE,4,MSG(MError),
						                MSG(MCopyCannotChangeFolderAttr),strDestPath,
						                MSG(MCopyRetry),MSG(MCopySkip),MSG(MCopySkipAll),MSG(MCopyCancel));

						if (MsgCode)
						{
							if (MsgCode==1)
								break;

							if (MsgCode==2)
							{
								Flags|=FCOPY_SKIPSETATTRFLD;
								break;
							}

							apiRemoveDirectory(strDestPath);
							return((MsgCode==-2 || MsgCode==3) ? COPY_CANCEL:COPY_NEXT);
						}
					}
				}
				else if (!(Flags & FCOPY_SKIPSETATTRFLD) && ((SetAttr & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY))
				{
					while (!ShellSetAttr(strDestPath,SetAttr))
					{
						int MsgCode=Message(MSG_WARNING|MSG_ERRORTYPE,4,MSG(MError),
						                MSG(MCopyCannotChangeFolderAttr),strDestPath,
						                MSG(MCopyRetry),MSG(MCopySkip),MSG(MCopySkipAll),MSG(MCopyCancel));

						if (MsgCode)
						{
							if (MsgCode==1)
								break;

							if (MsgCode==2)
							{
								Flags|=FCOPY_SKIPSETATTRFLD;
								break;
							}

							apiRemoveDirectory(strDestPath);
							return((MsgCode==-2 || MsgCode==3) ? COPY_CANCEL:COPY_NEXT);
						}
					}
				}
			}

			// [ ] Copy contents of symbolic links
			// For file symbolic links only!!!
			// Directory symbolic links and junction points are handled by CreateDirectoryEx.
			if (!(SrcData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && SrcData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT && !(Flags&FCOPY_COPYSYMLINKCONTENTS) && RPT==RP_EXACTCOPY)
			{
				switch (MkSymLink(Src, strDestPath,RPT,0))
				{
					case 2:
						return COPY_CANCEL;
					case 1:
						break;
					case 0:
						return COPY_FAILURE;
				}
			}

			TreeList::AddTreeName(strDestPath);
			return COPY_SUCCESS;
		}

		if (DestAttr!=INVALID_FILE_ATTRIBUTES && !(DestAttr & FILE_ATTRIBUTE_DIRECTORY))
		{
			if (SrcData.nFileSize==DestData.nFileSize)
			{
				int CmpCode=CmpFullNames(Src,strDestPath);

				if(CmpCode && SrcData.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT && RPT==RP_EXACTCOPY && !(Flags&FCOPY_COPYSYMLINKCONTENTS))
				{
					CmpCode = 0;
				}

				if (CmpCode==1) // TODO: error check
				{
					SameName=1;

					if (Rename)
					{
						CmpCode=!StrCmp(PointToName(Src),PointToName(strDestPath));
					}

					if (CmpCode==1 && !Rename)
					{
						Message(MSG_WARNING,1,MSG(MError),MSG(MCannotCopyFileToItself1),
							    Src,MSG(MCannotCopyFileToItself2),MSG(MOk));
						return(COPY_CANCEL);
					}
				}
			}

			int RetCode=0;
			string strNewName;

			if (!AskOverwrite(SrcData,Src,strDestPath,DestAttr,SameName,Rename,((Flags&FCOPY_LINK)?0:1),Append,strNewName,RetCode))
			{
				return((COPY_CODES)RetCode);
			}

			if (RetCode==COPY_RETRY)
			{
				strDest=strNewName;

				if (CutToSlash(strNewName) && apiGetFileAttributes(strNewName)==INVALID_FILE_ATTRIBUTES)
				{
					CreatePath(strNewName);
				}

				return COPY_RETRY;
			}
		}
	}
	else
	{
		if (SrcData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			return COPY_SUCCESS;
		}
	}

	int NWFS_Attr=(Opt.Nowell.MoveRO && !StrCmp(strDestFSName,L"NWFS"))?TRUE:FALSE;
	{
		for (;;)
		{
			int CopyCode=0;
			unsigned __int64 SaveTotalSize=TotalCopiedSize;

			if (!(Flags&FCOPY_COPYTONUL) && Rename)
			{
				int MoveCode=FALSE,AskDelete;

				if ((!StrCmp(strDestFSName,L"NWFS")) && !Append &&
				        DestAttr!=INVALID_FILE_ATTRIBUTES && !SameName)
				{
					apiDeleteFile(strDestPath); //BUGBUG
				}

				if (!Append)
				{
					string strSrcFullName;
					ConvertNameToFull(Src,strSrcFullName);

					if (NWFS_Attr)
						apiSetFileAttributes(strSrcFullName,SrcData.dwFileAttributes&(~FILE_ATTRIBUTE_READONLY));

					SECURITY_ATTRIBUTES sa;
					IsSetSecuty=FALSE;

					// ��� Move ��� ���������� ������ ������� ��������, ����� �������� ��� ���������
					if (Rename && !(Flags&(FCOPY_COPYSECURITY|FCOPY_LEAVESECURITY)))
					{
						if (CmpFullPath(Src,strDest)) // � �������� ������ �������� ������ �� ������
							IsSetSecuty=FALSE;
						else if (apiGetFileAttributes(strDest) == INVALID_FILE_ATTRIBUTES) // ���� �������� ���...
						{
							string strDestFullName;

							// ...�������� ��������� ��������
							if (GetSecurity(GetParentFolder(strDest,strDestFullName),sa))
								IsSetSecuty=TRUE;
						}
						else if (GetSecurity(strDest,sa)) // ����� �������� ��������� Dest`�
							IsSetSecuty=TRUE;
					}

					if (!StrCmp(strDestFSName,L"NWFS"))
						MoveCode=apiMoveFile(strSrcFullName,strDestPath);
					else
						MoveCode=apiMoveFileEx(strSrcFullName,strDestPath,SameName ? MOVEFILE_COPY_ALLOWED:MOVEFILE_COPY_ALLOWED|MOVEFILE_REPLACE_EXISTING);

					if (!MoveCode)
					{
						int MoveLastError=GetLastError();

						if (NWFS_Attr)
							apiSetFileAttributes(strSrcFullName,SrcData.dwFileAttributes);

						if (MoveLastError==ERROR_NOT_SAME_DEVICE)
							return COPY_FAILURE;

						SetLastError(MoveLastError);
					}
					else
					{
						if (IsSetSecuty)
							SetSecurity(strDestPath,sa);
					}

					if (NWFS_Attr)
						apiSetFileAttributes(strDestPath,SrcData.dwFileAttributes);

					if (ShowTotalCopySize && MoveCode)
					{
						TotalCopiedSize+=SrcData.nFileSize;
						CP->SetTotalProgressValue(TotalCopiedSize,TotalCopySize);
					}

					AskDelete=0;
				}
				else
				{
					do
					{
						DWORD Attr=INVALID_FILE_ATTRIBUTES;
						CopyCode=ShellCopyFile(Src,SrcData,strDestPath,Attr,Append);
					}
					while (CopyCode==COPY_RETRY);

					switch (CopyCode)
					{
						case COPY_SUCCESS:
							MoveCode=TRUE;
							break;
						case COPY_FAILUREREAD:
						case COPY_FAILURE:
							MoveCode=FALSE;
							break;
						case COPY_CANCEL:
							return COPY_CANCEL;
						case COPY_NEXT:
							return COPY_NEXT;
					}

					AskDelete=1;
				}

				if (MoveCode)
				{
					if (DestAttr==INVALID_FILE_ATTRIBUTES || !(DestAttr & FILE_ATTRIBUTE_DIRECTORY))
					{
						if (PointToName(strDestPath)==strDestPath.CPtr())
							strRenamedName = strDestPath;
						else
							strCopiedName = PointToName(strDestPath);
					}

					if (IsDriveTypeCDROM(SrcDriveType) && Opt.ClearReadOnly &&
					        (SrcData.dwFileAttributes & FILE_ATTRIBUTE_READONLY))
						ShellSetAttr(strDestPath,SrcData.dwFileAttributes & (~FILE_ATTRIBUTE_READONLY));

					TotalFiles++;

					if (AskDelete && DeleteAfterMove(Src,SrcData.dwFileAttributes)==COPY_CANCEL)
						return COPY_CANCEL;

					return(COPY_SUCCESS_MOVE);
				}
			}
			else
			{
				do
				{
					CopyCode=ShellCopyFile(Src,SrcData,strDestPath,DestAttr,Append);
				}
				while (CopyCode==COPY_RETRY);

				if (CopyCode==COPY_SUCCESS)
				{
					strCopiedName = PointToName(strDestPath);

					if (!(Flags&FCOPY_COPYTONUL))
					{
						if (IsDriveTypeCDROM(SrcDriveType) && Opt.ClearReadOnly &&
						        (SrcData.dwFileAttributes & FILE_ATTRIBUTE_READONLY))
							ShellSetAttr(strDestPath,SrcData.dwFileAttributes & ~FILE_ATTRIBUTE_READONLY);

						if (DestAttr!=INVALID_FILE_ATTRIBUTES && !StrCmpI(strCopiedName,DestData.strFileName) &&
						        StrCmp(strCopiedName,DestData.strFileName))
							apiMoveFile(strDestPath,strDestPath); //???
					}

					TotalFiles++;

					if (DestAttr!=INVALID_FILE_ATTRIBUTES && Append)
						apiSetFileAttributes(strDestPath,DestAttr);

					return COPY_SUCCESS;
				}
				else if (CopyCode==COPY_CANCEL || CopyCode==COPY_NEXT)
				{
					if (DestAttr!=INVALID_FILE_ATTRIBUTES && Append)
						apiSetFileAttributes(strDestPath,DestAttr);

					return((COPY_CODES)CopyCode);
				}

				if (DestAttr!=INVALID_FILE_ATTRIBUTES && Append)
					apiSetFileAttributes(strDestPath,DestAttr);
			}

			//????
			if (CopyCode == COPY_FAILUREREAD)
				return COPY_FAILURE;

			//????
			string strMsg1, strMsg2;
			int MsgMCannot=(Flags&FCOPY_LINK) ? MCannotLink: (Flags&FCOPY_MOVE) ? MCannotMove: MCannotCopy;
			strMsg1 = Src;
			strMsg2 = strDestPath;
			InsertQuote(strMsg1);
			InsertQuote(strMsg2);

			int MsgCode;

			if (SkipMode!=-1)
				MsgCode=SkipMode;
			else
			{
				MsgCode=Message(MSG_WARNING|MSG_ERRORTYPE,4,MSG(MError),
						        MSG(MsgMCannot),
						        strMsg1,
						        MSG(MCannotCopyTo),
						        strMsg2,
						        MSG(MCopyRetry),MSG(MCopySkip),
						        MSG(MCopySkipAll),MSG(MCopyCancel));
			}

			switch (MsgCode)
			{
				case -1:
				case  1:
					return COPY_NEXT;
				case  2:
					SkipMode=1;
					return COPY_NEXT;
				case -2:
				case  3:
					return COPY_CANCEL;
			}

			TotalCopiedSize=SaveTotalSize;
			int RetCode;
			string strNewName;

			if (!AskOverwrite(SrcData,Src,strDestPath,DestAttr,SameName,Rename,((Flags&FCOPY_LINK)?0:1),Append,strNewName,RetCode))
				return((COPY_CODES)RetCode);

			if (RetCode==COPY_RETRY)
			{
				strDest=strNewName;

				if (CutToSlash(strNewName) && apiGetFileAttributes(strNewName)==INVALID_FILE_ATTRIBUTES)
				{
					CreatePath(strNewName);
				}

				return COPY_RETRY;
			}
		}
	}
}


// �������� ���������� ��������� �� ������
COPY_CODES ShellCopy::CheckStreams(const wchar_t *Src,const wchar_t *DestPath)
{

#if 0
	int AscStreams=(Flags&FCOPY_STREAMSKIP)?2:((Flags&FCOPY_STREAMALL)?0:1);

	if (!(Flags&FCOPY_USESYSTEMCOPY) && AscStreams)
	{
		int CountStreams=EnumNTFSStreams(Src,nullptr,nullptr);

		if (CountStreams > 1 ||
		        (CountStreams >= 1 && (GetFileAttributes(Src)&FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY))
		{
			if (AscStreams == 2)
			{
				return(COPY_NEXT);
			}

			SetMessageHelp("WarnCopyStream");
			//char SrcFullName[NM];
			//ConvertNameToFull(Src,SrcFullName, sizeof(SrcFullName));
			//TruncPathStr(SrcFullName,ScrX-16);
			int MsgCode=Message(MSG_WARNING,5,MSG(MWarning),
			                    MSG(MCopyStream1),
			                    MSG(CanCreateHardLinks(DestPath,nullptr)?MCopyStream2:MCopyStream3),
			                    MSG(MCopyStream4),"\1",//SrcFullName,"\1",
			                    MSG(MCopyResume),MSG(MCopyOverwriteAll),MSG(MCopySkipOvr),MSG(MCopySkipAllOvr),MSG(MCopyCancelOvr));

			switch (MsgCode)
			{
				case 0: break;
				case 1: Flags|=FCOPY_STREAMALL; break;
				case 2: return(COPY_NEXT);
				case 3: Flags|=FCOPY_STREAMSKIP; return(COPY_NEXT);
				default:
					return COPY_CANCEL;
			}
		}
	}

#endif
	return COPY_SUCCESS;
}

int ShellCopy::DeleteAfterMove(const wchar_t *Name,DWORD Attr)
{
	if (Attr & FILE_ATTRIBUTE_READONLY)
	{
		int MsgCode;

		if (!Opt.Confirm.RO)
			ReadOnlyDelMode=1;

		if (ReadOnlyDelMode!=-1)
			MsgCode=ReadOnlyDelMode;
		else
			MsgCode=Message(MSG_WARNING,5,MSG(MWarning),
			                MSG(MCopyFileRO),Name,MSG(MCopyAskDelete),
			                MSG(MCopyDeleteRO),MSG(MCopyDeleteAllRO),
			                MSG(MCopySkipRO),MSG(MCopySkipAllRO),MSG(MCopyCancelRO));

		switch (MsgCode)
		{
			case 1:
				ReadOnlyDelMode=1;
				break;
			case 2:
				return(COPY_NEXT);
			case 3:
				ReadOnlyDelMode=3;
				return(COPY_NEXT);
			case -1:
			case -2:
			case 4:
				return(COPY_CANCEL);
		}

		apiSetFileAttributes(Name,FILE_ATTRIBUTE_NORMAL);
	}

	while ((Attr&FILE_ATTRIBUTE_DIRECTORY)?!apiRemoveDirectory(Name):!apiDeleteFile(Name))
	{
		int MsgCode;

		if (SkipDeleteMode!=-1)
			MsgCode=SkipDeleteMode;
		else
			MsgCode=Message(MSG_WARNING|MSG_ERRORTYPE,4,MSG(MError),MSG(MCannotDeleteFile),Name,
			                MSG(MDeleteRetry),MSG(MDeleteSkip),MSG(MDeleteSkipAll),MSG(MDeleteCancel));

		switch (MsgCode)
		{
			case 1:
				return COPY_NEXT;
			case 2:
				SkipDeleteMode=1;
				return COPY_NEXT;
			case -1:
			case -2:
			case 3:
				return(COPY_CANCEL);
		}
	}

	return(COPY_SUCCESS);
}



int ShellCopy::ShellCopyFile(const wchar_t *SrcName,const FAR_FIND_DATA_EX &SrcData,
                             string &strDestName,DWORD &DestAttr,int Append)
{
	OrigScrX=ScrX;
	OrigScrY=ScrY;

	if ((Flags&FCOPY_LINK))
	{
		if (RPT==RP_HARDLINK)
		{
			apiDeleteFile(strDestName); //BUGBUG
			return(MkHardLink(SrcName,strDestName) ? COPY_SUCCESS:COPY_FAILURE);
		}
		else
		{
			return(MkSymLink(SrcName,strDestName,RPT,0) ? COPY_SUCCESS:COPY_FAILURE);
		}
	}

	if ((SrcData.dwFileAttributes&FILE_ATTRIBUTE_ENCRYPTED) &&
	        !CheckDisksProps(SrcName,strDestName,CHECKEDPROPS_ISDST_ENCRYPTION)
	   )
	{
		int MsgCode;

		if (SkipEncMode!=-1)
		{
			MsgCode=SkipEncMode;

			if (SkipEncMode == 1)
				Flags|=FCOPY_DECRYPTED_DESTINATION;
		}
		else
		{
			SetMessageHelp(L"WarnCopyEncrypt");
			string strSrcName = SrcName;
			InsertQuote(strSrcName);
			MsgCode=Message(MSG_WARNING,3,MSG(MWarning),
			                MSG(MCopyEncryptWarn1),
			                strSrcName,
			                MSG(MCopyEncryptWarn2),
			                MSG(MCopyEncryptWarn3),
			                MSG(MCopyIgnore),MSG(MCopyIgnoreAll),MSG(MCopyCancel));
		}

		switch (MsgCode)
		{
			case  0:
				_LOGCOPYR(SysLog(L"return COPY_NEXT -> %d",__LINE__));
				Flags|=FCOPY_DECRYPTED_DESTINATION;
				break;//return COPY_NEXT;
			case  1:
				SkipEncMode=1;
				Flags|=FCOPY_DECRYPTED_DESTINATION;
				_LOGCOPYR(SysLog(L"return COPY_NEXT -> %d",__LINE__));
				break;//return COPY_NEXT;
			case -1:
			case -2:
			case  2:
				_LOGCOPYR(SysLog(L"return COPY_CANCEL -> %d",__LINE__));
				return COPY_CANCEL;
		}
	}

	if (!(Flags&FCOPY_COPYTONUL)&&(Flags&FCOPY_USESYSTEMCOPY)&&!Append)
	{
		if (!(SrcData.dwFileAttributes&FILE_ATTRIBUTE_ENCRYPTED) ||
		        ((SrcData.dwFileAttributes&FILE_ATTRIBUTE_ENCRYPTED) &&
		         ((WinVer.dwMajorVersion >= 5 && WinVer.dwMinorVersion > 0) ||
		          !(Flags&(FCOPY_DECRYPTED_DESTINATION))))
		   )
		{
			if (!Opt.CMOpt.CopyOpened)
			{
				File SrcFile;
				if (!SrcFile.Open(SrcName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN))
				{
					_LOGCOPYR(SysLog(L"return COPY_FAILURE -> %d if (SrcHandle==INVALID_HANDLE_VALUE)",__LINE__));
					return COPY_FAILURE;
				}

				SrcFile.Close();
			}

			//_LOGCOPYR(SysLog(L"call ShellSystemCopy('%s','%s',%p)",SrcName,DestName,SrcData));
			return(ShellSystemCopy(SrcName,strDestName,SrcData));
		}
	}

	SECURITY_ATTRIBUTES sa;
	if ((Flags&FCOPY_COPYSECURITY) && !GetSecurity(SrcName,sa))
		return COPY_CANCEL;

	int OpenMode=FILE_SHARE_READ;

	if (Opt.CMOpt.CopyOpened)
		OpenMode|=FILE_SHARE_WRITE;

	File SrcFile;
	bool Opened = SrcFile.Open(SrcName, GENERIC_READ, OpenMode, nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN);

	if (!Opened && Opt.CMOpt.CopyOpened)
	{
		_localLastError=GetLastError();

		if (_localLastError == ERROR_SHARING_VIOLATION)
		{
			Opened = SrcFile.Open(SrcName, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN);
		}
	}

	if (!Opened)
	{
		_localLastError=GetLastError();
		return COPY_FAILURE;
	}

	File DestFile;
	__int64 AppendPos=0;

	bool CopySparse=false;

	if (!(Flags&FCOPY_COPYTONUL))
	{
		//if (DestAttr!=INVALID_FILE_ATTRIBUTES && !Append) //��� ��� ������ ����������� ������ ����������
		//apiDeleteFile(DestName);
		bool DstOpened = DestFile.Open(strDestName, GENERIC_WRITE, FILE_SHARE_READ, (Flags&FCOPY_COPYSECURITY) ? &sa:nullptr, (Append ? OPEN_EXISTING:CREATE_ALWAYS), SrcData.dwFileAttributes&(~((Flags&(FCOPY_DECRYPTED_DESTINATION))?FILE_ATTRIBUTE_ENCRYPTED|FILE_FLAG_SEQUENTIAL_SCAN:FILE_FLAG_SEQUENTIAL_SCAN)));
		Flags&=~FCOPY_DECRYPTED_DESTINATION;

		if (!DstOpened)
		{
			_localLastError=GetLastError();
			SrcFile.Close();
			_LOGCOPYR(SysLog(L"return COPY_FAILURE -> %d CreateFile=-1, LastError=%d (0x%08X)",__LINE__,_localLastError,_localLastError));
			return COPY_FAILURE;
		}

		string strDriveRoot;
		GetPathRoot(strDestName,strDriveRoot);

		if (SrcData.dwFileAttributes&FILE_ATTRIBUTE_SPARSE_FILE)
		{
			DWORD VolFlags=0;
			if(apiGetVolumeInformation(strDriveRoot,nullptr,nullptr,nullptr,&VolFlags,nullptr))
			{
				if(VolFlags&FILE_SUPPORTS_SPARSE_FILES)
				{
					DWORD Temp;
					if (DestFile.IoControl(FSCTL_SET_SPARSE, nullptr, 0, nullptr, 0, &Temp))
					{
						CopySparse=true;
					}
				}
			}
		}

		if (Append)
		{
			if (!DestFile.SetPointer(0,&AppendPos,FILE_END))
			{
				_localLastError=GetLastError();
				SrcFile.Close();
				DestFile.SetEnd();
				DestFile.Close();
				return COPY_FAILURE;
			}
		}

		// ���� ����� � �������� ������� - ����� �����.
		UINT64 FreeBytes=0;
		if (apiGetDiskSize(strDriveRoot,nullptr,nullptr,&FreeBytes))
		{
			if (FreeBytes>SrcData.nFileSize)
			{
				INT64 CurPtr=DestFile.GetPointer();
				if (DestFile.SetPointer(SrcData.nFileSize,nullptr,FILE_CURRENT) && DestFile.SetEnd())
					DestFile.SetPointer(CurPtr,nullptr,FILE_BEGIN);
			}
		}
	}

	int   AbortOp = FALSE;
	BOOL SparseQueryResult=TRUE;
	FILE_ALLOCATED_RANGE_BUFFER queryrange;
	static FILE_ALLOCATED_RANGE_BUFFER ranges[1024];
	queryrange.FileOffset.QuadPart = 0;
	queryrange.Length.QuadPart = SrcData.nFileSize;
	CP->SetProgressValue(0,0);

	do
	{
		DWORD n=0,nbytes=0;

		if (CopySparse)
		{
			SparseQueryResult=SrcFile.IoControl(FSCTL_QUERY_ALLOCATED_RANGES, &queryrange, sizeof(queryrange), ranges, sizeof(ranges), &nbytes);

			if (!SparseQueryResult && GetLastError()!=ERROR_MORE_DATA)
				break;

			n=nbytes/sizeof(FILE_ALLOCATED_RANGE_BUFFER);
		}

		for (DWORD i=0; i<(CopySparse?n:i+1); i++)
		{
			INT64 Size=0;

			if (CopySparse)
			{
				Size=ranges[i].Length.QuadPart;
				SrcFile.SetPointer(ranges[i].FileOffset.QuadPart,nullptr,FILE_BEGIN);
				INT64 DestPos=ranges[i].FileOffset.QuadPart;

				if (Append)
					DestPos+=AppendPos;

				DestFile.SetPointer(DestPos,nullptr,FILE_BEGIN);
			}

			DWORD BytesRead,BytesWritten;

			while (CopySparse?(Size>0):true)
			{
				BOOL IsChangeConsole=OrigScrX != ScrX || OrigScrY != ScrY;

				if (CP->Cancelled())
				{
					AbortOp=true;
				}

				IsChangeConsole=CheckAndUpdateConsole(IsChangeConsole);

				if (IsChangeConsole)
				{
					OrigScrX=ScrX;
					OrigScrY=ScrY;
					PR_ShellCopyMsg();
				}

				CP->SetProgressValue(CurCopiedSize,SrcData.nFileSize);

				if (ShowTotalCopySize)
				{
					CP->SetTotalProgressValue(TotalCopiedSize,TotalCopySize);
				}

				if (AbortOp)
				{
					SrcFile.Close();

					if (!(Flags&FCOPY_COPYTONUL))
					{
						if (Append)
						{
							DestFile.SetPointer(AppendPos,nullptr,FILE_BEGIN);
						}

						DestFile.SetEnd();
						DestFile.Close();

						if (!Append)
						{
							apiSetFileAttributes(strDestName,FILE_ATTRIBUTE_NORMAL);
							apiDeleteFile(strDestName); //BUGBUG
						}
					}

					return COPY_CANCEL;
				}

				while (!SrcFile.Read(CopyBuffer,(CopySparse?(DWORD)Min((LONGLONG)CopyBufferSize,Size):CopyBufferSize),BytesRead,nullptr))
				{
					int MsgCode = Message(MSG_WARNING|MSG_ERRORTYPE,2,MSG(MError),
					                      MSG(MCopyReadError),SrcName,
					                      MSG(MRetry),MSG(MCancel));
					PR_ShellCopyMsg();

					if (!MsgCode)
						continue;

					DWORD LastError=GetLastError();
					SrcFile.Close();

					if (!(Flags&FCOPY_COPYTONUL))
					{
						if (Append)
						{
							DestFile.SetPointer(AppendPos,nullptr,FILE_BEGIN);
						}

						DestFile.SetEnd();
						DestFile.Close();

						if (!Append)
						{
							apiSetFileAttributes(strDestName,FILE_ATTRIBUTE_NORMAL);
							apiDeleteFile(strDestName); //BUGBUG
						}
					}

					CP->SetProgressValue(0,0);
					SetLastError(_localLastError=LastError);
					CurCopiedSize = 0; // �������� ������� ��������
					return COPY_FAILURE;
				}

				if (!BytesRead)
				{
					SparseQueryResult=FALSE;
					break;
				}

				if (!(Flags&FCOPY_COPYTONUL))
				{
					while (!DestFile.Write(CopyBuffer,BytesRead,BytesWritten,nullptr))
					{
						DWORD LastError=GetLastError();
						int Split=FALSE,SplitCancelled=FALSE,SplitSkipped=FALSE;

						if ((LastError==ERROR_DISK_FULL || LastError==ERROR_HANDLE_DISK_FULL) &&
						        !strDestName.IsEmpty() && strDestName.At(1)==L':')
						{
							string strDriveRoot;
							GetPathRoot(strDestName,strDriveRoot);
							UINT64 FreeSize=0;

							if (apiGetDiskSize(strDriveRoot,nullptr,nullptr,&FreeSize))
							{
								if (FreeSize<BytesRead &&
								        DestFile.Write(CopyBuffer,(DWORD)FreeSize,BytesWritten,nullptr) &&
										SrcFile.SetPointer(FreeSize-BytesRead,nullptr,FILE_CURRENT))
								{
									DestFile.Close();
									SetMessageHelp(L"CopyFiles");
									int MsgCode=Message(MSG_WARNING,4,MSG(MError),
									                    MSG(MErrorInsufficientDiskSpace),strDestName,
									                    MSG(MSplit),MSG(MSkip),MSG(MRetry),MSG(MCancel));
									PR_ShellCopyMsg();

									if (MsgCode==2)
									{
										SrcFile.Close();

										if (!Append)
										{
											apiSetFileAttributes(strDestName,FILE_ATTRIBUTE_NORMAL);
											apiDeleteFile(strDestName); //BUGBUG
										}

										return COPY_FAILURE;
									}

									if (!MsgCode)
									{
										Split=TRUE;

										for (;;)
										{
											if (apiGetDiskSize(strDriveRoot,nullptr,nullptr,&FreeSize))
												if (FreeSize<BytesRead)
												{
													int MsgCode2 = Message(MSG_WARNING,2,MSG(MWarning),
													                       MSG(MCopyErrorDiskFull),strDestName,
													                       MSG(MRetry),MSG(MCancel));
													PR_ShellCopyMsg();

													if (MsgCode2)
													{
														Split=FALSE;
														SplitCancelled=TRUE;
													}
													else
														continue;
												}

											break;
										}
									}

									if (MsgCode==1)
										SplitSkipped=TRUE;

									if (MsgCode==-1 || MsgCode==3)
										SplitCancelled=TRUE;
								}
							}
						}

						if (Split)
						{
							INT64 FilePtr=SrcFile.GetPointer();
							FAR_FIND_DATA_EX SplitData=SrcData;
							SplitData.nFileSize-=FilePtr;
							int RetCode;
							string strNewName;

							if (!AskOverwrite(SplitData,SrcName,strDestName,INVALID_FILE_ATTRIBUTES,FALSE,((Flags&FCOPY_MOVE)?TRUE:FALSE),((Flags&FCOPY_LINK)?0:1),Append,strNewName,RetCode))
							{
								SrcFile.Close();
								return(COPY_CANCEL);
							}

							if (RetCode==COPY_RETRY)
							{
								strDestName=strNewName;

								if (CutToSlash(strNewName) && apiGetFileAttributes(strNewName)==INVALID_FILE_ATTRIBUTES)
								{
									CreatePath(strNewName);
								}

								return COPY_RETRY;
							}

							string strDestDir = strDestName;

							if (CutToSlash(strDestDir,true))
								CreatePath(strDestDir);

							;

							if (!DestFile.Open(strDestName, GENERIC_WRITE, FILE_SHARE_READ, nullptr, (Append ? OPEN_EXISTING:CREATE_ALWAYS), SrcData.dwFileAttributes|FILE_FLAG_SEQUENTIAL_SCAN) || (Append && !DestFile.SetPointer(0,nullptr,FILE_END)))
							{
								_localLastError=GetLastError();
								SrcFile.Close();
								DestFile.Close();
								return COPY_FAILURE;
							}
						}
						else
						{
							if (!SplitCancelled && !SplitSkipped &&
							        !Message(MSG_WARNING|MSG_ERRORTYPE,2,MSG(MError),
							                MSG(MCopyWriteError),strDestName,MSG(MRetry),MSG(MCancel)))
							{
								continue;
							}

							SrcFile.Close();

							if (Append)
							{
								DestFile.SetPointer(AppendPos,nullptr,FILE_BEGIN);
							}

							DestFile.SetEnd();
							DestFile.Close();

							if (!Append)
							{
								apiSetFileAttributes(strDestName,FILE_ATTRIBUTE_NORMAL);
								apiDeleteFile(strDestName); //BUGBUG
							}

							CP->SetProgressValue(0,0);
							SetLastError(_localLastError=LastError);

							if (SplitSkipped)
								return COPY_NEXT;

							return(SplitCancelled ? COPY_CANCEL:COPY_FAILURE);
						}

						break;
					}
				}
				else
				{
					BytesWritten=BytesRead; // �� ������� ���������� ���������� ���������� ����
				}

				CurCopiedSize+=BytesWritten;

				if (ShowTotalCopySize)
					TotalCopiedSize+=BytesWritten;

				CP->SetProgressValue(CurCopiedSize,SrcData.nFileSize);

				if (ShowTotalCopySize)
				{
					CP->SetTotalProgressValue(TotalCopiedSize,TotalCopySize);
				}

				CP->SetNames(SrcData.strFileName,strDestName);

				if (CopySparse)
					Size -= BytesRead;
			}

			if (!CopySparse || !SparseQueryResult)
				break;
		} /* for */

		if (!SparseQueryResult)
			break;

		if (CopySparse)
		{
			if (!SparseQueryResult && n>0)
			{
				queryrange.FileOffset.QuadPart=ranges[n-1].FileOffset.QuadPart+ranges[n-1].Length.QuadPart;
				queryrange.Length.QuadPart = SrcData.nFileSize-queryrange.FileOffset.QuadPart;
			}
		}
	}
	while (!SparseQueryResult && CopySparse);

	if (!(Flags&FCOPY_COPYTONUL))
	{
		DestFile.SetTime(nullptr, nullptr, &SrcData.ftLastWriteTime, nullptr);
		SrcFile.Close();

		if (CopySparse)
		{
			INT64 Pos=SrcData.nFileSize;

			if (Append)
				Pos+=AppendPos;

			DestFile.SetPointer(Pos,nullptr,FILE_BEGIN);
			DestFile.SetEnd();
		}

		DestFile.Close();
		// TODO: ����� ������� Compressed???
		Flags&=~FCOPY_DECRYPTED_DESTINATION;
	}
	else
		SrcFile.Close();

	return COPY_SUCCESS;
}

void ShellCopy::SetDestDizPath(const wchar_t *DestPath)
{
	if (!(Flags&FCOPY_DIZREAD))
	{
		ConvertNameToFull(DestPath, strDestDizPath);
		CutToSlash(strDestDizPath);

		if (strDestDizPath.IsEmpty())
			strDestDizPath = L".";

		if ((Opt.Diz.UpdateMode==DIZ_UPDATE_IF_DISPLAYED && !SrcPanel->IsDizDisplayed()) ||
		        Opt.Diz.UpdateMode==DIZ_NOT_UPDATE)
			strDestDizPath.Clear();

		if (!strDestDizPath.IsEmpty())
			DestDiz.Read(strDestDizPath);

		Flags|=FCOPY_DIZREAD;
	}
}

enum WarnDlgItems
{
	WDLG_BORDER,
	WDLG_TEXT,
	WDLG_FILENAME,
	WDLG_SEPARATOR,
	WDLG_SRCFILEBTN,
	WDLG_DSTFILEBTN,
	WDLG_SEPARATOR2,
	WDLG_CHECKBOX,
	WDLG_SEPARATOR3,
	WDLG_OVERWRITE,
	WDLG_SKIP,
	WDLG_RENAME,
	WDLG_APPEND,
	WDLG_CANCEL,
};

enum
{
 DM_OPENVIEWER = DM_USER+33,
};

INT_PTR WINAPI WarnDlgProc(HANDLE hDlg,int Msg,int Param1,void* Param2)
{
	switch (Msg)
	{
		case DM_OPENVIEWER:
		{
			LPCWSTR ViewName=nullptr;
			string** WFN=reinterpret_cast<string**>(SendDlgMessage(hDlg,DM_GETDLGDATA,0,0));

			if (WFN)
			{
				switch (Param1)
				{
					case WDLG_SRCFILEBTN:
						ViewName=*WFN[0];
						break;
					case WDLG_DSTFILEBTN:
						ViewName=*WFN[1];
						break;
				}

				FileViewer Viewer(ViewName,FALSE,FALSE,TRUE,-1,nullptr,nullptr,FALSE);
				Viewer.SetDynamicallyBorn(FALSE);
				// � ���� ���� �� ���� ������������ ������� ������� ������� �� CtrlF10 � ���� ������ � ����������� �����:
				Viewer.SetTempViewName(L"nul",FALSE);
				FrameManager->EnterModalEV();
				FrameManager->ExecuteModal();
				FrameManager->ExitModalEV();
				FrameManager->ProcessKey(KEY_CONSOLE_BUFFER_RESIZE);
			}
		}
		break;
		case DN_CTLCOLORDLGITEM:
		{
			if (Param1==WDLG_FILENAME)
			{
				FarColor Color=ColorIndexToColor(COL_WARNDIALOGTEXT);
				FarDialogItemColors* Colors = static_cast<FarDialogItemColors*>(Param2);
				Colors->Colors[0] = Color;
				Colors->Colors[2] = Color;
			}
		}
		break;
		case DN_BTNCLICK:
		{
			switch (Param1)
			{
				case WDLG_SRCFILEBTN:
				case WDLG_DSTFILEBTN:
					SendDlgMessage(hDlg,DM_OPENVIEWER,Param1,0);
					break;
				case WDLG_RENAME:
				{
					string** WFN=reinterpret_cast<string**>(SendDlgMessage(hDlg,DM_GETDLGDATA,0,0));
					string strDestName=*WFN[1];
					GenerateName(strDestName,*WFN[2]);

					if (SendDlgMessage(hDlg,DM_GETCHECK,WDLG_CHECKBOX,0)==BSTATE_UNCHECKED)
					{
						int All=BSTATE_UNCHECKED;

						if (GetString(MSG(MCopyRenameTitle),MSG(MCopyRenameText),nullptr,strDestName,*WFN[1],L"CopyAskOverwrite",FIB_BUTTONS|FIB_NOAMPERSAND|FIB_EXPANDENV|FIB_CHECKBOX,&All,MSG(MCopyRememberChoice)))
						{
							if (All!=BSTATE_UNCHECKED)
							{
								*WFN[2]=*WFN[1];
								CutToSlash(*WFN[2]);
							}

							SendDlgMessage(hDlg,DM_SETCHECK,WDLG_CHECKBOX,ToPtr(All));
						}
						else
						{
							return TRUE;
						}
					}
					else
					{
						*WFN[1]=strDestName;
					}
				}
				break;
			}
		}
		break;
		case DN_CONTROLINPUT:
		{
			const INPUT_RECORD* record=(const INPUT_RECORD *)Param2;
			if (record->EventType==KEY_EVENT)
			{
				int key = InputRecordToKey(record);
				if ((Param1==WDLG_SRCFILEBTN || Param1==WDLG_DSTFILEBTN) && key==KEY_F3)
				{
					SendDlgMessage(hDlg,DM_OPENVIEWER,Param1,0);
				}
			}
		}
		break;

		default:
			break;
	}

	return DefDlgProc(hDlg,Msg,Param1,Param2);
}

int ShellCopy::AskOverwrite(const FAR_FIND_DATA_EX &SrcData,
                            const wchar_t *SrcName,
                            const wchar_t *DestName, DWORD DestAttr,
                            int SameName,int Rename,int AskAppend,
                            int &Append,string &strNewName,int &RetCode)
{
	enum
	{
		WARN_DLG_HEIGHT=13,
		WARN_DLG_WIDTH=72,
	};
	FarDialogItem WarnCopyDlgData[]=
	{
		{DI_DOUBLEBOX,3,1,WARN_DLG_WIDTH-4,WARN_DLG_HEIGHT-2,0,nullptr,nullptr,0,MSG(MWarning)},
		{DI_TEXT,5,2,WARN_DLG_WIDTH-6,2,0,nullptr,nullptr,DIF_CENTERTEXT,MSG(MCopyFileExist)},
		{DI_EDIT,5,3,WARN_DLG_WIDTH-6,3,0,nullptr,nullptr,DIF_READONLY,(wchar_t*)DestName},
		{DI_TEXT,3,4,0,4,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_BUTTON,5,5,WARN_DLG_WIDTH-6,5,0,nullptr,nullptr,DIF_BTNNOCLOSE|DIF_NOBRACKETS,L""},
		{DI_BUTTON,5,6,WARN_DLG_WIDTH-6,6,0,nullptr,nullptr,DIF_BTNNOCLOSE|DIF_NOBRACKETS,L""},
		{DI_TEXT,3,7,0,7,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_CHECKBOX,5,8,0,8,0,nullptr,nullptr,DIF_FOCUS,MSG(MCopyRememberChoice)},
		{DI_TEXT,3,9,0,9,0,nullptr,nullptr,DIF_SEPARATOR,L""},

		{DI_BUTTON,0,10,0,10,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_CENTERGROUP,MSG(MCopyOverwrite)},
		{DI_BUTTON,0,10,0,10,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MCopySkipOvr)},
		{DI_BUTTON,0,10,0,10,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MCopyRename)},
		{DI_BUTTON,0,10,0,10,0,nullptr,nullptr,DIF_CENTERGROUP|(AskAppend?0:(DIF_DISABLE|DIF_HIDDEN)),MSG(MCopyAppend)},
		{DI_BUTTON,0,10,0,10,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MCopyCancelOvr)},
	};
	FAR_FIND_DATA_EX DestData;
	DestData.Clear();
	int DestDataFilled=FALSE;
	Append=FALSE;

	if ((Flags&FCOPY_COPYTONUL))
	{
		RetCode=COPY_NEXT;
		return TRUE;
	}

	if (DestAttr==INVALID_FILE_ATTRIBUTES)
		if ((DestAttr=apiGetFileAttributes(DestName))==INVALID_FILE_ATTRIBUTES)
			return TRUE;

	if (DestAttr & FILE_ATTRIBUTE_DIRECTORY)
		return TRUE;

	int MsgCode=OvrMode;
	string strDestName=DestName;

	if (OvrMode==-1)
	{
		int Type;

		if ((!Opt.Confirm.Copy && !Rename) || (!Opt.Confirm.Move && Rename) ||
		        SameName || (Type=apiGetFileTypeByName(DestName))==FILE_TYPE_CHAR ||
		        Type==FILE_TYPE_PIPE || (Flags&FCOPY_OVERWRITENEXT))
			MsgCode=1;
		else
		{
			DestData.Clear();
			apiGetFindDataEx(DestName,DestData);
			DestDataFilled=TRUE;

			if ((Flags&FCOPY_ONLYNEWERFILES))
			{
				// ������� �����
				__int64 RetCompare=FileTimeDifference(&DestData.ftLastWriteTime,&SrcData.ftLastWriteTime);

				if (RetCompare < 0)
					MsgCode=0;
				else
					MsgCode=2;
			}
			else
			{
				FormatString strSrcFileStr, strDestFileStr;
				unsigned __int64 SrcSize = SrcData.nFileSize;
				FILETIME SrcLastWriteTime = SrcData.ftLastWriteTime;
				if(Flags&FCOPY_COPYSYMLINKCONTENTS && SrcData.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT)
				{
					string RealName;
					ConvertNameToReal(SrcName, RealName);
					FAR_FIND_DATA_EX FindData;
					apiGetFindDataEx(RealName,FindData);
					SrcSize=FindData.nFileSize;
					SrcLastWriteTime = FindData.ftLastWriteTime;

				}
				FormatString strSrcSizeText;
				strSrcSizeText<<SrcSize;
				unsigned __int64 DestSize = DestData.nFileSize;
				FormatString strDestSizeText;
				strDestSizeText<<DestSize;
				string strDateText, strTimeText;
				ConvertDate(SrcLastWriteTime,strDateText,strTimeText,8,FALSE,FALSE,TRUE,TRUE);
				strSrcFileStr<<fmt::LeftAlign()<<fmt::Width(17)<<MSG(MCopySource)<<L" "<<fmt::Width(25)<<fmt::Precision(25)<<strSrcSizeText<<L" "<<strDateText<<L" "<<strTimeText;
				ConvertDate(DestData.ftLastWriteTime,strDateText,strTimeText,8,FALSE,FALSE,TRUE,TRUE);
				strDestFileStr<<fmt::LeftAlign()<<fmt::Width(17)<<MSG(MCopyDest)<<L" "<<fmt::Width(25)<<fmt::Precision(25)<<strDestSizeText<<L" "<<strDateText<<L" "<<strTimeText;

				WarnCopyDlgData[WDLG_SRCFILEBTN].Data=strSrcFileStr;
				WarnCopyDlgData[WDLG_DSTFILEBTN].Data=strDestFileStr;
				MakeDialogItemsEx(WarnCopyDlgData,WarnCopyDlg);
				string strFullSrcName;
				ConvertNameToFull(SrcName,strFullSrcName);
				string *WFN[]={&strFullSrcName,&strDestName,&strRenamedFilesPath};
				Dialog WarnDlg(WarnCopyDlg,ARRAYSIZE(WarnCopyDlg),WarnDlgProc,&WFN);
				WarnDlg.SetDialogMode(DMODE_WARNINGSTYLE);
				WarnDlg.SetPosition(-1,-1,WARN_DLG_WIDTH,WARN_DLG_HEIGHT);
				WarnDlg.SetHelp(L"CopyAskOverwrite");
				WarnDlg.SetId(CopyOverwriteId);
				WarnDlg.Process();

				switch (WarnDlg.GetExitCode())
				{
					case WDLG_OVERWRITE:
						MsgCode=WarnCopyDlg[WDLG_CHECKBOX].Selected?1:0;
						break;
					case WDLG_SKIP:
						MsgCode=WarnCopyDlg[WDLG_CHECKBOX].Selected?3:2;
						break;
					case WDLG_RENAME:
						MsgCode=WarnCopyDlg[WDLG_CHECKBOX].Selected?5:4;
						break;
					case WDLG_APPEND:
						MsgCode=WarnCopyDlg[WDLG_CHECKBOX].Selected?7:6;
						break;
					case -1:
					case -2:
					case WDLG_CANCEL:
						MsgCode=8;
						break;
				}
			}
		}
	}

	switch (MsgCode)
	{
		case 1:
			OvrMode=1;
		case 0:
			break;
		case 3:
			OvrMode=2;
		case 2:
			RetCode=COPY_NEXT;
			return FALSE;
		case 5:
			OvrMode=5;
			GenerateName(strDestName,strRenamedFilesPath);
		case 4:
			RetCode=COPY_RETRY;
			strNewName=strDestName;
			break;
		case 7:
			OvrMode=6;
		case 6:
			Append=TRUE;
			break;
		case -1:
		case -2:
		case 8:
			RetCode=COPY_CANCEL;
			return FALSE;
	}

	if (RetCode!=COPY_RETRY)
	{
		if ((DestAttr & FILE_ATTRIBUTE_READONLY) && !(Flags&FCOPY_OVERWRITENEXT))
		{
			int MsgCode=0;

			if (!SameName)
			{
				if (ReadOnlyOvrMode!=-1)
				{
					MsgCode=ReadOnlyOvrMode;
				}
				else
				{
					if (!DestDataFilled)
					{
						DestData.Clear();
						apiGetFindDataEx(DestName,DestData);
					}

					string strDateText,strTimeText;
					FormatString strSrcFileStr, strDestFileStr;
					unsigned __int64 SrcSize = SrcData.nFileSize;
					FormatString strSrcSizeText;
					strSrcSizeText<<SrcSize;
					unsigned __int64 DestSize = DestData.nFileSize;
					FormatString strDestSizeText;
					strDestSizeText<<DestSize;
					ConvertDate(SrcData.ftLastWriteTime,strDateText,strTimeText,8,FALSE,FALSE,TRUE,TRUE);
					strSrcFileStr<<fmt::LeftAlign()<<fmt::Width(17)<<MSG(MCopySource)<<L" "<<fmt::Width(25)<<fmt::Precision(25)<<strSrcSizeText<<L" "<<strDateText<<L" "<<strTimeText;
					ConvertDate(DestData.ftLastWriteTime,strDateText,strTimeText,8,FALSE,FALSE,TRUE,TRUE);
					strDestFileStr<<fmt::LeftAlign()<<fmt::Width(17)<<MSG(MCopyDest)<<L" "<<fmt::Width(25)<<fmt::Precision(25)<<strDestSizeText<<L" "<<strDateText<<L" "<<strTimeText;
					WarnCopyDlgData[WDLG_SRCFILEBTN].Data=strSrcFileStr;
					WarnCopyDlgData[WDLG_DSTFILEBTN].Data=strDestFileStr;
					WarnCopyDlgData[WDLG_TEXT].Data=MSG(MCopyFileRO);
					WarnCopyDlgData[WDLG_OVERWRITE].Data=MSG(Append?MCopyAppend:MCopyOverwrite);
					WarnCopyDlgData[WDLG_RENAME].Type=DI_TEXT;
					WarnCopyDlgData[WDLG_RENAME].Data=L"";
					WarnCopyDlgData[WDLG_APPEND].Type=DI_TEXT;
					WarnCopyDlgData[WDLG_APPEND].Data=L"";
					MakeDialogItemsEx(WarnCopyDlgData,WarnCopyDlg);
					string strSrcName;
					ConvertNameToFull(SrcData.strFileName,strSrcName);
					LPCWSTR WFN[2]={strSrcName,DestName};
					Dialog WarnDlg(WarnCopyDlg,ARRAYSIZE(WarnCopyDlg),WarnDlgProc,&WFN);
					WarnDlg.SetDialogMode(DMODE_WARNINGSTYLE);
					WarnDlg.SetPosition(-1,-1,WARN_DLG_WIDTH,WARN_DLG_HEIGHT);
					WarnDlg.SetHelp(L"CopyFiles");
					WarnDlg.SetId(CopyReadOnlyId);
					WarnDlg.Process();

					switch (WarnDlg.GetExitCode())
					{
						case WDLG_OVERWRITE:
							MsgCode=WarnCopyDlg[WDLG_CHECKBOX].Selected?1:0;
							break;
						case WDLG_SKIP:
							MsgCode=WarnCopyDlg[WDLG_CHECKBOX].Selected?3:2;
							break;
						case -1:
						case -2:
						case WDLG_CANCEL:
							MsgCode=8;
							break;
					}
				}
			}

			switch (MsgCode)
			{
				case 1:
					ReadOnlyOvrMode=1;
				case 0:
					break;
				case 3:
					ReadOnlyOvrMode=2;
				case 2:
					RetCode=COPY_NEXT;
					return FALSE;
				case -1:
				case -2:
				case 8:
					RetCode=COPY_CANCEL;
					return FALSE;
			}
		}

		if (!SameName && (DestAttr & (FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM)))
			apiSetFileAttributes(DestName,FILE_ATTRIBUTE_NORMAL);
	}

	return TRUE;
}



int ShellCopy::GetSecurity(const wchar_t *FileName,SECURITY_ATTRIBUTES &sa)
{
	SECURITY_INFORMATION si=DACL_SECURITY_INFORMATION;
	SECURITY_DESCRIPTOR *sd=(SECURITY_DESCRIPTOR *)sddata;
	DWORD Needed;
	BOOL RetSec=GetFileSecurity(NTPath(FileName),si,sd,SDDATA_SIZE,&Needed);
	int LastError=GetLastError();

	if (!RetSec || Needed>SDDATA_SIZE)
	{
		sd=nullptr;

		if (LastError!=ERROR_SUCCESS && LastError!=ERROR_FILE_NOT_FOUND &&
		        Message(MSG_WARNING|MSG_ERRORTYPE,2,MSG(MError),
		                MSG(MCannotGetSecurity),FileName,MSG(MOk),MSG(MCancel))==1)
			return FALSE;
	}

	sa.nLength=sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor=sd;
	sa.bInheritHandle=FALSE;
	return TRUE;
}



int ShellCopy::SetSecurity(const wchar_t *FileName,const SECURITY_ATTRIBUTES &sa)
{
	SECURITY_INFORMATION si=DACL_SECURITY_INFORMATION;
	BOOL RetSec=SetFileSecurity(NTPath(FileName),si,(PSECURITY_DESCRIPTOR)sa.lpSecurityDescriptor);
	int LastError=GetLastError();

	if (!RetSec)
	{
		if (LastError!=ERROR_SUCCESS && LastError!=ERROR_FILE_NOT_FOUND &&
		        Message(MSG_WARNING|MSG_ERRORTYPE,2,MSG(MError),
		                MSG(MCannotSetSecurity),FileName,MSG(MOk),MSG(MCancel))==1)
			return FALSE;
	}

	return TRUE;
}

BOOL ShellCopySecuryMsg(const wchar_t *Name)
{
	static clock_t PrepareSecuryStartTime;

	if (!Name || !*Name || (static_cast<DWORD>(clock() - PrepareSecuryStartTime) > Opt.ShowTimeoutDACLFiles))
	{
		static int Width=30;
		int WidthTemp;
		if (Name && *Name)
		{
			PrepareSecuryStartTime = clock();     // ������ ���� �������� ������
			WidthTemp=Max(StrLength(Name),30);
		}
		else
			Width=WidthTemp=30;

		// ������ ������ - 38%
		WidthTemp=Min(WidthTemp,WidthNameForMessage);
		Width=Max(Width,WidthTemp);

		string strOutFileName = Name; //??? nullptr ???
		TruncPathStr(strOutFileName,Width);
		CenterStr(strOutFileName, strOutFileName,Width+4);
		Message(0,0,MSG(MMoveDlgTitle),MSG(MCopyPrepareSecury),strOutFileName);

		if (CP->Cancelled())
		{
			return FALSE;
		}
	}

	PreRedrawItem preRedrawItem=PreRedraw.Peek();
	preRedrawItem.Param.Param1=Name;
	PreRedraw.SetParam(preRedrawItem.Param);
	return TRUE;
}


int ShellCopy::SetRecursiveSecurity(const wchar_t *FileName,const SECURITY_ATTRIBUTES &sa)
{
	if (SetSecurity(FileName,sa))
	{
		if (apiGetFileAttributes(FileName) & FILE_ATTRIBUTE_DIRECTORY)
		{
			ScanTree ScTree(TRUE,TRUE,Flags&FCOPY_COPYSYMLINKCONTENTS);
			ScTree.SetFindPath(FileName,L"*",FSCANTREE_FILESFIRST);

			string strFullName;
			FAR_FIND_DATA_EX SrcData;
			while (ScTree.GetNextName(&SrcData,strFullName))
			{
				if (!ShellCopySecuryMsg(strFullName))
					break;

				if (!SetSecurity(strFullName,sa))
				{
					return FALSE;
				}
			}
		}

		return TRUE;
	}

	return FALSE;
}



int ShellCopy::ShellSystemCopy(const wchar_t *SrcName,const wchar_t *DestName,const FAR_FIND_DATA_EX &SrcData)
{
	SECURITY_ATTRIBUTES sa;

	if ((Flags&FCOPY_COPYSECURITY) && !GetSecurity(SrcName,sa))
		return COPY_CANCEL;

	CP->SetNames(SrcName,DestName);
	CP->SetProgressValue(0,0);
	TotalCopiedSizeEx=TotalCopiedSize;
	if (!apiCopyFileEx(SrcName,DestName,CopyProgressRoutine,nullptr,nullptr,Flags&FCOPY_DECRYPTED_DESTINATION?COPY_FILE_ALLOW_DECRYPTED_DESTINATION:0))
	{
		Flags&=~FCOPY_DECRYPTED_DESTINATION;
		return (_localLastError=GetLastError())==ERROR_REQUEST_ABORTED ? COPY_CANCEL:COPY_FAILURE;
	}

	Flags&=~FCOPY_DECRYPTED_DESTINATION;

	if ((Flags&FCOPY_COPYSECURITY) && !SetSecurity(DestName,sa))
		return COPY_CANCEL;

	return COPY_SUCCESS;
}

DWORD WINAPI CopyProgressRoutine(LARGE_INTEGER TotalFileSize,
                                 LARGE_INTEGER TotalBytesTransferred,LARGE_INTEGER StreamSize,
                                 LARGE_INTEGER StreamBytesTransferred,DWORD dwStreamNumber,
                                 DWORD dwCallbackReason,HANDLE hSourceFile,HANDLE hDestinationFile,
                                 LPVOID lpData)
{
	// // _LOGCOPYR(CleverSysLog clv(L"CopyProgressRoutine"));
	// // _LOGCOPYR(SysLog(L"dwStreamNumber=%d",dwStreamNumber));
	bool Abort = false;
	if (CP->Cancelled())
	{
		// // _LOGCOPYR(SysLog(L"2='%s'/0x%08X  3='%s'/0x%08X  Flags=0x%08X",(char*)PreRedrawParam.Param2,PreRedrawParam.Param2,(char*)PreRedrawParam.Param3,PreRedrawParam.Param3,PreRedrawParam.Flags));
		Abort=true;
	}

	if (CheckAndUpdateConsole(OrigScrX!=ScrX || OrigScrY!=ScrY))
	{
		OrigScrX=ScrX;
		OrigScrY=ScrY;
		PR_ShellCopyMsg();
	}

	CurCopiedSize = TotalBytesTransferred.QuadPart;
	CP->SetProgressValue(TotalBytesTransferred.QuadPart,TotalFileSize.QuadPart);

	if (ShowTotalCopySize && dwStreamNumber==1)
	{
		TotalCopiedSize=TotalCopiedSizeEx+CurCopiedSize;
		CP->SetTotalProgressValue(TotalCopiedSize,TotalCopySize);
	}

	return Abort?PROGRESS_CANCEL:PROGRESS_CONTINUE;
}

bool ShellCopy::CalcTotalSize()
{
	string strSelName, strSelShortName;
	DWORD FileAttr;
	unsigned __int64 FileSize;
	// ��� �������
	FAR_FIND_DATA_EX fd;
	PreRedraw.Push(PR_ShellCopyMsg);
	PreRedrawItem preRedrawItem=PreRedraw.Peek();
	preRedrawItem.Param.Param1=CP;
	PreRedraw.SetParam(preRedrawItem.Param);
	TotalCopySize=CurCopiedSize=0;
	TotalFilesToProcess = 0;
	SrcPanel->GetSelName(nullptr,FileAttr);

	while (SrcPanel->GetSelName(&strSelName,FileAttr,&strSelShortName,&fd))
	{
		if ((FileAttr&FILE_ATTRIBUTE_REPARSE_POINT) && !(Flags&FCOPY_COPYSYMLINKCONTENTS))
			continue;

		if (FileAttr & FILE_ATTRIBUTE_DIRECTORY)
		{
			{
				unsigned long DirCount,FileCount,ClusterSize;
				unsigned __int64 CompressedSize,RealFileSize;
				CP->SetScanName(strSelName);
				int __Ret=GetDirInfo(L"",strSelName,DirCount,FileCount,FileSize,CompressedSize,
				                     RealFileSize,ClusterSize,-1,
				                     Filter,
				                     (Flags&FCOPY_COPYSYMLINKCONTENTS?GETDIRINFO_SCANSYMLINK:0)|
				                     (UseFilter?GETDIRINFO_USEFILTER:0));

				if (__Ret <= 0)
				{
					ShowTotalCopySize=false;
					PreRedraw.Pop();
					return FALSE;
				}

				if (FileCount > 0)
				{
					TotalCopySize+=FileSize;
					TotalFilesToProcess += FileCount;
				}
			}
		}
		else
		{
			//  ���������� ���������� ������
			if (UseFilter)
			{
				if (!Filter->FileInFilter(fd))
					continue;
			}

			FileSize = SrcPanel->GetLastSelectedSize();

			if (FileSize != (unsigned __int64)-1)
			{
				TotalCopySize+=FileSize;
				TotalFilesToProcess++;
			}
		}
	}

	// INFO: ��� ��� ��������, ����� "����� = ����� ������ * ���������� �����"
	TotalCopySize *= CountTarget;
	TotalFilesToProcess *= CountTarget;
	InsertCommas(TotalCopySize,strTotalCopySizeText);
	PreRedraw.Pop();
	return true;
}

/*
  �������� ������ SetFileAttributes() ���
  ����������� ����������� ���������
*/
bool ShellCopy::ShellSetAttr(const wchar_t *Dest,DWORD Attr)
{
	string strRoot;
	ConvertNameToFull(Dest,strRoot);
	GetPathRoot(strRoot,strRoot);

	if (apiGetFileAttributes(strRoot)==INVALID_FILE_ATTRIBUTES) // �������, ����� ������� ����, �� ��� � �������
	{
		// ... � ���� ������ �������� AS IS
		ConvertNameToFull(Dest,strRoot);
		GetPathRoot(strRoot,strRoot);

		if (apiGetFileAttributes(strRoot)==INVALID_FILE_ATTRIBUTES)
		{
			return false;
		}
	}

	DWORD FileSystemFlagsDst=0;
	int GetInfoSuccess=apiGetVolumeInformation(strRoot,nullptr,nullptr,nullptr,&FileSystemFlagsDst,nullptr);

	if (GetInfoSuccess)
	{
		if (!(FileSystemFlagsDst&FILE_FILE_COMPRESSION))
		{
			Attr&=~FILE_ATTRIBUTE_COMPRESSED;
		}

		if (!(FileSystemFlagsDst&FILE_SUPPORTS_ENCRYPTION))
		{
			Attr&=~FILE_ATTRIBUTE_ENCRYPTED;
		}
	}

	if (!apiSetFileAttributes(Dest,Attr))
	{
		return FALSE;
	}

	if ((Attr&FILE_ATTRIBUTE_COMPRESSED) && !(Attr&FILE_ATTRIBUTE_ENCRYPTED))
	{
		int Ret=ESetFileCompression(Dest,1,Attr&(~FILE_ATTRIBUTE_COMPRESSED),SkipMode);

		if (Ret==SETATTR_RET_ERROR)
		{
			return false;
		}
		else if (Ret==SETATTR_RET_SKIPALL)
		{
			this->SkipMode=SETATTR_RET_SKIP;
		}
	}

	// ��� �����������/�������� ���������� FILE_ATTRIBUTE_ENCRYPTED
	// ��� ��������, ���� �� ����
	if (GetInfoSuccess && FileSystemFlagsDst&FILE_SUPPORTS_ENCRYPTION && Attr&FILE_ATTRIBUTE_ENCRYPTED && Attr&FILE_ATTRIBUTE_DIRECTORY)
	{
		int Ret=ESetFileEncryption(Dest,1,0,SkipMode);

		if (Ret==SETATTR_RET_ERROR)
		{
			return false;
		}
		else if (Ret==SETATTR_RET_SKIPALL)
		{
			SkipMode=SETATTR_RET_SKIP;
		}
	}

	return true;
}

void ShellCopy::CheckUpdatePanel() // ���������� ���� FCOPY_UPDATEPPANEL
{
}
