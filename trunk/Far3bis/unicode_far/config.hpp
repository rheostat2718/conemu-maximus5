#pragma once

/*
config.hpp

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

#include "panelctype.hpp"

//  +CASR_* ��������� Ctrl-Alt-Shift ��� AllCtrlAltShiftRule
enum
{
	CASR_PANEL  = 0x0001,
	CASR_EDITOR = 0x0002,
	CASR_VIEWER = 0x0004,
	CASR_HELP   = 0x0008,
	CASR_DIALOG = 0x0010,
};

enum ExcludeCmdHistoryType
{
	EXCLUDECMDHISTORY_NOTWINASS    = 0x00000001,  // �� �������� � ������� ������� ���������� Windows
	EXCLUDECMDHISTORY_NOTFARASS    = 0x00000002,  // �� �������� � ������� ������� ���������� ���������� ������
	EXCLUDECMDHISTORY_NOTPANEL     = 0x00000004,  // �� �������� � ������� ������� ���������� � ������
	EXCLUDECMDHISTORY_NOTCMDLINE   = 0x00000008,  // �� �������� � ������� ������� ���������� � ���.������
	//EXCLUDECMDHISTORY_NOTAPPLYCMD   = 0x00000010,  // �� �������� � ������� ������� ���������� �� "Apply Commang"
};

// ��� Opt.QuotedName
enum QUOTEDNAMETYPE
{
	QUOTEDNAME_INSERT         = 0x00000001,            // �������� ��� ������ � ��������� ������, � �������� � ���������
	QUOTEDNAME_CLIPBOARD      = 0x00000002,            // �������� ��� ��������� � ����� ������
};

//��� Opt.Dialogs.MouseButton
#define DMOUSEBUTTON_LEFT   0x00000001
#define DMOUSEBUTTON_RIGHT  0x00000002

//��� Opt.VMenu.xBtnClick
#define VMENUCLICK_IGNORE 0
#define VMENUCLICK_CANCEL 1
#define VMENUCLICK_APPLY  2

//��� Opt.Diz.UpdateMode
enum DIZUPDATETYPE
{
	DIZ_NOT_UPDATE,
	DIZ_UPDATE_IF_DISPLAYED,
	DIZ_UPDATE_ALWAYS
};

enum FarPoliciesFlags
{
	FFPOL_MAINMENUSYSTEM        = 0x00000001,
	FFPOL_MAINMENUPANEL         = 0x00000002,
	FFPOL_MAINMENUINTERFACE     = 0x00000004,
	FFPOL_MAINMENULANGUAGE      = 0x00000008,
	FFPOL_MAINMENUPLUGINS       = 0x00000010,
	FFPOL_MAINMENUDIALOGS       = 0x00000020,
	FFPOL_MAINMENUCONFIRMATIONS = 0x00000040,
	FFPOL_MAINMENUPANELMODE     = 0x00000080,
	FFPOL_MAINMENUFILEDESCR     = 0x00000100,
	FFPOL_MAINMENUFOLDERDESCR   = 0x00000200,
	FFPOL_MAINMENUVIEWER        = 0x00000800,
	FFPOL_MAINMENUEDITOR        = 0x00001000,
	FFPOL_MAINMENUCOLORS        = 0x00004000,
	FFPOL_MAINMENUHILIGHT       = 0x00008000,
	FFPOL_MAINMENUSAVEPARAMS    = 0x00020000,

	FFPOL_CREATEMACRO           = 0x00040000,
	FFPOL_USEPSWITCH            = 0x00080000,
	FFPOL_PERSONALPATH          = 0x00100000,
	FFPOL_KILLTASK              = 0x00200000,
	FFPOL_SHOWHIDDENDRIVES      = 0x80000000,
};

struct PanelOptions
{
	int Type;
	int Visible;
	int Focus;
	int ViewMode;
	int SortMode;
	int SortOrder;
	int SortGroups;
	int ShowShortNames;
	int NumericSort;
	int CaseSensitiveSort;
	int DirectoriesFirst;
};

struct AutoCompleteOptions
{
	int ShowList;
	int ModalList;
	int AppendCompletion;
};


struct PluginConfirmation
{
	int OpenFilePlugin;
	int StandardAssociation;
	int EvenIfOnlyOnePlugin;
	int SetFindList;
	int Prefix;
};

struct Confirmation
{
	int Copy;
	int Move;
	int RO;
	int Drag;
	int Delete;
	int DeleteFolder;
	int Exit;
	int Esc;  // ��� CheckForEsc
	/* $ 12.03.2002 VVM
	  + Opt.EscTwiceToInterrupt
	    ���������� ��������� ��� ���������� ���������� ��������
	    0 - ������ ESC ���������� ��������
	    1 - ������ ESC ��������� �������� */
	int EscTwiceToInterrupt;
	int RemoveConnection;
	/* $ 23.05.2001
	  +  Opt.Confirmation.AllowReedit - ����, ������� �������� ��������� ��������
	    ����� �� �������������� ����, ������ ���� ��� �������������. �� ��������� - 1
	    0 - ���� ��� �������� ���� �� ��� �������, �� ���������� ������� � ��������� ���������
	        ��� �������������� ��������. ���� ���� ��� �������, �� �������� ������, � � ������
	        ���� ������� ������� Reload, �� ����������� ����� ����� �����, ��� ���� ���������
	        ��������� ��������.
	    1 - ��� ��� ���� ������. �������� ������ � ���������� ������� ���� ��� � ��������� �����
	        ���� ����������� ����� ������ ���������.
	    */
	int AllowReedit;
	int HistoryClear;
	int RemoveSUBST;
	int RemoveHotPlug;
	int DetachVHD;
};

