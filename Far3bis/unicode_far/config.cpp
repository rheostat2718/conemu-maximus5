/*
config.cpp

������������
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

#include "config.hpp"
#include "keys.hpp"
#include "colors.hpp"
#include "cmdline.hpp"
#include "ctrlobj.hpp"
#include "dialog.hpp"
#include "filepanels.hpp"
#include "filelist.hpp"
#include "panel.hpp"
#include "help.hpp"
#include "filefilter.hpp"
#include "poscache.hpp"
#include "findfile.hpp"
#include "hilight.hpp"
#include "interf.hpp"
#include "keyboard.hpp"
#include "palette.hpp"
#include "message.hpp"
#include "stddlg.hpp"
#include "pathmix.hpp"
#include "panelmix.hpp"
#include "strmix.hpp"
#include "udlist.hpp"
#include "FarDlgBuilder.hpp"
#include "elevation.hpp"
#include "configdb.hpp"
#include "FarGuid.hpp"

Options Opt={};

// ����������� ����� ������������
static const wchar_t *WordDiv0 = L"~!%^&*()+|{}:\"<>?`-=\\[];',./";

// ����������� ����� ������������ ��� ������� Xlat
static const wchar_t *WordDivForXlat0=L" \t!#$%^&*()+|=\\/@?";

const wchar_t *constBatchExt=L".BAT;.CMD;";

#if defined(TREEFILE_PROJECT)
const wchar_t *constLocalDiskTemplate=L"%D.%SN.tree";
const wchar_t *constNetDiskTemplate=L"%D.%SN.tree";
const wchar_t *constNetPathTemplate=L"%SR.%SH.tree";
const wchar_t *constRemovableDiskTemplate=L"%SN.tree";
const wchar_t *constCDDiskTemplate=L"CD.%L.%SN.tree";
#endif

string strKeyNameConsoleDetachKey;
static const wchar_t szCtrlDot[]=L"Ctrl.";
static const wchar_t szRCtrlDot[]=L"RCtrl.";
static const wchar_t szCtrlShiftDot[]=L"CtrlShift.";
static const wchar_t szRCtrlShiftDot[]=L"RCtrlShift.";

// KeyName
const wchar_t NKeyColors[]=L"Colors";
const wchar_t NKeyScreen[]=L"Screen";
const wchar_t NKeyCmdline[]=L"Cmdline";
const wchar_t NKeyInterface[]=L"Interface";
const wchar_t NKeyInterfaceCompletion[]=L"Interface.Completion";
const wchar_t NKeyViewer[]=L"Viewer";
const wchar_t NKeyDialog[]=L"Dialog";
const wchar_t NKeyEditor[]=L"Editor";
const wchar_t NKeyXLat[]=L"XLat";
const wchar_t NKeySystem[]=L"System";
const wchar_t NKeySystemException[]=L"System.Exception";
const wchar_t NKeySystemKnownIDs[]=L"System.KnownIDs";
const wchar_t NKeySystemExecutor[]=L"System.Executor";
const wchar_t NKeySystemNowell[]=L"System.Nowell";
const wchar_t NKeyHelp[]=L"Help";
const wchar_t NKeyLanguage[]=L"Language";
const wchar_t NKeyConfirmations[]=L"Confirmations";
const wchar_t NKeyPluginConfirmations[]=L"PluginConfirmations";
const wchar_t NKeyPanel[]=L"Panel";
const wchar_t NKeyPanelLeft[]=L"Panel.Left";
const wchar_t NKeyPanelRight[]=L"Panel.Right";
const wchar_t NKeyPanelLayout[]=L"Panel.Layout";
const wchar_t NKeyPanelTree[]=L"Panel.Tree";
const wchar_t NKeyPanelInfo[]=L"Panel.Info";
const wchar_t NKeyLayout[]=L"Layout";
const wchar_t NKeyDescriptions[]=L"Descriptions";
const wchar_t NKeyKeyMacros[]=L"Macros";
const wchar_t NKeyPolicies[]=L"Policies";
const wchar_t NKeyFileFilter[]=L"OperationsFilter";
const wchar_t NKeyCodePages[]=L"CodePages";
const wchar_t NKeyVMenu[]=L"VMenu";
const wchar_t NKeyCommandHistory[]=L"History.CommandHistory";
const wchar_t NKeyViewEditHistory[]=L"History.ViewEditHistory";
const wchar_t NKeyFolderHistory[]=L"History.FolderHistory";
const wchar_t NKeyDialogHistory[]=L"History.DialogHistory";

const wchar_t NParamHistoryCount[]=L"HistoryCount";

static const WCHAR _BoxSymbols[48+1] =
{
	0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556,
	0x2555, 0x2563, 0x2551, 0x2557, 0x255D, 0x255C, 0x255B, 0x2510,
	0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x255E, 0x255F,
	0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x2567,
	0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256B,
	0x256A, 0x2518, 0x250C, 0x2588, 0x2584, 0x258C, 0x2590, 0x2580,
	0x0000
};


void SystemSettings()
{
	DialogBuilder Builder(MConfigSystemTitle, L"SystemSettings");

	DialogItemEx *DeleteToRecycleBin = Builder.AddCheckbox(MConfigRecycleBin, &Opt.DeleteToRecycleBin);
	DialogItemEx *DeleteLinks = Builder.AddCheckbox(MConfigRecycleBinLink, &Opt.DeleteToRecycleBinKillLink);
	DeleteLinks->Indent(4);
	Builder.LinkFlags(DeleteToRecycleBin, DeleteLinks, DIF_DISABLE);

	Builder.AddCheckbox(MConfigSystemCopy, &Opt.CMOpt.UseSystemCopy);
	Builder.AddCheckbox(MConfigCopySharing, &Opt.CMOpt.CopyOpened);
	Builder.AddCheckbox(MConfigScanJunction, &Opt.ScanJunction);
	Builder.AddCheckbox(MConfigCreateUppercaseFolders, &Opt.CreateUppercaseFolders);

	Builder.AddCheckbox(MConfigSaveHistory, &Opt.SaveHistory);
	Builder.AddCheckbox(MConfigSaveFoldersHistory, &Opt.SaveFoldersHistory);
	Builder.AddCheckbox(MConfigSaveViewHistory, &Opt.SaveViewHistory);
	Builder.AddCheckbox(MConfigRegisteredTypes, &Opt.UseRegisteredTypes);
	Builder.AddCheckbox(MConfigCloseCDGate, &Opt.CloseCDGate);
	Builder.AddCheckbox(MConfigUpdateEnvironment, &Opt.UpdateEnvironment);
	Builder.AddText(MConfigElevation);
	Builder.AddCheckbox(MConfigElevationModify, &Opt.ElevationMode, ELEVATION_MODIFY_REQUEST)->Indent(4);
	Builder.AddCheckbox(MConfigElevationRead, &Opt.ElevationMode, ELEVATION_READ_REQUEST)->Indent(4);
	Builder.AddCheckbox(MConfigElevationUsePrivileges, &Opt.ElevationMode, ELEVATION_USE_PRIVILEGES)->Indent(4);
	Builder.AddCheckbox(MConfigAutoSave, &Opt.AutoSaveSetup);
	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		Opt.CurrentElevationMode = Opt.ElevationMode;
	}
}


void PanelSettings()
{
	DialogBuilder Builder(MConfigPanelTitle, L"PanelSettings");
	BOOL AutoUpdate = Opt.AutoUpdateLimit;

	Builder.AddCheckbox(MConfigHidden, &Opt.ShowHidden);
	Builder.AddCheckbox(MConfigHighlight, &Opt.Highlight);
	Builder.AddCheckbox(MConfigSelectFolders, &Opt.SelectFolders);
	Builder.AddCheckbox(MConfigSortFolderExt, &Opt.SortFolderExt);
	Builder.AddCheckbox(MConfigReverseSort, &Opt.ReverseSort);

	DialogItemEx *AutoUpdateEnabled = Builder.AddCheckbox(MConfigAutoUpdateLimit, &AutoUpdate);
	DialogItemEx *AutoUpdateLimit = Builder.AddIntEditField((int *) &Opt.AutoUpdateLimit, 6);
	Builder.LinkFlags(AutoUpdateEnabled, AutoUpdateLimit, DIF_DISABLE, false);
	DialogItemEx *AutoUpdateText = Builder.AddTextBefore(AutoUpdateLimit, MConfigAutoUpdateLimit2);
	AutoUpdateLimit->Indent(4);
	AutoUpdateText->Indent(4);
	Builder.AddCheckbox(MConfigAutoUpdateRemoteDrive, &Opt.AutoUpdateRemoteDrive);

	Builder.AddSeparator();
	Builder.AddCheckbox(MConfigShowColumns, &Opt.ShowColumnTitles);
	Builder.AddCheckbox(MConfigShowStatus, &Opt.ShowPanelStatus);
	Builder.AddCheckbox(MConfigDetailedJunction, &Opt.PanelDetailedJunction);
	Builder.AddCheckbox(MConfigShowTotal, &Opt.ShowPanelTotals);
	Builder.AddCheckbox(MConfigShowFree, &Opt.ShowPanelFree);
	Builder.AddCheckbox(MConfigShowScrollbar, &Opt.ShowPanelScrollbar);
	Builder.AddCheckbox(MConfigShowScreensNumber, &Opt.ShowScreensNumber);
	Builder.AddCheckbox(MConfigShowSortMode, &Opt.ShowSortMode);
	Builder.AddCheckbox(MConfigShowDotsInRoot, &Opt.ShowDotsInRoot);
	Builder.AddCheckbox(MConfigHighlightColumnSeparator, &Opt.HighlightColumnSeparator);
	Builder.AddCheckbox(MConfigDoubleGlobalColumnSeparator, &Opt.DoubleGlobalColumnSeparator);
	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		if (!AutoUpdate)
			Opt.AutoUpdateLimit = 0;

	//  FrameManager->RefreshFrame();
		CtrlObject->Cp()->LeftPanel->Update(UPDATE_KEEP_SELECTION);
		CtrlObject->Cp()->RightPanel->Update(UPDATE_KEEP_SELECTION);
		CtrlObject->Cp()->Redraw();
	}
}

void TreeSettings()
{
	DialogBuilder Builder(MConfigTreeTitle, L"TreeSettings");

	DialogItemEx *TemplateEdit;

	Builder.AddCheckbox(MConfigTreeAutoChange, &Opt.Tree.AutoChangeFolder);

	TemplateEdit = Builder.AddIntEditField((int *) &Opt.Tree.MinTreeCount, 3);
	Builder.AddTextBefore(TemplateEdit, MConfigTreeLabelMinFolder);

#if defined(TREEFILE_PROJECT)
	DialogItemEx *Checkbox;

	Builder.AddSeparator(MConfigTreeLabel1);

	Checkbox = Builder.AddCheckbox(MConfigTreeLabelLocalDisk, &Opt.Tree.LocalDisk);
	TemplateEdit = Builder.AddEditField(&Opt.Tree.strLocalDisk, 36);
	TemplateEdit->Indent(4);
	Builder.LinkFlags(Checkbox, TemplateEdit, DIF_DISABLE);

	Checkbox = Builder.AddCheckbox(MConfigTreeLabelNetDisk, &Opt.Tree.NetDisk);
	TemplateEdit = Builder.AddEditField(&Opt.Tree.strNetDisk, 36);
	TemplateEdit->Indent(4);
	Builder.LinkFlags(Checkbox, TemplateEdit, DIF_DISABLE);

	Checkbox = Builder.AddCheckbox(MConfigTreeLabelNetPath, &Opt.Tree.NetPath);
	TemplateEdit = Builder.AddEditField(&Opt.Tree.strNetPath, 36);
	TemplateEdit->Indent(4);
	Builder.LinkFlags(Checkbox, TemplateEdit, DIF_DISABLE);

	Checkbox = Builder.AddCheckbox(MConfigTreeLabelRemovableDisk, &Opt.Tree.RemovableDisk);
	TemplateEdit = Builder.AddEditField(&Opt.Tree.strRemovableDisk, 36);
	TemplateEdit->Indent(4);
	Builder.LinkFlags(Checkbox, TemplateEdit, DIF_DISABLE);

	Checkbox = Builder.AddCheckbox(MConfigTreeLabelCDDisk, &Opt.Tree.CDDisk);
	TemplateEdit = Builder.AddEditField(&Opt.Tree.strCDDisk, 36);
	TemplateEdit->Indent(4);
	Builder.LinkFlags(Checkbox, TemplateEdit, DIF_DISABLE);

	Builder.AddText(MConfigTreeLabelSaveLocalPath);
	Builder.AddEditField(&Opt.Tree.strSaveLocalPath, 40);

	Builder.AddText(MConfigTreeLabelSaveNetPath);
	Builder.AddEditField(&Opt.Tree.strSaveNetPath, 40);

	Builder.AddText(MConfigTreeLabelExceptPath);
	Builder.AddEditField(&Opt.Tree.strExceptPath, 40);
#endif

	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		CtrlObject->Cp()->LeftPanel->Update(UPDATE_KEEP_SELECTION);
		CtrlObject->Cp()->RightPanel->Update(UPDATE_KEEP_SELECTION);
		CtrlObject->Cp()->Redraw();
	}
}


/* $ 17.12.2001 IS
   ��������� ������� ������ ���� ��� �������. ������� ���� ����, ����� ����
   ��������� � ����������� ������ �� ���������������� ����.
*/
void InterfaceSettings()
{
	DialogBuilder Builder(MConfigInterfaceTitle, L"InterfSettings");

	Builder.AddCheckbox(MConfigClock, &Opt.Clock);
	Builder.AddCheckbox(MConfigViewerEditorClock, &Opt.ViewerEditorClock);
	Builder.AddCheckbox(MConfigMouse, &Opt.Mouse);
	Builder.AddCheckbox(MConfigKeyBar, &Opt.ShowKeyBar);
	Builder.AddCheckbox(MConfigMenuBar, &Opt.ShowMenuBar);
	DialogItemEx *SaverCheckbox = Builder.AddCheckbox(MConfigSaver, &Opt.ScreenSaver);

	DialogItemEx *SaverEdit = Builder.AddIntEditField(&Opt.ScreenSaverTime, 2);
	SaverEdit->Indent(4);
	Builder.AddTextAfter(SaverEdit, MConfigSaverMinutes);
	Builder.LinkFlags(SaverCheckbox, SaverEdit, DIF_DISABLE);

	Builder.AddCheckbox(MConfigCopyTotal, &Opt.CMOpt.CopyShowTotal);
	Builder.AddCheckbox(MConfigCopyTimeRule, &Opt.CMOpt.CopyTimeRule);
	Builder.AddCheckbox(MConfigDeleteTotal, &Opt.DelOpt.DelShowTotal);
	Builder.AddCheckbox(MConfigPgUpChangeDisk, &Opt.PgUpChangeDisk);
	Builder.AddCheckbox(MConfigClearType, &Opt.ClearType);
	Builder.AddText(MConfigTitleAddons);
	Builder.AddEditField(&Opt.strTitleAddons, 47);
	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		if (Opt.CMOpt.CopyTimeRule)
			Opt.CMOpt.CopyTimeRule = 3;

		SetFarConsoleMode();
		CtrlObject->Cp()->LeftPanel->Update(UPDATE_KEEP_SELECTION);
		CtrlObject->Cp()->RightPanel->Update(UPDATE_KEEP_SELECTION);
		CtrlObject->Cp()->SetScreenPosition();
		// $ 10.07.2001 SKV ! ���� ��� ������, ����� ���� ������ ��������, ����� ������ ����.
		CtrlObject->Cp()->Redraw();
	}
}

