
/*
macro.cpp

�������
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

#include "macro.hpp"
#include "macroopcode.hpp"
#include "keys.hpp"
#include "keyboard.hpp"
#include "lang.hpp"
#include "lockscrn.hpp"
#include "viewer.hpp"
#include "fileedit.hpp"
#include "fileview.hpp"
#include "dialog.hpp"
#include "dlgedit.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "cmdline.hpp"
#include "manager.hpp"
#include "scrbuf.hpp"
#include "udlist.hpp"
#include "filelist.hpp"
#include "treelist.hpp"
#include "flink.hpp"
#include "TStack.hpp"
#include "syslog.hpp"
#include "registry.hpp"
#include "plugapi.hpp"
#include "plugin.hpp"
#include "plugins.hpp"
#include "cddrv.hpp"
#include "interf.hpp"
#include "grabber.hpp"
#include "message.hpp"
#include "clipboard.hpp"
#include "xlat.hpp"
#include "datetime.hpp"
#include "stddlg.hpp"
#include "pathmix.hpp"
#include "drivemix.hpp"
#include "strmix.hpp"
#include "panelmix.hpp"
#include "constitle.hpp"
#include "dirmix.hpp"
#include "console.hpp"
#include "imports.hpp"
#include "CFileMask.hpp"
#include "vmenu.hpp"
#include "elevation.hpp"

// ��� ������� ���������� �������
struct DlgParam
{
	KeyMacro *Handle;
	DWORD Key;
	int Mode;
	int Recurse;
};

TMacroKeywords MKeywords[] =
{
	{0,  L"Other",              MCODE_C_AREA_OTHER,0},
	{0,  L"Shell",              MCODE_C_AREA_SHELL,0},
	{0,  L"Viewer",             MCODE_C_AREA_VIEWER,0},
	{0,  L"Editor",             MCODE_C_AREA_EDITOR,0},
	{0,  L"Dialog",             MCODE_C_AREA_DIALOG,0},
	{0,  L"Search",             MCODE_C_AREA_SEARCH,0},
	{0,  L"Disks",              MCODE_C_AREA_DISKS,0},
	{0,  L"MainMenu",           MCODE_C_AREA_MAINMENU,0},
	{0,  L"Menu",               MCODE_C_AREA_MENU,0},
	{0,  L"Help",               MCODE_C_AREA_HELP,0},
	{0,  L"Info",               MCODE_C_AREA_INFOPANEL,0},
	{0,  L"QView",              MCODE_C_AREA_QVIEWPANEL,0},
	{0,  L"Tree",               MCODE_C_AREA_TREEPANEL,0},
	{0,  L"FindFolder",         MCODE_C_AREA_FINDFOLDER,0},
	{0,  L"UserMenu",           MCODE_C_AREA_USERMENU,0},
	{0,  L"AutoCompletion",     MCODE_C_AREA_AUTOCOMPLETION,0},

	// ������
	{2,  L"Bof",                MCODE_C_BOF,0},
	{2,  L"Eof",                MCODE_C_EOF,0},
	{2,  L"Empty",              MCODE_C_EMPTY,0},
	{2,  L"Selected",           MCODE_C_SELECTED,0},

	{2,  L"Far.Width",          MCODE_V_FAR_WIDTH,0},
	{2,  L"Far.Height",         MCODE_V_FAR_HEIGHT,0},
	{2,  L"Far.Title",          MCODE_V_FAR_TITLE,0},
	{2,  L"Far.UpTime",         MCODE_V_FAR_UPTIME,0},
	{2,  L"MacroArea",          MCODE_V_MACROAREA,0},

	{2,  L"ItemCount",          MCODE_V_ITEMCOUNT,0},  // ItemCount - ����� ��������� � ������� �������
	{2,  L"CurPos",             MCODE_V_CURPOS,0},    // CurPos - ������� ������ � ������� �������
	{2,  L"Title",              MCODE_V_TITLE,0},
	{2,  L"Height",             MCODE_V_HEIGHT,0},
	{2,  L"Width",              MCODE_V_WIDTH,0},

	{2,  L"APanel.Empty",       MCODE_C_APANEL_ISEMPTY,0},
	{2,  L"PPanel.Empty",       MCODE_C_PPANEL_ISEMPTY,0},
	{2,  L"APanel.Bof",         MCODE_C_APANEL_BOF,0},
	{2,  L"PPanel.Bof",         MCODE_C_PPANEL_BOF,0},
	{2,  L"APanel.Eof",         MCODE_C_APANEL_EOF,0},
	{2,  L"PPanel.Eof",         MCODE_C_PPANEL_EOF,0},
	{2,  L"APanel.Root",        MCODE_C_APANEL_ROOT,0},
	{2,  L"PPanel.Root",        MCODE_C_PPANEL_ROOT,0},
	{2,  L"APanel.Visible",     MCODE_C_APANEL_VISIBLE,0},
	{2,  L"PPanel.Visible",     MCODE_C_PPANEL_VISIBLE,0},
	{2,  L"APanel.Plugin",      MCODE_C_APANEL_PLUGIN,0},
	{2,  L"PPanel.Plugin",      MCODE_C_PPANEL_PLUGIN,0},
	{2,  L"APanel.FilePanel",   MCODE_C_APANEL_FILEPANEL,0},
	{2,  L"PPanel.FilePanel",   MCODE_C_PPANEL_FILEPANEL,0},
	{2,  L"APanel.Folder",      MCODE_C_APANEL_FOLDER,0},
	{2,  L"PPanel.Folder",      MCODE_C_PPANEL_FOLDER,0},
	{2,  L"APanel.Selected",    MCODE_C_APANEL_SELECTED,0},
	{2,  L"PPanel.Selected",    MCODE_C_PPANEL_SELECTED,0},
	{2,  L"APanel.Left",        MCODE_C_APANEL_LEFT,0},
	{2,  L"PPanel.Left",        MCODE_C_PPANEL_LEFT,0},
	{2,  L"APanel.LFN",         MCODE_C_APANEL_LFN,0},
	{2,  L"PPanel.LFN",         MCODE_C_PPANEL_LFN,0},
	{2,  L"APanel.Filter",      MCODE_C_APANEL_FILTER,0},
	{2,  L"PPanel.Filter",      MCODE_C_PPANEL_FILTER,0},

	{2,  L"APanel.Type",        MCODE_V_APANEL_TYPE,0},
	{2,  L"PPanel.Type",        MCODE_V_PPANEL_TYPE,0},
	{2,  L"APanel.ItemCount",   MCODE_V_APANEL_ITEMCOUNT,0},
	{2,  L"PPanel.ItemCount",   MCODE_V_PPANEL_ITEMCOUNT,0},
	{2,  L"APanel.CurPos",      MCODE_V_APANEL_CURPOS,0},
	{2,  L"PPanel.CurPos",      MCODE_V_PPANEL_CURPOS,0},
	{2,  L"APanel.Current",     MCODE_V_APANEL_CURRENT,0},
	{2,  L"PPanel.Current",     MCODE_V_PPANEL_CURRENT,0},
	{2,  L"APanel.SelCount",    MCODE_V_APANEL_SELCOUNT,0},
	{2,  L"PPanel.SelCount",    MCODE_V_PPANEL_SELCOUNT,0},
	{2,  L"APanel.Path",        MCODE_V_APANEL_PATH,0},
	{2,  L"PPanel.Path",        MCODE_V_PPANEL_PATH,0},
	{2,  L"APanel.Path0",       MCODE_V_APANEL_PATH0,0},
	{2,  L"PPanel.Path0",       MCODE_V_PPANEL_PATH0,0},
	{2,  L"APanel.UNCPath",     MCODE_V_APANEL_UNCPATH,0},
	{2,  L"PPanel.UNCPath",     MCODE_V_PPANEL_UNCPATH,0},
	{2,  L"APanel.Height",      MCODE_V_APANEL_HEIGHT,0},
	{2,  L"PPanel.Height",      MCODE_V_PPANEL_HEIGHT,0},
	{2,  L"APanel.StatusHeight",MCODE_V_APANEL_STATUSHEIGHT,0},
	{2,  L"PPanel.StatusHeight",MCODE_V_PPANEL_STATUSHEIGHT,0},
	{2,  L"APanel.Width",       MCODE_V_APANEL_WIDTH,0},
	{2,  L"PPanel.Width",       MCODE_V_PPANEL_WIDTH,0},
	{2,  L"APanel.OPIFlags",    MCODE_V_APANEL_OPIFLAGS,0},
	{2,  L"PPanel.OPIFlags",    MCODE_V_PPANEL_OPIFLAGS,0},
	{2,  L"APanel.DriveType",   MCODE_V_APANEL_DRIVETYPE,0}, // APanel.DriveType - �������� ������: ��� �������
	{2,  L"PPanel.DriveType",   MCODE_V_PPANEL_DRIVETYPE,0}, // PPanel.DriveType - ��������� ������: ��� �������
	{2,  L"APanel.ColumnCount", MCODE_V_APANEL_COLUMNCOUNT,0}, // APanel.ColumnCount - �������� ������:  ���������� �������
	{2,  L"PPanel.ColumnCount", MCODE_V_PPANEL_COLUMNCOUNT,0}, // PPanel.ColumnCount - ��������� ������: ���������� �������
	{2,  L"APanel.HostFile",    MCODE_V_APANEL_HOSTFILE,0},
	{2,  L"PPanel.HostFile",    MCODE_V_PPANEL_HOSTFILE,0},
	{2,  L"APanel.Prefix",      MCODE_V_APANEL_PREFIX,0},
	{2,  L"PPanel.Prefix",      MCODE_V_PPANEL_PREFIX,0},

	{2,  L"CmdLine.Bof",        MCODE_C_CMDLINE_BOF,0}, // ������ � ������ cmd-������ ��������������?
	{2,  L"CmdLine.Eof",        MCODE_C_CMDLINE_EOF,0}, // ������ � ������ cmd-������ ��������������?
	{2,  L"CmdLine.Empty",      MCODE_C_CMDLINE_EMPTY,0},
	{2,  L"CmdLine.Selected",   MCODE_C_CMDLINE_SELECTED,0},
	{2,  L"CmdLine.ItemCount",  MCODE_V_CMDLINE_ITEMCOUNT,0},
	{2,  L"CmdLine.CurPos",     MCODE_V_CMDLINE_CURPOS,0},
	{2,  L"CmdLine.Value",      MCODE_V_CMDLINE_VALUE,0},

	{2,  L"Editor.FileName",    MCODE_V_EDITORFILENAME,0},
	{2,  L"Editor.CurLine",     MCODE_V_EDITORCURLINE,0},  // ������� ����� � ��������� (� ���������� � Count)
	{2,  L"Editor.Lines",       MCODE_V_EDITORLINES,0},
	{2,  L"Editor.CurPos",      MCODE_V_EDITORCURPOS,0},
	{2,  L"Editor.RealPos",     MCODE_V_EDITORREALPOS,0},
	{2,  L"Editor.State",       MCODE_V_EDITORSTATE,0},
	{2,  L"Editor.Value",       MCODE_V_EDITORVALUE,0},
	{2,  L"Editor.SelValue",    MCODE_V_EDITORSELVALUE,0},

	{2,  L"Dlg.ItemType",       MCODE_V_DLGITEMTYPE,0},
	{2,  L"Dlg.ItemCount",      MCODE_V_DLGITEMCOUNT,0},
	{2,  L"Dlg.CurPos",         MCODE_V_DLGCURPOS,0},
	{2,  L"Dlg.Info.Id",        MCODE_V_DLGINFOID,0},

	{2,  L"Help.FileName",      MCODE_V_HELPFILENAME, 0},
	{2,  L"Help.Topic",         MCODE_V_HELPTOPIC, 0},
	{2,  L"Help.SelTopic",      MCODE_V_HELPSELTOPIC, 0},

	{2,  L"Drv.ShowPos",        MCODE_V_DRVSHOWPOS,0},
	{2,  L"Drv.ShowMode",       MCODE_V_DRVSHOWMODE,0},

	{2,  L"Viewer.FileName",    MCODE_V_VIEWERFILENAME,0},
	{2,  L"Viewer.State",       MCODE_V_VIEWERSTATE,0},

	{2,  L"Menu.Value",         MCODE_V_MENU_VALUE,0},

	{2,  L"Fullscreen",         MCODE_C_FULLSCREENMODE,0},
	{2,  L"IsUserAdmin",        MCODE_C_ISUSERADMIN,0},
};

TMacroKeywords MKeywordsArea[] =
{
	{0,  L"Funcs",              MACRO_FUNCS,0},
	{0,  L"Consts",             MACRO_CONSTS,0},
	{0,  L"Vars",               MACRO_VARS,0},
	{0,  L"Other",              MACRO_OTHER,0},
	{0,  L"Shell",              MACRO_SHELL,0},
	{0,  L"Viewer",             MACRO_VIEWER,0},
	{0,  L"Editor",             MACRO_EDITOR,0},
	{0,  L"Dialog",             MACRO_DIALOG,0},
	{0,  L"Search",             MACRO_SEARCH,0},
	{0,  L"Disks",              MACRO_DISKS,0},
	{0,  L"MainMenu",           MACRO_MAINMENU,0},
	{0,  L"Menu",               MACRO_MENU,0},
	{0,  L"Help",               MACRO_HELP,0},
	{0,  L"Info",               MACRO_INFOPANEL,0},
	{0,  L"QView",              MACRO_QVIEWPANEL,0},
	{0,  L"Tree",               MACRO_TREEPANEL,0},
	{0,  L"FindFolder",         MACRO_FINDFOLDER,0},
	{0,  L"UserMenu",           MACRO_USERMENU,0},
	{0,  L"AutoCompletion",     MACRO_AUTOCOMPLETION,0},
	{0,  L"Common",             MACRO_COMMON,0},
};

TMacroKeywords MKeywordsFlags[] =
{
	// �����
	{1,  L"DisableOutput",      MFLAGS_DISABLEOUTPUT,0},
	{1,  L"RunAfterFARStart",   MFLAGS_RUNAFTERFARSTART,0},
	{1,  L"EmptyCommandLine",   MFLAGS_EMPTYCOMMANDLINE,0},
	{1,  L"NotEmptyCommandLine",MFLAGS_NOTEMPTYCOMMANDLINE,0},
	{1,  L"EVSelection",        MFLAGS_EDITSELECTION,0},
	{1,  L"NoEVSelection",      MFLAGS_EDITNOSELECTION,0},

	{1,  L"NoFilePanels",       MFLAGS_NOFILEPANELS,0},
	{1,  L"NoPluginPanels",     MFLAGS_NOPLUGINPANELS,0},
	{1,  L"NoFolders",          MFLAGS_NOFOLDERS,0},
	{1,  L"NoFiles",            MFLAGS_NOFILES,0},
	{1,  L"Selection",          MFLAGS_SELECTION,0},
	{1,  L"NoSelection",        MFLAGS_NOSELECTION,0},

	{1,  L"NoFilePPanels",      MFLAGS_PNOFILEPANELS,0},
	{1,  L"NoPluginPPanels",    MFLAGS_PNOPLUGINPANELS,0},
	{1,  L"NoPFolders",         MFLAGS_PNOFOLDERS,0},
	{1,  L"NoPFiles",           MFLAGS_PNOFILES,0},
	{1,  L"PSelection",         MFLAGS_PSELECTION,0},
	{1,  L"NoPSelection",       MFLAGS_PNOSELECTION,0},

	{1,  L"NoSendKeysToPlugins",MFLAGS_NOSENDKEYSTOPLUGINS,0},
};

// ������������� ������� - ��� <-> ��� ������������
static struct TKeyCodeName
{
	int Key;
	int Len;
	const wchar_t *Name;
} KeyMacroCodes[]=
{
	{ MCODE_OP_AKEY,                 5, L"$AKey"      }, // �������, ������� ������� ������
	{ MCODE_OP_CONTINUE,             9, L"$Continue"  },
	{ MCODE_OP_ELSE,                 5, L"$Else"      },
	{ MCODE_OP_END,                  4, L"$End"       },
	{ MCODE_OP_EXIT,                 5, L"$Exit"      },
	{ MCODE_OP_IF,                   3, L"$If"        },
	{ MCODE_OP_REP,                  4, L"$Rep"       },
	{ MCODE_OP_SELWORD,              8, L"$SelWord"   },
	{ MCODE_OP_PLAINTEXT,            5, L"$Text"      }, // $Text "Plain Text"
	{ MCODE_OP_WHILE,                6, L"$While"     },
	{ MCODE_OP_XLAT,                 5, L"$XLat"      },
};

static bool absFunc(const TMacroFunction*);
static bool ascFunc(const TMacroFunction*);
static bool atoiFunc(const TMacroFunction*);
static bool beepFunc(const TMacroFunction*);
static bool callpluginFunc(const TMacroFunction*);
static bool chrFunc(const TMacroFunction*);
static bool clipFunc(const TMacroFunction*);
static bool dateFunc(const TMacroFunction*);
static bool dlggetvalueFunc(const TMacroFunction*);
static bool editorposFunc(const TMacroFunction*);
static bool editorselFunc(const TMacroFunction*);
static bool editorsetFunc(const TMacroFunction*);
static bool editorsettitleFunc(const TMacroFunction*);
static bool editorundoFunc(const TMacroFunction*);
static bool environFunc(const TMacroFunction*);
static bool fattrFunc(const TMacroFunction*);
static bool fexistFunc(const TMacroFunction*);
static bool floatFunc(const TMacroFunction*);
static bool flockFunc(const TMacroFunction*);
static bool fmatchFunc(const TMacroFunction*);
static bool fsplitFunc(const TMacroFunction*);
static bool iifFunc(const TMacroFunction*);
static bool indexFunc(const TMacroFunction*);
static bool intFunc(const TMacroFunction*);
static bool itowFunc(const TMacroFunction*);
static bool kbdLayoutFunc(const TMacroFunction*);
static bool keybarshowFunc(const TMacroFunction*);
static bool keyFunc(const TMacroFunction*);
static bool lcaseFunc(const TMacroFunction*);
static bool lenFunc(const TMacroFunction*);
static bool maxFunc(const TMacroFunction*);
static bool menushowFunc(const TMacroFunction*);
static bool minFunc(const TMacroFunction*);
static bool mloadFunc(const TMacroFunction*);
static bool modFunc(const TMacroFunction*);
static bool msaveFunc(const TMacroFunction*);
static bool msgBoxFunc(const TMacroFunction*);
static bool panelfattrFunc(const TMacroFunction*);
static bool panelfexistFunc(const TMacroFunction*);
static bool panelitemFunc(const TMacroFunction*);
static bool panelitemFunc(const TMacroFunction*);
static bool panelselectFunc(const TMacroFunction*);
static bool panelsetpathFunc(const TMacroFunction*);
static bool panelsetposFunc(const TMacroFunction*);
static bool panelsetposidxFunc(const TMacroFunction*);
static bool pluginsFunc(const TMacroFunction*);
static bool promptFunc(const TMacroFunction*);
static bool replaceFunc(const TMacroFunction*);
static bool rindexFunc(const TMacroFunction*);
static bool sleepFunc(const TMacroFunction*);
static bool stringFunc(const TMacroFunction*);
static bool substrFunc(const TMacroFunction*);
static bool testfolderFunc(const TMacroFunction*);
static bool trimFunc(const TMacroFunction*);
static bool ucaseFunc(const TMacroFunction*);
static bool usersFunc(const TMacroFunction*);
static bool waitkeyFunc(const TMacroFunction*);
static bool windowscrollFunc(const TMacroFunction*);
static bool xlatFunc(const TMacroFunction*);
static bool pluginloadFunc(const TMacroFunction*);
static bool pluginunloadFunc(const TMacroFunction*);

static bool __CheckCondForSkip(DWORD Op);

static TMacroFunction intMacroFunction[]=
{
	{L"ABS",              1, 0,   MCODE_F_ABS,              nullptr, 0,nullptr,L"N=Abs(N)",0,absFunc},
	{L"AKEY",             2, 1,   MCODE_F_AKEY,             nullptr, 0,nullptr,L"V=Akey(Mode[,Type])",0,usersFunc},
	{L"ASC",              1, 0,   MCODE_F_ASC,              nullptr, 0,nullptr,L"N=Asc(N)",0,ascFunc},
	{L"ATOI",             2, 1,   MCODE_F_ATOI,             nullptr, 0,nullptr,L"N=Atoi(S[,Radix])",0,atoiFunc},
	{L"BEEP",             1, 1,   MCODE_F_BEEP,             nullptr, 0,nullptr,L"N=Beep([N])",0,beepFunc},
	{L"BM.ADD",           0, 0,   MCODE_F_BM_ADD,           nullptr, 0,nullptr,L"N=BM.Add()",0,usersFunc},
	{L"BM.CLEAR",         0, 0,   MCODE_F_BM_CLEAR,         nullptr, 0,nullptr,L"N=BM.Clear()",0,usersFunc},
	{L"BM.DEL",           1, 1,   MCODE_F_BM_DEL,           nullptr, 0,nullptr,L"N=BM.Del([Idx])",0,usersFunc},
	{L"BM.GET",           2, 0,   MCODE_F_BM_GET,           nullptr, 0,nullptr,L"N=BM.Get(Idx,M)",0,usersFunc},
	{L"BM.GOTO",          1, 1,   MCODE_F_BM_GOTO,          nullptr, 0,nullptr,L"N=BM.Goto([N])",0,usersFunc},
	{L"BM.NEXT",          0, 0,   MCODE_F_BM_NEXT,          nullptr, 0,nullptr,L"N=BM.Next()",0,usersFunc},
	{L"BM.POP",           0, 0,   MCODE_F_BM_POP,           nullptr, 0,nullptr,L"N=BM.Pop()",0,usersFunc},
	{L"BM.PREV",          0, 0,   MCODE_F_BM_PREV,          nullptr, 0,nullptr,L"N=BM.Prev()",0,usersFunc},
	{L"BM.BACK",          0, 0,   MCODE_F_BM_BACK,          nullptr, 0,nullptr,L"N=BM.Back()",0,usersFunc},
	{L"BM.PUSH",          0, 0,   MCODE_F_BM_PUSH,          nullptr, 0,nullptr,L"N=BM.Push()",0,usersFunc},
	{L"BM.STAT",          1, 1,   MCODE_F_BM_STAT,          nullptr, 0,nullptr,L"N=BM.Stat([N])",0,usersFunc},
	{L"CALLPLUGIN",       2, 1,   MCODE_F_CALLPLUGIN,       nullptr, 0,nullptr,L"V=CallPlugin(SysID[,param])",0,callpluginFunc},
	{L"CHECKHOTKEY",      2, 1,   MCODE_F_MENU_CHECKHOTKEY, nullptr, 0,nullptr,L"N=CheckHotkey(S[,N])",0,usersFunc},
	{L"CHR",              1, 0,   MCODE_F_CHR,              nullptr, 0,nullptr,L"S=Chr(N)",0,chrFunc},
	{L"CLIP",             2, 1,   MCODE_F_CLIP,             nullptr, 0,nullptr,L"V=Clip(N[,V])",0,clipFunc},
	{L"DATE",             1, 1,   MCODE_F_DATE,             nullptr, 0,nullptr,L"S=Date([S])",0,dateFunc},
	{L"DLG.GETVALUE",     2, 0,   MCODE_F_DLG_GETVALUE,     nullptr, 0,nullptr,L"V=Dlg.GetValue(ID,N)",0,dlggetvalueFunc},
	{L"EDITOR.POS",       3, 1,   MCODE_F_EDITOR_POS,       nullptr, 0,nullptr,L"N=Editor.Pos(Op,What[,Where])",0,editorposFunc},
	{L"EDITOR.SEL",       2, 1,   MCODE_F_EDITOR_SEL,       nullptr, 0,nullptr,L"V=Editor.Sel(Action[,Opt])",0,editorselFunc},
	{L"EDITOR.SET",       2, 0,   MCODE_F_EDITOR_SET,       nullptr, 0,nullptr,L"N=Editor.Set(N,Var)",0,editorsetFunc},
	{L"EDITOR.SETTITLE",  1, 1,   MCODE_F_EDITOR_SETTITLE,  nullptr, 0,nullptr,L"N=Editor.SetTitle([Title])",0,editorsettitleFunc},
	{L"EDITOR.UNDO",      1, 0,   MCODE_F_EDITOR_UNDO,      nullptr, 0,nullptr,L"V=Editor.Undo(N)",0,editorundoFunc},
	{L"ENV",              1, 0,   MCODE_F_ENVIRON,          nullptr, 0,nullptr,L"S=Env(S)",0,environFunc},
	{L"EVAL",             2, 1,   MCODE_F_EVAL,             nullptr, 0,nullptr,L"N=Eval(S[,N])",0,usersFunc},
	{L"FATTR",            1, 0,   MCODE_F_FATTR,            nullptr, 0,nullptr,L"N=FAttr(S)",0,fattrFunc},
	{L"FEXIST",           1, 0,   MCODE_F_FEXIST,           nullptr, 0,nullptr,L"N=FExist(S)",0,fexistFunc},
	{L"FLOAT",            1, 0,   MCODE_F_FLOAT,            nullptr, 0,nullptr,L"N=Float(V)",0,floatFunc},
	{L"FLOCK",            2, 0,   MCODE_F_FLOCK,            nullptr, 0,nullptr,L"N=FLock(N,N)",0,flockFunc},
	{L"FMATCH",           2, 0,   MCODE_F_FMATCH,           nullptr, 0,nullptr,L"N=FMatch(S,Mask)",0,fmatchFunc},
	{L"FSPLIT",           2, 0,   MCODE_F_FSPLIT,           nullptr, 0,nullptr,L"S=FSplit(S,N)",0,fsplitFunc},
	{L"GETHOTKEY",        1, 1,   MCODE_F_MENU_GETHOTKEY,   nullptr, 0,nullptr,L"S=GetHotkey([N])",0,usersFunc},
	{L"HISTORY.ENABLE",   1, 1,   MCODE_F_HISTIORY_ENABLE,  nullptr, 0,nullptr,L"N=History.Enable([State])",0,usersFunc},
	{L"IIF",              3, 0,   MCODE_F_IIF,              nullptr, 0,nullptr,L"V=Iif(Condition,V1,V2)",0,iifFunc},
	{L"INDEX",            3, 1,   MCODE_F_INDEX,            nullptr, 0,nullptr,L"S=Index(S1,S2[,Mode])",0,indexFunc},
	{L"INT",              1, 0,   MCODE_F_INT,              nullptr, 0,nullptr,L"N=Int(V)",0,intFunc},
	{L"ITOA",             2, 1,   MCODE_F_ITOA,             nullptr, 0,nullptr,L"S=Itoa(N[,radix])",0,itowFunc},
	{L"KBDLAYOUT",        1, 1,   MCODE_F_KBDLAYOUT,        nullptr, 0,nullptr,L"N=kbdLayout([N])",0,kbdLayoutFunc},
	{L"KEY",              1, 0,   MCODE_F_KEY,              nullptr, 0,nullptr,L"S=Key(V)",0,keyFunc},
	{L"KEYBAR.SHOW",      1, 1,   MCODE_F_KEYBAR_SHOW,      nullptr, 0,nullptr,L"N=KeyBar.Show([N])",0,keybarshowFunc},
	{L"LCASE",            1, 0,   MCODE_F_LCASE,            nullptr, 0,nullptr,L"S=LCase(S1)",0,lcaseFunc},
	{L"LEN",              1, 0,   MCODE_F_LEN,              nullptr, 0,nullptr,L"N=Len(S)",0,lenFunc},
	{L"MAX",              2, 0,   MCODE_F_MAX,              nullptr, 0,nullptr,L"N=Max(N1,N2)",0,maxFunc},
	{L"MENU.FILTER",      2, 2,   MCODE_F_MENU_FILTER,      nullptr, 0,nullptr,L"N=Menu.Filter([Action[,Mode]])",0,usersFunc},
	{L"MENU.FILTERSTR",   2, 2,   MCODE_F_MENU_FILTERSTR,   nullptr, 0,nullptr,L"N=Menu.FilterStr([Action[,S]])",0,usersFunc},
	{L"MENU.GETVALUE",    1, 1,   MCODE_F_MENU_GETVALUE,    nullptr, 0,nullptr,L"S=Menu.GetValue([N])",0,usersFunc},
	{L"MENU.ITEMSTATUS",  1, 1,   MCODE_F_MENU_ITEMSTATUS,  nullptr, 0,nullptr,L"N=Menu.ItemStatus([N])",0,usersFunc},
	{L"MENU.SELECT",      3, 2,   MCODE_F_MENU_SELECT,      nullptr, 0,nullptr,L"N=Menu.Select(S[,N[,Dir]])",0,usersFunc},
	{L"MENU.SHOW",        6, 5,   MCODE_F_MENU_SHOW,        nullptr, 0,nullptr,L"S=Menu.Show(Items[,Title[,Flags[,FindOrFilter[,X[,Y]]]]])",IMFF_UNLOCKSCREEN|IMFF_DISABLEINTINPUT,menushowFunc},
	{L"MIN",              2, 0,   MCODE_F_MIN,              nullptr, 0,nullptr,L"N=Min(N1,N2)",0,minFunc},
	{L"MLOAD",            1, 0,   MCODE_F_MLOAD,            nullptr, 0,nullptr,L"N=MLoad(S)",0,mloadFunc},
	{L"MMODE",            2, 1,   MCODE_F_MMODE,            nullptr, 0,nullptr,L"N=MMode(Action[,Value])",0,usersFunc},
	{L"MOD",              2, 0,   MCODE_F_MOD,              nullptr, 0,nullptr,L"N=Mod(a,b)",0,modFunc},
	{L"MSAVE",            1, 0,   MCODE_F_MSAVE,            nullptr, 0,nullptr,L"N=MSave(S)",0,msaveFunc},
	{L"MSGBOX",           3, 3,   MCODE_F_MSGBOX,           nullptr, 0,nullptr,L"N=MsgBox([Title[,Text[,flags]]])",IMFF_UNLOCKSCREEN|IMFF_DISABLEINTINPUT,msgBoxFunc},
	{L"PANEL.FATTR",      2, 0,   MCODE_F_PANEL_FATTR,      nullptr, 0,nullptr,L"N=Panel.FAttr(panelType,fileMask)",0,panelfattrFunc},
	{L"PANEL.FEXIST",     2, 0,   MCODE_F_PANEL_FEXIST,     nullptr, 0,nullptr,L"N=Panel.FExist(panelType,fileMask)",0,panelfexistFunc},
	{L"PANEL.ITEM",       3, 0,   MCODE_F_PANELITEM,        nullptr, 0,nullptr,L"V=Panel.Item(Panel,Index,TypeInfo)",0,panelitemFunc},
	{L"PANEL.SELECT",     4, 2,   MCODE_F_PANEL_SELECT,     nullptr, 0,nullptr,L"V=Panel.Select(panelType,Action[,Mode[,Items]])",0,panelselectFunc},
	{L"PANEL.SETPATH",    3, 1,   MCODE_F_PANEL_SETPATH,    nullptr, 0,nullptr,L"N=panel.SetPath(panelType,pathName[,fileName])",IMFF_UNLOCKSCREEN|IMFF_DISABLEINTINPUT,panelsetpathFunc},
	{L"PANEL.SETPOS",     2, 0,   MCODE_F_PANEL_SETPOS,     nullptr, 0,nullptr,L"N=panel.SetPos(panelType,fileName)",IMFF_UNLOCKSCREEN|IMFF_DISABLEINTINPUT,panelsetposFunc},
	{L"PANEL.SETPOSIDX",  3, 1,   MCODE_F_PANEL_SETPOSIDX,  nullptr, 0,nullptr,L"N=Panel.SetPosIdx(panelType,Idx[,InSelection])",IMFF_UNLOCKSCREEN|IMFF_DISABLEINTINPUT,panelsetposidxFunc},
	{L"PANELITEM",        3, 0,   MCODE_F_PANELITEM,        nullptr, 0,nullptr,L"V=PanelItem(Panel,Index,TypeInfo)",0,panelitemFunc},
	{L"PLUGIN.LOAD",      2, 1,   MCODE_F_PLUGIN_LOAD,      nullptr, 0,nullptr,L"N=Plugin.Load(DllPath[,ForceLoad])",0,pluginloadFunc},
	{L"PLUGIN.UNLOAD",    1, 0,   MCODE_F_PLUGIN_UNLOAD,    nullptr, 0,nullptr,L"N=Plugin.UnLoad(DllPath)",0,pluginunloadFunc},
	{L"PRINT",            1, 0,   MCODE_F_PRINT,            nullptr, 0,nullptr,L"N=Print(Str)",0,usersFunc},
	{L"PROMPT",           5, 4,   MCODE_F_PROMPT,           nullptr, 0,nullptr,L"S=Prompt(Title[,Prompt[,flags[,Src[,History]]]])",IMFF_UNLOCKSCREEN|IMFF_DISABLEINTINPUT,promptFunc},
	{L"REPLACE",          5, 2,   MCODE_F_REPLACE,          nullptr, 0,nullptr,L"S=Replace(Str,Find,Replace[,Cnt[,Mode]])",0,replaceFunc},
	{L"RINDEX",           3, 1,   MCODE_F_RINDEX,           nullptr, 0,nullptr,L"S=RIndex(S1,S2[,Mode])",0,rindexFunc},
	{L"SLEEP",            1, 0,   MCODE_F_SLEEP,            nullptr, 0,nullptr,L"N=Sleep(N)",0,sleepFunc},
	{L"STRING",           1, 0,   MCODE_F_STRING,           nullptr, 0,nullptr,L"S=String(V)",0,stringFunc},
	{L"SUBSTR",           3, 1,   MCODE_F_SUBSTR,           nullptr, 0,nullptr,L"S=substr(S,start[,length])",0,substrFunc},
	{L"TESTFOLDER",       1, 0,   MCODE_F_TESTFOLDER,       nullptr, 0,nullptr,L"N=testfolder(S)",0,testfolderFunc},
	{L"TRIM",             2, 1,   MCODE_F_TRIM,             nullptr, 0,nullptr,L"S=Trim(S[,N])",0,trimFunc},
	{L"UCASE",            1, 0,   MCODE_F_UCASE,            nullptr, 0,nullptr,L"S=UCase(S1)",0,ucaseFunc},
	{L"WAITKEY",          2, 2,   MCODE_F_WAITKEY,          nullptr, 0,nullptr,L"V=Waitkey([N,[T]])",0,waitkeyFunc},
	{L"WINDOW.SCROLL",    2, 1,   MCODE_F_WINDOW_SCROLL,    nullptr, 0,nullptr,L"N=Window.Scroll(Lines[,Axis])",0,windowscrollFunc},
	{L"XLAT",             2, 1,   MCODE_F_XLAT,             nullptr, 0,nullptr,L"S=Xlat(S[,Flags])",0,xlatFunc},

	{0}
};


int MKeywordsSize = ARRAYSIZE(MKeywords);
int MKeywordsFlagsSize = ARRAYSIZE(MKeywordsFlags);

DWORD KeyMacro::LastOpCodeUF=KEY_MACRO_U_BASE;
size_t KeyMacro::CMacroFunction=0;
size_t KeyMacro::AllocatedFuncCount=0;
TMacroFunction *KeyMacro::AMacroFunction=nullptr;

TVarTable glbVarTable;
TVarTable glbConstTable;

static TVar __varTextDate;

class TVMStack: public TStack<TVar>
{
	private:
		const TVar Error;

	public:
		TVMStack() {}
		~TVMStack() {}

	public:
		const TVar &Pop()
		{
			static TVar temp; //���� ����� ���� ������� �� ��������.

			if (TStack<TVar>::Pop(temp))
				return temp;

			return Error;
		}

		TVar &Pop(TVar &dest)
		{
			if (!TStack<TVar>::Pop(dest))
				dest=Error;

			return dest;
		}

		const TVar &Peek()
		{
			TVar *var = TStack<TVar>::Peek();

			if (var)
				return *var;

			return Error;
		}
};

TVMStack VMStack;

static LONG _RegWriteString(const wchar_t *Key,const wchar_t *ValueName,const wchar_t *Data);

// ������� �������������� ���� ������������ � �����
BOOL WINAPI KeyMacroToText(int Key,string &strKeyText0)
{
	string strKeyText;

	for (int I=0; I<int(ARRAYSIZE(KeyMacroCodes)); I++)
	{
		if (Key==KeyMacroCodes[I].Key)
		{
			strKeyText = KeyMacroCodes[I].Name;
			break;
		}
	}

	if (strKeyText.IsEmpty())
	{
		strKeyText0.Clear();
		return FALSE;
	}

	strKeyText0 = strKeyText;
	return TRUE;
}

// ������� �������������� �������� � ��� ������������
// ������ -1, ���� ��� �����������!
int WINAPI KeyNameMacroToKey(const wchar_t *Name)
{
	// ��������� �� ���� �������������
	for (int I=0; I < int(ARRAYSIZE(KeyMacroCodes)); ++I)
		if (!StrCmpNI(Name,KeyMacroCodes[I].Name,KeyMacroCodes[I].Len))
			return KeyMacroCodes[I].Key;

	return -1;
}

KeyMacro::KeyMacro():
	MacroVersion(GetRegKey(L"KeyMacros",L"MacroVersion",0)),
	Recording(MACROMODE_NOMACRO),
	Mode(MACRO_SHELL),
	CurPCStack(-1),
	StopMacro(false),
	MacroLIB(nullptr),
	RecBufferSize(0),
	RecBuffer(nullptr),
	RecSrc(nullptr),
	LockScr(nullptr),
  IsRedrawEditor(TRUE)
{
	Work.Init(nullptr);
	memset(&IndexMode,0,sizeof(IndexMode));
}

KeyMacro::~KeyMacro()
{
	InitInternalVars();

	if (Work.AllocVarTable && Work.locVarTable)
		xf_free(Work.locVarTable);

	UnregMacroFunction(-1);
}

void KeyMacro::InitInternalLIBVars()
{
	if (MacroLIB)
	{
		for (int I=0; I<MacroLIBCount; I++)
		{
			if (MacroLIB[I].BufferSize > 1 && MacroLIB[I].Buffer)
				xf_free(MacroLIB[I].Buffer);

			if (MacroLIB[I].Src)
				xf_free(MacroLIB[I].Src);

			if (MacroLIB[I].Description)
				xf_free(MacroLIB[I].Description);
		}

		xf_free(MacroLIB);
	}

	if (RecBuffer)
		xf_free(RecBuffer);
	RecBuffer=nullptr;
	RecBufferSize=0;

	memset(&IndexMode,0,sizeof(IndexMode));
	MacroLIBCount=0;
	MacroLIB=nullptr;
	//LastOpCodeUF=KEY_MACRO_U_BASE;
}

// ������������� ���� ����������
void KeyMacro::InitInternalVars(BOOL InitedRAM)
{
	InitInternalLIBVars();

	if (LockScr)
	{
		delete LockScr;
		LockScr=nullptr;
	}

	if (InitedRAM)
	{
		ReleaseWORKBuffer(TRUE);
		Work.Executing=MACROMODE_NOMACRO;
	}

	Work.HistroyEnable=0;
	RecBuffer=nullptr;
	RecBufferSize=0;
	RecSrc=nullptr;
	Recording=MACROMODE_NOMACRO;
	InternalInput=FALSE;
	VMStack.Free();
	CurPCStack=-1;
}

// �������� ���������� ������, ���� �� ���������� �����������
// (����������� - ������ � PlayMacros �������� ������.
void KeyMacro::ReleaseWORKBuffer(BOOL All)
{
	if (Work.MacroWORK)
	{
		if (All || Work.MacroWORKCount <= 1)
		{
			for (int I=0; I<Work.MacroWORKCount; I++)
			{
				if (Work.MacroWORK[I].BufferSize > 1 && Work.MacroWORK[I].Buffer)
					xf_free(Work.MacroWORK[I].Buffer);

				if (Work.MacroWORK[I].Src)
					xf_free(Work.MacroWORK[I].Src);

				if (Work.MacroWORK[I].Description)
					xf_free(Work.MacroWORK[I].Description);
			}

			xf_free(Work.MacroWORK);

			if (Work.AllocVarTable)
			{
				deleteVTable(*Work.locVarTable);
				//xf_free(Work.locVarTable);
				//Work.locVarTable=nullptr;
				//Work.AllocVarTable=false;
			}

			Work.MacroWORK=nullptr;
			Work.MacroWORKCount=0;
		}
		else
		{
			if (Work.MacroWORK->BufferSize > 1 && Work.MacroWORK->Buffer)
				xf_free(Work.MacroWORK->Buffer);

			if (Work.MacroWORK->Src)
				xf_free(Work.MacroWORK->Src);

			if (Work.MacroWORK->Description)
				xf_free(Work.MacroWORK->Description);

			if (Work.AllocVarTable)
			{
				deleteVTable(*Work.locVarTable);
				//xf_free(Work.locVarTable);
				//Work.locVarTable=nullptr;
				//Work.AllocVarTable=false;
			}

			Work.MacroWORKCount--;
			memmove(Work.MacroWORK,((BYTE*)Work.MacroWORK)+sizeof(MacroRecord),sizeof(MacroRecord)*Work.MacroWORKCount);
			Work.MacroWORK=(MacroRecord *)xf_realloc(Work.MacroWORK,sizeof(MacroRecord)*Work.MacroWORKCount);
		}
	}
}

// �������� ���� �������� �� �������
int KeyMacro::LoadMacros(BOOL InitedRAM,BOOL LoadAll)
{
	int ErrCount=0;
	InitInternalVars(InitedRAM);

	if (Opt.Macro.DisableMacro&MDOL_ALL)
		return FALSE;

	string strBuffer;
	ReadVarsConst(MACRO_VARS,strBuffer);
	ReadVarsConst(MACRO_CONSTS,strBuffer);
	ReadMacroFunction(MACRO_FUNCS,strBuffer);

	int Areas[MACRO_LAST];

	for (int i=MACRO_OTHER; i < MACRO_LAST; i++)
	{
		Areas[i]=i;
	}

	if (!LoadAll)
	{
		// "������� �� �����" �������� ������� - ����� ����������� ������ ��, ��� �� ����� �������� MACRO_LAST
		Areas[MACRO_SHELL]=
			Areas[MACRO_SEARCH]=
			Areas[MACRO_DISKS]=
			Areas[MACRO_MAINMENU]=
			Areas[MACRO_INFOPANEL]=
			Areas[MACRO_QVIEWPANEL]=
			Areas[MACRO_TREEPANEL]=
			Areas[MACRO_USERMENU]= // <-- Mantis#0001594
			Areas[MACRO_FINDFOLDER]=MACRO_LAST;
	}

	for (int i=MACRO_OTHER; i < MACRO_LAST; i++)
	{
		if (Areas[i] == MACRO_LAST)
			continue;

		if (!ReadMacros(i,strBuffer))
		{
			ErrCount++;
		}
	}

	KeyMacro::Sort();

	return ErrCount?FALSE:TRUE;
}

int KeyMacro::ProcessKey(int Key)
{
	if (InternalInput || Key==KEY_IDLE || Key==KEY_NONE || !FrameManager->GetCurrentFrame())
		return FALSE;

	if (Recording) // ���� ������?
	{
		if ((unsigned int)Key==Opt.Macro.KeyMacroCtrlDot || (unsigned int)Key==Opt.Macro.KeyMacroCtrlShiftDot) // ������� ����� ������?
		{
			_KEYMACRO(CleverSysLog Clev(L"MACRO End record..."));
			DWORD MacroKey;
			int WaitInMainLoop0=WaitInMainLoop;
			InternalInput=TRUE;
			WaitInMainLoop=FALSE;
			// �������� _�������_ �����, � �� _��������� �����������_
			FrameManager->GetCurrentFrame()->Lock(); // ������� ���������� ������
			MacroKey=AssignMacroKey();
			FrameManager->ResetLastInputRecord();
			FrameManager->GetCurrentFrame()->Unlock(); // ������ ����� :-)
			// ���������� ����� �� ���������.
			DWORD Flags=MFLAGS_DISABLEOUTPUT; // ???
			// ������� �������� �� ��������
			// ���� �������, �� �� ����� �������� ������ ���������.
			//if (MacroKey != (DWORD)-1 && (Key==KEY_CTRLSHIFTDOT || Recording==2) && RecBufferSize)
			if (MacroKey != (DWORD)-1 && (unsigned int)Key==Opt.Macro.KeyMacroCtrlShiftDot && RecBufferSize)
			{
				if (!GetMacroSettings(MacroKey,Flags))
					MacroKey=(DWORD)-1;
			}

			WaitInMainLoop=WaitInMainLoop0;
			InternalInput=FALSE;

			if (MacroKey==(DWORD)-1)
			{
				if (RecBuffer)
				{
					xf_free(RecBuffer);
					RecBuffer=nullptr;
					RecBufferSize=0;
				}
			}
			else
			{
				// � ������� common ����� ������ ������ ��� ��������
				int Pos=GetIndex(MacroKey,StartMode,!(RecBuffer && RecBufferSize));

				if (Pos == -1)
				{
					Pos=MacroLIBCount;

					if (RecBufferSize > 0)
					{
						MacroRecord *NewMacroLIB=(MacroRecord *)xf_realloc(MacroLIB,sizeof(*MacroLIB)*(MacroLIBCount+1));

						if (!NewMacroLIB)
						{
							WaitInFastFind++;
							return FALSE;
						}

						MacroLIB=NewMacroLIB;
						MacroLIBCount++;
					}
				}
				else
				{
					if (MacroLIB[Pos].BufferSize > 1 && MacroLIB[Pos].Buffer)
						xf_free(MacroLIB[Pos].Buffer);

					if (MacroLIB[Pos].Src)
						xf_free(MacroLIB[Pos].Src);

					if (MacroLIB[Pos].Description)
						xf_free(MacroLIB[Pos].Description);

					MacroLIB[Pos].Buffer=nullptr;
					MacroLIB[Pos].Src=nullptr;
					MacroLIB[Pos].Description=nullptr;
				}

				if (Pos < MacroLIBCount)
				{
					MacroLIB[Pos].Key=MacroKey;

					if (RecBufferSize > 0 && !RecSrc)
						RecBuffer[RecBufferSize++]=MCODE_OP_ENDKEYS;

					if (RecBufferSize > 1)
						MacroLIB[Pos].Buffer=RecBuffer;
					else if (RecBuffer && RecBufferSize > 0)
						MacroLIB[Pos].Buffer=reinterpret_cast<DWORD*>((DWORD_PTR)(*RecBuffer));
					else if (!RecBufferSize)
						MacroLIB[Pos].Buffer=nullptr;

					MacroLIB[Pos].BufferSize=RecBufferSize;
					MacroLIB[Pos].Src=RecSrc?RecSrc:MkTextSequence(MacroLIB[Pos].Buffer,MacroLIB[Pos].BufferSize);
					MacroLIB[Pos].Description=nullptr;

					// ���� ������� ������ - ������������� StartMode,
					// ����� ������ �� common ������� �� �������, � ������� ��� ������ �������.
					if (!MacroLIB[Pos].BufferSize||!MacroLIB[Pos].Src)
						StartMode=MacroLIB[Pos].Flags&MFLAGS_MODEMASK;

					MacroLIB[Pos].Flags=Flags|(StartMode&MFLAGS_MODEMASK)|MFLAGS_NEEDSAVEMACRO|(Recording==MACROMODE_RECORDING_COMMON?0:MFLAGS_NOSENDKEYSTOPLUGINS);
				}
			}

			Recording=MACROMODE_NOMACRO;
			RecBuffer=nullptr;
			RecBufferSize=0;
			RecSrc=nullptr;
			ScrBuf.RestoreMacroChar();
			WaitInFastFind++;
			KeyMacro::Sort();

			if (Opt.AutoSaveSetup)
				SaveMacros(FALSE); // �������� ������ ���������!

			return TRUE;
		}
		else // ������� ������ ������������.
		{
			if ((unsigned int)Key>=KEY_NONE && (unsigned int)Key<=KEY_END_SKEY) // ����������� ������� ��������
				return FALSE;

			RecBuffer=(DWORD *)xf_realloc(RecBuffer,sizeof(*RecBuffer)*(RecBufferSize+3));

			if (!RecBuffer)
			{
				RecBufferSize=0;
				return FALSE;
			}

			if (IntKeyState.ReturnAltValue) // "����������" ������ ;-)
				Key|=KEY_ALTDIGIT;

			if (!RecBufferSize)
				RecBuffer[RecBufferSize++]=MCODE_OP_KEYS;

			RecBuffer[RecBufferSize++]=Key;
			return FALSE;
		}
	}
	else if ((unsigned int)Key==Opt.Macro.KeyMacroCtrlDot || (unsigned int)Key==Opt.Macro.KeyMacroCtrlShiftDot) // ������ ������?
	{
		_KEYMACRO(CleverSysLog Clev(L"MACRO Begin record..."));

		// ������� 18
		if (Opt.Policies.DisabledOptions&FFPOL_CREATEMACRO)
			return FALSE;

		//if(CtrlObject->Plugins.CheckFlags(PSIF_ENTERTOOPENPLUGIN))
		//	return FALSE;

		if (LockScr)
			delete LockScr;
		LockScr=nullptr;

		// ��� ��?
		StartMode=(Mode==MACRO_SHELL && !WaitInMainLoop)?MACRO_OTHER:Mode;
		// ��� ������ - � ������� ������� �������� ���...
		// � ����������� �� ����, ��� ������ ������ ������, ��������� ����� ����� (Ctrl-.
		// � ��������� ������� ����) ��� ����������� (Ctrl-Shift-. - ��� �������� ������ �������)
		Recording=((unsigned int)Key==Opt.Macro.KeyMacroCtrlDot) ? MACROMODE_RECORDING_COMMON:MACROMODE_RECORDING;

		if (RecBuffer)
			xf_free(RecBuffer);
		RecBuffer=nullptr;
		RecBufferSize=0;

		RecSrc=nullptr;
		ScrBuf.ResetShadow();
		ScrBuf.Flush();
		WaitInFastFind--;
		return TRUE;
	}
	else
	{
		if (Work.Executing == MACROMODE_NOMACRO) // ��� ��� �� ����� ����������?
		{
			//_KEYMACRO(CleverSysLog Clev(L"MACRO find..."));
			//_KEYMACRO(SysLog(L"Param Key=%s",_FARKEY_ToName(Key)));
			DWORD CurFlags;

			StopMacro=false;

			if ((Key&(~KEY_CTRLMASK)) > 0x01 && (Key&(~KEY_CTRLMASK)) < KEY_FKEY_BEGIN) // 0xFFFF ??
			{
				//Key=KeyToKeyLayout(Key&0x0000FFFF)|(Key&(~0x0000FFFF));
				//Key=Upper(Key&0x0000FFFF)|(Key&(~0x0000FFFF));
				//_KEYMACRO(SysLog(L"Upper(Key)=%s",_FARKEY_ToName(Key)));

				if ((Key&(~KEY_CTRLMASK)) > 0x7F && (Key&(~KEY_CTRLMASK)) < KEY_FKEY_BEGIN)
					Key=KeyToKeyLayout(Key&0x0000FFFF)|(Key&(~0x0000FFFF));

				if ((DWORD)Key < KEY_FKEY_BEGIN)
					Key=Upper(Key&0x0000FFFF)|(Key&(~0x0000FFFF));

			}

			int I=GetIndex(Key,(Mode==MACRO_SHELL && !WaitInMainLoop) ? MACRO_OTHER:Mode);

			if (I != -1 && !((CurFlags=MacroLIB[I].Flags)&MFLAGS_DISABLEMACRO) && CtrlObject)
			{
				_KEYMACRO(SysLog(L"[%d] Found KeyMacro (I=%d Key=%s,%s)",__LINE__,I,_FARKEY_ToName(Key),_FARKEY_ToName(MacroLIB[I].Key)));

				if (!CheckAll(Mode,CurFlags))
					return FALSE;

				// ��������� ������� ���������� � MacroWORK
				//PostNewMacro(MacroLIB+I);
				// ��������� �����?
				if (CurFlags&MFLAGS_DISABLEOUTPUT)
				{
					if (LockScr)
						delete LockScr;

					LockScr=new LockScreen;
				}

				// ��������� ����� ����� (� ��������� ������� ����) ��� ����������� (��� �������� ������ �������)
				Work.HistroyEnable=0;
				Work.ExecLIBPos=0;
				PostNewMacro(MacroLIB+I);
				Work.cRec=*FrameManager->GetLastInputRecord();
				_SVS(FarSysLog_INPUT_RECORD_Dump(L"Macro",&Work.cRec));
				Work.MacroPC=I;
				IsRedrawEditor=CtrlObject->Plugins.CheckFlags(PSIF_ENTERTOOPENPLUGIN)?FALSE:TRUE;
				_KEYMACRO(SysLog(L"**** Start Of Execute Macro ****"));
				_KEYMACRO(SysLog(1));
				return TRUE;
			}
		}

		return FALSE;
	}
}

bool KeyMacro::GetPlainText(string& strDest)
{
	strDest.Clear();

	if (!Work.MacroWORK)
		return false;

	MacroRecord *MR=Work.MacroWORK;
	int LenTextBuf=(int)(StrLength((wchar_t*)&MR->Buffer[Work.ExecLIBPos]))*sizeof(wchar_t);

	if (LenTextBuf && MR->Buffer[Work.ExecLIBPos])
	{
		strDest=(const wchar_t *)&MR->Buffer[Work.ExecLIBPos];
		_SVS(SysLog(L"strDest='%s'",strDest.CPtr()));
		_SVS(SysLog(L"Work.ExecLIBPos=%d",Work.ExecLIBPos));
		Work.ExecLIBPos+=(LenTextBuf+sizeof(wchar_t))/sizeof(DWORD);
		_SVS(SysLog(L"Work.ExecLIBPos=%d",Work.ExecLIBPos));

		if (((LenTextBuf+sizeof(wchar_t))%sizeof(DWORD)) )
			++Work.ExecLIBPos;

		_SVS(SysLog(L"Work.ExecLIBPos=%d",Work.ExecLIBPos));
		return true;
	}
	else
	{
		Work.ExecLIBPos++;
	}

	return false;
}

int KeyMacro::GetPlainTextSize()
{
	if (!Work.MacroWORK)
		return 0;

	MacroRecord *MR=Work.MacroWORK;
	return StrLength((wchar_t*)&MR->Buffer[Work.ExecLIBPos]);
}

TVar KeyMacro::FARPseudoVariable(DWORD Flags,DWORD CheckCode,DWORD& Err)
{
	_KEYMACRO(CleverSysLog Clev(L"KeyMacro::FARPseudoVariable()"));
	size_t I;
	TVar Cond(0ll);
	string strFileName;
	DWORD FileAttr=INVALID_FILE_ATTRIBUTES;

	// ������ ������ ������� ��������
	for (I=0 ; I < ARRAYSIZE(MKeywords) ; ++I)
		if (MKeywords[I].Value == CheckCode)
			break;

	if (I == ARRAYSIZE(MKeywords))
	{
		Err=1;
		_KEYMACRO(SysLog(L"return; Err=%d",Err));
		return Cond; // ����� TRUE �����������, ����� ���������� ���������� �������, ��� ��� �� ���������.
	}

	Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;

	// ������ ������� ����������� ��������
	switch (MKeywords[I].Type)
	{
		case 0: // �������� �� �������
		{
			if (WaitInMainLoop) // ����� ���� ������ ��� ����� WaitInMainLoop, ���� ���� � ���������!!!
				Cond=int(CheckCode-MCODE_C_AREA_OTHER+MACRO_OTHER) == FrameManager->GetCurrentFrame()->GetMacroMode()?1:0;
			else
				Cond=int(CheckCode-MCODE_C_AREA_OTHER+MACRO_OTHER) == CtrlObject->Macro.GetMode()?1:0;

			break;
		}
		case 2:
		{
			Panel *PassivePanel=nullptr;

			if (ActivePanel)
				PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

			Frame* CurFrame=FrameManager->GetCurrentFrame();

			switch (CheckCode)
			{
				case MCODE_V_FAR_WIDTH:
					Cond=(__int64)(ScrX+1);
					break;
				case MCODE_V_FAR_HEIGHT:
					Cond=(__int64)(ScrY+1);
					break;
				case MCODE_V_FAR_TITLE:
					Console.GetTitle(strFileName);
					Cond=strFileName.CPtr();
					break;
				case MCODE_V_FAR_UPTIME:
				{
					__int64 Frequency, Counter;
					QueryPerformanceFrequency((LARGE_INTEGER *) &Frequency);
					QueryPerformanceCounter((LARGE_INTEGER *) &Counter);
					Cond=((Counter-FarUpTime)*1000)/Frequency;
					break;
				}
				case MCODE_V_MACROAREA:
					Cond=GetSubKey(CtrlObject->Macro.GetMode());
					break;
				case MCODE_C_FULLSCREENMODE: // Fullscreen?
					Cond=IsFullscreen()?1:0;
					break;
				case MCODE_C_ISUSERADMIN: // IsUserAdmin?
					Cond=(__int64)Opt.IsUserAdmin;
					break;
				case MCODE_V_DRVSHOWPOS: // Drv.ShowPos
					Cond=(__int64)Macro_DskShowPosType;
					break;
				case MCODE_V_DRVSHOWMODE: // Drv.ShowMode
					Cond=(__int64)Opt.ChangeDriveMode;
					break;
				case MCODE_C_CMDLINE_BOF:              // CmdLine.Bof - ������ � ������ cmd-������ ��������������?
				case MCODE_C_CMDLINE_EOF:              // CmdLine.Eof - ������ � ������ cmd-������ ��������������?
				case MCODE_C_CMDLINE_EMPTY:            // CmdLine.Empty
				case MCODE_C_CMDLINE_SELECTED:         // CmdLine.Selected
				case MCODE_V_CMDLINE_ITEMCOUNT:        // CmdLine.ItemCount
				case MCODE_V_CMDLINE_CURPOS:           // CmdLine.CurPos
				{
					Cond=CtrlObject->CmdLine?CtrlObject->CmdLine->VMProcess(CheckCode):-1;
					break;
				}
				case MCODE_V_CMDLINE_VALUE:            // CmdLine.Value
				{
					if (CtrlObject->CmdLine)
						CtrlObject->CmdLine->GetString(strFileName);
					Cond=strFileName.CPtr();
					break;
				}
				case MCODE_C_APANEL_ROOT:  // APanel.Root
				case MCODE_C_PPANEL_ROOT:  // PPanel.Root
				{
					Panel *SelPanel=(CheckCode==MCODE_C_APANEL_ROOT)?ActivePanel:PassivePanel;

					if (SelPanel)
						Cond=SelPanel->VMProcess(MCODE_C_ROOTFOLDER)?1:0;

					break;
				}
				case MCODE_C_APANEL_BOF:
				case MCODE_C_PPANEL_BOF:
				case MCODE_C_APANEL_EOF:
				case MCODE_C_PPANEL_EOF:
				{
					Panel *SelPanel=(CheckCode==MCODE_C_APANEL_BOF || CheckCode==MCODE_C_APANEL_EOF)?ActivePanel:PassivePanel;

					if (SelPanel)
						Cond=SelPanel->VMProcess(CheckCode==MCODE_C_APANEL_BOF || CheckCode==MCODE_C_PPANEL_BOF?MCODE_C_BOF:MCODE_C_EOF)?1:0;

					break;
				}
				case MCODE_C_SELECTED:    // Selected?
				{
					int NeedType = Mode == MACRO_EDITOR? MODALTYPE_EDITOR : (Mode == MACRO_VIEWER? MODALTYPE_VIEWER : (Mode == MACRO_DIALOG? MODALTYPE_DIALOG : MODALTYPE_PANELS));

					if (!(Mode == MACRO_USERMENU || Mode == MACRO_MAINMENU || Mode == MACRO_MENU) && CurFrame && CurFrame->GetType()==NeedType)
					{
						int CurSelected;

						if (Mode==MACRO_SHELL && CtrlObject->CmdLine->IsVisible())
							CurSelected=(int)CtrlObject->CmdLine->VMProcess(CheckCode);
						else
							CurSelected=(int)CurFrame->VMProcess(CheckCode);

						Cond=CurSelected?1:0;
					}
					else
					{
					#if 1
						Frame *f=FrameManager->GetCurrentFrame(), *fo=nullptr;

						//f=f->GetTopModal();
						while (f)
						{
							fo=f;
							f=f->GetTopModal();
						}

						if (!f)
							f=fo;

						if (f)
						{
							Cond=f->VMProcess(CheckCode);
						}
					#else

						Frame *f=FrameManager->GetTopModal();

						if (f)
							Cond=(__int64)f->VMProcess(CheckCode);
					#endif
					}
					break;
				}
				case MCODE_C_EMPTY:   // Empty
				case MCODE_C_BOF:
				case MCODE_C_EOF:
				{
					int CurMMode=CtrlObject->Macro.GetMode();

					if (!(Mode == MACRO_USERMENU || Mode == MACRO_MAINMENU || Mode == MACRO_MENU) && CurFrame && CurFrame->GetType() == MODALTYPE_PANELS && !(CurMMode == MACRO_INFOPANEL || CurMMode == MACRO_QVIEWPANEL || CurMMode == MACRO_TREEPANEL))
					{
						if (CheckCode == MCODE_C_EMPTY)
							Cond=CtrlObject->CmdLine->GetLength()?0:1;
						else
							Cond=CtrlObject->CmdLine->VMProcess(CheckCode);
					}
					else
					{
						//if (IsMenuArea(CurMMode) || CurMMode == MACRO_DIALOG)
						{
							Frame *f=FrameManager->GetCurrentFrame(), *fo=nullptr;

							//f=f->GetTopModal();
							while (f)
							{
								fo=f;
								f=f->GetTopModal();
							}

							if (!f)
								f=fo;

							if (f)
							{
								Cond=f->VMProcess(CheckCode);
							}
						}
					}

					break;
				}
				case MCODE_V_DLGITEMCOUNT: // Dlg.ItemCount
				case MCODE_V_DLGCURPOS:    // Dlg.CurPos
				case MCODE_V_DLGITEMTYPE:  // Dlg.ItemType
				{
					if (CurFrame && CurFrame->GetType()==MODALTYPE_DIALOG) // ?? Mode == MACRO_DIALOG ??
						Cond=(__int64)CurFrame->VMProcess(CheckCode);

					break;
				}
				case MCODE_V_DLGINFOID:        // Dlg.Info.Id
				{
					if (CurFrame && CurFrame->GetType()==MODALTYPE_DIALOG) // ?? Mode == MACRO_DIALOG ??
					{
						Cond=reinterpret_cast<LPCWSTR>(static_cast<INT_PTR>(CurFrame->VMProcess(CheckCode)));
					}

					break;
				}
				case MCODE_C_APANEL_VISIBLE:  // APanel.Visible
				case MCODE_C_PPANEL_VISIBLE:  // PPanel.Visible
				{
					Panel *SelPanel=CheckCode==MCODE_C_APANEL_VISIBLE?ActivePanel:PassivePanel;

					if (SelPanel)
						Cond = SelPanel->IsVisible()?1:0;

					break;
				}
				case MCODE_C_APANEL_ISEMPTY: // APanel.Empty
				case MCODE_C_PPANEL_ISEMPTY: // PPanel.Empty
				{
					Panel *SelPanel=CheckCode==MCODE_C_APANEL_ISEMPTY?ActivePanel:PassivePanel;

					if (SelPanel)
					{
						SelPanel->GetFileName(strFileName,SelPanel->GetCurrentPos(),FileAttr);
						int GetFileCount=SelPanel->GetFileCount();
						Cond=(!GetFileCount ||
						      (GetFileCount == 1 && TestParentFolderName(strFileName)))
						     ?1:0;
					}

					break;
				}
				case MCODE_C_APANEL_FILTER:
				case MCODE_C_PPANEL_FILTER:
				{
					Panel *SelPanel=(CheckCode==MCODE_C_APANEL_FILTER)?ActivePanel:PassivePanel;

					if (SelPanel)
						Cond=SelPanel->VMProcess(MCODE_C_APANEL_FILTER)?1:0;

					break;
				}
				case MCODE_C_APANEL_LFN:
				case MCODE_C_PPANEL_LFN:
				{
					Panel *SelPanel = CheckCode == MCODE_C_APANEL_LFN ? ActivePanel : PassivePanel;

					if (SelPanel )
						Cond = SelPanel->GetShowShortNamesMode()?0:1;

					break;
				}
				case MCODE_C_APANEL_LEFT: // APanel.Left
				case MCODE_C_PPANEL_LEFT: // PPanel.Left
				{
					Panel *SelPanel = CheckCode == MCODE_C_APANEL_LEFT ? ActivePanel : PassivePanel;

					if (SelPanel )
						Cond = SelPanel == CtrlObject->Cp()->LeftPanel ? 1 : 0;

					break;
				}
				case MCODE_C_APANEL_FILEPANEL: // APanel.FilePanel
				case MCODE_C_PPANEL_FILEPANEL: // PPanel.FilePanel
				{
					Panel *SelPanel = CheckCode == MCODE_C_APANEL_FILEPANEL ? ActivePanel : PassivePanel;

					if (SelPanel )
						Cond=SelPanel->GetType() == FILE_PANEL;

					break;
				}
				case MCODE_C_APANEL_PLUGIN: // APanel.Plugin
				case MCODE_C_PPANEL_PLUGIN: // PPanel.Plugin
				{
					Panel *SelPanel=CheckCode==MCODE_C_APANEL_PLUGIN?ActivePanel:PassivePanel;

					if (SelPanel)
						Cond=SelPanel->GetMode() == PLUGIN_PANEL;

					break;
				}
				case MCODE_C_APANEL_FOLDER: // APanel.Folder
				case MCODE_C_PPANEL_FOLDER: // PPanel.Folder
				{
					Panel *SelPanel=CheckCode==MCODE_C_APANEL_FOLDER?ActivePanel:PassivePanel;

					if (SelPanel)
					{
						SelPanel->GetFileName(strFileName,SelPanel->GetCurrentPos(),FileAttr);

						if (FileAttr != INVALID_FILE_ATTRIBUTES)
							Cond=(FileAttr&FILE_ATTRIBUTE_DIRECTORY)?1:0;
					}

					break;
				}
				case MCODE_C_APANEL_SELECTED: // APanel.Selected
				case MCODE_C_PPANEL_SELECTED: // PPanel.Selected
				{
					Panel *SelPanel=CheckCode==MCODE_C_APANEL_SELECTED?ActivePanel:PassivePanel;

					if (SelPanel)
					{
						int SelCount=SelPanel->GetRealSelCount();
						Cond=SelCount >= 1?1:0; //??
					}

					break;
				}
				case MCODE_V_APANEL_CURRENT: // APanel.Current
				case MCODE_V_PPANEL_CURRENT: // PPanel.Current
				{
					Panel *SelPanel = CheckCode == MCODE_V_APANEL_CURRENT ? ActivePanel : PassivePanel;

					Cond = L"";

					if (SelPanel )
					{
						SelPanel->GetFileName(strFileName,SelPanel->GetCurrentPos(),FileAttr);

						if (FileAttr != INVALID_FILE_ATTRIBUTES)
							Cond = strFileName.CPtr();
					}

					break;
				}
				case MCODE_V_APANEL_SELCOUNT: // APanel.SelCount
				case MCODE_V_PPANEL_SELCOUNT: // PPanel.SelCount
				{
					Panel *SelPanel = CheckCode == MCODE_V_APANEL_SELCOUNT ? ActivePanel : PassivePanel;

					if (SelPanel )
						Cond = (__int64)SelPanel->GetRealSelCount();

					break;
				}
				case MCODE_V_APANEL_COLUMNCOUNT:       // APanel.ColumnCount - �������� ������:  ���������� �������
				case MCODE_V_PPANEL_COLUMNCOUNT:       // PPanel.ColumnCount - ��������� ������: ���������� �������
				{
					Panel *SelPanel = CheckCode == MCODE_V_APANEL_COLUMNCOUNT ? ActivePanel : PassivePanel;

					if (SelPanel )
						Cond = (__int64)SelPanel->GetColumnsCount();

					break;
				}
				case MCODE_V_APANEL_WIDTH: // APanel.Width
				case MCODE_V_PPANEL_WIDTH: // PPanel.Width
				case MCODE_V_APANEL_HEIGHT: // APanel.Height
				case MCODE_V_PPANEL_HEIGHT: // PPanel.Height
				{
					Panel *SelPanel = CheckCode == MCODE_V_APANEL_WIDTH || CheckCode == MCODE_V_APANEL_HEIGHT? ActivePanel : PassivePanel;

					if (SelPanel )
					{
						int X1, Y1, X2, Y2;
						SelPanel->GetPosition(X1,Y1,X2,Y2);

						if (CheckCode == MCODE_V_APANEL_HEIGHT || CheckCode == MCODE_V_PPANEL_HEIGHT)
							Cond = (__int64)(Y2-Y1+1);
						else
							Cond = (__int64)(X2-X1+1);
					}

					break;
				}

				case MCODE_V_APANEL_STATUSHEIGHT: // APanel.StatusHeight
				case MCODE_V_PPANEL_STATUSHEIGHT: // PPanel.StatusHeight
					{
						Panel *SelPanel = CheckCode == MCODE_V_APANEL_STATUSHEIGHT ? ActivePanel : PassivePanel;

						if (SelPanel )
							Cond = (__int64)SelPanel->GetPanelStatusHeight();

						break;
					}

				case MCODE_V_APANEL_OPIFLAGS:  // APanel.OPIFlags
				case MCODE_V_PPANEL_OPIFLAGS:  // PPanel.OPIFlags
				case MCODE_V_APANEL_HOSTFILE: // APanel.HostFile
				case MCODE_V_PPANEL_HOSTFILE: // PPanel.HostFile
				{
					Panel *SelPanel = CheckCode == MCODE_V_APANEL_OPIFLAGS || CheckCode == MCODE_V_APANEL_HOSTFILE? ActivePanel : PassivePanel;

					if (CheckCode == MCODE_V_APANEL_HOSTFILE || CheckCode == MCODE_V_PPANEL_HOSTFILE)
						Cond = L"";

					if (SelPanel )
					{
						if (SelPanel->GetMode() == PLUGIN_PANEL)
						{
							OpenPluginInfo Info={0};
							Info.StructSize=sizeof(OpenPluginInfo);
							SelPanel->GetOpenPluginInfo(&Info);
							if (CheckCode == MCODE_V_APANEL_OPIFLAGS || CheckCode == MCODE_V_PPANEL_OPIFLAGS)
								Cond = (__int64)Info.Flags;
							else
								Cond = Info.HostFile;
						}
					}

					break;
				}

				case MCODE_V_APANEL_PREFIX:           // APanel.Prefix
				case MCODE_V_PPANEL_PREFIX:           // PPanel.Prefix
				{
					Panel *SelPanel = CheckCode == MCODE_V_APANEL_PREFIX ? ActivePanel : PassivePanel;

					Cond = L"";

					if (SelPanel )
					{
						PluginInfo PInfo;
						if (SelPanel->VMProcess(MCODE_V_APANEL_PREFIX,&PInfo))
							Cond = PInfo.CommandPrefix;
					}

					break;
				}

				case MCODE_V_APANEL_PATH0:           // APanel.Path0
				case MCODE_V_PPANEL_PATH0:           // PPanel.Path0
				{
					Panel *SelPanel = CheckCode == MCODE_V_APANEL_PATH0 ? ActivePanel : PassivePanel;

					Cond = L"";

					if (SelPanel )
					{
						if (!SelPanel->VMProcess(CheckCode,&strFileName,0))
							SelPanel->GetCurDir(strFileName);
						Cond = strFileName.CPtr();
					}

					break;
				}

				case MCODE_V_APANEL_PATH: // APanel.Path
				case MCODE_V_PPANEL_PATH: // PPanel.Path
				{
					Panel *SelPanel = CheckCode == MCODE_V_APANEL_PATH ? ActivePanel : PassivePanel;

					Cond = L"";

					if (SelPanel )
					{
						if (SelPanel->GetMode() == PLUGIN_PANEL)
						{
							OpenPluginInfo Info={0};
							Info.StructSize=sizeof(OpenPluginInfo);
							SelPanel->GetOpenPluginInfo(&Info);
							strFileName = Info.CurDir;
						}
						else
							SelPanel->GetCurDir(strFileName);
						DeleteEndSlash(strFileName); // - ����� � ����� ����� ���� C:, ����� ����� ������ ���: APanel.Path + "\\file"
						Cond = strFileName.CPtr();
					}

					break;
				}

				case MCODE_V_APANEL_UNCPATH: // APanel.UNCPath
				case MCODE_V_PPANEL_UNCPATH: // PPanel.UNCPath
				{
					Cond = L"";

					if (_MakePath1(CheckCode == MCODE_V_APANEL_UNCPATH?KEY_ALTSHIFTBRACKET:KEY_ALTSHIFTBACKBRACKET,strFileName,L""))
					{
						UnquoteExternal(strFileName);
						DeleteEndSlash(strFileName);
						Cond = strFileName.CPtr();
					}

					break;
				}
				//FILE_PANEL,TREE_PANEL,QVIEW_PANEL,INFO_PANEL
				case MCODE_V_APANEL_TYPE: // APanel.Type
				case MCODE_V_PPANEL_TYPE: // PPanel.Type
				{
					Panel *SelPanel = CheckCode == MCODE_V_APANEL_TYPE ? ActivePanel : PassivePanel;

					if (SelPanel )
						Cond=(__int64)SelPanel->GetType();

					break;
				}
				case MCODE_V_APANEL_DRIVETYPE: // APanel.DriveType - �������� ������: ��� �������
				case MCODE_V_PPANEL_DRIVETYPE: // PPanel.DriveType - ��������� ������: ��� �������
				{
					Panel *SelPanel = CheckCode == MCODE_V_APANEL_DRIVETYPE ? ActivePanel : PassivePanel;
					Cond=-1;

					if (SelPanel  && SelPanel->GetMode() != PLUGIN_PANEL)
					{
						SelPanel->GetCurDir(strFileName);
						GetPathRoot(strFileName, strFileName);
						UINT DriveType=FAR_GetDriveType(strFileName,nullptr,0);

						if (IsLocalPath(strFileName))
						{
							string strRemoteName;
							strFileName.SetLength(2);

							if (GetSubstName(DriveType,strFileName,strRemoteName))
								DriveType=DRIVE_SUBSTITUTE;
						}

						Cond=TVar((__int64)DriveType);
					}

					break;
				}
				case MCODE_V_APANEL_ITEMCOUNT: // APanel.ItemCount
				case MCODE_V_PPANEL_ITEMCOUNT: // PPanel.ItemCount
				{
					Panel *SelPanel = CheckCode == MCODE_V_APANEL_ITEMCOUNT ? ActivePanel : PassivePanel;

					if (SelPanel )
						Cond=(__int64)SelPanel->GetFileCount();

					break;
				}
				case MCODE_V_APANEL_CURPOS: // APanel.CurPos
				case MCODE_V_PPANEL_CURPOS: // PPanel.CurPos
				{
					Panel *SelPanel = CheckCode == MCODE_V_APANEL_CURPOS ? ActivePanel : PassivePanel;

					if (SelPanel )
						Cond=SelPanel->GetCurrentPos()+(SelPanel->GetFileCount()>0?1:0);

					break;
				}
				case MCODE_V_TITLE: // Title
				{
					Frame *f=FrameManager->GetTopModal();

					if (f)
					{
						if (CtrlObject->Cp() == f)
						{
							ActivePanel->GetTitle(strFileName);
						}
						else
						{
							string strType;

							switch (f->GetTypeAndName(strType,strFileName))
							{
								case MODALTYPE_EDITOR:
								case MODALTYPE_VIEWER:
									f->GetTitle(strFileName);
									break;
							}
						}

						RemoveExternalSpaces(strFileName);
					}

					Cond=strFileName.CPtr();
					break;
				}
				case MCODE_V_HEIGHT:  // Height - ������ �������� �������
				case MCODE_V_WIDTH:   // Width - ������ �������� �������
				{
					Frame *f=FrameManager->GetTopModal();

					if (f)
					{
						int X1, Y1, X2, Y2;
						f->GetPosition(X1,Y1,X2,Y2);

						if (CheckCode == MCODE_V_HEIGHT)
							Cond = (__int64)(Y2-Y1+1);
						else
							Cond = (__int64)(X2-X1+1);
					}

					break;
				}
				case MCODE_V_MENU_VALUE: // Menu.Value
				{
					int CurMMode=GetMode();
					Cond=L"";

					if (IsMenuArea(CurMMode) || CurMMode == MACRO_DIALOG)
					{
						Frame *f=FrameManager->GetCurrentFrame(), *fo=nullptr;

						//f=f->GetTopModal();
						while (f)
						{
							fo=f;
							f=f->GetTopModal();
						}

						if (!f)
							f=fo;

						if (f)
						{
							string NewStr;

							switch(CheckCode)
							{
								case MCODE_V_MENU_VALUE:
									if (f->VMProcess(CheckCode,&NewStr))
									{
										HiText2Str(strFileName, NewStr);
										RemoveExternalSpaces(strFileName);
										Cond=strFileName.CPtr();
									}
									break;
							}
						}
					}

					break;
				}
				case MCODE_V_ITEMCOUNT: // ItemCount - ����� ��������� � ������� �������
				case MCODE_V_CURPOS: // CurPos - ������� ������ � ������� �������
				{
					#if 1
						Frame *f=FrameManager->GetCurrentFrame(), *fo=nullptr;

						//f=f->GetTopModal();
						while (f)
						{
							fo=f;
							f=f->GetTopModal();
						}

						if (!f)
							f=fo;

						if (f)
						{
							Cond=f->VMProcess(CheckCode);
						}
					#else

						Frame *f=FrameManager->GetTopModal();

						if (f)
							Cond=(__int64)f->VMProcess(CheckCode);
					#endif
					break;
				}
				// *****************
				case MCODE_V_EDITORCURLINE: // Editor.CurLine - ������� ����� � ��������� (� ���������� � Count)
				case MCODE_V_EDITORSTATE:   // Editor.State
				case MCODE_V_EDITORLINES:   // Editor.Lines
				case MCODE_V_EDITORCURPOS:  // Editor.CurPos
				case MCODE_V_EDITORREALPOS: // Editor.RealPos
				case MCODE_V_EDITORFILENAME: // Editor.FileName
				case MCODE_V_EDITORVALUE:   // Editor.Value
				case MCODE_V_EDITORSELVALUE: // Editor.SelValue
				{
					if (CheckCode == MCODE_V_EDITORVALUE || CheckCode == MCODE_V_EDITORSELVALUE)
						Cond=L"";

					if (CtrlObject->Macro.GetMode()==MACRO_EDITOR && CtrlObject->Plugins.CurEditor && CtrlObject->Plugins.CurEditor->IsVisible())
					{
						if (CheckCode == MCODE_V_EDITORFILENAME)
						{
							string strType;
							CtrlObject->Plugins.CurEditor->GetTypeAndName(strType, strFileName);
							Cond=strFileName.CPtr();
						}
						else if (CheckCode == MCODE_V_EDITORVALUE)
						{
							EditorGetString egs;
							egs.StringNumber=-1;
							CtrlObject->Plugins.CurEditor->EditorControl(ECTL_GETSTRING,&egs);
							Cond=egs.StringText;
						}
						else if (CheckCode == MCODE_V_EDITORSELVALUE)
						{
							CtrlObject->Plugins.CurEditor->VMProcess(CheckCode,&strFileName);
							Cond=strFileName.CPtr();
						}
						else
							Cond=CtrlObject->Plugins.CurEditor->VMProcess(CheckCode);
					}

					break;
				}
				case MCODE_V_HELPFILENAME:  // Help.FileName
				case MCODE_V_HELPTOPIC:     // Help.Topic
				case MCODE_V_HELPSELTOPIC:  // Help.SelTopic
				{
					Cond=L"";

					if (CtrlObject->Macro.GetMode() == MACRO_HELP)
					{
						CurFrame->VMProcess(CheckCode,&strFileName,0);
						Cond=strFileName.CPtr();
					}

					break;
				}
				case MCODE_V_VIEWERFILENAME: // Viewer.FileName
				case MCODE_V_VIEWERSTATE: // Viewer.State
				{
					if (CheckCode == MCODE_V_VIEWERFILENAME)
						Cond=L"";

					if ((CtrlObject->Macro.GetMode()==MACRO_VIEWER || CtrlObject->Macro.GetMode()==MACRO_QVIEWPANEL) &&
					        CtrlObject->Plugins.CurViewer && CtrlObject->Plugins.CurViewer->IsVisible())
					{
						if (CheckCode == MCODE_V_VIEWERFILENAME)
						{
							CtrlObject->Plugins.CurViewer->GetFileName(strFileName);//GetTypeAndName(nullptr,FileName);
							Cond=strFileName.CPtr();
						}
						else
							Cond=CtrlObject->Plugins.CurViewer->VMProcess(MCODE_V_VIEWERSTATE);
					}

					break;
				}
			}

			break;
		}
		default:
		{
			Err=1;
			break;
		}
	}

	_KEYMACRO(SysLog(L"return; Err=%d",Err));
	return Cond;
}


/* ------------------------------------------------------------------- */
// S=trim(S[,N])
static bool trimFunc(const TMacroFunction*)
{
	int  mode = (int) VMStack.Pop().getInteger();
	TVar Val;
	VMStack.Pop(Val);
	wchar_t *p = (wchar_t *)Val.toString();
	bool Ret=true;

	switch (mode)
	{
		case 0: p=RemoveExternalSpaces(p); break;  // alltrim
		case 1: p=RemoveLeadingSpaces(p); break;   // ltrim
		case 2: p=RemoveTrailingSpaces(p); break;  // rtrim
		default: Ret=false;
	}

	VMStack.Push(p);
	return Ret;
}