struct DizOptions
{
	string strListNames;
	int ROUpdate;
	int UpdateMode;
	int SetHidden;
	int StartPos;
	int AnsiByDefault;
	int SaveInUTF;
};

struct CodeXLAT
{
	HKL Layouts[10];
	string Rules[3]; // �������:
	// [0] "���� ���������� ������ ���������"
	// [1] "���� ���������� ������ ����������� ������"
	// [2] "���� ���������� ������ �� ���/lat"
	string Table[2]; // [0] non-english �����, [1] english �����
	string strWordDivForXlat;
	DWORD Flags;       // �������������� �����

	/* $ 25.11.2000 IS
	   �������������� ���� �� ������� ��� ������� Xlat
	*/
	int CurrentLayout;
};

struct EditorOptions
{
	int TabSize;
	int ExpandTabs;
	int PersistentBlocks;
	int DelRemovesBlocks;
	int AutoIndent;
	int AutoDetectCodePage;
	int AnsiCodePageForNewFile;
	int AnsiCodePageAsDefault;
	int CursorBeyondEOL;
	int BSLikeDel;
	int CharCodeBase;
	int SavePos;
	int SaveShortPos;
	int F7Rules; // $ 28.11.2000 SVS - ������� �� ���� ������ � ���������
	int AllowEmptySpaceAfterEof; // $ 21.06.2005 SKV - ��������� ���������� ������ ������������ ����� ��������� ������ �������������� �����.
	int ReadOnlyLock; // $ 29.11.2000 SVS - ������ ���� ��� �������� � ���������, ���� �� ����� �������� R|S|H
	int UndoSize; // $ 03.12.2001 IS - ������ ������ undo � ���������
	int UseExternalEditor;
	DWORD FileSizeLimitLo;
	DWORD FileSizeLimitHi;
	int ShowKeyBar;
	int ShowTitleBar;
	int ShowScrollBar;
	int EditOpenedForWrite;
	int SearchSelFound;
	int SearchRegexp;
	int SearchPickUpWord;
	int ShowWhiteSpace;

	string strWordDiv;

	void Clear()
	{
		TabSize = 0;
		ExpandTabs = 0;
		PersistentBlocks = 0;
		DelRemovesBlocks = 0;
		AutoIndent = 0;
		AutoDetectCodePage = 0;
		AnsiCodePageForNewFile = 0;
		AnsiCodePageAsDefault = 0;
		CursorBeyondEOL = 0;
		BSLikeDel = 0;
		CharCodeBase = 0;
		SavePos = 0;
		SaveShortPos = 0;
		F7Rules = 0;
		AllowEmptySpaceAfterEof = 0;
		ReadOnlyLock = 0;
		UndoSize = 0;
		UseExternalEditor = 0;
		ShowKeyBar = 0;
		ShowTitleBar = 0;
		ShowScrollBar=0;
		EditOpenedForWrite=0;
		SearchSelFound=0;
		SearchRegexp=0;
		SearchPickUpWord=0;
		ShowWhiteSpace=0;
		FileSizeLimitLo = 0;
		FileSizeLimitHi = 0;
		strWordDiv.Clear();
	}