void AutoCompleteSettings()
{
	DialogBuilder Builder(MConfigAutoCompleteTitle, L"AutoCompleteSettings");
	DialogItemEx *ListCheck=Builder.AddCheckbox(MConfigAutoCompleteShowList, &Opt.AutoComplete.ShowList);
	DialogItemEx *ModalModeCheck=Builder.AddCheckbox(MConfigAutoCompleteModalList, &Opt.AutoComplete.ModalList);
	ModalModeCheck->Indent(4);
	Builder.AddCheckbox(MConfigAutoCompleteAutoAppend, &Opt.AutoComplete.AppendCompletion);
	Builder.LinkFlags(ListCheck, ModalModeCheck, DIF_DISABLE);
	Builder.AddOKCancel();
	Builder.ShowDialog();
}

void InfoPanelSettings()
{
	DialogBuilderListItem UNListItems[]=
	{
		{ MConfigInfoPanelUNUnknown, NameUnknown },                            // 0  - unknown name type
		{ MConfigInfoPanelUNFullyQualifiedDN, NameFullyQualifiedDN },          // 1  - CN=John Doe, OU=Software, OU=Engineering, O=Widget, C=US
		{ MConfigInfoPanelUNSamCompatible, NameSamCompatible },                // 2  - Engineering\JohnDoe, If the user account is not in a domain, only NameSamCompatible is supported.
		{ MConfigInfoPanelUNDisplay, NameDisplay },                            // 3  - Probably "John Doe" but could be something else.  I.e. The display name is not necessarily the defining RDN.
		{ MConfigInfoPanelUNUniqueId, NameUniqueId },                          // 6  - String-ized GUID as returned by IIDFromString(). eg: {4fa050f0-f561-11cf-bdd9-00aa003a77b6}
		{ MConfigInfoPanelUNCanonical, NameCanonical },                        // 7  - engineering.widget.com/software/John Doe
		{ MConfigInfoPanelUNUserPrincipal, NameUserPrincipal },                // 8  - someone@example.com
		{ MConfigInfoPanelUNServicePrincipal, NameServicePrincipal },          // 10 - www/srv.engineering.com/engineering.com
		{ MConfigInfoPanelUNDnsDomain, NameDnsDomain },                        // 12 - DNS domain name + SAM username eg: engineering.widget.com\JohnDoe
	};
	DialogBuilderListItem CNListItems[]=
	{
		{ MConfigInfoPanelCNNetBIOS, ComputerNameNetBIOS },                                     // The NetBIOS name of the local computer or the cluster associated with the local computer. This name is limited to MAX_COMPUTERNAME_LENGTH + 1 characters and may be a truncated version of the DNS host name. For example, if the DNS host name is "corporate-mail-server", the NetBIOS name would be "corporate-mail-".
		{ MConfigInfoPanelCNDnsHostname, ComputerNameDnsHostname },                             // The DNS name of the local computer or the cluster associated with the local computer.
		{ MConfigInfoPanelCNDnsDomain, ComputerNameDnsDomain },                                 // The name of the DNS domain assigned to the local computer or the cluster associated with the local computer.
		{ MConfigInfoPanelCNDnsFullyQualified, ComputerNameDnsFullyQualified },                 // The fully-qualified DNS name that uniquely identifies the local computer or the cluster associated with the local computer. This name is a combination of the DNS host name and the DNS domain name, using the form HostName.DomainName. For example, if the DNS host name is "corporate-mail-server" and the DNS domain name is "microsoft.com", the fully qualified DNS name is "corporate-mail-server.microsoft.com".
		{ MConfigInfoPanelCNPhysicalNetBIOS, ComputerNamePhysicalNetBIOS },                     // The NetBIOS name of the local computer. On a cluster, this is the NetBIOS name of the local node on the cluster.
		{ MConfigInfoPanelCNPhysicalDnsHostname, ComputerNamePhysicalDnsHostname },             // The DNS host name of the local computer. On a cluster, this is the DNS host name of the local node on the cluster.
		{ MConfigInfoPanelCNPhysicalDnsDomain, ComputerNamePhysicalDnsDomain },                 // The name of the DNS domain assigned to the local computer. On a cluster, this is the DNS domain of the local node on the cluster.
		{ MConfigInfoPanelCNPhysicalDnsFullyQualified, ComputerNamePhysicalDnsFullyQualified }, // The fully-qualified DNS name that uniquely identifies the computer. On a cluster, this is the fully qualified DNS name of the local node on the cluster. The fully qualified DNS name is a combination of the DNS host name and the DNS domain name, using the form HostName.DomainName.
	};

	DialogBuilder Builder(MConfigInfoPanelTitle, L"InfoPanelSettings");
	Builder.AddText(MConfigInfoPanelCNTitle);
	Builder.AddComboBox((int *) &Opt.InfoPanel.ComputerNameFormat, 50, CNListItems, ARRAYSIZE(CNListItems), DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTWRAPMODE);
	Builder.AddText(MConfigInfoPanelUNTitle);
	Builder.AddComboBox((int *) &Opt.InfoPanel.UserNameFormat, 50, UNListItems, ARRAYSIZE(UNListItems), DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTWRAPMODE);
	Builder.AddCheckbox(MConfigInfoPanelShowPowerStatus, &Opt.InfoPanel.ShowPowerStatus);
	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		bool needRedraw=false;
		if (CtrlObject->Cp()->LeftPanel->GetType() == INFO_PANEL)
		{
			CtrlObject->Cp()->LeftPanel->Update(UPDATE_KEEP_SELECTION);
			needRedraw=true;
		}
		if (CtrlObject->Cp()->RightPanel->GetType() == INFO_PANEL)
		{
			CtrlObject->Cp()->RightPanel->Update(UPDATE_KEEP_SELECTION);
			needRedraw=true;
		}
		if (needRedraw)
		{
			//CtrlObject->Cp()->SetScreenPosition();
			CtrlObject->Cp()->Redraw();
		}
	}
}

void ApplyDefaultMaskGroups()
{
	struct MaskGroups
	{
		const wchar_t* Group;
		const wchar_t* Mask;
	}
	Sets[] =
	{
		{L"arc", L"*.rar,*.zip,*.[zj],*.[bg7]z,*.[bg]zip,*.tar,*.t[agbx]z,*.ar[cj],*.r[0-9][0-9],*.a[0-9][0-9],*.bz2,*.cab,*.msi,*.jar,*.lha,*.lzh,*.ha,*.ac[bei],*.pa[ck],*.rk,*.cpio,*.rpm,*.zoo,*.hqx,*.sit,*.ice,*.uc2,*.ain,*.imp,*.777,*.ufa,*.boa,*.bs[2a],*.sea,*.hpk,*.ddi,*.x2,*.rkv,*.[lw]sz,*.h[ay]p,*.lim,*.sqz,*.chz"},
		{L"temp", L"*.bak,*.tmp"},
		{L"exec", L"*.exe,*.com,*.bat,*.cmd,%PATHEXT%"},
	};

	for (size_t i = 0; i < ARRAYSIZE(Sets); ++i)
	{
		GeneralCfg->SetValue(L"Masks", Sets[i].Group, Sets[i].Mask);
	}
}

void FillMasksMenu(VMenu& MasksMenu, int SelPos = 0)
{
	MasksMenu.DeleteItems();
	string Name, Value;
	for(DWORD i = 0; GeneralCfg->EnumValues(L"Masks", i, Name, Value); ++i)
	{
		MenuItemEx Item = {};
		string DisplayName(Name);
		const int NameWidth = 10;
		TruncStr(DisplayName, NameWidth);
		Item.strName = FormatString() << fmt::Width(NameWidth) << fmt::Precision(NameWidth) << fmt::LeftAlign() << DisplayName << L' ' << BoxSymbols[BS_V1] << L' ' << Value;
		Item.UserData = const_cast<wchar_t*>(Name.CPtr());
		Item.UserDataSize = (Name.GetLength()+1)*sizeof(wchar_t);
		MasksMenu.AddItem(&Item);
	}
	MasksMenu.SetSelectPos(SelPos, 0);
}

