#pragma once

/*
macroopcode.hpp

OpCode ��� ��������
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

#include "keys.hpp"

/*
  ��������!
  ��� ���������� ����...
  ... ��� �� �� �� ���������� ��������� � � syslog.cpp (������� __MCODE_ToName)

*/

enum MACRO_OP_CODE
{
	/* ************************************************************************* */
	MCODE_OP_EXIT=KEY_MACRO_OP_BASE,  // ������������� ��������� ���������� �����������������������

	MCODE_OP_JMP,                     // Jumps..
	MCODE_OP_JZ,
	MCODE_OP_JNZ,
	MCODE_OP_JLT,
	MCODE_OP_JLE,
	MCODE_OP_JGT,
	MCODE_OP_JGE,

	MCODE_OP_NOP,                     // ��� ��������

	MCODE_OP_SAVE,                    // ������������ ����������. ��� ���������� ��������� DWORD (��� � $Text).
	MCODE_OP_SAVEREPCOUNT,
	MCODE_OP_PUSHUNKNOWN,             // ������������������� �������� (���������� ��������� �������)
	MCODE_OP_PUSHINT,                 // �������� �������� �� ����. ����
	MCODE_OP_PUSHFLOAT,               // �������� �������� �� ����. double
	MCODE_OP_PUSHSTR,                 // �������� - ��������� DWORD
	MCODE_OP_PUSHVAR,                 // ��� ��������� ������� (��� � $Text)
	MCODE_OP_PUSHCONST,               // � ���� �������� ���������

	MCODE_OP_REP,                     // $rep - ������� ������ �����
	MCODE_OP_END,                     // $end - ������� ����� �����/�������

	// ����������� ��������
	// ++a, --a

	MCODE_OP_NEGATE,                  // -a
	MCODE_OP_NOT,                     // !a
	MCODE_OP_BITNOT,                  // ~a

	// ���������� ��������
	MCODE_OP_MUL,                     // a *  b
	MCODE_OP_DIV,                     // a /  b

	MCODE_OP_ADD,                     // a +  b
	MCODE_OP_SUB,                     // a -  b

	MCODE_OP_BITSHR,                  // a >> b
	MCODE_OP_BITSHL,                  // a << b

	MCODE_OP_LT,                      // a <  b
	MCODE_OP_LE,                      // a <= b
	MCODE_OP_GT,                      // a >  b
	MCODE_OP_GE,                      // a >= b

	MCODE_OP_EQ,                      // a == b
	MCODE_OP_NE,                      // a != b

	MCODE_OP_BITAND,                  // a &  b

	MCODE_OP_BITXOR,                  // a ^  b

	MCODE_OP_BITOR,                   // a |  b

	MCODE_OP_AND,                     // a && b

	MCODE_OP_XOR,                     // a ^^ b

	MCODE_OP_OR,                      // a || b

	MCODE_OP_ADDEQ,                   // a +=  b
	MCODE_OP_SUBEQ,                   // a -=  b
	MCODE_OP_MULEQ,                   // a *=  b
	MCODE_OP_DIVEQ,                   // a /=  b
	MCODE_OP_BITSHREQ,                // a >>= b
	MCODE_OP_BITSHLEQ,                // a <<= b
	MCODE_OP_BITANDEQ,                // a &=  b
	MCODE_OP_BITXOREQ,                // a ^=  b
	MCODE_OP_BITOREQ,                 // a |=  b

	// a++, a--

	MCODE_OP_DISCARD,                 // ������ �������� � ������� �����
	MCODE_OP_DUP,                     // �������������� ������� �������� � �����
	MCODE_OP_SWAP,                    // �������� ������� ��� �������� � ������� �����
	MCODE_OP_POP,                     // ��������� �������� ���������� � ������ �� ������� �����
	MCODE_OP_COPY,                    // %a=%d, ���� �� ������������

	MCODE_OP_KEYS,                    // �� ���� ����� ������� ������ ���� ������
	MCODE_OP_ENDKEYS,                 // ������ ���� �����������.