	void CopyTo(EditorOptions &dest)
	{
		dest.TabSize = TabSize;
		dest.ExpandTabs = ExpandTabs;
		dest.PersistentBlocks = PersistentBlocks;
		dest.DelRemovesBlocks = DelRemovesBlocks;
		dest.AutoIndent = AutoIndent;
		dest.AutoDetectCodePage = AutoDetectCodePage;
		dest.AnsiCodePageForNewFile = AnsiCodePageForNewFile;
		dest.AnsiCodePageAsDefault = AnsiCodePageAsDefault;
		dest.CursorBeyondEOL = CursorBeyondEOL;
		dest.BSLikeDel = BSLikeDel;
		dest.CharCodeBase = CharCodeBase;
		dest.SavePos = SavePos;
		dest.SaveShortPos = SaveShortPos;
		dest.F7Rules = F7Rules;
		dest.AllowEmptySpaceAfterEof = AllowEmptySpaceAfterEof;
		dest.ReadOnlyLock = ReadOnlyLock;
		dest.UndoSize = UndoSize;
		dest.UseExternalEditor = UseExternalEditor;
		dest.ShowKeyBar = ShowKeyBar;
		dest.ShowTitleBar = ShowTitleBar;
		dest.strWordDiv = strWordDiv;
		dest.ShowScrollBar=ShowScrollBar;
		dest.SearchSelFound=SearchSelFound;
		dest.SearchRegexp=SearchRegexp;
		dest.ShowWhiteSpace=ShowWhiteSpace;
		dest.SearchPickUpWord=SearchPickUpWord;
		dest.FileSizeLimitLo = FileSizeLimitLo;
		dest.FileSizeLimitHi = FileSizeLimitHi;
	}
};

/* $ 29.03.2001 IS
     ��� ������� ������� "���������" ��������� ��� ��������� ���������
*/
struct ViewerOptions
{
	enum EViewerLineSize
	{
		eMinLineSize = 100,
		eDefLineSize = 10*1000,
		eMaxLineSize = 100*1000
	};

	int TabSize;
	int AutoDetectCodePage;
	int ShowScrollbar;     // $ 18.07.2000 tran ���� �������� ��� viewer
	int ShowArrows;
	int PersistentBlocks; // $ 14.05.2002 VVM ���������� ����� �� ������
	int ViewerIsWrap; // (Wrap|WordWarp)=1 | UnWrap=0
	int ViewerWrap; // Wrap=0|WordWarp=1
	int SavePos;
	int SaveShortPos;
	int UseExternalViewer;
	int ShowKeyBar; // $ 15.07.2000 tran + ShowKeyBar
	int AnsiCodePageAsDefault;
	int ShowTitleBar;
	int SearchRegexp;
	int MaxLineSize; // 100..100000, default=10000
	int SearchEditFocus; // auto-focus on edit text/hex window
	int Visible0x00;
	int ZeroChar;
};

// "�������"
struct PoliciesOptions
{
	int DisabledOptions;  // ������������� ���� ������������
	int ShowHiddenDrives; // ���������� ������� ���������� �����
};

struct DialogsOptions
{
	int   EditBlock;            // ���������� ����� � ������� �����
	int   EditHistory;          // ��������� � �������?
	int   AutoComplete;         // ��������� ��������������?
	int   EULBsClear;           // = 1 - BS � �������� ��� UnChanged ������ ������� ����� ������ �����, ��� � Del
	int   SelectFromHistory;    // = 0 then (ctrl-down � ������ � �������� ������ �������������� �� ����� ������� ������)
	DWORD EditLine;             // ����� ���������� � ������ ����� (������ ��� ����... ��������� ��������� ����������)
	int   MouseButton;          // ���������� ���������� ������/����� ������ ���� ��� ������ �������� ���� �������
	int   DelRemovesBlocks;
	int   CBoxMaxHeight;        // ������������ ������ ������������ ������ (�� ���������=8)
};

struct VMenuOptions
{
	int   LBtnClick;
	int   RBtnClick;
	int   MBtnClick;
};

struct CommandLineOptions
{
	int EditBlock;
	int DelRemovesBlocks;
	int AutoComplete;
	int UsePromptFormat;
	string strPromptFormat;
};

struct NowellOptions
{
	int MoveRO;               // ����� ��������� Move ������� R/S/H ��������, ����� �������� - ���������� �������
};

struct ScreenSizes
{
	COORD DeltaXY;            // �� ������� ���. �������� ������� ��� ������������ ������
	int WScreenSizeSet;
	COORD WScreenSize[4];
};