// S=substr(S,start[,length])
static bool substrFunc(const TMacroFunction*)
{
	/*
		TODO: http://bugs.farmanager.com/view.php?id=1480
			���� start  >= 0, �� ������� ���������, ������� �� start-������� �� ������ ������.
			���� start  <  0, �� ������� ���������, ������� �� start-������� �� ����� ������.
			���� length >  0, �� ������������ ��������� ����� �������� �������� �� length �������� �������� ������ ������� � start
			���� length <  0, �� � ������������ ��������� ����� ������������� length �������� �� ����� �������� ������, ��� ���, ��� ��� ����� ���������� � ������� start.
								���: length - ����� ����, ��� ����� (���� >=0) ��� ����������� (���� <0).

			������ ������ ������������:
				���� length = 0
				���� ...
	*/
	bool Ret=false;

	TVar VarLength;  VMStack.Pop(VarLength);
	int  start     = (int)VMStack.Pop().getInteger();
	TVar Val;        VMStack.Pop(Val);

	wchar_t *p = (wchar_t *)Val.toString();
	int length_str = StrLength(p);

	int length=VarLength.isUnknown()?length_str:(int)VarLength.getInteger();


	if (length)
	{
		if (start < 0)
		{
			start=length_str+start;
			if (start < 0)
				start=0;
		}

		if (start >= length_str)
		{
			length=0;
		}
		else
		{
			if (length > 0)
			{
				if (start+length >= length_str)
					length=length_str-start;
			}
			else
			{
				length=length_str-start+length;

				if (length < 0)
				{
					length=0;
				}
			}
		}
	}

	if (!length)
	{
		VMStack.Push(L"");
	}
	else
	{
		p += start;
		p[length] = 0;
		Ret=true;
		VMStack.Push(p);
	}

	return Ret;
}

