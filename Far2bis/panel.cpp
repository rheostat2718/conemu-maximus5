/*
panel.cpp

Parent class ��� �������
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
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

#include "panel.hpp"
#include "macroopcode.hpp"
#include "keyboard.hpp"
#include "flink.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "vmenu.hpp"
#include "filepanels.hpp"
#include "cmdline.hpp"
#include "chgmmode.hpp"
#include "chgprior.hpp"
#include "edit.hpp"
#include "treelist.hpp"
#include "filelist.hpp"
#include "dialog.hpp"
#include "savescr.hpp"
#include "manager.hpp"
#include "ctrlobj.hpp"
#include "scrbuf.hpp"
#include "array.hpp"
#include "lockscrn.hpp"
#include "help.hpp"
#include "syslog.hpp"
#include "plugapi.hpp"
#include "network.hpp"
#include "cddrv.hpp"
#include "interf.hpp"
#include "message.hpp"
#include "hotplug.hpp"
#include "eject.hpp"
#include "clipboard.hpp"
#include "config.hpp"
#include "scrsaver.hpp"
#include "execute.hpp"
#include "ffolders.hpp"
#include "options.hpp"
#include "pathmix.hpp"
#include "dirmix.hpp"
#include "imports.hpp"
#include "constitle.hpp"
#include "FarDlgBuilder.hpp"
#include "setattr.hpp"
#include "window.hpp"
#include "palette.hpp"

static int DragX,DragY,DragMove;
static Panel *SrcDragPanel;
static SaveScreen *DragSaveScr=nullptr;
static string strDragName;

static int MessageRemoveConnection(wchar_t Letter, int &UpdateProfile);

/* $ 21.08.2002 IS
   ����� ��� �������� ������ ������� � ���� ������ ������
*/

class ChDiskPluginItem
{
	public:
		MenuItemEx Item;
		WCHAR HotKey;

		ChDiskPluginItem() { Clear(); }
		~ChDiskPluginItem() {}

		void Clear() { HotKey = 0; Item.Clear(); }
		bool operator==(const ChDiskPluginItem &rhs) const { return HotKey==rhs.HotKey && !StrCmpI(Item.strName,rhs.Item.strName) && Item.UserData==rhs.Item.UserData; }
		int operator<(const ChDiskPluginItem &rhs) const {return (Opt.ChangeDriveMode&DRIVE_SORT_PLUGINS_BY_HOTKEY && HotKey!=rhs.HotKey)?unsigned(HotKey-1)<unsigned(rhs.HotKey-1):StrCmpI(Item.strName,rhs.Item.strName)<0;}
		const ChDiskPluginItem& operator=(const ChDiskPluginItem &rhs);
};

const ChDiskPluginItem& ChDiskPluginItem::operator=(const ChDiskPluginItem &rhs)
{
	if (this != &rhs)
	{
		Item=rhs.Item;
		HotKey=rhs.HotKey;
	}

	return *this;
}


Panel::Panel():
	Focus(0),
	EnableUpdate(TRUE),
	PanelMode(NORMAL_PANEL),
	PrevViewMode(VIEW_3),
	NumericSort(0),
	CaseSensitiveSort(0),
	DirectoriesFirst(1),
	ModalMode(0),
	ViewSettings(),
	ProcessingPluginCommand(0)
{
	_OT(SysLog(L"[%p] Panel::Panel()", this));
	SrcDragPanel=nullptr;
	DragX=DragY=-1;
};


Panel::~Panel()
{
	_OT(SysLog(L"[%p] Panel::~Panel()", this));
	EndDrag();
}


void Panel::SetViewMode(int ViewMode)
{
	PrevViewMode=ViewMode;
	Panel::ViewMode=ViewMode;
};


void Panel::ChangeDirToCurrent()
{
	string strNewDir;
	apiGetCurrentDirectory(strNewDir);
	SetCurDir(strNewDir,TRUE);
}


void Panel::ChangeDisk()
{
	int Pos=0,FirstCall=TRUE;

	if (!strCurDir.IsEmpty() && strCurDir.At(1)==L':')
	{
		Pos=Upper(strCurDir.At(0))-L'A';
	}

	while (Pos!=-1)
	{
		Pos=ChangeDiskMenu(Pos,FirstCall);
		FirstCall=FALSE;
	}
}


struct PanelMenuItem
{
	bool bIsPlugin;

	union
	{
		struct
		{
			Plugin *pPlugin;
			int nItem;
		};

		struct
		{
			wchar_t cDrive;
			int nDriveType;
		};
	};
};

struct TypeMessage
{
	int DrvType;
	int FarMsg;
};

const TypeMessage DrTMsg[]=
{
	{DRIVE_REMOVABLE,MChangeDriveRemovable},
	{DRIVE_FIXED,MChangeDriveFixed},
	{DRIVE_REMOTE,MChangeDriveNetwork},
	{DRIVE_REMOTE_NOT_CONNECTED,MChangeDriveDisconnectedNetwork},
	{DRIVE_CDROM,MChangeDriveCDROM},
	{DRIVE_CD_RW,MChangeDriveCD_RW},
	{DRIVE_CD_RWDVD,MChangeDriveCD_RWDVD},
	{DRIVE_DVD_ROM,MChangeDriveDVD_ROM},
	{DRIVE_DVD_RW,MChangeDriveDVD_RW},
	{DRIVE_DVD_RAM,MChangeDriveDVD_RAM},
	{DRIVE_BD_ROM,MChangeDriveBD_ROM},
	{DRIVE_BD_RW,MChangeDriveBD_RW},
	{DRIVE_HDDVD_ROM,MChangeDriveHDDVD_ROM},
	{DRIVE_HDDVD_RW,MChangeDriveHDDVD_RW},
	{DRIVE_RAMDISK,MChangeDriveRAM},
	{DRIVE_SUBSTITUTE,MChangeDriveSUBST},
	{DRIVE_VIRTUAL,MChangeDriveVirtual},
	{DRIVE_USBDRIVE,MChangeDriveRemovable},
};

static size_t AddPluginItems(VMenu &ChDisk, int Pos, int DiskCount, bool SetSelected)
{
	TArray<ChDiskPluginItem> MPItems;
	ChDiskPluginItem OneItem;
	int PluginItem, PluginNumber = 0; // IS: �������� - �������� � ������� �������
	bool ItemPresent,Done=false;
	string strMenuText;
	string strPluginText;
	size_t PluginMenuItemsCount = 0;

	while (!Done)
	{
		for (PluginItem=0;; ++PluginItem)
		{
			if (PluginNumber >= CtrlObject->Plugins.GetPluginsCount())
			{
				Done=true;
				break;
			}

			Plugin *pPlugin = CtrlObject->Plugins.GetPlugin(PluginNumber);

			WCHAR HotKey = 0;
			if (!CtrlObject->Plugins.GetDiskMenuItem(
			            pPlugin,
			            PluginItem,
			            ItemPresent,
			            HotKey,
			            strPluginText
			        ))
			{
				Done=true;
				break;
			}

			if (!ItemPresent)
				break;

			strMenuText = strPluginText;

			if (!strMenuText.IsEmpty())
			{
				OneItem.Clear();
				PanelMenuItem *item = new PanelMenuItem;
				item->bIsPlugin = true;
				item->pPlugin = pPlugin;
				item->nItem = PluginItem;

				if (pPlugin->IsOemPlugin())
					OneItem.Item.Flags=LIF_CHECKED|L'A';

				OneItem.Item.strName = strMenuText;
				OneItem.Item.UserDataSize=sizeof(PanelMenuItem);
				OneItem.Item.UserData=(char*)item;
				OneItem.HotKey=HotKey;
				ChDiskPluginItem *pResult = MPItems.addItem(OneItem);

				if (pResult)
				{
					pResult->Item.UserData = (char*)item; //BUGBUG, ��� ���������� ������. ���������!!!! ������� � ������� TArray
					pResult->Item.UserDataSize = sizeof(PanelMenuItem);
				}

				/*
				else BUGBUG, � ��� ���, ������, ������
				{
					Done=TRUE;
					break;
				}
				*/
			}
		} // END: for (PluginItem=0;;++PluginItem)

		++PluginNumber;
	}

	MPItems.Sort();
	MPItems.Pack(); // ������� �����
	PluginMenuItemsCount=MPItems.getSize();

	if (PluginMenuItemsCount)
	{
		MenuItemEx ChDiskItem;

		ChDiskItem.Clear();
		ChDiskItem.Flags|=LIF_SEPARATOR;
		ChDiskItem.UserDataSize=0;
		ChDisk.AddItem(&ChDiskItem);
		ChDiskItem.Flags&=~LIF_SEPARATOR;

		for (size_t I=0; I < PluginMenuItemsCount; ++I)
		{
			if (Pos > DiskCount && !SetSelected)
			{
				MPItems.getItem(I)->Item.SetSelect(DiskCount+static_cast<int>(I)+1==Pos);

				if (!SetSelected)
					SetSelected=DiskCount+static_cast<int>(I)+1==Pos;
			}
			const wchar_t HotKeyStr[]={MPItems.getItem(I)->HotKey?L'&':L' ',MPItems.getItem(I)->HotKey?MPItems.getItem(I)->HotKey:L' ',L' ',MPItems.getItem(I)->HotKey?L' ':L'\0',L'\0'};
			MPItems.getItem(I)->Item.strName = string(HotKeyStr) + MPItems.getItem(I)->Item.strName;
			ChDisk.AddItem(&MPItems.getItem(I)->Item);

			delete(PanelMenuItem*)MPItems.getItem(I)->Item.UserData;  //����...
		}
	}
	return PluginMenuItemsCount;
}

static void ConfigureChangeDriveMode()
{
	DialogBuilder Builder(MChangeDriveConfigure, L"");
	Builder.AddCheckbox(MChangeDriveShowDiskType, &Opt.ChangeDriveMode, DRIVE_SHOW_TYPE);
	Builder.AddCheckbox(MChangeDriveShowNetworkName, &Opt.ChangeDriveMode, DRIVE_SHOW_NETNAME);
	Builder.AddCheckbox(MChangeDriveShowLabel, &Opt.ChangeDriveMode, DRIVE_SHOW_LABEL);
	Builder.AddCheckbox(MChangeDriveShowFileSystem, &Opt.ChangeDriveMode, DRIVE_SHOW_FILESYSTEM);

	BOOL ShowSizeAny = Opt.ChangeDriveMode & (DRIVE_SHOW_SIZE | DRIVE_SHOW_SIZE_FLOAT);

	DialogItemEx *ShowSize = Builder.AddCheckbox(MChangeDriveShowSize, &ShowSizeAny);
	DialogItemEx *ShowSizeFloat = Builder.AddCheckbox(MChangeDriveShowSizeFloat, &Opt.ChangeDriveMode, DRIVE_SHOW_SIZE_FLOAT);
	ShowSizeFloat->Indent(3);
	Builder.LinkFlags(ShowSize, ShowSizeFloat, DIF_DISABLE);

	Builder.AddCheckbox(MChangeDriveShowRemovableDrive, &Opt.ChangeDriveMode, DRIVE_SHOW_REMOVABLE);
	Builder.AddCheckbox(MChangeDriveShowPlugins, &Opt.ChangeDriveMode, DRIVE_SHOW_PLUGINS);
	Builder.AddCheckbox(MChangeDriveSortPluginsByHotkey, &Opt.ChangeDriveMode, DRIVE_SORT_PLUGINS_BY_HOTKEY)->Indent(4);
	Builder.AddCheckbox(MChangeDriveShowCD, &Opt.ChangeDriveMode, DRIVE_SHOW_CDROM);
	Builder.AddCheckbox(MChangeDriveShowNetworkDrive, &Opt.ChangeDriveMode, DRIVE_SHOW_REMOTE);

	Builder.AddOKCancel();
	if (Builder.ShowDialog())
	{
		if (ShowSizeAny)
		{
			bool ShowSizeFloat = (Opt.ChangeDriveMode & DRIVE_SHOW_SIZE_FLOAT) ? true : false;
			if (ShowSizeFloat)
				Opt.ChangeDriveMode &= ~DRIVE_SHOW_SIZE;
			else
				Opt.ChangeDriveMode |= DRIVE_SHOW_SIZE;
		}
		else
			Opt.ChangeDriveMode &= ~(DRIVE_SHOW_SIZE | DRIVE_SHOW_SIZE_FLOAT);
	}
}


