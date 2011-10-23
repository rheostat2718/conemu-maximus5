/*
plugapi.cpp

API, ��������� �������� (�������, ����, ...)
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

#include "plugapi.hpp"
#include "keys.hpp"
#include "lang.hpp"
#include "help.hpp"
#include "vmenu.hpp"
#include "dialog.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "cmdline.hpp"
#include "scantree.hpp"
#include "rdrwdsk.hpp"
#include "fileview.hpp"
#include "fileedit.hpp"
#include "plugins.hpp"
#include "savescr.hpp"
#include "flink.hpp"
#include "manager.hpp"
#include "ctrlobj.hpp"
#include "frame.hpp"
#include "scrbuf.hpp"
#include "farexcpt.hpp"
#include "lockscrn.hpp"
#include "constitle.hpp"
#include "TPreRedrawFunc.hpp"
#include "syslog.hpp"
#include "interf.hpp"
#include "keyboard.hpp"
#include "palette.hpp"
#include "message.hpp"
#include "eject.hpp"
#include "filefilter.hpp"
#include "fileowner.hpp"
#include "stddlg.hpp"
#include "pathmix.hpp"
#include "exitcode.hpp"
#include "processname.hpp"
#include "RegExp.hpp"
#include "TaskBar.hpp"
#include "console.hpp"
#include "plugsettings.hpp"
#include "farversion.hpp"
#include "mix.hpp"
#include "FarGuid.hpp"

//#define SHOW_UNSAFE_MSG
#undef SHOW_UNSAFE_MSG
BOOL gbInOpenPlugin = FALSE;
BOOL gbInEditorEvent = FALSE;
#ifndef _DEBUG
BOOL gbReportInRelease = FALSE;
#endif
void ReportThreadUnsafeCall(const wchar_t* asFormat, DWORD anCommand)
{
	static bool bSkipBuzz = false;
	if (bSkipBuzz)
		return;
	//TODO: ��������� � MessageBoxW � ���� "��" - �� ������������ MiniDump
	wchar_t szUnsafe[2048], szReady[1024], szFar[128];
	wsprintf(szReady, asFormat, anCommand);
	wsprintf(szUnsafe, L"Thread UnSafe api call detected!\n%s\nCurrent TID=%u, Main TID=%u\n", szReady, GetCurrentThreadId(), gnMainThreadId);
	wsprintf(szFar, L"Far PID=%u", GetCurrentProcessId());
#ifdef _DEBUG
	int nBtn;
	#ifdef SHOW_UNSAFE_MSG
	nBtn = MessageBox(NULL, szUnsafe, szFar, MB_ICONSTOP|MB_SYSTEMMODAL|MB_CANCELTRYCONTINUE);
	#else
	OutputDebugString(szUnsafe);
	nBtn = IDCONTINUE;
	#endif
	if (nBtn == IDCANCEL)
	{
		bSkipBuzz = true;
		return;
	}
	if (nBtn == IDTRYAGAIN)
	{
		DebugBreak();
		nBtn = IDTRYAGAIN; // ����� ���� ���� breakpoint �������
	}
#else
	if (gbReportInRelease)
		OutputDebugString(szUnsafe);
#endif
}

wchar_t *WINAPI FarItoa(int value, wchar_t *string, int radix)
{
	if (string)
		return _itow(value,string,radix);

	return nullptr;
}

wchar_t *WINAPI FarItoa64(__int64 value, wchar_t *string, int radix)
{
	if (string)
		return _i64tow(value, string, radix);

	return nullptr;
}

int WINAPI FarAtoi(const wchar_t *s)
{
	if (s)
		return _wtoi(s);

	return 0;
}
__int64 WINAPI FarAtoi64(const wchar_t *s)
{
	return s?_wtoi64(s):0;
}

void WINAPI FarQsort(void *base, size_t nelem, size_t width,
                     int (__cdecl *fcmp)(const void *, const void *))
{
	if (base && fcmp)
		far_qsort(base,nelem,width,fcmp);
}

void WINAPI FarQsortEx(void *base, size_t nelem, size_t width,
                       int (__cdecl *fcmp)(const void *, const void *,void *user),void *user)
{
	if (base && fcmp)
		qsortex((char*)base,nelem,width,fcmp,user);
}

void *WINAPI FarBsearch(const void *key, const void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *))
{
	if (key && fcmp && base)
		return bsearch(key,base,nelem,width,fcmp);

	return nullptr;
}

void WINAPI DeleteBuffer(void *Buffer)
{
	if (Buffer) xf_free(Buffer);
}

void ScanPluginDir();

/* $ 07.12.2001 IS
   ������� ������ GetString ��� �������� - � ������� �����������������.
   ������� ��� ����, ����� �� ����������� ��� GetString.
*/
int WINAPI FarInputBox(
    INT_PTR PluginNumber,
    const GUID* Id,
    const wchar_t *Title,
    const wchar_t *Prompt,
    const wchar_t *HistoryName,
    const wchar_t *SrcText,
    wchar_t *DestText,
    int DestLength,
    const wchar_t *HelpTopic,
    unsigned __int64 Flags
)
{
	if (FrameManager->ManagerIsDown())
		return FALSE;

	string strDest;
	int nResult = GetString(Title,Prompt,HistoryName,SrcText,strDest,HelpTopic,Flags&~FIB_CHECKBOX,nullptr,nullptr,PluginNumber,Id);
	xwcsncpy(DestText, strDest, DestLength+1);
	return nResult;
}

/* ������� ������ ������ */
BOOL WINAPI FarShowHelp(
    const wchar_t *ModuleName,
    const wchar_t *HelpTopic,
    FARHELPFLAGS Flags
)
{
	if (FrameManager->ManagerIsDown())
		return FALSE;

	if (!HelpTopic)
		HelpTopic=L"Contents";

	UINT64 OFlags=Flags;
	Flags&=~(FHELP_NOSHOWERROR|FHELP_USECONTENTS);
	string strPath, strTopic;
	string strMask;

	// ��������� � ������ ������ ���� �� ������������ � � ��� ������,
	// ���� ����� FHELP_FARHELP...
	if ((Flags&FHELP_FARHELP) || *HelpTopic==L':')
		strTopic = HelpTopic+((*HelpTopic == L':')?1:0);
	else
	{
		if (ModuleName)
		{
			// FHELP_SELFHELP=0 - ���������� ������ ���-� ��� Info.ModuleName
			//                   � �������� ����� �� ����� ���������� �������
			/* $ 17.11.2000 SVS
			   � �������� FHELP_SELFHELP ����� ����? ��������� - 0
			   � ����� ����� ��������� ����, ��� ������� �� �������� :-(
			*/
			if (Flags == FHELP_SELFHELP || (Flags&(FHELP_CUSTOMFILE|FHELP_CUSTOMPATH)))
			{
				strPath = ModuleName;

				if (Flags == FHELP_SELFHELP || (Flags&(FHELP_CUSTOMFILE)))
				{
					if (Flags&FHELP_CUSTOMFILE)
						strMask=PointToName(strPath);
					else
						strMask.Clear();

					CutToSlash(strPath);
				}
			}
			else
				return FALSE;

			strTopic.Format(HelpFormatLink,strPath.CPtr(),HelpTopic);
		}
		else
			return FALSE;
	}

	{
		Help Hlp(strTopic,strMask,OFlags);

		if (Hlp.GetError())
			return FALSE;
	}

	return TRUE;
}