void MaskGroupsSettings()
{
	VMenu MasksMenu(MSG(MMenuMaskGroups), nullptr, 0, 0, VMENU_WRAPMODE|VMENU_SHOWAMPERSAND);
	MasksMenu.SetBottomTitle(MSG(MMaskGroupBottom));
	MasksMenu.SetHelp(L"MaskGroupsSettings");
	FillMasksMenu(MasksMenu);
	MasksMenu.SetPosition(-1, -1, -1, -1);
	MasksMenu.Show();

	bool Changed = false;
	while (!MasksMenu.Done())
	{
		DWORD Key=MasksMenu.ReadInput();
		int ItemPos = MasksMenu.GetSelectPos();
		void* Data = MasksMenu.GetUserData(nullptr, 0, ItemPos);
		const wchar_t* Item = static_cast<const wchar_t*>(Data);
		switch (Key)
		{
		case KEY_NUMDEL:
		case KEY_DEL:
			if(Item && !Message(0,2,MSG(MMenuMaskGroups),MSG(MMaskGroupAskDelete), Item, MSG(MDelete), MSG(MCancel)))
			{
				GeneralCfg->DeleteValue(L"Masks", Item);
				Changed = true;
			}
			break;

		case KEY_NUMPAD0:
		case KEY_INS:
			Item = nullptr;
		case KEY_ENTER:
		case KEY_NUMENTER:
		case KEY_F4:
			{
				string Name(Item), Value;
				if(Item)
				{
					GeneralCfg->GetValue(L"Masks", Name, Value, L"");
				}
				DialogBuilder Builder(MMenuMaskGroups, nullptr);
				Builder.AddText(MMaskGroupName);
				Builder.AddEditField(&Name, 60);
				Builder.AddText(MMaskGroupMasks);
				Builder.AddEditField(&Value, 60);
				Builder.AddOKCancel();
				if(Builder.ShowDialog())
				{
					if(Item)
					{
						GeneralCfg->DeleteValue(L"Masks", Item);
					}
					GeneralCfg->SetValue(L"Masks", Name, Value);
					Changed = true;
				}
			}
			break;

		case KEY_CTRLR:
		case KEY_RCTRLR:
			{
				if (!Message(MSG_WARNING, 2,
					MSG(MMenuMaskGroups),
					MSG(MMaskGroupRestore),
					MSG(MYes),MSG(MCancel)))
				{
					ApplyDefaultMaskGroups();
					Changed = true;
				}
			}
			break;

		case KEY_F7:
			{
				string Value;
				DialogBuilder Builder(MFileFilterTitle, nullptr);
				Builder.AddText(MMaskGroupFindMask);
				Builder.AddEditField(&Value, 60, L"MaskGroupsFindMask");
				Builder.AddOKCancel();
				if(Builder.ShowDialog())
				{
					for (int i=0; i < MasksMenu.GetItemCount(); ++i)
					{
						string CurrentMasks;
						GeneralCfg->GetValue(L"Masks", static_cast<const wchar_t*>(MasksMenu.GetUserData(nullptr, 0, i)), CurrentMasks, L"");
						CFileMask Masks;
						Masks.Set(CurrentMasks, 0);
						if(!Masks.Compare(Value))
						{
							MasksMenu.UpdateItemFlags(i, MasksMenu.GetItemPtr(i)->Flags|MIF_HIDDEN);
						}
					}
					MasksMenu.SetPosition(-1, -1, -1, -1);
					MasksMenu.SetTitle(Value);
					MasksMenu.SetBottomTitle(LangString(MMaskGroupTotal) << MasksMenu.GetShowItemCount());
					MasksMenu.Show();
					while (!MasksMenu.Done())
					{
						DWORD Key=MasksMenu.ReadInput();
						if(Key == KEY_ESC || Key == KEY_F10 || Key == KEY_ENTER || Key == KEY_NUMENTER)
							break;
						else
							MasksMenu.ProcessKey(Key);
					}
					for (int i = 0; i < MasksMenu.GetItemCount(); ++i)
					{
						MasksMenu.UpdateItemFlags(i, MasksMenu.GetItemPtr(i)->Flags&~MIF_HIDDEN);
					}
					MasksMenu.SetPosition(-1, -1, -1, -1);
					MasksMenu.SetTitle(MSG(MMenuMaskGroups));
					MasksMenu.SetBottomTitle(MSG(MMaskGroupBottom));
					MasksMenu.Show();
				}
			}
			break;


		default:
			MasksMenu.ProcessInput();
			break;
		}

		if(Changed)
		{
			Changed = false;

			FillMasksMenu(MasksMenu, MasksMenu.GetSelectPos());
			MasksMenu.SetPosition(-1, -1, -1, -1);
			MasksMenu.SetUpdateRequired(true);
			MasksMenu.Show();
		}
	}
}

void DialogSettings()
{
	DialogBuilder Builder(MConfigDlgSetsTitle, L"DialogSettings");

	Builder.AddCheckbox(MConfigDialogsEditHistory, &Opt.Dialogs.EditHistory);
	Builder.AddCheckbox(MConfigDialogsEditBlock, &Opt.Dialogs.EditBlock);
	Builder.AddCheckbox(MConfigDialogsDelRemovesBlocks, &Opt.Dialogs.DelRemovesBlocks);
	Builder.AddCheckbox(MConfigDialogsAutoComplete, &Opt.Dialogs.AutoComplete);
	Builder.AddCheckbox(MConfigDialogsEULBsClear, &Opt.Dialogs.EULBsClear);
	Builder.AddCheckbox(MConfigDialogsMouseButton, &Opt.Dialogs.MouseButton);
	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		if (Opt.Dialogs.MouseButton )
			Opt.Dialogs.MouseButton = 0xFFFF;
	}
}

void VMenuSettings()
{
	DialogBuilderListItem CAListItems[]=
	{
		{ MConfigVMenuClickCancel, VMENUCLICK_CANCEL },  // Cancel menu
		{ MConfigVMenuClickApply,  VMENUCLICK_APPLY  },  // Execute selected item
		{ MConfigVMenuClickIgnore, VMENUCLICK_IGNORE },  // Do nothing
	};

	DialogBuilder Builder(MConfigVMenuTitle, L"VMenuSettings");

	Builder.AddText(MConfigVMenuLBtnClick);
	Builder.AddComboBox((int *) &Opt.VMenu.LBtnClick, 40, CAListItems, ARRAYSIZE(CAListItems), DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTWRAPMODE);
	Builder.AddText(MConfigVMenuRBtnClick);
	Builder.AddComboBox((int *) &Opt.VMenu.RBtnClick, 40, CAListItems, ARRAYSIZE(CAListItems), DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTWRAPMODE);
	Builder.AddText(MConfigVMenuMBtnClick);
	Builder.AddComboBox((int *) &Opt.VMenu.MBtnClick, 40, CAListItems, ARRAYSIZE(CAListItems), DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTWRAPMODE);
	Builder.AddOKCancel();
	Builder.ShowDialog();
}

void CmdlineSettings()
{
	DialogBuilder Builder(MConfigCmdlineTitle, L"CmdlineSettings");

	Builder.AddCheckbox(MConfigCmdlineEditBlock, &Opt.CmdLine.EditBlock);
	Builder.AddCheckbox(MConfigCmdlineDelRemovesBlocks, &Opt.CmdLine.DelRemovesBlocks);
	Builder.AddCheckbox(MConfigCmdlineAutoComplete, &Opt.CmdLine.AutoComplete);
	DialogItemEx *UsePromptFormat = Builder.AddCheckbox(MConfigCmdlineUsePromptFormat, &Opt.CmdLine.UsePromptFormat);
	DialogItemEx *PromptFormat = Builder.AddEditField(&Opt.CmdLine.strPromptFormat, 33);
	PromptFormat->Indent(4);
	Builder.LinkFlags(UsePromptFormat, PromptFormat, DIF_DISABLE);

	UsePromptFormat = Builder.AddCheckbox(MConfigCmdlineUseHomeDir, &Opt.Exec.UseHomeDir);
	PromptFormat = Builder.AddEditField(&Opt.Exec.strHomeDir, 33);
	PromptFormat->Indent(4);
	Builder.LinkFlags(UsePromptFormat, PromptFormat, DIF_DISABLE);

	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		CtrlObject->CmdLine->SetPersistentBlocks(Opt.CmdLine.EditBlock);
		CtrlObject->CmdLine->SetDelRemovesBlocks(Opt.CmdLine.DelRemovesBlocks);
		CtrlObject->CmdLine->SetAutoComplete(Opt.CmdLine.AutoComplete);
	}
}

void SetConfirmations()
{
	DialogBuilder Builder(MSetConfirmTitle, L"ConfirmDlg");

	Builder.AddCheckbox(MSetConfirmCopy, &Opt.Confirm.Copy);
	Builder.AddCheckbox(MSetConfirmMove, &Opt.Confirm.Move);
	Builder.AddCheckbox(MSetConfirmRO, &Opt.Confirm.RO);
	Builder.AddCheckbox(MSetConfirmDrag, &Opt.Confirm.Drag);
	Builder.AddCheckbox(MSetConfirmDelete, &Opt.Confirm.Delete);
	Builder.AddCheckbox(MSetConfirmDeleteFolders, &Opt.Confirm.DeleteFolder);
	Builder.AddCheckbox(MSetConfirmEsc, &Opt.Confirm.Esc);
	Builder.AddCheckbox(MSetConfirmRemoveConnection, &Opt.Confirm.RemoveConnection);
	Builder.AddCheckbox(MSetConfirmRemoveSUBST, &Opt.Confirm.RemoveSUBST);
	Builder.AddCheckbox(MSetConfirmDetachVHD, &Opt.Confirm.DetachVHD);
	Builder.AddCheckbox(MSetConfirmRemoveHotPlug, &Opt.Confirm.RemoveHotPlug);
	Builder.AddCheckbox(MSetConfirmAllowReedit, &Opt.Confirm.AllowReedit);
	Builder.AddCheckbox(MSetConfirmHistoryClear, &Opt.Confirm.HistoryClear);
	Builder.AddCheckbox(MSetConfirmExit, &Opt.Confirm.Exit);
	Builder.AddOKCancel();

	Builder.ShowDialog();
}

void PluginsManagerSettings()
{
	DialogBuilder Builder(MPluginsManagerSettingsTitle, L"PluginsManagerSettings");
#ifndef NO_WRAPPER
	Builder.AddCheckbox(MPluginsManagerOEMPluginsSupport, &Opt.LoadPlug.OEMPluginsSupport);
#endif // NO_WRAPPER
	Builder.AddCheckbox(MPluginsManagerScanSymlinks, &Opt.LoadPlug.ScanSymlinks);
	Builder.AddSeparator(MPluginConfirmationTitle);
	DialogItemEx *ConfirmOFP = Builder.AddCheckbox(MPluginsManagerOFP, &Opt.PluginConfirm.OpenFilePlugin);
	ConfirmOFP->Flags|=DIF_3STATE;
	DialogItemEx *StandardAssoc = Builder.AddCheckbox(MPluginsManagerStdAssoc, &Opt.PluginConfirm.StandardAssociation);
	DialogItemEx *EvenIfOnlyOne = Builder.AddCheckbox(MPluginsManagerEvenOne, &Opt.PluginConfirm.EvenIfOnlyOnePlugin);
	StandardAssoc->Indent(2);
	EvenIfOnlyOne->Indent(4);

	Builder.AddCheckbox(MPluginsManagerSFL, &Opt.PluginConfirm.SetFindList);
	Builder.AddCheckbox(MPluginsManagerPF, &Opt.PluginConfirm.Prefix);
	Builder.AddOKCancel();

	Builder.ShowDialog();
}


void SetDizConfig()
{
	DialogBuilder Builder(MCfgDizTitle, L"FileDiz");

	Builder.AddText(MCfgDizListNames);
	Builder.AddEditField(&Opt.Diz.strListNames, 65);
	Builder.AddSeparator();

	Builder.AddCheckbox(MCfgDizSetHidden, &Opt.Diz.SetHidden);
	Builder.AddCheckbox(MCfgDizROUpdate, &Opt.Diz.ROUpdate);
	DialogItemEx *StartPos = Builder.AddIntEditField(&Opt.Diz.StartPos, 2);
	Builder.AddTextAfter(StartPos, MCfgDizStartPos);
	Builder.AddSeparator();

	static int DizOptions[] = { MCfgDizNotUpdate, MCfgDizUpdateIfDisplayed, MCfgDizAlwaysUpdate };
	Builder.AddRadioButtons(&Opt.Diz.UpdateMode, 3, DizOptions);
	Builder.AddSeparator();

	Builder.AddCheckbox(MCfgDizAnsiByDefault, &Opt.Diz.AnsiByDefault);
	Builder.AddCheckbox(MCfgDizSaveInUTF, &Opt.Diz.SaveInUTF);
	Builder.AddOKCancel();
	Builder.ShowDialog();
}