LONG_PTR WINAPI ChDiskDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
	switch (Msg)
	{
		case DN_CTLCOLORDLGITEM:
		{
			if (Param1 == 1) // BUGBUG, magic number
			{
				int Color=FarColorToReal(COL_WARNDIALOGTEXT);
				return ((Param2&0xFF00FF00)|(Color<<16)|Color);
			}
		}
		break;
	}
	return DefDlgProc(hDlg,Msg,Param1,Param2);
}

int Panel::ChangeDiskMenu(int Pos,int FirstCall)
{
	Events.DeviceArivalEvent.Reset();
	Events.DeviceRemoveEvent.Reset();
	Events.MediaArivalEvent.Reset();
	Events.MediaRemoveEvent.Reset();

	class Guard_Macro_DskShowPosType  //����� �����-��
	{
		public:
			Guard_Macro_DskShowPosType(Panel *curPanel) {Macro_DskShowPosType=(curPanel==CtrlObject->Cp()->LeftPanel)?1:2;};
			~Guard_Macro_DskShowPosType() {Macro_DskShowPosType=0;};
	};
	Guard_Macro_DskShowPosType _guard_Macro_DskShowPosType(this);
	MenuItemEx ChDiskItem;
	string strDiskType, strRootDir, strDiskLetter;
	DWORD Mask,DiskMask;
	int DiskCount,Focus;
	WCHAR I;
	bool SetSelected=false;
	Mask = FarGetLogicalDrives();
	DWORD NetworkMask = 0;
	AddSavedNetworkDisks(Mask, NetworkMask);

	for (DiskMask=Mask,DiskCount=0; DiskMask; DiskMask>>=1)
		DiskCount+=DiskMask & 1;

	PanelMenuItem Item, *mitem=0;
	{ // ��� ������ ����, ��. M#605
		VMenu ChDisk(MSG(MChangeDriveTitle),nullptr,0,ScrY-Y1-3);
		ChDisk.SetBottomTitle(MSG(MChangeDriveMenuFooter));
		ChDisk.SetFlags(VMENU_NOTCENTER);

		if (this == CtrlObject->Cp()->LeftPanel)
			ChDisk.SetFlags(VMENU_LEFTMOST);

		ChDisk.SetHelp(L"DriveDlg");
		ChDisk.SetFlags(VMENU_WRAPMODE);
		string strMenuText;
		int DriveType,MenuLine;
		int LabelWidth = Max(11,StrLength(MSG(MChangeDriveLabelAbsent)));
		int DiskTypeWidth=0;

		for (size_t J=0; J < ARRAYSIZE(DrTMsg); ++J)
		{
			DiskTypeWidth = Max(DiskTypeWidth,StrLength(MSG(DrTMsg[J].FarMsg)));
		}

		/* $ 02.04.2001 VVM
		! ������� �� ������ ������ �����... */
		for (DiskMask=Mask,MenuLine=I=0; DiskMask; DiskMask>>=1,I++)
		{
			if (!(DiskMask & 1))   //���� �����
				continue;

			wchar_t Drv[]={L'&',L'A'+I,L':',L'\\',L'\0'};
			strRootDir=Drv+1;
			Drv[3]=L' ';
			strMenuText=Drv;
			DriveType = FAR_GetDriveType(strRootDir,nullptr,Opt.ChangeDriveMode & DRIVE_SHOW_CDROM?0x01:0);

			if ((1<<I)&NetworkMask)
				DriveType = DRIVE_REMOTE_NOT_CONNECTED;

			string strAssocPath;

			if (Opt.ChangeDriveMode & DRIVE_SHOW_TYPE)
			{
				strDiskType.Format(L"%*s",StrLength(MSG(MChangeDriveFixed)),L"");

				const wchar_t LocalName[]={strRootDir.At(0),L':',L'\0'};

				if (GetSubstName(DriveType, LocalName, strAssocPath))
				{
					DriveType=DRIVE_SUBSTITUTE;
				}
				else if(DriveType == DRIVE_FIXED && GetVHDName(LocalName, strAssocPath))
				{
					DriveType=DRIVE_VIRTUAL;
				}

				for (size_t J=0; J < ARRAYSIZE(DrTMsg); ++J)
				{
					if (DrTMsg[J].DrvType == DriveType)
					{
						strDiskType = MSG(DrTMsg[J].FarMsg);
						break;
					}
				}

				strDiskType.Format(L"%-*s",DiskTypeWidth,strDiskType.CPtr());
				strMenuText += strDiskType;
			}

			int ShowDisk = (DriveType!=DRIVE_REMOVABLE || (Opt.ChangeDriveMode & DRIVE_SHOW_REMOVABLE)) &&
			               (!IsDriveTypeCDROM(DriveType) || (Opt.ChangeDriveMode & DRIVE_SHOW_CDROM)) &&
			               (!IsDriveTypeRemote(DriveType) || (Opt.ChangeDriveMode & DRIVE_SHOW_REMOTE));

			if (Opt.ChangeDriveMode & (DRIVE_SHOW_LABEL|DRIVE_SHOW_FILESYSTEM))
			{
				string strVolumeName, strFileSystemName;

				if (ShowDisk && !apiGetVolumeInformation(
				            strRootDir,
				            &strVolumeName,
				            nullptr,
				            nullptr,
				            nullptr,
				            &strFileSystemName
				        ))
				{
					strVolumeName = MSG(MChangeDriveLabelAbsent);
					ShowDisk = FALSE;
				}

				string strTemp;

				if (Opt.ChangeDriveMode & DRIVE_SHOW_LABEL)
				{
					TruncStrFromEnd(strVolumeName,LabelWidth);
					strTemp.Format(L"%c%-*s",BoxSymbols[BS_V1],LabelWidth,strVolumeName.CPtr());
					strMenuText += strTemp;
				}

				if (Opt.ChangeDriveMode & DRIVE_SHOW_FILESYSTEM)
				{
					strTemp.Format(L"%c%-8.8s",BoxSymbols[BS_V1],strFileSystemName.CPtr());
					strMenuText += strTemp;
				}
			}

			if (Opt.ChangeDriveMode & (DRIVE_SHOW_SIZE|DRIVE_SHOW_SIZE_FLOAT))
			{
				string strTotalText, strFreeText;
				unsigned __int64 TotalSize,TotalFree,UserFree;

				if (ShowDisk && apiGetDiskSize(strRootDir,&TotalSize,&TotalFree,&UserFree))
				{
					if (Opt.ChangeDriveMode & DRIVE_SHOW_SIZE)
					{
						//������ ��� ������� � ����������
						FileSizeToStr(strTotalText,TotalSize,9,COLUMN_COMMAS|COLUMN_MINSIZEINDEX|1);
						FileSizeToStr(strFreeText,UserFree,9,COLUMN_COMMAS|COLUMN_MINSIZEINDEX|1);
					}
					else
					{
						//������ � ������ � ��� 0 ��������� ����� ������� (B)
						FileSizeToStr(strTotalText,TotalSize,9,COLUMN_FLOATSIZE|COLUMN_SHOWBYTESINDEX);
						FileSizeToStr(strFreeText,UserFree,9,COLUMN_FLOATSIZE|COLUMN_SHOWBYTESINDEX);
					}
				}

				string strTemp;
				strTemp.Format(L"%c%-9s%c%-9s",BoxSymbols[BS_V1],strTotalText.CPtr(),BoxSymbols[BS_V1],strFreeText.CPtr());
				strMenuText += strTemp;
			}

			if (Opt.ChangeDriveMode & DRIVE_SHOW_NETNAME)
			{
				switch(DriveType)
				{
				case DRIVE_REMOTE:
					{
						string strRemoteName;
						DriveLocalToRemoteName(DriveType,strRootDir.At(0),strRemoteName);
						//TruncPathStr(strRemoteName,ScrX-(int)strMenuText.GetLength()-12);

						if (!strRemoteName.IsEmpty())
						{
							strMenuText += L"  ";
							strMenuText += strRemoteName;
						}
					}
					break;

				case DRIVE_SUBSTITUTE:
				case DRIVE_VIRTUAL:
					if(!strAssocPath.IsEmpty())
					{
						strMenuText += L"  ";
						strMenuText += strAssocPath;
					}
					break;
				}
			}

			ChDiskItem.Clear();

			if (FirstCall)
			{
				ChDiskItem.SetSelect(I==Pos);

				if (!SetSelected)
					SetSelected=(I==Pos);
			}
			else
			{
				if (Pos < DiskCount)
				{
					ChDiskItem.SetSelect(MenuLine==Pos);

					if (!SetSelected)
						SetSelected=(MenuLine==Pos);
				}
			}

			ChDiskItem.strName = strMenuText;

			PanelMenuItem item;
			item.bIsPlugin = false;
			item.cDrive = L'A'+I;
			item.nDriveType = DriveType;
			ChDisk.SetUserData(&item, sizeof(item), ChDisk.AddItem(&ChDiskItem));
			MenuLine++;
		}

		size_t PluginMenuItemsCount=0;

		if (Opt.ChangeDriveMode & DRIVE_SHOW_PLUGINS)
		{
			PluginMenuItemsCount = AddPluginItems(ChDisk, Pos, DiskCount, SetSelected);
		}

		int X=X1+5;

		if ((this == CtrlObject->Cp()->RightPanel) && IsFullScreen() && (X2-X1 > 40))
			X = (X2-X1+1)/2+5;

		int Y = (ScrY+1-(DiskCount+static_cast<int>(PluginMenuItemsCount)+5))/2;

		if (Y < 1)
			Y=1;

		ChDisk.SetPosition(X,Y,0,0);

		if (Y < 3)
			ChDisk.SetBoxType(SHORT_DOUBLE_BOX);

		ChDisk.Show();

		while (!ChDisk.Done())
		{
			int Key;
			if(Events.DeviceArivalEvent.Signaled() || Events.DeviceRemoveEvent.Signaled() || Events.MediaArivalEvent.Signaled() || Events.MediaRemoveEvent.Signaled())
			{
				Key=KEY_CTRLR;
			}
			else
			{
				{ //��������� �����
					ChangeMacroMode MacroMode(MACRO_DISKS);
					Key=ChDisk.ReadInput();
				}
			}
			int SelPos=ChDisk.GetSelectPos();
			PanelMenuItem *item = (PanelMenuItem*)ChDisk.GetUserData(nullptr,0);

			switch (Key)
			{
				// Shift-Enter � ���� ������ ������ �������� ��������� ��� ������� �����
				case KEY_SHIFTNUMENTER:
				case KEY_SHIFTENTER:
				{
					if (item && !item->bIsPlugin)
					{
						wchar_t DosDeviceName[]={item->cDrive,L':',L'\\',L'\0'};
						Execute(DosDeviceName,FALSE,TRUE,TRUE);
					}
				}
				break;
				case KEY_CTRLPGUP:
				case KEY_CTRLNUMPAD9:
				{
					if (Opt.PgUpChangeDisk)
						return -1;
				}
				break;
				// �.�. ��� ������� �������� ��������� "����������" ����������,
				// �� ������� ��������� Ins ��� CD - "������� ����"
				case KEY_INS:
				case KEY_NUMPAD0:
				{
					if (item && !item->bIsPlugin)
					{
						if (IsDriveTypeCDROM(item->nDriveType) /* || DriveType == DRIVE_REMOVABLE*/)
						{
							SaveScreen SvScrn;
							EjectVolume(item->cDrive, EJECT_LOAD_MEDIA);
							return SelPos;
						}
					}
				}
				break;
				case KEY_NUMDEL:
				case KEY_DEL:
				{
					if (item && !item->bIsPlugin)
					{
						int Code = DisconnectDrive(item, ChDisk);
						if (Code != DRIVE_DEL_FAIL && Code != DRIVE_DEL_NONE)
						{
							ScrBuf.Lock(); // �������� ������ ����������
							FrameManager->ResizeAllFrame();
							FrameManager->PluginCommit(); // ��������.
							ScrBuf.Unlock(); // ��������� ����������
							return (((DiskCount-SelPos)==1) && (SelPos > 0) && (Code != DRIVE_DEL_EJECT))?SelPos-1:SelPos;
						}
					}
				}
				break;
				case KEY_CTRLA:
				case KEY_F4:
				{
					if (item)
					{
						if (!item->bIsPlugin)
						{
							wchar_t DeviceName[]={item->cDrive,L':',L'\\',L'\0'};
							ShellSetFileAttributes(nullptr,DeviceName);
							ChDisk.Redraw();
						}
						else
						{
							string strRegKey;
							CtrlObject->Plugins.GetHotKeyRegKey(item->pPlugin, item->nItem,strRegKey);
							string strName = ChDisk.GetItemPtr(SelPos)->strName + 3;
							RemoveExternalSpaces(strName);
							if(CtrlObject->Plugins.SetHotKeyDialog(strName, strRegKey, L"DriveMenuHotkey"))
							{
								return SelPos;
							}
						}
					}
					break;
				}
				case KEY_SHIFTNUMDEL:
				case KEY_SHIFTDECIMAL:
				case KEY_SHIFTDEL:
				{
					if (item && !item->bIsPlugin)
					{
						RemoveHotplugDevice(item, ChDisk);
						return SelPos;
					}
				}
				break;
				case KEY_CTRL1:
				case KEY_RCTRL1:
					Opt.ChangeDriveMode ^= DRIVE_SHOW_TYPE;
					return SelPos ;
				case KEY_CTRL2:
				case KEY_RCTRL2:
					Opt.ChangeDriveMode ^= DRIVE_SHOW_NETNAME;
					return SelPos;
				case KEY_CTRL3:
				case KEY_RCTRL3:
					Opt.ChangeDriveMode ^= DRIVE_SHOW_LABEL;
					return SelPos;
				case KEY_CTRL4:
				case KEY_RCTRL4:
					Opt.ChangeDriveMode ^= DRIVE_SHOW_FILESYSTEM;
					return SelPos;
				case KEY_CTRL5:
				case KEY_RCTRL5:
				{
					if (Opt.ChangeDriveMode & DRIVE_SHOW_SIZE)
					{
						Opt.ChangeDriveMode ^= DRIVE_SHOW_SIZE;
						Opt.ChangeDriveMode |= DRIVE_SHOW_SIZE_FLOAT;
					}
					else
					{
						if (Opt.ChangeDriveMode & DRIVE_SHOW_SIZE_FLOAT)
							Opt.ChangeDriveMode ^= DRIVE_SHOW_SIZE_FLOAT;
						else
							Opt.ChangeDriveMode ^= DRIVE_SHOW_SIZE;
					}

					return SelPos;
				}
				case KEY_CTRL6:
				case KEY_RCTRL6:
					Opt.ChangeDriveMode ^= DRIVE_SHOW_REMOVABLE;
					return SelPos;
				case KEY_CTRL7:
				case KEY_RCTRL7:
					Opt.ChangeDriveMode ^= DRIVE_SHOW_PLUGINS;
					return SelPos;
				case KEY_CTRL8:
				case KEY_RCTRL8:
					Opt.ChangeDriveMode ^= DRIVE_SHOW_CDROM;
					return SelPos;
				case KEY_CTRL9:
				case KEY_RCTRL9:
					Opt.ChangeDriveMode ^= DRIVE_SHOW_REMOTE;
					return SelPos;
				case KEY_F9:
					ConfigureChangeDriveMode();
					return SelPos;
				case KEY_SHIFTF1:
				{
					if (item && item->bIsPlugin)
					{
						// �������� ������ �����, ������� �������� � CommandsMenu()
						FarShowHelp(
						    item->pPlugin->GetModuleName(),
						    nullptr,
						    FHELP_SELFHELP|FHELP_NOSHOWERROR|FHELP_USECONTENTS
						);
					}

					break;
				}
				case KEY_ALTSHIFTF9:

					if (Opt.ChangeDriveMode&DRIVE_SHOW_PLUGINS)
					{
						ChDisk.Hide();
						CtrlObject->Plugins.Configure();
					}

					return SelPos;
				case KEY_SHIFTF9:

					if (item && item->bIsPlugin && item->pPlugin->HasConfigure())
						CtrlObject->Plugins.ConfigureCurrent(item->pPlugin, item->nItem);

					return SelPos;
				case KEY_CTRLR:
					return SelPos;
				default:
					ChDisk.ProcessInput();
					break;
			}

			if (ChDisk.Done() &&
			        (ChDisk.Modal::GetExitCode() < 0) &&
			        !strCurDir.IsEmpty() &&
			        (StrCmpN(strCurDir,L"\\\\",2) ))
			{
				const wchar_t RootDir[4] = {strCurDir.At(0),L':',L'\\',L'\0'};

				if (FAR_GetDriveType(RootDir) == DRIVE_NO_ROOT_DIR)
					ChDisk.ClearDone();
			}
		} // while (!Done)

		if (ChDisk.Modal::GetExitCode()<0)
			return -1;

		mitem=(PanelMenuItem*)ChDisk.GetUserData(nullptr,0);

		if (mitem)
		{
			Item=*mitem;
			mitem=&Item;
		}
	} // ��� ������ ����, ��. M#605

	if (Opt.CloseCDGate && mitem && !mitem->bIsPlugin && IsDriveTypeCDROM(mitem->nDriveType))
	{
		const wchar_t RootDir[]={mitem->cDrive,L':',L'\0'};

		if (!apiIsDiskInDrive(RootDir))
		{
			if (!EjectVolume(mitem->cDrive, EJECT_READY|EJECT_NO_MESSAGE))
			{
				SaveScreen SvScrn;
				Message(0,0,L"",MSG(MChangeWaitingLoadDisk));
				EjectVolume(mitem->cDrive, EJECT_LOAD_MEDIA|EJECT_NO_MESSAGE);
			}
		}
	}

	if (ProcessPluginEvent(FE_CLOSE,nullptr))
		return -1;

	ScrBuf.Flush();
	INPUT_RECORD rec;
	PeekInputRecord(&rec);

	if (!mitem)
		return -1; //???

	if (!mitem->bIsPlugin)
	{
		for (;;)
		{
			wchar_t NewDir[]={mitem->cDrive,L':',0,0};

			if (NetworkMask & (1<<(mitem->cDrive-L'A')))
			{
				ConnectToNetworkDrive(NewDir);
			}

			if (FarChDir(NewDir))
			{
				break;
			}
			else
			{
				NewDir[2]=L'\\';

				if (FarChDir(NewDir))
				{
					break;
				}
			}

			enum
			{
				CHDISKERROR_DOUBLEBOX,
				CHDISKERROR_TEXT0,
				CHDISKERROR_TEXT1,
				CHDISKERROR_FIXEDIT,
				CHDISKERROR_TEXT2,
				CHDISKERROR_SEPARATOR,
				CHDISKERROR_BUTTON_OK,
				CHDISKERROR_BUTTON_CANCEL,
			};
			const wchar_t Drive[]={mitem->cDrive,L'\0'};
			string strError;
			GetErrorString(strError);
			int Len1=static_cast<int>(strError.GetLength());
			int Len2=StrLength(MSG(MChangeDriveCannotReadDisk));
			int MaxMsg=Min(Max(Len1,Len2), static_cast<int>(MAX_WIDTH_MESSAGE));
			const int DX=Max(MaxMsg+13,40),DY=8;
			const DialogDataEx ChDiskData[]=
			{
				DI_DOUBLEBOX,3,1,DX-4,DY-2,0,0,MSG(MError),
				DI_EDIT,5,2,DX-6,2,0,DIF_READONLY,strError.CPtr(),
				DI_TEXT,5,3,DX-9,3,0,0,MSG(MChangeDriveCannotReadDisk),
				DI_FIXEDIT,5+Len2+1,3,5+Len2+1,3,0,DIF_FOCUS,Drive,
				DI_TEXT,5+Len2+2,3,5+Len2+2,3,0,0,L":",
				DI_TEXT,3,DY-4,0,DY-4,0,DIF_SEPARATOR,L"",
				DI_BUTTON,0,DY-3,0,DY-3,0,DIF_DEFAULT|DIF_CENTERGROUP,MSG(MRetry),
				DI_BUTTON,0,DY-3,0,DY-3,0,DIF_CENTERGROUP,MSG(MCancel),
			};
			MakeDialogItemsEx(ChDiskData,ChDiskDlg);
			Dialog Dlg(ChDiskDlg, ARRAYSIZE(ChDiskData), ChDiskDlgProc, 0);
			Dlg.SetPosition(-1,-1,DX,DY);
			Dlg.SetDialogMode(DMODE_WARNINGSTYLE);
			Dlg.Process();
			if(Dlg.GetExitCode()==CHDISKERROR_BUTTON_OK)
			{
				mitem->cDrive=ChDiskDlg[CHDISKERROR_FIXEDIT].strData.At(0);
			}
			else
			{
				return -1;
			}
		}

		string strNewCurDir;
		apiGetCurrentDirectory(strNewCurDir);

		if ((PanelMode == NORMAL_PANEL) &&
		        (GetType() == FILE_PANEL) &&
		        !StrCmpI(strCurDir,strNewCurDir) &&
		        IsVisible())
		{
			// � ����� �� ������ ����� Update????
			Update(UPDATE_KEEP_SELECTION);
		}
		else
		{
			Focus=GetFocus();
			Panel *NewPanel=CtrlObject->Cp()->ChangePanel(this, FILE_PANEL, TRUE, FALSE);
			NewPanel->SetCurDir(strNewCurDir,TRUE);
			NewPanel->Show();

			if (Focus || !CtrlObject->Cp()->GetAnotherPanel(this)->IsVisible())
				NewPanel->SetFocus();

			if (!Focus && CtrlObject->Cp()->GetAnotherPanel(this)->GetType() == INFO_PANEL)
				CtrlObject->Cp()->GetAnotherPanel(this)->UpdateKeyBar();
		}
	}
	else //��� ������, ��
	{
		HANDLE hPlugin = CtrlObject->Plugins.OpenPlugin(
		                     mitem->pPlugin,
		                     OPEN_DISKMENU,
		                     mitem->nItem
		                 );

		if (hPlugin != INVALID_HANDLE_VALUE)
		{
			Focus=GetFocus();
			Panel *NewPanel = CtrlObject->Cp()->ChangePanel(this,FILE_PANEL,TRUE,TRUE);
			NewPanel->SetPluginMode(hPlugin,L"",Focus || !CtrlObject->Cp()->GetAnotherPanel(NewPanel)->IsVisible());
			NewPanel->Update(0);
			NewPanel->Show();

			if (!Focus && CtrlObject->Cp()->GetAnotherPanel(this)->GetType() == INFO_PANEL)
				CtrlObject->Cp()->GetAnotherPanel(this)->UpdateKeyBar();
		}
	}

	return -1;
}