/* $ 05.07.2000 IS
  �������, ������� ����� ����������� � � ���������, � � �������, �...
*/
INT_PTR WINAPI FarAdvControl(INT_PTR ModuleNumber, ADVANCED_CONTROL_COMMANDS Command, int Param1, void* Param2)
{
	// ��� ������ ���������������� ��������
	if (gnMainThreadId != GetCurrentThreadId())
	{
		// ��������� ������� ��������� ������� ���� ��� ���������� ������� API
		// ������ ��� ��������� - ������, ������� ������ ���������, � �� � OpenPlugin �� ��?
		BOOL lbSafe = gbInOpenPlugin;

		if (Command == ACTL_GETFARMANAGERVERSION
			|| Command == ACTL_GETPLUGINMAXREADDATA
			|| Command == ACTL_GETCOLOR
			|| Command == ACTL_GETARRAYCOLOR
			|| Command == ACTL_GETFARHWND
			|| Command == ACTL_GETDIALOGSETTINGS
			|| Command == ACTL_GETSYSTEMSETTINGS
			|| Command == ACTL_GETPANELSETTINGS
			|| Command == ACTL_GETINTERFACESETTINGS
			|| Command == ACTL_GETCONFIRMATIONS
			|| Command == ACTL_GETDESCSETTINGS
			|| Command == ACTL_GETFARRECT
			|| Command == ACTL_SETPROGRESSSTATE || Command == ACTL_SETPROGRESSVALUE
			)
		{
			lbSafe = TRUE;
		}
		//// ���������� - ������ ACTL_SYNCHRO thread-safe � Far2,
		//// �� ���� ������� MCMD_GETAREA, ������� ����� ����� �� ������ ������
		//else if (Command == ACTL_KEYMACRO)
		//{
		//	ActlKeyMacro *KeyMacro=(ActlKeyMacro*)Param;
		//	lbSafe = (KeyMacro->Command == MCMD_GETAREA);
		//}
		if (!lbSafe)
		{
			ReportThreadUnsafeCall(L"AdvControl(%u)", Command);
		}
	}

	struct Opt2Flags
	{
		int *Opt;
		DWORD Flags;
	};

	switch (Command)
	{
		case ACTL_GETFARMANAGERVERSION:
		case ACTL_GETSYSWORDDIV:
		case ACTL_GETCOLOR:
		case ACTL_GETARRAYCOLOR:
		case ACTL_GETFARHWND:
		case ACTL_GETSYSTEMSETTINGS:
		case ACTL_GETPANELSETTINGS:
		case ACTL_GETINTERFACESETTINGS:
		case ACTL_GETCONFIRMATIONS:
		case ACTL_GETDESCSETTINGS:
		case ACTL_GETPLUGINMAXREADDATA:
		case ACTL_GETMEDIATYPE:
		case ACTL_SETPROGRESSSTATE:
		case ACTL_SETPROGRESSVALUE:
		case ACTL_GETFARRECT:
		case ACTL_GETCURSORPOS:
		case ACTL_SETCURSORPOS:
		case ACTL_PROGRESSNOTIFY:
			break;
		default:

			if (FrameManager && FrameManager->ManagerIsDown())
				return 0;
	}

	switch (Command)
	{
		case ACTL_GETFARMANAGERVERSION:
		{
			if (Param2)
				*(VersionInfo*)Param2=FAR_VERSION;

			return TRUE;
		}
		case ACTL_GETPLUGINMAXREADDATA:
		{
			return Opt.PluginMaxReadData;
		}
		case ACTL_GETSYSWORDDIV:
		{
			if (Param1 && Param2)
				xwcsncpy((wchar_t *)Param2, Opt.strWordDiv, Param1);

			return Opt.strWordDiv.GetLength()+1;
		}
		/* $ 24.08.2000 SVS
		   ������� ������������ (��� �����) �������
		   (const INPUT_RECORD*)Param2 - ��� �������, ������� �������, ��� NULL
		   ���� ��� ����� ����� ������� �����.
		   ���������� 0;
		*/
		case ACTL_WAITKEY:
		{
			return WaitKey(Param2?InputRecordToKey((const INPUT_RECORD*)Param2):-1,0,false);
		}
		/* $ 04.12.2000 SVS
		  ACTL_GETCOLOR - �������� ������������ ���� �� �������, �������������
		   � farcolor.hpp
		  Param2 - [OUT] �������� �����
		  Return - TRUE ���� OK ��� FALSE ���� ������ �������.
		*/
		case ACTL_GETCOLOR:
		{
			if (static_cast<UINT>(Param1) < Opt.Palette.SizeArrayPalette)
			{
				*static_cast<FarColor*>(Param2) = Opt.Palette.CurrentPalette[static_cast<size_t>(Param1)];
				return TRUE;
			}
			return FALSE;
		}
		/* $ 04.12.2000 SVS
		  ACTL_GETARRAYCOLOR - �������� ���� ������ ������
		  Param1 - ������ ������ (� ��������� FarColor)
		  Param2 - ��������� �� ����� ��� nullptr, ����� �������� ����������� ������
		  Return - ������ �������.
		*/
		case ACTL_GETARRAYCOLOR:
		{
			if (Param2 && static_cast<size_t>(Param1) >= Opt.Palette.SizeArrayPalette)
			{
				memcpy(Param2, Opt.Palette.CurrentPalette, Opt.Palette.SizeArrayPalette*sizeof(FarColor));
			}
			return Opt.Palette.SizeArrayPalette;
		}
		/*
		  Param=FARColor{
		    DWORD Flags;
		    int StartIndex;
		    int ColorItem;
		    LPBYTE Colors;
		  };
		*/
		case ACTL_SETARRAYCOLOR:
		{
			if (Param2)
			{
				FarSetColors *Pal=(FarSetColors*)Param2;

				if (Pal->Colors && Pal->StartIndex+Pal->ColorsCount <= Opt.Palette.SizeArrayPalette)
				{
					memmove(Opt.Palette.CurrentPalette+Pal->StartIndex,Pal->Colors,Pal->ColorsCount*sizeof(FarColor));

					if (Pal->Flags&FSETCLR_REDRAW)
					{
						ScrBuf.Lock(); // �������� ������ ����������
						FrameManager->ResizeAllFrame();
						FrameManager->PluginCommit(); // ��������.
						ScrBuf.Unlock(); // ��������� ����������
					}

					return TRUE;
				}
			}

			return FALSE;
		}
		/* $ 14.12.2000 SVS
		  ACTL_EJECTMEDIA - ������� ���� �� �������� ����������
		  Param - ��������� �� ��������� ActlEjectMedia
		  Return - TRUE - �������� ����������, FALSE - ������.
		*/
		case ACTL_EJECTMEDIA:
		{
			return Param2?EjectVolume((wchar_t)((ActlEjectMedia*)Param2)->Letter,
			                         ((ActlEjectMedia*)Param2)->Flags):FALSE;
			/*
			      if(Param)
			      {
							ActlEjectMedia *aem=(ActlEjectMedia *)Param;
			        char DiskLetter[4]=" :\\";
			        DiskLetter[0]=(char)aem->Letter;
			        int DriveType = FAR_GetDriveType(DiskLetter,nullptr,FALSE); // ����� �� ���������� ��� CD

			        if(DriveType == DRIVE_USBDRIVE && RemoveUSBDrive((char)aem->Letter,aem->Flags))
			          return TRUE;
			        if(DriveType == DRIVE_SUBSTITUTE && DelSubstDrive(DiskLetter))
			          return TRUE;
			        if(IsDriveTypeCDROM(DriveType) && EjectVolume((char)aem->Letter,aem->Flags))
			          return TRUE;

			      }
			      return FALSE;
			*/
		}
		/*
		    case ACTL_GETMEDIATYPE:
		    {
					ActlMediaType *amt=(ActlMediaType *)Param;
		      char DiskLetter[4]=" :\\";
		      DiskLetter[0]=(amt)?(char)amt->Letter:0;
		      return FAR_GetDriveType(DiskLetter,nullptr,(amt && !(amt->Flags&MEDIATYPE_NODETECTCDROM)?TRUE:FALSE));
		    }
		*/
		/* $ 05.06.2001 tran
		   ����� ACTL_ ��� ������ � �������� */
		case ACTL_GETWINDOWINFO:
		{
			if (FrameManager && Param2)
			{
				string strType, strName;
				WindowInfo *wi=(WindowInfo*)Param2;
				Frame *f;

				/* $ 22.12.2001 VVM
				  + ���� Pos == -1 �� ����� ������� ����� */
				if (wi->Pos == -1)
					f=FrameManager->GetCurrentFrame();
				else
					f=(*FrameManager)[wi->Pos];

				if (!f)
					return FALSE;

				f->GetTypeAndName(strType, strName);

				if (wi->TypeNameSize && wi->TypeName)
				{
					xwcsncpy(wi->TypeName,strType,wi->TypeNameSize);
				}
				else
				{
					wi->TypeNameSize=static_cast<int>(strType.GetLength()+1);
				}

				if (wi->NameSize && wi->Name)
				{
					xwcsncpy(wi->Name,strName,wi->NameSize);
				}
				else
				{
					wi->NameSize=static_cast<int>(strName.GetLength()+1);
				}

				wi->Pos=FrameManager->IndexOf(f);
				wi->Type=static_cast<WINDOWINFO_TYPE>(f->GetType());
				wi->Flags=0;
				if (f->IsFileModified()) wi->Flags|=WIF_MODIFIED;
				if (f==FrameManager->GetCurrentFrame()) wi->Flags|=WIF_CURRENT;
				switch (wi->Type)
				{
					case WTYPE_VIEWER:
						wi->Id=static_cast<FileViewer*>(f)->GetId();
						break;
					case WTYPE_EDITOR:
						wi->Id=static_cast<FileEditor*>(f)->GetId();
						break;
					case WTYPE_DIALOG:
						wi->Id=(INT_PTR)f;
						break;
					default:
						wi->Id=0;
						break;
				}
				return TRUE;
			}

			return FALSE;
		}
		case ACTL_GETWINDOWCOUNT:
		{
			return FrameManager?FrameManager->GetFrameCount():0;
		}
		case ACTL_SETCURRENTWINDOW:
		{
			// �������� ������������ �������, ���� ��������� � ��������� ���������/������.
			if (FrameManager && !FrameManager->InModalEV() && (*FrameManager)[Param1])
			{
				int TypeFrame=FrameManager->GetCurrentFrame()->GetType();

				// �������� ������������ �������, ���� ��������� � ����� ��� ������� (���� ���������)
				if (TypeFrame != MODALTYPE_HELP && TypeFrame != MODALTYPE_DIALOG)
				{
					Frame* PrevFrame = FrameManager->GetCurrentFrame();
					FrameManager->ActivateFrame(Param1);
					FrameManager->DeactivateFrame(PrevFrame, 0);
					return TRUE;
				}
			}

			return FALSE;
		}
		/*$ 26.06.2001 SKV
		  ��� ����������� ������ � ACTL_SETCURRENTWINDOW
		  (� ����� ��� ��� ���� � �������)
		*/
		case ACTL_COMMIT:
		{
			return FrameManager?FrameManager->PluginCommit():FALSE;
		}
		/* $ 15.09.2001 tran
		   ���������� �������� */
		case ACTL_GETFARHWND:
		{
			return (INT_PTR)Console.GetWindow();
		}
		case ACTL_GETDIALOGSETTINGS:
		{
			DWORD Options=0;
			static Opt2Flags ODlg[]=
			{
				{&Opt.Dialogs.EditHistory,FDIS_HISTORYINDIALOGEDITCONTROLS},
				{&Opt.Dialogs.EditBlock,FDIS_PERSISTENTBLOCKSINEDITCONTROLS},
				{&Opt.Dialogs.AutoComplete,FDIS_AUTOCOMPLETEININPUTLINES},
				{&Opt.Dialogs.EULBsClear,FDIS_BSDELETEUNCHANGEDTEXT},
				{&Opt.Dialogs.DelRemovesBlocks,FDIS_DELREMOVESBLOCKS},
				{&Opt.Dialogs.MouseButton,FDIS_MOUSECLICKOUTSIDECLOSESDIALOG},
			};

			for (size_t I=0; I < ARRAYSIZE(ODlg); ++I)
				if (*ODlg[I].Opt)
					Options|=ODlg[I].Flags;

			return Options;
		}
		/* $ 24.11.2001 IS
		   ��������� � ����������� ����������, ������, ����������, �������������
		*/
		case ACTL_GETSYSTEMSETTINGS:
		{
			DWORD Options=0;
			static Opt2Flags OSys[]=
			{
				{&Opt.ClearReadOnly,FSS_CLEARROATTRIBUTE},
				{&Opt.DeleteToRecycleBin,FSS_DELETETORECYCLEBIN},
				{&Opt.CMOpt.UseSystemCopy,FSS_USESYSTEMCOPYROUTINE},
				{&Opt.CMOpt.CopyOpened,FSS_COPYFILESOPENEDFORWRITING},
				{&Opt.ScanJunction,FSS_SCANSYMLINK},
				{&Opt.CreateUppercaseFolders,FSS_CREATEFOLDERSINUPPERCASE},
				{&Opt.SaveHistory,FSS_SAVECOMMANDSHISTORY},
				{&Opt.SaveFoldersHistory,FSS_SAVEFOLDERSHISTORY},
				{&Opt.SaveViewHistory,FSS_SAVEVIEWANDEDITHISTORY},
				{&Opt.UseRegisteredTypes,FSS_USEWINDOWSREGISTEREDTYPES},
				{&Opt.AutoSaveSetup,FSS_AUTOSAVESETUP},
			};

			for (size_t I=0; I < ARRAYSIZE(OSys); ++I)
				if (*OSys[I].Opt)
					Options|=OSys[I].Flags;

			return Options;
		}
		case ACTL_GETPANELSETTINGS:
		{
			DWORD Options=0;
			static Opt2Flags OSys[]=
			{
				{&Opt.ShowHidden,FPS_SHOWHIDDENANDSYSTEMFILES},
				{&Opt.Highlight,FPS_HIGHLIGHTFILES},
				{&Opt.Tree.AutoChangeFolder,FPS_AUTOCHANGEFOLDER},
				{&Opt.SelectFolders,FPS_SELECTFOLDERS},
				{&Opt.ReverseSort,FPS_ALLOWREVERSESORTMODES},
				{&Opt.ShowColumnTitles,FPS_SHOWCOLUMNTITLES},
				{&Opt.ShowPanelStatus,FPS_SHOWSTATUSLINE},
				{&Opt.ShowPanelTotals,FPS_SHOWFILESTOTALINFORMATION},
				{&Opt.ShowPanelFree,FPS_SHOWFREESIZE},
				{&Opt.ShowPanelScrollbar,FPS_SHOWSCROLLBAR},
				{&Opt.ShowScreensNumber,FPS_SHOWBACKGROUNDSCREENSNUMBER},
				{&Opt.ShowSortMode,FPS_SHOWSORTMODELETTER},
			};

			for (size_t I=0; I < ARRAYSIZE(OSys); ++I)
				if (*OSys[I].Opt)
					Options|=OSys[I].Flags;

			return Options;
		}
		case ACTL_GETINTERFACESETTINGS:
		{
			DWORD Options=0;
			static Opt2Flags OSys[]=
			{
				{&Opt.Clock,FIS_CLOCKINPANELS},
				{&Opt.ViewerEditorClock,FIS_CLOCKINVIEWERANDEDITOR},
				{&Opt.Mouse,FIS_MOUSE},
				{&Opt.ShowKeyBar,FIS_SHOWKEYBAR},
				{&Opt.ShowMenuBar,FIS_ALWAYSSHOWMENUBAR},
				{&Opt.CMOpt.CopyShowTotal,FIS_SHOWTOTALCOPYPROGRESSINDICATOR},
				{&Opt.CMOpt.CopyTimeRule,FIS_SHOWCOPYINGTIMEINFO},
				{&Opt.PgUpChangeDisk,FIS_USECTRLPGUPTOCHANGEDRIVE},
				{&Opt.DelOpt.DelShowTotal,FIS_SHOWTOTALDELPROGRESSINDICATOR},
			};

			for (size_t I=0; I < ARRAYSIZE(OSys); ++I)
				if (*OSys[I].Opt)
					Options|=OSys[I].Flags;

			return Options;
		}
		case ACTL_GETCONFIRMATIONS:
		{
			DWORD Options=0;
			static Opt2Flags OSys[]=
			{
				{&Opt.Confirm.Copy,FCS_COPYOVERWRITE},
				{&Opt.Confirm.Move,FCS_MOVEOVERWRITE},
				{&Opt.Confirm.RO,FCS_OVERWRITEDELETEROFILES},
				{&Opt.Confirm.Drag,FCS_DRAGANDDROP},
				{&Opt.Confirm.Delete,FCS_DELETE},
				{&Opt.Confirm.DeleteFolder,FCS_DELETENONEMPTYFOLDERS},
				{&Opt.Confirm.Esc,FCS_INTERRUPTOPERATION},
				{&Opt.Confirm.RemoveConnection,FCS_DISCONNECTNETWORKDRIVE},
				{&Opt.Confirm.AllowReedit,FCS_RELOADEDITEDFILE},
				{&Opt.Confirm.HistoryClear,FCS_CLEARHISTORYLIST},
				{&Opt.Confirm.Exit,FCS_EXIT},
			};

			for (size_t I=0; I < ARRAYSIZE(OSys); ++I)
				if (*OSys[I].Opt)
					Options|=OSys[I].Flags;

			return Options;
		}
		case ACTL_GETDESCSETTINGS:
		{
			// ����� ���� - � �������� �� ��������������
			DWORD Options=0;

			if (Opt.Diz.UpdateMode == DIZ_UPDATE_IF_DISPLAYED)
				Options |= FDS_UPDATEIFDISPLAYED;
			else if (Opt.Diz.UpdateMode == DIZ_UPDATE_ALWAYS)
				Options |= FDS_UPDATEALWAYS;

			if (Opt.Diz.SetHidden)
				Options |= FDS_SETHIDDEN;

			if (Opt.Diz.ROUpdate)
				Options |= FDS_UPDATEREADONLY;

			return Options;
		}
		case ACTL_REDRAWALL:
		{
			int Ret=FrameManager->ProcessKey(KEY_CONSOLE_BUFFER_RESIZE);
			FrameManager->PluginCommit();
			return Ret;
		}

		case ACTL_SETPROGRESSSTATE:
		{
			TBC.SetProgressState(static_cast<TBPFLAG>(Param1));
			return TRUE;
		}

		case ACTL_SETPROGRESSVALUE:
		{
			BOOL Result=FALSE;
			if(Param2)
			{
				ProgressValue* PV=static_cast<ProgressValue*>(Param2);
				TBC.SetProgressValue(PV->Completed,PV->Total);
				Result=TRUE;
			}
			return Result;
		}

		case ACTL_QUIT:
		{
			CloseFARMenu=TRUE;
			FrameManager->ExitMainLoop(FALSE);
			return TRUE;
		}

		case ACTL_GETFARRECT:
			{
				BOOL Result=FALSE;
				if(Param2)
				{
					SMALL_RECT& Rect=*static_cast<PSMALL_RECT>(Param2);
					if(Opt.WindowMode)
					{
						Result=Console.GetWorkingRect(Rect);
					}
					else
					{
						COORD Size;
						if(Console.GetSize(Size))
						{
							Rect.Left=0;
							Rect.Top=0;
							Rect.Right=Size.X-1;
							Rect.Bottom=Size.Y-1;
							Result=TRUE;
						}
					}
				}
				return Result;
			}
			break;

		case ACTL_GETCURSORPOS:
			{
				BOOL Result=FALSE;
				if(Param2)
				{
					COORD& Pos=*static_cast<PCOORD>(Param2);
					Result=Console.GetCursorPosition(Pos);
				}
				return Result;
			}
			break;

		case ACTL_SETCURSORPOS:
			{
				BOOL Result=FALSE;
				if(Param2)
				{
					COORD& Pos=*static_cast<PCOORD>(Param2);
					Result=Console.SetCursorPosition(Pos);
				}
				return Result;
			}
			break;

		case ACTL_PROGRESSNOTIFY:
		{
			TBC.Flash();
			return TRUE;
		}
		default:
			break;

	}

	return FALSE;
}