void ViewerConfig(ViewerOptions &ViOpt,bool Local)
{
	DialogBuilder Builder(MViewConfigTitle, L"ViewerSettings");
	if (!Local)
	{
		Builder.AddCheckbox(MViewConfigExternalF3, &Opt.ViOpt.UseExternalViewer);
		Builder.AddText(MViewConfigExternalCommand);
		Builder.AddEditField(&Opt.strExternalViewer, 64, L"ExternalViewer", DIF_EDITPATH);
		Builder.AddSeparator(MViewConfigInternal);
	}

	Builder.StartColumns();
	Builder.AddCheckbox(MViewConfigPersistentSelection, &ViOpt.PersistentBlocks);
	DialogItemEx *SavePos = Builder.AddCheckbox(MViewConfigSavePos, &Opt.ViOpt.SavePos); // can't be local
	Builder.AddCheckbox(MViewConfigEditAutofocus, &ViOpt.SearchEditFocus);
	DialogItemEx *TabSize = Builder.AddIntEditField(&ViOpt.TabSize, 3);
	Builder.AddTextAfter(TabSize, MViewConfigTabSize);
	Builder.ColumnBreak();
	Builder.AddCheckbox(MViewConfigArrows, &ViOpt.ShowArrows);
	DialogItemEx *SaveShortPos = Builder.AddCheckbox(MViewConfigSaveShortPos, &Opt.ViOpt.SaveShortPos); // can't be local
	Builder.LinkFlags(SavePos, SaveShortPos, DIF_DISABLE);
	Builder.AddCheckbox(MViewConfigVisible0x00, &ViOpt.Visible0x00);
	Builder.AddCheckbox(MViewConfigScrollbar, &ViOpt.ShowScrollbar);
	Builder.EndColumns();

	if (!Local)
	{
		Builder.AddEmptyLine();
		DialogItemEx *MaxLineSize = Builder.AddIntEditField(&Opt.ViOpt.MaxLineSize, 6);
		Builder.AddTextAfter(MaxLineSize, MViewConfigMaxLineSize);
		Builder.AddCheckbox(MViewAutoDetectCodePage, &Opt.ViOpt.AutoDetectCodePage);
		Builder.AddCheckbox(MViewConfigAnsiCodePageAsDefault, &Opt.ViOpt.AnsiCodePageAsDefault);
	}
	Builder.AddOKCancel();
	if (Builder.ShowDialog())
	{
		if (ViOpt.TabSize<1 || ViOpt.TabSize>512)
			ViOpt.TabSize=8;
		// Maximus5: BUGBUG: ��� ���� �� ��������� �� ��������� ��������� ���� Viewer.
		if (!Opt.ViOpt.MaxLineSize)
			Opt.ViOpt.MaxLineSize = ViewerOptions::eDefLineSize;
		else if (Opt.ViOpt.MaxLineSize < ViewerOptions::eMinLineSize)
			Opt.ViOpt.MaxLineSize = ViewerOptions::eMinLineSize;
		else if (Opt.ViOpt.MaxLineSize > ViewerOptions::eMaxLineSize)
			Opt.ViOpt.MaxLineSize = ViewerOptions::eMaxLineSize;
	}
}

void EditorConfig(EditorOptions &EdOpt,bool Local)
{
	DialogBuilder Builder(MEditConfigTitle, L"EditorSettings");
	if (!Local)
	{
		Builder.AddCheckbox(MEditConfigEditorF4, &Opt.EdOpt.UseExternalEditor);
		Builder.AddText(MEditConfigEditorCommand);
		Builder.AddEditField(&Opt.strExternalEditor, 64, L"ExternalEditor", DIF_EDITPATH);
		Builder.AddSeparator(MEditConfigInternal);
	}

	Builder.AddText(MEditConfigExpandTabsTitle);
	DialogBuilderListItem ExpandTabsItems[] = {
		{ MEditConfigDoNotExpandTabs, EXPAND_NOTABS },
		{ MEditConfigExpandTabs, EXPAND_NEWTABS },
		{ MEditConfigConvertAllTabsToSpaces, EXPAND_ALLTABS }
	};
	Builder.AddComboBox(&EdOpt.ExpandTabs, 64, ExpandTabsItems, 3, DIF_DROPDOWNLIST|DIF_LISTAUTOHIGHLIGHT|DIF_LISTWRAPMODE);

	Builder.StartColumns();
	Builder.AddCheckbox(MEditConfigPersistentBlocks, &EdOpt.PersistentBlocks);
	DialogItemEx *SavePos = Builder.AddCheckbox(MEditConfigSavePos, &EdOpt.SavePos);
	Builder.AddCheckbox(MEditConfigAutoIndent, &EdOpt.AutoIndent);
	DialogItemEx *TabSize = Builder.AddIntEditField(&EdOpt.TabSize, 3);
	Builder.AddTextAfter(TabSize, MEditConfigTabSize);
	Builder.AddCheckbox(MEditShowWhiteSpace, &EdOpt.ShowWhiteSpace, 0, true);
	Builder.ColumnBreak();
	Builder.AddCheckbox(MEditConfigDelRemovesBlocks, &EdOpt.DelRemovesBlocks);
	DialogItemEx *SaveShortPos = Builder.AddCheckbox(MEditConfigSaveShortPos, &EdOpt.SaveShortPos);
	Builder.LinkFlags(SavePos, SaveShortPos, DIF_DISABLE);
	Builder.AddCheckbox(MEditCursorBeyondEnd, &EdOpt.CursorBeyondEOL);
	Builder.AddCheckbox(MEditConfigScrollbar, &EdOpt.ShowScrollBar);
	Builder.AddCheckbox(MEditConfigPickUpWord, &EdOpt.SearchPickUpWord);
	Builder.EndColumns();

	if (!Local)
	{
		Builder.AddEmptyLine();
		Builder.AddCheckbox(MEditShareWrite, &EdOpt.EditOpenedForWrite);
		Builder.AddCheckbox(MEditLockROFileModification, &EdOpt.ReadOnlyLock, 1);
		Builder.AddCheckbox(MEditWarningBeforeOpenROFile, &EdOpt.ReadOnlyLock, 2);
		Builder.AddCheckbox(MEditAutoDetectCodePage, &EdOpt.AutoDetectCodePage);
		Builder.AddCheckbox(MEditConfigAnsiCodePageAsDefault, &EdOpt.AnsiCodePageAsDefault);
		Builder.AddCheckbox(MEditConfigAnsiCodePageForNewFile, &EdOpt.AnsiCodePageForNewFile);
	}

	Builder.AddOKCancel();

	if (Builder.ShowDialog())
	{
		if (EdOpt.TabSize<1 || EdOpt.TabSize>512)
			EdOpt.TabSize=8;
	}
}


void SetFolderInfoFiles()
{
	string strFolderInfoFiles;

	if (GetString(MSG(MSetFolderInfoTitle),MSG(MSetFolderInfoNames),L"FolderInfoFiles",
	              Opt.InfoPanel.strFolderInfoFiles,strFolderInfoFiles,L"FolderDiz",FIB_ENABLEEMPTY|FIB_BUTTONS))
	{
		Opt.InfoPanel.strFolderInfoFiles = strFolderInfoFiles;

		if (CtrlObject->Cp()->LeftPanel->GetType() == INFO_PANEL)
			CtrlObject->Cp()->LeftPanel->Update(0);

		if (CtrlObject->Cp()->RightPanel->GetType() == INFO_PANEL)
			CtrlObject->Cp()->RightPanel->Update(0);
	}
}