int Panel::DisconnectDrive(PanelMenuItem *item, VMenu &ChDisk)
{
	if ((item->nDriveType == DRIVE_REMOVABLE) || IsDriveTypeCDROM(item->nDriveType))
	{
		if ((item->nDriveType == DRIVE_REMOVABLE) && !IsEjectableMedia(item->cDrive))
			return -1;

		// ������ ������� ������� ����

		if (!EjectVolume(item->cDrive, EJECT_NO_MESSAGE))
		{
			// ���������� ��������� �������
			int CMode=GetMode();
			int AMode=CtrlObject->Cp()->GetAnotherPanel(this)->GetMode();
			string strTmpCDir, strTmpADir;
			GetCurDir(strTmpCDir);
			CtrlObject->Cp()->GetAnotherPanel(this)->GetCurDir(strTmpADir);
			// �������� ����, ����� ���� � ����������� ���� ����� ����
			// (���� ���� ������� ������ ������)
			ChDisk.Hide();
			ChDisk.Lock(); // ... � �������� �� �����������.
			// "���� �� �������������"
			int DoneEject=FALSE;

			while (!DoneEject)
			{
				// "��������� ����" - �������� ��� ������������� � �������� �������
				// TODO: � ���� �������� ������� - CD? ;-)
				IfGoHome(item->cDrive);
				// ��������� ������� ���������� ��� ������ ���������
				int ResEject = EjectVolume(item->cDrive, EJECT_NO_MESSAGE);

				if (!ResEject)
				{
					// ����������� ���� - ��� ������� ��� �� ����� ������ � ������.
					if (AMode != PLUGIN_PANEL)
						CtrlObject->Cp()->GetAnotherPanel(this)->SetCurDir(strTmpADir, FALSE);

					if (CMode != PLUGIN_PANEL)
						SetCurDir(strTmpCDir, FALSE);

					// ... � ������� ����� �...
					string strMsgText;
					strMsgText.Format(MSG(MChangeCouldNotEjectMedia), item->cDrive);
					SetLastError(ERROR_DRIVE_LOCKED); // ...� "The disk is in use or locked by another process."
					DoneEject = Message(
					                MSG_WARNING|MSG_ERRORTYPE,
					                2,
					                MSG(MError),
					                strMsgText,
					                MSG(MRetry),
					                MSG(MCancel)
					            ) ;
				}
				else
					DoneEject=TRUE;
			}

			// "��������" ������ ������ ������
			ChDisk.Unlock();
			ChDisk.Show();
		}
		return DRIVE_DEL_NONE;
	}
	else
	{
		return ProcessDelDisk(item->cDrive, item->nDriveType, &ChDisk);
	}
}