	/* ************************************************************************* */
	MCODE_OP_IF,                      // ������-�� ��� ������ � �������
	MCODE_OP_ELSE,                    // �� ������� ������� :)
	MCODE_OP_WHILE,
	/* ************************************************************************* */
	MCODE_OP_CONTINUE,                // $continue

	MCODE_OP_XLAT,
	MCODE_OP_PLAINTEXT,

	MCODE_OP_AKEY,                    // $AKey - �������, ������� ������� ������
	MCODE_OP_SELWORD,                 // $SelWord - �������� "�����"


	/* ************************************************************************* */
	// �������
	MCODE_F_NOFUNC=KEY_MACRO_F_BASE,

	MCODE_F_ABS,                      // N=abs(N)
	MCODE_F_AKEY,                     // V=akey(Mode[,Type])
	MCODE_F_ASC,                      // N=asc(S)
	MCODE_F_ATOI,                     // N=atoi(S[,radix])
	MCODE_F_CLIP,                     // V=clip(N[,V])
	MCODE_F_CHR,                      // S=chr(N)
	MCODE_F_DATE,                     // S=date([S])
	MCODE_F_DLG_GETVALUE,             // V=Dlg.GetValue(ID,N)
	MCODE_F_EDITOR_SEL,               // V=Editor.Sel(Action[,Opt])
	MCODE_F_EDITOR_SET,               // N=Editor.Set(N,Var)
	MCODE_F_EDITOR_UNDO,              // V=Editor.Undo(N)
	MCODE_F_EDITOR_POS,               // N=Editor.Pos(Op,What[,Where])
	MCODE_F_ENVIRON,                  // S=env(S)
	MCODE_F_FATTR,                    // N=fattr(S)
	MCODE_F_FEXIST,                   // S=fexist(S)
	MCODE_F_FSPLIT,                   // S=fsplit(S,N)
	MCODE_F_IIF,                      // V=iif(C,V1,V2)
	MCODE_F_INDEX,                    // S=index(S1,S2[,Mode])
	MCODE_F_INT,                      // N=int(V)
	MCODE_F_ITOA,                     // S=itoa(N[,radix])
	MCODE_F_KEY,                      // S=key(V)
	MCODE_F_LCASE,                    // S=lcase(S1)
	MCODE_F_LEN,                      // N=len(S)
	MCODE_F_MAX,                      // N=max(N1,N2)
	MCODE_F_MENU_CHECKHOTKEY,         // N=checkhotkey(S[,N])
	MCODE_F_MENU_GETHOTKEY,           // S=gethotkey([N])
	MCODE_F_MENU_SELECT,              // N=Menu.Select(S[,N[,Dir]])
	MCODE_F_MENU_SHOW,                // S=Menu.Show(Items[,Title[,Flags[,FindOrFilter[,X[,Y]]]]])
	MCODE_F_MIN,                      // N=min(N1,N2)
	MCODE_F_MOD,                      // N=mod(a,b) == a %  b
	MCODE_F_MLOAD,                    // B=mload(var)
	MCODE_F_MSAVE,                    // B=msave(var)
	MCODE_F_MSGBOX,                   // N=msgbox(["Title"[,"Text"[,flags]]])
	MCODE_F_PANEL_FATTR,              // N=Panel.FAttr(panelType,fileMask)
	MCODE_F_PANEL_SETPATH,            // N=panel.SetPath(panelType,pathName[,fileName])
	MCODE_F_PANEL_FEXIST,             // N=Panel.FExist(panelType,fileMask)
	MCODE_F_PANEL_SETPOS,             // N=Panel.SetPos(panelType,fileName)
	MCODE_F_PANEL_SETPOSIDX,          // N=Panel.SetPosIdx(panelType,Idx[,InSelection])
	MCODE_F_PANEL_SELECT,             // V=Panel.Select(panelType,Action[,Mode[,Items]])
	MCODE_F_PANELITEM,                // V=PanelItem(Panel,Index,TypeInfo)
	MCODE_F_EVAL,                     // N=eval(S[,N])
	MCODE_F_RINDEX,                   // S=rindex(S1,S2[,Mode])
	MCODE_F_SLEEP,                    // Sleep(N)
	MCODE_F_STRING,                   // S=string(V)
	MCODE_F_SUBSTR,                   // S=substr(S,start[,length])
	MCODE_F_UCASE,                    // S=ucase(S1)
	MCODE_F_WAITKEY,                  // V=waitkey([N,[T]])
	MCODE_F_XLAT,                     // S=xlat(S)
	MCODE_F_FLOCK,                    // N=FLock(N,N)
	MCODE_F_CALLPLUGIN,               // V=callplugin(SysID[,param])
	MCODE_F_REPLACE,                  // S=replace(sS,sF,sR[,Count[,Mode]])
	MCODE_F_PROMPT,                   // S=prompt(["Title"[,"Prompt"[,flags[, "Src"[, "History"]]]]])
	MCODE_F_BM_ADD,                   // N=BM.Add()  - �������� ������� ���������� � �������� �����
	MCODE_F_BM_CLEAR,                 // N=BM.Clear() - �������� ��� ��������
	MCODE_F_BM_DEL,                   // N=BM.Del([Idx]) - ������� �������� � ��������� �������� (x=1...), 0 - ������� ������� ��������
	MCODE_F_BM_GET,                   // N=BM.Get(Idx,M) - ���������� ���������� ������ (M==0) ��� ������� (M==1) �������� � �������� (Idx=1...)
	MCODE_F_BM_GOTO,                  // N=BM.Goto([n]) - ������� �� �������� � ��������� �������� (0 --> �������)
	MCODE_F_BM_NEXT,                  // N=BM.Next() - ������� �� ��������� ��������
	MCODE_F_BM_POP,                   // N=BM.Pop() - ������������ ������� ������� �� �������� � ����� ����� � ������� ��������
	MCODE_F_BM_PREV,                  // N=BM.Prev() - ������� �� ���������� ��������
	MCODE_F_BM_BACK,                  // N=BM.Back() - ������� �� ���������� �������� � ��������� ����������� ������� �������
	MCODE_F_BM_PUSH,                  // N=BM.Push() - ��������� ������� ������� � ���� �������� � ����� �����
	MCODE_F_BM_STAT,                  // N=BM.Stat([M]) - ���������� ���������� � ���������, N=0 - ������� ���������� ��������	MCODE_F_TRIM,                     // S=trim(S[,N])
	MCODE_F_TRIM,                     // S=trim(S[,N])
	MCODE_F_FLOAT,                    // N=float(V)
	MCODE_F_TESTFOLDER,               // N=testfolder(S)
	MCODE_F_PRINT,                    // N=Print(Str)
	MCODE_F_MMODE,                    // N=MMode(Action[,Value])
	MCODE_F_EDITOR_SETTITLE,          // N=Editor.SetTitle([Title])
	MCODE_F_MENU_GETVALUE,            // S=Menu.GetValue([N])
	MCODE_F_MENU_ITEMSTATUS,          // N=Menu.ItemStatus([N])
	MCODE_F_BEEP,                     // N=beep([N])
	MCODE_F_KBDLAYOUT,                // N=kbdLayout([N])
	MCODE_F_WINDOW_SCROLL,            // N=Window.Scroll(Lines[,Axis])
	MCODE_F_KEYBAR_SHOW,              // N=KeyBar.Show([N])
	MCODE_F_HISTIORY_ENABLE,          // N=History.Enable([State])
	MCODE_F_FMATCH,                   // N=FMatch(S,Mask)
	MCODE_F_PLUGIN_LOAD,              // N=Plugin.Load(DllPath[,ForceLoad])
	MCODE_F_PLUGIN_UNLOAD,            // N=Plugin.UnLoad(DllPath)
	MCODE_F_MENU_FILTER,              // N=Menu.Filter(Action[,Mode])
	MCODE_F_MENU_FILTERSTR,           // S=Menu.FilterStr([Action[,S]])