static BOOL SplitFileName(const wchar_t *lpFullName,string &strDest,int nFlags)
{
#define FLAG_DISK   1
#define FLAG_PATH   2
#define FLAG_NAME   4
#define FLAG_EXT    8
	const wchar_t *s = lpFullName; //start of sub-string
	const wchar_t *p = s; //current string pointer
	const wchar_t *es = s+StrLength(s); //end of string
	const wchar_t *e; //end of sub-string

	if (!*p)
		return FALSE;

	if ((*p == L'\\') && (*(p+1) == L'\\'))   //share
	{
		p += 2;
		p = wcschr(p, L'\\');

		if (!p)
			return FALSE; //invalid share (\\server\)

		p = wcschr(p+1, L'\\');

		if (!p)
			p = es;

		if ((nFlags & FLAG_DISK) == FLAG_DISK)
		{
			strDest=s;
			strDest.SetLength(p-s);
		}
	}
	else
	{
		if (*(p+1) == L':')
		{
			p += 2;

			if ((nFlags & FLAG_DISK) == FLAG_DISK)
			{
				size_t Length=strDest.GetLength()+p-s;
				strDest+=s;
				strDest.SetLength(Length);
			}
		}
	}

	e = nullptr;
	s = p;

	while (p)
	{
		p = wcschr(p, L'\\');

		if (p)
		{
			e = p;
			p++;
		}
	}

	if (e)
	{
		if ((nFlags & FLAG_PATH))
		{
			size_t Length=strDest.GetLength()+e-s;
			strDest+=s;
			strDest.SetLength(Length);
		}

		s = e+1;
		p = s;
	}

	if (!p)
		p = s;

	e = nullptr;

	while (p)
	{
		p = wcschr(p+1, L'.');

		if (p)
			e = p;
	}

	if (!e)
		e = es;

	if (!strDest.IsEmpty())
		AddEndSlash(strDest);

	if (nFlags & FLAG_NAME)
	{
		const wchar_t *ptr=wcspbrk(s,L":");

		if (ptr)
			s=ptr+1;

		size_t Length=strDest.GetLength()+e-s;
		strDest+=s;
		strDest.SetLength(Length);
	}

	if (nFlags & FLAG_EXT)
		strDest+=e;

	return TRUE;
}


// S=fsplit(S,N)
static bool fsplitFunc(const TMacroFunction*)
{
	int m = (int)VMStack.Pop().getInteger();
	TVar Val;
	VMStack.Pop(Val);
	const wchar_t *s = Val.toString();
	bool Ret=false;
	string strPath;

	if (!SplitFileName(s,strPath,m))
		strPath.Clear();
	else
		Ret=true;

	VMStack.Push(strPath.CPtr());
	return Ret;
}

#if 0
// S=Meta("!.!") - � �������� ����� ������ �����������
static bool metaFunc(const TMacroFunction*)
{
	TVar Val;
	VMStack.Pop(Val);
	const wchar_t *s = Val.toString();

	if (s && *s)
	{
		char SubstText[512];
		char Name[NM],ShortName[NM];
		xstrncpy(SubstText,s,sizeof(SubstText));
		SubstFileName(SubstText,sizeof(SubstText),Name,ShortName,nullptr,nullptr,TRUE);
		return TVar(SubstText);
	}

	return TVar(L"");
}
#endif


// N=atoi(S[,radix])
static bool atoiFunc(const TMacroFunction*)
{
	bool Ret=true;
	wchar_t *endptr;
	TVar R, S;
	VMStack.Pop(R);
	VMStack.Pop(S);
	VMStack.Push(TVar(_wcstoi64(S.toString(),&endptr,(int)R.toInteger())));
	return Ret;
}


// N=Window.Scroll(Lines[,Axis])
static bool windowscrollFunc(const TMacroFunction*)
{
	bool Ret=false;
	TVar A, L;
	VMStack.Pop(A); // 0 - ��������� (�� ���������), 1 - �����������.
	VMStack.Pop(L); // ������������� ����� - ����� (����/������), ������������� - ����� (�����/�����).

	if (Opt.WindowMode)
	{
		int Lines=(int)L.i(), Columns=0;
		L=0;
		if (A.i())
		{
			Columns=Lines;
			Lines=0;
		}

		if (Console.ScrollWindow(Lines, Columns))
		{
			Ret=true;
			L=1;
		}
	}
	else
		L=0;

	VMStack.Push(L);
	return Ret;
}

// S=itoa(N[,radix])
static bool itowFunc(const TMacroFunction*)
{
	bool Ret=false;
	TVar R, N;
	VMStack.Pop(R);
	VMStack.Pop(N);

	if (N.isInteger())
	{
		wchar_t value[65];
		int Radix=(int)R.toInteger();

		if (!Radix)
			Radix=10;

		Ret=true;
		N=TVar(_i64tow(N.toInteger(),value,Radix));
	}

	VMStack.Push(N);
	return Ret;
}

// N=sleep(N)
static bool sleepFunc(const TMacroFunction*)
{
	long Period=(long)VMStack.Pop().getInteger();

	if (Period > 0)
	{
		Sleep((DWORD)Period);
		VMStack.Push(1);
		return true;
	}

	VMStack.Push(0ll);
	return false;
}


// N=KeyBar.Show([N])
static bool keybarshowFunc(const TMacroFunction*)
{
	/*
	Mode:
		0 - visible?
			ret: 0 - hide, 1 - show, -1 - KeyBar not found
		1 - show
		2 - hide
		3 - swap
		ret: prev mode or -1 - KeyBar not found
    */
	Frame *f=FrameManager->GetCurrentFrame(), *fo=nullptr;

	//f=f->GetTopModal();
	while (f)
	{
		fo=f;
		f=f->GetTopModal();
	}

	if (!f)
		f=fo;

	VMStack.Push(f?f->VMProcess(MCODE_F_KEYBAR_SHOW,nullptr,VMStack.Pop().getInteger())-1:-1);
	return f?true:false;
}


// S=key(V)
static bool keyFunc(const TMacroFunction*)
{
	TVar VarKey;
	VMStack.Pop(VarKey);
	string strKeyText;

	if (VarKey.isInteger())
	{
		if (VarKey.i())
			KeyToText((int)VarKey.i(),strKeyText);
	}
	else
	{
		// ��������...
		DWORD Key=(DWORD)KeyNameToKey(VarKey.s());

		if (Key != (DWORD)-1 && Key==(DWORD)VarKey.i())
			strKeyText=VarKey.s();
	}

	VMStack.Push(strKeyText.CPtr());
	return !strKeyText.IsEmpty()?true:false;
}

// V=waitkey([N,[T]])
static bool waitkeyFunc(const TMacroFunction*)
{
	long Type=(long)VMStack.Pop().getInteger();
	long Period=(long)VMStack.Pop().getInteger();
	DWORD Key=WaitKey((DWORD)-1,Period);

	if (!Type)
	{
		string strKeyText;

		if (Key != KEY_NONE)
			if (!KeyToText(Key,strKeyText))
				strKeyText.Clear();

		VMStack.Push(strKeyText.CPtr());
		return !strKeyText.IsEmpty()?true:false;
	}

	if (Key == KEY_NONE)
		Key=-1;

	VMStack.Push((__int64)Key);
	return Key != (DWORD)-1;
}

// n=min(n1,n2)
static bool minFunc(const TMacroFunction*)
{
	TVar V2, V1;
	VMStack.Pop(V2);
	VMStack.Pop(V1);
	VMStack.Push(V2 < V1 ? V2 : V1);
	return true;
}

// n=max(n1.n2)
static bool maxFunc(const TMacroFunction*)
{
	TVar V2, V1;
	VMStack.Pop(V2);
	VMStack.Pop(V1);
	VMStack.Push(V2 > V1  ? V2 : V1);
	return true;
}

// n=mod(n1,n2)
static bool modFunc(const TMacroFunction*)
{
	TVar V2, V1;
	VMStack.Pop(V2);
	VMStack.Pop(V1);

	if (!V2.i())
	{
		_KEYMACRO(___FILEFUNCLINE___;SysLog(L"Error: Divide (mod) by zero"));
		VMStack.Push(0ll);
		return false;
	}

	VMStack.Push(V1 % V2);
	return true;
}

// n=iif(expression,n1,n2)
static bool iifFunc(const TMacroFunction*)
{
	TVar V2, V1;
	VMStack.Pop(V2);
	VMStack.Pop(V1);
	VMStack.Push(__CheckCondForSkip(MCODE_OP_JZ) ? V2 : V1);
	return true;
}

// N=index(S1,S2[,Mode])
static bool indexFunc(const TMacroFunction*)
{
	TVar Mode;  VMStack.Pop(Mode);
	TVar S2;    VMStack.Pop(S2);
	TVar S1;    VMStack.Pop(S1);

	const wchar_t *s = S1.toString();
	const wchar_t *p = S2.toString();
	const wchar_t *i = !Mode.getInteger() ? StrStrI(s,p) : StrStr(s,p);
	bool Ret= i ? true : false;
	VMStack.Push(TVar((__int64)(i ? i-s : -1)));
	return Ret;
}

// S=rindex(S1,S2[,Mode])
static bool rindexFunc(const TMacroFunction*)
{
	TVar Mode;  VMStack.Pop(Mode);
	TVar S2;    VMStack.Pop(S2);
	TVar S1;    VMStack.Pop(S1);

	const wchar_t *s = S1.toString();
	const wchar_t *p = S2.toString();
	const wchar_t *i = !Mode.getInteger() ? RevStrStrI(s,p) : RevStrStr(s,p);
	bool Ret= i ? true : false;
	VMStack.Push(TVar((__int64)(i ? i-s : -1)));
	return Ret;
}

// S=date([S])
static bool dateFunc(const TMacroFunction*)
{
	TVar Val;
	VMStack.Pop(Val);

	if (Val.isInteger() && !Val.i())
		Val=L"";

	const wchar_t *s = Val.toString();
	bool Ret=false;
	string strTStr;

	if (MkStrFTime(strTStr,s))
		Ret=true;
	else
		strTStr.Clear();

	VMStack.Push(TVar(strTStr.CPtr()));
	return Ret;
}

// S=xlat(S[,Flags])
/*
  Flags:
  	XLAT_SWITCHKEYBLAYOUT  = 1
	XLAT_SWITCHKEYBBEEP    = 2
	XLAT_USEKEYBLAYOUTNAME = 4
*/
static bool xlatFunc(const TMacroFunction*)
{
	TVar Flags; VMStack.Pop(Flags);
	TVar Val;   VMStack.Pop(Val);

	wchar_t *Str = (wchar_t *)Val.toString();
	bool Ret=::Xlat(Str,0,StrLength(Str),Flags.i())?true:false;
	VMStack.Push(TVar(Str));
	return Ret;
}

// N=beep([N])
static bool beepFunc(const TMacroFunction*)
{
	TVar Val;
	VMStack.Pop(Val);
	/*
		MB_ICONASTERISK = 0x00000040
			���� ���������
		MB_ICONEXCLAMATION = 0x00000030
		    ���� �����������
		MB_ICONHAND = 0x00000010
		    ���� ����������� ������
		MB_ICONQUESTION = 0x00000020
		    ���� ������
		MB_OK = 0x0
		    ����������� ����
		SIMPLE_BEEP = 0xffffffff
		    ���������� �������
	*/
	bool Ret=MessageBeep((UINT)Val.i())?true:false;

	/*
		http://msdn.microsoft.com/en-us/library/dd743680%28VS.85%29.aspx
		BOOL PlaySound(
	    	LPCTSTR pszSound,
	    	HMODULE hmod,
	    	DWORD fdwSound
		);

		http://msdn.microsoft.com/en-us/library/dd798676%28VS.85%29.aspx
		BOOL sndPlaySound(
	    	LPCTSTR lpszSound,
	    	UINT fuSound
		);
	*/

	VMStack.Push(Ret?1:0);
	return Ret;
}

/*
Res=kbdLayout([N])

�������� N:
�) ����������: 0x0409 ��� 0x0419 ���...
�) 1 - ��������� ��������� (�� �����)
�) -1 - ���������� ��������� (�� �����)
�) 0 ��� �� ������ - ������� ������� ���������.

���������� ���������� ��������� (��� N=0 �������)
*/
// N=kbdLayout([N])
static bool kbdLayoutFunc(const TMacroFunction*)
{
	DWORD dwLayout = (DWORD)VMStack.Pop().getInteger();

	BOOL Ret=TRUE;
	HKL  Layout=(HKL)0, RetLayout=(HKL)0;

	if (ifn.pfnGetConsoleKeyboardLayoutName)
	{
		wchar_t LayoutName[1024]={}; // BUGBUG!!!
		if (ifn.pfnGetConsoleKeyboardLayoutName(LayoutName))
		{
			wchar_t *endptr;
			DWORD res=(DWORD)wcstoul(LayoutName, &endptr, 16);
			RetLayout=(HKL)(INT_PTR)(HIWORD(res)? res : MAKELONG(res,res));
		}
	}

	HWND hWnd = Console.GetWindow();

	if (hWnd && dwLayout)
	{
		WPARAM wParam;

		if ((long)dwLayout == -1)
		{
			wParam=INPUTLANGCHANGE_BACKWARD;
			Layout=(HKL)HKL_PREV;
		}
		else if (dwLayout == 1)
		{
			wParam=INPUTLANGCHANGE_FORWARD;
			Layout=(HKL)HKL_NEXT;
		}
		else
		{
			wParam=0;
			Layout=(HKL)(INT_PTR)(HIWORD(dwLayout)? dwLayout : MAKELONG(dwLayout,dwLayout));
		}

		Ret=PostMessage(hWnd,WM_INPUTLANGCHANGEREQUEST, wParam, (LPARAM)Layout);
	}

	VMStack.Push(Ret?TVar(static_cast<INT64>(reinterpret_cast<INT_PTR>(RetLayout))):0);

	return Ret?true:false;
}

// S=prompt("Title"[,"Prompt"[,flags[, "Src"[, "History"]]]])
static bool promptFunc(const TMacroFunction*)
{
	TVar ValHistory;
	VMStack.Pop(ValHistory);
	TVar ValSrc;
	VMStack.Pop(ValSrc);
	DWORD Flags = (DWORD)VMStack.Pop().getInteger();
	TVar ValPrompt;
	VMStack.Pop(ValPrompt);
	TVar ValTitle;
	VMStack.Pop(ValTitle);
	TVar Result(L"");
	bool Ret=false;

	if (!(ValTitle.isInteger() && !ValTitle.i()))
	{
		const wchar_t *history=nullptr;

		if (!(ValHistory.isInteger() && !ValHistory.i()))
			history=ValHistory.s();

		const wchar_t *src=L"";

		if (!(ValSrc.isInteger() && !ValSrc.i()))
			src=ValSrc.s();

		const wchar_t *prompt=L"";

		if (!(ValPrompt.isInteger() && !ValPrompt.i()))
			prompt=ValPrompt.s();

		const wchar_t *title=NullToEmpty(ValTitle.toString());
		string strDest;

		if (GetString(title,prompt,history,src,strDest,nullptr,Flags&~FIB_CHECKBOX,nullptr,nullptr))
		{
			Result=strDest.CPtr();
			Result.toString();
			Ret=true;
		}
	}

	VMStack.Push(Result);
	return Ret;
}

// N=msgbox(["Title"[,"Text"[,flags]]])
static bool msgBoxFunc(const TMacroFunction*)
{
	DWORD Flags = (DWORD)VMStack.Pop().getInteger();
	TVar ValB, ValT;
	VMStack.Pop(ValB);
	VMStack.Pop(ValT);
	const wchar_t *title = L"";

	if (!(ValT.isInteger() && !ValT.i()))
		title=NullToEmpty(ValT.toString());

	const wchar_t *text  = L"";

	if (!(ValB.isInteger() && !ValB.i()))
		text =NullToEmpty(ValB.toString());

	Flags&=~(FMSG_KEEPBACKGROUND|FMSG_ERRORTYPE);
	Flags|=FMSG_ALLINONE;

	if (!HIWORD(Flags) || HIWORD(Flags) > HIWORD(FMSG_MB_RETRYCANCEL))
		Flags|=FMSG_MB_OK;

	//_KEYMACRO(SysLog(L"title='%s'",title));
	//_KEYMACRO(SysLog(L"text='%s'",text));
	string TempBuf = title;
	TempBuf += L"\n";
	TempBuf += text;
	int Result=FarMessageFn(-1,Flags,nullptr,(const wchar_t * const *)TempBuf.CPtr(),0,0)+1;
	VMStack.Push((__int64)Result);
	return true;
}


static int __cdecl CompareItems(const MenuItemEx **el1, const MenuItemEx **el2, const SortItemParam *Param)
{
	if (((*el1)->Flags & LIF_SEPARATOR) || ((*el2)->Flags & LIF_SEPARATOR))
		return 0;

	string strName1((*el1)->strName);
	string strName2((*el2)->strName);
	RemoveChar(strName1,L'&',TRUE);
	RemoveChar(strName2,L'&',TRUE);
	int Res = NumStrCmpI(strName1.CPtr()+Param->Offset,strName2.CPtr()+Param->Offset);
	return (Param->Direction?(Res<0?1:(Res>0?-1:0)):Res);
}

//S=Menu.Show(Items[,Title[,Flags[,FindOrFilter[,X[,Y]]]]])
//Flags:
//0x001 - BoxType
//0x002 - BoxType
//0x004 - BoxType
//0x008 - ������������ ��������� - ������ ��� ������
//0x010 - ��������� ������� ���������� �������
//0x020 - ������������� (� ������ ��������)
//0x040 - ������� ������������� ������
//0x080 - ������������� ��������� ������ |= VMENU_AUTOHIGHLIGHT
//0x100 - FindOrFilter - ����� ��� �������������
//0x200 - �������������� ��������� �����
//0x400 - ����������� ���������� ����� ����
//0x800 - ������������ ������ �� �����
static bool menushowFunc(const TMacroFunction*)
{
	TVar VY; VMStack.Pop(VY);
	TVar VX; VMStack.Pop(VX);
	TVar VFindOrFilter; VMStack.Pop(VFindOrFilter);
	DWORD Flags = (DWORD)VMStack.Pop().getInteger();
	TVar Title; VMStack.Pop(Title);

	if (Title.isUnknown())
		Title=L"";

	string strTitle=Title.toString();
	string strBottom;
	TVar Items; VMStack.Pop(Items);
	string strItems = Items.toString();
	ReplaceStrings(strItems,L"\r\n",L"\n");

	if (!strItems.Equal(strItems.GetLength()-1,L"\n"))
		strItems.Append(L"\n");

	TVar Result = -1;
	int BoxType = (Flags & 0x7)?(Flags & 0x7)-1:3;
	bool bResultAsIndex = (Flags & 0x08)?true:false;
	bool bMultiSelect = (Flags & 0x010)?true:false;
	bool bSorting = (Flags & 0x20)?true:false;
	bool bPacking = (Flags & 0x40)?true:false;
	bool bAutohighlight = (Flags & 0x80)?true:false;
	bool bSetMenuFilter = (Flags & 0x100)?true:false;
	bool bAutoNumbering = (Flags & 0x200)?true:false;
	bool bExitAfterNavigate = (Flags & 0x400)?true:false;
	bool bFilterMaskMode = (Flags & 0x800)?true:false;
	int nLeftShift=bAutoNumbering?9:0;
	int X = -1;
	int Y = -1;
	unsigned __int64 MenuFlags = VMENU_WRAPMODE;

	if (!VX.isUnknown())
		X=VX.toInteger();

	if (!VY.isUnknown())
		Y=VY.toInteger();

	if (bAutohighlight)
		MenuFlags |= VMENU_AUTOHIGHLIGHT;

	int SelectedPos=0;
	int LineCount=0;
	size_t CurrentPos=0;
	size_t PosLF;
	size_t SubstrLen;
	ReplaceStrings(strTitle,L"\r\n",L"\n");
	bool CRFound=strTitle.Pos(PosLF, L"\n");

	if(CRFound)
	{
		strBottom=strTitle.SubStr(PosLF+1);
		strTitle=strTitle.SubStr(0,PosLF);
	}
	VMenu Menu(strTitle.CPtr(),nullptr,0,ScrY-4);
	Menu.SetBottomTitle(strBottom.CPtr());
	Menu.SetFlags(MenuFlags);
	Menu.SetPosition(X,Y,0,0);
	Menu.SetBoxType(BoxType);

	CRFound=strItems.Pos(PosLF, L"\n");
	while(CRFound)
	{
		MenuItemEx NewItem;
		NewItem.Clear();
		SubstrLen=PosLF-CurrentPos;

		if (SubstrLen==0)
			SubstrLen=1;

		NewItem.strName=strItems.SubStr(CurrentPos,SubstrLen);

		if (NewItem.strName!=L"\n")
		{
			wchar_t *CurrentChar=(wchar_t *)NewItem.strName.CPtr();
			bool bContunue=(*CurrentChar<=L'\x4');
			while(*CurrentChar && bContunue)
			{
				switch (*CurrentChar)
				{
					case L'\x1':
						NewItem.Flags|=LIF_SEPARATOR;
						CurrentChar++;
						break;

					case L'\x2':
						NewItem.Flags|=LIF_CHECKED;
						CurrentChar++;
						break;

					case L'\x3':
						NewItem.Flags|=LIF_DISABLE;
						CurrentChar++;
						break;

					case L'\x4':
						NewItem.Flags|=LIF_GRAYED;
						CurrentChar++;
						break;

					default:
						bContunue=false;
						CurrentChar++;
						break;
				}
			}
			NewItem.strName=CurrentChar;
		}
		else
			NewItem.strName.Clear();

		if (bAutoNumbering && !(bSorting || bPacking) && !(NewItem.Flags & LIF_SEPARATOR))
		{
			LineCount++;
			NewItem.strName.Format(L"%6d - %s", LineCount, NewItem.strName.CPtr());
		}
		Menu.AddItem(&NewItem);
		CurrentPos=PosLF+1;
		CRFound=strItems.Pos(PosLF, L"\n",CurrentPos);
	}

	strItems.Clear();

	if (bSorting)
		Menu.SortItems(reinterpret_cast<TMENUITEMEXCMPFUNC>(CompareItems));

	if (bPacking)
		Menu.Pack();

	if ((bAutoNumbering) && (bSorting || bPacking))
	{
		for (int i = 0; i < Menu.GetShowItemCount(); i++)
		{
			MenuItemEx *Item=Menu.GetItemPtr(i);
			if (!(Item->Flags & LIF_SEPARATOR))
			{
				LineCount++;
				Item->strName.Format(L"%6d - %s", LineCount, Item->strName.CPtr());
			}
		}
	}

	if (!VFindOrFilter.isUnknown())
	{
		if (bSetMenuFilter)
		{
			Menu.SetFilterEnabled(true);
			Menu.SetFilterMaskMode(bFilterMaskMode);
			Menu.SetFilterString(VFindOrFilter.toString());
			Menu.FilterStringUpdated();
			Menu.Show();
		}
		else
		{
			if (VFindOrFilter.isInteger())
			{
				if (VFindOrFilter.toInteger()-1>=0)
					Menu.SetSelectPos(VFindOrFilter.toInteger()-1,1);
				else
					Menu.SetSelectPos(Menu.GetItemCount()+VFindOrFilter.toInteger(),1);
			}
			else
				if (VFindOrFilter.isString())
					Menu.SetSelectPos(Max(0,Menu.FindItem(0, VFindOrFilter.toString())),1);
		}
	}

	Frame *frame;

	if ((frame=FrameManager->GetBottomFrame()) )
		frame->Lock();

	Menu.Show();
	int PrevSelectedPos=Menu.GetSelectPos();
	DWORD Key=0;
	int RealPos;
	bool CheckFlag;
	int X1, Y1, X2, Y2, NewY2;
	while (!Menu.Done() && !CloseFARMenu)
	{
		SelectedPos=Menu.GetSelectPos();
		Key=Menu.ReadInput();
		switch (Key)
		{
			case KEY_NUMPAD0:
			case KEY_INS:
				if (bMultiSelect)
				{
					Menu.SetCheck(!Menu.GetCheck(SelectedPos));
					Menu.Show();
				}
				break;

			case KEY_CTRLADD:
			case KEY_CTRLSUBTRACT:
			case KEY_CTRLMULTIPLY:
				if (bMultiSelect)
				{
					for(int i=0; i<Menu.GetShowItemCount(); i++)
					{
						RealPos=Menu.VisualPosToReal(i);
						if (Key==KEY_CTRLMULTIPLY)
						{
							CheckFlag=Menu.GetCheck(RealPos)?false:true;
						}
						else
						{
							CheckFlag=(Key==KEY_CTRLADD);
						}
						Menu.SetCheck(CheckFlag, RealPos);
					}
					Menu.Show();
				}
				break;

			case KEY_CTRLA:
			{
				Menu.GetPosition(X1, Y1, X2, Y2);
				NewY2=Y1+Menu.GetShowItemCount()+1;

				if (NewY2>ScrY-2)
					NewY2=ScrY-2;

				Menu.SetPosition(X1,Y1,X2,NewY2);
				Menu.Show();
				break;
			}

			case KEY_CTRLC:     case KEY_CTRLINS:    case KEY_CTRLNUMPAD0:
			{
				if (bMultiSelect)
				{
					strItems.Clear();
					for(int i=0; i<Menu.GetShowItemCount(); i++)
					{
						RealPos=Menu.VisualPosToReal(i);
						if (Menu.GetCheck(RealPos))
						{
							strItems+=(*Menu.GetItemPtr(RealPos)).strName.CPtr();
							strItems+=L"\n";
						}
					}

					if(strItems.IsEmpty())
					{
						if(Menu.GetShowItemCount()>0)
							CopyToClipboard((*Menu.GetItemPtr(SelectedPos)).strName.CPtr());
					}
					else
						CopyToClipboard(strItems.SubStr(0, strItems.GetLength()-1));
				}
				else
				{
					if(Menu.GetShowItemCount()>0)
						CopyToClipboard((*Menu.GetItemPtr(SelectedPos)).strName.CPtr());
				}
				break;
			}

			case KEY_CTRLSHIFTV:     case KEY_CTRLSHIFTINS:    case KEY_CTRLSHIFTNUMPAD0:
			{
				if (bMultiSelect)
				{
					strItems=PasteFromClipboard();
					ReplaceStrings(strItems,L"\r\n",L"\n");
					strItems=L"\n"+strItems+L"\n";
					for(int i=0; i<Menu.GetShowItemCount(); i++)
					{
						RealPos=Menu.VisualPosToReal(i);
						strTitle=((*Menu.GetItemPtr(RealPos)).strName.CPtr());
						if(strItems.Pos(CurrentPos, strTitle.CPtr()) && strItems.Equal(CurrentPos-1, L"\n") && strItems.Equal(CurrentPos+strTitle.GetLength(), L"\n"))
						{
							Menu.SetCheck(true,RealPos);
						}
					}
					Menu.Show();
				}
				break;
			}

			case KEY_BREAK:
				CtrlObject->Macro.SendDropProcess();
				Menu.SetExitCode(-1);
				break;

			default:
				Menu.ProcessInput();
				break;
		}

		if (bExitAfterNavigate && (PrevSelectedPos!=SelectedPos))
		{
			SelectedPos=Menu.GetSelectPos();
			break;
		}

		PrevSelectedPos=SelectedPos;
	}

	wchar_t temp[65];

	if (Menu.Modal::GetExitCode() >= 0)
	{
		SelectedPos=Menu.GetExitCode();
		if (bMultiSelect)
		{
			Result=L"";
			for(int i=0; i<Menu.GetItemCount(); i++)
			{
				if (Menu.GetCheck(i))
				{
					if (bResultAsIndex)
					{
						_i64tow(i+1,temp,10);
						Result+=temp;
					}
					else
						Result+=(*Menu.GetItemPtr(i)).strName.CPtr()+nLeftShift;
					Result+=L"\n";
				}
			}
			if(Result==L"")
			{
				if (bResultAsIndex)
				{
					_i64tow(SelectedPos+1,temp,10);
					Result=temp;
				}
				else
					Result=(*Menu.GetItemPtr(SelectedPos)).strName.CPtr()+nLeftShift;
			}
		}
		else
			if(!bResultAsIndex)
				Result=(*Menu.GetItemPtr(SelectedPos)).strName.CPtr()+nLeftShift;
			else
				Result=SelectedPos+1;
		Menu.Hide();
	}
	else
	{
		Menu.Hide();
		if (bExitAfterNavigate)
		{
			Result=SelectedPos+1;
			if ((Key == KEY_ESC) || (Key == KEY_F10) || (Key == KEY_BREAK))
				Result=-Result;
		}
		else
		{
			if(bResultAsIndex)
				Result=0;
			else
				Result=L"";
		}
	}

	if (frame )
		frame->Unlock();

	VMStack.Push(Result);
	return true;
}