void Panel::RemoveHotplugDevice(PanelMenuItem *item, VMenu &ChDisk)
{
	int Code = ProcessRemoveHotplugDevice(item->cDrive, EJECT_NOTIFY_AFTERREMOVE);

	if (!Code)
	{
		// ���������� ��������� �������
		int CMode=GetMode();
		int AMode=CtrlObject->Cp()->GetAnotherPanel(this)->GetMode();
		string strTmpCDir, strTmpADir;
		GetCurDir(strTmpCDir);
		CtrlObject->Cp()->GetAnotherPanel(this)->GetCurDir(strTmpADir);
		// �������� ����, ����� ���� � ����������� ���� ����� ����
		// (���� ���� ������� ������ ������)
		ChDisk.Hide();
		ChDisk.Lock(); // ... � �������� �� �����������.
		// "���� �� �������������"
		int DoneEject=FALSE;

		while (!DoneEject)
		{
			// "��������� ����" - �������� ��� ������������� � �������� �������
			// TODO: � ���� �������� ������� - USB? ;-)
			IfGoHome(item->cDrive);
			// ��������� ������� ���������� ��� ������ ���������
			Code = ProcessRemoveHotplugDevice(item->cDrive, EJECT_NO_MESSAGE|EJECT_NOTIFY_AFTERREMOVE);

			if (!Code)
			{
				// ����������� ���� - ��� ������� ��� �� ����� ������ � ������.
				if (AMode != PLUGIN_PANEL)
					CtrlObject->Cp()->GetAnotherPanel(this)->SetCurDir(strTmpADir, FALSE);

				if (CMode != PLUGIN_PANEL)
					SetCurDir(strTmpCDir, FALSE);

				// ... � ������� ����� �...
				string strMsgText;
				strMsgText.Format(MSG(MChangeCouldNotEjectHotPlugMedia), item->cDrive);
				SetLastError(ERROR_DRIVE_LOCKED); // ...� "The disk is in use or locked by another process."
				DoneEject = Message(
				                MSG_WARNING|MSG_ERRORTYPE,
				                2,
				                MSG(MError),
				                strMsgText,
				                MSG(MHRetry),
				                MSG(MHCancel)
				            ) ;
			}
			else
				DoneEject=TRUE;
		}

		// "��������" ������ ������ ������
		ChDisk.Unlock();
		ChDisk.Show();
	}
}

/* $ 28.12.2001 DJ
   ��������� Del � ���� ������
*/

int Panel::ProcessDelDisk(wchar_t Drive, int DriveType,VMenu *ChDiskMenu)
{
	string strMsgText;
	wchar_t DiskLetter[]={Drive,L':',0};

	switch(DriveType)
	{
	case DRIVE_SUBSTITUTE:
		{
			if (Opt.Confirm.RemoveSUBST)
			{
				strMsgText.Format(MSG(MChangeSUBSTDisconnectDriveQuestion),Drive);
				if (Message(MSG_WARNING,2,MSG(MChangeSUBSTDisconnectDriveTitle),strMsgText,MSG(MYes),MSG(MNo)))
				{
					return DRIVE_DEL_FAIL;
				}
			}
			if (DelSubstDrive(DiskLetter))
			{
				return DRIVE_DEL_SUCCESS;
			}
			else
			{
				int LastError=GetLastError();
				strMsgText.Format(MSG(MChangeDriveCannotDelSubst),DiskLetter);
				if (LastError==ERROR_OPEN_FILES || LastError==ERROR_DEVICE_IN_USE)
				{
					if (!Message(MSG_WARNING|MSG_ERRORTYPE,2,MSG(MError),strMsgText,
								L"\x1",MSG(MChangeDriveOpenFiles),
								MSG(MChangeDriveAskDisconnect),MSG(MOk),MSG(MCancel)))
					{
						if (DelSubstDrive(DiskLetter))
						{
							return DRIVE_DEL_SUCCESS;
						}
					}
					else
					{
						return DRIVE_DEL_FAIL;
					}
				}
				Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),strMsgText,MSG(MOk));
			}
			return DRIVE_DEL_FAIL; // ����. � ������� ��� ����� ��� ��� ����...
		}
		break;

	case DRIVE_REMOTE:
		{
			int UpdateProfile=CONNECT_UPDATE_PROFILE;
			if (MessageRemoveConnection(Drive,UpdateProfile))
			{
				// <�������>
				LockScreen LckScr;
				// ���� �� ��������� �� ��������� ����� - ������ � ����, ����� �� ������
				// ��������
				IfGoHome(Drive);
				FrameManager->ResizeAllFrame();
				FrameManager->GetCurrentFrame()->Show();
				ChDiskMenu->Show();
				// </�������>

				if (WNetCancelConnection2(DiskLetter,UpdateProfile,FALSE)==NO_ERROR)
				{
					return DRIVE_DEL_SUCCESS;
				}
				else
				{
					int LastError=GetLastError();
					strMsgText.Format(MSG(MChangeDriveCannotDisconnect),DiskLetter);
					if (LastError==ERROR_OPEN_FILES || LastError==ERROR_DEVICE_IN_USE)
					{
						if (!Message(MSG_WARNING|MSG_ERRORTYPE,2,MSG(MError),strMsgText,
									L"\x1",MSG(MChangeDriveOpenFiles),
									MSG(MChangeDriveAskDisconnect),MSG(MOk),MSG(MCancel)))
						{
							if (WNetCancelConnection2(DiskLetter,UpdateProfile,TRUE)==NO_ERROR)
							{
								return DRIVE_DEL_SUCCESS;
							}
						}
						else
						{
							return DRIVE_DEL_FAIL;
						}
					}
					const wchar_t RootDir[]={*DiskLetter,L':',L'\\',L'\0'};
					if (FAR_GetDriveType(RootDir)==DRIVE_REMOTE)
					{
						Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),strMsgText,MSG(MOk));
					}
				}
				return DRIVE_DEL_FAIL;
			}
		}
		break;

	case DRIVE_VIRTUAL:
		{
			if (Opt.Confirm.DetachVHD)
			{
				strMsgText.Format(MSG(MChangeVHDDisconnectDriveQuestion),Drive);
				if (Message(MSG_WARNING,2,MSG(MChangeVHDDisconnectDriveTitle),strMsgText,MSG(MYes),MSG(MNo)))
				{
					return DRIVE_DEL_FAIL;
				}
			}
			string strVhdPath;
			if(GetVHDName(DiskLetter, strVhdPath) && !strVhdPath.IsEmpty())
			{
				VIRTUAL_STORAGE_TYPE vst;
				vst.DeviceId=VIRTUAL_STORAGE_TYPE_DEVICE_VHD;
				const GUID VIRTUAL_STORAGE_TYPE_VENDOR_MICROSOFT={0xec984aec, 0xa0f9, 0x47e9, 0x90, 0x1f, 0x71, 0x41, 0x5a, 0x66, 0x34, 0x5b};
				vst.VendorId=VIRTUAL_STORAGE_TYPE_VENDOR_MICROSOFT;
				OPEN_VIRTUAL_DISK_PARAMETERS ovdp = {OPEN_VIRTUAL_DISK_VERSION_1, 0};
				HANDLE Handle;
				if(ifn.pfnOpenVirtualDisk(&vst, strVhdPath, VIRTUAL_DISK_ACCESS_DETACH, OPEN_VIRTUAL_DISK_FLAG_NONE, &ovdp, &Handle) == ERROR_SUCCESS)
				{
					if(ifn.pfnDetachVirtualDisk(Handle, DETACH_VIRTUAL_DISK_FLAG_NONE, 0) == ERROR_SUCCESS)
					{
						return DRIVE_DEL_SUCCESS;
					}
					else
					{
						return DRIVE_DEL_FAIL;
					}
				}
			}
		}
		break;

	}
	return DRIVE_DEL_FAIL;
}