// ���������, ����������� ��� ������������(!)
static struct FARConfig
{
	int   IsSave;   // =1 - ����� ������������ � SaveConfig()
	DWORD ValType;  // TYPE_INTEGER, TYPE_TEXT, TYPE_BLOB
	size_t ApiRoot;
	const wchar_t *KeyName;
	const wchar_t *ValName;
	void *ValPtr;   // ����� ����������, ���� �������� ������
	DWORD DefDWord; // �� �� ������ ������ ��� TYPE_BLOB
	const void* DefStr;   // ������/������ �� ���������
} CFG[]=
{
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyCmdline, L"AutoComplete",&Opt.CmdLine.AutoComplete,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyCmdline, L"EditBlock", &Opt.CmdLine.EditBlock,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyCmdline, L"DelRemovesBlocks", &Opt.CmdLine.DelRemovesBlocks,1, 0},
	{1, GeneralConfig::TYPE_TEXT,    FSSF_PRIVATE,           NKeyCmdline, L"PromptFormat",&Opt.CmdLine.strPromptFormat, 0, L"$p$g"},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyCmdline, L"UsePromptFormat", &Opt.CmdLine.UsePromptFormat,0, 0},

	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyCodePages,L"CPMenuMode",&Opt.CPMenuMode,0,0},
	{1, GeneralConfig::TYPE_TEXT,    FSSF_PRIVATE,           NKeyCodePages,L"NoAutoDetectCP",&Opt.strNoAutoDetectCP,0,L""},

	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyConfirmations,L"AllowReedit",&Opt.Confirm.AllowReedit,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_CONFIRMATIONS,     NKeyConfirmations,L"Copy",&Opt.Confirm.Copy,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_CONFIRMATIONS,     NKeyConfirmations,L"Delete",&Opt.Confirm.Delete,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_CONFIRMATIONS,     NKeyConfirmations,L"DeleteFolder",&Opt.Confirm.DeleteFolder,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyConfirmations,L"DetachVHD",&Opt.Confirm.DetachVHD,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_CONFIRMATIONS,     NKeyConfirmations,L"Drag",&Opt.Confirm.Drag,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_CONFIRMATIONS,     NKeyConfirmations,L"Esc",&Opt.Confirm.Esc,1, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyConfirmations,L"EscTwiceToInterrupt",&Opt.Confirm.EscTwiceToInterrupt,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_CONFIRMATIONS,     NKeyConfirmations,L"Exit",&Opt.Confirm.Exit,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_CONFIRMATIONS,     NKeyConfirmations,L"HistoryClear",&Opt.Confirm.HistoryClear,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_CONFIRMATIONS,     NKeyConfirmations,L"Move",&Opt.Confirm.Move,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_CONFIRMATIONS,     NKeyConfirmations,L"RemoveConnection",&Opt.Confirm.RemoveConnection,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyConfirmations,L"RemoveHotPlug",&Opt.Confirm.RemoveHotPlug,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyConfirmations,L"RemoveSUBST",&Opt.Confirm.RemoveSUBST,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_CONFIRMATIONS,     NKeyConfirmations,L"RO",&Opt.Confirm.RO,1, 0},

	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyDescriptions,L"AnsiByDefault",&Opt.Diz.AnsiByDefault,0, 0},
	{1, GeneralConfig::TYPE_TEXT,    FSSF_PRIVATE,           NKeyDescriptions,L"ListNames",&Opt.Diz.strListNames, 0, L"Descript.ion,Files.bbs"},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyDescriptions,L"ROUpdate",&Opt.Diz.ROUpdate,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyDescriptions,L"SaveInUTF",&Opt.Diz.SaveInUTF,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyDescriptions,L"SetHidden",&Opt.Diz.SetHidden,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyDescriptions,L"StartPos",&Opt.Diz.StartPos,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyDescriptions,L"UpdateMode",&Opt.Diz.UpdateMode,DIZ_UPDATE_IF_DISPLAYED, 0},

	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyDialog,L"AutoComplete",&Opt.Dialogs.AutoComplete,1, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyDialog,L"CBoxMaxHeight",&Opt.Dialogs.CBoxMaxHeight,8, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_DIALOG,            NKeyDialog,L"EditBlock",&Opt.Dialogs.EditBlock,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyDialog,L"EditHistory",&Opt.Dialogs.EditHistory,1, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyDialog,L"EditLine",&Opt.Dialogs.EditLine,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_DIALOG,            NKeyDialog,L"DelRemovesBlocks",&Opt.Dialogs.DelRemovesBlocks,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_DIALOG,            NKeyDialog,L"EULBsClear",&Opt.Dialogs.EULBsClear,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyDialog,L"MouseButton",&Opt.Dialogs.MouseButton,0xFFFF, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyDialog,L"SelectFromHistory",&Opt.Dialogs.SelectFromHistory,0, 0},

	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyEditor,L"AllowEmptySpaceAfterEof", &Opt.EdOpt.AllowEmptySpaceAfterEof,0,0},//skv
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyEditor,L"AnsiCodePageAsDefault",&Opt.EdOpt.AnsiCodePageAsDefault,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyEditor,L"AnsiCodePageForNewFile",&Opt.EdOpt.AnsiCodePageForNewFile,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyEditor,L"AutoDetectCodePage",&Opt.EdOpt.AutoDetectCodePage,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyEditor,L"AutoIndent",&Opt.EdOpt.AutoIndent,0, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyEditor,L"BSLikeDel",&Opt.EdOpt.BSLikeDel,1, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyEditor,L"CharCodeBase",&Opt.EdOpt.CharCodeBase,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyEditor,L"DelRemovesBlocks",&Opt.EdOpt.DelRemovesBlocks,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyEditor,L"EditOpenedForWrite",&Opt.EdOpt.EditOpenedForWrite,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyEditor,L"EditorCursorBeyondEOL",&Opt.EdOpt.CursorBeyondEOL,1, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyEditor,L"EditorF7Rules",&Opt.EdOpt.F7Rules,0, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyEditor,L"EditorUndoSize",&Opt.EdOpt.UndoSize,0, 0}, // $ 03.12.2001 IS ������ ������ undo � ���������
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyEditor,L"ExpandTabs",&Opt.EdOpt.ExpandTabs,0, 0},
	{1, GeneralConfig::TYPE_TEXT,    FSSF_PRIVATE,           NKeyEditor,L"ExternalEditorName",&Opt.strExternalEditor, 0, L""},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyEditor,L"FileSizeLimit",&Opt.EdOpt.FileSizeLimitLo, 0, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyEditor,L"FileSizeLimitHi",&Opt.EdOpt.FileSizeLimitHi, 0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyEditor,L"PersistentBlocks",&Opt.EdOpt.PersistentBlocks,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyEditor,L"ReadOnlyLock",&Opt.EdOpt.ReadOnlyLock,0, 0}, // ����� ����� ������ 1.65 - �� ������������� � �� �����������
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyEditor,L"SaveEditorPos",&Opt.EdOpt.SavePos,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyEditor,L"SaveEditorShortPos",&Opt.EdOpt.SaveShortPos,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyEditor,L"SearchPickUpWord",&Opt.EdOpt.SearchPickUpWord,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyEditor,L"SearchRegexp",&Opt.EdOpt.SearchRegexp,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyEditor,L"SearchSelFound",&Opt.EdOpt.SearchSelFound,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyEditor,L"ShowKeyBar",&Opt.EdOpt.ShowKeyBar,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyEditor,L"ShowScrollBar",&Opt.EdOpt.ShowScrollBar,0, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyEditor,L"ShowTitleBar",&Opt.EdOpt.ShowTitleBar,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyEditor,L"ShowWhiteSpace",&Opt.EdOpt.ShowWhiteSpace,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyEditor,L"TabSize",&Opt.EdOpt.TabSize,8, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyEditor,L"UseExternalEditor",&Opt.EdOpt.UseExternalEditor,0, 0},
	{0, GeneralConfig::TYPE_TEXT,    FSSF_EDITOR,            NKeyEditor,L"WordDiv",&Opt.strWordDiv, 0, WordDiv0},

	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyHelp,L"ActivateURL",&Opt.HelpURLRules,1, 0},

	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyCommandHistory, NParamHistoryCount,&Opt.HistoryCount,512, 0}, //BUGBUG
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyDialogHistory, NParamHistoryCount,&Opt.DialogsHistoryCount,512, 0}, //BUGBUG
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyFolderHistory, NParamHistoryCount,&Opt.FoldersHistoryCount,512, 0}, //BUGBUG
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyViewEditHistory, NParamHistoryCount,&Opt.ViewHistoryCount,512, 0}, //BUGBUG

	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyInterface,L"DelShowTotal",&Opt.DelOpt.DelShowTotal,0, 0},

	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyInterface, L"AltF9",&Opt.AltF9, 1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyInterface, L"ClearType",&Opt.ClearType, 1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyInterface, L"CopyShowTotal",&Opt.CMOpt.CopyShowTotal,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyInterface, L"CtrlPgUp",&Opt.PgUpChangeDisk, 1, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyInterface, L"CursorSize1",&Opt.CursorSize[0],15, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyInterface, L"CursorSize2",&Opt.CursorSize[1],10, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyInterface, L"CursorSize3",&Opt.CursorSize[2],99, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyInterface, L"CursorSize4",&Opt.CursorSize[3],99, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyInterface, L"FormatNumberSeparators",&Opt.FormatNumberSeparators, 0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyInterface, L"Mouse",&Opt.Mouse,1, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyInterface, L"ShiftsKeyRules",&Opt.ShiftsKeyRules,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyInterface, L"ShowDotsInRoot",&Opt.ShowDotsInRoot, 0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_INTERFACE,         NKeyInterface, L"ShowMenuBar",&Opt.ShowMenuBar,0, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyInterface, L"ShowTimeoutDACLFiles",&Opt.ShowTimeoutDACLFiles, 50, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyInterface, L"ShowTimeoutDelFiles",&Opt.ShowTimeoutDelFiles, 50, 0},
	{1, GeneralConfig::TYPE_TEXT,    FSSF_PRIVATE,           NKeyInterface, L"TitleAddons",&Opt.strTitleAddons, 0, L"%Ver.%Build %Platform %Admin"},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyInterface, L"UseVk_oem_x",&Opt.UseVk_oem_x,1, 0},

	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyInterfaceCompletion,L"Append",&Opt.AutoComplete.AppendCompletion, 0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyInterfaceCompletion,L"ModalList",&Opt.AutoComplete.ModalList, 0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyInterfaceCompletion,L"ShowList",&Opt.AutoComplete.ShowList, 1, 0},

	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyLayout,L"FullscreenHelp",&Opt.FullScreenHelp,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyLayout,L"LeftHeightDecrement",&Opt.LeftHeightDecrement,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyLayout,L"RightHeightDecrement",&Opt.RightHeightDecrement,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyLayout,L"WidthDecrement",&Opt.WidthDecrement,0, 0},

	{0, GeneralConfig::TYPE_TEXT,    FSSF_PRIVATE,           NKeyKeyMacros,L"CONVFMT",&Opt.Macro.strMacroCONVFMT, 0, L"%.6g"},
	{0, GeneralConfig::TYPE_TEXT,    FSSF_PRIVATE,           NKeyKeyMacros,L"DateFormat",&Opt.Macro.strDateFormat, 0, L"%a %b %d %H:%M:%S %Z %Y"},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyKeyMacros,L"MacroReuseRules",&Opt.Macro.MacroReuseRules,0, 0},

	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanel,L"AutoUpdateLimit",&Opt.AutoUpdateLimit, 0, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanel,L"CtrlAltShiftRule",&Opt.PanelCtrlAltShiftRule,0, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanel,L"CtrlFRule",&Opt.PanelCtrlFRule,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanel,L"Highlight",&Opt.Highlight,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanel,L"ReverseSort",&Opt.ReverseSort,1, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanel,L"RememberLogicalDrives",&Opt.RememberLogicalDrives, 0, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanel,L"RightClickRule",&Opt.PanelRightClickRule,2, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanel,L"SelectFolders",&Opt.SelectFolders,0, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanel,L"ShellRightLeftArrowsRule",&Opt.ShellRightLeftArrowsRule,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PANEL,             NKeyPanel,L"ShowHidden",&Opt.ShowHidden,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanel,L"SortFolderExt",&Opt.SortFolderExt,0, 0},

	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelInfo,L"InfoComputerNameFormat",&Opt.InfoPanel.ComputerNameFormat, ComputerNamePhysicalNetBIOS, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelInfo,L"InfoUserNameFormat",&Opt.InfoPanel.UserNameFormat, NameUserPrincipal, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelInfo,L"ShowPowerStatus",&Opt.InfoPanel.ShowPowerStatus, 0, 0},

	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelLayout,L"ColoredGlobalColumnSeparator",&Opt.HighlightColumnSeparator,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PANELLAYOUT,       NKeyPanelLayout,L"ColumnTitles",&Opt.ShowColumnTitles,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PANELLAYOUT,       NKeyPanelLayout,L"DetailedJunction",&Opt.PanelDetailedJunction,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelLayout,L"DoubleGlobalColumnSeparator",&Opt.DoubleGlobalColumnSeparator,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelLayout,L"FreeInfo",&Opt.ShowPanelFree,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelLayout,L"ScreensNumber",&Opt.ShowScreensNumber,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelLayout,L"Scrollbar",&Opt.ShowPanelScrollbar,0, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelLayout,L"ScrollbarMenu",&Opt.ShowMenuScrollbar,1, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelLayout,L"ShowUnknownReparsePoint",&Opt.ShowUnknownReparsePoint,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PANELLAYOUT,       NKeyPanelLayout,L"SortMode",&Opt.ShowSortMode,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PANELLAYOUT,       NKeyPanelLayout,L"StatusLine",&Opt.ShowPanelStatus,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelLayout,L"TotalInfo",&Opt.ShowPanelTotals,1, 0},

	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelLeft,L"CaseSensitiveSort",&Opt.LeftPanel.CaseSensitiveSort,0, 0},
	{1, GeneralConfig::TYPE_TEXT,    FSSF_PRIVATE,           NKeyPanelLeft,L"CurFile",&Opt.strLeftCurFile, 0, L""},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelLeft,L"DirectoriesFirst",&Opt.LeftPanel.DirectoriesFirst,1,0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelLeft,L"Focus",&Opt.LeftPanel.Focus,1, 0},
	{1, GeneralConfig::TYPE_TEXT,    FSSF_PRIVATE,           NKeyPanelLeft,L"Folder",&Opt.strLeftFolder, 0, L""},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelLeft,L"NumericSort",&Opt.LeftPanel.NumericSort,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelLeft,L"SelectedFirst",&Opt.LeftSelectedFirst,0,0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelLeft,L"ShortNames",&Opt.LeftPanel.ShowShortNames,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelLeft,L"SortGroups",&Opt.LeftPanel.SortGroups,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelLeft,L"SortMode",&Opt.LeftPanel.SortMode,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelLeft,L"SortOrder",&Opt.LeftPanel.SortOrder,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelLeft,L"Type",&Opt.LeftPanel.Type,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelLeft,L"ViewMode",&Opt.LeftPanel.ViewMode,2, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelLeft,L"Visible",&Opt.LeftPanel.Visible,1, 0},

	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelRight,L"CaseSensitiveSort",&Opt.RightPanel.CaseSensitiveSort,0, 0},
	{1, GeneralConfig::TYPE_TEXT,    FSSF_PRIVATE,           NKeyPanelRight,L"CurFile",&Opt.strRightCurFile, 0,L""},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelRight,L"DirectoriesFirst",&Opt.RightPanel.DirectoriesFirst,1,0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelRight,L"Focus",&Opt.RightPanel.Focus,0, 0},
	{1, GeneralConfig::TYPE_TEXT,    FSSF_PRIVATE,           NKeyPanelRight,L"Folder",&Opt.strRightFolder, 0,L""},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelRight,L"NumericSort",&Opt.RightPanel.NumericSort,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelRight,L"SelectedFirst",&Opt.RightSelectedFirst,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelRight,L"ShortNames",&Opt.RightPanel.ShowShortNames,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelRight,L"SortGroups",&Opt.RightPanel.SortGroups,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelRight,L"SortMode",&Opt.RightPanel.SortMode,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelRight,L"SortOrder",&Opt.RightPanel.SortOrder,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelRight,L"Type",&Opt.RightPanel.Type,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelRight,L"ViewMode",&Opt.RightPanel.ViewMode,2, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelRight,L"Visible",&Opt.RightPanel.Visible,1, 0},

	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelTree,L"AutoChangeFolder",&Opt.Tree.AutoChangeFolder,0, 0}, // ???
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelTree,L"MinTreeCount",&Opt.Tree.MinTreeCount, 4, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelTree,L"TreeFileAttr",&Opt.Tree.TreeFileAttr, FILE_ATTRIBUTE_HIDDEN, 0},
#if defined(TREEFILE_PROJECT)
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelTree,L"CDDisk",&Opt.Tree.CDDisk, 2, 0},
	{0, GeneralConfig::TYPE_TEXT,    FSSF_PRIVATE,           NKeyPanelTree,L"CDDiskTemplate,0",&Opt.Tree.strCDDisk,0,constCDDiskTemplate},
	{0, GeneralConfig::TYPE_TEXT,    FSSF_PRIVATE,           NKeyPanelTree,L"ExceptPath",&Opt.Tree.strExceptPath,0,L""},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelTree,L"LocalDisk",&Opt.Tree.LocalDisk, 2, 0},
	{0, GeneralConfig::TYPE_TEXT,    FSSF_PRIVATE,           NKeyPanelTree,L"LocalDiskTemplate",&Opt.Tree.strLocalDisk,0,constLocalDiskTemplate},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelTree,L"NetDisk",&Opt.Tree.NetDisk, 2, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelTree,L"NetPath",&Opt.Tree.NetPath, 2, 0},
	{0, GeneralConfig::TYPE_TEXT,    FSSF_PRIVATE,           NKeyPanelTree,L"NetDiskTemplate",&Opt.Tree.strNetDisk,0,constNetDiskTemplate},
	{0, GeneralConfig::TYPE_TEXT,    FSSF_PRIVATE,           NKeyPanelTree,L"NetPathTemplate",&Opt.Tree.strNetPath,0,constNetPathTemplate},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPanelTree,L"RemovableDisk",&Opt.Tree.RemovableDisk, 2, 0},
	{0, GeneralConfig::TYPE_TEXT,    FSSF_PRIVATE,           NKeyPanelTree,L"RemovableDiskTemplate,",&Opt.Tree.strRemovableDisk,0,constRemovableDiskTemplate},
	{0, GeneralConfig::TYPE_TEXT,    FSSF_PRIVATE,           NKeyPanelTree,L"SaveLocalPath",&Opt.Tree.strSaveLocalPath,0,L""},
	{0, GeneralConfig::TYPE_TEXT,    FSSF_PRIVATE,           NKeyPanelTree,L"SaveNetPath",&Opt.Tree.strSaveNetPath,0,L""},
