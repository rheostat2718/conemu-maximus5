#pragma once

/*
fileedit.hpp

�������������� ����� - ���������� ��� editor.cpp
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

#include "frame.hpp"
#include "editor.hpp"
#include "keybar.hpp"

class NamesList;

// ���� �������� Editor::SaveFile()
enum
{
	SAVEFILE_ERROR   = 0,         // �������� ���������, �� ����������
	SAVEFILE_SUCCESS = 1,         // ���� ������� ���������, ���� ��������� ���� �� ����
	SAVEFILE_CANCEL  = 2          // ���������� ��������, �������� �� ���������
};

// ��� ���������
enum FEOPMODEEXISTFILE
{
	FEOPMODE_QUERY        =0x00000000,
	FEOPMODE_NEWIFOPEN    =0x10000000,
	FEOPMODE_USEEXISTING  =0x20000000,
	FEOPMODE_BREAKIFOPEN  =0x30000000,
	FEOPMODE_RELOAD       =0x40000000,
};

enum FFILEEDIT_FLAGS
{
	FFILEEDIT_NEW                   = 0x00010000,  // ���� ���� ����������! ����� ��� ��� ������ �������! ���� ������ � ��� ���.
	FFILEEDIT_REDRAWTITLE           = 0x00020000,  // ����� ��������� ���������?
	FFILEEDIT_FULLSCREEN            = 0x00040000,  // ������������� �����?
	FFILEEDIT_DISABLEHISTORY        = 0x00080000,  // ��������� ������ � �������?
	FFILEEDIT_ENABLEF6              = 0x00100000,  // ������������� �� ������ �����?
	FFILEEDIT_SAVETOSAVEAS          = 0x00200000,  // $ 17.08.2001 KM  ��������� ��� ������ �� AltF7.
	//   ��� �������������� ���������� ����� �� ������ ���
	//   ������� F2 ������� ����� ShiftF2.
	FFILEEDIT_SAVEWQUESTIONS        = 0x00400000,  // ��������� ��� ��������
	FFILEEDIT_LOCKED                = 0x00800000,  // �������������?
	FFILEEDIT_OPENFAILED            = 0x01000000,  // ���� ������� �� �������
	FFILEEDIT_DELETEONCLOSE         = 0x02000000,  // ������� � ����������� ���� ������ � ��������� (���� ��� ����)
	FFILEEDIT_DELETEONLYFILEONCLOSE = 0x04000000,  // ������� � ����������� ������ ����
	FFILEEDIT_DISABLESAVEPOS        = 0x08000000,  // �� ��������� ������� ��� �����
	FFILEEDIT_CANNEWFILE            = 0x10000000,  // ����������� ����� ����?
	FFILEEDIT_SERVICEREGION         = 0x20000000,  // ������������ ��������� �������
	FFILEEDIT_CODEPAGECHANGEDBYUSER = 0x40000000,
};


class FileEditor : public Frame
{
	public:
		FileEditor(const string&  Name, UINT codepage, DWORD InitFlags,int StartLine=-1,int StartChar=-1,const string* PluginData=nullptr,int OpenModeExstFile=FEOPMODE_QUERY);
		FileEditor(const string&  Name, UINT codepage, DWORD InitFlags,int StartLine,int StartChar,const string* Title,int X1,int Y1,int X2,int Y2,int DeleteOnClose=0,int OpenModeExstFile=FEOPMODE_QUERY);
		virtual ~FileEditor();

		void ShowStatus();
		void SetLockEditor(BOOL LockMode);
		bool IsFullScreen() {return Flags.Check(FFILEEDIT_FULLSCREEN)!=FALSE;}
		void SetNamesList(NamesList *Names);
		void SetEnableF6(int AEnableF6) { Flags.Change(FFILEEDIT_ENABLEF6,AEnableF6); InitKeyBar(); }
		// ��������� ��� ������ �� AltF7. ��� �������������� ���������� ����� ��
		// ������ ��� ������� F2 ������� ����� ShiftF2.
		void SetSaveToSaveAs(int ToSaveAs) { Flags.Change(FFILEEDIT_SAVETOSAVEAS,ToSaveAs); InitKeyBar(); }
		virtual BOOL IsFileModified() const { return m_editor->IsFileModified(); };
		virtual int GetTypeAndName(string &strType, string &strName);
		int EditorControl(int Command,void *Param);
		bool SetCodePage(UINT codepage);  //BUGBUG
		BOOL IsFileChanged() const { return m_editor->IsFileChanged(); };
		virtual __int64 VMProcess(int OpCode,void *vParam=nullptr,__int64 iParam=0);
		void GetEditorOptions(EditorOptions& EdOpt);
		void SetEditorOptions(EditorOptions& EdOpt);
		void CodepageChangedByUser() {Flags.Set(FFILEEDIT_CODEPAGECHANGEDBYUSER);};
		virtual void Show();
		void SetPluginTitle(const string* PluginTitle);
		int GetId(void) const { return m_editor->EditorID; };

		static const FileEditor *CurrentEditor;

	private:
		Editor *m_editor;
		KeyBar EditKeyBar;
		NamesList *EditNamesList;
		bool F4KeyOnly;
		string strFileName;
		string strFullFileName;
		string strStartDir;
		string strTitle;
		string strPluginTitle;
		string strPluginData;
		FAR_FIND_DATA_EX FileInfo;
		wchar_t AttrStr[4];            // 13.02.2001 IS - ���� �������� ����� ���������, ����� �� ��������� �� ����� ���
		DWORD FileAttributes;          // 12.02.2001 IS - ���� �������� �������� ����� ��� ��������, ���������� ���-������...
		BOOL  FileAttributesModified;  // 04.11.2003 SKV - ���� �� ��������������� ��������� ��� save
		DWORD SysErrorCode;
		bool m_bClosing;               // 28.04.2005 AY: true ����� �������� ������������ (�.�. � �����������)
		bool bEE_READ_Sent;
		bool m_bAddSignature;
		bool BadConversion;
		UINT m_codepage; //BUGBUG

		virtual void DisplayObject();
		int  ProcessQuitKey(int FirstSave,BOOL NeedQuestion=TRUE);
		BOOL UpdateFileList();
		/* Ret:
		      0 - �� ������� ������
		      1 - ������� ���� � �������
		      2 - ������� ������ ����
		*/
		void SetDeleteOnClose(int NewMode);
		int ReProcessKey(int Key,int CalledFromControl=TRUE);
		bool AskOverwrite(const string& FileName);
		void Init(const string& Name, UINT codepage, const string* Title, DWORD InitFlags, int StartLine, int StartChar, const string* PluginData, int DeleteOnClose, int OpenModeExstFile);
		virtual void InitKeyBar();
		virtual int ProcessKey(int Key);
		virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
		virtual void ShowConsoleTitle();
		virtual void OnChangeFocus(int focus);
		virtual void SetScreenPosition();
		virtual const wchar_t *GetTypeName() {return L"[FileEdit]";};
		virtual int GetType() { return MODALTYPE_EDITOR; }
		virtual void OnDestroy();
		virtual int GetCanLoseFocus(int DynamicMode=FALSE);
		virtual int FastHide(); // ��� ���� CtrlAltShift
		// ���������� ������� ����, �������� �� ���� ���������
		// ������������ ��� �������� ������� ���������� � ������� �� CtrlF10
		BOOL isTemporary();
		virtual void ResizeConsole();
		int LoadFile(const string& Name, int &UserBreak);
		//TextFormat, Codepage � AddSignature ������������ ������, ���� bSaveAs = true!
		int SaveFile(const string& Name, int Ask, bool bSaveAs, int TextFormat = 0, UINT Codepage = CP_UNICODE, bool AddSignature=false);
		void SetTitle(const string* Title);
		virtual string &GetTitle(string &Title,int SubLen=-1,int TruncSize=0);
		BOOL SetFileName(const string& NewFileName);
		int ProcessEditorInput(INPUT_RECORD *Rec);
		void ChangeEditKeyBar();
		DWORD EditorGetFileAttributes(const string& Name);
		void SetPluginData(const string* PluginData);
		const wchar_t *GetPluginData() {return strPluginData.CPtr();}
		bool LoadFromCache(EditorPosCache &pc);
		void SaveToCache();
};

bool dlgOpenEditor(string &strFileName, UINT &codepage);