static DWORD NormalizeControlKeys(DWORD Value)
{
	DWORD result=Value&(LEFT_CTRL_PRESSED|LEFT_ALT_PRESSED|SHIFT_PRESSED);
	if(Value&RIGHT_CTRL_PRESSED) result|=LEFT_CTRL_PRESSED;
	if(Value&RIGHT_ALT_PRESSED) result|=LEFT_ALT_PRESSED;
	return result;
}

int WINAPI FarMenuFn(
    INT_PTR PluginNumber,
    const GUID* Id,
    int X,
    int Y,
    int MaxHeight,
    unsigned __int64 Flags,
    const wchar_t *Title,
    const wchar_t *Bottom,
    const wchar_t *HelpTopic,
    const FarKey *BreakKeys,
    int *BreakCode,
    const FarMenuItem *Item,
    size_t ItemsNumber
)
{
	if (FrameManager->ManagerIsDown())
		return -1;

	if (DisablePluginsOutput)
		return -1;

	int ExitCode;
	{
		VMenu FarMenu(Title,nullptr,0,MaxHeight);
		CtrlObject->Macro.SetMode(MACRO_MENU);
		FarMenu.SetPosition(X,Y,0,0);
		if(Id)
		{
			FarMenu.SetId(*Id);
		}

		if (BreakCode)
			*BreakCode=-1;

		{
			string strTopic;

			if (Help::MkTopic(PluginNumber,HelpTopic,strTopic))
				FarMenu.SetHelp(strTopic);
		}

		if (Bottom)
			FarMenu.SetBottomTitle(Bottom);

		// ����� ����� ����
		DWORD MenuFlags=0;

		if (Flags & FMENU_SHOWAMPERSAND)
			MenuFlags|=VMENU_SHOWAMPERSAND;

		if (Flags & FMENU_WRAPMODE)
			MenuFlags|=VMENU_WRAPMODE;

		if (Flags & FMENU_CHANGECONSOLETITLE)
			MenuFlags|=VMENU_CHANGECONSOLETITLE;

		FarMenu.SetFlags(MenuFlags);
		MenuItemEx CurItem;
		CurItem.Clear();
		int Selected=0;

		for (size_t i=0; i < ItemsNumber; i++)
		{
			CurItem.Flags=Item[i].Flags;
			CurItem.strName.Clear();
			// ��������� MultiSelected, �.�. � ��� ������ ������ � ����� �� ������������, ��������� ������ ������
			DWORD SelCurItem=CurItem.Flags&LIF_SELECTED;
			CurItem.Flags&=~LIF_SELECTED;

			if (!Selected && !(CurItem.Flags&LIF_SEPARATOR) && SelCurItem)
			{
				CurItem.Flags|=SelCurItem;
				Selected++;
			}

			CurItem.strName=Item[i].Text;
			if(CurItem.Flags&LIF_SEPARATOR)
			{
				CurItem.AccelKey=0;
			}
			else
			{
				INPUT_RECORD input={0};
				FarKeyToInputRecord(Item[i].AccelKey,&input);
				CurItem.AccelKey=InputRecordToKey(&input);
			}
			FarMenu.AddItem(&CurItem);
		}

		if (!Selected)
			FarMenu.SetSelectPos(0,1);

		// ����� ����, � ������� ���������
		if (Flags & FMENU_AUTOHIGHLIGHT)
			FarMenu.AssignHighlights(FALSE);

		if (Flags & FMENU_REVERSEAUTOHIGHLIGHT)
			FarMenu.AssignHighlights(TRUE);

		FarMenu.SetTitle(Title);
		FarMenu.Show();

		while (!FarMenu.Done() && !CloseFARMenu)
		{
			INPUT_RECORD ReadRec;
			int ReadKey=GetInputRecord(&ReadRec);

			if (ReadKey==KEY_CONSOLE_BUFFER_RESIZE)
			{
				LockScreen LckScr;
				FarMenu.Hide();
				FarMenu.Show();
			}
			else if (ReadRec.EventType==MOUSE_EVENT)
			{
				FarMenu.ProcessMouse(&ReadRec.Event.MouseEvent);
			}
			else if (ReadKey!=KEY_NONE)
			{
				if (BreakKeys)
				{
					for (int I=0; BreakKeys[I].VirtualKeyCode; I++)
					{
						if (CtrlObject->Macro.IsExecuting())
						{
							int VirtKey,ControlState;
							TranslateKeyToVK(ReadKey,VirtKey,ControlState,&ReadRec);
						}

						if (ReadRec.Event.KeyEvent.wVirtualKeyCode==BreakKeys[I].VirtualKeyCode)
						{
							DWORD Flags=NormalizeControlKeys(BreakKeys[I].ControlKeyState);
							DWORD RealFlags=NormalizeControlKeys(ReadRec.Event.KeyEvent.dwControlKeyState);

							if (RealFlags == Flags)
							{
								if (BreakCode)
									*BreakCode=I;

								FarMenu.Hide();
//								CheckScreenLock();
								return FarMenu.GetSelectPos();
							}
						}
					}
				}

				FarMenu.ProcessKey(ReadKey);
			}
		}

		ExitCode=FarMenu.Modal::GetExitCode();
	}
//  CheckScreenLock();
	return(ExitCode);
}

// ������� FarDefDlgProc ��������� ������� �� ���������
INT_PTR WINAPI FarDefDlgProc(HANDLE hDlg,int Msg,int Param1,void* Param2)
{
	if (hDlg) // ��������� ������ ����� ��� hDlg=0
		return DefDlgProc(hDlg,Msg,Param1,Param2);

	return 0;
}

// ������� ��������� �������
INT_PTR WINAPI FarSendDlgMessage(HANDLE hDlg,int Msg,int Param1,void* Param2)
{
	if (hDlg) // ��������� ������ ����� ��� hDlg=0
		return SendDlgMessage(hDlg,Msg,Param1,Param2);

	return 0;
}

