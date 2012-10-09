#pragma once

/*
help.hpp

������
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
#include "keybar.hpp"
#include "array.hpp"

class CallBackStack;

#define HelpBeginLink L'<'
#define HelpEndLink L'>'
#define HelpFormatLink L"<%s\\>%s"

#define HELPMODE_CLICKOUTSIDE  0x20000000 // ���� ������� ���� ��� �����?

struct StackHelpData
{
	UINT64 Flags;                  // �����
	int   TopStr;                 // ����� ������� ������� ������ ����
	int   CurX,CurY;              // ���������� (???)

	string strHelpMask;           // �������� �����
	string strHelpPath;           // ���� � ������
	string strHelpTopic;         // ������� �����
	string strSelTopic;          // ���������� ����� (???)

	void Clear()
	{
		Flags=0;
		TopStr=0;
		CurX=CurY=0;
		strHelpMask.Clear();
		strHelpPath.Clear();
		strHelpTopic.Clear();
		strSelTopic.Clear();
	}
};

enum HELPDOCUMENTSHELPTYPE
{
	HIDX_PLUGINS,                 // ������ ��������
	HIDX_DOCUMS,                  // ������ ����������
};

enum
{
	FHELPOBJ_ERRCANNOTOPENHELP  = 0x80000000,
};

class HelpRecord
{
	public:
		wchar_t *HelpStr;

		HelpRecord(const wchar_t *HStr=nullptr)
		{
			HelpStr = nullptr;
			if (HStr )
				HelpStr = xf_wcsdup(HStr);
		};

		HelpRecord& operator=(const HelpRecord &rhs)
		{
			if (this != &rhs)
			{
				HelpStr = xf_wcsdup(rhs.HelpStr);
			}

			return *this;
		};

		bool operator==(const HelpRecord &rhs) const
		{
			return !StrCmpI(HelpStr,rhs.HelpStr);
		};

		int operator<(const HelpRecord &rhs) const
		{
			return StrCmpI(HelpStr,rhs.HelpStr) < 0;
		};

		~HelpRecord()
		{
			if (HelpStr) xf_free(HelpStr);
		}
};

class Help:public Frame
{
	private:
		BOOL  ErrorHelp;            // TRUE - ������! �������� - ��� ������ ������
		SaveScreen *TopScreen;      // ������� ���������� ��� ������
		KeyBar      HelpKeyBar;     // ������
		CallBackStack *Stack;       // ���� ��������
		string  strFullHelpPathName;

		StackHelpData StackData;
		TArray<HelpRecord> HelpList; // "����" � ������.

		int   StrCount;             // ���������� ����� � ����
		int   FixCount;             // ���������� ����� ���������������� �������
		int   FixSize;              // ������ ���������������� �������
		int   TopicFound;           // TRUE - ����� ������
		int   IsNewTopic;           // ��� ����� �����?

		int   MouseDown;
		int   MouseDownX, MouseDownY, BeforeMouseDownX, BeforeMouseDownY;
		int   MsX, MsY;

		string strCtrlColorChar;    // CtrlColorChar - �����! ��� �����������-
		//   ������� - ��� ���������
		int   CurColor;             // CurColor - ������� ���� ���������
		int   CtrlTabSize;          // CtrlTabSize - �����! ������ ���������

		int   PrevMacroMode;        // ���������� ����� �������

		string strCurPluginContents; // ������ PluginContents (��� ����������� � ���������)

		DWORD LastStartPos;
		DWORD StartPos;

		string strCtrlStartPosChar;

	private:
		virtual void DisplayObject();
		int  ReadHelp(const wchar_t *Mask=nullptr);
		void AddLine(const wchar_t *Line);
		void AddTitle(const wchar_t *Title);
		void HighlightsCorrection(string &strStr);
		void FastShow();
		void DrawWindowFrame();
		void OutString(const wchar_t *Str);
		int  StringLen(const wchar_t *Str);
		void CorrectPosition();
		int  IsReferencePresent();
		bool GetTopic(int realX, int realY, string& strTopic);
		void MoveToReference(int Forward,int CurScreen);
		void ReadDocumentsHelp(int TypeIndex);
		int  JumpTopic(const wchar_t *JumpTopic=nullptr);
		const HelpRecord* GetHelpItem(int Pos);

	public:
		Help(const wchar_t *Topic,const wchar_t *Mask=nullptr,UINT64 Flags=0);
		virtual ~Help();

	public:
		virtual void Hide();
		virtual int  ProcessKey(int Key);
		virtual int  ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
		virtual void InitKeyBar();
		BOOL GetError() {return ErrorHelp;}
		virtual void SetScreenPosition();
		virtual void OnChangeFocus(int focus); // ���������� ��� ����� ������
		virtual void ResizeConsole();

		virtual int  FastHide(); // ������� ��� ���� CtrlAltShift

		virtual const wchar_t *GetTypeName() {return L"[Help]";}
		virtual int GetTypeAndName(string &strType, string &strName);
		virtual int GetType() { return MODALTYPE_HELP; }

		virtual __int64 VMProcess(int OpCode,void *vParam,__int64 iParam);

		static string &MkTopic(class Plugin* pPlugin,const wchar_t *HelpTopic,string &strTopic);
};