// S=env(S)
static bool environFunc(const TMacroFunction*)
{
	TVar S;
	VMStack.Pop(S);
	bool Ret=false;
	string strEnv;

	if (apiGetEnvironmentVariable(S.toString(), strEnv))
		Ret=true;
	else
		strEnv.Clear();

	VMStack.Push(strEnv.CPtr());
	return Ret;
}

// V=Panel.Select(panelType,Action[,Mode[,Items]])
static bool panelselectFunc(const TMacroFunction*)
{
	TVar ValItems;  VMStack.Pop(ValItems);
	int Mode=(int)VMStack.Pop().getInteger();
	DWORD Action=(int)VMStack.Pop().getInteger();
	int typePanel=(int)VMStack.Pop().getInteger();
	__int64 Result=-1;

	Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
	Panel *PassivePanel=nullptr;

	if (ActivePanel)
		PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

	Panel *SelPanel = !typePanel ? ActivePanel : (typePanel == 1?PassivePanel:nullptr);

	if (SelPanel)
	{
		__int64 Index=-1;
		if (Mode == 1)
		{
			Index=ValItems.getInteger();
			if (!Index)
				Index=SelPanel->GetCurrentPos();
			else
				Index--;
		}

		if (Mode == 2 || Mode == 3)
		{
			string strStr=ValItems.s();
			ReplaceStrings(strStr,L"\r\n",L";");
			ReplaceStrings(strStr,L"\n",L";");
			ValItems=strStr.CPtr();
		}

		MacroPanelSelect mps;
		mps.Action      = Action & 0xF;
		mps.ActionFlags = (Action & (~0xF)) >> 4;
		mps.Mode        = Mode;
		mps.Index       = Index;
		mps.Item        = &ValItems;
		Result=SelPanel->VMProcess(MCODE_F_PANEL_SELECT,&mps,0);
	}

	VMStack.Push(Result);
	return Result==-1?false:true;
}

static bool _fattrFunc(int Type)
{
	bool Ret=false;
	DWORD FileAttr=INVALID_FILE_ATTRIBUTES;
	long Pos=-1;

	if (!Type || Type == 2) // �� ������: fattr(0) & fexist(2)
	{
		TVar Str;
		VMStack.Pop(Str);
		FAR_FIND_DATA_EX FindData;
		apiGetFindDataEx(Str.toString(), FindData);
		FileAttr=FindData.dwFileAttributes;
		Ret=true;
	}
	else // panel.fattr(1) & panel.fexist(3)
	{
		TVar S;
		VMStack.Pop(S);
		int typePanel=(int)VMStack.Pop().getInteger();
		const wchar_t *Str = S.toString();
		Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
		Panel *PassivePanel=nullptr;

		if (ActivePanel)
			PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

		//Frame* CurFrame=FrameManager->GetCurrentFrame();
		Panel *SelPanel = !typePanel ? ActivePanel : (typePanel == 1?PassivePanel:nullptr);

		if (SelPanel)
		{
			if (wcspbrk(Str,L"*?") )
				Pos=SelPanel->FindFirst(Str);
			else
				Pos=SelPanel->FindFile(Str,wcspbrk(Str,L"\\/:")?FALSE:TRUE);

			if (Pos >= 0)
			{
				string strFileName;
				SelPanel->GetFileName(strFileName,Pos,FileAttr);
				Ret=true;
			}
		}
	}

	if (Type == 2) // fexist(2)
		FileAttr=(FileAttr!=INVALID_FILE_ATTRIBUTES)?1:0;
	else if (Type == 3) // panel.fexist(3)
		FileAttr=(DWORD)Pos+1;

	VMStack.Push(TVar((__int64)(long)FileAttr));
	return Ret;
}

// N=fattr(S)
static bool fattrFunc(const TMacroFunction*)
{
	return _fattrFunc(0);
}

// N=fexist(S)
static bool fexistFunc(const TMacroFunction*)
{
	return _fattrFunc(2);
}

// N=panel.fattr(S)
static bool panelfattrFunc(const TMacroFunction*)
{
	return _fattrFunc(1);
}

// N=panel.fexist(S)
static bool panelfexistFunc(const TMacroFunction*)
{
	return _fattrFunc(3);
}

// N=FLock(Nkey,NState)
/*
  Nkey:
     0 - NumLock
     1 - CapsLock
     2 - ScrollLock

  State:
    -1 get state
     0 off
     1 on
     2 flip
*/
static bool flockFunc(const TMacroFunction*)
{
	TVar Ret(-1);
	int stateFLock=(int)VMStack.Pop().getInteger();
	UINT vkKey=(UINT)VMStack.Pop().getInteger();

	switch (vkKey)
	{
		case 0:
			vkKey=VK_NUMLOCK;
			break;
		case 1:
			vkKey=VK_CAPITAL;
			break;
		case 2:
			vkKey=VK_SCROLL;
			break;
		default:
			vkKey=0;
			break;
	}

	if (vkKey)
		Ret=(__int64)SetFLockState(vkKey,stateFLock);

	VMStack.Push(Ret);
	return Ret.i()!=-1;
}

// V=Dlg.GetValue(ID,N)
static bool dlggetvalueFunc(const TMacroFunction*)
{
	TVar Ret(-1);
	int TypeInf=(int)VMStack.Pop().getInteger();
	unsigned Index=(unsigned)VMStack.Pop().getInteger()-1;
	Frame* CurFrame=FrameManager->GetCurrentFrame();

	if (CtrlObject->Macro.GetMode()==MACRO_DIALOG && CurFrame && CurFrame->GetType()==MODALTYPE_DIALOG)
	{
		unsigned DlgItemCount=((Dialog*)CurFrame)->GetAllItemCount();
		const DialogItemEx **DlgItem=((Dialog*)CurFrame)->GetAllItem();

		if (Index == (unsigned)-1)
		{
			SMALL_RECT Rect;

			if (SendDlgMessage((HANDLE)CurFrame,DM_GETDLGRECT,0,(LONG_PTR)&Rect))
			{
				switch (TypeInf)
				{
					case 0: Ret=(__int64)DlgItemCount; break;
					case 2: Ret=(__int64)Rect.Left; break;
					case 3: Ret=(__int64)Rect.Top; break;
					case 4: Ret=(__int64)Rect.Right; break;
					case 5: Ret=(__int64)Rect.Bottom; break;
					case 6: Ret=(__int64)(((Dialog*)CurFrame)->GetDlgFocusPos()+1); break;
				}
			}
		}
		else if (Index < DlgItemCount && DlgItem)
		{
			const DialogItemEx *Item=DlgItem[Index];
			int ItemType=Item->Type;
			DWORD ItemFlags=Item->Flags;

			if (!TypeInf)
			{
				if (ItemType == DI_CHECKBOX || ItemType == DI_RADIOBUTTON)
				{
					TypeInf=7;
				}
				else if (ItemType == DI_COMBOBOX || ItemType == DI_LISTBOX)
				{
					FarListGetItem ListItem;
					ListItem.ItemIndex=Item->ListPtr->GetSelectPos();

					if (SendDlgMessage((HANDLE)CurFrame,DM_LISTGETITEM,Index,(LONG_PTR)&ListItem))
					{
						Ret=ListItem.Item.Text;
					}
					else
					{
						Ret=L"";
					}

					TypeInf=-1;
				}
				else
				{
					TypeInf=10;
				}
			}

			switch (TypeInf)
			{
				case 1: Ret=(__int64)ItemType;    break;
				case 2: Ret=(__int64)Item->X1;    break;
				case 3: Ret=(__int64)Item->Y1;    break;
				case 4: Ret=(__int64)Item->X2;    break;
				case 5: Ret=(__int64)Item->Y2;    break;
				case 6: Ret=(__int64)Item->Focus; break;
				case 7:
				{
					if (ItemType == DI_CHECKBOX || ItemType == DI_RADIOBUTTON)
					{
						Ret=(__int64)Item->Selected;
					}
					else if (ItemType == DI_COMBOBOX || ItemType == DI_LISTBOX)
					{
						Ret=(__int64)(Item->ListPtr->GetSelectPos()+1);
					}
					else
					{
						Ret=0ll;
						/*
						int Item->Selected;
						const char *Item->History;
						const char *Item->Mask;
						FarList *Item->ListItems;
						int  Item->ListPos;
						CHAR_INFO *Item->VBuf;
						*/
					}

					break;
				}
				case 8: Ret=(__int64)ItemFlags; break;
				case 9: Ret=(__int64)Item->DefaultButton; break;
				case 10:
				{
					Ret=Item->strData.CPtr();

					if (IsEdit(ItemType))
					{
						DlgEdit *EditPtr;

						if ((EditPtr = (DlgEdit *)(Item->ObjPtr)) )
							Ret=EditPtr->GetStringAddr();
					}

					break;
				}
			}
		}
	}

	VMStack.Push(Ret);
	return Ret.i()!=-1;
}

// N=Editor.Pos(Op,What[,Where])
// Op: 0 - get, 1 - set
static bool editorposFunc(const TMacroFunction*)
{
	TVar Ret(-1);
	int Where = (int)VMStack.Pop().getInteger();
	int What  = (int)VMStack.Pop().getInteger();
	int Op    = (int)VMStack.Pop().getInteger();

	if (CtrlObject->Macro.GetMode()==MACRO_EDITOR && CtrlObject->Plugins.CurEditor && CtrlObject->Plugins.CurEditor->IsVisible())
	{
		EditorInfo ei;
		CtrlObject->Plugins.CurEditor->EditorControl(ECTL_GETINFO,&ei);

		switch (Op)
		{
			case 0: // get
			{
				switch (What)
				{
					case 1: // CurLine
						Ret=ei.CurLine+1;
						break;
					case 2: // CurPos
						Ret=ei.CurPos+1;
						break;
					case 3: // CurTabPos
						Ret=ei.CurTabPos+1;
						break;
					case 4: // TopScreenLine
						Ret=ei.TopScreenLine+1;
						break;
					case 5: // LeftPos
						Ret=ei.LeftPos+1;
						break;
					case 6: // Overtype
						Ret=ei.Overtype;
						break;
				}

				break;
			}
			case 1: // set
			{
				EditorSetPosition esp;
				esp.CurLine=-1;
				esp.CurPos=-1;
				esp.CurTabPos=-1;
				esp.TopScreenLine=-1;
				esp.LeftPos=-1;
				esp.Overtype=-1;

				switch (What)
				{
					case 1: // CurLine
						esp.CurLine=Where-1;

						if (esp.CurLine < 0)
							esp.CurLine=-1;

						break;
					case 2: // CurPos
						esp.CurPos=Where-1;

						if (esp.CurPos < 0)
							esp.CurPos=-1;

						break;
					case 3: // CurTabPos
						esp.CurTabPos=Where-1;

						if (esp.CurTabPos < 0)
							esp.CurTabPos=-1;

						break;
					case 4: // TopScreenLine
						esp.TopScreenLine=Where-1;

						if (esp.TopScreenLine < 0)
							esp.TopScreenLine=-1;

						break;
					case 5: // LeftPos
					{
						int Delta=Where-1-ei.LeftPos;
						esp.LeftPos=Where-1;

						if (esp.LeftPos < 0)
							esp.LeftPos=-1;

						esp.CurPos=ei.CurPos+Delta;
						break;
					}
					case 6: // Overtype
						esp.Overtype=Where;
						break;
				}

				int Result=CtrlObject->Plugins.CurEditor->EditorControl(ECTL_SETPOSITION,&esp);

				if (Result)
					CtrlObject->Plugins.CurEditor->EditorControl(ECTL_REDRAW,nullptr);

				Ret=Result;
				break;
			}
		}
	}

	VMStack.Push(Ret);
	return Ret.i() != -1;
}

// OldVar=Editor.Set(Idx,Var)
static bool editorsetFunc(const TMacroFunction*)
{
	TVar Ret(-1);
	TVar _longState;
	VMStack.Pop(_longState);
	int Index=(int)VMStack.Pop().getInteger();

	if (CtrlObject->Macro.GetMode()==MACRO_EDITOR && CtrlObject->Plugins.CurEditor && CtrlObject->Plugins.CurEditor->IsVisible())
	{
		long longState=-1L;

		if (Index != 12)
			longState=(long)_longState.toInteger();

		EditorOptions EdOpt;
		CtrlObject->Plugins.CurEditor->GetEditorOptions(EdOpt);

		switch (Index)
		{
			case 0:  // TabSize;
				Ret=(__int64)EdOpt.TabSize; break;
			case 1:  // ExpandTabs;
				Ret=(__int64)EdOpt.ExpandTabs; break;
			case 2:  // PersistentBlocks;
				Ret=(__int64)EdOpt.PersistentBlocks; break;
			case 3:  // DelRemovesBlocks;
				Ret=(__int64)EdOpt.DelRemovesBlocks; break;
			case 4:  // AutoIndent;
				Ret=(__int64)EdOpt.AutoIndent; break;
			case 5:  // AutoDetectCodePage;
				Ret=(__int64)EdOpt.AutoDetectCodePage; break;
			case 6:  // AnsiCodePageForNewFile;
				Ret=(__int64)EdOpt.AnsiCodePageForNewFile; break;
			case 7:  // CursorBeyondEOL;
				Ret=(__int64)EdOpt.CursorBeyondEOL; break;
			case 8:  // BSLikeDel;
				Ret=(__int64)EdOpt.BSLikeDel; break;
			case 9:  // CharCodeBase;
				Ret=(__int64)EdOpt.CharCodeBase; break;
			case 10: // SavePos;
				Ret=(__int64)EdOpt.SavePos; break;
			case 11: // SaveShortPos;
				Ret=(__int64)EdOpt.SaveShortPos; break;
			case 12: // char WordDiv[256];
				Ret=TVar(EdOpt.strWordDiv); break;
			case 13: // F7Rules;
				Ret=(__int64)EdOpt.F7Rules; break;
			case 14: // AllowEmptySpaceAfterEof;
				Ret=(__int64)EdOpt.AllowEmptySpaceAfterEof; break;
			case 15: // ShowScrollBar;
				Ret=(__int64)EdOpt.ShowScrollBar; break;
			case 16: // EditOpenedForWrite;
				Ret=(__int64)EdOpt.EditOpenedForWrite; break;
			case 17: // SearchSelFound;
				Ret=(__int64)EdOpt.SearchSelFound; break;
			case 18: // SearchRegexp;
				Ret=(__int64)EdOpt.SearchRegexp; break;
			case 19: // SearchPickUpWord;
				Ret=(__int64)EdOpt.SearchPickUpWord; break;
			case 20: // ShowWhiteSpace;
				Ret=static_cast<INT64>(EdOpt.ShowWhiteSpace); break;
			default:
				Ret=(__int64)-1L;
		}

		if ((Index != 12 && longState != -1) || (Index == 12 && _longState.i() == -1))
		{
			switch (Index)
			{
				case 0:  // TabSize;
					EdOpt.TabSize=longState; break;
				case 1:  // ExpandTabs;
					EdOpt.ExpandTabs=longState; break;
				case 2:  // PersistentBlocks;
					EdOpt.PersistentBlocks=longState; break;
				case 3:  // DelRemovesBlocks;
					EdOpt.DelRemovesBlocks=longState; break;
				case 4:  // AutoIndent;
					EdOpt.AutoIndent=longState; break;
				case 5:  // AutoDetectCodePage;
					EdOpt.AutoDetectCodePage=longState; break;
				case 6:  // AnsiCodePageForNewFile;
					EdOpt.AnsiCodePageForNewFile=longState; break;
				case 7:  // CursorBeyondEOL;
					EdOpt.CursorBeyondEOL=longState; break;
				case 8:  // BSLikeDel;
					EdOpt.BSLikeDel=longState; break;
				case 9:  // CharCodeBase;
					EdOpt.CharCodeBase=longState; break;
				case 10: // SavePos;
					EdOpt.SavePos=longState; break;
				case 11: // SaveShortPos;
					EdOpt.SaveShortPos=longState; break;
				case 12: // char WordDiv[256];
					EdOpt.strWordDiv = _longState.toString(); break;
				case 13: // F7Rules;
					EdOpt.F7Rules=longState; break;
				case 14: // AllowEmptySpaceAfterEof;
					EdOpt.AllowEmptySpaceAfterEof=longState; break;
				case 15: // ShowScrollBar;
					EdOpt.ShowScrollBar=longState; break;
				case 16: // EditOpenedForWrite;
					EdOpt.EditOpenedForWrite=longState; break;
				case 17: // SearchSelFound;
					EdOpt.SearchSelFound=longState; break;
				case 18: // SearchRegexp;
					EdOpt.SearchRegexp=longState; break;
				case 19: // SearchPickUpWord;
					EdOpt.SearchPickUpWord=longState; break;
				case 20: // ShowWhiteSpace;
					EdOpt.ShowWhiteSpace=longState; break;
				default:
					Ret=-1;
					break;
			}

			CtrlObject->Plugins.CurEditor->SetEditorOptions(EdOpt);
			CtrlObject->Plugins.CurEditor->ShowStatus();
			if (Index == 0 || Index == 12 || Index == 15 || Index == 20)
				CtrlObject->Plugins.CurEditor->Show();
		}
	}

	VMStack.Push(Ret);
	return Ret.i()==-1;
}

// b=mload(var)
static bool mloadFunc(const TMacroFunction*)
{
	TVar Val;
	VMStack.Pop(Val);
	TVarTable *t = &glbVarTable;
	const wchar_t *Name=Val.s();

	if (!Name || *Name!= L'%')
	{
		VMStack.Push(0ll);
		return false;
	}

	DWORD Ret=(DWORD)-1;
	DWORD ValType;

	if (CheckRegValue(L"KeyMacros\\Vars",Name, &ValType))
	{
		switch(ValType)
		{
			case REG_SZ:
			case REG_MULTI_SZ:
			{
				string strSData;
				strSData.Clear();
				GetRegKey(L"KeyMacros\\Vars",Name,strSData,L"");

				if (ValType == REG_MULTI_SZ)
				{
					wchar_t *ptrSData = strSData.GetBuffer();
					for (;;)
					{
						ptrSData+=StrLength(ptrSData);

						if (!ptrSData[0] && !ptrSData[1])
							break;

						*ptrSData=L'\n';
					}
					strSData.ReleaseBuffer();
				}

				varInsert(*t, Name+1)->value = strSData.CPtr();

				Ret=ERROR_SUCCESS;

				break;
			}
			case REG_DWORD:
			{
				varInsert(*t, Name+1)->value = GetRegKey(L"KeyMacros\\Vars",Name,0);
				Ret=ERROR_SUCCESS;
				break;
			}
			case REG_QWORD:
			{
				varInsert(*t, Name+1)->value = GetRegKey64(L"KeyMacros\\Vars",Name,0);
				Ret=ERROR_SUCCESS;
				break;
			}

		}
	}

	VMStack.Push(TVar(Ret==ERROR_SUCCESS?1:0));
	return Ret==ERROR_SUCCESS;
}

// b=msave(var)
static bool msaveFunc(const TMacroFunction*)
{
	TVar Val;
	VMStack.Pop(Val);
	TVarTable *t = &glbVarTable;
	const wchar_t *Name=Val.s();

	if (!Name || *Name!= L'%')
	{
		VMStack.Push(0ll);
		return false;
	}

	TVarSet *tmpVarSet=varLook(*t, Name+1);

	if (!tmpVarSet)
	{
		VMStack.Push(0ll);
		return false;
	}

	TVar Result=tmpVarSet->value;
	DWORD Ret=(DWORD)-1;
	string strValueName = Val.s();

	switch (Result.type())
	{
		case vtInteger:
		{
			__int64 rrr=Result.toInteger();
			Ret=SetRegKey64(L"KeyMacros\\Vars",strValueName,rrr);
			break;
		}
		case vtDouble:
		{
			Ret=(DWORD)_RegWriteString(L"KeyMacros\\Vars",strValueName,Result.toString());
			break;
		}
		case vtString:
		{
			Ret=(DWORD)_RegWriteString(L"KeyMacros\\Vars",strValueName,Result.toString());
			break;
		}
		default:
			break;
	}

	VMStack.Push(TVar(Ret==ERROR_SUCCESS?1:0));
	return Ret==ERROR_SUCCESS;
}

// V=Clip(N[,V])
static bool clipFunc(const TMacroFunction*)
{
	TVar Val;
	VMStack.Pop(Val);
	int cmdType=(int)VMStack.Pop().getInteger();

	// ������������� ������ �������� ������ AS string
	if (cmdType != 5 && Val.isInteger() && !Val.i())
	{
		Val=L"";
		Val.toString();
	}

	int Ret=0;

	switch (cmdType)
	{
		case 0: // Get from Clipboard, "S" - ignore
		{
			wchar_t *ClipText=PasteFromClipboard();

			if (ClipText)
			{
				TVar varClip(ClipText);
				xf_free(ClipText);
				VMStack.Push(varClip);
				return true;
			}

			break;
		}
		case 1: // Put "S" into Clipboard
		{
			Ret=CopyToClipboard(Val.s());
			VMStack.Push(TVar((__int64)Ret)); // 0!  ???
			return Ret?true:false;
		}
		case 2: // Add "S" into Clipboard
		{
			TVar varClip(Val.s());
			Clipboard clip;

			Ret=FALSE;

			if (clip.Open())
			{
				wchar_t *CopyData=clip.Paste();

				if (CopyData)
				{
					size_t DataSize=StrLength(CopyData);
					wchar_t *NewPtr=(wchar_t *)xf_realloc(CopyData,(DataSize+StrLength(Val.s())+2)*sizeof(wchar_t));

					if (NewPtr)
					{
						CopyData=NewPtr;
						wcscpy(CopyData+DataSize,Val.s());
						varClip=CopyData;
						xf_free(CopyData);
					}
					else
					{
						xf_free(CopyData);
					}
				}

				Ret=clip.Copy(varClip.s());

				clip.Close();
			}

			VMStack.Push(TVar((__int64)Ret)); // 0!  ???
			return Ret?true:false;
		}
		case 3: // Copy Win to internal, "S" - ignore
		case 4: // Copy internal to Win, "S" - ignore
		{
			bool OldUseInternalClipboard=Clipboard::SetUseInternalClipboardState((cmdType-3)?true:false);
			TVar varClip(L"");
			wchar_t *ClipText=PasteFromClipboard();

			if (ClipText)
			{
				varClip=ClipText;
				xf_free(ClipText);
			}

			Clipboard::SetUseInternalClipboardState(!Clipboard::GetUseInternalClipboardState());
			Ret=CopyToClipboard(varClip.s());

			Clipboard::SetUseInternalClipboardState(OldUseInternalClipboard);
			VMStack.Push(TVar((__int64)Ret)); // 0!  ???
			return Ret?true:false;
		}
		case 5: // ClipMode
		{
			// 0 - flip, 1 - �������� �����, 2 - ����������, -1 - ��� ������?
			int Action=(int)Val.getInteger();
			bool mode=Clipboard::GetUseInternalClipboardState();
			if (Action >= 0)
			{
				switch (Action)
				{
					case 0: mode=!mode; break;
					case 1: mode=false; break;
					case 2: mode=true;  break;
				}
				mode=Clipboard::SetUseInternalClipboardState(mode);
			}
			VMStack.Push((__int64)(mode?2:1)); // 0!  ???
			return Ret?true:false;
		}
	}

	return Ret?true:false;
}


// N=Panel.SetPosIdx(panelType,Idx[,InSelection])
/*
*/
static bool panelsetposidxFunc(const TMacroFunction*)
{
	int InSelection=(int)VMStack.Pop().getInteger();
	long idxItem=(long)VMStack.Pop().getInteger();
	int typePanel=(int)VMStack.Pop().getInteger();
	Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
	Panel *PassivePanel=nullptr;

	if (ActivePanel)
		PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

	//Frame* CurFrame=FrameManager->GetCurrentFrame();
	Panel *SelPanel = typePanel? (typePanel == 1?PassivePanel:nullptr):ActivePanel;
	__int64 Ret=0;

	if (SelPanel)
	{
		int TypePanel=SelPanel->GetType(); //FILE_PANEL,TREE_PANEL,QVIEW_PANEL,INFO_PANEL
		//int PanelMode=SelPanel->GetMode(); //NORMAL_PANEL,PLUGIN_PANEL

		if (TypePanel == FILE_PANEL || TypePanel ==TREE_PANEL)
		{
			long EndPos=SelPanel->GetFileCount();
			long StartPos;
			long I;
			long idxFoundItem=0;

			if (idxItem) // < 0 || > 0
			{
				EndPos--;
				if ( EndPos > 0 )
				{
					long Direct=idxItem < 0?-1:1;

					if( Direct < 0 )
						idxItem=-idxItem;
					idxItem--;

					if( Direct < 0 )
					{
						StartPos=EndPos;
						EndPos=0;//InSelection?0:idxItem;
					}
					else
						StartPos=0;//!InSelection?0:idxItem;

					bool found=false;

					for ( I=StartPos ; ; I+=Direct )
					{
						if (Direct > 0)
						{
							if(I > EndPos)
								break;
						}
						else
						{
							if(I < EndPos)
								break;
						}

						if ( (!InSelection || (InSelection && SelPanel->IsSelected(I))) && SelPanel->FileInFilter(I) )
						{
							if (idxFoundItem == idxItem)
							{
								idxItem=I;
								found=true;
								break;
							}
							idxFoundItem++;
						}
					}

					if (!found)
						idxItem=-1;

					if (idxItem != -1 && SelPanel->GoToFile(idxItem))
					{
						//SelPanel->Show();
						// <Mantis#0000289> - ������, �� �� ������ :-)
						//ShellUpdatePanels(SelPanel);
						SelPanel->UpdateIfChanged(UIC_UPDATE_NORMAL);
						FrameManager->RefreshFrame(FrameManager->GetTopModal());
						// </Mantis#0000289>

						if ( !InSelection )
							Ret=(__int64)(SelPanel->GetCurrentPos()+1);
						else
							Ret=(__int64)(idxFoundItem+1);
					}
				}
			}
			else // = 0 - ������ ������� �������
			{
				if ( !InSelection )
					Ret=(__int64)(SelPanel->GetCurrentPos()+1);
				else
				{
					long CurPos=SelPanel->GetCurrentPos();
					for ( I=0 ; I < EndPos ; I++ )
					{
						if ( SelPanel->IsSelected(I) && SelPanel->FileInFilter(I) )
						{
							if (I == CurPos)
							{
								Ret=(__int64)(idxFoundItem+1);
								break;
							}
							idxFoundItem++;
						}
					}
				}
			}
		}
	}

	VMStack.Push(Ret);
	return Ret?true:false;
}

// N=panel.SetPath(panelType,pathName[,fileName])
static bool panelsetpathFunc(const TMacroFunction*)
{
	TVar ValFileName;  VMStack.Pop(ValFileName);
	TVar Val;          VMStack.Pop(Val);
	int typePanel=(int)VMStack.Pop().getInteger();
	__int64 Ret=0;

	if (!(Val.isInteger() && !Val.i()))
	{
		const wchar_t *pathName=Val.s();
		const wchar_t *fileName=L"";

		if (!ValFileName.isInteger())
			fileName=ValFileName.s();

		Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
		Panel *PassivePanel=nullptr;

		if (ActivePanel)
			PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

		//Frame* CurFrame=FrameManager->GetCurrentFrame();
		Panel *SelPanel = typePanel? (typePanel == 1?PassivePanel:nullptr):ActivePanel;

		if (SelPanel)
		{
			if (SelPanel->SetCurDir(pathName,SelPanel->GetMode()==PLUGIN_PANEL && IsAbsolutePath(pathName)))
			{
				ActivePanel=CtrlObject->Cp()->ActivePanel;
				PassivePanel=ActivePanel?CtrlObject->Cp()->GetAnotherPanel(ActivePanel):nullptr;
				SelPanel = typePanel? (typePanel == 1?PassivePanel:nullptr):ActivePanel;

				//����������� ������� ����� �� �������� ������.
				ActivePanel->SetCurPath();
				// Need PointToName()?
				SelPanel->GoToFile(fileName); // ����� ��� ��������, �.�. �������� fileName ��� ������������
				//SelPanel->Show();
				// <Mantis#0000289> - ������, �� �� ������ :-)
				//ShellUpdatePanels(SelPanel);
				SelPanel->UpdateIfChanged(UIC_UPDATE_NORMAL);
				FrameManager->RefreshFrame(FrameManager->GetTopModal());
				// </Mantis#0000289>
				Ret=1;
			}
		}
	}

	VMStack.Push(Ret);
	return Ret?true:false;
}

// N=Panel.SetPos(panelType,fileName)
static bool panelsetposFunc(const TMacroFunction*)
{
	TVar Val; VMStack.Pop(Val);
	int typePanel=(int)VMStack.Pop().getInteger();
	const wchar_t *fileName=Val.s();

	if (!fileName || !*fileName)
		fileName=L"";

	Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
	Panel *PassivePanel=nullptr;

	if (ActivePanel)
		PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

	//Frame* CurFrame=FrameManager->GetCurrentFrame();
	Panel *SelPanel = typePanel? (typePanel == 1?PassivePanel:nullptr):ActivePanel;
	__int64 Ret=0;

	if (SelPanel)
	{
		int TypePanel=SelPanel->GetType(); //FILE_PANEL,TREE_PANEL,QVIEW_PANEL,INFO_PANEL

		if (TypePanel == FILE_PANEL || TypePanel ==TREE_PANEL)
		{
			// Need PointToName()?
			if (SelPanel->GoToFile(fileName))
			{
				//SelPanel->Show();
				// <Mantis#0000289> - ������, �� �� ������ :-)
				//ShellUpdatePanels(SelPanel);
				SelPanel->UpdateIfChanged(UIC_UPDATE_NORMAL);
				FrameManager->RefreshFrame(FrameManager->GetTopModal());
				// </Mantis#0000289>
				Ret=(__int64)(SelPanel->GetCurrentPos()+1);
			}
		}
	}

	VMStack.Push(Ret);
	return Ret?true:false;
}