	/* ************************************************************************* */
	// ������� ���������� - ��������� ���������
	MCODE_C_AREA_OTHER=KEY_MACRO_C_BASE,// ����� ����������� ������ � ������, ������������ ����
	MCODE_C_AREA_SHELL,               // �������� ������
	MCODE_C_AREA_VIEWER,              // ���������� ��������� ���������
	MCODE_C_AREA_EDITOR,              // ��������
	MCODE_C_AREA_DIALOG,              // �������
	MCODE_C_AREA_SEARCH,              // ������� ����� � �������
	MCODE_C_AREA_DISKS,               // ���� ������ ������
	MCODE_C_AREA_MAINMENU,            // �������� ����
	MCODE_C_AREA_MENU,                // ������ ����
	MCODE_C_AREA_HELP,                // ������� ������
	MCODE_C_AREA_INFOPANEL,           // �������������� ������
	MCODE_C_AREA_QVIEWPANEL,          // ������ �������� ���������
	MCODE_C_AREA_TREEPANEL,           // ������ ������ �����
	MCODE_C_AREA_FINDFOLDER,          // ����� �����
	MCODE_C_AREA_USERMENU,            // ���� ������������
	MCODE_C_AREA_AUTOCOMPLETION,      // ������ ��������������

	MCODE_C_FULLSCREENMODE,           // ������������� �����?
	MCODE_C_ISUSERADMIN,              // Administrator status
	MCODE_C_BOF,                      // ������ �����/��������� ��������?
	MCODE_C_EOF,                      // ����� �����/��������� ��������?
	MCODE_C_EMPTY,                    // ���.������ �����?
	MCODE_C_SELECTED,                 // ���������� ���� ����?
	MCODE_C_ROOTFOLDER,               // ������ MCODE_C_APANEL_ROOT ��� �������� ������

	MCODE_C_APANEL_BOF,               // ������ ���������  ��������?
	MCODE_C_PPANEL_BOF,               // ������ ���������� ��������?
	MCODE_C_APANEL_EOF,               // ����� ���������  ��������?
	MCODE_C_PPANEL_EOF,               // ����� ���������� ��������?
	MCODE_C_APANEL_ISEMPTY,           // �������� ������:  �����?
	MCODE_C_PPANEL_ISEMPTY,           // ��������� ������: �����?
	MCODE_C_APANEL_SELECTED,          // �������� ������:  ���������� �������� ����?
	MCODE_C_PPANEL_SELECTED,          // ��������� ������: ���������� �������� ����?
	MCODE_C_APANEL_ROOT,              // ��� �������� ������� �������� ������?
	MCODE_C_PPANEL_ROOT,              // ��� �������� ������� ��������� ������?
	MCODE_C_APANEL_VISIBLE,           // �������� ������:  ������?
	MCODE_C_PPANEL_VISIBLE,           // ��������� ������: ������?
	MCODE_C_APANEL_PLUGIN,            // �������� ������:  ����������?
	MCODE_C_PPANEL_PLUGIN,            // ��������� ������: ����������?
	MCODE_C_APANEL_FILEPANEL,         // �������� ������:  ��������?
	MCODE_C_PPANEL_FILEPANEL,         // ��������� ������: ��������?
	MCODE_C_APANEL_FOLDER,            // �������� ������:  ������� ������� �������?
	MCODE_C_PPANEL_FOLDER,            // ��������� ������: ������� ������� �������?
	MCODE_C_APANEL_LEFT,              // �������� ������ �����?
	MCODE_C_PPANEL_LEFT,              // ��������� ������ �����?
	MCODE_C_APANEL_LFN,               // �� �������� ������ ������� �����?
	MCODE_C_PPANEL_LFN,               // �� ��������� ������ ������� �����?
	MCODE_C_APANEL_FILTER,            // �� �������� ������ ������� ������?
	MCODE_C_PPANEL_FILTER,            // �� ��������� ������ ������� ������?