struct LoadPluginsOptions
{
	string strCustomPluginsPath;  // ���� ��� ������ ��������, ��������� � /p
	string strPersonalPluginsPath;
//  DWORD TypeLoadPlugins;       // see TYPELOADPLUGINSOPTIONS
	int MainPluginDir; // TRUE - ������������ ����������� ���� � �������� ��������
	int PluginsCacheOnly; // seting by '/co' switch, not saved in registry
	int PluginsPersonal;

	int SilentLoadPlugin; // ��� �������� ������� � ������...
#ifndef NO_WRAPPER
	int OEMPluginsSupport;
#endif // NO_WRAPPER
	int ScanSymlinks;
};

struct FindFileOptions
{
	int FileSearchMode;
	bool FindFolders;
	bool FindSymLinks;
	bool CollectFiles;
	bool UseFilter;
	bool FindAlternateStreams;
	string strSearchInFirstSize;

	string strSearchOutFormat;
	string strSearchOutFormatWidth;
	int OutColumnCount;
	unsigned __int64 OutColumnTypes[PANEL_COLUMNCOUNT];
	int OutColumnWidths[PANEL_COLUMNCOUNT];
	int OutColumnWidthType[PANEL_COLUMNCOUNT];
};

struct InfoPanelOptions
{
	COMPUTER_NAME_FORMAT ComputerNameFormat;
	EXTENDED_NAME_FORMAT UserNameFormat;
	int ShowPowerStatus;
	string strShowStatusInfo;
	string strFolderInfoFiles;
};

struct TreeOptions
{
	int LocalDisk;         // ������� ���� ��������� ����� ��� ��������� ������
	int NetDisk;           // ������� ���� ��������� ����� ��� ������� ������
	int NetPath;           // ������� ���� ��������� ����� ��� ������� �����
	int RemovableDisk;     // ������� ���� ��������� ����� ��� ������� ������
	int MinTreeCount;      // ����������� ���������� ����� ��� ���������� ������ � �����.
	int AutoChangeFolder;  // ��������� ����� ��� ����������� �� ������
	DWORD TreeFileAttr;    // �������� �������� ��� ������-�������
};

struct CopyMoveOptions
{
	int UseSystemCopy;         // ������������ ��������� ������� �����������
	int CopyOpened;            // ���������� �������� �� ������ �����
	int CopyShowTotal;         // �������� ����� ��������� �����������
	int MultiCopy;             // "��������� �����������������/�����������/�������� ������"
	DWORD CopySecurityOptions; // ��� �������� Move - ��� ������ � ������ "Copy access rights"
	int CopyTimeRule;          // $ 30.01.2001 VVM  ���������� ����� �����������,���������� ����� � ������� ��������
};

struct DeleteOptions
{
	int DelShowTotal;         // �������� ����� ��������� ��������
};

struct MacroOptions
{
	int MacroReuseRules; // ������� �� ���� �������� ������������� ����������� ������
	DWORD DisableMacro; // ��������� /m ��� /ma ��� /m....
	DWORD KeyMacroCtrlDot, KeyMacroRCtrlDot; // ��� KEY_CTRLDOT/KEY_RCTRLDOT
	DWORD KeyMacroCtrlShiftDot, KeyMacroRCtrlShiftDot; // ��� KEY_CTRLSHIFTDOT/KEY_RCTRLSHIFTDOT
	int CallPluginRules; // 0 - ����������� ������� ��� ������ �������, 1 - ��������� ������� (������!)
	string strMacroCONVFMT; // ������ �������������� double � ������
	string strDateFormat; // ��� $Date
};

struct Options
{
	palette Palette;
	int Clock;
	int Mouse;
	int ShowKeyBar;
	int ScreenSaver;
	int ScreenSaverTime;
	int UseVk_oem_x;
	int ShowHidden;
	int Highlight;

	string strLeftFolder;
	string strRightFolder;

	string strLeftCurFile;
	string strRightCurFile;

	int RightSelectedFirst;
	int LeftSelectedFirst;
	int SelectFolders;
	int ReverseSort;
	int ClearReadOnly;
	int SortFolderExt;
	int DeleteToRecycleBin;         // ������� � �������?
	int DeleteToRecycleBinKillLink; // ����� ��������� ����� � ������� ������� ��������� ��������.
	int WipeSymbol; // ������ ����������� ��� "ZAP-��������"

	CopyMoveOptions CMOpt;

	DeleteOptions DelOpt;