// Result=replace(Str,Find,Replace[,Cnt[,Mode]])
/*
Find=="" - return Str
Cnt==0 - return Str
Replace=="" - return Str (� ��������� ���� �������� Find)
Str=="" return ""

Mode:
      0 - case insensitive
      1 - case sensitive

*/
static bool replaceFunc(const TMacroFunction*)
{
	int Mode=(int)VMStack.Pop().getInteger();
	TVar Count; VMStack.Pop(Count);
	TVar Repl;  VMStack.Pop(Repl);
	TVar Find;  VMStack.Pop(Find);
	TVar Src;   VMStack.Pop(Src);
	__int64 Ret=1;
	// TODO: ����� ����� ��������� � ������������ � ��������!
	string strStr;
	int lenS=(int)StrLength(Src.s());
	int lenF=(int)StrLength(Find.s());
	int lenR=(int)StrLength(Repl.s());
	int cnt=0;

	if( lenF )
	{
		const wchar_t *Ptr=Src.s();
		if( !Mode )
		{
			while ((Ptr=StrStrI(Ptr,Find.s())) )
			{
				cnt++;
				Ptr+=lenF;
			}
		}
		else
		{
			while ((Ptr=StrStr(Ptr,Find.s())) )
			{
				cnt++;
				Ptr+=lenF;
			}
		}
	}

	if (cnt)
	{
		if (lenR > lenF)
			lenS+=cnt*(lenR-lenF+1); //???

		strStr=Src.s();
		cnt=(int)Count.i();

		if (cnt <= 0)
			cnt=-1;

		ReplaceStrings(strStr,Find.s(),Repl.s(),cnt,!Mode);
		VMStack.Push(strStr.CPtr());
	}
	else
		VMStack.Push(Src);

	return Ret?true:false;
}

// V=Panel.Item(typePanel,Index,TypeInfo)
static bool panelitemFunc(const TMacroFunction*)
{
	TVar P2; VMStack.Pop(P2);
	TVar P1; VMStack.Pop(P1);
	int typePanel=(int)VMStack.Pop().getInteger();
	TVar Ret(0ll);
	Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
	Panel *PassivePanel=nullptr;

	if (ActivePanel)
		PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

	//Frame* CurFrame=FrameManager->GetCurrentFrame();
	Panel *SelPanel = typePanel? (typePanel == 1?PassivePanel:nullptr):ActivePanel;

	if (!SelPanel)
	{
		VMStack.Push(Ret);
		return false;
	}

	int TypePanel=SelPanel->GetType(); //FILE_PANEL,TREE_PANEL,QVIEW_PANEL,INFO_PANEL
	int SelPanelMode=SelPanel->GetMode(); //NORMAL_PANEL,PLUGIN_PANEL

	if (!(TypePanel == FILE_PANEL || TypePanel ==TREE_PANEL))
	{
		VMStack.Push(Ret);
		return false;
	}

	int Index=(int)(P1.toInteger())-1;
	int TypeInfo=(int)P2.toInteger();
	FileListItem filelistItem;

	if (TypePanel == TREE_PANEL)
	{
		TreeItem treeItem;

		if (SelPanel->GetItem(Index,&treeItem) && !TypeInfo)
		{
			VMStack.Push(TVar(treeItem.strName));
			return true;
		}
	}
	else
	{
		string strDate, strTime;

		if (TypeInfo == 11)
			SelPanel->ReadDiz();

		if (!SelPanel->GetItem(Index,&filelistItem))
			TypeInfo=-1;

		switch (TypeInfo)
		{
			case 0:  // Name
				Ret=TVar(filelistItem.strName);
				break;
			case 1:  // ShortName
				Ret=TVar(filelistItem.strShortName);
				break;
			case 2:  // FileAttr
				Ret=TVar((__int64)(long)filelistItem.FileAttr);
				break;
			case 3:  // CreationTime
				ConvertDate(filelistItem.CreationTime,strDate,strTime,8,FALSE,FALSE,TRUE,TRUE);
				strDate += L" ";
				strDate += strTime;
				Ret=TVar(strDate.CPtr());
				break;
			case 4:  // AccessTime
				ConvertDate(filelistItem.AccessTime,strDate,strTime,8,FALSE,FALSE,TRUE,TRUE);
				strDate += L" ";
				strDate += strTime;
				Ret=TVar(strDate.CPtr());
				break;
			case 5:  // WriteTime
				ConvertDate(filelistItem.WriteTime,strDate,strTime,8,FALSE,FALSE,TRUE,TRUE);
				strDate += L" ";
				strDate += strTime;
				Ret=TVar(strDate.CPtr());
				break;
			case 6:  // UnpSize
				Ret=TVar((__int64)filelistItem.UnpSize);
				break;
			case 7:  // PackSize
				Ret=TVar((__int64)filelistItem.PackSize);
				break;
			case 8:  // Selected
				Ret=TVar((__int64)((DWORD)filelistItem.Selected));
				break;
			case 9:  // NumberOfLinks
				Ret=TVar((__int64)filelistItem.NumberOfLinks);
				break;
			case 10:  // SortGroup
				Ret=TVar((__int64)filelistItem.SortGroup);
				break;
			case 11:  // DizText
				Ret=TVar((const wchar_t *)filelistItem.DizText);
				break;
			case 12:  // Owner
				Ret=TVar(filelistItem.strOwner);
				break;
			case 13:  // CRC32
				Ret=TVar((__int64)filelistItem.CRC32);
				break;
			case 14:  // Position
				Ret=TVar((__int64)filelistItem.Position);
				break;
			case 15:  // CreationTime (FILETIME)
				Ret=TVar((__int64)FileTimeToUI64(&filelistItem.CreationTime));
				break;
			case 16:  // AccessTime (FILETIME)
				Ret=TVar((__int64)FileTimeToUI64(&filelistItem.AccessTime));
				break;
			case 17:  // WriteTime (FILETIME)
				Ret=TVar((__int64)FileTimeToUI64(&filelistItem.WriteTime));
				break;
			case 18: // NumberOfStreams
				Ret=TVar((__int64)filelistItem.NumberOfStreams);
				break;
			case 19: // StreamsSize
				Ret=TVar((__int64)filelistItem.StreamsSize);
				break;
			case 20:  // ChangeTime
				ConvertDate(filelistItem.ChangeTime,strDate,strTime,8,FALSE,FALSE,TRUE,TRUE);
				strDate += L" ";
				strDate += strTime;
				Ret=TVar(strDate.CPtr());
				break;
			case 21:  // ChangeTime (FILETIME)
				Ret=TVar((__int64)FileTimeToUI64(&filelistItem.ChangeTime));
				break;
			case 22:  // CustomData
				if (SelPanelMode==NORMAL_PANEL && !filelistItem.CustomDataLoaded)
				{
					//Maximus: BUGBUG: ��������� ��������� ������� ����� ��� ������ (��� ����� ���� ��������� ������!)
					CtrlObject->Plugins.GetCustomData(&filelistItem);
				}
				Ret=TVar(filelistItem.strCustomData);
				break;
		}
	}

	VMStack.Push(Ret);
	return false;
}

// N=len(V)
static bool lenFunc(const TMacroFunction*)
{
	TVar Val;
	VMStack.Pop(Val);
	VMStack.Push(TVar(StrLength(Val.toString())));
	return true;
}

static bool ucaseFunc(const TMacroFunction*)
{
	TVar Val; VMStack.Pop(Val);
	StrUpper((wchar_t *)Val.toString());
	VMStack.Push(Val);
	return true;
}

static bool lcaseFunc(const TMacroFunction*)
{
	TVar Val; VMStack.Pop(Val);
	StrLower((wchar_t *)Val.toString());
	VMStack.Push(Val);
	return true;
}

static bool stringFunc(const TMacroFunction*)
{
	TVar Val;
	VMStack.Pop(Val);
	Val.toString();
	VMStack.Push(Val);
	return true;
}

static bool intFunc(const TMacroFunction*)
{
	TVar Val;
	VMStack.Pop(Val);
	Val.toInteger();
	VMStack.Push(Val);
	return true;
}

static bool floatFunc(const TMacroFunction*)
{
	TVar Val;
	VMStack.Pop(Val);
	//Val.toDouble();
	VMStack.Push(Val);
	return true;
}

static bool absFunc(const TMacroFunction*)
{
	TVar tmpVar;
	VMStack.Pop(tmpVar);

	if (tmpVar < 0ll)
		tmpVar=-tmpVar;

	VMStack.Push(tmpVar);
	return true;
}

static bool ascFunc(const TMacroFunction*)
{
	TVar tmpVar;
	VMStack.Pop(tmpVar);

	if (tmpVar.isString())
	{
		tmpVar = (__int64)((DWORD)((WORD)*tmpVar.toString()));
		tmpVar.toInteger();
	}

	VMStack.Push(tmpVar);
	return true;
}

static bool chrFunc(const TMacroFunction*)
{
	TVar tmpVar;
	VMStack.Pop(tmpVar);

	if (tmpVar.isInteger())
	{
		const wchar_t tmp[]={tmpVar.i()&0xFFFF,L'\0'};
		tmpVar = tmp;
		tmpVar.toString();
	}

	VMStack.Push(tmpVar);
	return true;
}

// N=FMatch(S,Mask)
static bool fmatchFunc(const TMacroFunction*)
{
	TVar Mask;  VMStack.Pop(Mask);
	TVar S;     VMStack.Pop(S);
	CFileMask FileMask;

	if (FileMask.Set(Mask.toString(), FMF_SILENT))
		VMStack.Push(FileMask.Compare(S.toString()));
	else
		VMStack.Push(-1);
	return true;
}

// V=Editor.Sel(Action[,Opt])
static bool editorselFunc(const TMacroFunction*)
{
	/*
	 MCODE_F_EDITOR_SEL
	  Action: 0 = Get Param
	              Opt:  0 = return FirstLine
	                    1 = return FirstPos
	                    2 = return LastLine
	                    3 = return LastPos
	                    4 = return block type (0=nothing 1=stream, 2=column)
	              return: 0 = failure, 1... request value

	          1 = Set Pos
	              Opt:  0 = begin block (FirstLine & FirstPos)
	                    1 = end block (LastLine & LastPos)
	              return: 0 = failure, 1 = success

	          2 = Set Stream Selection Edge
	              Opt:  0 = selection start
	                    1 = selection finish
	              return: 0 = failure, 1 = success

	          3 = Set Column Selection Edge
	              Opt:  0 = selection start
	                    1 = selection finish
	              return: 0 = failure, 1 = success
	          4 = Unmark selected block
	              Opt: ignore
	              return 1
	*/
	TVar Ret(0ll);
	TVar Opt; VMStack.Pop(Opt);
	TVar Action; VMStack.Pop(Action);
	int Mode=CtrlObject->Macro.GetMode();
	Frame* CurFrame=FrameManager->GetCurrentFrame();
	int NeedType = Mode == MACRO_EDITOR?MODALTYPE_EDITOR:(Mode == MACRO_VIEWER?MODALTYPE_VIEWER:(Mode == MACRO_DIALOG?MODALTYPE_DIALOG:MODALTYPE_PANELS)); // MACRO_SHELL?

	if (CurFrame && CurFrame->GetType()==NeedType)
	{
		if (Mode==MACRO_SHELL && CtrlObject->CmdLine->IsVisible())
			Ret=CtrlObject->CmdLine->VMProcess(MCODE_F_EDITOR_SEL,ToPtr(Action.toInteger()),Opt.i());
		else
			Ret=CurFrame->VMProcess(MCODE_F_EDITOR_SEL,ToPtr(Action.toInteger()),Opt.i());
	}

	VMStack.Push(Ret);
	return Ret.i() == 1;
}

// V=Editor.Undo(N)
static bool editorundoFunc(const TMacroFunction*)
{
	TVar Ret(0ll);
	TVar Action; VMStack.Pop(Action);

	if (CtrlObject->Macro.GetMode()==MACRO_EDITOR && CtrlObject->Plugins.CurEditor && CtrlObject->Plugins.CurEditor->IsVisible())
	{
		EditorUndoRedo eur;
		eur.Command=(int)Action.toInteger();
		Ret=(__int64)CtrlObject->Plugins.CurEditor->EditorControl(ECTL_UNDOREDO,&eur);
	}

	VMStack.Push(Ret);
	return Ret.i()!=0;
}

// N=Editor.SetTitle([Title])
static bool editorsettitleFunc(const TMacroFunction*)
{
	TVar Ret(0ll);
	TVar Title; VMStack.Pop(Title);

	if (CtrlObject->Macro.GetMode()==MACRO_EDITOR && CtrlObject->Plugins.CurEditor && CtrlObject->Plugins.CurEditor->IsVisible())
	{
		if (Title.isInteger() && !Title.i())
		{
			Title=L"";
			Title.toString();
		}
		Ret=(__int64)CtrlObject->Plugins.CurEditor->EditorControl(ECTL_SETTITLE,(void*)Title.s());
	}

	VMStack.Push(Ret);
	return Ret.i()!=0;
}

// N=Plugin.Load(DllPath[,ForceLoad])
static bool pluginloadFunc(const TMacroFunction*)
{
	TVar Ret(0ll);
	TVar ForceLoad; VMStack.Pop(ForceLoad);
	TVar DllPath; VMStack.Pop(DllPath);
	if (DllPath.s())
		Ret=(__int64)farPluginsControl(INVALID_HANDLE_VALUE, !ForceLoad.i()?PCTL_LOADPLUGIN:PCTL_FORCEDLOADPLUGIN, 0, (LONG_PTR)DllPath.s());
	VMStack.Push(Ret);
	return Ret.i()!=0;
}

// N=Plugin.UnLoad(DllPath)
static bool pluginunloadFunc(const TMacroFunction*)
{
	TVar Ret(0ll);
	TVar DllPath; VMStack.Pop(DllPath);
	if (DllPath.s())
		Ret=(__int64)farPluginsControl(INVALID_HANDLE_VALUE, PCTL_UNLOADPLUGIN, 0, (LONG_PTR)DllPath.s());
	VMStack.Push(Ret);
	return Ret.i()!=0;
}


// V=callplugin(SysID[,param])
#if 0
static bool callpluginFunc(const TMacroFunction*)
{
/*
���� � OpenPlugin ���������� ���� OPEN_FROMMACRO - �� ������������, ������� � ���, ��� ����� ������� ��� �� ��������.
OPEN_FROMMACROSTRING ���������� ���� - �������� Data �������� ������, ���� ���� ���� �� ������, �� Data �������� �����.
������ �������� ��� OpenFrom - ����������������... ����� ���������� ��...
*/

	__int64 Ret=0;
	TVar Param; VMStack.Pop(Param);
	TVar SysID; VMStack.Pop(SysID);

	if (CtrlObject->Plugins.FindPlugin((DWORD)SysID.i()))
	{
		int OpenFrom = -1;
		Frame* frame = FrameManager->GetCurrentFrame();

		if (frame)
			switch (frame->GetType())
			{
	/*
OPEN_DISKMENU 	������ �� ���� ������
OPEN_PLUGINSMENU 	������ �� ���� �������� (F11)
OPEN_FINDLIST 	������ �� ������� "������ ������" ���� ������������� ������ ������� ������ � ��� ������, ���� �� ������������ ������� SetFindListW. ����������� ����� ������� SetFindListW ��������� ������ � ��� ������, ���� ������� OpenPluginW ����� �������� �������� �� INVALID_HANDLE_VALUE.
OPEN_SHORTCUT 	������ ����� ������ �� ����� (���� Commands|Folder shortcuts)
OPEN_COMMANDLINE 	��� ������ �� ��������� ������. ���� �������� ����� ��������������, ������ ���� ������ ��������� ���������� ������� � ������� GetPluginInfoW � ���� �������, � ���������� ����� ����, ��� ������ � ��������� ������.
OPEN_EDITOR 	������ �� ���������
OPEN_VIEWER 	������ �� ���������� ��������� ���������
OPEN_FILEPANEL 	������ �� �������
OPEN_DIALOG 	������ �� �������
OPEN_ANALYSE 	������ �� ???
OPEN_FROMMACRO 	������ �� ������������


# ��� OPEN_FINDLIST Item ������ 0.
# ��� OPEN_SHORTCUT Item �������� ����� ������, ������� ���� ��������
	� ������� ShortcutData ��������� OpenPluginInfo � ������ ���������� ������� �������.
	������ ����� ������������ ��� ���� ��� ���������� �������������� ���������� � ������� ���������.
	�� ����������� ��������� � �� ���������� � ������� ����������, ��� ��� ���� ���������� ��� FAR.

	OPEN_DISKMENU
	OPEN_PLUGINSMENU
	OPEN_FINDLIST
	OPEN_SHORTCUT
	OPEN_FILEPANEL
	OPEN_ANALYSE
    */

				/*
					��� ���������� OPEN_DISKMENU, OPEN_PLUGINSMENU, OPEN_EDITOR � OPEN_VIEWER Item - ��� ����� ����������
					������ � ���� �� ������������������ �������� �������. ���� ������ ������������ ������ ���� �������,
					���� �������� ������ ����� ����.
				*/
				case MODALTYPE_EDITOR:
					OpenFrom = OPEN_EDITOR      | OPEN_FROMMACRO;
					break;
				case MODALTYPE_VIEWER:
					OpenFrom = OPEN_VIEWER      | OPEN_FROMMACRO;
					break;
				// ��� OPEN_COMMANDLINE Item �������� ����� ��������� ������������� � ��������� ������ ���������.
				case MODALTYPE_PANELS:
					OpenFrom = OPEN_COMMANDLINE | OPEN_FROMMACRO;
					break;
				case MODALTYPE_DIALOG:
					// ��� OPEN_DIALOG Item �������� ����� ��������� OpenDlgPluginData.
					OpenFrom = OPEN_DIALOG      | OPEN_FROMMACRO;
                    /*
struct OpenDlgPluginData
{
	int ItemNumber;
	HANDLE hDlg;
};
                    */
					break;

				/*
				*/
				case MODALTYPE_VMENU:
				case MODALTYPE_COMBOBOX:

				case MODALTYPE_VIRTUAL:
				case MODALTYPE_HELP:
				case MODALTYPE_FINDFOLDER:
				case MODALTYPE_USER:
				default:
					break;
			}

		if (OpenFrom != -1)
		{
			if( Opt.Macro.CallPluginRules )
				CtrlObject->Macro.PushState(true);

			OpenFrom |= Param.isString() ? OPEN_FROMMACROSTRING : 0;

			Ret=CtrlObject->Plugins.CallPlugin((DWORD)SysID.i(),OpenFrom,
			                                   Param.isString() ? (void*)Param.s() :
			                                   (void*)(size_t)Param.i());

			if( Opt.Macro.CallPluginRules )
				CtrlObject->Macro.PopState();
		}
	}

	VMStack.Push(Ret);
	return Ret?true:false;
}
#else
static bool callpluginFunc(const TMacroFunction*)
{
	__int64 Ret=0;
	TVar Param; VMStack.Pop(Param);
	TVar SysID; VMStack.Pop(SysID);

	if (CtrlObject->Plugins.FindPlugin((DWORD)SysID.i()))
	{
		// OpenFrom => OPEN_FROMMACRO [+OPEN_FROMMACROSTRING] + FARMACROAREA(i)
		int OpenFrom = OPEN_FROMMACRO | (Param.isString() ? OPEN_FROMMACROSTRING : 0) | CtrlObject->Macro.GetMode();

		if( Opt.Macro.CallPluginRules )
			CtrlObject->Macro.PushState(true);

		int ResultCallPlugin=0;

		if (CtrlObject->Plugins.CallPlugin((DWORD)SysID.i(),OpenFrom,
		                                   Param.isString() ? (void*)Param.s() :
		                                   (void*)(size_t)Param.i(),&ResultCallPlugin))
			Ret=(__int64)ResultCallPlugin;

		if( Opt.Macro.CallPluginRules )
			CtrlObject->Macro.PopState();
	}

	VMStack.Push(Ret);
	return Ret?true:false;
}
#endif

// N=testfolder(S)
/*
���������� ���� ��������� ������������ ��������:

TSTFLD_NOTFOUND   (2) - ��� ������
TSTFLD_NOTEMPTY   (1) - �� �����
TSTFLD_EMPTY      (0) - �����
TSTFLD_NOTACCESS (-1) - ��� �������
TSTFLD_ERROR     (-2) - ������ (������ ��������� ��� ��������� ������ ��� ��������� ������������� �������)
*/
static bool testfolderFunc(const TMacroFunction*)
{
	TVar tmpVar;
	VMStack.Pop(tmpVar);
	__int64 Ret=TSTFLD_ERROR;

	if (tmpVar.isString())
	{
		DisableElevation de;
		Ret=(__int64)TestFolder(tmpVar.s());
	}

	VMStack.Push(Ret);
	return Ret?true:false;
}

// ����� ���������� �������
static bool pluginsFunc(const TMacroFunction *thisFunc)
{
	TVar V;
	bool Ret=false;
	int nParam=(int)thisFunc->nParam;
/*
enum FARMACROVARTYPE
{
	FMVT_INTEGER                = 0,
	FMVT_STRING                 = 1,
	FMVT_DOUBLE                 = 2,
};

struct FarMacroValue
{
	FARMACROVARTYPE type;
	union
	{
		__int64  i;
		double   d;
		const wchar_t *s;
	} v;
};
*/
#if defined(PROCPLUGINMACROFUNC)
	int I;

	FarMacroValue *vParams=new FarMacroValue[nParam];
	if (vParams)
	{
		memset(vParams,0,sizeof(FarMacroValue) * nParam);

		for (I=nParam-1; I >= 0; --I)
		{
			VMStack.Pop(V);
			(vParams+I)->type=(FARMACROVARTYPE)V.type();
			switch(V.type())
			{
				case vtInteger:
					(vParams+I)->v.i=V.i();
					break;
				case vtString:
					(vParams+I)->v.s=xf_wcsdup(V.s());
					break;
				case vtDouble:
					(vParams+I)->v.d=V.d();
					break;
			}
		}

		FarMacroValue *Results;
		int nResults=0;
		// fnGUID ???
		if (CtrlObject->Plugins.ProcessMacroFunc(thisFunc->Name,vParams,thisFunc->nParam,&Results,&nResults))
		{
			if (Results)
			{
				for (I=0; I < nResults; ++I)
				//for (I=nResults-1; I >= 0; --I)
				{
					//V.type()=(TVarType)(Results+I)->type;
					switch((Results+I)->type)
					{
						case FMVT_INTEGER:
							V=(Results+I)->v.i;
							break;
						case FMVT_STRING:
							V=(Results+I)->v.s;
							break;
						case FMVT_DOUBLE:
							V=(Results+I)->v.d;
							break;
					}
					VMStack.Push(V);
				}
			}
		}

		for (I=0; I < nParam; ++I)
			if((vParams+I)->type == vtString && (vParams+I)->v.s)
				xf_free((void*)(vParams+I)->v.s);

		delete[] vParams;
	}
	else
		VMStack.Push(0);
#else
	/* �������� */ while(--nParam >= 0) VMStack.Pop(V);
#endif
	return Ret;
}

// ����� ���������������� �������
static bool usersFunc(const TMacroFunction *thisFunc)
{
	TVar V;
	bool Ret=false;

	int nParam=thisFunc->nParam;
	/* �������� */ while(--nParam >= 0) VMStack.Pop(V);

	VMStack.Push(0);
	return Ret;
}


const wchar_t *eStackAsString(int)
{
	const wchar_t *s=__varTextDate.toString();
	return !s?L"":s;
}

static bool __CheckCondForSkip(DWORD Op)
{
	TVar tmpVar=VMStack.Pop();
	if (tmpVar.isString() && *tmpVar.s())
		return false;

	__int64 res=tmpVar.getInteger();
	switch(Op)
	{
		case MCODE_OP_JZ:
			return !res?true:false;
		case MCODE_OP_JNZ:
			return res?true:false;
		case MCODE_OP_JLT:
			return res < 0?true:false;
		case MCODE_OP_JLE:
			return res <= 0?true:false;
		case MCODE_OP_JGT:
			return res > 0?true:false;
		case MCODE_OP_JGE:
			return res >= 0?true:false;
	}
	return false;
}