#if !defined(__GNUC__)
/* ���� ������ ������� - ��������� ���� Flags - ������� ����, ���
   �� ����� ��� �� � �������
*/
static int Except_FarDialogEx()
{
	if (CtrlObject)
		CtrlObject->Plugins.Flags.Set(PSIF_DIALOG);

	Frame *frame;

	if ((frame=FrameManager->GetBottomFrame()) )
	{
		//while(!frame->Refreshable()) // � ����� ��� ���� �����???
		frame->Unlock(); // ������ ����� :-)
	}

//  CheckScreenLock();
	FrameManager->RefreshFrame(); //??
	return EXCEPTION_CONTINUE_SEARCH; // ��������� ���������� ������� ����������!
}
#endif

static int FarDialogExSehed(Dialog *FarDialog)
{
	__try
	{
		FarDialog->Process();
		return FarDialog->GetExitCode();
	}
	__except(Except_FarDialogEx())
	{
		return -1;
	}
}

HANDLE WINAPI FarDialogInit(INT_PTR PluginNumber, const GUID* Id, int X1, int Y1, int X2, int Y2,
                            const wchar_t *HelpTopic, const FarDialogItem *Item,
                            size_t ItemsNumber, DWORD Reserved, unsigned __int64 Flags,
                            FARWINDOWPROC DlgProc, void* Param)
{
	HANDLE hDlg=INVALID_HANDLE_VALUE;

	if (FrameManager->ManagerIsDown())
		return hDlg;

	if (DisablePluginsOutput || ItemsNumber <= 0 || !Item)
		return hDlg;

	// ����! ������ ��������� ������������� X2 � Y2
	if (X2 < 0 || Y2 < 0)
		return hDlg;

	{
		Dialog *FarDialog = new Dialog(Item,ItemsNumber,DlgProc,Param);

		if (!FarDialog)
			return hDlg;

		if (!FarDialog->InitOK())
		{
			delete FarDialog;
			return hDlg;
		}

		hDlg = (HANDLE)FarDialog;
		FarDialog->SetPosition(X1,Y1,X2,Y2);

		if (Flags & FDLG_WARNING)
			FarDialog->SetDialogMode(DMODE_WARNINGSTYLE);

		if (Flags & FDLG_SMALLDIALOG)
			FarDialog->SetDialogMode(DMODE_SMALLDIALOG);

		if (Flags & FDLG_NODRAWSHADOW)
			FarDialog->SetDialogMode(DMODE_NODRAWSHADOW);

		if (Flags & FDLG_NODRAWPANEL)
			FarDialog->SetDialogMode(DMODE_NODRAWPANEL);

		if (Flags & FDLG_KEEPCONSOLETITLE)
			FarDialog->SetDialogMode(DMODE_KEEPCONSOLETITLE);

		if (Flags & FDLG_NONMODAL)
			FarDialog->SetCanLoseFocus(TRUE);

		FarDialog->SetHelp(HelpTopic);

		FarDialog->SetId(*Id);
		/* $ 29.08.2000 SVS
		   �������� ����� ������� - ������ � �������� ��� ������������ HelpTopic
		*/
		FarDialog->SetPluginNumber(PluginNumber);
	}
	return hDlg;
}

int WINAPI FarDialogRun(HANDLE hDlg)
{
	if (FrameManager->ManagerIsDown())
		return -1;

	if (hDlg==INVALID_HANDLE_VALUE) return -1;

	Frame *frame;

	if ((frame=FrameManager->GetBottomFrame()) )
		frame->Lock(); // ������� ���������� ������

	int ExitCode=-1;
	Dialog *FarDialog = (Dialog *)hDlg;

	if (Opt.ExceptRules)
	{
		CtrlObject->Plugins.Flags.Clear(PSIF_DIALOG);
		ExitCode=FarDialogExSehed(FarDialog);
	}
	else
	{
		FarDialog->Process();
		ExitCode=FarDialog->GetExitCode();
	}

	/* $ 15.05.2002 SKV
		������ ����������� ����� ����� ��, ��� ��������.
	*/
	if (frame )
		frame->Unlock(); // ������ ����� :-)

	//CheckScreenLock();
	FrameManager->RefreshFrame(); //?? - //AY - ��� ����� ���� ��������� ������ ����� ������ �� �������
	return(ExitCode);
}

void WINAPI FarDialogFree(HANDLE hDlg)
{
	if (hDlg!=INVALID_HANDLE_VALUE)
	{
		Dialog *FarDialog = (Dialog *)hDlg;
		delete FarDialog;
	}
}

const wchar_t* WINAPI FarGetMsgFn(INT_PTR PluginHandle,int MsgId)
{
	if (PluginHandle!=-1)
	{
		Plugin *pPlugin = (Plugin*)PluginHandle;
		if (pPlugin)
		{
			string strPath = pPlugin->GetModuleName();
			CutToSlash(strPath);

			if (pPlugin->InitLang(strPath))
				return pPlugin->GetMsg(MsgId);
		}
		_ASSERTE((PluginHandle!=-1) && FALSE);
	}
	else
	{
		_ASSERTE((PluginHandle!=-1) && FALSE);
	}
	return L"";
}

int WINAPI FarMessageFn(INT_PTR PluginNumber,const GUID* Id,unsigned __int64 Flags,const wchar_t *HelpTopic,
                        const wchar_t * const *Items,size_t ItemsNumber,
                        int ButtonsNumber)
{
	if (FrameManager->ManagerIsDown())
		return -1;

	if (DisablePluginsOutput)
		return -1;

	if ((!(Flags&(FMSG_ALLINONE|FMSG_ERRORTYPE)) && ItemsNumber<2) || !Items)
		return -1;

	wchar_t *SingleItems=nullptr;
	wchar_t *Msg;

	// ������ ���������� ����� ��� FMSG_ALLINONE
	if (Flags&FMSG_ALLINONE)
	{
		ItemsNumber=0;

		if (!(SingleItems=(wchar_t *)xf_malloc((StrLength((const wchar_t *)Items)+2)*sizeof(wchar_t))))
			return -1;

		Msg=wcscpy(SingleItems,(const wchar_t *)Items);

		while ((Msg = wcschr(Msg, L'\n')) )
		{
//      *Msg='\0';
			if (*++Msg == L'\0')
				break;

			++ItemsNumber;
		}

		ItemsNumber++; //??
	}

	const wchar_t **MsgItems=(const wchar_t **)xf_malloc(sizeof(wchar_t*)*(ItemsNumber+ADDSPACEFORPSTRFORMESSAGE));

	if (!MsgItems)
	{
		xf_free(SingleItems);
		return -1;
	}

	memset(MsgItems,0,sizeof(wchar_t*)*(ItemsNumber+ADDSPACEFORPSTRFORMESSAGE));

	if (Flags&FMSG_ALLINONE)
	{
		int I=0;
		Msg=SingleItems;
		// ������ ���������� ����� � �������� �� ������
		wchar_t *MsgTemp;

		while ((MsgTemp = wcschr(Msg, L'\n')) )
		{
			*MsgTemp=L'\0';
			MsgItems[I]=Msg;
			Msg+=StrLength(Msg)+1;

			if (*Msg == L'\0')
				break;

			++I;
		}

		if (*Msg)
		{
			MsgItems[I]=Msg;
		}
	}
	else
	{
		for (size_t i=0; i < ItemsNumber; i++)
			MsgItems[i]=Items[i];
	}

	// ����������� �� ������
	if (ItemsNumber > static_cast<SIZE_T>(ScrY-2))
	{
		ItemsNumber=ScrY-2-(Flags&0x000F0000?1:0);
	}

	/* $ 22.03.2001 tran
	   ItemsNumber++ -> ++ItemsNumber
	   �������� ��������� ������� */
	switch (Flags&0x000F0000)
	{
		case FMSG_MB_OK:
			ButtonsNumber=1;
			MsgItems[ItemsNumber++]=MSG(MOk);
			break;
		case FMSG_MB_OKCANCEL:
			ButtonsNumber=2;
			MsgItems[ItemsNumber++]=MSG(MOk);
			MsgItems[ItemsNumber++]=MSG(MCancel);
			break;
		case FMSG_MB_ABORTRETRYIGNORE:
			ButtonsNumber=3;
			MsgItems[ItemsNumber++]=MSG(MAbort);
			MsgItems[ItemsNumber++]=MSG(MRetry);
			MsgItems[ItemsNumber++]=MSG(MIgnore);
			break;
		case FMSG_MB_YESNO:
			ButtonsNumber=2;
			MsgItems[ItemsNumber++]=MSG(MYes);
			MsgItems[ItemsNumber++]=MSG(MNo);
			break;
		case FMSG_MB_YESNOCANCEL:
			ButtonsNumber=3;
			MsgItems[ItemsNumber++]=MSG(MYes);
			MsgItems[ItemsNumber++]=MSG(MNo);
			MsgItems[ItemsNumber++]=MSG(MCancel);
			break;
		case FMSG_MB_RETRYCANCEL:
			ButtonsNumber=2;
			MsgItems[ItemsNumber++]=MSG(MRetry);
			MsgItems[ItemsNumber++]=MSG(MCancel);
			break;
	}

	// ���������� �����
	if (PluginNumber != -1)
	{
		string strTopic;

		if (Help::MkTopic(PluginNumber,HelpTopic,strTopic))
			SetMessageHelp(strTopic);
	}

	// ���������������... �����
	Frame *frame;

	if ((frame=FrameManager->GetBottomFrame()) )
		frame->Lock(); // ������� ���������� ������

	int MsgCode=Message(Flags&(FMSG_WARNING|FMSG_ERRORTYPE|FMSG_KEEPBACKGROUND|FMSG_LEFTALIGN),ButtonsNumber,MsgItems[0],MsgItems+1,ItemsNumber-1,PluginNumber,Id);

	/* $ 15.05.2002 SKV
	  ������ ����������� ���� ����� ��, ��� ��������.
	*/
	if (frame )
		frame->Unlock(); // ������ ����� :-)

	//CheckScreenLock();

	if (SingleItems)
		xf_free(SingleItems);

	xf_free(MsgItems);
	return(MsgCode);
}