void Panel::FastFindProcessName(Edit *FindEdit,const wchar_t *Src,string &strLastName,string &strName)
{
	wchar_t *Ptr=(wchar_t *)xf_malloc((StrLength(Src)+StrLength(FindEdit->GetStringAddr())+32)*sizeof(wchar_t));

	if (Ptr)
	{
		wcscpy(Ptr,FindEdit->GetStringAddr());
		wchar_t *EndPtr=Ptr+StrLength(Ptr);
		wcscat(Ptr,Src);
		Unquote(EndPtr);
		EndPtr=Ptr+StrLength(Ptr);
		DWORD Key;

		for (;;)
		{
			if (EndPtr == Ptr)
			{
				Key=KEY_NONE;
				break;
			}

			if (FindPartName(Ptr,FALSE,1,1))
			{
				Key=*(EndPtr-1);
				*EndPtr=0;
				FindEdit->SetString(Ptr);
				strLastName = Ptr;
				strName = Ptr;
				FindEdit->Show();
				break;
			}

			*--EndPtr=0;
		}

		xf_free(Ptr);
	}
}

__int64 Panel::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
	return 0;
}

// ������������� ����
static DWORD _CorrectFastFindKbdLayout(INPUT_RECORD *rec,DWORD Key)
{
	if ((Key&KEY_ALT))// && Key!=(KEY_ALT|0x3C))
	{
		// // _SVS(SysLog(L"_CorrectFastFindKbdLayout>>> %s | %s",_FARKEY_ToName(Key),_INPUT_RECORD_Dump(rec)));
		if (rec->Event.KeyEvent.uChar.UnicodeChar && (Key&KEY_MASKF) != rec->Event.KeyEvent.uChar.UnicodeChar) //???
			Key=(Key&0xFFF10000)|rec->Event.KeyEvent.uChar.UnicodeChar;   //???

		// // _SVS(SysLog(L"_CorrectFastFindKbdLayout<<< %s | %s",_FARKEY_ToName(Key),_INPUT_RECORD_Dump(rec)));
	}

	return Key;
}

void Panel::FastFind(int FirstKey)
{
	// // _SVS(CleverSysLog Clev(L"Panel::FastFind"));
	INPUT_RECORD rec;
	string strLastName, strName;
	int Key,KeyToProcess=0;
	WaitInFastFind++;
	{
		int FindX=Min(X1+9,ScrX-22);
		int FindY=Min(Y2,ScrY-2);
		ChangeMacroMode MacroMode(MACRO_SEARCH);
		SaveScreen SaveScr(FindX,FindY,FindX+21,FindY+2);
		FastFindShow(FindX,FindY);
		Edit FindEdit;
		FindEdit.SetPosition(FindX+2,FindY+1,FindX+19,FindY+1);
		FindEdit.SetEditBeyondEnd(FALSE);
		FindEdit.SetObjectColor(COL_DIALOGEDIT);
		FindEdit.Show();

		while (!KeyToProcess)
		{
			if (FirstKey)
			{
				FirstKey=_CorrectFastFindKbdLayout(FrameManager->GetLastInputRecord(),FirstKey);
				// // _SVS(SysLog(L"Panel::FastFind  FirstKey=%s  %s",_FARKEY_ToName(FirstKey),_INPUT_RECORD_Dump(FrameManager->GetLastInputRecord())));
				// // _SVS(SysLog(L"if (FirstKey)"));
				Key=FirstKey;
			}
			else
			{
				// // _SVS(SysLog(L"else if (FirstKey)"));
				Key=GetInputRecord(&rec);

				if (rec.EventType==MOUSE_EVENT)
				{
					if (!(rec.Event.MouseEvent.dwButtonState & 3))
						continue;
					else
						Key=KEY_ESC;
				}
				else if (!rec.EventType || rec.EventType==KEY_EVENT || rec.EventType==FARMACRO_KEY_EVENT)
				{
					// ��� ������� ������������� ������������...
					if (Key==KEY_CTRLV || Key==KEY_SHIFTINS || Key==KEY_SHIFTNUMPAD0)
					{
						wchar_t *ClipText=PasteFromClipboard();

						if (ClipText)
						{
							if (*ClipText)
							{
								FastFindProcessName(&FindEdit,ClipText,strLastName,strName);
								FastFindShow(FindX,FindY);
							}

							xf_free(ClipText);
						}

						continue;
					}
					else if (Key == KEY_OP_XLAT)
					{
						string strTempName;
						FindEdit.Xlat();
						FindEdit.GetString(strTempName);
						FindEdit.SetString(L"");
						FastFindProcessName(&FindEdit,strTempName,strLastName,strName);
						FastFindShow(FindX,FindY);
						continue;
					}
					else if (Key == KEY_OP_PLAINTEXT)
					{
						string strTempName;
						FindEdit.ProcessKey(Key);
						FindEdit.GetString(strTempName);
						FindEdit.SetString(L"");
						FastFindProcessName(&FindEdit,strTempName,strLastName,strName);
						FastFindShow(FindX,FindY);
						continue;
					}
					else
						Key=_CorrectFastFindKbdLayout(&rec,Key);
				}
			}

			if (Key==KEY_ESC || Key==KEY_F10)
			{
				KeyToProcess=KEY_NONE;
				break;
			}

			// // _SVS(if (!FirstKey) SysLog(L"Panel::FastFind  Key=%s  %s",_FARKEY_ToName(Key),_INPUT_RECORD_Dump(&rec)));
			if (Key>=KEY_ALT_BASE+0x01 && Key<=KEY_ALT_BASE+65535)
				Key=Lower(static_cast<WCHAR>(Key-KEY_ALT_BASE));

			if (Key>=KEY_ALTSHIFT_BASE+0x01 && Key<=KEY_ALTSHIFT_BASE+65535)
				Key=Lower(static_cast<WCHAR>(Key-KEY_ALTSHIFT_BASE));

			if (Key==KEY_MULTIPLY)
				Key=L'*';

			switch (Key)
			{
				case KEY_F1:
				{
					FindEdit.Hide();
					SaveScr.RestoreArea();
					{
						Help Hlp(L"FastFind");
					}
					FindEdit.Show();
					FastFindShow(FindX,FindY);
					break;
				}
				case KEY_CTRLNUMENTER:
				case KEY_CTRLENTER:
					FindPartName(strName,TRUE,1,1);
					FindEdit.Show();
					FastFindShow(FindX,FindY);
					break;
				case KEY_CTRLSHIFTNUMENTER:
				case KEY_CTRLSHIFTENTER:
					FindPartName(strName,TRUE,-1,1);
					FindEdit.Show();
					FastFindShow(FindX,FindY);
					break;
				case KEY_NONE:
				case KEY_IDLE:
					break;
				default:

					if ((Key<32 || Key>=65536) && Key!=KEY_BS && Key!=KEY_CTRLY &&
					        Key!=KEY_CTRLBS && Key!=KEY_ALT && Key!=KEY_SHIFT &&
					        Key!=KEY_CTRL && Key!=KEY_RALT && Key!=KEY_RCTRL &&
					        !(Key==KEY_CTRLINS||Key==KEY_CTRLNUMPAD0) && !(Key==KEY_SHIFTINS||Key==KEY_SHIFTNUMPAD0))
					{
						KeyToProcess=Key;
						break;
					}

					if (FindEdit.ProcessKey(Key))
					{
						FindEdit.GetString(strName);

						// ������ ������� '**'
						if (strName.GetLength() > 1
						        && strName.At(strName.GetLength()-1) == L'*'
						        && strName.At(strName.GetLength()-2) == L'*')
						{
							strName.SetLength(strName.GetLength()-1);
							FindEdit.SetString(strName);
						}

						/* $ 09.04.2001 SVS
						   �������� � ������� �������.
						   ��������� � 00573.ChangeDirCrash.txt
						*/
						if (strName.At(0) == L'"')
						{
							strName.LShift(1);
							FindEdit.SetString(strName);
						}

						if (FindPartName(strName,FALSE,1,1))
						{
							strLastName = strName;
						}
						else
						{
							if (CtrlObject->Macro.IsExecuting())// && CtrlObject->Macro.GetLevelState() > 0) // ���� ������� ��������...
							{
								//CtrlObject->Macro.DropProcess(); // ... �� ������� ������������
								//CtrlObject->Macro.PopState();
								;
							}

							FindEdit.SetString(strLastName);
							strName = strLastName;
						}

						FindEdit.Show();
						FastFindShow(FindX,FindY);
					}

					break;
			}

			FirstKey=0;
		}
	}
	WaitInFastFind--;
	Show();
	CtrlObject->MainKeyBar->Redraw();
	ScrBuf.Flush();
	Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;

	if ((KeyToProcess==KEY_ENTER||KeyToProcess==KEY_NUMENTER) && ActivePanel->GetType()==TREE_PANEL)
		((TreeList *)ActivePanel)->ProcessEnter();
	else
		CtrlObject->Cp()->ProcessKey(KeyToProcess);
}


void Panel::FastFindShow(int FindX,int FindY)
{
	SetColor(COL_DIALOGTEXT);
	GotoXY(FindX+1,FindY+1);
	Text(L" ");
	GotoXY(FindX+20,FindY+1);
	Text(L" ");
	Box(FindX,FindY,FindX+21,FindY+2,COL_DIALOGBOX,DOUBLE_BOX);
	GotoXY(FindX+7,FindY);
	SetColor(COL_DIALOGBOXTITLE);
	Text(MSearchFileTitle);
}


void Panel::SetFocus()
{
	if (CtrlObject->Cp()->ActivePanel!=this)
	{
		CtrlObject->Cp()->ActivePanel->KillFocus();
		CtrlObject->Cp()->ActivePanel=this;
	}

	ProcessPluginEvent(FE_GOTFOCUS,nullptr);

	if (!GetFocus())
	{
		CtrlObject->Cp()->RedrawKeyBar();
		Focus=TRUE;
		Redraw();
		FarChDir(strCurDir);
	}
}


void Panel::KillFocus()
{
	Focus=FALSE;
	ProcessPluginEvent(FE_KILLFOCUS,nullptr);
	Redraw();
}