int KeyMacro::GetKey()
{
	MacroRecord *MR;
	TVar tmpVar;
	TVarSet *tmpVarSet=nullptr;

	//_SVS(SysLog(L">KeyMacro::GetKey() InternalInput=%d Executing=%d (%p)",InternalInput,Work.Executing,FrameManager->GetCurrentFrame()));
	if (InternalInput || !FrameManager->GetCurrentFrame())
	{
		//_KEYMACRO(SysLog(L"[%d] return RetKey=%d",__LINE__,RetKey));
		return 0;
	}

	int RetKey=0;  // ������� ������ ������� 0 - ������ � ���, ��� ����������������������� ���

	if (Work.Executing == MACROMODE_NOMACRO)
	{
		if (!Work.MacroWORK)
		{
			if (CurPCStack >= 0)
			{
				//_KEYMACRO(SysLog(L"[%d] if(CurPCStack >= 0)",__LINE__));
				PopState();
				return RetKey;
			}

			if (Mode==MACRO_EDITOR &&
			        IsRedrawEditor &&
			        CtrlObject->Plugins.CurEditor &&
			        CtrlObject->Plugins.CurEditor->IsVisible() &&
			        LockScr)
			{
				CtrlObject->Plugins.ProcessEditorEvent(EE_REDRAW,EEREDRAW_CHANGE);
				CtrlObject->Plugins.ProcessEditorEvent(EE_REDRAW,EEREDRAW_ALL);
				CtrlObject->Plugins.CurEditor->Show();
			}

			if (CurPCStack < 0)
			{
				if (LockScr)
					delete LockScr;

				LockScr=nullptr;
			}

			if (ConsoleTitle::WasTitleModified())
				ConsoleTitle::SetFarTitle(nullptr);

			Clipboard::SetUseInternalClipboardState(false); //??
			//_KEYMACRO(SysLog(L"[%d] return RetKey=%d",__LINE__,RetKey));
			return RetKey;
		}

		/*
		else if(Work.ExecLIBPos>=MR->BufferSize)
		{
			ReleaseWORKBuffer();
			Work.Executing=MACROMODE_NOMACRO;
			return FALSE;
		}
		else
		*/
		//if(Work.MacroWORK)
		{
			Work.Executing=Work.MacroWORK->Flags&MFLAGS_NOSENDKEYSTOPLUGINS?MACROMODE_EXECUTING:MACROMODE_EXECUTING_COMMON;
			Work.ExecLIBPos=0; //?????????????????????????????????
		}
		//else
		//	return FALSE;
	}

initial:

	if (!(MR=Work.MacroWORK) || !MR->Buffer)
	{
		//_KEYMACRO(SysLog(L"[%d] return RetKey=%d",__LINE__,RetKey));
		return 0; // RetKey; ?????
	}

	//_SVS(SysLog(L"KeyMacro::GetKey() initial: Work.ExecLIBPos=%d (%d) %p",Work.ExecLIBPos,MR->BufferSize,Work.MacroWORK));

	// ��������! �������� �����!
	if (!Work.ExecLIBPos && !LockScr && (MR->Flags&MFLAGS_DISABLEOUTPUT))
		LockScr=new LockScreen;

begin:

	if (Work.ExecLIBPos>=MR->BufferSize || !MR->Buffer)
	{
done:

		/*$ 10.08.2000 skv
			If we are in editor mode, and CurEditor defined,
			we need to call this events.
			EE_REDRAW 1 - to notify that text changed.
			EE_REDRAW 0 - to notify that whole screen updated
			->Show() to actually update screen.

			This duplication take place since ShowEditor method
			will NOT send this event while screen is locked.
		*/
		if (Mode==MACRO_EDITOR &&
		        IsRedrawEditor &&
		        CtrlObject->Plugins.CurEditor &&
		        CtrlObject->Plugins.CurEditor->IsVisible() &&
		        LockScr)
		{
			CtrlObject->Plugins.ProcessEditorEvent(EE_REDRAW,EEREDRAW_CHANGE);
			CtrlObject->Plugins.ProcessEditorEvent(EE_REDRAW,EEREDRAW_ALL);
			CtrlObject->Plugins.CurEditor->Show();
		}

		if (CurPCStack < 0 && (Work.MacroWORKCount-1) <= 0) // mantis#351
		{
			if (LockScr) delete LockScr;
			LockScr=nullptr;
			MR->Flags&=~MFLAGS_DISABLEOUTPUT; // ????
		}

		Clipboard::SetUseInternalClipboardState(false); //??
		Work.Executing=MACROMODE_NOMACRO;
		ReleaseWORKBuffer();

		// �������� - "� ���� �� � ��������� ����� ��� �������"?
		if (Work.MacroWORKCount > 0)
		{
			// �������, �������� ��������� �� �����
			Work.ExecLIBPos=0;
		}

		if (ConsoleTitle::WasTitleModified())
			ConsoleTitle::SetFarTitle(nullptr); // �������� ������ ��������� �� ���������� �������

		//FrameManager->RefreshFrame();
		//FrameManager->PluginCommit();
		_KEYMACRO(SysLog(-1); SysLog(L"[%d] **** End Of Execute Macro ****",__LINE__));

		if (Work.MacroWORKCount <= 0 && CurPCStack >= 0)
		{
			PopState();
			goto initial;
		}

		ScrBuf.RestoreMacroChar();
		Work.HistroyEnable=0;

		StopMacro=false;

		return KEY_NONE; // ����� ������!
	}

	if (!Work.ExecLIBPos)
		Work.Executing=Work.MacroWORK->Flags&MFLAGS_NOSENDKEYSTOPLUGINS?MACROMODE_EXECUTING:MACROMODE_EXECUTING_COMMON;

	// Mantis#0000581: �������� ����������� �������� ���������� �������
	{
		INPUT_RECORD rec;

		if (StopMacro || (PeekInputRecord(&rec) && rec.EventType==KEY_EVENT && rec.Event.KeyEvent.wVirtualKeyCode == VK_CANCEL))
		{
			GetInputRecord(&rec,true);  // ������� �� ������� ��� "�������"...
			Work.KeyProcess=0;
			VMStack.Pop();              // Mantis#0000841 - (TODO: �������� ����� ����� Pop`�� �� ��������, ����� ���������!)
			goto done;                  // ...� ��������� ������.
		}
	}

	DWORD Key=!MR?MCODE_OP_EXIT:GetOpCode(MR,Work.ExecLIBPos++);

	string value;
	_KEYMACRO(SysLog(L"[%d] IP=%d Op=%08X ==> %s or %s",__LINE__,Work.ExecLIBPos-1,Key,_MCODE_ToName(Key),_FARKEY_ToName(Key)));

	if (Work.KeyProcess && Key != MCODE_OP_ENDKEYS)
	{
		_KEYMACRO(SysLog(L"[%d] IP=%d  %s (Work.KeyProcess && Key != MCODE_OP_ENDKEYS)",__LINE__,Work.ExecLIBPos-1,_FARKEY_ToName(Key)));
		goto return_func;
	}

	switch (Key)
	{
		case MCODE_OP_CONTINUE:
			goto begin; // ������ ���� Jump

		case MCODE_OP_NOP:
			goto begin;
		case MCODE_OP_KEYS:                    // �� ���� ����� ������� ������ ���� ������
		{
			_KEYMACRO(SysLog(L"MCODE_OP_KEYS"));
			Work.KeyProcess++;
			goto begin;
		}
		case MCODE_OP_ENDKEYS:                 // ������ ���� �����������.
		{
			_KEYMACRO(SysLog(L"MCODE_OP_ENDKEYS"));
			Work.KeyProcess--;
			goto begin;
		}
		case KEY_ALTINS:
		{
			if (RunGraber())
				return KEY_NONE;

			break;
		}

		case MCODE_OP_XLAT:               // $XLat
		{
			return KEY_OP_XLAT;
		}
		case MCODE_OP_SELWORD:            // $SelWord
		{
			return KEY_OP_SELWORD;
		}
		case MCODE_F_PRINT:               // N=Print(Str)
		case MCODE_OP_PLAINTEXT:          // $Text "Text"
		{
			if (VMStack.empty())
				return KEY_NONE;

			VMStack.Pop(__varTextDate);
			if (Key == MCODE_F_PRINT)
				VMStack.Push(1);
			return KEY_OP_PLAINTEXT;
		}
		case MCODE_OP_EXIT:               // $Exit
		{
			goto done;
		}

		case MCODE_OP_AKEY:               // $AKey
		{
			DWORD aKey=KEY_NONE;
			if (!(MR->Flags&MFLAGS_POSTFROMPLUGIN))
			{
				INPUT_RECORD *inRec=&Work.cRec;
				if (!inRec->EventType)
					inRec->EventType = KEY_EVENT;
				if(inRec->EventType == KEY_EVENT || inRec->EventType == FARMACRO_KEY_EVENT)
					aKey=ShieldCalcKeyCode(inRec,FALSE,nullptr);
			}
			else
				aKey=MR->Key;
			return aKey;
		}

		case MCODE_F_AKEY:                // V=akey(Mode[,Type])
		{
			int tmpType=(int)VMStack.Pop().getInteger();
			int tmpMode=(int)VMStack.Pop().getInteger();

			DWORD aKey=MR->Key;

			if (!tmpType)
			{
				if (!(MR->Flags&MFLAGS_POSTFROMPLUGIN))
				{
					INPUT_RECORD *inRec=&Work.cRec;
					if (!inRec->EventType)
						inRec->EventType = KEY_EVENT;
					if(inRec->EventType == KEY_EVENT || inRec->EventType == FARMACRO_KEY_EVENT)
						aKey=ShieldCalcKeyCode(inRec,FALSE,nullptr);
				}
				else if (!aKey)
					aKey=KEY_NONE;
			}

			if (!tmpMode)
				tmpVar=(__int64)aKey;
			else
			{
				KeyToText(aKey,value);
				tmpVar=value.CPtr();
				tmpVar.toString();
			}
			VMStack.Push(tmpVar);
			goto begin;
		}

		case MCODE_F_HISTIORY_ENABLE: // N=History.Enable([State])
		{
			TVar State; VMStack.Pop(State);

			DWORD oldHistroyEnable=Work.HistroyEnable;

			if (!State.isUnknown())
				Work.HistroyEnable=(DWORD)State.getInteger();

			VMStack.Push((__int64)oldHistroyEnable);
			goto begin;
		}

		// $Rep (expr) ... $End
		// -------------------------------------
		//            <expr>
		//            MCODE_OP_SAVEREPCOUNT       1
		// +--------> MCODE_OP_REP                2   p1=*
		// |          <counter>                   3
		// |          <counter>                   4
		// |          MCODE_OP_JZ  ------------+  5   p2=*+2
		// |          ...                      |
		// +--------- MCODE_OP_JMP             |
		//            MCODE_OP_END <-----------+
		case MCODE_OP_SAVEREPCOUNT:
		{
			// ������� ������������ �������� ��������
			// �� ����� � ������� ��� � ������� �����
			LARGE_INTEGER Counter;

			if ((Counter.QuadPart=VMStack.Pop().getInteger()) < 0)
				Counter.QuadPart=0;

			SetOpCode(MR,Work.ExecLIBPos+1,Counter.u.HighPart);
			SetOpCode(MR,Work.ExecLIBPos+2,Counter.u.LowPart);
			SetMacroConst(constRCounter,Counter.QuadPart);
			goto begin;
		}
		case MCODE_OP_REP:
		{
			// ������� ������� �������� ��������
			LARGE_INTEGER Counter;
			Counter.u.HighPart=GetOpCode(MR,Work.ExecLIBPos);
			Counter.u.LowPart=GetOpCode(MR,Work.ExecLIBPos+1);
			// � ������� ��� �� ������� �����
			VMStack.Push(Counter.QuadPart);
			SetMacroConst(constRCounter,Counter.QuadPart);
			// �������� ��� � ������ �� MCODE_OP_JZ
			Counter.QuadPart--;
			SetOpCode(MR,Work.ExecLIBPos++,Counter.u.HighPart);
			SetOpCode(MR,Work.ExecLIBPos++,Counter.u.LowPart);
			goto begin;
		}
		case MCODE_OP_END:
			// ������ ��������� ���� �������� ���������� :)
			goto begin;
		case MCODE_OP_SAVE:
		{
			TVar Val0; VMStack.Pop(Val0);
			GetPlainText(value);

			// ����� �������� �����, �.�. ���������� ������� ������ �������, ��� ���������� ����������
			if (!value.IsEmpty())
			{
				TVarTable *t = (value.At(0) == L'%') ? &glbVarTable : Work.locVarTable;
				varInsert(*t, value)->value = Val0;
			}

			goto begin;
		}

		case MCODE_F_MMODE:               // N=MMode(Action[,Value])
		{
			__int64 nValue = (__int64)VMStack.Pop().getInteger();
			TVar Action(1);
			if (Key == MCODE_F_MMODE)
				VMStack.Pop(Action);

			__int64 Result=0;

			switch (Action.getInteger())
			{
				case 1: // DisableOutput
				{
					Result=LockScr?1:0;

					if (nValue == 2) // �������� ����� ����������� ("DisableOutput").
					{
						if (MR->Flags&MFLAGS_DISABLEOUTPUT)
							nValue=0;
						else
							nValue=1;
					}

					switch (nValue)
					{
						case 0: // DisableOutput=0, ��������� �����
							if (LockScr)
							{
								delete LockScr;
								LockScr=nullptr;
							}
							MR->Flags&=~MFLAGS_DISABLEOUTPUT;
							break;
						case 1: // DisableOutput=1, �������� �����
							if (!LockScr)
								LockScr=new LockScreen;
							MR->Flags|=MFLAGS_DISABLEOUTPUT;
							break;
					}

					break;
				}

				case 2: // Get MacroRecord Flags
				{
					Result=(__int64)MR->Flags;
					if ((Result&MFLAGS_MODEMASK) == MACRO_COMMON)
						Result|=0x00FF; // ...��� �� Common ��� ������ ���������.
					break;
				}

			}

			VMStack.Push(Result);
			break;
		}

		case MCODE_OP_DUP:        // �������������� ������� �������� � �����
			tmpVar=VMStack.Peek();
			VMStack.Push(tmpVar);
			goto begin;

		case MCODE_OP_SWAP:
		{
			TVar Val0;
			VMStack.Pop(Val0);
			VMStack.Pop(tmpVar);
			VMStack.Push(Val0);
			VMStack.Push(tmpVar);
			goto begin;
		}

		case MCODE_OP_DISCARD:    // ������ �������� � ������� �����
			VMStack.Pop();
			goto begin;

		case MCODE_OP_POP:        // 0: pop 1: varname -> ��������� �������� ���������� � ������ �� ������� �����
		{
			VMStack.Pop(tmpVar);
			GetPlainText(value);
			TVarTable *t = (value.At(0) == L'%') ? &glbVarTable : Work.locVarTable;
			tmpVarSet=varLook(*t, value);

			if (tmpVarSet)
				tmpVarSet->value=tmpVar;

			goto begin;
		}
		/*                               ������
			0: MCODE_OP_COPY                 0:   MCODE_OP_PUSHVAR
			1: szVarDest                     1:   VarSrc
			...                              ...
			N: szVarSrc                      N:   MCODE_OP_SAVE
			...                            N+1:   VarDest
			                               N+2:
			                                 ...
		*/
		case MCODE_OP_COPY:       // 0: Copy 1: VarDest 2: VarSrc ==>  %a=%d
		{
			GetPlainText(value);
			TVarTable *t = (value.At(0) == L'%') ? &glbVarTable : Work.locVarTable;
			tmpVarSet=varLook(*t, value);

			if (tmpVarSet)
				tmpVar=tmpVarSet->value;

			GetPlainText(value);
			t = (value.At(0) == L'%') ? &glbVarTable : Work.locVarTable;
			tmpVarSet=varLook(*t, value);

			if (tmpVarSet)
				tmpVar=tmpVarSet->value;

			goto begin;
		}
		case MCODE_OP_PUSHFLOAT:
		{
			union { struct { DWORD l, h; }; double d; } u;
			u.h = GetOpCode(MR,Work.ExecLIBPos++);   //???
			u.l = GetOpCode(MR,Work.ExecLIBPos++);    //???
			VMStack.Push(u.d);
			goto begin;
		}
		case MCODE_OP_PUSHUNKNOWN:
		case MCODE_OP_PUSHINT: // �������� ����� �������� �� ����.
		{
			LARGE_INTEGER i64;
			i64.u.HighPart=GetOpCode(MR,Work.ExecLIBPos++);   //???
			i64.u.LowPart=GetOpCode(MR,Work.ExecLIBPos++);    //???
			TVar *ptrVar=VMStack.Push(i64.QuadPart);
			if (Key == MCODE_OP_PUSHUNKNOWN)
				ptrVar->SetType(vtUnknown);
			goto begin;
		}
		case MCODE_OP_PUSHCONST:  // �������� �� ���� ���������.
		{
			GetPlainText(value);
			tmpVarSet=varLook(glbConstTable, value);

			if (tmpVarSet)
				VMStack.Push(tmpVarSet->value);
			else
				VMStack.Push(0ll);

			goto begin;
		}
		case MCODE_OP_PUSHVAR: // �������� �� ���� ����������.
		{
			GetPlainText(value);
			TVarTable *t = (value.At(0) == L'%') ? &glbVarTable : Work.locVarTable;
			// %%name - ���������� ����������
			tmpVarSet=varLook(*t, value);

			if (tmpVarSet)
				VMStack.Push(tmpVarSet->value);
			else
				VMStack.Push(0ll);

			goto begin;
		}
		case MCODE_OP_PUSHSTR: // �������� �� ���� ������-���������.
		{
			GetPlainText(value);
			VMStack.Push(TVar(value.CPtr()));
			goto begin;
		}
		// ��������
		case MCODE_OP_JMP:
			Work.ExecLIBPos=GetOpCode(MR,Work.ExecLIBPos);
			goto begin;

		case MCODE_OP_JZ:
		case MCODE_OP_JNZ:
		case MCODE_OP_JLT:
		case MCODE_OP_JLE:
		case MCODE_OP_JGT:
		case MCODE_OP_JGE:
			if(__CheckCondForSkip(Key))
				Work.ExecLIBPos=GetOpCode(MR,Work.ExecLIBPos);
			else
				Work.ExecLIBPos++;

			goto begin;

			// ��������
		case MCODE_OP_NEGATE: VMStack.Pop(tmpVar); VMStack.Push(-tmpVar); goto begin;
		case MCODE_OP_NOT:    VMStack.Pop(tmpVar); VMStack.Push(!tmpVar); goto begin;
		case MCODE_OP_LT:     VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() <  tmpVar); goto begin;
		case MCODE_OP_LE:     VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() <= tmpVar); goto begin;
		case MCODE_OP_GT:     VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() >  tmpVar); goto begin;
		case MCODE_OP_GE:     VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() >= tmpVar); goto begin;
		case MCODE_OP_EQ:     VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() == tmpVar); goto begin;
		case MCODE_OP_NE:     VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() != tmpVar); goto begin;
		case MCODE_OP_ADD:    VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() +  tmpVar); goto begin;
		case MCODE_OP_SUB:    VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() -  tmpVar); goto begin;
		case MCODE_OP_MUL:    VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() *  tmpVar); goto begin;
		case MCODE_OP_DIV:

			if (VMStack.Peek()==0ll)
			{
				_KEYMACRO(SysLog(L"[%d] IP=%d/0x%08X Error: Divide by zero",__LINE__,Work.ExecLIBPos,Work.ExecLIBPos));
				goto done;
			}

			VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() /  tmpVar);
			goto begin;
			// Logical
		case MCODE_OP_AND:    VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() && tmpVar); goto begin;
		case MCODE_OP_OR:     VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() || tmpVar); goto begin;
		case MCODE_OP_XOR:    VMStack.Pop(tmpVar); VMStack.Push(xor_op(VMStack.Pop(),tmpVar)); goto begin;
			// Bit Op
		case MCODE_OP_BITAND: VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() &  tmpVar); goto begin;
		case MCODE_OP_BITOR:  VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() |  tmpVar); goto begin;
		case MCODE_OP_BITXOR: VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() ^  tmpVar); goto begin;
		case MCODE_OP_BITSHR: VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() >> tmpVar); goto begin;
		case MCODE_OP_BITSHL: VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() << tmpVar); goto begin;
		case MCODE_OP_BITNOT: VMStack.Pop(tmpVar); VMStack.Push(~tmpVar); goto begin;

		case MCODE_OP_ADDEQ:                   // a +=  b
		{
			GetPlainText(value);
			TVarTable *t = (value.At(0) == L'%') ? &glbVarTable : Work.locVarTable;
			tmpVarSet=varLook(*t, value);
			VMStack.Pop(tmpVar);
			tmpVarSet->value += tmpVar;
			goto begin;
		}
		case MCODE_OP_SUBEQ:                   // a -=  b
		{
			GetPlainText(value);
			TVarTable *t = (value.At(0) == L'%') ? &glbVarTable : Work.locVarTable;
			tmpVarSet=varLook(*t, value);
			VMStack.Pop(tmpVar);
			tmpVarSet->value -= tmpVar;
			goto begin;
		}
		case MCODE_OP_MULEQ:                   // a *=  b
		{
			GetPlainText(value);
			TVarTable *t = (value.At(0) == L'%') ? &glbVarTable : Work.locVarTable;
			tmpVarSet=varLook(*t, value);
			VMStack.Pop(tmpVar);
			tmpVarSet->value *= tmpVar;
			goto begin;
		}
		case MCODE_OP_DIVEQ:                   // a /=  b
		{
			GetPlainText(value);
			TVarTable *t = (value.At(0) == L'%') ? &glbVarTable : Work.locVarTable;
			tmpVarSet=varLook(*t, value);
			VMStack.Pop(tmpVar);
			if (tmpVar == 0ll)
				goto done;
			tmpVarSet->value /= tmpVar;
			goto begin;
		}
		case MCODE_OP_BITSHREQ:                // a >>= b
		{
			GetPlainText(value);
			TVarTable *t = (value.At(0) == L'%') ? &glbVarTable : Work.locVarTable;
			tmpVarSet=varLook(*t, value);
			VMStack.Pop(tmpVar);
			tmpVarSet->value >>= tmpVar;
			goto begin;
		}
		case MCODE_OP_BITSHLEQ:                // a <<= b
		{
			GetPlainText(value);
			TVarTable *t = (value.At(0) == L'%') ? &glbVarTable : Work.locVarTable;
			tmpVarSet=varLook(*t, value);
			VMStack.Pop(tmpVar);
			tmpVarSet->value <<= tmpVar;
			goto begin;
		}
		case MCODE_OP_BITANDEQ:                // a &=  b
		{
			GetPlainText(value);
			TVarTable *t = (value.At(0) == L'%') ? &glbVarTable : Work.locVarTable;
			tmpVarSet=varLook(*t, value);
			VMStack.Pop(tmpVar);
			tmpVarSet->value &= tmpVar;
			goto begin;
		}
		case MCODE_OP_BITXOREQ:                // a ^=  b
		{
			GetPlainText(value);
			TVarTable *t = (value.At(0) == L'%') ? &glbVarTable : Work.locVarTable;
			tmpVarSet=varLook(*t, value);
			VMStack.Pop(tmpVar);
			tmpVarSet->value ^= tmpVar;
			goto begin;
		}
		case MCODE_OP_BITOREQ:                 // a |=  b
		{
			GetPlainText(value);
			TVarTable *t = (value.At(0) == L'%') ? &glbVarTable : Work.locVarTable;
			tmpVarSet=varLook(*t, value);
			VMStack.Pop(tmpVar);
			tmpVarSet->value |= tmpVar;
			goto begin;
		}
			// Function
		case MCODE_F_EVAL: // N=eval(S[,N])
		{
			DWORD Cmd=(DWORD)VMStack.Pop().getInteger();
			TVar Val;
			VMStack.Pop(Val);
			MacroRecord RBuf;
			int KeyPos;

			if (!(Val.isInteger() && !Val.i())) // ��������� ������ ���������� ���������� ������ ����������
			{
				int Ret=-1;

				switch (Cmd)
				{
					case 0:
					{
						GetCurRecord(&RBuf,&KeyPos);
						PushState(true);

						if (!(MR->Flags&MFLAGS_DISABLEOUTPUT))
							RBuf.Flags &= ~MFLAGS_DISABLEOUTPUT;

						if (!PostNewMacro(Val.toString(),RBuf.Flags&(~MFLAGS_REG_MULTI_SZ),RBuf.Key))
							PopState();
						else
							Ret=1;
						VMStack.Push((__int64)__getMacroErrorCode());
						break;
					}

					case 1: // ������ ��������?
					{
						PostNewMacro(Val.toString(),0,0,TRUE);
						VMStack.Push((__int64)__getMacroErrorCode());
						break;
					}

					case 2: // ����������� ����� �������, ����������� �� ���������������
					{
						/*
						   ��� �����:
						   �) ������ �������� ������� ���������� � 2
						   �) ������ ���������� ������� ������ � ������� "Area/Key"
						      �����:
						        "Area" - �������, �� ������� ����� ������� ������
						        "/" - �����������
						        "Key" - �������� �������
						      "Area/" ����� �� ���������, � ���� ������ ����� "Key" ����� ������� � ������� �������� ������������,
						         ���� � ������� ������� "Key" �� ������, �� ����� ����������� � ������� Common.
						         ��� �� ��������� ����� � ������� Common (����������� ������ "����" ��������),
						         ���������� � �������� "Area" ������� �����.

						   ��� ������ 2 ������� ������
						     -1 - ������
						     -2 - ��� �������, ��������� ���������������� (��� ������ ������������)
						      0 - Ok
						*/
						int _Mode;
						bool UseCommon=true;
						string strVal=Val.toString();
						strVal=RemoveExternalSpaces(strVal);

						wchar_t *lpwszVal = strVal.GetBuffer();
						wchar_t *p=wcsrchr(lpwszVal,L'/');

						if (p  && p[1])
						{
							*p++=0;
							if ((_Mode = GetSubKey(lpwszVal)) < MACRO_FUNCS)
							{
								_Mode=GetMode();
								if (lpwszVal[0] == L'.' && !lpwszVal[1]) // ������� "./Key" �� ������������� ����� � Common`�
									UseCommon=false;
							}
							else
								UseCommon=false;
						}
						else
						{
							p=lpwszVal;
							_Mode=GetMode();
						}

						DWORD KeyCode = KeyNameToKey(p);
						strVal.ReleaseBuffer();

						int I=GetIndex(KeyCode,_Mode,UseCommon);
						if (I != -1 && !(MacroLIB[I].Flags&MFLAGS_DISABLEMACRO)) // && CtrlObject)
						{
							PushState(true);
							// __setMacroErrorCode(err_Success); // ???
							PostNewMacro(MacroLIB+I);
							VMStack.Push((__int64)__getMacroErrorCode()); // ???
							Ret=1;
						}
						else
						{
							VMStack.Push(-2);
						}
						break;
					}
				}

				if (Ret > 0)
					goto initial; // �.�.
			}
			else
				VMStack.Push(-1);
			goto begin;
		}

		case MCODE_F_BM_ADD:              // N=BM.Add()
		case MCODE_F_BM_CLEAR:            // N=BM.Clear()
		case MCODE_F_BM_NEXT:             // N=BM.Next()
		case MCODE_F_BM_PREV:             // N=BM.Prev()
		case MCODE_F_BM_BACK:             // N=BM.Back()
		case MCODE_F_BM_STAT:             // N=BM.Stat([N])
		case MCODE_F_BM_DEL:              // N=BM.Del([Idx]) - ������� �������� � ��������� �������� (x=1...), 0 - ������� ������� ��������
		case MCODE_F_BM_GET:              // N=BM.Get(Idx,M) - ���������� ���������� ������ (M==0) ��� ������� (M==1) �������� � �������� (Idx=1...)
		case MCODE_F_BM_GOTO:             // N=BM.Goto([n]) - ������� �� �������� � ��������� �������� (0 --> �������)
		case MCODE_F_BM_PUSH:             // N=BM.Push() - ��������� ������� ������� � ���� �������� � ����� �����
		case MCODE_F_BM_POP:              // N=BM.Pop() - ������������ ������� ������� �� �������� � ����� ����� � ������� ��������
		{
			TVar p1, p2;

			if (Key == MCODE_F_BM_GET)
				VMStack.Pop(p2);

			if (Key == MCODE_F_BM_GET || Key == MCODE_F_BM_DEL || Key == MCODE_F_BM_STAT || Key == MCODE_F_BM_GOTO)
				VMStack.Pop(p1);

			__int64 Result=0;
			Frame *f=FrameManager->GetCurrentFrame(), *fo=nullptr;

			while (f)
			{
				fo=f;
				f=f->GetTopModal();
			}

			if (!f)
				f=fo;

			if (f)
				Result=f->VMProcess(Key,ToPtr(p2.i()),p1.i());

			VMStack.Push(Result);
			goto begin;
		}

		case MCODE_F_MENU_ITEMSTATUS:     // N=Menu.ItemStatus([N])
		case MCODE_F_MENU_GETVALUE:       // S=Menu.GetValue([N])
		case MCODE_F_MENU_GETHOTKEY:      // S=gethotkey([N])
		{
			_KEYMACRO(CleverSysLog Clev(Key == MCODE_F_MENU_GETHOTKEY?L"MCODE_F_MENU_GETHOTKEY":L"MCODE_F_MENU_GETVALUE"));
			VMStack.Pop(tmpVar);

			if (!tmpVar.isInteger())
				tmpVar=0ll;

			int CurMMode=CtrlObject->Macro.GetMode();

			if (IsMenuArea(CurMMode) || CurMMode == MACRO_DIALOG)
			{
				Frame *f=FrameManager->GetCurrentFrame(), *fo=nullptr;

				//f=f->GetTopModal();
				while (f)
				{
					fo=f;
					f=f->GetTopModal();
				}

				if (!f)
					f=fo;

				__int64 Result;

				if (f)
				{
					__int64 MenuItemPos=tmpVar.i()-1;
					if (Key == MCODE_F_MENU_GETHOTKEY)
					{
						if ((Result=f->VMProcess(Key,nullptr,MenuItemPos)) )
						{

							const wchar_t _value[]={static_cast<wchar_t>(Result),0};
							tmpVar=_value;
						}
						else
							tmpVar=L"";
					}
					else if (Key == MCODE_F_MENU_GETVALUE)
					{
						string NewStr;
						if (f->VMProcess(Key,&NewStr,MenuItemPos))
						{
							HiText2Str(NewStr, NewStr);
							RemoveExternalSpaces(NewStr);
							tmpVar=NewStr.CPtr();
						}
						else
							tmpVar=L"";
					}
					else if (Key == MCODE_F_MENU_ITEMSTATUS)
					{
						tmpVar=f->VMProcess(Key,nullptr,MenuItemPos);
					}
				}
				else
					tmpVar=L"";
			}
			else
				tmpVar=L"";

			VMStack.Push(tmpVar);
			goto begin;
		}
		case MCODE_F_MENU_SELECT:      // N=Menu.Select(S[,N[,Dir]])
		case MCODE_F_MENU_CHECKHOTKEY: // N=checkhotkey(S[,N])
		{
			_KEYMACRO(CleverSysLog Clev(Key == MCODE_F_MENU_CHECKHOTKEY? L"MCODE_F_MENU_CHECKHOTKEY":L"MCODE_F_MENU_SELECT"));
			__int64 Result=-1;
			__int64 tmpMode=0;
			__int64 tmpDir=0;

			if (Key == MCODE_F_MENU_SELECT)
				tmpDir=VMStack.Pop().getInteger();

			tmpMode=VMStack.Pop().getInteger();

			if (Key == MCODE_F_MENU_SELECT)
				tmpMode |= (tmpDir << 8);
			else
			{
				if (tmpMode > 0)
					tmpMode--;
			}

			VMStack.Pop(tmpVar);
			//const wchar_t *checkStr=tmpVar.toString();
			int CurMMode=CtrlObject->Macro.GetMode();

			if (IsMenuArea(CurMMode) || CurMMode == MACRO_DIALOG)
			{
				Frame *f=FrameManager->GetCurrentFrame(), *fo=nullptr;

				//f=f->GetTopModal();
				while (f)
				{
					fo=f;
					f=f->GetTopModal();
				}

				if (!f)
					f=fo;

				if (f)
					Result=f->VMProcess(Key,(void*)tmpVar.toString(),tmpMode);
			}

			VMStack.Push(Result);
			goto begin;
		}
		case MCODE_F_MENU_FILTER:      // N=Menu.Filter([Action[,Mode]])
		case MCODE_F_MENU_FILTERSTR:   // S=Menu.FilterStr([Action[,S]])
		{
			_KEYMACRO(CleverSysLog Clev(Key == MCODE_F_MENU_FILTER? L"MCODE_F_MENU_FILTER":L"MCODE_F_MENU_FILTERSTR"));
			bool succees=false;
			TVar tmpAction;

			VMStack.Pop(tmpVar);
			VMStack.Pop(tmpAction);
			if (tmpAction.isUnknown())
				tmpAction=Key == MCODE_F_MENU_FILTER ? 4 : 0;

			int CurMMode=CtrlObject->Macro.GetMode();

			if (IsMenuArea(CurMMode) || CurMMode == MACRO_DIALOG)
			{
				Frame *f=FrameManager->GetCurrentFrame(), *fo=nullptr;

				//f=f->GetTopModal();
				while (f)
				{
					fo=f;
					f=f->GetTopModal();
				}

				if (!f)
					f=fo;

				if (f)
				{
					if (Key == MCODE_F_MENU_FILTER)
					{
						if (tmpVar.isUnknown())
							tmpVar = -1;
						tmpVar=f->VMProcess(Key,(void*)static_cast<INT_PTR>(tmpVar.toInteger()),tmpAction.toInteger());
						succees=true;
					}
					else
					{
						string NewStr;
						if (tmpVar.isString())
							NewStr = tmpVar.toString();
						if (f->VMProcess(Key,(void*)&NewStr,tmpAction.toInteger()))
						{
							tmpVar=NewStr.CPtr();
							succees=true;
						}
					}
				}
			}

			if (!succees)
			{
				if (Key == MCODE_F_MENU_FILTER)
					tmpVar = -1;
				else
					tmpVar = L"";
			}

			VMStack.Push(tmpVar);
			goto begin;
		}

		default:
		{
			size_t J;

			for (J=0; J < CMacroFunction; ++J)
			{
				const TMacroFunction *MFunc = KeyMacro::GetMacroFunction(J);
				if (MFunc->Code == (TMacroOpCode)Key && MFunc->Func)
				{
					DWORD Flags=MR->Flags;

					if (MFunc->IntFlags&IMFF_UNLOCKSCREEN)
					{
						if (Flags&MFLAGS_DISABLEOUTPUT) // ���� ��� - ������
						{
							if (LockScr) delete LockScr;

							LockScr=nullptr;
						}
					}

					if (MFunc->IntFlags&IMFF_DISABLEINTINPUT)
						InternalInput++;

					MFunc->Func(MFunc);

					if (MFunc->IntFlags&IMFF_DISABLEINTINPUT)
						InternalInput--;

					if (MFunc->IntFlags&IMFF_UNLOCKSCREEN)
					{
						if (Flags&MFLAGS_DISABLEOUTPUT) // ���� ���� - �������
						{
							if (LockScr) delete LockScr;

							LockScr=new LockScreen;
						}
					}
					break;
				}
			}

			if (J >= CMacroFunction)
			{
				DWORD Err=0;
				tmpVar=FARPseudoVariable(MR->Flags, Key, Err);

				if (!Err)
					VMStack.Push(tmpVar);
				else
				{
					if (Key >= KEY_MACRO_BASE && Key <= KEY_MACRO_ENDBASE)
					{
						// ��� �� �������, � ������������ OpCode, ��������� ���������� �������
						goto done;
					}
					break; // ������� ����� ����������
				}
			}

			goto begin;
		} // END default
	} // END: switch(Key)

return_func:

	if (Work.KeyProcess && (Key&KEY_ALTDIGIT)) // "����������" ������ ;-)
	{
		Key&=~KEY_ALTDIGIT;
		IntKeyState.ReturnAltValue=1;
	}

#if 0

	if (MR==Work.MacroWORK &&
	        (Work.ExecLIBPos>=MR->BufferSize || Work.ExecLIBPos+1==MR->BufferSize && MR->Buffer[Work.ExecLIBPos]==KEY_NONE) &&
	        Mode==MACRO_DIALOG
	   )
	{
		RetKey=Key;
		goto done;
	}

#else

	if (MR==Work.MacroWORK && Work.ExecLIBPos>=MR->BufferSize)
	{
		_KEYMACRO(SysLog(-1); SysLog(L"[%d] **** End Of Execute Macro ****",__LINE__));
		ReleaseWORKBuffer();
		Work.Executing=MACROMODE_NOMACRO;

		if (ConsoleTitle::WasTitleModified())
			ConsoleTitle::SetFarTitle(nullptr);
	}

#endif
	return(Key);
}

// ��������� - ���� �� ��� �������?
int KeyMacro::PeekKey()
{
	if (InternalInput || !Work.MacroWORK)
		return 0;

	MacroRecord *MR=Work.MacroWORK;

	if ((Work.Executing == MACROMODE_NOMACRO && !Work.MacroWORK) || Work.ExecLIBPos >= MR->BufferSize)
		return FALSE;

	DWORD OpCode=GetOpCode(MR,Work.ExecLIBPos);
	return OpCode;
}

DWORD KeyMacro::SwitchFlags(DWORD& Flags,DWORD Value)
{
	if (Flags&Value) Flags&=~Value;
	else Flags|=Value;

	return Flags;
}


string &KeyMacro::MkRegKeyName(int IdxMacro, string &strRegKeyName)
{
	string strKeyText;
	KeyToText(MacroLIB[IdxMacro].Key, strKeyText);
	strRegKeyName=L"KeyMacros\\";
	strRegKeyName+=GetSubKey(MacroLIB[IdxMacro].Flags&MFLAGS_MODEMASK);
	AddEndSlash(strRegKeyName);

	if (MacroLIB[IdxMacro].Flags&MFLAGS_DISABLEMACRO)
	{
		strRegKeyName+=L"~";
	}

	strRegKeyName+=strKeyText;
	return strRegKeyName;
}

/*
  ����� ������ ���� ������� ����� ������� ������!!!
  ������� ���������� ������ ������� ������������������, �.�.... �������
  � ��������� ������ ���������� Src
*/
wchar_t *KeyMacro::MkTextSequence(DWORD *Buffer,int BufferSize,const wchar_t *Src)
{
	int J, Key;
	string strMacroKeyText;
	string strTextBuffer;

	if (!Buffer)
		return nullptr;

#if 0

	if (BufferSize == 1)
	{
		if (
		    (((DWORD)(DWORD_PTR)Buffer)&KEY_MACRO_ENDBASE) >= KEY_MACRO_BASE && (((DWORD)(DWORD_PTR)Buffer)&KEY_MACRO_ENDBASE) <= KEY_MACRO_ENDBASE ||
		    (((DWORD)(DWORD_PTR)Buffer)&KEY_OP_ENDBASE) >= KEY_OP_BASE && (((DWORD)(DWORD_PTR)Buffer)&KEY_OP_ENDBASE) <= KEY_OP_ENDBASE
		)
		{
			return Src?xf_wcsdup(Src):nullptr;
		}

		if (KeyToText((DWORD)(DWORD_PTR)Buffer,strMacroKeyText))
			return xf_wcsdup(strMacroKeyText.CPtr());

		return nullptr;
	}

#endif
	strTextBuffer.Clear();

	if (Buffer[0] == MCODE_OP_KEYS)
		for (J=1; J < BufferSize; J++)
		{
			Key=Buffer[J];

			if (Key == MCODE_OP_ENDKEYS || Key == MCODE_OP_KEYS)
				continue;

			if (/*
				(Key&KEY_MACRO_ENDBASE) >= KEY_MACRO_BASE && (Key&KEY_MACRO_ENDBASE) <= KEY_MACRO_ENDBASE ||
				(Key&KEY_OP_ENDBASE) >= KEY_OP_BASE && (Key&KEY_OP_ENDBASE) <= KEY_OP_ENDBASE ||
				*/
			    !KeyToText(Key,strMacroKeyText)
			)
			{
				return Src?xf_wcsdup(Src):nullptr;
			}

			if (J > 1)
				strTextBuffer += L" ";

			strTextBuffer += strMacroKeyText;
		}

	if (!strTextBuffer.IsEmpty())
		return xf_wcsdup(strTextBuffer.CPtr());

	return nullptr;
}

// ���������� ���� ��������
void KeyMacro::SaveMacros(BOOL AllSaved)
{
	string strRegKeyName;
	//WriteVarsConst(MACRO_VARS);
	//WriteVarsConst(MACRO_CONSTS);

	for (int I=0; I<MacroLIBCount; I++)
	{
		if (!AllSaved  && !(MacroLIB[I].Flags&MFLAGS_NEEDSAVEMACRO))
			continue;

		MkRegKeyName(I, strRegKeyName);

		if (!MacroLIB[I].BufferSize || !MacroLIB[I].Src)
		{
			DeleteRegKey(strRegKeyName);
			continue;
		}

#if 0

		if (!(TextBuffer=MkTextSequence(MacroLIB[I].Buffer,MacroLIB[I].BufferSize,MacroLIB[I].Src)))
			continue;

		SetRegKey(RegKeyName,"Sequence",TextBuffer);

		//_SVS(SysLog(L"%3d) %s|Sequence='%s'",I,RegKeyName,TextBuffer));
		if (TextBuffer)
			xf_free(TextBuffer);

#endif
		BOOL Ok=TRUE;

		if (MacroLIB[I].Flags&MFLAGS_REG_MULTI_SZ)
		{
			int Len=StrLength(MacroLIB[I].Src)+2;
			wchar_t *ptrSrc=new wchar_t[Len];

			if (ptrSrc)
			{
				wcscpy(ptrSrc,MacroLIB[I].Src);

				for (int J=0; ptrSrc[J]; ++J)
					if (ptrSrc[J] == L'\n')
						ptrSrc[J]=0;

				ptrSrc[Len-1]=0;
				SetRegKey(strRegKeyName,L"Sequence",ptrSrc,Len*sizeof(wchar_t),REG_MULTI_SZ);
				delete[] ptrSrc;
				Ok=FALSE;
			}
		}

		if (Ok)
			SetRegKey(strRegKeyName,L"Sequence",MacroLIB[I].Src);

		if (MacroLIB[I].Description)
			SetRegKey(strRegKeyName,L"Description",MacroLIB[I].Description);
		else
			DeleteRegValue(strRegKeyName,L"Description");

		// ����������� ����...
		for (size_t J=0; J < ARRAYSIZE(MKeywordsFlags); ++J)
		{
			if (MacroLIB[I].Flags & MKeywordsFlags[J].Value)
				SetRegKey(strRegKeyName,MKeywordsFlags[J].Name,1);
			else
				DeleteRegValue(strRegKeyName,MKeywordsFlags[J].Name);
		}
	}
}


int KeyMacro::WriteVarsConst(int WriteMode)
{
	string strUpKeyName=L"KeyMacros\\";
	strUpKeyName+=(WriteMode==MACRO_VARS?L"Vars":L"Consts");
	string strValueName;
	TVarTable *t = (WriteMode==MACRO_VARS)?&glbVarTable:&glbConstTable;

	for (int I=0; ; I++)
	{
		TVarSet *var=varEnum(*t,I);

		if (!var)
			break;

		strValueName = var->str;
		strValueName = (WriteMode==MACRO_VARS?L"%":L"")+strValueName;

		switch (var->value.type())
		{
			case vtInteger:
				SetRegKey64(strUpKeyName,strValueName,var->value.i());
				break;
			case vtDouble:
				//_RegWriteString(strUpKeyName,strValueName,var->value.d());
				break;
			case vtString:
				_RegWriteString(strUpKeyName,strValueName,var->value.s());
				break;
			default:
				break;
		}
	}

	return TRUE;
}

/*
   KeyMacros\\Vars
     "StringName":REG_SZ
     "IntName":REG_DWORD
*/
int KeyMacro::ReadVarsConst(int ReadMode, string &strSData)
{
	string strValueName;
	long IData;
	__int64 IData64;
	string strUpKeyName=L"KeyMacros\\";
	strUpKeyName+=(ReadMode==MACRO_VARS?L"Vars":L"Consts");
	TVarTable *t = (ReadMode==MACRO_VARS)?&glbVarTable:&glbConstTable;

	for (int i=0; ; i++)
	{
		IData=0;
		strValueName.Clear();
		strSData.Clear();
		int Type=EnumRegValueEx(strUpKeyName,i,strValueName,strSData,(LPDWORD)&IData,(__int64*)&IData64);

		if (Type == REG_NONE)
			break;

		if (ReadMode == MACRO_VARS &&  !(strValueName.At(0) == L'%' && strValueName.At(1) == L'%'))
			continue;

		const wchar_t *lpwszValueName=strValueName.CPtr()+(ReadMode==MACRO_VARS);

		if (Type == REG_SZ)
			varInsert(*t, lpwszValueName)->value = strSData.CPtr();
		else if (Type == REG_MULTI_SZ)
		{
			// ��������� ��� �� REG_MULTI_SZ
			wchar_t *ptrSData = strSData.GetBuffer();

			for (;;)
			{
				ptrSData+=StrLength(ptrSData);

				if (!ptrSData[0] && !ptrSData[1])
					break;

				*ptrSData=L'\n';
			}

			strSData.ReleaseBuffer();
			varInsert(*t, lpwszValueName)->value = strSData.CPtr();
		}
		else if (Type == REG_DWORD)
			varInsert(*t, lpwszValueName)->value = (__int64)IData;
		else if (Type == REG_QWORD)
			varInsert(*t, lpwszValueName)->value = IData64;
	}

	if (ReadMode == MACRO_CONSTS)
	{
		INT64 Value=0;
		SetMacroConst(constMsX,Value);
		SetMacroConst(constMsY,Value);
		SetMacroConst(constMsButton,Value);
		SetMacroConst(constMsCtrlState,Value);
		SetMacroConst(constMsEventFlags,Value);
		SetMacroConst(constRCounter,Value);
	}

	return TRUE;
}

void KeyMacro::SetMacroConst(const wchar_t *ConstName, const TVar& Value)
{
#ifdef _DEBUG
	if (ConstName == constMsButton)
	{
		static int LastMsButton;
		if ((LastMsButton & 1) && (Value.i() == 0))
		{
			// LButton was Down, now - Up
			LastMsButton = (int)Value.i();
		}
		else if (!LastMsButton && (Value.i() & 1))
		{
			// LButton was Up, now - Down
			LastMsButton = (int)Value.i();
		}
		else
		{
			LastMsButton = (int)Value.i();
		}
	}
#endif
	varLook(glbConstTable, ConstName,1)->value = Value;
}