INT_PTR WINAPI FarPanelControl(HANDLE hPlugin,FILE_CONTROL_COMMANDS Command,int Param1,void* Param2)
{
	_FCTLLOG(CleverSysLog CSL(L"Control"));
	_FCTLLOG(SysLog(L"(hPlugin=0x%08X, Command=%s, Param1=[%d/0x%08X], Param2=[%d/0x%08X])",hPlugin,_FCTL_ToName(Command),(int)Param1,Param1,(int)Param2,Param2));
	_ALGO(CleverSysLog clv(L"FarPanelControl"));
	_ALGO(SysLog(L"(hPlugin=0x%08X, Command=%s, Param1=[%d/0x%08X], Param2=[%d/0x%08X])",hPlugin,_FCTL_ToName(Command),(int)Param1,Param1,(int)Param2,Param2));

	if (Command == FCTL_CHECKPANELSEXIST)
		return Opt.OnlyEditorViewerUsed? FALSE:TRUE;

	// ��� ������ ���������������� ��������
	if (gnMainThreadId != GetCurrentThreadId())
	{
		// ��������� ������� ��������� ������� ���� ��� ���������� ������� API
		// ������ ��� ��������� - ������, ������� ������ ���������, � �� � OpenPlugin �� ��?
		BOOL lbSafe = gbInOpenPlugin;
		if (!lbSafe)
		{
			ReportThreadUnsafeCall(L"Control(%u)", Command);
		}
	}

	if (Opt.OnlyEditorViewerUsed || !CtrlObject || !FrameManager || FrameManager->ManagerIsDown())
		return 0;

	FilePanels *FPanels=CtrlObject->Cp();
	CommandLine *CmdLine=CtrlObject->CmdLine;

	switch (Command)
	{
		case FCTL_CLOSEPANEL:
			g_strDirToSet = (wchar_t *)Param2;
		case FCTL_GETPANELINFO:
		case FCTL_GETPANELITEM:
		case FCTL_GETSELECTEDPANELITEM:
		case FCTL_GETCURRENTPANELITEM:
		case FCTL_GETPANELDIR:
		case FCTL_GETCOLUMNTYPES:
		case FCTL_GETCOLUMNWIDTHS:
		case FCTL_UPDATEPANEL:
		case FCTL_REDRAWPANEL:
		case FCTL_SETPANELDIR:
		case FCTL_BEGINSELECTION:
		case FCTL_SETSELECTION:
		case FCTL_CLEARSELECTION:
		case FCTL_ENDSELECTION:
		case FCTL_SETVIEWMODE:
		case FCTL_SETSORTMODE:
		case FCTL_SETSORTORDER:
		case FCTL_SETNUMERICSORT:
		case FCTL_SETCASESENSITIVESORT:
		case FCTL_SETDIRECTORIESFIRST:
		case FCTL_GETPANELFORMAT:
		case FCTL_GETPANELHOSTFILE:
		case FCTL_GETPANELPREFIX:
		{
			if (!FPanels)
				return FALSE;

			if ((hPlugin == PANEL_ACTIVE) || (hPlugin == PANEL_PASSIVE))
			{
				Panel *pPanel = (hPlugin == PANEL_ACTIVE)?FPanels->ActivePanel:FPanels->GetAnotherPanel(FPanels->ActivePanel);

				if (pPanel)
				{
					return pPanel->SetPluginCommand(Command,Param1,Param2);
				}

				return FALSE; //???
			}

			HANDLE hInternal;
			Panel *LeftPanel=FPanels->LeftPanel;
			Panel *RightPanel=FPanels->RightPanel;
			int Processed=FALSE;
			PluginHandle *PlHandle;

			if (LeftPanel && LeftPanel->GetMode()==PLUGIN_PANEL)
			{
				PlHandle=(PluginHandle *)LeftPanel->GetPluginHandle();

				if (PlHandle)
				{
					hInternal=PlHandle->hPlugin;

					if (hPlugin==hInternal)
					{
						Processed=LeftPanel->SetPluginCommand(Command,Param1,Param2);
					}
				}
			}

			if (RightPanel && RightPanel->GetMode()==PLUGIN_PANEL)
			{
				PlHandle=(PluginHandle *)RightPanel->GetPluginHandle();

				if (PlHandle)
				{
					hInternal=PlHandle->hPlugin;

					if (hPlugin==hInternal)
					{
						Processed=RightPanel->SetPluginCommand(Command,Param1,Param2);
					}
				}
			}

			return(Processed);
		}
		case FCTL_SETUSERSCREEN:
		{
			if (!FPanels || !FPanels->LeftPanel || !FPanels->RightPanel)
				return FALSE;

			KeepUserScreen++;
			FPanels->LeftPanel->ProcessingPluginCommand++;
			FPanels->RightPanel->ProcessingPluginCommand++;
			ScrBuf.FillBuf();
			ScrollScreen(1);
			SaveScreen SaveScr;
			{
				RedrawDesktop Redraw;
				CmdLine->Hide();
				SaveScr.RestoreArea(FALSE);
			}
			KeepUserScreen--;
			FPanels->LeftPanel->ProcessingPluginCommand--;
			FPanels->RightPanel->ProcessingPluginCommand--;
			return TRUE;
		}
		case FCTL_GETUSERSCREEN:
		{
			FrameManager->ShowBackground();
			int Lock=ScrBuf.GetLockCount();
			ScrBuf.SetLockCount(0);
			MoveCursor(0,ScrY-1);
			SetInitialCursorType();
			ScrBuf.Flush();
			ScrBuf.SetLockCount(Lock);
			return TRUE;
		}
		case FCTL_GETCMDLINE:
		{
			string strParam;

			CmdLine->GetString(strParam);

			if (Param1&&Param2)
				xwcsncpy((wchar_t*)Param2,strParam,Param1);

			return (int)strParam.GetLength()+1;
		}
		case FCTL_SETCMDLINE:
		case FCTL_INSERTCMDLINE:
		{
			CmdLine->DisableAC();
			if (Command==FCTL_SETCMDLINE)
				CmdLine->SetString((const wchar_t*)Param2);
			else
				CmdLine->InsertString((const wchar_t*)Param2);
			CmdLine->RevertAC();
			CmdLine->Redraw();
			return TRUE;
		}
		case FCTL_SETCMDLINEPOS:
		{
			CmdLine->SetCurPos(Param1);
			CmdLine->Redraw();
			return TRUE;
		}
		case FCTL_GETCMDLINEPOS:
		{
			if (Param2)
			{
				*(int *)Param2=CmdLine->GetCurPos();
				return TRUE;
			}

			return FALSE;
		}
		case FCTL_GETCMDLINESELECTION:
		{
			if (Param2)
			{
				CmdLineSelect *sel=(CmdLineSelect*)Param2;
				CmdLine->GetSelection(sel->SelStart,sel->SelEnd);
				return TRUE;
			}

			return FALSE;
		}
		case FCTL_SETCMDLINESELECTION:
		{
			if (Param2)
			{
				CmdLineSelect *sel=(CmdLineSelect*)Param2;
				CmdLine->Select(sel->SelStart,sel->SelEnd);
				CmdLine->Redraw();
				return TRUE;
			}

			return FALSE;
		}
		case FCTL_ISACTIVEPANEL:
		{
			if (hPlugin == PANEL_ACTIVE)
				return TRUE;

			Panel *pPanel = FPanels->ActivePanel;
			PluginHandle *PlHandle;

			if (pPanel && (pPanel->GetMode() == PLUGIN_PANEL))
			{
				PlHandle = (PluginHandle *)pPanel->GetPluginHandle();

				if (PlHandle)
				{
					if (PlHandle->hPlugin == hPlugin)
						return TRUE;
				}
			}

			return FALSE;
		}
		default:
			break;
	}

	return FALSE;
}


HANDLE WINAPI FarSaveScreen(int X1,int Y1,int X2,int Y2)
{
	if (DisablePluginsOutput || FrameManager->ManagerIsDown())
		return nullptr;

	if (X2==-1)
		X2=ScrX;

	if (Y2==-1)
		Y2=ScrY;

	return((HANDLE)(new SaveScreen(X1,Y1,X2,Y2)));
}


void WINAPI FarRestoreScreen(HANDLE hScreen)
{
	if (DisablePluginsOutput || FrameManager->ManagerIsDown())
		return;

	if (!hScreen)
		ScrBuf.FillBuf();

	if (hScreen)
		delete(SaveScreen *)hScreen;
}


static void PR_FarGetDirListMsg()
{
	Message(0,0,L"",MSG(MPreparingList));
}

int WINAPI FarGetDirList(const wchar_t *Dir,PluginPanelItem **pPanelItem,size_t *pItemsNumber)
{
	if (FrameManager->ManagerIsDown() || !Dir || !*Dir || !pItemsNumber || !pPanelItem)
		return FALSE;

	string strDirName;
	ConvertNameToFull(Dir, strDirName);
	{
		TPreRedrawFuncGuard preRedrawFuncGuard(PR_FarGetDirListMsg);
		SaveScreen SaveScr;
		clock_t StartTime=clock();
		int MsgOut=0;
		FAR_FIND_DATA_EX FindData;
		string strFullName;
		ScanTree ScTree(FALSE);
		ScTree.SetFindPath(strDirName,L"*");
		*pItemsNumber=0;
		*pPanelItem=nullptr;
		PluginPanelItem *ItemsList=nullptr;
		int ItemsNumber=0;

		while (ScTree.GetNextName(&FindData,strFullName))
		{
			if (!(ItemsNumber & 31))
			{
				if (CheckForEsc())
				{
					if (ItemsList)
						FarFreeDirList(ItemsList,ItemsNumber);

					return FALSE;
				}

				if (!MsgOut && clock()-StartTime > 500)
				{
					SetCursorType(FALSE,0);
					PR_FarGetDirListMsg();
					MsgOut=1;
				}

				ItemsList=(PluginPanelItem*)xf_realloc(ItemsList,sizeof(*ItemsList)*(ItemsNumber+32+1));

				if (!ItemsList)
				{
					return FALSE;
				}
			}

			ItemsList[ItemsNumber].FileAttributes = FindData.dwFileAttributes;
			ItemsList[ItemsNumber].FileSize = FindData.nFileSize;
			ItemsList[ItemsNumber].PackSize = FindData.nPackSize;
			ItemsList[ItemsNumber].CreationTime = FindData.ftCreationTime;
			ItemsList[ItemsNumber].LastAccessTime = FindData.ftLastAccessTime;
			ItemsList[ItemsNumber].LastWriteTime = FindData.ftLastWriteTime;
			ItemsList[ItemsNumber].FileName = xf_wcsdup(strFullName.CPtr());
			ItemsList[ItemsNumber].AlternateFileName = xf_wcsdup(FindData.strAlternateFileName);
			ItemsNumber++;
		}

		*pPanelItem=ItemsList;
		*pItemsNumber=ItemsNumber;
	}
	return TRUE;
}


static PluginPanelItem *PluginDirList;
static int DirListItemsNumber;
static string strPluginSearchPath;
static int StopSearch;
static HANDLE hDirListPlugin;
static int PluginSearchMsgOut;

static void FarGetPluginDirListMsg(const wchar_t *Name,DWORD Flags)
{
	Message(Flags,0,L"",MSG(MPreparingList),Name);
	PreRedrawItem preRedrawItem=PreRedraw.Peek();
	preRedrawItem.Param.Flags=Flags;
	preRedrawItem.Param.Param1=(void*)Name;
	PreRedraw.SetParam(preRedrawItem.Param);
}

static void PR_FarGetPluginDirListMsg()
{
	PreRedrawItem preRedrawItem=PreRedraw.Peek();
	FarGetPluginDirListMsg((const wchar_t *)preRedrawItem.Param.Param1,preRedrawItem.Param.Flags&(~MSG_KEEPBACKGROUND));
}