int  Panel::PanelProcessMouse(MOUSE_EVENT_RECORD *MouseEvent,int &RetCode)
{
	RetCode=TRUE;

	if (!ModalMode && !MouseEvent->dwMousePosition.Y)
	{
		if (MouseEvent->dwMousePosition.X==ScrX)
		{
			if (Opt.ScreenSaver && !(MouseEvent->dwButtonState & 3))
			{
				EndDrag();
				ScreenSaver(TRUE);
				return TRUE;
			}
		}
		else
		{
			if ((MouseEvent->dwButtonState & 3) && !MouseEvent->dwEventFlags)
			{
				EndDrag();

				if (!MouseEvent->dwMousePosition.X)
					CtrlObject->Cp()->ProcessKey(KEY_CTRLO);
				else
					ShellOptions(0,MouseEvent);

				return TRUE;
			}
		}
	}

	if (!IsVisible() ||
	        (MouseEvent->dwMousePosition.X<X1 || MouseEvent->dwMousePosition.X>X2 ||
	         MouseEvent->dwMousePosition.Y<Y1 || MouseEvent->dwMousePosition.Y>Y2))
	{
		RetCode=FALSE;
		return TRUE;
	}

	if (DragX!=-1)
	{
		if (!(MouseEvent->dwButtonState & 3))
		{
			EndDrag();

			if (!MouseEvent->dwEventFlags && SrcDragPanel!=this)
			{
				MoveToMouse(MouseEvent);
				Redraw();
				SrcDragPanel->ProcessKey(DragMove ? KEY_DRAGMOVE:KEY_DRAGCOPY);
			}

			return TRUE;
		}

		if (MouseEvent->dwMousePosition.Y<=Y1 || MouseEvent->dwMousePosition.Y>=Y2 ||
		        !CtrlObject->Cp()->GetAnotherPanel(SrcDragPanel)->IsVisible())
		{
			EndDrag();
			return TRUE;
		}

		if ((MouseEvent->dwButtonState & 2) && !MouseEvent->dwEventFlags)
			DragMove=!DragMove;

		if (MouseEvent->dwButtonState & 1)
		{
			if ((abs(MouseEvent->dwMousePosition.X-DragX)>15 || SrcDragPanel!=this) &&
			        !ModalMode)
			{
				if (SrcDragPanel->GetSelCount()==1 && !DragSaveScr)
				{
					SrcDragPanel->GoToFile(strDragName);
					SrcDragPanel->Show();
				}

				DragMessage(MouseEvent->dwMousePosition.X,MouseEvent->dwMousePosition.Y,DragMove);
				return TRUE;
			}
			else
			{
				delete DragSaveScr;
				DragSaveScr=nullptr;
			}
		}
	}

	if (!(MouseEvent->dwButtonState & 3))
		return TRUE;

	if ((MouseEvent->dwButtonState & 1) && !MouseEvent->dwEventFlags &&
	        X2-X1<ScrX)
	{
		DWORD FileAttr;
		MoveToMouse(MouseEvent);
		GetSelName(nullptr,FileAttr);

		if (GetSelName(&strDragName,FileAttr) && !TestParentFolderName(strDragName))
		{
			SrcDragPanel=this;
			DragX=MouseEvent->dwMousePosition.X;
			DragY=MouseEvent->dwMousePosition.Y;
			DragMove=IntKeyState.ShiftPressed;
		}
	}

	return FALSE;
}


int  Panel::IsDragging()
{
	return DragSaveScr!=nullptr;
}


void Panel::EndDrag()
{
	delete DragSaveScr;
	DragSaveScr=nullptr;
	DragX=DragY=-1;
}


void Panel::DragMessage(int X,int Y,int Move)
{
	string strDragMsg, strSelName;
	int SelCount,MsgX,Length;

	if (!(SelCount=SrcDragPanel->GetSelCount()))
		return;

	if (SelCount==1)
	{
		string strCvtName;
		DWORD FileAttr;
		SrcDragPanel->GetSelName(nullptr,FileAttr);
		SrcDragPanel->GetSelName(&strSelName,FileAttr);
		strCvtName = PointToName(strSelName);
		QuoteSpace(strCvtName);
		strSelName = strCvtName;
	}
	else
		strSelName.Format(MSG(MDragFiles), SelCount);

	if (Move)
		strDragMsg.Format(MSG(MDragMove), strSelName.CPtr());
	else
		strDragMsg.Format(MSG(MDragCopy), strSelName.CPtr());

	if ((Length=(int)strDragMsg.GetLength())+X>ScrX)
	{
		MsgX=ScrX-Length;

		if (MsgX<0)
		{
			MsgX=0;
			TruncStrFromEnd(strDragMsg,ScrX);
			Length=(int)strDragMsg.GetLength();
		}
	}
	else
		MsgX=X;

	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
	delete DragSaveScr;
	DragSaveScr=new SaveScreen(MsgX,Y,MsgX+Length-1,Y);
	GotoXY(MsgX,Y);
	SetColor(COL_PANELDRAGTEXT);
	Text(strDragMsg);
}



int Panel::GetCurDir(string &strCurDir)
{
	strCurDir = Panel::strCurDir; // TODO: ������!!!
	return (int)strCurDir.GetLength();
}



BOOL Panel::SetCurDir(const wchar_t *CurDir,int ClosePlugin)
{
	if (StrCmpI(strCurDir,CurDir) || !TestCurrentDirectory(CurDir))
	{
		strCurDir = CurDir;

		if (PanelMode!=PLUGIN_PANEL)
			PrepareDiskPath(strCurDir);
	}

	return TRUE;
}


void Panel::InitCurDir(const wchar_t *CurDir)
{
	if (StrCmpI(strCurDir,CurDir) || !TestCurrentDirectory(CurDir))
	{
		strCurDir = CurDir;

		if (PanelMode!=PLUGIN_PANEL)
			PrepareDiskPath(strCurDir);
	}
}


/* $ 14.06.2001 KM
   + ��������� ��������� ���������� ���������, ������������
     ������� ���������� ������ ��� ��� ��������, ��� � ���
     ��������� ������. ��� ���������� ���������� �����������
     �� FAR.
*/
/* $ 05.10.2001 SVS
   ! ������� ��� ������ �������� ������ �������� ��� ��������� ������,
     � �� �����...
     � �� ����� �����-�� ����������...
*/
/* $ 14.01.2002 IS
   ! ����� ��������� ���������� ���������, ������ ��� ��� ������������
     � FarChDir, ������� ������ ������������ � ��� ��� ������������
     �������� ��������.
*/
int Panel::SetCurPath()
{
	if (GetMode()==PLUGIN_PANEL)
		return TRUE;

	Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);

	if (AnotherPanel->GetType()!=PLUGIN_PANEL)
	{
		if (IsAlpha(AnotherPanel->strCurDir.At(0)) && AnotherPanel->strCurDir.At(1)==L':' &&
		        Upper(AnotherPanel->strCurDir.At(0))!=Upper(strCurDir.At(0)))
		{
			// ������� ��������� ���������� ��������� ��� ��������� ������
			// (��� �������� ����� ����, ����� ������ ��� ��������� �������
			// �� ������������)
			FarChDir(AnotherPanel->strCurDir,FALSE);
		}
	}

	if (!FarChDir(strCurDir))
	{
		// ����� �� ����� :-)
#if 1

		while (!FarChDir(strCurDir))
		{
			string strRoot;
			GetPathRoot(strCurDir, strRoot);

			if (FAR_GetDriveType(strRoot) != DRIVE_REMOVABLE || apiIsDiskInDrive(strRoot))
			{
				int Result=TestFolder(strCurDir);

				if (Result == TSTFLD_NOTFOUND)
				{
					if (CheckShortcutFolder(&strCurDir,FALSE,TRUE) && FarChDir(strCurDir))
					{
						SetCurDir(strCurDir,TRUE);
						return TRUE;
					}
				}
				else
					break;
			}

			if (FrameManager && FrameManager->ManagerStarted()) // ������� �������� - � ������� �� ��������
			{
				SetCurDir(g_strFarPath,TRUE);                    // ���� ������� - �������� ���� ������� �� ����� ����� ��� ����������
				ChangeDisk();                                    // � ������� ���� ������ ������
			}
			else                                               // ����...
			{
				string strTemp=strCurDir;
				CutToFolderNameIfFolder(strCurDir);             // ���������� �����, ��� ��������� ������ ChDir

				if (strTemp.GetLength()==strCurDir.GetLength())  // ����� �������� - ������ ���� ����������
				{
					SetCurDir(g_strFarPath,TRUE);                 // ����� ������ ��������� � �������, ������ ��������� FAR.
					break;
				}
				else
				{
					if (FarChDir(strCurDir))
					{
						SetCurDir(strCurDir,TRUE);
						break;
					}
				}
			}
		}

#else

		do
		{
			BOOL IsChangeDisk=FALSE;
			char Root[1024];
			GetPathRoot(CurDir,Root);

			if (FAR_GetDriveType(Root) == DRIVE_REMOVABLE && !apiIsDiskInDrive(Root))
				IsChangeDisk=TRUE;
			else if (TestFolder(CurDir) == TSTFLD_NOTACCESS)
			{
				if (FarChDir(Root))
					SetCurDir(Root,TRUE);
				else
					IsChangeDisk=TRUE;
			}

			if (IsChangeDisk)
				ChangeDisk();
		}
		while (!FarChDir(CurDir));

#endif
		return FALSE;
	}

	return TRUE;
}


void Panel::Hide()
{
	ScreenObject::Hide();
	Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);

	if (AnotherPanel->IsVisible())
	{
		if (AnotherPanel->GetFocus())
			if ((AnotherPanel->GetType()==FILE_PANEL && AnotherPanel->IsFullScreen()) ||
			        (GetType()==FILE_PANEL && IsFullScreen()))
				AnotherPanel->Show();
	}
}


void Panel::Show()
{
	if (Locked())
		return;

	/* $ 03.10.2001 IS ���������� ������� ���� */
	if (Opt.ShowMenuBar)
		CtrlObject->TopMenuBar->Show();

	/* $ 09.05.2001 OT */
//  SavePrevScreen();
	Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);

	if (AnotherPanel->IsVisible() && !GetModalMode())
	{
		if (SaveScr)
		{
			SaveScr->AppendArea(AnotherPanel->SaveScr);
		}

		if (AnotherPanel->GetFocus())
		{
			if (AnotherPanel->IsFullScreen())
			{
				SetVisible(TRUE);
				return;
			}

			if (GetType()==FILE_PANEL && IsFullScreen())
			{
				ScreenObject::Show();
				AnotherPanel->Show();
				return;
			}
		}
	}

	ScreenObject::Show();
	ShowScreensCount();
}


void Panel::DrawSeparator(int Y)
{
	if (Y<Y2)
	{
		SetColor(COL_PANELBOX);
		GotoXY(X1,Y);
		ShowSeparator(X2-X1+1,1);
	}
}


void Panel::ShowScreensCount()
{
	if (Opt.ShowScreensNumber && !X1)
	{
		int Viewers=FrameManager->GetFrameCountByType(MODALTYPE_VIEWER);
		int Editors=FrameManager->GetFrameCountByType(MODALTYPE_EDITOR);
		int Dialogs=FrameManager->GetFrameCountByType(MODALTYPE_DIALOG);

		if (Viewers>0 || Editors>0 || Dialogs > 0)
		{
			string strScreensText;
			string strAdd;
			strScreensText.Format(L"[%d", Viewers);

			if (Editors > 0)
			{
				strAdd.Format(L"+%d", Editors);
				strScreensText += strAdd;
			}

			if (Dialogs > 0)
			{
				strAdd.Format(L",%d", Dialogs);
				strScreensText += strAdd;
			}

			strScreensText += L"]";
			GotoXY(Opt.ShowColumnTitles ? X1:X1+2,Y1);
			SetColor(COL_PANELSCREENSNUMBER);
			Text(strScreensText);
		}
	}
}