	MCODE_C_CMDLINE_BOF,              // ������ � ������ cmd-������ ��������������?
	MCODE_C_CMDLINE_EOF,              // ������ � ����� cmd-������ ��������������?
	MCODE_C_CMDLINE_EMPTY,            // ���.������ �����?
	MCODE_C_CMDLINE_SELECTED,         // � ���.������ ���� ��������� �����?

	/* ************************************************************************* */
	// �� ������� ����������
	MCODE_V_FAR_WIDTH=KEY_MACRO_V_BASE,// Far.Width - ������ ����������� ����
	MCODE_V_FAR_HEIGHT,               // Far.Height - ������ ����������� ����
	MCODE_V_FAR_TITLE,                // Far.Title - ������� ��������� ����������� ����
	MCODE_V_FAR_UPTIME,               // Far.UpTime - ����� ������ Far � �������������
	MCODE_V_FAR_PID,                  // Far.PID - �������� �� ������� ���������� ����� Far Manager
	MCODE_V_MACROAREA,                // MacroArea - ��� ������� ������ �������

	MCODE_V_APANEL_CURRENT,           // APanel.Current - ��� ����� �� �������� ������
	MCODE_V_PPANEL_CURRENT,           // PPanel.Current - ��� ����� �� ��������� ������
	MCODE_V_APANEL_SELCOUNT,          // APanel.SelCount - �������� ������:  ����� ���������� ���������
	MCODE_V_PPANEL_SELCOUNT,          // PPanel.SelCount - ��������� ������: ����� ���������� ���������
	MCODE_V_APANEL_PATH,              // APanel.Path - �������� ������:  ���� �� ������
	MCODE_V_PPANEL_PATH,              // PPanel.Path - ��������� ������: ���� �� ������
	MCODE_V_APANEL_PATH0,             // APanel.Path0 - �������� ������:  ���� �� ������ �� ������ ��������
	MCODE_V_PPANEL_PATH0,             // PPanel.Path0 - ��������� ������: ���� �� ������ �� ������ ��������
	MCODE_V_APANEL_UNCPATH,           // APanel.UNCPath - �������� ������:  UNC-���� �� ������
	MCODE_V_PPANEL_UNCPATH,           // PPanel.UNCPath - ��������� ������: UNC-���� �� ������
	MCODE_V_APANEL_WIDTH,             // APanel.Width - �������� ������:  ������ ������
	MCODE_V_PPANEL_WIDTH,             // PPanel.Width - ��������� ������: ������ ������
	MCODE_V_APANEL_TYPE,              // APanel.Type - ��� �������� ������
	MCODE_V_PPANEL_TYPE,              // PPanel.Type - ��� ��������� ������
	MCODE_V_APANEL_ITEMCOUNT,         // APanel.ItemCount - �������� ������:  ����� ���������
	MCODE_V_PPANEL_ITEMCOUNT,         // PPanel.ItemCount - ��������� ������: ����� ���������
	MCODE_V_APANEL_CURPOS,            // APanel.CurPos - �������� ������:  ������� ������
	MCODE_V_PPANEL_CURPOS,            // PPanel.CurPos - ��������� ������: ������� ������
	MCODE_V_APANEL_OPIFLAGS,          // APanel.OPIFlags - �������� ������: ����� ��������� �������
	MCODE_V_PPANEL_OPIFLAGS,          // PPanel.OPIFlags - ��������� ������: ����� ��������� �������
	MCODE_V_APANEL_DRIVETYPE,         // APanel.DriveType - �������� ������: ��� �������
	MCODE_V_PPANEL_DRIVETYPE,         // PPanel.DriveType - ��������� ������: ��� �������
	MCODE_V_APANEL_HEIGHT,            // APanel.Height - �������� ������:  ������ ������
	MCODE_V_PPANEL_HEIGHT,            // PPanel.Height - ��������� ������: ������ ������
	MCODE_V_APANEL_STATUSHEIGHT,      // APanel.StatusHeight - �������� ������:  ������ ��������� ������� ������
	MCODE_V_PPANEL_STATUSHEIGHT,      // PPanel.StatusHeight - ��������� ������: ������ ��������� ������� ������
	MCODE_V_APANEL_COLUMNCOUNT,       // APanel.ColumnCount - �������� ������:  ���������� �������
	MCODE_V_PPANEL_COLUMNCOUNT,       // PPanel.ColumnCount - ��������� ������: ���������� �������
	MCODE_V_APANEL_HOSTFILE,          // APanel.HostFile - �������� ������:  ��� Host-�����
	MCODE_V_PPANEL_HOSTFILE,          // PPanel.HostFile - ��������� ������: ��� Host-�����
	MCODE_V_APANEL_PREFIX,            // APanel.Prefix
	MCODE_V_PPANEL_PREFIX,            // PPanel.Prefix
	MCODE_V_APANEL_FORMAT,            // APanel.Format
	MCODE_V_PPANEL_FORMAT,            // PPanel.Format