#endif

	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPluginConfirmations, L"EvenIfOnlyOnePlugin", &Opt.PluginConfirm.EvenIfOnlyOnePlugin, 0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPluginConfirmations, L"OpenFilePlugin", &Opt.PluginConfirm.OpenFilePlugin, 0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPluginConfirmations, L"Prefix", &Opt.PluginConfirm.Prefix, 0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPluginConfirmations, L"SetFindList", &Opt.PluginConfirm.SetFindList, 0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPluginConfirmations, L"StandardAssociation", &Opt.PluginConfirm.StandardAssociation, 0, 0},

	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPolicies,L"DisabledOptions",&Opt.Policies.DisabledOptions,0, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyPolicies,L"ShowHiddenDrives",&Opt.Policies.ShowHiddenDrives,1, 0},

	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyScreen, L"Clock", &Opt.Clock, 1, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyScreen, L"DeltaXY", &Opt.ScrSize.DeltaXY, 0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_SCREEN,            NKeyScreen, L"KeyBar",&Opt.ShowKeyBar,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyScreen, L"ScreenSaver",&Opt.ScreenSaver, 0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyScreen, L"ScreenSaverTime",&Opt.ScreenSaverTime,5, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyScreen, L"ViewerEditorClock",&Opt.ViewerEditorClock,1, 0},

	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"AllCtrlAltShiftRule",&Opt.AllCtrlAltShiftRule,0x0000FFFF, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"AutoSaveSetup",&Opt.AutoSaveSetup,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"AutoUpdateRemoteDrive",&Opt.AutoUpdateRemoteDrive,1, 0},
	{0, GeneralConfig::TYPE_TEXT,    FSSF_PRIVATE,           NKeySystem,L"BoxSymbols",&Opt.strBoxSymbols, 0, _BoxSymbols},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"CASRule",&Opt.CASRule,0xFFFFFFFFU, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"CloseCDGate",&Opt.CloseCDGate,1, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"CmdHistoryRule",&Opt.CmdHistoryRule,0, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"CollectFiles",&Opt.FindOpt.CollectFiles, 1, 0},
	{0, GeneralConfig::TYPE_TEXT,    FSSF_PRIVATE,           NKeySystem,L"ConsoleDetachKey", &strKeyNameConsoleDetachKey, 0, L"CtrlAltTab"},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"CopyBufferSize",&Opt.CMOpt.BufferSize,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_SYSTEM,            NKeySystem,L"CopyOpened",&Opt.CMOpt.CopyOpened,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"CopyTimeRule",  &Opt.CMOpt.CopyTimeRule, 3, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"CopySecurityOptions",&Opt.CMOpt.CopySecurityOptions,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"CreateUppercaseFolders",&Opt.CreateUppercaseFolders,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_SYSTEM,            NKeySystem,L"DeleteToRecycleBin",&Opt.DeleteToRecycleBin,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"DeleteToRecycleBinKillLink",&Opt.DeleteToRecycleBinKillLink,1, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"DelThreadPriority", &Opt.DelThreadPriority, THREAD_PRIORITY_NORMAL, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"DriveDisconnectMode",&Opt.ChangeDriveDisconnectMode,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"DriveMenuMode",&Opt.ChangeDriveMode,DRIVE_SHOW_TYPE|DRIVE_SHOW_PLUGINS|DRIVE_SHOW_SIZE_FLOAT|DRIVE_SHOW_CDROM, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"ElevationMode",&Opt.ElevationMode,0x0FFFFFFFU, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"ExcludeCmdHistory",&Opt.ExcludeCmdHistory,0, 0}, //AN
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"FileSearchMode",&Opt.FindOpt.FileSearchMode,FINDAREA_FROM_CURRENT, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"FindAlternateStreams",&Opt.FindOpt.FindAlternateStreams,0,0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"FindCodePage",&Opt.FindCodePage, CP_DEFAULT, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"FlagPosixSemantics", &Opt.FlagPosixSemantics, 1, 0},
	{1, GeneralConfig::TYPE_TEXT,    FSSF_PRIVATE,           NKeySystem,L"FolderInfo",&Opt.InfoPanel.strFolderInfoFiles, 0, L"DirInfo,File_Id.diz,Descript.ion,ReadMe.*,Read.Me"},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"MaxPositionCache",&Opt.MaxPositionCache, 512/*MAX_POSITIONS*/, 0}, //BUGBUG
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"MsWheelDelta", &Opt.MsWheelDelta, 1, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"MsWheelDeltaEdit", &Opt.MsWheelDeltaEdit, 1, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"MsWheelDeltaHelp", &Opt.MsWheelDeltaHelp, 1, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"MsWheelDeltaView", &Opt.MsWheelDeltaView, 1, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"MsHWheelDelta", &Opt.MsHWheelDelta, 1, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"MsHWheelDeltaEdit", &Opt.MsHWheelDeltaEdit, 1, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"MsHWheelDeltaView", &Opt.MsHWheelDeltaView, 1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"MultiCopy",&Opt.CMOpt.MultiCopy,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"MultiMakeDir",&Opt.MultiMakeDir,0, 0},
#ifndef NO_WRAPPER
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"OEMPluginsSupport",  &Opt.LoadPlug.OEMPluginsSupport, 1, 0},
#endif // NO_WRAPPER
	{0, GeneralConfig::TYPE_INTEGER, FSSF_SYSTEM,            NKeySystem,L"PluginMaxReadData",&Opt.PluginMaxReadData,0x20000, 0},
	#if 1
	//Maximus: ����������� ���� ��������
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"PlugMenuMode",&Opt.ChangePlugMenuMode, 0, 0},
	#endif
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"QuotedName",&Opt.QuotedName,0xFFFFFFFFU, 0},
	{0, GeneralConfig::TYPE_TEXT,    FSSF_PRIVATE,           NKeySystem,L"QuotedSymbols",&Opt.strQuotedSymbols, 0, L" &()[]{}^=;!'+,`\xA0"}, //xA0 => 160 =>oem(0xFF)
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"SaveHistory",&Opt.SaveHistory,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"SaveFoldersHistory",&Opt.SaveFoldersHistory,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"SaveViewHistory",&Opt.SaveViewHistory,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_SYSTEM,            NKeySystem,L"ScanJunction",&Opt.ScanJunction,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"ScanSymlinks",  &Opt.LoadPlug.ScanSymlinks, 1, 0},
	{1, GeneralConfig::TYPE_TEXT,    FSSF_PRIVATE,           NKeySystem,L"SearchInFirstSize",&Opt.FindOpt.strSearchInFirstSize, 0, L""},
	{1, GeneralConfig::TYPE_TEXT,    FSSF_PRIVATE,           NKeySystem,L"SearchOutFormat",&Opt.FindOpt.strSearchOutFormat, 0, L"D,S,A"},
	{1, GeneralConfig::TYPE_TEXT,    FSSF_PRIVATE,           NKeySystem,L"SearchOutFormatWidth",&Opt.FindOpt.strSearchOutFormatWidth, 0, L"14,13,0"},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"SetAttrFolderRules",&Opt.SetAttrFolderRules,1, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"ShowCheckingFile", &Opt.ShowCheckingFile, 0, 0},
	{1, GeneralConfig::TYPE_TEXT,    FSSF_PRIVATE,           NKeySystem,L"ShowStatusInfo",&Opt.InfoPanel.strShowStatusInfo, 0, L""},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"SilentLoadPlugin",  &Opt.LoadPlug.SilentLoadPlugin, 0, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"SubstNameRule", &Opt.SubstNameRule, 2, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"SubstPluginPrefix",&Opt.SubstPluginPrefix, 0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"UpdateEnvironment",&Opt.UpdateEnvironment,0,0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"UseFilterInSearch",&Opt.FindOpt.UseFilter,0,0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"UseRegisteredTypes",&Opt.UseRegisteredTypes,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"UseSystemCopy",&Opt.CMOpt.UseSystemCopy,1, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"WindowMode",&Opt.WindowMode, 0, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystem,L"WipeSymbol",&Opt.WipeSymbol,0, 0},

	{0, GeneralConfig::TYPE_TEXT,    FSSF_PRIVATE,           NKeySystemKnownIDs,L"EMenu",&Opt.KnownIDs.EmenuGuidStr, 0, L"742910F1-02ED-4542-851F-DEE37C2E13B2"},
	{0, GeneralConfig::TYPE_TEXT,    FSSF_PRIVATE,           NKeySystemKnownIDs,L"Network",&Opt.KnownIDs.NetworkGuidStr, 0, L"773B5051-7C5F-4920-A201-68051C4176A4"},

	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystemNowell,L"MoveRO",&Opt.Nowell.MoveRO,1, 0},

	{0, GeneralConfig::TYPE_TEXT,    FSSF_PRIVATE,           NKeySystemException,L"FarEventSvc",&Opt.strExceptEventSvc, 0, L""},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystemException,L"Used",&Opt.ExceptUsed, 0, 0},

	{1, GeneralConfig::TYPE_TEXT,    FSSF_PRIVATE,           NKeySystemExecutor,L"~",&Opt.Exec.strHomeDir,0,L"%FARHOME%"},
	{0, GeneralConfig::TYPE_TEXT,    FSSF_PRIVATE,           NKeySystemExecutor,L"BatchType",&Opt.Exec.strExecuteBatchType,0,constBatchExt},
	{0, GeneralConfig::TYPE_TEXT,    FSSF_PRIVATE,           NKeySystemExecutor,L"ExcludeCmds",&Opt.Exec.strExcludeCmds,0,L""},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystemExecutor,L"FullTitle",&Opt.Exec.ExecuteFullTitle,0, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystemExecutor,L"RestoreCP",&Opt.Exec.RestoreCPAfterExecute,1, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystemExecutor,L"SilentExternal",&Opt.Exec.ExecuteSilentExternal,0, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystemExecutor,L"UseAppPath",&Opt.Exec.ExecuteUseAppPath,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeySystemExecutor,L"UseHomeDir", &Opt.Exec.UseHomeDir,1, 0},

	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyViewer,L"AnsiCodePageAsDefault",&Opt.ViOpt.AnsiCodePageAsDefault,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyViewer,L"AutoDetectCodePage",&Opt.ViOpt.AutoDetectCodePage,1, 0},
	{1, GeneralConfig::TYPE_TEXT,    FSSF_PRIVATE,           NKeyViewer,L"ExternalViewerName",&Opt.strExternalViewer, 0, L""},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyViewer,L"IsWrap",&Opt.ViOpt.ViewerIsWrap,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyViewer,L"MaxLineSize",&Opt.ViOpt.MaxLineSize,ViewerOptions::eDefLineSize, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyViewer,L"PersistentBlocks",&Opt.ViOpt.PersistentBlocks,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyViewer,L"SaveViewerPos",&Opt.ViOpt.SavePos,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyViewer,L"SaveViewerShortPos",&Opt.ViOpt.SaveShortPos,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyViewer,L"SearchEditFocus",&Opt.ViOpt.SearchEditFocus,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyViewer,L"SearchRegexp",&Opt.ViOpt.SearchRegexp,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyViewer,L"ShowArrows",&Opt.ViOpt.ShowArrows,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyViewer,L"ShowKeyBar",&Opt.ViOpt.ShowKeyBar,1, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyViewer,L"ShowTitleBar",&Opt.ViOpt.ShowTitleBar,1, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyViewer,L"ShowScrollbar",&Opt.ViOpt.ShowScrollbar,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyViewer,L"TabSize",&Opt.ViOpt.TabSize,8, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyViewer,L"UseExternalViewer",&Opt.ViOpt.UseExternalViewer,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyViewer,L"Visible0x00",&Opt.ViOpt.Visible0x00,0, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyViewer,L"Wrap",&Opt.ViOpt.ViewerWrap,0, 0},
	{0, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyViewer,L"ZeroChar",&Opt.ViOpt.ZeroChar,0x00B7, 0}, // middle dot

	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyVMenu,L"LBtnClick",&Opt.VMenu.LBtnClick, VMENUCLICK_CANCEL, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyVMenu,L"MBtnClick",&Opt.VMenu.MBtnClick, VMENUCLICK_APPLY, 0},
	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyVMenu,L"RBtnClick",&Opt.VMenu.RBtnClick, VMENUCLICK_CANCEL, 0},

	{1, GeneralConfig::TYPE_INTEGER, FSSF_PRIVATE,           NKeyXLat,L"Flags",&Opt.XLat.Flags,(DWORD)XLAT_SWITCHKEYBLAYOUT|XLAT_CONVERTALLCMDLINE, 0},
	{1, GeneralConfig::TYPE_TEXT,    FSSF_PRIVATE,           NKeyXLat,L"Rules1",&Opt.XLat.Rules[0],0,L""},
	{1, GeneralConfig::TYPE_TEXT,    FSSF_PRIVATE,           NKeyXLat,L"Rules2",&Opt.XLat.Rules[1],0,L""},
	{1, GeneralConfig::TYPE_TEXT,    FSSF_PRIVATE,           NKeyXLat,L"Rules3",&Opt.XLat.Rules[2],0,L""},
	{1, GeneralConfig::TYPE_TEXT,    FSSF_PRIVATE,           NKeyXLat,L"Table1",&Opt.XLat.Table[0],0,L""},
	{1, GeneralConfig::TYPE_TEXT,    FSSF_PRIVATE,           NKeyXLat,L"Table2",&Opt.XLat.Table[1],0,L""},
	{1, GeneralConfig::TYPE_TEXT,    FSSF_PRIVATE,           NKeyXLat,L"WordDivForXlat",&Opt.XLat.strWordDivForXlat, 0,WordDivForXlat0},
};