/*
   KeyMacros\Function
*/
int KeyMacro::ReadMacroFunction(int ReadMode, string& strBuffer)
{
	/*
	 � ������� ������� ������ "KeyMacros\Funcs" - ���������� ������������, �������������� ��������� (ProcessMacroW)
     ��� ���������� - ��� ��� "�������"
     �������� � ������� ����������:
       Syntax:reg_sz - ��������� ������� (�� ������� - � �������� ���������)
       Params:reg_dword - ���������� ���������� � �������
       OParams:reg_dword - �������������� ��������� �������
       Sequence:reg_sz - ���� �������
       Flags:reg_dword - �����
       GUID:reg_sz - GUID ��� ���� � ������� � �������� PluginsCache (������� �� Flags)
       Description:reg_sz - �������������� ��������

     Flags - ����� �����
       0: � GUID ���� � �������, ��� � PluginsCache ����� GUID
       1: ������������ Sequence ������ �������; ��� �� ����� �������, ���� GUID ����
       2: ...


     ��������� � ����� �������, ��� � ������� abs, mix, len, etc.
     ���� Plugin �� ����, Sequence ������������.
     Plugin - ��� ���������� �� ����� PluginsCache

	[HKEY_CURRENT_USER\Software\Far2\KeyMacros\Funcs\math.sin]
	"Syntax"="d=sin(V)"
	"nParams"=dword:1
	"oParams"=dword:0
	"Sequence"=""
	"Flags"=dword:0
	"GUID"="C:/Program Files/Far2/Plugins/Calc/bin/calc.dll"
	"Description"="���������� �������� ������ � ������� �����"

	Flags:
		����:
			0: � GUID ���� � �������, ��� � PluginsCache ����� GUID
			1: ������������ Sequence ������ �������; ��� �� ����� �������, ���� GUID ����
			2:

	$1, $2, $3 - ���������
	*/
	if (ReadMode == MACRO_FUNCS)
	{
#if 1
		int I;
		string strUpKeyName=L"KeyMacros\\Funcs";
		string strRegKeyName;
		string strFuncName;
		string strSyntax;
		DWORD  nParams;
		DWORD  oParams;
		DWORD  Flags;
		string strGUID;
		string strDescription;
		DWORD regType=0;

		for (I=0;; I++)
		{
			if (!EnumRegKey(strUpKeyName,I,strRegKeyName))
				break;

			size_t pos;

			strRegKeyName.RPos(pos,L'\\');
			strFuncName = strRegKeyName;
			strFuncName.LShift(pos+1);

			if (GetRegKey(strRegKeyName,L"Sequence",strBuffer,L"",&regType) && regType == REG_MULTI_SZ)
			{
				wchar_t *ptrBuffer = strBuffer.GetBuffer();

				while (1)
				{
					ptrBuffer+=StrLength(ptrBuffer);

					if (!ptrBuffer[0] && !ptrBuffer[1])
						break;

					*ptrBuffer=L'\n';
				}

				strBuffer.ReleaseBuffer();
			}

			RemoveExternalSpaces(strBuffer);
			nParams=GetRegKey(strRegKeyName,L"nParams",0);
			oParams=GetRegKey(strRegKeyName,L"oParams",0);
			Flags=GetRegKey(strRegKeyName,L"Flags",0);

			regType=0;

			if (GetRegKey(strRegKeyName,L"GUID",strGUID,L"",&regType))
				RemoveExternalSpaces(strGUID);

			regType=0;

			if (GetRegKey(strRegKeyName,L"Syntax",strSyntax,L"",&regType))
				RemoveExternalSpaces(strSyntax);

			regType=0;

			if (GetRegKey(strRegKeyName,L"Description",strDescription,L"",&regType))
				RemoveExternalSpaces(strDescription);

			MacroRecord mr={0};
			bool UsePluginFunc=true;
			if (!strBuffer.IsEmpty())
			{
				if (!ParseMacroString(&mr,strBuffer.CPtr()))
					mr.Buffer=0;
			}

			// ������������ Sequence ������ �������; ��� �� ����� �������, ���� GUID ����
			if ((Flags & 2) && (mr.Buffer || strGUID.IsEmpty()))
			{
				UsePluginFunc=false;
			}

			// ���������������� �������
			TMacroFunction MFunc={
				strFuncName.CPtr(),
				(int)nParams,
				(int)oParams,
				MCODE_F_NOFUNC,
				strGUID.CPtr(),
				mr.BufferSize,
				mr.Buffer,
				strSyntax.CPtr(),
				0,
				(UsePluginFunc?pluginsFunc:usersFunc)
			};

			KeyMacro::RegisterMacroFunction(&MFunc);

			if (mr.Buffer)
				xf_free(mr.Buffer);

		}

#endif
		return TRUE;
	}

	return FALSE;
}

void KeyMacro::RegisterMacroIntFunction()
{
	static bool InitedInternalFuncs=false;

	if (!InitedInternalFuncs)
	{
		for(size_t I=0; intMacroFunction[I].Name; ++I)
		{
			if (intMacroFunction[I].Code == MCODE_F_CALLPLUGIN)
			{
				if(!Opt.Macro.CallPluginRules)
					intMacroFunction[I].IntFlags |= IMFF_DISABLEINTINPUT;
			}

			KeyMacro::RegisterMacroFunction(intMacroFunction+I);
		}

		InitedInternalFuncs=true;
	}
}

TMacroFunction *KeyMacro::RegisterMacroFunction(const TMacroFunction *tmfunc)
{
	if (!tmfunc->Name || !tmfunc->Name[0])
		return nullptr;

	TMacroOpCode Code = tmfunc->Code;
	if ( !Code || Code == MCODE_F_NOFUNC) // �������� ��������� OpCode ������������ KEY_MACRO_U_BASE
		Code=(TMacroOpCode)GetNewOpCode();

	TMacroFunction *pTemp;

	if (CMacroFunction >= AllocatedFuncCount)
	{
		AllocatedFuncCount=AllocatedFuncCount+64;

		if (!(pTemp=(TMacroFunction *)xf_realloc(AMacroFunction,AllocatedFuncCount*sizeof(TMacroFunction))))
			return false;

		AMacroFunction=pTemp;
	}

	pTemp=AMacroFunction+CMacroFunction;

	pTemp->Name=xf_wcsdup(tmfunc->Name);
	pTemp->fnGUID=tmfunc->fnGUID?xf_wcsdup(tmfunc->fnGUID):nullptr;
	pTemp->Syntax=tmfunc->Syntax?xf_wcsdup(tmfunc->Syntax):nullptr;
	//pTemp->Src=tmfunc->Src?xf_wcsdup(tmfunc->Src):nullptr;
	//pTemp->Description=tmfunc->Description?xf_wcsdup(tmfunc->Description):nullptr;
	pTemp->nParam=tmfunc->nParam;
	pTemp->oParam=tmfunc->oParam;
	pTemp->Code=Code;
	pTemp->BufferSize=tmfunc->BufferSize;

	if (tmfunc->BufferSize > 0)
	{
		pTemp->Buffer=(DWORD *)xf_malloc(sizeof(DWORD)*tmfunc->BufferSize);
		if (pTemp->Buffer)
			memmove(pTemp->Buffer,tmfunc->Buffer,sizeof(DWORD)*tmfunc->BufferSize);
	}
	else
		pTemp->Buffer=nullptr;
	pTemp->IntFlags=tmfunc->IntFlags;
	pTemp->Func=tmfunc->Func;

	CMacroFunction++;
	return pTemp;
}

bool KeyMacro::UnregMacroFunction(size_t Index)
{
	if (static_cast<int>(Index) == -1)
	{
		if (AMacroFunction)
		{
			TMacroFunction *pTemp;
			for (size_t I=0; I < CMacroFunction; ++I)
			{
				pTemp=AMacroFunction+I;
				if (pTemp->Name)        xf_free((void*)pTemp->Name);        pTemp->Name=nullptr;
				if (pTemp->fnGUID)      xf_free((void*)pTemp->fnGUID);      pTemp->fnGUID=nullptr;
				if (pTemp->Syntax)      xf_free((void*)pTemp->Syntax);      pTemp->Syntax=nullptr;
				if (pTemp->Buffer)      xf_free((void*)pTemp->Buffer);      pTemp->Buffer=nullptr;
				//if (pTemp->Src)         xf_free((void*)pTemp->Src);         pTemp->Src=nullptr;
				//if (pTemp->Description) xf_free((void*)pTemp->Description); pTemp->Description=nullptr;
			}
			CMacroFunction=0;
			AllocatedFuncCount=0;
			xf_free(AMacroFunction);
			AMacroFunction=nullptr;
		}
	}
	else
	{
		if (AMacroFunction && Index < CMacroFunction)
			AMacroFunction[Index].Code=MCODE_F_NOFUNC;
		else
			return false;
	}

	return true;
}

const TMacroFunction *KeyMacro::GetMacroFunction(size_t Index)
{
	if (AMacroFunction && Index < CMacroFunction)
		return AMacroFunction+Index;

	return nullptr;
}

size_t KeyMacro::GetCountMacroFunction()
{
	return CMacroFunction;
}

DWORD KeyMacro::GetNewOpCode()
{
	return LastOpCodeUF++;
}

int KeyMacro::ReadMacros(int ReadMode, string &strBuffer)
{
	int I, J;
	MacroRecord CurMacro={0};
	string strUpKeyName=L"KeyMacros\\";
	strUpKeyName+=GetSubKey(ReadMode);
	string strRegKeyName, strKeyText;
	string strDescription;
	int ErrorCount=0;

	for (I=0;; I++)
	{
		DWORD MFlags=0;

		if (!EnumRegKey(strUpKeyName,I,strRegKeyName))
			break;

		size_t pos;

		if (strRegKeyName.RPos(pos,L'\\'))
		{
			strKeyText = strRegKeyName;
			strKeyText.LShift(pos+1);

			// ������! ��� �������� �������, ������������ �� ������ ~ - ���
			// ������������� ������!!!
			if (strKeyText.At(0) == L'~' && strKeyText.At(1))
			{
				pos = 1;

				while (strKeyText.At(pos) && strKeyText.At(pos) == L'~')// && IsSpace(KeyText[1]))
					++pos;

				strKeyText.LShift(pos);
				MFlags|=MFLAGS_DISABLEMACRO;
			}
		}
		else
			strKeyText.Clear();

		int KeyCode=KeyNameToKey(strKeyText);

		if (KeyCode==-1)
			continue;

		DWORD regType=0;

		if (GetRegKey(strRegKeyName,L"Sequence",strBuffer,L"",&regType) && regType == REG_MULTI_SZ)
		{
			//BUGBUG � ����� ����� REG_MULTI_SZ �������� � string?
			// ��������� ��� �� REG_MULTI_SZ
			wchar_t *ptrBuffer = strBuffer.GetBuffer();

			for (;;)
			{
				ptrBuffer+=StrLength(ptrBuffer);

				if (!ptrBuffer[0] && !ptrBuffer[1])
					break;

				*ptrBuffer=L'\n';
			}

			strBuffer.ReleaseBuffer();
		}

		RemoveExternalSpaces(strBuffer);

		if (strBuffer.IsEmpty())
		{
			//ErrorCount++; // �������������, ���� �� ����������� ������ "Sequence"
			continue;
		}

		CurMacro.Key=KeyCode;
		CurMacro.Buffer=nullptr;
		CurMacro.Src=nullptr;
		CurMacro.Description=nullptr;
		CurMacro.BufferSize=0;
		CurMacro.Flags=MFlags|(ReadMode&MFLAGS_MODEMASK)|(regType == REG_MULTI_SZ?MFLAGS_REG_MULTI_SZ:0);

		for (J=0; J < int(ARRAYSIZE(MKeywordsFlags)); ++J)
			CurMacro.Flags|=GetRegKey(strRegKeyName,MKeywordsFlags[J].Name,0)?MKeywordsFlags[J].Value:0;

		if (ReadMode == MACRO_EDITOR || ReadMode == MACRO_DIALOG || ReadMode == MACRO_VIEWER)
		{
			if (CurMacro.Flags&MFLAGS_SELECTION)
			{
				CurMacro.Flags&=~MFLAGS_SELECTION;
				CurMacro.Flags|=MFLAGS_EDITSELECTION;
			}

			if (CurMacro.Flags&MFLAGS_NOSELECTION)
			{
				CurMacro.Flags&=~MFLAGS_NOSELECTION;
				CurMacro.Flags|=MFLAGS_EDITNOSELECTION;
			}
		}

		if (!ParseMacroString(&CurMacro,strBuffer))
		{
			ErrorCount++;
			continue;
		}

		MacroRecord *NewMacros=(MacroRecord *)xf_realloc(MacroLIB,sizeof(*MacroLIB)*(MacroLIBCount+1));

		if (!NewMacros)
		{
			return FALSE;
		}

		MacroLIB=NewMacros;
		CurMacro.Src=xf_wcsdup(strBuffer);
		regType=0;

		if (GetRegKey(strRegKeyName,L"Description",strDescription,L"",&regType))
		{
			CurMacro.Description=xf_wcsdup(strDescription);
		}

		MacroLIB[MacroLIBCount]=CurMacro;
		MacroLIBCount++;
	}

	return ErrorCount?FALSE:TRUE;
}

// ��� ������� ����� ���������� �� ��� �������, ������� ����� ���������� ��������
void KeyMacro::RestartAutoMacro(int /*Mode*/)
{
#if 0
	/*
	�������      �������
	-------------------------------------------------------
	Other         0
	Shell         1 ���, ��� ������� ����
	Viewer        ��� ������ ����� ����� �������
	Editor        ��� ������ ����� ����� ��������
	Dialog        0
	Search        0
	Disks         0
	MainMenu      0
	Menu          0
	Help          0
	Info          1 ���, ��� ������� ���� � ����������� ����� ������
	QView         1 ���, ��� ������� ���� � ����������� ����� ������
	Tree          1 ���, ��� ������� ���� � ����������� ����� ������
	Common        0
	*/
#endif
}

// �������, ����������� ������� ��� ������ ����
// ���� �� ��������� �������������� � �������������� ���������
// �������� ��������, �� ������ ����!
void KeyMacro::RunStartMacro()
{
	if (Opt.Macro.DisableMacro&MDOL_ALL)
		return;

	if (Opt.Macro.DisableMacro&MDOL_AUTOSTART)
		return;

	// �������� ������� ������ �������
#if 1

	if (!(CtrlObject->Cp() && CtrlObject->Cp()->ActivePanel && !Opt.OnlyEditorViewerUsed && CtrlObject->Plugins.IsPluginsLoaded()))
		return;

	static int IsRunStartMacro=FALSE;

	if (IsRunStartMacro)
		return;

	if (!IndexMode[MACRO_SHELL][1])
		return;

	MacroRecord *MR=MacroLIB+IndexMode[MACRO_SHELL][0];

	for (int I=0; I < IndexMode[MACRO_SHELL][1]; ++I)
	{
		DWORD CurFlags;

		if (((CurFlags=MR[I].Flags)&MFLAGS_MODEMASK)==MACRO_SHELL &&
		        MR[I].BufferSize>0 &&
		        // ��������� �� ������������� �������
		        !(CurFlags&MFLAGS_DISABLEMACRO) &&
		        (CurFlags&MFLAGS_RUNAFTERFARSTART) && CtrlObject)
		{
			if (CheckAll(MACRO_SHELL,CurFlags))
				PostNewMacro(MR+I);
		}
	}

	IsRunStartMacro=TRUE;

#else
	static int AutoRunMacroStarted=FALSE;

	if (AutoRunMacroStarted || !MacroLIB || !IndexMode[Mode][1])
		return;

	//if (!(CtrlObject->Cp() && CtrlObject->Cp()->ActivePanel && !Opt.OnlyEditorViewerUsed && CtrlObject->Plugins.IsPluginsLoaded()))
	if (!(CtrlObject && CtrlObject->Plugins.IsPluginsLoaded()))
		return;

	MacroRecord *MR=MacroLIB+IndexMode[Mode][0];

	for (int I=0; I < IndexMode[Mode][1]; ++I)
	{
		DWORD CurFlags;

		if (((CurFlags=MR[I].Flags)&MFLAGS_MODEMASK)==Mode &&   // ���� ������ �� ���� �����?
		        MR[I].BufferSize > 0 &&                             // ���-�� ������ ����
		        !(CurFlags&MFLAGS_DISABLEMACRO) &&                  // ��������� �� ������������� �������
		        (CurFlags&MFLAGS_RUNAFTERFARSTART) &&               // � ���� ��, ��� ������ ����������
		        !(CurFlags&MFLAGS_RUNAFTERFARSTARTED)      // � ��� �����, ������� ��� �� ����������
		   )
		{
			if (CheckAll(Mode,CurFlags)) // ������ ��� ��������� - �������� �����
			{
				PostNewMacro(MR+I);
				MR[I].Flags|=MFLAGS_RUNAFTERFARSTARTED; // ���� ������ ������� �������� �� �����
			}
		}
	}

	// ��������� ���������� ���������� �������������� ��������
	int CntStart=0;

	for (int I=0; I < MacroLIBCount; ++I)
		if ((MacroLIB[I].Flags&MFLAGS_RUNAFTERFARSTART) && !(MacroLIB[I].Flags&MFLAGS_RUNAFTERFARSTARTED))
			CntStart++;

	if (!CntStart) // ������ ����� �������, ��� ��� ���������� � � ������� RunStartMacro() ������ ������
		AutoRunMacroStarted=TRUE;

#endif

	if (Work.Executing == MACROMODE_NOMACRO)
		Work.ExecLIBPos=0;  // � ���� ��?
}

// ���������� ����������� ���� ���������� �������
LONG_PTR WINAPI KeyMacro::AssignMacroDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
	string strKeyText;
	static int LastKey=0;
	static DlgParam *KMParam=nullptr;
	int Index;

	//_SVS(SysLog(L"LastKey=%d Msg=%s",LastKey,_DLGMSG_ToName(Msg)));
	if (Msg == DN_INITDIALOG)
	{
		KMParam=reinterpret_cast<DlgParam*>(Param2);
		LastKey=0;
		// <�������, ������� �� ������� � ������� ����������>
		DWORD PreDefKeyMain[]=
		{
			KEY_CTRLDOWN,KEY_ENTER,KEY_NUMENTER,KEY_ESC,KEY_F1,KEY_CTRLF5,
		};

		for (size_t i=0; i<ARRAYSIZE(PreDefKeyMain); i++)
		{
			KeyToText(PreDefKeyMain[i],strKeyText);
			SendDlgMessage(hDlg,DM_LISTADDSTR,2,reinterpret_cast<LONG_PTR>(strKeyText.CPtr()));
		}

		DWORD PreDefKey[]=
		{
			KEY_MSWHEEL_UP,KEY_MSWHEEL_DOWN,KEY_MSWHEEL_LEFT,KEY_MSWHEEL_RIGHT,
			KEY_MSLCLICK,KEY_MSRCLICK,KEY_MSM1CLICK,KEY_MSM2CLICK,KEY_MSM3CLICK,
#if 0
			KEY_MSLDBLCLICK,KEY_MSRDBLCLICK,KEY_MSM1DBLCLICK,KEY_MSM2DBLCLICK,KEY_MSM3DBLCLICK,
#endif
		};
		DWORD PreDefModKey[]=
		{
			0,KEY_CTRL,KEY_SHIFT,KEY_ALT,KEY_CTRLSHIFT,KEY_CTRLALT,KEY_ALTSHIFT,
		};

		for (size_t i=0; i<ARRAYSIZE(PreDefKey); i++)
		{
			SendDlgMessage(hDlg,DM_LISTADDSTR,2,reinterpret_cast<LONG_PTR>(L"\1"));

			for (size_t j=0; j<ARRAYSIZE(PreDefModKey); j++)
			{
				KeyToText(PreDefKey[i]|PreDefModKey[j],strKeyText);
				SendDlgMessage(hDlg,DM_LISTADDSTR,2,reinterpret_cast<LONG_PTR>(strKeyText.CPtr()));
			}
		}

		/*
		int KeySize=GetRegKeySize("KeyMacros","DlgKeys");
		char *KeyStr;
		if(KeySize &&
			(KeyStr=(char*)xf_malloc(KeySize+1))  &&
			GetRegKey("KeyMacros","DlgKeys",KeyStr,"",KeySize)
		)
		{
			UserDefinedList KeybList;
			if(KeybList.Set(KeyStr))
			{
				KeybList.Start();
				const char *OneKey;
				*KeyText=0;
				while(nullptr!=(OneKey=KeybList.GetNext()))
				{
					xstrncpy(KeyText, OneKey, sizeof(KeyText));
					SendDlgMessage(hDlg,DM_LISTADDSTR,2,(long)KeyText);
				}
			}
			xf_free(KeyStr);
		}
		*/
		SendDlgMessage(hDlg,DM_SETTEXTPTR,2,reinterpret_cast<LONG_PTR>(L""));
		// </�������, ������� �� ������� � ������� ����������>
	}
	else if (Param1 == 2 && Msg == DN_EDITCHANGE)
	{
		LastKey=0;
		_SVS(SysLog(L"[%d] ((FarDialogItem*)Param2)->PtrData='%s'",__LINE__,((FarDialogItem*)Param2)->PtrData));
		Param2=KeyNameToKey(((FarDialogItem*)Param2)->PtrData);

		if (Param2 != -1 && !KMParam->Recurse)
			goto M1;
	}
	else if (Msg == DN_KEY && (((Param2&KEY_END_SKEY) < KEY_END_FKEY) ||
	                           (((Param2&KEY_END_SKEY) > INTERNAL_KEY_BASE) && (Param2&KEY_END_SKEY) < INTERNAL_KEY_BASE_2)))
	{
		//if((Param2&0x00FFFFFF) >= 'A' && (Param2&0x00FFFFFF) <= 'Z' && ShiftPressed)
		//Param2|=KEY_SHIFT;

		//_SVS(SysLog(L"Macro: Key=%s",_FARKEY_ToName(Param2)));
		// <��������� ������ ������: F1 & Enter>
		// Esc & (Enter � ���������� Enter) - �� ������������
		if (Param2 == KEY_ESC ||
		        ((Param2 == KEY_ENTER||Param2 == KEY_NUMENTER) && (LastKey == KEY_ENTER||LastKey == KEY_NUMENTER)) ||
		        Param2 == KEY_CTRLDOWN ||
		        Param2 == KEY_F1)
		{
			return FALSE;
		}

		/*
		// F1 - ������ ������ - ����� ���� 2 ����
		// ������ ��� ����� ������� ����,
		// � ������ ��� - ������ ��� ��� ����������
		if(Param2 == KEY_F1 && LastKey!=KEY_F1)
		{
		  LastKey=KEY_F1;
		  return FALSE;
		}
		*/
		// ���� ���-�� ��� ������ � Enter`�� ������������
		_SVS(SysLog(L"[%d] Assign ==> Param2='%s',LastKey='%s'",__LINE__,_FARKEY_ToName((DWORD)Param2),(LastKey?_FARKEY_ToName(LastKey):L"")));

		if ((Param2 == KEY_ENTER||Param2 == KEY_NUMENTER) && LastKey && !(LastKey == KEY_ENTER||LastKey == KEY_NUMENTER))
			return FALSE;

		// </��������� ������ ������: F1 & Enter>
M1:
		_SVS(SysLog(L"[%d] Assign ==> Param2='%s',LastKey='%s'",__LINE__,_FARKEY_ToName((DWORD)Param2),LastKey?_FARKEY_ToName(LastKey):L""));
		KeyMacro *MacroDlg=KMParam->Handle;

		if ((Param2&0x00FFFFFF) > 0x7F && (Param2&0x00FFFFFF) < 0xFFFF)
			Param2=KeyToKeyLayout((int)(Param2&0x0000FFFF))|(DWORD)(Param2&(~0x0000FFFF));

		//���������
		if (Param2<0xFFFF)
			Param2=Upper((wchar_t)(Param2&0x0000FFFF))|(Param2&(~0x0000FFFF));

		_SVS(SysLog(L"[%d] Assign ==> Param2='%s',LastKey='%s'",__LINE__,_FARKEY_ToName((DWORD)Param2),LastKey?_FARKEY_ToName(LastKey):L""));
		KMParam->Key=(DWORD)Param2;
		KeyToText((int)Param2,strKeyText);

		// ���� ��� ���� ����� ������...
		if ((Index=MacroDlg->GetIndex((int)Param2,KMParam->Mode)) != -1)
		{
			MacroRecord *Mac=MacroDlg->MacroLIB+Index;

			// ����� ������� ��������� ������ ��� ��������.
			if (!MacroDlg->RecBuffer || !MacroDlg->RecBufferSize || (Mac->Flags&0xFF)!=MACRO_COMMON)
			{
				string strRegKeyName;
				MacroDlg->MkRegKeyName(Index, strRegKeyName);

				string strBufKey;
				if (Mac->Src )
				{
					strBufKey=Mac->Src;
					InsertQuote(strBufKey);
				}

				DWORD DisFlags=Mac->Flags&MFLAGS_DISABLEMACRO;
				string strBuf;
				if ((Mac->Flags&0xFF)==MACRO_COMMON)
					strBuf.Format(MSG(!MacroDlg->RecBufferSize?
					                  (DisFlags?MMacroCommonDeleteAssign:MMacroCommonDeleteKey):
							                  MMacroCommonReDefinedKey), strKeyText.CPtr());
				else
					strBuf.Format(MSG(!MacroDlg->RecBufferSize?
					                  (DisFlags?MMacroDeleteAssign:MMacroDeleteKey):
							                  MMacroReDefinedKey), strKeyText.CPtr());

				// �������� "� �� ��������� �� ��?"
				int Result=0;
				if (!(!DisFlags &&
				        Mac->Buffer && MacroDlg->RecBuffer &&
				        Mac->BufferSize == MacroDlg->RecBufferSize &&
				        (
				            (Mac->BufferSize >  1 && !memcmp(Mac->Buffer,MacroDlg->RecBuffer,MacroDlg->RecBufferSize*sizeof(DWORD))) ||
				            (Mac->BufferSize == 1 && (DWORD)(DWORD_PTR)Mac->Buffer == (DWORD)(DWORD_PTR)MacroDlg->RecBuffer)
				        )
				   ))
					Result=Message(MSG_WARNING,2,MSG(MWarning),
					          strBuf,
					          MSG(MMacroSequence),
					          strBufKey,
					          MSG(!MacroDlg->RecBufferSize?MMacroDeleteKey2:
					              (DisFlags?MMacroDisDisabledKey:MMacroReDefinedKey2)),
					          MSG(DisFlags && MacroDlg->RecBufferSize?MMacroDisOverwrite:MYes),
					          MSG(DisFlags && MacroDlg->RecBufferSize?MMacroDisAnotherKey:MNo));

				if (!Result)
				{
					if (DisFlags)
					{
						// ������� �� ������� ������ ���� ������� ��������
						if (Opt.AutoSaveSetup)
						{
							// ������ ������ ������ �� �������
							DeleteRegKey(strRegKeyName);
						}
						// �����������
						Mac->Flags&=~MFLAGS_DISABLEMACRO;
					}

					// � ����� ������ - ������������
					SendDlgMessage(hDlg,DM_CLOSE,1,0);
					return TRUE;
				}

				// ����� - ����� �� �������� "���", �� � �� ��� � ���� ���
				//  � ������ ������� ���� �����.
				strKeyText.Clear();
			}
		}

		KMParam->Recurse++;
		SendDlgMessage(hDlg,DM_SETTEXTPTR,2,(LONG_PTR)strKeyText.CPtr());
		KMParam->Recurse--;
		//if(Param2 == KEY_F1 && LastKey == KEY_F1)
		//LastKey=-1;
		//else
		LastKey=(int)Param2;
		return TRUE;
	}
	return DefDlgProc(hDlg,Msg,Param1,Param2);
}

DWORD KeyMacro::AssignMacroKey()
{
	/*
	  +------ Define macro ------+
	  | Press the desired key    |
	  | ________________________ |
	  +--------------------------+
	*/
	DialogDataEx MacroAssignDlgData[]=
	{
		DI_DOUBLEBOX,3,1,30,4,0,0,MSG(MDefineMacroTitle),
		DI_TEXT,-1,2,0,2,0,0,MSG(MDefineMacro),
		DI_COMBOBOX,5,3,28,3,0,DIF_FOCUS|DIF_DEFAULT,L"",
	};
	MakeDialogItemsEx(MacroAssignDlgData,MacroAssignDlg);
	DlgParam Param={this,0,StartMode,0};
	//_SVS(SysLog(L"StartMode=%d",StartMode));
	IsProcessAssignMacroKey++;
	Dialog Dlg(MacroAssignDlg,ARRAYSIZE(MacroAssignDlg),AssignMacroDlgProc,(LONG_PTR)&Param);
	Dlg.SetPosition(-1,-1,34,6);
	Dlg.SetHelp(L"KeyMacro");
	Dlg.Process();
	IsProcessAssignMacroKey--;

	if (Dlg.GetExitCode() == -1)
		return (DWORD)-1;

	return Param.Key;
}

static int Set3State(DWORD Flags,DWORD Chk1,DWORD Chk2)
{
	DWORD Chk12=Chk1|Chk2, FlagsChk12=Flags&Chk12;

	if (FlagsChk12 == Chk12 || !FlagsChk12)
		return (2);
	else
		return (Flags&Chk1?1:0);
}

enum MACROSETTINGSDLG
{
	MS_DOUBLEBOX,
	MS_TEXT_SEQUENCE,
	MS_EDIT_SEQUENCE,
	MS_SEPARATOR1,
	MS_CHECKBOX_OUPUT,
	MS_CHECKBOX_START,
	MS_SEPARATOR2,
	MS_CHECKBOX_A_PANEL,
	MS_CHECKBOX_A_PLUGINPANEL,
	MS_CHECKBOX_A_FOLDERS,
	MS_CHECKBOX_A_SELECTION,
	MS_CHECKBOX_P_PANEL,
	MS_CHECKBOX_P_PLUGINPANEL,
	MS_CHECKBOX_P_FOLDERS,
	MS_CHECKBOX_P_SELECTION,
	MS_SEPARATOR3,
	MS_CHECKBOX_CMDLINE,
	MS_CHECKBOX_SELBLOCK,
	MS_SEPARATOR4,
	MS_BUTTON_OK,
	MS_BUTTON_CANCEL,
};

LONG_PTR WINAPI KeyMacro::ParamMacroDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
	static DlgParam *KMParam=nullptr;

	switch (Msg)
	{
		case DN_INITDIALOG:
			KMParam=(DlgParam *)Param2;
			break;
		case DN_BTNCLICK:

			if (Param1==MS_CHECKBOX_A_PANEL || Param1==MS_CHECKBOX_P_PANEL)
				for (int i=1; i<=3; i++)
					SendDlgMessage(hDlg,DM_ENABLE,Param1+i,Param2);

			break;
		case DN_CLOSE:

			if (Param1==MS_BUTTON_OK)
			{
				MacroRecord mr={0};
				KeyMacro *Macro=KMParam->Handle;
				LPCWSTR Sequence=(LPCWSTR)SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,MS_EDIT_SEQUENCE,0);

				if (*Sequence)
				{
					if (Macro->ParseMacroString(&mr,Sequence))
					{
						xf_free(Macro->RecBuffer);
						Macro->RecBufferSize=mr.BufferSize;
						Macro->RecBuffer=mr.Buffer;
						Macro->RecSrc=xf_wcsdup(Sequence);
						return TRUE;
					}
				}

				return FALSE;
			}

			break;

		default:
			break;
	}

#if 0
	else if (Msg==DN_KEY && Param2==KEY_ALTF4)
	{
		KeyMacro *MacroDlg=KMParam->Handle;
		(*FrameManager)[0]->UnlockRefresh();
		FILE *MacroFile;
		char MacroFileName[NM];

		if (!FarMkTempEx(MacroFileName) || !(MacroFile=fopen(MacroFileName,"wb")))
			return TRUE;

		char *TextBuffer;
		DWORD Buf[1];
		Buf[0]=MacroDlg->RecBuffer[0];

		if ((TextBuffer=MacroDlg->MkTextSequence((MacroDlg->RecBufferSize==1?Buf:MacroDlg->RecBuffer),MacroDlg->RecBufferSize)) )
		{
			fwrite(TextBuffer,strlen(TextBuffer),1,MacroFile);
			fclose(MacroFile);
			xf_free(TextBuffer);
			{
				//ConsoleTitle *OldTitle=new ConsoleTitle;
				FileEditor ShellEditor(MacroFileName,-1,FFILEEDIT_DISABLEHISTORY,-1,-1,nullptr);
				//delete OldTitle;
				ShellEditor.SetDynamicallyBorn(false);
				FrameManager->EnterModalEV();
				FrameManager->ExecuteModal();
				FrameManager->ExitModalEV();

				if (!ShellEditor.IsFileChanged() || !(MacroFile=fopen(MacroFileName,"rb")))
					;
				else
				{
					MacroRecord NewMacroWORK2={0};
					long FileSize=filelen(MacroFile);
					TextBuffer=(char*)xf_malloc(FileSize);

					if (TextBuffer)
					{
						fread(TextBuffer,FileSize,1,MacroFile);

						if (!MacroDlg->ParseMacroString(&NewMacroWORK2,TextBuffer))
						{
							if (NewMacroWORK2.BufferSize > 1)
								xf_free(NewMacroWORK2.Buffer);
						}
						else
						{
							MacroDlg->RecBuffer=NewMacroWORK2.Buffer;
							MacroDlg->RecBufferSize=NewMacroWORK2.BufferSize;
						}
					}

					fclose(MacroFile);
				}
			}
			FrameManager->ResizeAllFrame();
			FrameManager->PluginCommit();
		}
		else
			fclose(MacroFile);

		remove(MacroFileName);
		return TRUE;
	}

#endif
	return DefDlgProc(hDlg,Msg,Param1,Param2);
}