void Panel::SetTitle()
{
	if (GetFocus())
	{
		string strTitleDir(L"{");

		if (!strCurDir.IsEmpty())
		{
			strTitleDir += strCurDir;
		}
		else
		{
			string strCmdText;
			CtrlObject->CmdLine->GetCurDir(strCmdText);
			strTitleDir += strCmdText;
		}

		strTitleDir += L"}";

		ConsoleTitle::SetFarTitle(strTitleDir);
	}
}

string &Panel::GetTitle(string &strTitle,int SubLen,int TruncSize)
{
	string strTitleDir;
	bool truncTitle = (SubLen==-1 || TruncSize==0)?false:true;

	if (PanelMode==PLUGIN_PANEL)
	{
		OpenPluginInfo Info;
		GetOpenPluginInfo(&Info);
		strTitleDir = Info.PanelTitle;
		RemoveExternalSpaces(strTitleDir);
		if (truncTitle)
			TruncStr(strTitleDir,SubLen-TruncSize);
	}
	else
	{
		if (ShowShortNames)
			ConvertNameToShort(strCurDir,strTitleDir);
		else
			strTitleDir = strCurDir;

		if (truncTitle)
			TruncPathStr(strTitleDir,SubLen-TruncSize);
	}

	strTitle = L" "+strTitleDir+L" ";
	return strTitle;
}

int Panel::SetPluginCommand(int Command,int Param1,LONG_PTR Param2)
{
	_ALGO(CleverSysLog clv(L"Panel::SetPluginCommand"));
	_ALGO(SysLog(L"(Command=%s, Param1=[%d/0x%08X], Param2=[%d/0x%08X])",_FCTL_ToName(Command),(int)Param1,Param1,(int)Param2,Param2));
	int Result=FALSE;
	ProcessingPluginCommand++;
	FilePanels *FPanels=CtrlObject->Cp();
	PluginCommand=Command;

	switch (Command)
	{
		case FCTL_SETVIEWMODE:
			Result=FPanels->ChangePanelViewMode(this,Param1,FPanels->IsTopFrame());
			break;

		case FCTL_SETSORTMODE:
		{
			int Mode=Param1;

			if ((Mode>SM_DEFAULT) && (Mode<=SM_CHTIME))
			{
				SetSortMode(--Mode); // �������� �� 1 ��-�� SM_DEFAULT
				Result=TRUE;
			}
			break;
		}

		case FCTL_SETNUMERICSORT:
		{
			ChangeNumericSort(Param1);
			Result=TRUE;
			break;
		}

		case FCTL_SETCASESENSITIVESORT:
		{
			ChangeCaseSensitiveSort(Param1);
			Result=TRUE;
			break;
		}

		case FCTL_SETSORTORDER:
		{
			ChangeSortOrder(Param1?-1:1);
			Result=TRUE;
			break;
		}

		case FCTL_SETDIRECTORIESFIRST:
		{
			ChangeDirectoriesFirst(Param1);
			Result=TRUE;
			break;
		}

		case FCTL_CLOSEPLUGIN:
			strPluginParam = (const wchar_t *)Param2;
			Result=TRUE;
			//if(Opt.CPAJHefuayor)
			//  CtrlObject->Plugins.ProcessCommandLine((char *)PluginParam);
			break;

		case FCTL_GETPANELINFO:
		{
			if (!Param2)
				break;

			PanelInfo *Info=(PanelInfo *)Param2;
			memset(Info,0,sizeof(*Info));
			UpdateIfRequired();

			switch (GetType())
			{
				case FILE_PANEL:
					Info->PanelType=PTYPE_FILEPANEL;
					break;
				case TREE_PANEL:
					Info->PanelType=PTYPE_TREEPANEL;
					break;
				case QVIEW_PANEL:
					Info->PanelType=PTYPE_QVIEWPANEL;
					break;
				case INFO_PANEL:
					Info->PanelType=PTYPE_INFOPANEL;
					break;
			}

			Info->Plugin=(GetMode()==PLUGIN_PANEL);
			int X1,Y1,X2,Y2;
			GetPosition(X1,Y1,X2,Y2);
			Info->PanelRect.left=X1;
			Info->PanelRect.top=Y1;
			Info->PanelRect.right=X2;
			Info->PanelRect.bottom=Y2;
			Info->Visible=IsVisible();
			Info->Focus=GetFocus();
			Info->ViewMode=GetViewMode();
			Info->SortMode=SM_UNSORTED-UNSORTED+GetSortMode();
			{
				static struct
				{
					int *Opt;
					DWORD Flags;
				} PFLAGS[]=
				{
					{&Opt.ShowHidden,PFLAGS_SHOWHIDDEN},
					{&Opt.Highlight,PFLAGS_HIGHLIGHT},
				};
				DWORD Flags=0;

				for (size_t I=0; I < ARRAYSIZE(PFLAGS); ++I)
					if (*(PFLAGS[I].Opt) )
						Flags|=PFLAGS[I].Flags;

				Flags|=GetSortOrder()<0?PFLAGS_REVERSESORTORDER:0;
				Flags|=GetSortGroups()?PFLAGS_USESORTGROUPS:0;
				Flags|=GetSelectedFirstMode()?PFLAGS_SELECTEDFIRST:0;
				Flags|=GetDirectoriesFirst()?PFLAGS_DIRECTORIESFIRST:0;
				Flags|=GetNumericSort()?PFLAGS_NUMERICSORT:0;
				Flags|=GetCaseSensitiveSort()?PFLAGS_CASESENSITIVESORT:0;

				if (CtrlObject->Cp()->LeftPanel == this)
					Flags|=PFLAGS_PANELLEFT;

				Info->Flags=Flags;
			}

			if (GetType()==FILE_PANEL)
			{
				FileList *DestFilePanel=(FileList *)this;
				static int Reenter=0;

				if (!Reenter && Info->Plugin)
				{
					Reenter++;
					OpenPluginInfo PInfo;
					DestFilePanel->GetOpenPluginInfo(&PInfo);

					if (PInfo.Flags & OPIF_REALNAMES)
						Info->Flags |= PFLAGS_REALNAMES;

					if (!(PInfo.Flags & OPIF_USEHIGHLIGHTING))
						Info->Flags &= ~PFLAGS_HIGHLIGHT;

					if (PInfo.Flags & OPIF_USECRC32)
						Info->Flags |= PFLAGS_USECRC32;

					Reenter--;
				}

				DestFilePanel->PluginGetPanelInfo(*Info);
			}

			if (!Info->Plugin) // $ 12.12.2001 DJ - �� ������������ ������ - ������ �������� �����
				Info->Flags |= PFLAGS_REALNAMES;

			Result=TRUE;
			break;
		}

		case FCTL_GETPANELHOSTFILE:
		case FCTL_GETPANELFORMAT:
		case FCTL_GETPANELDIR:
		{
			string strTemp;

			if (Command == FCTL_GETPANELDIR)
				GetCurDir(strTemp);

			if (GetType()==FILE_PANEL)
			{
				FileList *DestFilePanel=(FileList *)this;
				static int Reenter=0;

				if (!Reenter && GetMode()==PLUGIN_PANEL)
				{
					Reenter++;

					OpenPluginInfo PInfo;
					DestFilePanel->GetOpenPluginInfo(&PInfo);

					switch (Command)
					{
						case FCTL_GETPANELHOSTFILE:
							strTemp=PInfo.HostFile;
							break;
						case FCTL_GETPANELFORMAT:
							strTemp=PInfo.Format;
							break;
						case FCTL_GETPANELDIR:
							strTemp=PInfo.CurDir;
							break;
					}

					Reenter--;
				}
			}

			if (Param1&&Param2)
				xwcsncpy((wchar_t*)Param2,strTemp,Param1);

			Result=(int)strTemp.GetLength()+1;
			break;
		}

		case FCTL_GETCOLUMNTYPES:
		case FCTL_GETCOLUMNWIDTHS:

			if (GetType()==FILE_PANEL)
			{
				string strColumnTypes,strColumnWidths;
				((FileList *)this)->PluginGetColumnTypesAndWidths(strColumnTypes,strColumnWidths);

				if (Command==FCTL_GETCOLUMNTYPES)
				{
					if (Param1&&Param2)
						xwcsncpy((wchar_t*)Param2,strColumnTypes,Param1);

					Result=(int)strColumnTypes.GetLength()+1;
				}
				else
				{
					if (Param1&&Param2)
						xwcsncpy((wchar_t*)Param2,strColumnWidths,Param1);

					Result=(int)strColumnWidths.GetLength()+1;
				}
			}
			break;

		case FCTL_GETPANELITEM:
		{
			Result=(int)((FileList*)this)->PluginGetPanelItem(Param1,(PluginPanelItem*)Param2);
			break;
		}

		case FCTL_GETSELECTEDPANELITEM:
		{
			Result=(int)((FileList*)this)->PluginGetSelectedPanelItem(Param1,(PluginPanelItem*)Param2);
			break;
		}

		case FCTL_GETCURRENTPANELITEM:
		{
			PanelInfo Info;
			FileList *DestPanel = ((FileList*)this);
			DestPanel->PluginGetPanelInfo(Info);
			Result = (int)DestPanel->PluginGetPanelItem(Info.CurrentItem,(PluginPanelItem*)Param2);
			break;
		}

		case FCTL_BEGINSELECTION:
		{
			if (GetType()==FILE_PANEL)
			{
				((FileList *)this)->PluginBeginSelection();
				Result=TRUE;
			}
			break;
		}

		case FCTL_SETSELECTION:
		{
			if (GetType()==FILE_PANEL)
			{
				((FileList *)this)->PluginSetSelection(Param1,Param2?true:false);
				Result=TRUE;
			}
			break;
		}

		case FCTL_CLEARSELECTION:
		{
			if (GetType()==FILE_PANEL)
			{
				static_cast<FileList*>(this)->PluginClearSelection(Param1);
				Result=TRUE;
			}
			break;
		}

		case FCTL_ENDSELECTION:
		{
			if (GetType()==FILE_PANEL)
			{
				((FileList *)this)->PluginEndSelection();
				Result=TRUE;
			}
			break;
		}

		case FCTL_UPDATEPANEL:
			Update(Param1?UPDATE_KEEP_SELECTION:0);

			if (GetType() == QVIEW_PANEL)
				UpdateViewPanel();

			Result=TRUE;
			break;

		case FCTL_REDRAWPANEL:
		{
			PanelRedrawInfo *Info=(PanelRedrawInfo *)Param2;

			if (Info)
			{
				CurFile=Info->CurrentItem;
				CurTopFile=Info->TopPanelItem;
			}

			// $ 12.05.2001 DJ ���������������� ������ � ��� ������, ���� �� - ������� �����
			if (FPanels->IsTopFrame())
				Redraw();

			Result=TRUE;
			break;
		}

		case FCTL_SETPANELDIR:
		{
			if (Param2)
			{
				Result = SetCurDir((const wchar_t *)Param2,TRUE);
				// restore current directory to active panel path
				Panel* ActivePanel = CtrlObject->Cp()->ActivePanel;
				if (Result && this != ActivePanel)
				{
					ActivePanel->SetCurPath();
				}
			}
			break;
		}

	}

	ProcessingPluginCommand--;
	return Result;
}