bool GetConfigValue(const wchar_t *Key, const wchar_t *Name, string &strValue)
{
	for (size_t I=0; I < ARRAYSIZE(CFG); ++I)
	{
		if (!StrCmpI(CFG[I].KeyName,Key) && !StrCmpI(CFG[I].ValName,Name))
		{
			if(FSSF_PRIVATE==CFG[I].ApiRoot) break;
			switch (CFG[I].ValType)
			{
				case GeneralConfig::TYPE_INTEGER:
				{
					wchar_t str[128];
					_itow(*(DWORD *)CFG[I].ValPtr, str, 10);
					strValue = str;
					break;
				}
				case GeneralConfig::TYPE_TEXT:
					strValue = *(string *)CFG[I].ValPtr;
					break;
				case GeneralConfig::TYPE_BLOB:
				{
					return false;
				}
			}
			return true;
		}
	}

	return false;
}

bool GetConfigValue(size_t Root,const wchar_t* Name,DWORD& Type,void*& Data)
{
	if(FSSF_PRIVATE!=Root)
	{
		for(size_t ii=0;ii<ARRAYSIZE(CFG);++ii)
		{
			if(Root==CFG[ii].ApiRoot&&!StrCmpI(CFG[ii].ValName,Name))
			{
				Type=CFG[ii].ValType;
				Data=CFG[ii].ValPtr;
				return true;
			}
		}
	}
	return false;
}

void ReadConfig()
{
	/* <�����������> *************************************************** */
	string strGlobalUserMenuDir;
	strGlobalUserMenuDir.ReleaseBuffer(GetPrivateProfileString(L"General", L"GlobalUserMenuDir", g_strFarPath, strGlobalUserMenuDir.GetBuffer(NT_MAX_PATH), NT_MAX_PATH, g_strFarINI));
	apiExpandEnvironmentStrings(strGlobalUserMenuDir, Opt.GlobalUserMenuDir);
	ConvertNameToFull(Opt.GlobalUserMenuDir,Opt.GlobalUserMenuDir);
	AddEndSlash(Opt.GlobalUserMenuDir);

	/* BUGBUG??
	SetRegRootKey(HKEY_LOCAL_MACHINE);
	DWORD OptPolicies_ShowHiddenDrives=GetRegKey(NKeyPolicies,L"ShowHiddenDrives",1)&1;
	DWORD OptPolicies_DisabledOptions=GetRegKey(NKeyPolicies,L"DisabledOptions",0);
	SetRegRootKey(HKEY_CURRENT_USER);
	*/

	GeneralCfg->GetValue(NKeyLanguage, L"Help", Opt.strHelpLanguage, Opt.strLanguage);

	bool ExplicitWindowMode=Opt.WindowMode!=FALSE;
	/* *************************************************** </�����������> */

	Opt.Palette.Load();

	for (size_t I=0; I < ARRAYSIZE(CFG); ++I)
	{
		switch (CFG[I].ValType)
		{
			case GeneralConfig::TYPE_INTEGER:
				GeneralCfg->GetValue(CFG[I].KeyName, CFG[I].ValName,(DWORD *)CFG[I].ValPtr,(DWORD)CFG[I].DefDWord);
				break;
			case GeneralConfig::TYPE_TEXT:
				GeneralCfg->GetValue(CFG[I].KeyName, CFG[I].ValName,*reinterpret_cast<string*>(CFG[I].ValPtr), reinterpret_cast<const wchar_t*>(CFG[I].DefStr));
				break;
			case GeneralConfig::TYPE_BLOB:
				int Size=GeneralCfg->GetValue(CFG[I].KeyName, CFG[I].ValName,(char *)CFG[I].ValPtr,(int)CFG[I].DefDWord,(const char *)CFG[I].DefStr);

				if (Size && Size < (int)CFG[I].DefDWord)
					memset(((BYTE*)CFG[I].ValPtr)+Size,0,CFG[I].DefDWord-Size);

				break;
		}
	}

	/* <������������> *************************************************** */

	Opt.CurrentElevationMode = Opt.ElevationMode;

	if (Opt.ShowMenuBar)
		Opt.ShowMenuBar=1;

	if (Opt.PluginMaxReadData < 0x1000) // || Opt.PluginMaxReadData > 0x80000)
		Opt.PluginMaxReadData=0x20000;

	if(ExplicitWindowMode)
	{
		Opt.WindowMode=TRUE;
	}

	Opt.HelpTabSize=8; // ���� ������ ��������...

	Opt.ViOpt.ViewerIsWrap &= 1;
	Opt.ViOpt.ViewerWrap &= 1;
	if (!Opt.ViOpt.MaxLineSize)
		Opt.ViOpt.MaxLineSize = ViewerOptions::eDefLineSize;
	else if (Opt.ViOpt.MaxLineSize < ViewerOptions::eMinLineSize)
		Opt.ViOpt.MaxLineSize = ViewerOptions::eMinLineSize;
	else if (Opt.ViOpt.MaxLineSize > ViewerOptions::eMaxLineSize)
		Opt.ViOpt.MaxLineSize = ViewerOptions::eMaxLineSize;
	Opt.ViOpt.SearchEditFocus &= 1;
	Opt.ViOpt.Visible0x00 &= 1;

	// ��������� ��������� �������� ������������ ;-)
	if (Opt.strWordDiv.IsEmpty())
		Opt.strWordDiv = WordDiv0;

	// ��������� ��������� �������� ������������
	if (Opt.XLat.strWordDivForXlat.IsEmpty())
		Opt.XLat.strWordDivForXlat = WordDivForXlat0;

	Opt.PanelRightClickRule%=3;
	Opt.PanelCtrlAltShiftRule%=3;
	Opt.ConsoleDetachKey=KeyNameToKey(strKeyNameConsoleDetachKey);

	if (Opt.EdOpt.TabSize<1 || Opt.EdOpt.TabSize>512)
		Opt.EdOpt.TabSize=8;

	if (Opt.ViOpt.TabSize<1 || Opt.ViOpt.TabSize>512)
		Opt.ViOpt.TabSize=8;

	string strKeyNameFromCfg;

	GeneralCfg->GetValue(NKeyKeyMacros,L"KeyRecordCtrlDot",strKeyNameFromCfg,szCtrlDot);
	if ((Opt.Macro.KeyMacroCtrlDot=KeyNameToKey(strKeyNameFromCfg)) == (DWORD)-1)
		Opt.Macro.KeyMacroCtrlDot=KEY_CTRLDOT;

	GeneralCfg->GetValue(NKeyKeyMacros,L"KeyRecordRCtrlDot",strKeyNameFromCfg,szRCtrlDot);
	if ((Opt.Macro.KeyMacroRCtrlDot=KeyNameToKey(strKeyNameFromCfg)) == (DWORD)-1)
		Opt.Macro.KeyMacroRCtrlDot=KEY_RCTRLDOT;

	GeneralCfg->GetValue(NKeyKeyMacros,L"KeyRecordCtrlShiftDot",strKeyNameFromCfg,szCtrlShiftDot);
	if ((Opt.Macro.KeyMacroCtrlShiftDot=KeyNameToKey(strKeyNameFromCfg)) == (DWORD)-1)
		Opt.Macro.KeyMacroCtrlShiftDot=KEY_CTRLSHIFTDOT;

	GeneralCfg->GetValue(NKeyKeyMacros,L"KeyRecordRCtrlShiftDot",strKeyNameFromCfg,szRCtrlShiftDot);
	if ((Opt.Macro.KeyMacroRCtrlShiftDot=KeyNameToKey(strKeyNameFromCfg)) == (DWORD)-1)
		Opt.Macro.KeyMacroRCtrlShiftDot=KEY_RCTRL|KEY_SHIFT|KEY_DOT;

	Opt.EdOpt.strWordDiv = Opt.strWordDiv;
	FileList::ReadPanelModes();

	/* BUGBUG??
	// �������� ��������� ��������
	// ��� ������ ���� ����� ������ �������� �����
	Opt.Policies.ShowHiddenDrives&=OptPolicies_ShowHiddenDrives;
	// ��� ����� ���� ����� ������ ��������� ��������� �������
	Opt.Policies.DisabledOptions|=OptPolicies_DisabledOptions;
	*/

	if (Opt.Exec.strExecuteBatchType.IsEmpty()) // ��������������
		Opt.Exec.strExecuteBatchType=constBatchExt;

	// ������������� XLat ��� ������� ��������� qwerty<->������
	if (Opt.XLat.Table[0].IsEmpty())
	{
		bool RussianExists=false;
		HKL Layouts[32];
		UINT Count=GetKeyboardLayoutList(ARRAYSIZE(Layouts), Layouts);
		WORD RussianLanguageId=MAKELANGID(LANG_RUSSIAN, SUBLANG_RUSSIAN_RUSSIA);
		for (UINT I=0; I<Count; I++)
		{
			if (LOWORD(Layouts[I]) == RussianLanguageId)
			{
				RussianExists = true;
				break;
			}
		}

		if (RussianExists)
		{
			Opt.XLat.Table[0] = L"\x2116\x0410\x0412\x0413\x0414\x0415\x0417\x0418\x0419\x041a\x041b\x041c\x041d\x041e\x041f\x0420\x0421\x0422\x0423\x0424\x0425\x0426\x0427\x0428\x0429\x042a\x042b\x042c\x042f\x0430\x0432\x0433\x0434\x0435\x0437\x0438\x0439\x043a\x043b\x043c\x043d\x043e\x043f\x0440\x0441\x0442\x0443\x0444\x0445\x0446\x0447\x0448\x0449\x044a\x044b\x044c\x044d\x044f\x0451\x0401\x0411\x042e";
			Opt.XLat.Table[1] = L"#FDULTPBQRKVYJGHCNEA{WXIO}SMZfdultpbqrkvyjghcnea[wxio]sm'z`~<>";
			Opt.XLat.Rules[0] = L",??&./\x0431,\x044e.:^\x0416:\x0436;;$\"@\x042d\"";
			Opt.XLat.Rules[1] = L"?,&?/.,\x0431.\x044e^::\x0416;\x0436$;@\"\"\x042d";
			Opt.XLat.Rules[2] = L"^::\x0416\x0416^$;;\x0436\x0436$@\"\"\x042d\x042d@&??,,\x0431\x0431&/..\x044e\x044e/";
		}
	}

	{
		Opt.XLat.CurrentLayout=0;
		ClearArray(Opt.XLat.Layouts);
		string strXLatLayouts;
		GeneralCfg->GetValue(NKeyXLat,L"Layouts",strXLatLayouts,L"");

		if (!strXLatLayouts.IsEmpty())
		{
			wchar_t *endptr;
			const wchar_t *ValPtr;
			UserDefinedList DestList;
			DestList.SetParameters(L';',0,ULF_UNIQUE);
			DestList.Set(strXLatLayouts);
			size_t I=0;

			while (nullptr!=(ValPtr=DestList.GetNext()))
			{
				DWORD res=(DWORD)wcstoul(ValPtr, &endptr, 16);
				Opt.XLat.Layouts[I]=(HKL)(INT_PTR)(HIWORD(res)? res : MAKELONG(res,res));
				++I;

				if (I >= ARRAYSIZE(Opt.XLat.Layouts))
					break;
			}

			if (I <= 1) // ���� ������� ������ ���� - "����������" ���
				Opt.XLat.Layouts[0]=0;
		}
	}

	ClearArray(Opt.FindOpt.OutColumnTypes);
	ClearArray(Opt.FindOpt.OutColumnWidths);
	ClearArray(Opt.FindOpt.OutColumnWidthType);
	Opt.FindOpt.OutColumnCount=0;


	if (!Opt.FindOpt.strSearchOutFormat.IsEmpty())
	{
		if (Opt.FindOpt.strSearchOutFormatWidth.IsEmpty())
			Opt.FindOpt.strSearchOutFormatWidth=L"0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0";
		TextToViewSettings(Opt.FindOpt.strSearchOutFormat.CPtr(),Opt.FindOpt.strSearchOutFormatWidth.CPtr(),false,
                                  Opt.FindOpt.OutColumnTypes,Opt.FindOpt.OutColumnWidths,Opt.FindOpt.OutColumnWidthType,
                                  Opt.FindOpt.OutColumnCount);
	}

	string tmp[2];
	if (!GeneralCfg->EnumValues(L"Masks", 0, tmp[0], tmp[1]))
	{
		ApplyDefaultMaskGroups();
	}

	StrToGuid(Opt.KnownIDs.EmenuGuidStr, Opt.KnownIDs.Emenu);
	StrToGuid(Opt.KnownIDs.NetworkGuidStr, Opt.KnownIDs.Network);

/* *************************************************** </������������> */
}