	int MultiMakeDir; // ����� �������� ���������� ��������� �� ���� �����

	int CreateUppercaseFolders;
	int UseRegisteredTypes;

	int ViewerEditorClock;
	int OnlyEditorViewerUsed; // =1, ���� ����� ��� /e ��� /v
	int SaveViewHistory;
	int ViewHistoryCount;

	string strExternalEditor;
	EditorOptions EdOpt;
	string strExternalViewer;
	ViewerOptions ViOpt;


	string strWordDiv; // $ 03.08.2000 SVS �������������� ���� �� �������
	string strQuotedSymbols;
	DWORD QuotedName;
	int AutoSaveSetup;
	int SetupArgv; // ���������� ��������� � ���������� ����
	int ChangeDriveMode;
	int ChangeDriveDisconnetMode;
	int ChangePlugMenuMode;

	int SaveHistory;
	int HistoryCount;
	int SaveFoldersHistory;
	int FoldersHistoryCount;
	int DialogsHistoryCount;

	FindFileOptions FindOpt;

	int LeftHeightDecrement;
	int RightHeightDecrement;
	int WidthDecrement;

	int ShowColumnTitles;
	int ShowPanelStatus;
	int ShowPanelTotals;
	int ShowPanelFree;
	int HighlightColumnSeparator;
	int DoubleGlobalColumnSeparator;

	int ShowPanelScrollbar;
	int ShowMenuScrollbar; // $ 29.06.2000 SVS �������� ������� ������ Scroll Bar � ����.
	int ShowScreensNumber;
	int ShowSortMode;
	int ShowMenuBar;
	int FormatNumberSeparators;
	int CleanAscii;
	int NoGraphics;

	Confirmation Confirm;
	PluginConfirmation PluginConfirm;

	DizOptions Diz;

	int ShellRightLeftArrowsRule;
	PanelOptions LeftPanel;
	PanelOptions RightPanel;

	AutoCompleteOptions AutoComplete;

	DWORD  AutoUpdateLimit; // ���� ����� ���������� ������������� �� ��������� ������.
	int AutoUpdateRemoteDrive;

	string strLanguage;
	int SmallIcon;
#ifndef NO_WRAPPER
	string strRegRoot;
#endif // NO_WRAPPER
	int PanelRightClickRule; // ������ ��������� ������ ������� ����
	int PanelCtrlAltShiftRule; // ������ ��������� Ctrl-Alt-Shift ��� �������.
	// Panel/CtrlFRule � ������� - ������ ��������� Ctrl-F. ���� = 0, �� ���������� ���� ��� ����, ����� - � ������ ����������� �� ������
	int PanelCtrlFRule;
	/*
	  ������� �����, ������ ��������� Ctrl-Alt-Shift
	   ��� ���������� - ������� ��������:
	   0 - Panel
	   1 - Edit
	   2 - View
	   3 - Help
	   4 - Dialog
	*/
	int AllCtrlAltShiftRule;

	int CASRule; // 18.12.2003 - ������� ��������� ����� � ������ CAS (������� #1).
	/*
	  ������ ��������� Esc ��� ��������� ������:
	    =1 - �� �������� ��������� � History, ���� ����� Ctrl-E/Ctrl/-X
	         ������ ESC (��������� - ��� VC).
	    =0 - ��������� ��� � ���� - �������� ��������� � History
	*/
	int CmdHistoryRule;

	DWORD ExcludeCmdHistory;
	int SubstPluginPrefix; // 1 = ��������������� ������� ������� (��� Ctrl-[ � ��� ��������)
	int MaxPositionCache; // ���������� ������� � ���� ����������
	int SetAttrFolderRules; // ������� �� ���� ��������� ��������� �� ��������
	int ExceptRules; // ������� �� ���� ������ ����������
	int ExceptCallDebugger; // �������� �������� ��� ����������
	/*
	 + Opt.ShiftsKeyRules - ������� �� ���� ������ ��������� ����������
	   Alt-����� ��� ����������� ������� � �������� "`-=[]\;',./" �
	   �������������� Alt-, Ctrl-, Alt-Shift-, Ctrl-Shift-, Ctrl-Alt-
	*/
	int ShiftsKeyRules;
	int CursorSize[4];   // ������ ������� ����

	CodeXLAT XLat;

	int ConsoleDetachKey; // ���������� ������ ��� ������ Far'������ ������� �� ����������� ���������������� �������� � ��� �����������.

	string strHelpLanguage;
	int FullScreenHelp;
	int HelpTabSize;