int Panel::GetCurName(string &strName, string &strShortName)
{
	strName.Clear();
	strShortName.Clear();
	return FALSE;
}


int Panel::GetCurBaseName(string &strName, string &strShortName)
{
	strName.Clear();
	strShortName.Clear();
	return FALSE;
}

static int MessageRemoveConnection(wchar_t Letter, int &UpdateProfile)
{
	int Len1, Len2, Len3,Len4;
	BOOL IsPersistent;
	string strMsgText;
	/*
	  0         1         2         3         4         5         6         7
	  0123456789012345678901234567890123456789012345678901234567890123456789012345
	0
	1   +-------- ���������� �������� ���������� --------+
	2   | �� ������ ������� ���������� � ����������� C:? |
	3   | �� ���������� %c: ��������� �������            |
	4   | \\host\share                                   |
	6   +------------------------------------------------+
	7   | [ ] ��������������� ��� ����� � �������        |
	8   +------------------------------------------------+
	9   |              [ �� ]   [ ������ ]               |
	10  +------------------------------------------------+
	11
	*/
	DialogDataEx DCDlgData[]=
	{
		DI_DOUBLEBOX, 3, 1, 72, 9, 0, 0,                L"",
		DI_TEXT,      5, 2,  0, 2, 0, DIF_SHOWAMPERSAND,L"",
		DI_TEXT,      5, 3,  0, 3, 0, DIF_SHOWAMPERSAND,L"",
		DI_TEXT,      5, 4,  0, 4, 0, DIF_SHOWAMPERSAND,L"",
		DI_TEXT,      0, 5,  0, 5, 0, DIF_SEPARATOR,    L"",
		DI_CHECKBOX,  5, 6, 70, 6, 0, 0,                L"",
		DI_TEXT,      0, 7,  0, 7, 0, DIF_SEPARATOR,    L"",
		DI_BUTTON,    0, 8,  0, 8, 0, DIF_FOCUS|DIF_DEFAULT|DIF_CENTERGROUP,  L"",
		DI_BUTTON,    0, 8,  0, 8, 0, DIF_CENTERGROUP,  L""
	};
	MakeDialogItemsEx(DCDlgData,DCDlg);
	DCDlg[0].strData = MSG(MChangeDriveDisconnectTitle);
	Len1 = (int)DCDlg[0].strData.GetLength();
	strMsgText.Format(MSG(MChangeDriveDisconnectQuestion),Letter);
	DCDlg[1].strData = strMsgText;
	Len2 = (int)DCDlg[1].strData.GetLength();
	strMsgText.Format(MSG(MChangeDriveDisconnectMapped),Letter);
	DCDlg[2].strData = strMsgText;
	Len4 = (int)DCDlg[2].strData.GetLength();
	DCDlg[5].strData = MSG(MChangeDriveDisconnectReconnect);
	Len3 = (int)DCDlg[5].strData.GetLength();
	Len1=Max(Len1,Max(Len2,Max(Len3,Len4)));
	DCDlg[3].strData = TruncPathStr(DriveLocalToRemoteName(DRIVE_REMOTE,Letter,strMsgText),Len1);
	DCDlg[7].strData = MSG(MYes);
	DCDlg[8].strData = MSG(MCancel);
	// ��������� - ��� ���� ���������� �������� ��� ���?
	// ���� ����� � ������� HKCU\Network\���������� ���� - ���
	//   ���� ���������� �����������.
	{
		HKEY hKey;
		IsPersistent=TRUE;
		const wchar_t KeyName[]={L'N',L'e',L't',L'w',L'o',L'r',L'k',L'\\',Letter,L'\0'};

		if (RegOpenKeyEx(HKEY_CURRENT_USER,KeyName,0,KEY_QUERY_VALUE,&hKey)!=ERROR_SUCCESS)
		{
			DCDlg[5].Flags|=DIF_DISABLE;
			DCDlg[5].Selected=0;
			IsPersistent=FALSE;
			RegCloseKey(hKey);
		}
		else
			DCDlg[5].Selected=Opt.ChangeDriveDisconnetMode;
	}
	// ������������� ������� ������� - ��� �������
	DCDlg[0].X2=DCDlg[0].X1+Len1+3;
	int ExitCode=7;

	if (Opt.Confirm.RemoveConnection)
	{
		Dialog Dlg(DCDlg,ARRAYSIZE(DCDlg));
		Dlg.SetPosition(-1,-1,DCDlg[0].X2+4,11);
		Dlg.SetHelp(L"DisconnectDrive");
		Dlg.SetDialogMode(DMODE_WARNINGSTYLE);
		Dlg.Process();
		ExitCode=Dlg.GetExitCode();
	}

	UpdateProfile=DCDlg[5].Selected?0:CONNECT_UPDATE_PROFILE;

	if (IsPersistent)
		Opt.ChangeDriveDisconnetMode=DCDlg[5].Selected;

	return ExitCode == 7;
}

BOOL Panel::NeedUpdatePanel(Panel *AnotherPanel)
{
	/* ��������, ���� ���������� ��������� � ���� ��������� */
	if ((!Opt.AutoUpdateLimit || static_cast<DWORD>(GetFileCount()) <= Opt.AutoUpdateLimit) &&
	        !StrCmpI(AnotherPanel->strCurDir,strCurDir))
		return TRUE;

	return FALSE;
}


bool Panel::SaveShortcutFolder(int Pos)
{
	string strShortcutFolder,strPluginModule,strPluginFile,strPluginData;

	if (PanelMode==PLUGIN_PANEL)
	{
		HANDLE hPlugin=GetPluginHandle();
		PluginHandle *ph = (PluginHandle*)hPlugin;
		strPluginModule = ph->pPlugin->GetModuleName();
		OpenPluginInfo Info;
		CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);
		strPluginFile = Info.HostFile;
		strShortcutFolder = Info.CurDir;
		strPluginData = Info.ShortcutData;
	}
	else
	{
		strPluginModule.Clear();
		strPluginFile.Clear();
		strPluginData.Clear();
		strShortcutFolder = strCurDir;
	}

	if (SaveFolderShortcut(Pos,&strShortcutFolder,&strPluginModule,&strPluginFile,&strPluginData))
		return true;

	return true;
}

/*
int Panel::ProcessShortcutFolder(int Key,BOOL ProcTreePanel)
{
	string strShortcutFolder, strPluginModule, strPluginFile, strPluginData;

	if (GetShortcutFolder(Key-KEY_RCTRL0,&strShortcutFolder,&strPluginModule,&strPluginFile,&strPluginData))
	{
		Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);

		if (ProcTreePanel)
		{
			if (AnotherPanel->GetType()==FILE_PANEL)
			{
				AnotherPanel->SetCurDir(strShortcutFolder,TRUE);
				AnotherPanel->Redraw();
			}
			else
			{
				SetCurDir(strShortcutFolder,TRUE);
				ProcessKey(KEY_ENTER);
			}
		}
		else
		{
			if (AnotherPanel->GetType()==FILE_PANEL && !strPluginModule.IsEmpty())
			{
				AnotherPanel->SetCurDir(strShortcutFolder,TRUE);
				AnotherPanel->Redraw();
			}
		}

		return TRUE;
	}

	return FALSE;
}
*/

bool Panel::ExecShortcutFolder(int Pos)
{
	string strShortcutFolder,strPluginModule,strPluginFile,strPluginData;

	if (GetShortcutFolder(Pos,&strShortcutFolder,&strPluginModule,&strPluginFile,&strPluginData))
	{
		Panel *SrcPanel=this;
		Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);

		switch (GetType())
		{
			case TREE_PANEL:
				if (AnotherPanel->GetType()==FILE_PANEL)
					SrcPanel=AnotherPanel;
				break;

			case QVIEW_PANEL:
			case INFO_PANEL:
			{
				if (AnotherPanel->GetType()==FILE_PANEL)
					SrcPanel=AnotherPanel;
				break;
			}
		}

		int CheckFullScreen=SrcPanel->IsFullScreen();

		if (!strPluginModule.IsEmpty())
		{
			if (!strPluginFile.IsEmpty())
			{
				switch (CheckShortcutFolder(&strPluginFile,TRUE))
				{
					case 0:
						//              return FALSE;
					case -1:
						return true;
				}

				/* ������������ ������� BugZ#50 */
				string strRealDir;
				strRealDir = strPluginFile;

				if (CutToSlash(strRealDir))
				{
					SrcPanel->SetCurDir(strRealDir,TRUE);
					SrcPanel->GoToFile(PointToName(strPluginFile));

					SrcPanel->ClearAllItem();
				}

				if (SrcPanel->GetType() == FILE_PANEL)
					((FileList*)SrcPanel)->OpenFilePlugin(strPluginFile,FALSE, OFP_SHORTCUT); //???

				if (!strShortcutFolder.IsEmpty())
						SrcPanel->SetCurDir(strShortcutFolder,FALSE);

				SrcPanel->Show();
			}
			else
			{
				switch (CheckShortcutFolder(nullptr,TRUE))
				{
					case 0:
						//              return FALSE;
					case -1:
						return true;
				}

				for (int I=0; I<CtrlObject->Plugins.GetPluginsCount(); I++)
				{
					Plugin *pPlugin = CtrlObject->Plugins.GetPlugin(I);

					if (!StrCmpI(pPlugin->GetModuleName(),strPluginModule))
					{
						if (pPlugin->HasOpenPlugin())
						{
							HANDLE hNewPlugin=CtrlObject->Plugins.OpenPlugin(pPlugin,OPEN_SHORTCUT,(INT_PTR)strPluginData.CPtr());

							if (hNewPlugin!=INVALID_HANDLE_VALUE)
							{
								int CurFocus=SrcPanel->GetFocus();

								Panel *NewPanel=CtrlObject->Cp()->ChangePanel(SrcPanel,FILE_PANEL,TRUE,TRUE);
								NewPanel->SetPluginMode(hNewPlugin,L"",CurFocus || !CtrlObject->Cp()->GetAnotherPanel(NewPanel)->IsVisible());

								if (!strShortcutFolder.IsEmpty())
									CtrlObject->Plugins.SetDirectory(hNewPlugin,strShortcutFolder,0);

								NewPanel->Update(0);
								NewPanel->Show();
							}
						}

						break;
					}
				}

				/*
				if(I == CtrlObject->Plugins.PluginsCount)
				{
				  char Target[NM*2];
				  xstrncpy(Target, PluginModule, sizeof(Target));
				  TruncPathStr(Target, ScrX-16);
				  Message (MSG_WARNING | MSG_ERRORTYPE, 1, MSG(MError), Target, MSG (MNeedNearPath), MSG(MOk))
				}
				*/
			}

			return true;
		}

		switch (CheckShortcutFolder(&strShortcutFolder,FALSE))
		{
			case 0:
				//          return FALSE;
			case -1:
				return true;
		}

        /*
		if (SrcPanel->GetType()!=FILE_PANEL)
		{
			SrcPanel=CtrlObject->Cp()->ChangePanel(SrcPanel,FILE_PANEL,TRUE,TRUE);
		}
        */

		SrcPanel->SetCurDir(strShortcutFolder,TRUE);

		if (CheckFullScreen!=SrcPanel->IsFullScreen())
			CtrlObject->Cp()->GetAnotherPanel(SrcPanel)->Show();

		SrcPanel->Redraw();
		return true;
	}
	return false;
}