int WINAPI FarGetPluginDirList(INT_PTR PluginNumber,
                               HANDLE hPlugin,
                               const wchar_t *Dir,
                               PluginPanelItem **pPanelItem,
                               size_t *pItemsNumber)
{
	if (FrameManager->ManagerIsDown() || !Dir || !*Dir || !pItemsNumber || !pPanelItem)
		return FALSE;

	{
		if (!StrCmp(Dir,L".") || TestParentFolderName(Dir))
			return FALSE;

		static PluginHandle DirListPlugin;

		// � �� ����� �� ������ ���������� �� ������� ������?
		if (hPlugin==PANEL_ACTIVE || hPlugin==PANEL_PASSIVE)
		{
			/* $ 30.11.2001 DJ
			   � ���������� �� ��� ������?
			*/
			HANDLE Handle = ((hPlugin==PANEL_ACTIVE)?CtrlObject->Cp()->ActivePanel:CtrlObject->Cp()->GetAnotherPanel(CtrlObject->Cp()->ActivePanel))->GetPluginHandle();

			if (!Handle || Handle == INVALID_HANDLE_VALUE)
				return FALSE;

			DirListPlugin=*(PluginHandle *)Handle;
		}
		else
		{
			DirListPlugin.pPlugin=(Plugin*)PluginNumber;
			DirListPlugin.hPlugin=hPlugin;
		}

		{
			SaveScreen SaveScr;
			TPreRedrawFuncGuard preRedrawFuncGuard(PR_FarGetPluginDirListMsg);
			{
				string strDirName;
				strDirName = Dir;
				TruncStr(strDirName,30);
				CenterStr(strDirName,strDirName,30);
				SetCursorType(FALSE,0);
				FarGetPluginDirListMsg(strDirName,0);
				PluginSearchMsgOut=FALSE;
				hDirListPlugin=(HANDLE)&DirListPlugin;
				StopSearch=FALSE;
				*pItemsNumber=DirListItemsNumber=0;
				*pPanelItem=PluginDirList=nullptr;
				OpenPanelInfo Info;
				CtrlObject->Plugins.GetOpenPanelInfo(hDirListPlugin,&Info);
				string strPrevDir = Info.CurDir;

				if (CtrlObject->Plugins.SetDirectory(hDirListPlugin,Dir,OPM_SILENT))
				{
					strPluginSearchPath = Dir;
					strPluginSearchPath += L"\x1";
					ScanPluginDir();
					*pPanelItem=PluginDirList;
					*pItemsNumber=DirListItemsNumber;
					CtrlObject->Plugins.SetDirectory(hDirListPlugin,L"..",OPM_SILENT);
					OpenPanelInfo NewInfo;
					CtrlObject->Plugins.GetOpenPanelInfo(hDirListPlugin,&NewInfo);

					if (StrCmpI(strPrevDir, NewInfo.CurDir) )
					{
						PluginPanelItem *PanelData=nullptr;
						size_t ItemCount=0;

						if (CtrlObject->Plugins.GetFindData(hDirListPlugin,&PanelData,&ItemCount,OPM_SILENT))
						{
							CtrlObject->Plugins.FreeFindData(hDirListPlugin,PanelData,ItemCount);
						}

						CtrlObject->Plugins.SetDirectory(hDirListPlugin,strPrevDir,OPM_SILENT);
					}
				}
			}
		}
	}
	return(!StopSearch);
}

/* $ 30.11.2001 DJ
   ������� � ������� ����� ��� ��� ����������� ������ � ScanPluginDir()
*/

static void CopyPluginDirItem(PluginPanelItem *CurPanelItem)
{
	string strFullName;
	strFullName = strPluginSearchPath;
	strFullName += CurPanelItem->FileName;
	wchar_t *lpwszFullName = strFullName.GetBuffer();

	for (int I=0; lpwszFullName[I]; I++)
		if (lpwszFullName[I]==L'\x1')
			lpwszFullName[I]=L'\\';

	strFullName.ReleaseBuffer();
	PluginPanelItem *DestItem=PluginDirList+DirListItemsNumber;
	*DestItem=*CurPanelItem;

	if (CurPanelItem->UserData && (CurPanelItem->Flags & PPIF_USERDATA))
	{
		DWORD Size=*(DWORD *)CurPanelItem->UserData;
		DestItem->UserData=(DWORD_PTR)xf_malloc(Size);
		memcpy((void *)DestItem->UserData,(void *)CurPanelItem->UserData,Size);
	}

	DestItem->FileName = xf_wcsdup(strFullName);
	DestItem->AlternateFileName=nullptr;
	DirListItemsNumber++;
}