void SaveConfig(int Ask)
{
	if (Opt.Policies.DisabledOptions&0x20000) // Bit 17 - ��������� ���������
		return;

	if (Ask && Message(0,2,MSG(MSaveSetupTitle),MSG(MSaveSetupAsk1),MSG(MSaveSetupAsk2),MSG(MSaveSetup),MSG(MCancel)))
		return;

	string strTemp;
	/* <�����������> *************************************************** */
	Panel *LeftPanel=CtrlObject->Cp()->LeftPanel;
	Panel *RightPanel=CtrlObject->Cp()->RightPanel;
	Opt.LeftPanel.Focus=LeftPanel->GetFocus();
	Opt.LeftPanel.Visible=LeftPanel->IsVisible();
	Opt.RightPanel.Focus=RightPanel->GetFocus();
	Opt.RightPanel.Visible=RightPanel->IsVisible();

	if (LeftPanel->GetMode()==NORMAL_PANEL)
	{
		Opt.LeftPanel.Type=LeftPanel->GetType();
		Opt.LeftPanel.ViewMode=LeftPanel->GetViewMode();
		Opt.LeftPanel.SortMode=LeftPanel->GetSortMode();
		Opt.LeftPanel.SortOrder=LeftPanel->GetSortOrder();
		Opt.LeftPanel.SortGroups=LeftPanel->GetSortGroups();
		Opt.LeftPanel.ShowShortNames=LeftPanel->GetShowShortNamesMode();
		Opt.LeftPanel.NumericSort=LeftPanel->GetNumericSort();
		Opt.LeftPanel.CaseSensitiveSort=LeftPanel->GetCaseSensitiveSort();
		Opt.LeftSelectedFirst=LeftPanel->GetSelectedFirstMode();
		Opt.LeftPanel.DirectoriesFirst=LeftPanel->GetDirectoriesFirst();
	}

	LeftPanel->GetCurDir(Opt.strLeftFolder);
	LeftPanel->GetCurBaseName(Opt.strLeftCurFile, strTemp);

	if (RightPanel->GetMode()==NORMAL_PANEL)
	{
		Opt.RightPanel.Type=RightPanel->GetType();
		Opt.RightPanel.ViewMode=RightPanel->GetViewMode();
		Opt.RightPanel.SortMode=RightPanel->GetSortMode();
		Opt.RightPanel.SortOrder=RightPanel->GetSortOrder();
		Opt.RightPanel.SortGroups=RightPanel->GetSortGroups();
		Opt.RightPanel.ShowShortNames=RightPanel->GetShowShortNamesMode();
		Opt.RightPanel.NumericSort=RightPanel->GetNumericSort();
		Opt.RightPanel.CaseSensitiveSort=RightPanel->GetCaseSensitiveSort();
		Opt.RightSelectedFirst=RightPanel->GetSelectedFirstMode();
		Opt.RightPanel.DirectoriesFirst=RightPanel->GetDirectoriesFirst();
	}

	RightPanel->GetCurDir(Opt.strRightFolder);
	RightPanel->GetCurBaseName(Opt.strRightCurFile,strTemp);
	CtrlObject->HiFiles->SaveHiData();
	/* *************************************************** </�����������> */

	Opt.Palette.Save();

	GeneralCfg->BeginTransaction();

	GeneralCfg->SetValue(NKeyLanguage,L"Main",Opt.strLanguage);
	GeneralCfg->SetValue(NKeyLanguage, L"Help", Opt.strHelpLanguage);

	for (size_t I=0; I < ARRAYSIZE(CFG); ++I)
	{
		if (CFG[I].IsSave)
			switch (CFG[I].ValType)
			{
				case GeneralConfig::TYPE_INTEGER:
					GeneralCfg->SetValue(CFG[I].KeyName, CFG[I].ValName,*(int *)CFG[I].ValPtr);
					break;
				case GeneralConfig::TYPE_TEXT:
					GeneralCfg->SetValue(CFG[I].KeyName, CFG[I].ValName,*(string *)CFG[I].ValPtr);
					break;
				case GeneralConfig::TYPE_BLOB:
					GeneralCfg->SetValue(CFG[I].KeyName, CFG[I].ValName,(char*)CFG[I].ValPtr,CFG[I].DefDWord);
					break;
			}
	}

	GeneralCfg->EndTransaction();

	/* <������������> *************************************************** */
	FileFilter::SaveFilters();
	FileList::SavePanelModes();

	if (Ask)
		CtrlObject->Macro.SaveMacros();

	/* *************************************************** </������������> */
}

inline const wchar_t* TypeToText(size_t Type)
{
	switch(Type)
	{
	case GeneralConfig::TYPE_INTEGER:
		return L"Integer";
	case GeneralConfig::TYPE_TEXT:
		return L"String";
	case GeneralConfig::TYPE_BLOB:
		return L"Blob";
	case GeneralConfig::TYPE_UNKNOWN:
	default:
		return L"Unknown";
	}
}

void FillListItem(FarListItem& Item, FormatString& fs, FARConfig& cfg)
{
	Item.Flags = 0;
	Item.Reserved[0] = Item.Reserved[1] = Item.Reserved[2] = 0;
	fs.Clear();
	fs << fmt::Width(42) << fmt::Precision(42) << fmt::LeftAlign() << (string(cfg.KeyName) + "." + cfg.ValName) << BoxSymbols[BS_V1]
	<< fmt::Width(7) << fmt::Precision(7) << fmt::LeftAlign() << TypeToText(cfg.ValType) << BoxSymbols[BS_V1];
	bool Changed = false;
	switch(cfg.ValType)
	{
	case GeneralConfig::TYPE_INTEGER:
		fs << *static_cast<int*>(cfg.ValPtr);
		Changed = *static_cast<int*>(cfg.ValPtr) != static_cast<int>(cfg.DefDWord);
		break;
	case GeneralConfig::TYPE_TEXT:
		fs << *static_cast<string*>(cfg.ValPtr);
		Changed = *static_cast<string*>(cfg.ValPtr) != static_cast<const wchar_t*>(cfg.DefStr);
		break;
	case GeneralConfig::TYPE_BLOB:
	case GeneralConfig::TYPE_UNKNOWN:
	default:
		fs << BlobToHexString(cfg.ValPtr, cfg.DefDWord);
		Changed = !memcmp(cfg.ValPtr, cfg.DefStr, cfg.DefDWord);
	}
	if(Changed)
	{
		Item.Flags = LIF_CHECKED|L'*';
	}
	Item.Text = fs;
}

INT_PTR WINAPI AdvancedConfigDlgProc(HANDLE hDlg, int Msg, int Param1, void* Param2)
{
	static FormatString* fs;
	switch (Msg)
	{
	case DN_INITDIALOG:
		fs = reinterpret_cast<FormatString*>(Param2);
		break;

	case DN_RESIZECONSOLE:
		{
			COORD Size = {Max(ScrX-4, 60), Max(ScrY-2, 20)};
			SendDlgMessage(hDlg, DM_RESIZEDIALOG, 0, &Size);
			SMALL_RECT ListPos = {3, 1, Size.X-4, Size.Y-2};
			SendDlgMessage(hDlg, DM_SETITEMPOSITION, 0, &ListPos);
		}
		break;

	case DN_CLOSE:
		if (Param1 == 0) // BUGBUG, magic
		{
			FarListInfo ListInfo = {sizeof(ListInfo)};
			SendDlgMessage(hDlg, DM_LISTINFO, Param1, &ListInfo);

			DialogBuilder Builder;
			Builder.AddText(string(CFG[ListInfo.SelectPos].KeyName) + "." + CFG[ListInfo.SelectPos].ValName + L" (" + TypeToText(CFG[ListInfo.SelectPos].ValType) + L"):");
			FormatString fstr;
			bool ReadOnly = false;
			switch(CFG[ListInfo.SelectPos].ValType)
			{
			case GeneralConfig::TYPE_INTEGER:
				fstr << *static_cast<int*>(CFG[ListInfo.SelectPos].ValPtr);
				break;
			case GeneralConfig::TYPE_TEXT:
				fstr << *static_cast<string*>(CFG[ListInfo.SelectPos].ValPtr);
				break;
			case GeneralConfig::TYPE_BLOB:
			case GeneralConfig::TYPE_UNKNOWN:
			default:
				fstr << BlobToHexString(CFG[ListInfo.SelectPos].ValPtr, CFG[ListInfo.SelectPos].DefDWord);
				ReadOnly = true;
			}
			DialogItemEx* Edit = Builder.AddEditField(&fstr, 40);
			Edit->Flags = ReadOnly? DIF_DISABLE : 0;
			((DialogBuilderBase<DialogItemEx>*)&Builder)->AddOKCancel(MOk, MConfigResetValue, MCancel);
			int Result = Builder.ShowDialogEx();
			if(Result == 0 || Result == 1)
			{
				switch(CFG[ListInfo.SelectPos].ValType)
				{
				case GeneralConfig::TYPE_INTEGER:
					*static_cast<int*>(CFG[ListInfo.SelectPos].ValPtr) = Result? CFG[ListInfo.SelectPos].DefDWord : wcstol(fstr, nullptr, 10);
					CFG[ListInfo.SelectPos].IsSave = true;
					break;
				case GeneralConfig::TYPE_TEXT:
					*static_cast<string*>(CFG[ListInfo.SelectPos].ValPtr) = Result? static_cast<const wchar_t*>(CFG[ListInfo.SelectPos].DefStr) : fstr;
					CFG[ListInfo.SelectPos].IsSave = true;
					break;
				case GeneralConfig::TYPE_BLOB:
				case GeneralConfig::TYPE_UNKNOWN:
				default:
					// blob edit not implemented
					;
				}
				SendDlgMessage(hDlg, DM_ENABLEREDRAW, 0 , 0);
				FarListUpdate flu = {sizeof(flu), ListInfo.SelectPos};
				FillListItem(flu.Item, fs[ListInfo.SelectPos], CFG[ListInfo.SelectPos]);
				SendDlgMessage(hDlg, DM_LISTUPDATE, Param1, &flu);
				FarListPos flp = {sizeof(flp), ListInfo.SelectPos, ListInfo.TopPos};
				SendDlgMessage(hDlg, DM_LISTSETCURPOS, Param1, &flp);
				SendDlgMessage(hDlg, DM_ENABLEREDRAW, 1 , 0);
			}
			return FALSE;
		}

		break;
	default:
		break;
	}

	return DefDlgProc(hDlg,Msg,Param1,Param2);
}

bool AdvancedConfig()
{
	int DlgWidth = Max(ScrX-4, 60), DlgHeight = Max(ScrY-2, 20);
	FarDialogItem AdvancedConfigDlgData[]=
	{
		{DI_LISTBOX,3,1,DlgWidth-4,DlgHeight-2,0,nullptr,nullptr,DIF_NONE,nullptr},
	};
	MakeDialogItemsEx(AdvancedConfigDlgData,AdvancedConfigDlg);

	FarList Items;
	Items.ItemsNumber = ARRAYSIZE(CFG);
	Items.Items = new FarListItem[Items.ItemsNumber];

	FormatString fs[ARRAYSIZE(CFG)];
	for(size_t i = 0; i < Items.ItemsNumber; ++i)
	{
		FillListItem(Items.Items[i], fs[i], CFG[i]);
	}

	AdvancedConfigDlg[0].ListItems = &Items;

	Dialog Dlg(AdvancedConfigDlg,ARRAYSIZE(AdvancedConfigDlg), AdvancedConfigDlgProc, &fs);
	//Dlg.SetHelp(L"");
	Dlg.SetPosition(-1, -1, DlgWidth, DlgHeight);
	Dlg.Process();
	delete[] Items.Items;
	return true;
}