	int HelpURLRules; // =0 ��������� ����������� ������� URL-����������

	// ���������� ���������� ����� � �� ���������� ������ ���. ��� �������������� "����������" "�������" ������.
	int RememberLogicalDrives;
	/*
	  ����� ������ ��:
	      ���������� ������ � ������� � ������ ���������
	      ���������� LastPositions � ��������� � ������
	*/
	int FlagPosixSemantics;

	int MsWheelDelta; // ������ �������� ��� ���������
	int MsWheelDeltaView;
	int MsWheelDeltaEdit;
	int MsWheelDeltaHelp;
	// �������������� ���������
	int MsHWheelDelta;
	int MsHWheelDeltaView;
	int MsHWheelDeltaEdit;

	/* $ 28.04.2001 VVM
	  + Opt.SubstNameRule ������� �����:
	    0 - ���� ����������, �� ���������� ������� ����� ��� GetSubstName()
	    1 - ���� ����������, �� ���������� ��� ��������� ��� GetSubstName() */
	int SubstNameRule;

	/* $ 23.05.2001 AltF9
	  + Opt.AltF9 ���� ��������� ������� ��������  ������ ���������� Alt-F9
	       (��������� ������� ������) � ������� ������. �� ��������� - 1.
	    0 - ������������ ��������, ����������� � FAR ������ 1.70 beta 3 �
	       ����, �.�. ������������ 25/50 �����.
	    1 - ������������ ������������������� �������� - ���� FAR Manager
	       ����� ������������� � ����������� �� ����������� ��������� ������
	       ����������� ���� � �������.*/
	int AltF9;

	int ClearType;

	int PgUpChangeDisk;
	int ShowCheckingFile;
	int CloseConsoleRule;
	int CloseCDGate;       // ���������������� CD
	int UpdateEnvironment;

	int RestoreCPAfterExecute;
	int ExecuteUseAppPath;
	int ExecuteFullTitle;
	int ExecuteSilentExternal;
	string strExecuteBatchType;

	DWORD PluginMaxReadData;
	int ScanJunction;

	DWORD ShowTimeoutDelFiles; // ������ � �������� �������� (� ms)
	DWORD ShowTimeoutDACLFiles;
	int DelThreadPriority; // ��������� �������� ��������, �� ��������� = THREAD_PRIORITY_NORMAL

	LoadPluginsOptions LoadPlug;

	DialogsOptions Dialogs;
	VMenuOptions VMenu;
	CommandLineOptions CmdLine;
	PoliciesOptions Policies;
	NowellOptions Nowell;
	ScreenSizes ScrSize;
	MacroOptions Macro;

	int FindCodePage;

	TreeOptions Tree;
	InfoPanelOptions InfoPanel;

	DWORD CPMenuMode;
	string strNoAutoDetectCP;
	// ������������� ����� ������� �������� ����� ��������� �� �������������� nsUniversalDetectorEx.
	// ���������� ��������� ������� �� ����� �� �������, ������� UTF-8 ����� ������������ ���� ����
	// 65001 ����� ������������. ���� UniversalDetector ������ �������� �� ����� ������, ��� �����
	// �������� �� ������������� ANSI ��� OEM, � ����������� �� ��������.
	// ������: L"1250,1252,1253,1255,855,10005,28592,28595,28597,28598,38598,65001"
	// ���� ������ ������ ������� ���������� ������� ������� � UCD ������� �� �����.
	// ���� "-1", �� � ����������� CPMenuMode (Ctrl-H � ���� ������� ������� ���������� UCD ���� �����
	// ���������, ���� ����� ���������� ������ '�������' � ��������� (OEM ANSI) ������� ��������.

	bool IsUserAdmin;
	string strTitleAddons;

	int ElevationMode;
	int CurrentElevationMode;

	BOOL WindowMode;

	string ProfilePath;
	string LocalProfilePath;
	string GlobalUserMenuDir;
};

extern Options Opt;

void SystemSettings();
void PanelSettings();
void InterfaceSettings();
void DialogSettings();
void VMenuSettings();
void CmdlineSettings();
void SetConfirmations();
void PluginsManagerSettings();
void SetDizConfig();
void ViewerConfig(ViewerOptions &ViOpt,bool Local=false);
void EditorConfig(EditorOptions &EdOpt,bool Local=false);
void ReadConfig();
void SaveConfig(int Ask);
void SetFolderInfoFiles();
void InfoPanelSettings();
void AutoCompleteSettings();