int KeyMacro::GetMacroSettings(int Key,DWORD &Flags)
{
	/*
	          1         2         3         4         5         6
	   3456789012345678901234567890123456789012345678901234567890123456789
	 1 �=========== ��������� ������������ ��� 'CtrlP' ==================�
	 2 | ������������������:                                             |
	 3 | _______________________________________________________________ |
	 4 |-----------------------------------------------------------------|
	 5 | [ ] ��������� �� ����� ���������� ����� �� �����                |
	 6 | [ ] ��������� ����� ������� FAR                                 |
	 7 |-----------------------------------------------------------------|
	 8 | [ ] �������� ������             [ ] ��������� ������            |
	 9 |   [?] �� ������ �������           [?] �� ������ �������         |
	10 |   [?] ��������� ��� �����         [?] ��������� ��� �����       |
	11 |   [?] �������� �����              [?] �������� �����            |
	12 |-----------------------------------------------------------------|
	13 | [?] ������ ��������� ������                                     |
	14 | [?] ������� ����                                                |
	15 |-----------------------------------------------------------------|
	16 |               [ ���������� ]  [ �������� ]                      |
	17 L=================================================================+

	*/
	DialogDataEx MacroSettingsDlgData[]=
	{
		DI_DOUBLEBOX,3,1,69,17,0,0,L"",
		DI_TEXT,5,2,0,2,0,0,MSG(MMacroSequence),
		DI_EDIT,5,3,67,3,0,DIF_FOCUS,L"",
		DI_TEXT,3,4,0,4,0,DIF_SEPARATOR,L"",
		DI_CHECKBOX,5,5,0,5,0,0,MSG(MMacroSettingsEnableOutput),
		DI_CHECKBOX,5,6,0,6,0,0,MSG(MMacroSettingsRunAfterStart),
		DI_TEXT,3,7,0,7,0,DIF_SEPARATOR,L"",
		DI_CHECKBOX,5,8,0,8,0,0,MSG(MMacroSettingsActivePanel),
		DI_CHECKBOX,7,9,0,9,2,DIF_3STATE|DIF_DISABLE,MSG(MMacroSettingsPluginPanel),
		DI_CHECKBOX,7,10,0,10,2,DIF_3STATE|DIF_DISABLE,MSG(MMacroSettingsFolders),
		DI_CHECKBOX,7,11,0,11,2,DIF_3STATE|DIF_DISABLE,MSG(MMacroSettingsSelectionPresent),
		DI_CHECKBOX,37,8,0,8,0,0,MSG(MMacroSettingsPassivePanel),
		DI_CHECKBOX,39,9,0,9,2,DIF_3STATE|DIF_DISABLE,MSG(MMacroSettingsPluginPanel),
		DI_CHECKBOX,39,10,0,10,2,DIF_3STATE|DIF_DISABLE,MSG(MMacroSettingsFolders),
		DI_CHECKBOX,39,11,0,11,2,DIF_3STATE|DIF_DISABLE,MSG(MMacroSettingsSelectionPresent),
		DI_TEXT,3,12,0,12,0,DIF_SEPARATOR,L"",
		DI_CHECKBOX,5,13,0,13,2,DIF_3STATE,MSG(MMacroSettingsCommandLine),
		DI_CHECKBOX,5,14,0,14,2,DIF_3STATE,MSG(MMacroSettingsSelectionBlockPresent),
		DI_TEXT,3,15,0,15,0,DIF_SEPARATOR,L"",
		DI_BUTTON,0,16,0,16,0,DIF_DEFAULT|DIF_CENTERGROUP,MSG(MOk),
		DI_BUTTON,0,16,0,16,0,DIF_CENTERGROUP,MSG(MCancel),
	};
	MakeDialogItemsEx(MacroSettingsDlgData,MacroSettingsDlg);
	string strKeyText;
	KeyToText(Key,strKeyText);
	MacroSettingsDlg[MS_DOUBLEBOX].strData.Format(MSG(MMacroSettingsTitle), strKeyText.CPtr());
	//if(!(Key&0x7F000000))
	//MacroSettingsDlg[3].Flags|=DIF_DISABLE;
	MacroSettingsDlg[MS_CHECKBOX_OUPUT].Selected=Flags&MFLAGS_DISABLEOUTPUT?0:1;
	MacroSettingsDlg[MS_CHECKBOX_START].Selected=Flags&MFLAGS_RUNAFTERFARSTART?1:0;
	MacroSettingsDlg[MS_CHECKBOX_A_PLUGINPANEL].Selected=Set3State(Flags,MFLAGS_NOFILEPANELS,MFLAGS_NOPLUGINPANELS);
	MacroSettingsDlg[MS_CHECKBOX_A_FOLDERS].Selected=Set3State(Flags,MFLAGS_NOFILES,MFLAGS_NOFOLDERS);
	MacroSettingsDlg[MS_CHECKBOX_A_SELECTION].Selected=Set3State(Flags,MFLAGS_SELECTION,MFLAGS_NOSELECTION);
	MacroSettingsDlg[MS_CHECKBOX_P_PLUGINPANEL].Selected=Set3State(Flags,MFLAGS_PNOFILEPANELS,MFLAGS_PNOPLUGINPANELS);
	MacroSettingsDlg[MS_CHECKBOX_P_FOLDERS].Selected=Set3State(Flags,MFLAGS_PNOFILES,MFLAGS_PNOFOLDERS);
	MacroSettingsDlg[MS_CHECKBOX_P_SELECTION].Selected=Set3State(Flags,MFLAGS_PSELECTION,MFLAGS_PNOSELECTION);
	MacroSettingsDlg[MS_CHECKBOX_CMDLINE].Selected=Set3State(Flags,MFLAGS_EMPTYCOMMANDLINE,MFLAGS_NOTEMPTYCOMMANDLINE);
	MacroSettingsDlg[MS_CHECKBOX_SELBLOCK].Selected=Set3State(Flags,MFLAGS_EDITSELECTION,MFLAGS_EDITNOSELECTION);
	LPWSTR Sequence=MkTextSequence(RecBuffer,RecBufferSize);
	MacroSettingsDlg[MS_EDIT_SEQUENCE].strData=Sequence;
	xf_free(Sequence);
	DlgParam Param={this,0,0,0};
	Dialog Dlg(MacroSettingsDlg,ARRAYSIZE(MacroSettingsDlg),ParamMacroDlgProc,(LONG_PTR)&Param);
	Dlg.SetPosition(-1,-1,73,19);
	Dlg.SetHelp(L"KeyMacroSetting");
	Frame* BottomFrame = FrameManager->GetBottomFrame();
	if(BottomFrame)
	{
		BottomFrame->Lock(); // ������� ���������� ������
	}
	Dlg.Process();
	if(BottomFrame)
	{
		BottomFrame->Unlock(); // ������ ����� :-)
	}

	if (Dlg.GetExitCode()!=MS_BUTTON_OK)
		return FALSE;

	Flags=MacroSettingsDlg[MS_CHECKBOX_OUPUT].Selected?0:MFLAGS_DISABLEOUTPUT;
	Flags|=MacroSettingsDlg[MS_CHECKBOX_START].Selected?MFLAGS_RUNAFTERFARSTART:0;

	if (MacroSettingsDlg[MS_CHECKBOX_A_PANEL].Selected)
	{
		Flags|=MacroSettingsDlg[MS_CHECKBOX_A_PLUGINPANEL].Selected==2?0:
		       (MacroSettingsDlg[MS_CHECKBOX_A_PLUGINPANEL].Selected==0?MFLAGS_NOPLUGINPANELS:MFLAGS_NOFILEPANELS);
		Flags|=MacroSettingsDlg[MS_CHECKBOX_A_FOLDERS].Selected==2?0:
		       (MacroSettingsDlg[MS_CHECKBOX_A_FOLDERS].Selected==0?MFLAGS_NOFOLDERS:MFLAGS_NOFILES);
		Flags|=MacroSettingsDlg[MS_CHECKBOX_A_SELECTION].Selected==2?0:
		       (MacroSettingsDlg[MS_CHECKBOX_A_SELECTION].Selected==0?MFLAGS_NOSELECTION:MFLAGS_SELECTION);
	}

	if (MacroSettingsDlg[MS_CHECKBOX_P_PANEL].Selected)
	{
		Flags|=MacroSettingsDlg[MS_CHECKBOX_P_PLUGINPANEL].Selected==2?0:
		       (MacroSettingsDlg[MS_CHECKBOX_P_PLUGINPANEL].Selected==0?MFLAGS_PNOPLUGINPANELS:MFLAGS_PNOFILEPANELS);
		Flags|=MacroSettingsDlg[MS_CHECKBOX_P_FOLDERS].Selected==2?0:
		       (MacroSettingsDlg[MS_CHECKBOX_P_FOLDERS].Selected==0?MFLAGS_PNOFOLDERS:MFLAGS_PNOFILES);
		Flags|=MacroSettingsDlg[MS_CHECKBOX_P_SELECTION].Selected==2?0:
		       (MacroSettingsDlg[MS_CHECKBOX_P_SELECTION].Selected==0?MFLAGS_PNOSELECTION:MFLAGS_PSELECTION);
	}

	Flags|=MacroSettingsDlg[MS_CHECKBOX_CMDLINE].Selected==2?0:
	       (MacroSettingsDlg[MS_CHECKBOX_CMDLINE].Selected==0?MFLAGS_NOTEMPTYCOMMANDLINE:MFLAGS_EMPTYCOMMANDLINE);
	Flags|=MacroSettingsDlg[MS_CHECKBOX_SELBLOCK].Selected==2?0:
	       (MacroSettingsDlg[MS_CHECKBOX_SELBLOCK].Selected==0?MFLAGS_EDITNOSELECTION:MFLAGS_EDITSELECTION);
	return TRUE;
}

int KeyMacro::PostNewMacro(const wchar_t *PlainText,DWORD Flags,DWORD AKey,BOOL onlyCheck)
{
	MacroRecord NewMacroWORK2={0};
	wchar_t *Buffer=(wchar_t *)PlainText;
	bool allocBuffer=false;

	if (Flags&MFLAGS_REG_MULTI_SZ) // ��������� ��� �� REG_MULTI_SZ
	{
		int lenPlainText=0;

		for (;;)
		{
			if (!PlainText[lenPlainText] && !PlainText[lenPlainText+1])
			{
				lenPlainText+=2;
				break;
			}

			lenPlainText++;
		}

		//lenPlainText++;
		Buffer=(wchar_t*)xf_malloc((lenPlainText+1)*(int)sizeof(wchar_t));

		if (Buffer)
		{
			allocBuffer=true;
			wmemmove(Buffer,PlainText,lenPlainText);
			Buffer[lenPlainText]=0; // +1
			wchar_t *ptrBuffer=Buffer;

			for (;;)
			{
				ptrBuffer+=StrLength(ptrBuffer);

				if (!ptrBuffer[0] && !ptrBuffer[1])
					break;

				*ptrBuffer=L'\n';
			}
		}
		else
			return FALSE;
	}

	// ������� ������� �� ������
	BOOL parsResult=ParseMacroString(&NewMacroWORK2,Buffer,onlyCheck);

	if (allocBuffer && Buffer)
		xf_free(Buffer);

	if (!parsResult)
	{
		if (NewMacroWORK2.BufferSize > 1)
			xf_free(NewMacroWORK2.Buffer);

		return FALSE;
	}

	if (onlyCheck)
	{
		if (NewMacroWORK2.BufferSize > 1)
			xf_free(NewMacroWORK2.Buffer);

		return TRUE;
	}

	NewMacroWORK2.Flags=Flags;
	NewMacroWORK2.Key=AKey;
	// ������ ��������� �������� ������� ������ ������
	MacroRecord *NewMacroWORK;

	if (!(NewMacroWORK=(MacroRecord *)xf_realloc(Work.MacroWORK,sizeof(MacroRecord)*(Work.MacroWORKCount+1))))
	{
		if (NewMacroWORK2.BufferSize > 1)
			xf_free(NewMacroWORK2.Buffer);

		return FALSE;
	}

	// ������ ������� � ���� "�������" ����� ������
	Work.MacroWORK=NewMacroWORK;
	NewMacroWORK=Work.MacroWORK+Work.MacroWORKCount;
	*NewMacroWORK=NewMacroWORK2;
	Work.MacroWORKCount++;

	//Work.Executing=Work.MacroWORK->Flags&MFLAGS_NOSENDKEYSTOPLUGINS?MACROMODE_EXECUTING:MACROMODE_EXECUTING_COMMON;
	if (Work.ExecLIBPos == Work.MacroWORK->BufferSize)
		Work.ExecLIBPos=0;

	return TRUE;
}

int KeyMacro::PostNewMacro(MacroRecord *MRec,BOOL NeedAddSendFlag,BOOL IsPluginSend)
{
	if (!MRec)
		return FALSE;

	MacroRecord NewMacroWORK2={0};
	NewMacroWORK2=*MRec;
	NewMacroWORK2.Src=nullptr;
	NewMacroWORK2.Description=nullptr;
	//if(MRec->BufferSize > 1)
	{
		if (!(NewMacroWORK2.Buffer=(DWORD*)xf_malloc((MRec->BufferSize+3)*sizeof(DWORD))))
		{
			return FALSE;
		}
	}
	// ������ ��������� �������� ������� ������ ������
	MacroRecord *NewMacroWORK;

	if (!(NewMacroWORK=(MacroRecord *)xf_realloc(Work.MacroWORK,sizeof(MacroRecord)*(Work.MacroWORKCount+1))))
	{
		//if(MRec->BufferSize > 1)
		xf_free(NewMacroWORK2.Buffer);
		return FALSE;
	}

	// ������ ������� � ���� "�������" ����� ������
	if (IsPluginSend)
		NewMacroWORK2.Buffer[0]=MCODE_OP_KEYS;

	if ((MRec->BufferSize+1) > 2)
		memcpy(&NewMacroWORK2.Buffer[IsPluginSend?1:0],MRec->Buffer,sizeof(DWORD)*MRec->BufferSize);
	else if (MRec->Buffer)
		NewMacroWORK2.Buffer[IsPluginSend?1:0]=(DWORD)(DWORD_PTR)MRec->Buffer;

	if (IsPluginSend)
		NewMacroWORK2.Buffer[NewMacroWORK2.BufferSize+1]=MCODE_OP_ENDKEYS;

	//NewMacroWORK2.Buffer[NewMacroWORK2.BufferSize]=MCODE_OP_NOP; // ���.�������/��������

	if (IsPluginSend)
		NewMacroWORK2.BufferSize+=2;

	Work.MacroWORK=NewMacroWORK;
	NewMacroWORK=Work.MacroWORK+Work.MacroWORKCount;
	*NewMacroWORK=NewMacroWORK2;
	Work.MacroWORKCount++;

	//Work.Executing=Work.MacroWORK->Flags&MFLAGS_NOSENDKEYSTOPLUGINS?MACROMODE_EXECUTING:MACROMODE_EXECUTING_COMMON;
	if (Work.ExecLIBPos == Work.MacroWORK->BufferSize)
		Work.ExecLIBPos=0;

	return TRUE;
}

int KeyMacro::ParseMacroString(MacroRecord *CurMacro,const wchar_t *BufPtr,BOOL onlyCheck)
{
	BOOL Result=FALSE;

	if (CurMacro)
	{
		Result=__parseMacroString(CurMacro->Buffer, CurMacro->BufferSize, BufPtr);

		if (!Result && !onlyCheck)
		{
			// TODO: ���� ����� ������ ������������ ����������� ������ SILENT!
			bool scrLocks=LockScr!=nullptr;
			string ErrMsg[4];

			if (scrLocks) // ���� ��� - ������
			{
				if (LockScr) delete LockScr;

				LockScr=nullptr;
			}

			InternalInput++; // InternalInput - ������������ ����, ����� ������ �� ��������� ���� ����������
			GetMacroParseError(&ErrMsg[0],&ErrMsg[1],&ErrMsg[2],&ErrMsg[3]);
			//if(...)
			string strTitle=MSG(MMacroPErrorTitle);
			if(CurMacro->Key)
			{
				strTitle+=L" ";
				string strKey;
				KeyToText(CurMacro->Key,strKey);
				strTitle.Append(GetSubKey(LOBYTE(LOWORD(CurMacro->Flags)))).Append(L"\\").Append(strKey);
			}
			Message(MSG_WARNING|MSG_LEFTALIGN,1,strTitle,ErrMsg[3]+L":",ErrMsg[0],L"\x1",ErrMsg[1],ErrMsg[2],L"\x1",MSG(MOk));
			//else
			// ������� ����������� � ����
			InternalInput--;

			if (scrLocks) // ���� ���� - �������
			{
				if (LockScr) delete LockScr;

				LockScr=new LockScreen;
			}
		}
	}

	return Result;
}


void MacroState::Init(TVarTable *tbl)
{
	KeyProcess=Executing=MacroPC=ExecLIBPos=MacroWORKCount=0;
	HistroyEnable=0;
	MacroWORK=nullptr;

	if (!tbl)
	{
		AllocVarTable=true;
		locVarTable=(TVarTable*)xf_malloc(sizeof(TVarTable));
		initVTable(*locVarTable);
	}
	else
	{
		AllocVarTable=false;
		locVarTable=tbl;
	}
}

int KeyMacro::PushState(bool CopyLocalVars)
{
	if (CurPCStack+1 >= STACKLEVEL)
		return FALSE;

	++CurPCStack;
	Work.UseInternalClipboard=Clipboard::GetUseInternalClipboardState();
	PCStack[CurPCStack]=Work;
	Work.Init(CopyLocalVars?PCStack[CurPCStack].locVarTable:nullptr);
	return TRUE;
}

int KeyMacro::PopState()
{
	if (CurPCStack < 0)
		return FALSE;

	Work=PCStack[CurPCStack];
	Clipboard::SetUseInternalClipboardState(Work.UseInternalClipboard);
	CurPCStack--;
	return TRUE;
}

// ������� ��������� ������� ������� ������� � �������
// Ret=-1 - �� ������ �������.
// ���� CheckMode=-1 - ������ ������ � ����� ������, �.�. ������ ����������
int KeyMacro::GetIndex(int Key, int ChechMode, bool UseCommon)
{
	if (MacroLIB)
	{
		for (int I=0; I < 2; ++I)
		{
			int Pos,Len;
			MacroRecord *MPtr=nullptr;

			if (ChechMode == -1)
			{
				Len=MacroLIBCount;
				MPtr=MacroLIB;
			}
			else if (ChechMode >= 0 && ChechMode < MACRO_LAST)
			{
				Len=IndexMode[ChechMode][1];

				if (Len)
					MPtr=MacroLIB+IndexMode[ChechMode][0];

				//_SVS(SysLog(L"ChechMode=%d (%d,%d)",ChechMode,IndexMode[ChechMode][0],IndexMode[ChechMode][1]));
			}
			else
			{
				Len=0;
			}

			if (Len)
			{
				for (Pos=0; Pos < Len; ++Pos, ++MPtr)
				{
					if (!((MPtr->Key ^ Key) & ~0xFFFF) &&
					        (Upper(static_cast<WCHAR>(MPtr->Key))==Upper(static_cast<WCHAR>(Key))) &&
					        (MPtr->BufferSize > 0))
					{
						//        && (ChechMode == -1 || (MPtr->Flags&MFLAGS_MODEMASK) == ChechMode))
						//_SVS(SysLog(L"GetIndex: Pos=%d MPtr->Key=0x%08X", Pos,MPtr->Key));
						if (!(MPtr->Flags&MFLAGS_DISABLEMACRO))
							return Pos+((ChechMode >= 0)?IndexMode[ChechMode][0]:0);
					}
				}
			}

			// ����� ������� �� MACRO_COMMON
			if (ChechMode != -1 && !I && UseCommon)
				ChechMode=MACRO_COMMON;
			else
				break;
		}
	}

	return -1;
}

// ��������� �������, ����������� ��������� ��������
// Ret= 0 - �� ������ �������.
// ���� CheckMode=-1 - ������ ������ � ����� ������, �.�. ������ ����������
int KeyMacro::GetRecordSize(int Key, int CheckMode)
{
	int Pos=GetIndex(Key,CheckMode);

	if (Pos == -1)
		return 0;

	return sizeof(MacroRecord)+MacroLIB[Pos].BufferSize;
}

// �������� �������� ���� �� ����
const wchar_t* KeyMacro::GetSubKey(int Mode)
{
	return (Mode >= MACRO_FUNCS && Mode < MACRO_LAST)?MKeywordsArea[Mode+3].Name:L"";
}

// �������� ��� ���� �� �����
int KeyMacro::GetSubKey(const wchar_t *Mode)
{
	for (int i=MACRO_FUNCS; i < MACRO_LAST; i++)
		if (!StrCmpI(MKeywordsArea[i+3].Name,Mode))
			return i;

	return MACRO_FUNCS-1;
}

int KeyMacro::GetMacroKeyInfo(bool FromReg,int Mode,int Pos, string &strKeyName, string &strDescription)
{
	if (Mode >= MACRO_FUNCS && Mode < MACRO_LAST)
	{
		if (FromReg)
		{
			FormatString strUpKeyName;
			string strRegKeyName;
			strUpKeyName << L"KeyMacros\\" << GetSubKey(Mode);

			if (Mode >= MACRO_OTHER)
			{
				string strSyntax, strDescr;

				if (!EnumRegKey(strUpKeyName,Pos,strRegKeyName))
					return -1;

				DWORD regType=0;
				GetRegKey(strRegKeyName,L"Description",strDescr,L"",&regType);

				if (Mode == MACRO_FUNCS)
				{
					regType=0;
					GetRegKey(strRegKeyName,L"Syntax",strSyntax,L"",&regType);
					strDescription = strSyntax + (strSyntax.GetLength() > 0 ? L" - " : L"") + strDescr;
				}
				else
				{
					strDescription = strDescr;
				}

				size_t pos;

				if (strRegKeyName.RPos(pos,L'\\'))
					strKeyName = strRegKeyName.SubStr(pos+1);
				else
					strKeyName.Clear();

				return Pos+1;
			}
			else if (Mode == MACRO_FUNCS)
			{
				// TODO: MACRO_FUNCS
				return -1;
			}
			else
			{
				string strSData;
				DWORD IData;
				__int64 IData64;
				DWORD Type;

				if (!EnumRegValueEx(strUpKeyName,Pos,strRegKeyName,strSData, &IData, &IData64, &Type))
					return -1;

				strKeyName = strRegKeyName;

				switch (Type)
				{
					case REG_DWORD:
						strDescription.Format(MSG(MMacroOutputFormatForHelpDWord), IData, IData);
						break;
					case REG_QWORD:
						strDescription.Format(MSG(MMacroOutputFormatForHelpQWord), IData64, IData64);
						break;
					case REG_SZ:
					case REG_EXPAND_SZ:
					case REG_MULTI_SZ:
						strDescription.Format(MSG(MMacroOutputFormatForHelpSz), strSData.CPtr());
						break;
				}

				return Pos+1;
			}
		}
		else
		{
			if (Mode >= MACRO_OTHER)
			{
				int Len=CtrlObject->Macro.IndexMode[Mode][1];

				if (Len && Pos < Len)
				{
					MacroRecord *MPtr=CtrlObject->Macro.MacroLIB+CtrlObject->Macro.IndexMode[Mode][0]+Pos;
					::KeyToText(MPtr->Key,strKeyName);
					strDescription=NullToEmpty(MPtr->Description);
					return Pos+1;
				}
			}
			else if (Mode == MACRO_FUNCS)
			{
				// TODO: MACRO_FUNCS
				return -1;
			}
			else
			{
				TVarSet *var=varEnum((Mode==MACRO_VARS)?glbVarTable:glbConstTable,Pos);

				if (!var)
					return -1;

				strKeyName = var->str;
				strKeyName = (Mode==MACRO_VARS?L"%":L"")+strKeyName;

				switch (var->value.type())
				{
					case vtInteger:
					{
						__int64 IData64=var->value.i();
						strDescription.Format(MSG(MMacroOutputFormatForHelpQWord), IData64, IData64);
						break;
					}
					case vtDouble:
					{
						double FData=var->value.d();
						strDescription.Format(MSG(MMacroOutputFormatForHelpDouble), FData);
						break;
					}
					case vtString:
						strDescription.Format(MSG(MMacroOutputFormatForHelpSz), var->value.s());
						break;
					default:
						break;
				}

				return Pos+1;
			}
		}
	}

	return -1;
}

BOOL KeyMacro::CheckEditSelected(DWORD CurFlags)
{
	if (Mode==MACRO_EDITOR || Mode==MACRO_DIALOG || Mode==MACRO_VIEWER || (Mode==MACRO_SHELL&&CtrlObject->CmdLine->IsVisible()))
	{
		int NeedType = Mode == MACRO_EDITOR?MODALTYPE_EDITOR:(Mode == MACRO_VIEWER?MODALTYPE_VIEWER:(Mode == MACRO_DIALOG?MODALTYPE_DIALOG:MODALTYPE_PANELS));
		Frame* CurFrame=FrameManager->GetCurrentFrame();

		if (CurFrame && CurFrame->GetType()==NeedType)
		{
			int CurSelected;

			if (Mode==MACRO_SHELL && CtrlObject->CmdLine->IsVisible())
				CurSelected=(int)CtrlObject->CmdLine->VMProcess(MCODE_C_SELECTED);
			else
				CurSelected=(int)CurFrame->VMProcess(MCODE_C_SELECTED);

			if (((CurFlags&MFLAGS_EDITSELECTION) && !CurSelected) ||	((CurFlags&MFLAGS_EDITNOSELECTION) && CurSelected))
				return FALSE;
		}
	}

	return TRUE;
}

BOOL KeyMacro::CheckInsidePlugin(DWORD CurFlags)
{
	if (CtrlObject && CtrlObject->Plugins.CurPluginItem && (CurFlags&MFLAGS_NOSENDKEYSTOPLUGINS)) // ?????
		//if(CtrlObject && CtrlObject->Plugins.CurEditor && (CurFlags&MFLAGS_NOSENDKEYSTOPLUGINS))
		return FALSE;

	return TRUE;
}

BOOL KeyMacro::CheckCmdLine(int CmdLength,DWORD CurFlags)
{
	if (((CurFlags&MFLAGS_EMPTYCOMMANDLINE) && CmdLength) || ((CurFlags&MFLAGS_NOTEMPTYCOMMANDLINE) && CmdLength==0))
		return FALSE;

	return TRUE;
}

BOOL KeyMacro::CheckPanel(int PanelMode,DWORD CurFlags,BOOL IsPassivePanel)
{
	if (IsPassivePanel)
	{
		if ((PanelMode == PLUGIN_PANEL && (CurFlags&MFLAGS_PNOPLUGINPANELS)) || (PanelMode == NORMAL_PANEL && (CurFlags&MFLAGS_PNOFILEPANELS)))
			return FALSE;
	}
	else
	{
		if ((PanelMode == PLUGIN_PANEL && (CurFlags&MFLAGS_NOPLUGINPANELS)) || (PanelMode == NORMAL_PANEL && (CurFlags&MFLAGS_NOFILEPANELS)))
			return FALSE;
	}

	return TRUE;
}

BOOL KeyMacro::CheckFileFolder(Panel *CheckPanel,DWORD CurFlags, BOOL IsPassivePanel)
{
	string strFileName;
	DWORD FileAttr=INVALID_FILE_ATTRIBUTES;
	CheckPanel->GetFileName(strFileName,CheckPanel->GetCurrentPos(),FileAttr);

	if (FileAttr != INVALID_FILE_ATTRIBUTES)
	{
		if (IsPassivePanel)
		{
			if (((FileAttr&FILE_ATTRIBUTE_DIRECTORY) && (CurFlags&MFLAGS_PNOFOLDERS)) || (!(FileAttr&FILE_ATTRIBUTE_DIRECTORY) && (CurFlags&MFLAGS_PNOFILES)))
				return FALSE;
		}
		else
		{
			if (((FileAttr&FILE_ATTRIBUTE_DIRECTORY) && (CurFlags&MFLAGS_NOFOLDERS)) || (!(FileAttr&FILE_ATTRIBUTE_DIRECTORY) && (CurFlags&MFLAGS_NOFILES)))
				return FALSE;
		}
	}

	return TRUE;
}

BOOL KeyMacro::CheckAll(int /*CheckMode*/,DWORD CurFlags)
{
	/* $TODO:
		����� ������ Check*() ����������� ������� IfCondition()
		��� ���������� �������������� ����.
	*/
	if (!CheckInsidePlugin(CurFlags))
		return FALSE;

	// �������� �� �����/�� ����� � ���.������ (� � ���������? :-)
	if (CurFlags&(MFLAGS_EMPTYCOMMANDLINE|MFLAGS_NOTEMPTYCOMMANDLINE))
		if (CtrlObject->CmdLine && !CheckCmdLine(CtrlObject->CmdLine->GetLength(),CurFlags))
			return FALSE;

	FilePanels *Cp=CtrlObject->Cp();

	if (!Cp)
		return FALSE;

	// �������� ������ � ���� �����
	Panel *ActivePanel=Cp->ActivePanel;
	Panel *PassivePanel=Cp->GetAnotherPanel(Cp->ActivePanel);

	if (ActivePanel && PassivePanel)// && (CurFlags&MFLAGS_MODEMASK)==MACRO_SHELL)
	{
		if (CurFlags&(MFLAGS_NOPLUGINPANELS|MFLAGS_NOFILEPANELS))
			if (!CheckPanel(ActivePanel->GetMode(),CurFlags,FALSE))
				return FALSE;

		if (CurFlags&(MFLAGS_PNOPLUGINPANELS|MFLAGS_PNOFILEPANELS))
			if (!CheckPanel(PassivePanel->GetMode(),CurFlags,TRUE))
				return FALSE;

		if (CurFlags&(MFLAGS_NOFOLDERS|MFLAGS_NOFILES))
			if (!CheckFileFolder(ActivePanel,CurFlags,FALSE))
				return FALSE;

		if (CurFlags&(MFLAGS_PNOFOLDERS|MFLAGS_PNOFILES))
			if (!CheckFileFolder(PassivePanel,CurFlags,TRUE))
				return FALSE;

		if (CurFlags&(MFLAGS_SELECTION|MFLAGS_NOSELECTION|MFLAGS_PSELECTION|MFLAGS_PNOSELECTION))
			if (Mode!=MACRO_EDITOR && Mode != MACRO_DIALOG && Mode!=MACRO_VIEWER)
			{
				int SelCount=ActivePanel->GetRealSelCount();

				if (((CurFlags&MFLAGS_SELECTION) && SelCount < 1) || ((CurFlags&MFLAGS_NOSELECTION) && SelCount >= 1))
					return FALSE;

				SelCount=PassivePanel->GetRealSelCount();

				if (((CurFlags&MFLAGS_PSELECTION) && SelCount < 1) || ((CurFlags&MFLAGS_PNOSELECTION) && SelCount >= 1))
					return FALSE;
			}
	}

	if (!CheckEditSelected(CurFlags))
		return FALSE;

	return TRUE;
}

/*
  Return: FALSE - ���� ����������� MFLAGS_* �� ���������� ���
                  ��� �� ����� ���������� �������!
          TRUE  - ����� ����(�) ����������(�)
*/
BOOL KeyMacro::CheckCurMacroFlags(DWORD Flags)
{
	if (Work.Executing && Work.MacroWORK)
	{
		return (Work.MacroWORK->Flags&Flags)?TRUE:FALSE;
	}

	return FALSE;
}

/*
  Return: 0 - �� � ������ �����, 1 - Executing, 2 - Executing common, 3 - Recording, 4 - Recording common
  See MacroRecordAndExecuteType
*/
int KeyMacro::GetCurRecord(MacroRecord* RBuf,int *KeyPos)
{
	if (KeyPos && RBuf)
	{
		*KeyPos=Work.Executing?Work.ExecLIBPos:0;
		memset(RBuf,0,sizeof(MacroRecord));

		if (Recording == MACROMODE_NOMACRO)
		{
			if (Work.Executing)
			{
				*RBuf=*Work.MacroWORK;   //MacroLIB[Work.MacroPC]; //????
				return Work.Executing;
			}

			memset(RBuf,0,sizeof(MacroRecord));
			return MACROMODE_NOMACRO;
		}

		RBuf->BufferSize=RecBufferSize;
		RBuf->Buffer=RecBuffer;

		return Recording==MACROMODE_RECORDING?MACROMODE_RECORDING:MACROMODE_RECORDING_COMMON;
	}

	return Recording?(Recording==MACROMODE_RECORDING?MACROMODE_RECORDING:MACROMODE_RECORDING_COMMON):(Work.Executing?Work.Executing:MACROMODE_NOMACRO);
}


bool KeyMacro::IsHistroyEnable(int TypeHistory)
{
	return (Work.HistroyEnable & (1 << TypeHistory))?true:false;
}

static int __cdecl SortMacros(const MacroRecord *el1,const MacroRecord *el2)
{
	int Mode1, Mode2;

	if ((Mode1=(el1->Flags&MFLAGS_MODEMASK)) == (Mode2=(el2->Flags&MFLAGS_MODEMASK)))
		return 0;

	if (Mode1 < Mode2)
		return -1;

	return 1;
}

// ���������� ��������� ������
void KeyMacro::Sort()
{
	typedef int (__cdecl *qsort_fn)(const void*,const void*);
	// ���������
	far_qsort(MacroLIB,MacroLIBCount,sizeof(MacroRecord),(qsort_fn)SortMacros);
	// ������������� ������ �����
	int CurMode=MACRO_OTHER;
	memset(IndexMode,0,sizeof(IndexMode));

	for (int I=0; I<MacroLIBCount; I++)
	{
		int J=MacroLIB[I].Flags&MFLAGS_MODEMASK;

		if (CurMode != J)
		{
			IndexMode[J][0]=I;
			CurMode=J;
		}

		IndexMode[J][1]++;
	}

	//_SVS(for(I=0; I < ARRAYSIZE(IndexMode); ++I)SysLog(L"IndexMode[%02d.%s]=%d,%d",I,GetSubKey(I),IndexMode[I][0],IndexMode[I][1]));
}

DWORD KeyMacro::GetOpCode(MacroRecord *MR,int PC)
{
	DWORD OpCode=(MR->BufferSize > 1)?MR->Buffer[PC]:(DWORD)(DWORD_PTR)MR->Buffer;
	return OpCode;
}

// function for Mantis#0000968
bool KeyMacro::CheckWaitKeyFunc()
{
	if (InternalInput || !Work.MacroWORK || Work.Executing == MACROMODE_NOMACRO)
		return false;

	MacroRecord *MR=Work.MacroWORK;

	if (Work.ExecLIBPos >= MR->BufferSize || Work.ExecLIBPos <= 0)
		return false;

	return (GetOpCode(MR,Work.ExecLIBPos-1) == MCODE_F_WAITKEY)?true:false;
}

// ������ OpCode � �����. ���������� ���������� ��������
DWORD KeyMacro::SetOpCode(MacroRecord *MR,int PC,DWORD OpCode)
{
	DWORD OldOpCode;

	if (MR->BufferSize > 1)
	{
		OldOpCode=MR->Buffer[PC];
		MR->Buffer[PC]=OpCode;
	}
	else
	{
		OldOpCode=(DWORD)(DWORD_PTR)MR->Buffer;
		MR->Buffer=(DWORD*)(DWORD_PTR)OpCode;
	}

	return OldOpCode;
}

// ��� ��� ����� ��� ���:
// BugZ#873 - ACTL_POSTKEYSEQUENCE � ��������� ����
int KeyMacro::IsExecutingLastKey()
{
	if (Work.Executing && Work.MacroWORK)
	{
		return (Work.ExecLIBPos == Work.MacroWORK->BufferSize-1);
	}

	return FALSE;
}


void KeyMacro::SendDropProcess()
{
	if (Work.Executing)
		StopMacro=true;
}

void KeyMacro::DropProcess()
{
	if (Work.Executing)
	{
		if (LockScr) delete LockScr;

		LockScr=nullptr;
		Clipboard::SetUseInternalClipboardState(false); //??
		Work.Executing=MACROMODE_NOMACRO;
		ReleaseWORKBuffer();
	}
}

bool checkMacroConst(const wchar_t *name)
{
	return !varLook(glbConstTable, name)?false:true;
}

void initMacroVarTable(int global)
{
	if (global)
	{
		initVTable(glbVarTable);
		initVTable(glbConstTable); //???
	}
}

void doneMacroVarTable(int global)
{
	if (global)
	{
		deleteVTable(glbVarTable);
		deleteVTable(glbConstTable); //???
	}
}

BOOL KeyMacro::GetMacroParseError(DWORD* ErrCode, COORD* ErrPos, string *ErrSrc)
{
	return __getMacroParseError(ErrCode,ErrPos,ErrSrc);
}

BOOL KeyMacro::GetMacroParseError(string *Err1, string *Err2, string *Err3, string *Err4)
{
	return __getMacroParseError(Err1, Err2, Err3, Err4);
}

// ��� OpCode (�� ����������� MCODE_OP_ENDKEYS)?
bool KeyMacro::IsOpCode(DWORD p)
{
	return (!(p&KEY_MACRO_BASE) || p == MCODE_OP_ENDKEYS)?false:true;
}

static LONG _RegWriteString(const wchar_t *Key,const wchar_t *ValueName,const wchar_t *Data)
{
	LONG Ret=-1;

	if (wcschr(Data,L'\n'))
	{
		int Len=StrLength(Data)+2;
		wchar_t *ptrSrc=(wchar_t *)xf_malloc(Len*sizeof(wchar_t));

		if (ptrSrc)
		{
			wcscpy(ptrSrc,Data);

			for (int J=0; ptrSrc[J]; ++J)
				if (ptrSrc[J] == L'\n')
					ptrSrc[J]=0;

			ptrSrc[Len-1]=0;
			Ret=SetRegKey(Key,ValueName,ptrSrc,(DWORD)Len*sizeof(wchar_t),REG_MULTI_SZ);
			xf_free(ptrSrc);
		}
	}
	else
	{
		Ret=SetRegKey(Key,ValueName,Data);
	}

	return Ret;
}