	MCODE_V_ITEMCOUNT,                // ItemCount - ����� ��������� � ������� �������
	MCODE_V_CURPOS,                   // CurPos - ������� ������ � ������� �������
	MCODE_V_TITLE,                    // Title - ��������� �������� �������
	MCODE_V_HEIGHT,                   // Height - ������ �������� �������
	MCODE_V_WIDTH,                    // Width - ������ �������� �������

	MCODE_V_EDITORFILENAME,           // Editor.FileName - ��� �������������� �����
	MCODE_V_EDITORLINES,              // Editor.Lines - ���������� ����� � ���������
	MCODE_V_EDITORCURLINE,            // Editor.CurLine - ������� ����� � ��������� (� ���������� � Count)
	MCODE_V_EDITORCURPOS,             // Editor.CurPos - ������� ���. � ���������
	MCODE_V_EDITORREALPOS,            // Editor.RealPos - ������� ���. � ��������� ��� �������� � ������� ���������
	MCODE_V_EDITORSTATE,              // Editor.State
	MCODE_V_EDITORVALUE,              // Editor.Value - ���������� ������� ������
	MCODE_V_EDITORSELVALUE,           // Editor.SelValue - �������� ���������� ����������� �����

	MCODE_V_DLGITEMTYPE,              // Dlg.ItemType
	MCODE_V_DLGITEMCOUNT,             // Dlg.ItemCount
	MCODE_V_DLGCURPOS,                // Dlg.CurPos
	MCODE_V_DLGINFOID,                // Dlg.Info.Id
	MCODE_V_DLGINFOOWNER,             // Dlg.Info.Owner

	MCODE_V_VIEWERFILENAME,           // Viewer.FileName - ��� ���������������� �����
	MCODE_V_VIEWERSTATE,              // Viewer.State

	MCODE_V_CMDLINE_ITEMCOUNT,        // CmdLine.ItemCount
	MCODE_V_CMDLINE_CURPOS,           // CmdLine.CurPos
	MCODE_V_CMDLINE_VALUE,            // CmdLine.Value

	MCODE_V_DRVSHOWPOS,               // Drv.ShowPos - ���� ������ ������ ����������: 1=����� (Alt-F1), 2=������ (Alt-F2), 0="���� ���"
	MCODE_V_DRVSHOWMODE,              // Drv.ShowMode - ������ ����������� ���� ������ ������

	MCODE_V_HELPFILENAME,             // Help.FileName
	MCODE_V_HELPTOPIC,                // Help.Topic
	MCODE_V_HELPSELTOPIC,             // Help.SelTopic

	MCODE_V_MENU_VALUE,               // Menu.Value
	MCODE_V_MENUINFOID,               // Menu.Info.Id
};

typedef enum MACRO_OP_CODE TMacroOpCode;