void ScanPluginDir()
{
	PluginPanelItem *PanelData=nullptr;
	size_t ItemCount=0;
	int AbortOp=FALSE;
	string strDirName;
	strDirName = strPluginSearchPath;
	wchar_t *lpwszDirName = strDirName.GetBuffer();

	for (int i=0; lpwszDirName[i]; i++)
		if (lpwszDirName[i]=='\x1')
			lpwszDirName[i]=lpwszDirName[i+1]?L'\\':0;

	strDirName.ReleaseBuffer();
	TruncStr(strDirName,30);
	CenterStr(strDirName,strDirName,30);

	if (CheckForEscSilent())
	{
		if (Opt.Confirm.Esc) // ����� ���������� ������?
			AbortOp=TRUE;

		if (ConfirmAbortOp())
			StopSearch=TRUE;
	}

	FarGetPluginDirListMsg(strDirName,AbortOp?0:MSG_KEEPBACKGROUND);

	if (StopSearch || !CtrlObject->Plugins.GetFindData(hDirListPlugin,&PanelData,&ItemCount,OPM_FIND))
		return;

	PluginPanelItem *NewList=(PluginPanelItem *)xf_realloc(PluginDirList,1+sizeof(*PluginDirList)*(DirListItemsNumber+ItemCount));

	if (!NewList)
	{
		StopSearch=TRUE;
		return;
	}

	PluginDirList=NewList;

	for (size_t i=0; i<ItemCount && !StopSearch; i++)
	{
		PluginPanelItem *CurPanelItem=PanelData+i;

		if (!(CurPanelItem->FileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			CopyPluginDirItem(CurPanelItem);
	}

	for (size_t i=0; i<ItemCount && !StopSearch; i++)
	{
		PluginPanelItem *CurPanelItem=PanelData+i;

		if ((CurPanelItem->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
		        StrCmp(CurPanelItem->FileName,L".") &&
		        !TestParentFolderName(CurPanelItem->FileName))
		{
			PluginPanelItem *NewList=(PluginPanelItem *)xf_realloc(PluginDirList,sizeof(*PluginDirList)*(DirListItemsNumber+1));

			if (!NewList)
			{
				StopSearch=TRUE;
				return;
			}

			PluginDirList=NewList;
			/* $ 30.11.2001 DJ
					���������� ����� ������� ��� ����������� FindData (�� ��������
					���������� PPIF_USERDATA)
			*/
			CopyPluginDirItem(CurPanelItem);
			string strFileName = CurPanelItem->FileName;

			if (CtrlObject->Plugins.SetDirectory(hDirListPlugin,strFileName,OPM_FIND))
			{
				strPluginSearchPath += CurPanelItem->FileName;
				strPluginSearchPath += L"\x1";
				ScanPluginDir();
				size_t pos = (size_t)-1;
				strPluginSearchPath.RPos(pos,L'\x1');
				strPluginSearchPath.SetLength(pos);

				if (strPluginSearchPath.RPos(pos,L'\x1'))
					strPluginSearchPath.SetLength(pos+1);
				else
					strPluginSearchPath.Clear();

				if (!CtrlObject->Plugins.SetDirectory(hDirListPlugin,L"..",OPM_FIND))
				{
					StopSearch=TRUE;
					break;
				}
			}
		}
	}

	CtrlObject->Plugins.FreeFindData(hDirListPlugin,PanelData,ItemCount);
}


void WINAPI FarFreeDirList(PluginPanelItem *PanelItem, size_t nItemsNumber)
{
	for (size_t I=0; I<nItemsNumber; I++)
	{
		PluginPanelItem *CurPanelItem=PanelItem+I;
		FreePluginPanelItem(CurPanelItem);
	}

	xf_free(PanelItem);
}


void WINAPI FarFreePluginDirList(PluginPanelItem *PanelItem, size_t ItemsNumber)
{
	if (!PanelItem)
		return;

	for (size_t I=0; I<ItemsNumber; I++)
	{
		PluginPanelItem *CurPanelItem=PanelItem+I;

		if (CurPanelItem->UserData && (CurPanelItem->Flags & PPIF_USERDATA))
		{
			xf_free((void *)CurPanelItem->UserData);
		}

		FreePluginPanelItem(CurPanelItem);
	}

	xf_free(PanelItem);
}

int WINAPI FarViewer(const wchar_t *FileName,const wchar_t *Title,
                     int X1,int Y1,int X2, int Y2,unsigned __int64 Flags, UINT CodePage)
{
	// ��� ������ ���������������� ��������
	if (gnMainThreadId != GetCurrentThreadId())
	{
		// ��������� ������� ��������� ������� ���� ��� ���������� ������� API
		// ������ ��� ��������� - ������, ������� ������ ���������, � �� � OpenPlugin �� ��?
		BOOL lbSafe = gbInOpenPlugin;
		if (!lbSafe)
		{
			ReportThreadUnsafeCall(L"Viewer()", 0);
		}
	}


	if (FrameManager->ManagerIsDown())
		return FALSE;

	class ConsoleTitle ct;
	int DisableHistory=(Flags & VF_DISABLEHISTORY)?TRUE:FALSE;

	// $ 15.05.2002 SKV - �������� ����� ������������ ��������� ������ �� ����������.
	if (FrameManager->InModalEV())
	{
		Flags&=~VF_NONMODAL;
	}

	if (Flags & VF_NONMODAL)
	{
		/* 09.09.2001 IS ! ������� ��� ����� � �������, ���� ����������� */
		FileViewer *Viewer=new FileViewer(FileName,TRUE,DisableHistory,Title,X1,Y1,X2,Y2,CodePage);

		if (!Viewer)
			return FALSE;

		/* $ 14.06.2002 IS
		   ��������� VF_DELETEONLYFILEONCLOSE - ���� ���� ����� ����� ������
		   ��������� �� ��������� � VF_DELETEONCLOSE
		*/
		if (Flags & (VF_DELETEONCLOSE|VF_DELETEONLYFILEONCLOSE))
			Viewer->SetTempViewName(FileName,(Flags&VF_DELETEONCLOSE)?TRUE:FALSE);

		Viewer->SetEnableF6((Flags & VF_ENABLE_F6) );

		/* $ 21.05.2002 SKV
		  ��������� ���� ���� ������ ���� �� ��� ������ ����.
		*/
		if (!(Flags&VF_IMMEDIATERETURN))
		{
			FrameManager->ExecuteNonModal();
		}
		else
		{
			if (GlobalSaveScrPtr)
				GlobalSaveScrPtr->Discard();

			FrameManager->PluginCommit();
		}
	}
	else
	{
		/* 09.09.2001 IS ! ������� ��� ����� � �������, ���� ����������� */
		FileViewer Viewer(FileName,FALSE,DisableHistory,Title,X1,Y1,X2,Y2,CodePage);
		/* $ 28.05.2001 �� ��������� �����, ������� ����� ����� ������� ��������� ���� */
		Viewer.SetDynamicallyBorn(false);
		FrameManager->EnterModalEV();
		FrameManager->ExecuteModal();
		FrameManager->ExitModalEV();

		/* $ 14.06.2002 IS
		   ��������� VF_DELETEONLYFILEONCLOSE - ���� ���� ����� ����� ������
		   ��������� �� ��������� � VF_DELETEONCLOSE
		*/
		if (Flags & (VF_DELETEONCLOSE|VF_DELETEONLYFILEONCLOSE))
			Viewer.SetTempViewName(FileName,(Flags&VF_DELETEONCLOSE)?TRUE:FALSE);

		Viewer.SetEnableF6((Flags & VF_ENABLE_F6) );

		if (!Viewer.GetExitCode())
		{
			return FALSE;
		}
	}

	return TRUE;
}


int WINAPI FarEditor(
    const wchar_t *FileName,
    const wchar_t *Title,
    int X1,
    int Y1,
    int X2,
    int Y2,
    unsigned __int64 Flags,
    int StartLine,
    int StartChar,
    UINT CodePage
)
{
	// ��� ������ ���������������� ��������
	if (gnMainThreadId != GetCurrentThreadId())
	{
		// ��������� ������� ��������� ������� ���� ��� ���������� ������� API
		// ������ ��� ��������� - ������, ������� ������ ���������, � �� � OpenPlugin �� ��?
		BOOL lbSafe = gbInOpenPlugin;
		if (!lbSafe)
		{
			ReportThreadUnsafeCall(L"Editor()", 0);
		}
	}

	if (FrameManager->ManagerIsDown())
		return EEC_OPEN_ERROR;

	ConsoleTitle ct;
	/* $ 12.07.2000 IS
	 �������� ������ ��������� (������ ��� ��������������) � ��������
	 ������������ ���������, ���� ���� ��������������� ����
	*/
	int CreateNew = (Flags & EF_CREATENEW)?TRUE:FALSE;
	int Locked=(Flags & EF_LOCKED)?TRUE:FALSE;
	int DisableHistory=(Flags & EF_DISABLEHISTORY)?TRUE:FALSE;
	int DisableSavePos=(Flags & EF_DISABLESAVEPOS)?TRUE:FALSE;
	/* $ 14.06.2002 IS
	   ��������� EF_DELETEONLYFILEONCLOSE - ���� ���� ����� ����� ������
	   ��������� �� ��������� � EF_DELETEONCLOSE
	*/
	int DeleteOnClose = 0;

	if (Flags & EF_DELETEONCLOSE)
		DeleteOnClose = 1;
	else if (Flags & EF_DELETEONLYFILEONCLOSE)
		DeleteOnClose = 2;

	int OpMode=FEOPMODE_QUERY;

	if ((Flags&EF_OPENMODE_MASK) )
		OpMode=Flags&EF_OPENMODE_MASK;

	/*$ 15.05.2002 SKV
	  �������� ����� ������������ ���������, ���� ��������� � ���������
	  ��������� ��� ������.
	*/
	if (FrameManager->InModalEV())
	{
		Flags&=~EF_NONMODAL;
	}

	int editorExitCode;
	int ExitCode=EEC_OPEN_ERROR;

	if (Flags & EF_NONMODAL)
	{
		/* 09.09.2001 IS ! ������� ��� ����� � �������, ���� ����������� */
		FileEditor *Editor=new FileEditor(FileName,CodePage,
		                                  (CreateNew?FFILEEDIT_CANNEWFILE:0)|FFILEEDIT_ENABLEF6|
		                                   (DisableHistory?FFILEEDIT_DISABLEHISTORY:0)|
		                                   (Locked?FFILEEDIT_LOCKED:0)|
		                                   (DisableSavePos?FFILEEDIT_DISABLESAVEPOS:0),
		                                  StartLine,StartChar,Title,
		                                  X1,Y1,X2,Y2,
		                                  DeleteOnClose,OpMode);

		if (Editor)
		{
			editorExitCode=Editor->GetExitCode();

			// ��������� - �������� ���� �������� (������ ��������� XC_OPEN_ERROR - ��. ��� FileEditor::Init())
			if (editorExitCode == XC_OPEN_ERROR || editorExitCode == XC_LOADING_INTERRUPTED)
			{
				delete Editor;
				Editor=nullptr;
				return editorExitCode;
			}

			Editor->SetEnableF6((Flags & EF_ENABLE_F6) );
			Editor->SetPluginTitle(Title);

			/* $ 21.05.2002 SKV - ��������� ���� ����, ������ ���� �� ��� ������ ����. */
			if (!(Flags&EF_IMMEDIATERETURN))
			{
				FrameManager->ExecuteNonModal();
			}
			else
			{
				if (GlobalSaveScrPtr)
					GlobalSaveScrPtr->Discard();

				FrameManager->PluginCommit();
			}

			ExitCode=XC_MODIFIED;
		}
	}
	else
	{
		/* 09.09.2001 IS ! ������� ��� ����� � �������, ���� ����������� */
		FileEditor Editor(FileName,CodePage,
		                  (CreateNew?FFILEEDIT_CANNEWFILE:0)|
		                    (DisableHistory?FFILEEDIT_DISABLEHISTORY:0)|
		                    (Locked?FFILEEDIT_LOCKED:0)|
		                    (DisableSavePos?FFILEEDIT_DISABLESAVEPOS:0),
		                  StartLine,StartChar,Title,
		                  X1,Y1,X2,Y2,
		                  DeleteOnClose,OpMode);
		editorExitCode=Editor.GetExitCode();

		// �������� ������������ (������ ������ ����� ����)
		if (editorExitCode == XC_OPEN_ERROR || editorExitCode == XC_LOADING_INTERRUPTED)
			ExitCode=editorExitCode;
		else
		{
			Editor.SetDynamicallyBorn(false);
			Editor.SetEnableF6((Flags & EF_ENABLE_F6) );
			Editor.SetPluginTitle(Title);
			/* $ 15.05.2002 SKV
			  ����������� ���� � ����� �/�� ���������� ���������.
			*/
			FrameManager->EnterModalEV();
			FrameManager->ExecuteModal();
			FrameManager->ExitModalEV();
			ExitCode = Editor.GetExitCode();

			if (ExitCode)
			{
#if 0

				if (OpMode==FEOPMODE_BREAKIFOPEN && ExitCode==XC_QUIT)
					ExitCode = XC_OPEN_ERROR;
				else
#endif
					ExitCode = Editor.IsFileChanged()?XC_MODIFIED:XC_NOT_MODIFIED;
			}
		}
	}

	return ExitCode;
}

void WINAPI FarText(int X,int Y,const FarColor* Color,const wchar_t *Str)
{
	if (DisablePluginsOutput || FrameManager->ManagerIsDown())
		return;

	if (!Str)
	{
		int PrevLockCount=ScrBuf.GetLockCount();
		ScrBuf.SetLockCount(0);
		ScrBuf.Flush();
		ScrBuf.SetLockCount(PrevLockCount);
	}
	else
	{
		Text(X,Y,*Color,Str);
	}
}


INT_PTR WINAPI FarEditorControl(int EditorID, EDITOR_CONTROL_COMMANDS Command, int Param1, void* Param2)
{
	// ��� ������ ���������������� ��������
	if (gnMainThreadId != GetCurrentThreadId())
	{
		// ��������� ������� ��������� ������� ���� ��� ���������� ������� API
		// ������ ��� ��������� - ������, ������� ������ ���������, � �� � OpenPlugin �� ��?
		BOOL lbSafe = gbInEditorEvent;
		if (!lbSafe)
		{
			ReportThreadUnsafeCall(L"EditorControl(%u)", Command);
		}
	}

	if (FrameManager->ManagerIsDown())
		return 0;

	if (EditorID == -1)
	{
		if (CtrlObject->Plugins.CurEditor)
			return CtrlObject->Plugins.CurEditor->EditorControl(Command,(void *)Param2);

		return 0;
	}
	else
	{
		int idx=0;
		Frame *frame;
		while((frame=FrameManager->Manager::operator[](idx++)) != nullptr)
		{
			if (frame->GetType() == MODALTYPE_EDITOR)
			{
				if (((FileEditor*)frame)->GetId() == EditorID)
				{
					return ((FileEditor*)frame)->EditorControl(Command,(void *)Param2);
				}
			}
		}
	}

	return 0;
}

INT_PTR WINAPI FarViewerControl(int ViewerID, VIEWER_CONTROL_COMMANDS Command, int Param1, void* Param2)
{
	// ��� ������ ���������������� ��������
	if (gnMainThreadId != GetCurrentThreadId())
	{
		// ��������� ������� ��������� ������� ���� ��� ���������� ������� API
		// ������ ��� ��������� - ������, ������� ������ ���������, � �� � OpenPlugin �� ��?
		BOOL lbSafe = gbInOpenPlugin;
		if (!lbSafe)
		{
			ReportThreadUnsafeCall(L"ViewerControl(%u)", Command);
		}
	}

	if (FrameManager->ManagerIsDown())
		return 0;

	if (ViewerID == -1)
	{
		if (CtrlObject->Plugins.CurViewer)
			return CtrlObject->Plugins.CurViewer->ViewerControl(Command,(void *)Param2);

		return 0;
	}
	else
	{
		int idx=0;
		Frame *frame;
		while((frame=FrameManager->Manager::operator[](idx++)) != nullptr)
		{
			if (frame->GetType() == MODALTYPE_VIEWER)
			{
				if (((FileViewer*)frame)->GetId() == ViewerID)
				{
					return ((FileViewer*)frame)->ViewerControl(Command,(void *)Param2);
				}
			}
		}
	}

	return 0;
}


void __stdcall farUpperBuf(wchar_t *Buf, int Length)
{
	return UpperBuf(Buf, Length);
}

void __stdcall farLowerBuf(wchar_t *Buf, int Length)
{
	return LowerBuf(Buf, Length);
}

void __stdcall farStrUpper(wchar_t *s1)
{
	return StrUpper(s1);
}

void __stdcall farStrLower(wchar_t *s1)
{
	return StrLower(s1);
}

wchar_t __stdcall farUpper(wchar_t Ch)
{
	return Upper(Ch);
}

wchar_t __stdcall farLower(wchar_t Ch)
{
	return Lower(Ch);
}

int __stdcall farStrCmpNI(const wchar_t *s1, const wchar_t *s2, int n)
{
	return StrCmpNI(s1, s2, n);
}

int __stdcall farStrCmpI(const wchar_t *s1, const wchar_t *s2)
{
	return StrCmpI(s1, s2);
}

int __stdcall farIsLower(wchar_t Ch)
{
	return IsLower(Ch);
}

int __stdcall farIsUpper(wchar_t Ch)
{
	return IsUpper(Ch);
}

int __stdcall farIsAlpha(wchar_t Ch)
{
	return IsAlpha(Ch);
}

int __stdcall farIsAlphaNum(wchar_t Ch)
{
	return IsAlphaNum(Ch);
}

size_t WINAPI farGetFileOwner(const wchar_t *Computer,const wchar_t *Name, wchar_t *Owner,size_t Size)
{
	string strOwner;
	GetFileOwner(Computer,Name,strOwner);

	if (Owner && Size)
		xwcsncpy(Owner,strOwner,Size);

	return strOwner.GetLength()+1;
}

size_t WINAPI farConvertPath(CONVERTPATHMODES Mode,const wchar_t *Src, wchar_t *Dest, size_t DestSize)
{
	if (Src && *Src)
	{
		string strDest;

		switch (Mode)
		{
			case CPM_NATIVE:
				strDest=NTPath(Src);
				break;
			case CPM_REAL:
				ConvertNameToReal(Src, strDest);
				break;
			case CPM_FULL:
			default:
				ConvertNameToFull(Src, strDest);
				break;
		}

		if (Dest && DestSize)
			xwcsncpy(Dest, strDest.CPtr(), DestSize);

		return strDest.GetLength() + 1;
	}
	else
	{
		if (Dest && DestSize)
			*Dest = 0;

		return 1;
	}
}

size_t WINAPI farGetReparsePointInfo(const wchar_t *Src, wchar_t *Dest, size_t DestSize)
{
	if (Src && *Src)
	{
		string strSrc(Src);
		string strDest;
		AddEndSlash(strDest);
		GetReparsePointInfo(strSrc,strDest,nullptr);

		if (DestSize && Dest)
			xwcsncpy(Dest,strDest,DestSize);

		return strDest.GetLength()+1;
	}
	else
	{
		if (DestSize && Dest)
			*Dest = 0;
		return 1;
	}
}

size_t WINAPI farGetPathRoot(const wchar_t *Path, wchar_t *Root, size_t DestSize)
{
	if (Path && *Path)
	{
		string strPath(Path), strRoot;
		GetPathRoot(strPath,strRoot);

		if (DestSize && Root)
			xwcsncpy(Root,strRoot,DestSize);

		return strRoot.GetLength()+1;
	}
	else
	{
		if (DestSize && Root)
			*Root = 0;

		return 1;
	}
}

INT_PTR WINAPI farMacroControl(const GUID* PluginId, FAR_MACRO_CONTROL_COMMANDS Command, int Param1, void* Param2)
{
	if (CtrlObject) // ��� ������� �� ���� ������.
	{
		KeyMacro& Macro=CtrlObject->Macro; //??

		switch (Command)
		{
			// Param1=0, Param2 - 0
			case MCTL_LOADALL: // �� ������� � ������ ��� � ���������� �����������
			{
				if (Macro.IsRecording())
					return FALSE;
				return Macro.LoadMacros(!Macro.IsExecuting());
			}

			// Param1=0, Param2 - 0
			case MCTL_SAVEALL: // �� ������ ���� � �������
			{
				if (Macro.IsRecording()) // || Macro.IsExecuting())
					return FALSE;

				Macro.SaveMacros();
				return TRUE;
			}

			// Param1=FARMACROSENDSTRINGCOMMAND, Param2...
			case MCTL_SENDSTRING:
			{
				if (!Param2)
					break;

				switch (Param1)
				{
					// Param1=FARMACROSENDSTRINGCOMMAND, Param2 - MacroSendMacroText*
					case MSSC_POST:
					{
						MacroSendMacroText *PlainText=(MacroSendMacroText*)Param2;
						if (PlainText->SequenceText && *PlainText->SequenceText)
						{
							return Macro.PostNewMacro(PlainText->SequenceText,(PlainText->Flags<<8)|MFLAGS_POSTFROMPLUGIN,InputRecordToKey(&PlainText->AKey));
						}

						break;
					}

					// Param1=FARMACROSENDSTRINGCOMMAND, Param2 - MacroSendMacroText*
					case MSSC_EXEC:
					{
						break;
					}

					// Param1=FARMACROSENDSTRINGCOMMAND, Param2 - MacroCheckMacroText*
					case MSSC_CHECK:
					{
						MacroCheckMacroText *CheckText=(MacroCheckMacroText*)Param2;
						if (CheckText->Text.SequenceText && *CheckText->Text.SequenceText)
						{
							MacroRecord CurMacro={};
							int Ret=Macro.ParseMacroString(&CurMacro,CheckText->Text.SequenceText,(CheckText->Text.Flags&KMFLAGS_SILENTCHECK)?TRUE:FALSE);

							if (Ret)
							{
								if (CurMacro.BufferSize > 1)
									xf_free(CurMacro.Buffer);

								ClearStruct(CheckText->Result);
								CheckText->Result.StructSize=sizeof(MacroParseResult);
							}
							else
							{
								static string ErrSrc;
								Macro.GetMacroParseError(&CheckText->Result.ErrCode,&CheckText->Result.ErrPos,&ErrSrc);
								CheckText->Result.ErrSrc=ErrSrc;
							}
							return Ret;
						}

						break;
					}
				}

				break;
			}

			// Param1=0, Param2 - 0
			case MCTL_GETSTATE:
			{
				return Macro.GetCurRecord(nullptr,nullptr);
			}

			// Param1=0, Param2 - 0
			case MCTL_GETAREA:
			{
				return Macro.GetMode();
			}

			case MCTL_ADDMACRO:
			{
				if (!Param2)
					break;
				MacroAddMacro *Data=(MacroAddMacro*)Param2;
				if (Data->SequenceText && *Data->SequenceText)
				{
					return Macro.AddMacro(Data->SequenceText,Data->Description,Data->Flags,Data->AKey,*PluginId,Data->Id,Data->Callback);
				}
				break;
			}

			case MCTL_DELMACRO:
			{
				return Macro.DelMacro(*PluginId,Param2);
			}
		}
	}


	return 0;
}

INT_PTR WINAPI farPluginsControl(HANDLE hHandle, FAR_PLUGINS_CONTROL_COMMANDS Command, int Param1, void* Param2)
{
	switch (Command)
	{
		case PCTL_LOADPLUGIN:
		case PCTL_UNLOADPLUGIN:
		case PCTL_FORCEDLOADPLUGIN:
		{
			if (Param1 == PLT_PATH)
			{
				if (Param2 )
				{
					string strPath;
					ConvertNameToFull((const wchar_t *)Param2, strPath);

					if (Command == PCTL_LOADPLUGIN)
						return CtrlObject->Plugins.LoadPluginExternal(strPath, false);
					else if (Command == PCTL_FORCEDLOADPLUGIN)
						return CtrlObject->Plugins.LoadPluginExternal(strPath, true);
					else
						return CtrlObject->Plugins.UnloadPluginExternal(strPath);
				}
			}

			break;
		}
	}

	return 0;
}

INT_PTR WINAPI farFileFilterControl(HANDLE hHandle, FAR_FILE_FILTER_CONTROL_COMMANDS Command, int Param1, void* Param2)
{
	FileFilter *Filter=nullptr;

	if (Command != FFCTL_CREATEFILEFILTER)
	{
		if (hHandle == INVALID_HANDLE_VALUE)
			return FALSE;

		Filter = (FileFilter *)hHandle;
	}

	switch (Command)
	{
		case FFCTL_CREATEFILEFILTER:
		{
			if (!Param2)
				break;

			*((HANDLE *)Param2) = INVALID_HANDLE_VALUE;

			if (hHandle != PANEL_ACTIVE && hHandle != PANEL_PASSIVE && hHandle != PANEL_NONE)
				break;

			switch (Param1)
			{
				case FFT_PANEL:
				case FFT_FINDFILE:
				case FFT_COPY:
				case FFT_SELECT:
				case FFT_CUSTOM:
					break;
				default:
					return FALSE;
			}

			Filter = new FileFilter((Panel *)hHandle, (FAR_FILE_FILTER_TYPE)Param1);

			if (Filter)
			{
				*((HANDLE *)Param2) = (HANDLE)Filter;
				return TRUE;
			}

			break;
		}
		case FFCTL_FREEFILEFILTER:
		{
			delete Filter;
			return TRUE;
		}
		case FFCTL_OPENFILTERSMENU:
		{
			return Filter->FilterEdit() ? TRUE : FALSE;
		}
		case FFCTL_STARTINGTOFILTER:
		{
			Filter->UpdateCurrentTime();
			return TRUE;
		}
		case FFCTL_ISFILEINFILTER:
		{
			if (!Param2)
				break;

			return Filter->FileInFilter(*(const PluginPanelItem *)Param2) ? TRUE : FALSE;
		}
	}

	return FALSE;
}

INT_PTR WINAPI farRegExpControl(HANDLE hHandle, FAR_REGEXP_CONTROL_COMMANDS Command, int Param1, void* Param2)
{
	RegExp* re=nullptr;

	if (Command != RECTL_CREATE)
	{
		if (hHandle == INVALID_HANDLE_VALUE)
			return FALSE;

		re = (RegExp*)hHandle;
	}

	switch (Command)
	{
		case RECTL_CREATE:

			if (!Param2)
				break;

			*((HANDLE*)Param2) = INVALID_HANDLE_VALUE;
			re = new RegExp;

			if (re)
			{
				*((HANDLE*)Param2) = (HANDLE)re;
				return TRUE;
			}

			break;
		case RECTL_FREE:
			delete re;
			return TRUE;
		case RECTL_COMPILE:
			return re->Compile((const wchar_t*)Param2,OP_PERLSTYLE);
		case RECTL_OPTIMIZE:
			return re->Optimize();
		case RECTL_MATCHEX:
		{
			RegExpSearch* data=(RegExpSearch*)Param2;
			return re->MatchEx(data->Text,data->Text+data->Position,data->Text+data->Length,data->Match,data->Count
#ifdef NAMEDBRACKETS
			                   ,data->Reserved
#endif
			                  );
		}
		case RECTL_SEARCHEX:
		{
			RegExpSearch* data=(RegExpSearch*)Param2;
			return re->SearchEx(data->Text,data->Text+data->Position,data->Text+data->Length,data->Match,data->Count
#ifdef NAMEDBRACKETS
			                    ,data->Reserved
#endif
			                   );
		}
		case RECTL_BRACKETSCOUNT:
			return re->GetBracketsCount();
	}

	return FALSE;
}

INT_PTR WINAPI farSettingsControl(HANDLE hHandle, FAR_SETTINGS_CONTROL_COMMANDS Command, int Param1, void* Param2)
{
	AbstractSettings* settings=nullptr;

	if (Command != SCTL_CREATE)
	{
		if (hHandle == INVALID_HANDLE_VALUE)
			return FALSE;

		settings = (AbstractSettings*)hHandle;
	}

	switch (Command)
	{
		case SCTL_CREATE:

			if (!Param2)
				break;

			{
			    FarSettingsCreate* data = (FarSettingsCreate*)Param2;
				if (data->StructSize>=sizeof(FarSettingsCreate))
				{
					if(IsEqualGUID(FarGuid,data->Guid)) settings=new FarSettings();
					else settings=new PluginSettings(data->Guid);
					if (settings->IsValid())
					{
						data->Handle=settings;
						return TRUE;
					}
					delete settings;
				}
			}
			break;
		case SCTL_FREE:
			delete settings;
			return TRUE;
		case SCTL_SET:
			return settings->Set(*(const FarSettingsItem*)Param2);
		case SCTL_GET:
			return settings->Get(*(FarSettingsItem*)Param2);
		case SCTL_ENUM:
			return settings->Enum(*(FarSettingsEnum*)Param2);
		case SCTL_DELETE:
			return settings->Delete(*(const FarSettingsValue*)Param2);
		case SCTL_CREATESUBKEY:
		case SCTL_OPENSUBKEY:
			return settings->SubKey(*(const FarSettingsValue*)Param2, Command==SCTL_CREATESUBKEY);
	}

	return FALSE;
}

size_t WINAPI farGetCurrentDirectory(size_t Size,wchar_t* Buffer)
{
	string strCurDir;
	apiGetCurrentDirectory(strCurDir);

	if (Buffer && Size)
	{
		xwcsncpy(Buffer,strCurDir,Size);
	}

	return strCurDir.GetLength()+1;
}
