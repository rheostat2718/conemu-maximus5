/*
edit.cpp

���������� ��������� ������ ��������������
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

#include "edit.hpp"
#include "keyboard.hpp"
#include "macroopcode.hpp"
#include "keys.hpp"
#include "editor.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "filelist.hpp"
#include "panel.hpp"
#include "scrbuf.hpp"
#include "interf.hpp"
#include "palette.hpp"
#include "clipboard.hpp"
#include "xlat.hpp"
#include "datetime.hpp"
#include "shortcuts.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "panelmix.hpp"
#include "RegExp.hpp"
#include "history.hpp"
#include "vmenu.hpp"
#include "chgmmode.hpp"
#include "colormix.hpp"
#include "fileedit.hpp"

static int Recurse=0;

enum {EOL_NONE,EOL_CR,EOL_LF,EOL_CRLF,EOL_CRCRLF};
static const wchar_t *EOL_TYPE_CHARS[]={L"",L"\r",L"\n",L"\r\n",L"\r\r\n"};

#define EDMASK_ANY    L'X' // ��������� ������� � ������ ����� ����� ������;
#define EDMASK_DSS    L'#' // ��������� ������� � ������ ����� �����, ������ � ���� ������;
#define EDMASK_DIGIT  L'9' // ��������� ������� � ������ ����� ������ �����;
#define EDMASK_DIGITS L'N' // ��������� ������� � ������ ����� ������ ����� � �������;
#define EDMASK_ALPHA  L'A' // ��������� ������� � ������ ����� ������ �����.
#define EDMASK_HEX    L'H' // ��������� ������� � ������ ����� ����������������� �������.

Edit::Edit(ScreenObject *pOwner, bool bAllocateData):
	Str(bAllocateData ? static_cast<wchar_t*>(xf_malloc(sizeof(wchar_t))) : nullptr),
	StrSize(0),
	CurPos(0),
	LeftPos(0),
	m_next(nullptr),
	m_prev(nullptr),
	MaxLength(-1),
	Mask(nullptr),
	PrevCurPos(0),
	MSelStart(-1),
	SelStart(-1),
	SelEnd(0),
	CursorSize(-1),
	CursorPos(0)
{
	SetOwner(pOwner);
	SetWordDiv(Opt.strWordDiv);

	if (bAllocateData)
		*Str=0;

	Flags.Set(FEDITLINE_EDITBEYONDEND);
	SetObjectColor(COL_COMMANDLINE, COL_COMMANDLINESELECTED);
	EndType=EOL_NONE;
	ColorList=nullptr;
	ColorCount=MaxColorCount=0;
	ColorListNeedSort=ColorListNeedFree=false;
	TabSize=Opt.EdOpt.TabSize;
	TabExpandMode = EXPAND_NOTABS;
	Flags.Change(FEDITLINE_DELREMOVESBLOCKS,Opt.EdOpt.DelRemovesBlocks);
	Flags.Change(FEDITLINE_PERSISTENTBLOCKS,Opt.EdOpt.PersistentBlocks);
	Flags.Change(FEDITLINE_SHOWWHITESPACE,Opt.EdOpt.ShowWhiteSpace);
	Flags.Change(FEDITLINE_SHOWLINEBREAK,Opt.EdOpt.ShowWhiteSpace==1);
	m_codepage = 0; //BUGBUG
}


Edit::~Edit()
{
	if (ColorList)
		xf_free(ColorList);

	if (Mask)
		xf_free(Mask);

	if (Str)
		xf_free(Str);
}

DWORD Edit::SetCodePage(UINT codepage, bool Set)
{
	DWORD Ret=SETCP_NOERROR;
	DWORD wc2mbFlags=WC_NO_BEST_FIT_CHARS;
	BOOL UsedDefaultChar=FALSE;
	LPBOOL lpUsedDefaultChar=&UsedDefaultChar;

	if (m_codepage==CP_UTF7 || m_codepage==CP_UTF8) // BUGBUG: CP_SYMBOL, 50xxx, 57xxx too
	{
		wc2mbFlags=0;
		lpUsedDefaultChar=nullptr;
	}

	DWORD mb2wcFlags=MB_ERR_INVALID_CHARS;

	if (codepage==CP_UTF7) // BUGBUG: CP_SYMBOL, 50xxx, 57xxx too
	{
		mb2wcFlags=0;
	}

	if (codepage != m_codepage)
	{
		if (Str && *Str)
		{
			//m_codepage = codepage;
			int length = WideCharToMultiByte(m_codepage, wc2mbFlags, Str, StrSize, nullptr, 0, nullptr, lpUsedDefaultChar);
			if (UsedDefaultChar)
			{
				Ret|=SETCP_WC2MBERROR;
			}
			if ((Ret == SETCP_NOERROR) || (Ret!=SETCP_NOERROR && Set))
			{
				char *decoded = (char*)xf_malloc(length);
				WideCharToMultiByte(m_codepage, 0, Str, StrSize, decoded, length, nullptr, nullptr);
				int length2 = MultiByteToWideChar(codepage, mb2wcFlags, decoded, length, nullptr, 0);
				if (!length2 && GetLastError()==ERROR_NO_UNICODE_TRANSLATION)
				{
					Ret|=SETCP_MB2WCERROR;
					length2 = MultiByteToWideChar(codepage, 0, decoded, length, nullptr, 0);
				}
				if (Set)
				{
					wchar_t *encoded = (wchar_t*)xf_malloc((length2+1)*sizeof(wchar_t));
					length2 = MultiByteToWideChar(codepage, 0, decoded, length, encoded, length2);
					encoded[length2] = L'\0';
					xf_free(Str);
					Str = encoded;
					StrSize = length2;
					m_codepage = codepage;
					Changed();
				}
				xf_free(decoded);
			}
		}
		else
		{
			m_codepage = codepage;
		}
	}
	return Ret;
}

UINT Edit::GetCodePage()
{
	return m_codepage;
}


void Edit::DisplayObject()
{
	if (Flags.Check(FEDITLINE_DROPDOWNBOX))
	{
		Flags.Clear(FEDITLINE_CLEARFLAG);  // ��� ����-���� ��� �� ����� �������� unchanged text
		SelStart=0;
		SelEnd=StrSize; // � ����� ������� ��� ��� �������� -
		//    ���� �� ���������� �� ������� Edit
	}

	//   ���������� ������ ��������� ������� � ������ � ������ Mask.
	int Value=(PrevCurPos>CurPos)?-1:1;
	CurPos=GetNextCursorPos(CurPos,Value);
	FastShow();

	/* $ 26.07.2000 tran
	   ��� DropDownBox ������ ���������
	   �� ���� ���� - ���������� �� �� ����� ������� ����� */
	if (Flags.Check(FEDITLINE_DROPDOWNBOX))
		::SetCursorType(0,10);
	else
	{
		if (Flags.Check(FEDITLINE_OVERTYPE))
		{
			int NewCursorSize=IsConsoleFullscreen()?
			                  (Opt.CursorSize[3]?Opt.CursorSize[3]:99):
					                  (Opt.CursorSize[2]?Opt.CursorSize[2]:99);
			::SetCursorType(1,CursorSize==-1?NewCursorSize:CursorSize);
		}
		else
{
			int NewCursorSize=IsConsoleFullscreen()?
			                  (Opt.CursorSize[1]?Opt.CursorSize[1]:10):
					                  (Opt.CursorSize[0]?Opt.CursorSize[0]:10);
			::SetCursorType(1,CursorSize==-1?NewCursorSize:CursorSize);
		}
	}

	MoveCursor(X1+CursorPos-LeftPos,Y1);
}


void Edit::SetCursorType(bool Visible, DWORD Size)
{
	Flags.Change(FEDITLINE_CURSORVISIBLE,Visible);
	CursorSize=Size;
	::SetCursorType(Visible,Size);
}

void Edit::GetCursorType(bool& Visible, DWORD& Size)
{
	Visible=Flags.Check(FEDITLINE_CURSORVISIBLE)!=FALSE;
	Size=CursorSize;
}

//   ���������� ������ ��������� ������� � ������ � ������ Mask.
int Edit::GetNextCursorPos(int Position,int Where)
{
	int Result=Position;

	if (Mask && *Mask && (Where==-1 || Where==1))
	{
		int PosChanged=FALSE;
		int MaskLen=StrLength(Mask);

		for (int i=Position; i<MaskLen && i>=0; i+=Where)
		{
			if (CheckCharMask(Mask[i]))
			{
				Result=i;
				PosChanged=TRUE;
				break;
			}
		}

		if (!PosChanged)
		{
			for (int i=Position; i>=0; i--)
			{
				if (CheckCharMask(Mask[i]))
				{
					Result=i;
					PosChanged=TRUE;
					break;
				}
			}
		}

		if (!PosChanged)
		{
			for (int i=Position; i<MaskLen; i++)
			{
				if (CheckCharMask(Mask[i]))
				{
					Result=i;
					break;
				}
			}
		}
	}

	return Result;
}

void Edit::FastShow()
{
	const int EditLength=ObjWidth;

	if (!Flags.Check(FEDITLINE_EDITBEYONDEND) && CurPos>StrSize && StrSize>=0)
		CurPos=StrSize;

	if (MaxLength!=-1)
	{
		if (StrSize>MaxLength)
		{
			Str[MaxLength]=0;
			StrSize=MaxLength;
		}

		if (CurPos>MaxLength-1)
			CurPos=MaxLength>0 ? (MaxLength-1):0;
	}

	int TabCurPos=GetTabCurPos();

	/* $ 31.07.2001 KM
	  ! ��� ���������� ������� ����������� ������
	    � ������ �������.
	*/
	int UnfixedLeftPos = LeftPos;
	if (!Flags.Check(FEDITLINE_DROPDOWNBOX))
	{
		FixLeftPos(TabCurPos);
	}

	GotoXY(X1,Y1);
	int TabSelStart=(SelStart==-1) ? -1:RealPosToTab(SelStart);
	int TabSelEnd=(SelEnd<0) ? -1:RealPosToTab(SelEnd);

	/* $ 17.08.2000 KM
	   ���� ���� �����, ������� ���������� ������, �� ����
	   ��� "����������" ������� � �����, �� ���������� ����������
	   ������ ��������� �������������� � Str
	*/
	if (Mask && *Mask)
		RefreshStrByMask();

	wchar_t *OutStrTmp=(wchar_t *)xf_malloc((EditLength+1)*sizeof(wchar_t));

	if (!OutStrTmp)
		return;

	wchar_t *OutStr=(wchar_t *)xf_malloc((EditLength+1)*sizeof(wchar_t));

	if (!OutStr)
	{
		xf_free(OutStrTmp);
		return;
	}

	CursorPos=TabCurPos;
	int RealLeftPos=TabPosToReal(LeftPos);
	int OutStrLength=Min(EditLength,StrSize-RealLeftPos);

	if (OutStrLength < 0)
	{
		OutStrLength=0;
	}
	else
	{
		wmemcpy(OutStrTmp,Str+RealLeftPos,OutStrLength);
	}

	{
		wchar_t *p=OutStrTmp;
		wchar_t *e=OutStrTmp+OutStrLength;

		for (OutStrLength=0; OutStrLength<EditLength && p<e; p++)
		{
			if (Flags.Check(FEDITLINE_SHOWWHITESPACE) && Flags.Check(FEDITLINE_EDITORMODE))
			{
				if (*p==L' ')
				{
					*p=L'\xB7';
				}
			}

			if (*p == L'\t')
			{
				int S=TabSize-((UnfixedLeftPos+OutStrLength) % TabSize);
				OutStr[OutStrLength]=(Flags.Check(FEDITLINE_SHOWWHITESPACE) && Flags.Check(FEDITLINE_EDITORMODE) && (OutStrLength || S==TabSize))?L'\x2192':L' ';
				OutStrLength++;
				for (int i=1; i<S && OutStrLength<EditLength; i++,OutStrLength++)
				{
					OutStr[OutStrLength]=L' ';
				}
			}
			else
			{
				if (!*p)
					OutStr[OutStrLength]=L' ';
				else
					OutStr[OutStrLength]=*p;

				OutStrLength++;
			}
		}

		if (Flags.Check(FEDITLINE_PASSWORDMODE))
			wmemset(OutStr,L'*',OutStrLength);

#if 0
	//Maximus: � bis ���� ���
	if (Flags.Check(FEDITLINE_SHOWLINEBREAK) && Flags.Check(FEDITLINE_EDITORMODE) && (OutStrLength < EditLength))
	{
		const wchar_t* EndSeq = GetEOL();
		if (EndSeq && *EndSeq)
			OutStr[OutStrLength++]=L'\xB6'; //L'\x266A';
	}
#endif
		if (Flags.Check(FEDITLINE_SHOWLINEBREAK) && Flags.Check(FEDITLINE_EDITORMODE) && (StrSize >= RealLeftPos) && (OutStrLength < EditLength))
		{
			switch(EndType)
			{
			case EOL_CR:
				OutStr[OutStrLength++]=Oem2Unicode[13];
				break;
			case EOL_LF:
				OutStr[OutStrLength++]=Oem2Unicode[10];
				break;
			case EOL_CRLF:
				#if 1
				//Maximus: "�������" �������� ����� - �������� ���������
				OutStr[OutStrLength++]=L'\xB6';
				#else
				OutStr[OutStrLength++]=Oem2Unicode[13];
				if(OutStrLength < EditLength)
				{
					OutStr[OutStrLength++]=Oem2Unicode[10];
				}
				#endif
				break;
			case EOL_CRCRLF:
				OutStr[OutStrLength++]=Oem2Unicode[13];
				if(OutStrLength < EditLength)
				{
					OutStr[OutStrLength++]=Oem2Unicode[13];
					if(OutStrLength < EditLength)
					{
						OutStr[OutStrLength++]=Oem2Unicode[10];
					}
				}
				break;
			}
		}

		if(!m_next && Flags.Check(FEDITLINE_SHOWWHITESPACE) && Flags.Check(FEDITLINE_EDITORMODE) && (StrSize >= RealLeftPos) && (OutStrLength < EditLength))
		{
			OutStr[OutStrLength++]=L'\x25a1';
		}
	}

	OutStr[OutStrLength]=0;
	SetColor(Color);

	if (TabSelStart==-1)
	{
		if (Flags.Check(FEDITLINE_CLEARFLAG))
		{
			SetUnchangedColor();

			if (Mask && *Mask)
				OutStrLength=StrLength(RemoveTrailingSpaces(OutStr));

			FS<<fmt::LeftAlign()<<fmt::Width(OutStrLength)<<fmt::Precision(OutStrLength)<<OutStr;
			SetColor(Color);
			int BlankLength=EditLength-OutStrLength;

			if (BlankLength > 0)
			{
				FS<<fmt::Width(BlankLength)<<L"";
			}
		}
		else
		{
			FS<<fmt::LeftAlign()<<fmt::Width(EditLength)<<fmt::Precision(EditLength)<<OutStr;
		}
	}
	else
	{
		if ((TabSelStart-=LeftPos)<0)
			TabSelStart=0;

		int AllString=(TabSelEnd==-1);

		if (AllString)
			TabSelEnd=EditLength;
		else if ((TabSelEnd-=LeftPos)<0)
			TabSelEnd=0;

		wmemset(OutStr+OutStrLength,L' ',EditLength-OutStrLength);
		OutStr[EditLength]=0;

		/* $ 24.08.2000 SVS
		   ! � DropDowList`� ��������� �� ������ ��������� - �� ��� ������� �����
		     ���� ���� ������ ������
		*/
		if (TabSelStart>=EditLength /*|| !AllString && TabSelStart>=StrSize*/ ||
		        TabSelEnd<TabSelStart)
		{
			if (Flags.Check(FEDITLINE_DROPDOWNBOX))
			{
				SetColor(SelColor);
				FS<<fmt::Width(X2-X1+1)<<OutStr;
			}
			else
				Text(OutStr);
		}
		else
		{
			FS<<fmt::Precision(TabSelStart)<<OutStr;
			SetColor(SelColor);

			if (!Flags.Check(FEDITLINE_DROPDOWNBOX))
			{
				FS<<fmt::Precision(TabSelEnd-TabSelStart)<<OutStr+TabSelStart;

				if (TabSelEnd<EditLength)
				{
					//SetColor(Flags.Check(FEDITLINE_CLEARFLAG) ? SelColor:Color);
					SetColor(Color);
					Text(OutStr+TabSelEnd);
				}
			}
			else
			{
				FS<<fmt::Width(X2-X1+1)<<OutStr;
			}
		}
	}

	xf_free(OutStr);
	xf_free(OutStrTmp);

	/* $ 26.07.2000 tran
	   ��� ����-���� ����� ��� �� ����� */
	if (!Flags.Check(FEDITLINE_DROPDOWNBOX))
		ApplyColor();
}


int Edit::RecurseProcessKey(int Key)
{
	Recurse++;
	int RetCode=ProcessKey(Key);
	Recurse--;
	return(RetCode);
}


// ������� ������� ������ ��������� - �� ��������� �� ���� ������
int Edit::ProcessInsPath(int Key,int PrevSelStart,int PrevSelEnd)
{
	int RetCode=FALSE;
	string strPathName;

	if (Key>=KEY_RCTRL0 && Key<=KEY_RCTRL9) // ��������?
	{
		if (CtrlObject->FolderShortcuts->Get(Key-KEY_RCTRL0,&strPathName,nullptr,nullptr,nullptr))
			RetCode=TRUE;
	}
	else // ����/�����?
	{
		RetCode=_MakePath1(Key,strPathName,L"");
	}

	// ���� ���-���� ����������, ������ ��� � ������� (PathName)
	if (RetCode)
	{
		if (Flags.Check(FEDITLINE_CLEARFLAG))
		{
			LeftPos=0;
			SetString(L"");
		}

		if (PrevSelStart!=-1)
		{
			SelStart=PrevSelStart;
			SelEnd=PrevSelEnd;
		}

		if (!Flags.Check(FEDITLINE_PERSISTENTBLOCKS))
			DeleteBlock();

		InsertString(strPathName);
		Flags.Clear(FEDITLINE_CLEARFLAG);
	}

	return RetCode;
}


__int64 Edit::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
	switch (OpCode)
	{
		case MCODE_C_EMPTY:
			return (__int64)!GetLength();
		case MCODE_C_SELECTED:
			return (__int64)(SelStart != -1 && SelStart < SelEnd);
		case MCODE_C_EOF:
			return (__int64)(CurPos >= StrSize);
		case MCODE_C_BOF:
			return (__int64)!CurPos;
		case MCODE_V_ITEMCOUNT:
			return (__int64)StrSize;
		case MCODE_V_CURPOS:
			return (__int64)(CursorPos+1);
		case MCODE_F_EDITOR_SEL:
		{
			int Action=(int)((INT_PTR)vParam);

			switch (Action)
			{
				case 0:  // Get Param
				{
					switch (iParam)
					{
						case 0:  // return FirstLine
						case 2:  // return LastLine
							return IsSelection()?1:0;
						case 1:  // return FirstPos
							return IsSelection()?SelStart+1:0;
						case 3:  // return LastPos
							return IsSelection()?SelEnd:0;
						case 4: // return block type (0=nothing 1=stream, 2=column)
							return IsSelection()?1:0;
					}

					break;
				}
				case 1:  // Set Pos
				{
					if (IsSelection())
					{
						switch (iParam)
						{
							case 0: // begin block (FirstLine & FirstPos)
							case 1: // end block (LastLine & LastPos)
							{
								SetTabCurPos(iParam?SelEnd:SelStart);
								Show();
								return 1;
							}
						}
					}

					break;
				}
				case 2: // Set Stream Selection Edge
				case 3: // Set Column Selection Edge
				{
					switch (iParam)
					{
						case 0:  // selection start
						{
							MSelStart=GetTabCurPos();
							return 1;
						}
						case 1:  // selection finish
						{
							if (MSelStart != -1)
							{
								if (MSelStart != GetTabCurPos())
									Select(MSelStart,GetTabCurPos());
								else
									Select(-1,0);

								Show();
								MSelStart=-1;
								return 1;
							}

							return 0;
						}
					}

					break;
				}
				case 4: // UnMark sel block
				{
					Select(-1,0);
					MSelStart=-1;
					Show();
					return 1;
				}
			}

			break;
		}
	}

	return 0;
}

int Edit::ProcessKey(int Key)
{
	switch (Key)
	{
		case KEY_ADD:
			Key=L'+';
			break;
		case KEY_SUBTRACT:
			Key=L'-';
			break;
		case KEY_MULTIPLY:
			Key=L'*';
			break;
		case KEY_DIVIDE:
			Key=L'/';
			break;
		case KEY_DECIMAL:
			Key=L'.';
			break;
		case KEY_CTRLC:
		case KEY_RCTRLC:
			Key=KEY_CTRLINS;
			break;
		case KEY_CTRLV:
		case KEY_RCTRLV:
			Key=KEY_SHIFTINS;
			break;
		case KEY_CTRLX:
		case KEY_RCTRLX:
			Key=KEY_SHIFTDEL;
			break;
	}

	int PrevSelStart=-1,PrevSelEnd=0;

	if (!Flags.Check(FEDITLINE_DROPDOWNBOX) && (Key==KEY_CTRLL || Key==KEY_RCTRLL))
	{
		Flags.Swap(FEDITLINE_READONLY);
	}

	/* $ 26.07.2000 SVS
	   Bugs #??
	     � ������� ����� ��� ���������� ����� �������� BS � ������
	     ���������� �������� ����� (��� � ���������) ��������:
	       - ������ ����� �������� ������
	       - ��������� ����� �����
	*/
	if ((((Key==KEY_BS || Key==KEY_DEL || Key==KEY_NUMDEL) && Flags.Check(FEDITLINE_DELREMOVESBLOCKS)) || Key==KEY_CTRLD || Key==KEY_RCTRLD) &&
	        !Flags.Check(FEDITLINE_EDITORMODE) && SelStart!=-1 && SelStart<SelEnd)
	{
		DeleteBlock();
		Show();
		return TRUE;
	}

	int _Macro_IsExecuting=CtrlObject->Macro.IsExecuting();

	// $ 04.07.2000 IG - ��������� ��������� �� ������ ������� (00025.edit.cpp.txt)
	if (!IntKeyState.ShiftPressed && (!_Macro_IsExecuting || (IsNavKey(Key) && _Macro_IsExecuting)) &&
	        !IsShiftKey(Key) && !Recurse &&
	        Key!=KEY_SHIFT && Key!=KEY_CTRL && Key!=KEY_ALT &&
	        Key!=KEY_RCTRL && Key!=KEY_RALT && Key!=KEY_NONE &&
	        Key!=KEY_INS &&
	        Key!=KEY_KILLFOCUS && Key != KEY_GOTFOCUS &&
	        ((Key&(~KEY_CTRLMASK)) != KEY_LWIN && (Key&(~KEY_CTRLMASK)) != KEY_RWIN && (Key&(~KEY_CTRLMASK)) != KEY_APPS)
	   )
	{
		Flags.Clear(FEDITLINE_MARKINGBLOCK); // ���... � ��� ����� ������ ����?

		if (!Flags.Check(FEDITLINE_PERSISTENTBLOCKS) && !(Key==KEY_CTRLINS || Key==KEY_RCTRLINS || Key==KEY_CTRLNUMPAD0 || Key==KEY_RCTRLNUMPAD0) &&
		        !(Key==KEY_SHIFTDEL||Key==KEY_SHIFTNUMDEL||Key==KEY_SHIFTDECIMAL) && !Flags.Check(FEDITLINE_EDITORMODE) &&
		        (Key != KEY_CTRLQ && Key != KEY_RCTRLQ) &&
		        !(Key == KEY_SHIFTINS || Key == KEY_SHIFTNUMPAD0)) //Key != KEY_SHIFTINS) //??
		{
			/* $ 12.11.2002 DJ
			   ����� ����������, ���� ������ �� ����������?
			*/
			if (SelStart != -1 || SelEnd )
			{
				PrevSelStart=SelStart;
				PrevSelEnd=SelEnd;
				Select(-1,0);
				Show();
			}
		}
	}

	/* $ 11.09.2000 SVS
	   ���� Opt.DlgEULBsClear = 1, �� BS � �������� ��� UnChanged ������
	   ������� ����� ������ �����, ��� � Del
	*/
	if (((Opt.Dialogs.EULBsClear && Key==KEY_BS) || Key==KEY_DEL || Key==KEY_NUMDEL) &&
	        Flags.Check(FEDITLINE_CLEARFLAG) && CurPos>=StrSize)
		Key=KEY_CTRLY;

	/* $ 15.09.2000 SVS
	   Bug - �������� ������� ������ -> Shift-Del ������� ��� ������
	         ��� ������ ���� ������ ��� UnChanged ���������
	*/
	if ((Key == KEY_SHIFTDEL || Key == KEY_SHIFTNUMDEL || Key == KEY_SHIFTDECIMAL) && Flags.Check(FEDITLINE_CLEARFLAG) && CurPos>=StrSize && SelStart==-1)
	{
		SelStart=0;
		SelEnd=StrSize;
	}

	if (Flags.Check(FEDITLINE_CLEARFLAG) && ((Key <= 0xFFFF && Key!=KEY_BS) || Key==KEY_CTRLBRACKET || Key==KEY_RCTRLBRACKET ||
	        Key==KEY_CTRLBACKBRACKET || Key==KEY_RCTRLBACKBRACKET || Key==KEY_CTRLSHIFTBRACKET || Key==KEY_RCTRLSHIFTBRACKET ||
	        Key==KEY_CTRLSHIFTBACKBRACKET || Key==KEY_RCTRLSHIFTBACKBRACKET || Key==KEY_SHIFTENTER || Key==KEY_SHIFTNUMENTER))
	{
		LeftPos=0;
		SetString(L"");
		Show();
	}

	// ����� - ����� ������� ������� �����/������
	if (ProcessInsPath(Key,PrevSelStart,PrevSelEnd))
	{
		Show();
		return TRUE;
	}

	if (Key!=KEY_NONE && Key!=KEY_IDLE && Key!=KEY_SHIFTINS && Key!=KEY_SHIFTNUMPAD0 && (Key!=KEY_CTRLINS && Key!=KEY_RCTRLINS) &&
	        ((unsigned int)Key<KEY_F1 || (unsigned int)Key>KEY_F12) && Key!=KEY_ALT && Key!=KEY_SHIFT &&
	        Key!=KEY_CTRL && Key!=KEY_RALT && Key!=KEY_RCTRL &&
	        !((Key>=KEY_ALT_BASE && Key <= KEY_ALT_BASE+0xFFFF) || (Key>=KEY_RALT_BASE && Key <= KEY_RALT_BASE+0xFFFF)) && // ???? 256 ???
	        !(((unsigned int)Key>=KEY_MACRO_BASE && (unsigned int)Key<=KEY_MACRO_ENDBASE) || ((unsigned int)Key>=KEY_OP_BASE && (unsigned int)Key <=KEY_OP_ENDBASE)) &&
	        (Key!=KEY_CTRLQ && Key!=KEY_RCTRLQ))
	{
		Flags.Clear(FEDITLINE_CLEARFLAG);
		Show();
	}

	switch (Key)
	{
		case KEY_CTRLA: case KEY_RCTRLA:
			{
				Select(0, GetLength());
				Show();
			}
			break;
		case KEY_SHIFTLEFT: case KEY_SHIFTNUMPAD4:
		{
			if (CurPos>0)
			{
				RecurseProcessKey(KEY_LEFT);

				if (!Flags.Check(FEDITLINE_MARKINGBLOCK))
				{
					Select(-1,0);
					Flags.Set(FEDITLINE_MARKINGBLOCK);
				}

				if (SelStart!=-1 && SelStart<=CurPos)
					Select(SelStart,CurPos);
				else
				{
					int EndPos=CurPos+1;
					int NewStartPos=CurPos;

					if (EndPos>StrSize)
						EndPos=StrSize;

					if (NewStartPos>StrSize)
						NewStartPos=StrSize;

					AddSelect(NewStartPos,EndPos);
				}

				Show();
			}

			return TRUE;
		}
		case KEY_SHIFTRIGHT: case KEY_SHIFTNUMPAD6:
		{
			if (!Flags.Check(FEDITLINE_MARKINGBLOCK))
			{
				Select(-1,0);
				Flags.Set(FEDITLINE_MARKINGBLOCK);
			}

			if ((SelStart!=-1 && SelEnd==-1) || SelEnd>CurPos)
			{
				if (CurPos+1==SelEnd)
					Select(-1,0);
				else
					Select(CurPos+1,SelEnd);
			}
			else
				AddSelect(CurPos,CurPos+1);

			RecurseProcessKey(KEY_RIGHT);
			return TRUE;
		}
		case KEY_CTRLSHIFTLEFT:  case KEY_CTRLSHIFTNUMPAD4:
		case KEY_RCTRLSHIFTLEFT: case KEY_RCTRLSHIFTNUMPAD4:
		{
			if (CurPos>StrSize)
			{
				PrevCurPos=CurPos;
				CurPos=StrSize;
			}

			if (CurPos>0)
				RecurseProcessKey(KEY_SHIFTLEFT);

			while (CurPos>0 && !(!IsWordDiv(WordDiv(), Str[CurPos]) &&
			                     IsWordDiv(WordDiv(),Str[CurPos-1]) && !IsSpace(Str[CurPos])))
			{
				if (!IsSpace(Str[CurPos]) && (IsSpace(Str[CurPos-1]) ||
				                              IsWordDiv(WordDiv(), Str[CurPos-1])))
					break;

				RecurseProcessKey(KEY_SHIFTLEFT);
			}

			Show();
			return TRUE;
		}
		case KEY_CTRLSHIFTRIGHT:  case KEY_CTRLSHIFTNUMPAD6:
		case KEY_RCTRLSHIFTRIGHT: case KEY_RCTRLSHIFTNUMPAD6:
		{
			if (CurPos>=StrSize)
				return FALSE;

			RecurseProcessKey(KEY_SHIFTRIGHT);

			while (CurPos<StrSize && !(IsWordDiv(WordDiv(), Str[CurPos]) &&
			                           !IsWordDiv(WordDiv(), Str[CurPos-1])))
			{
				if (!IsSpace(Str[CurPos]) && (IsSpace(Str[CurPos-1]) || IsWordDiv(WordDiv(), Str[CurPos-1])))
					break;

				RecurseProcessKey(KEY_SHIFTRIGHT);

				if (MaxLength!=-1 && CurPos==MaxLength-1)
					break;
			}

			Show();
			return TRUE;
		}
		case KEY_SHIFTHOME:  case KEY_SHIFTNUMPAD7:
		{
			Lock();

			while (CurPos>0)
				RecurseProcessKey(KEY_SHIFTLEFT);

			Unlock();
			Show();
			return TRUE;
		}
		case KEY_SHIFTEND:  case KEY_SHIFTNUMPAD1:
		{
			Lock();
			int Len;

			if (Mask && *Mask)
			{
				wchar_t *ShortStr=new wchar_t[StrSize+1];

				if (!ShortStr)
					return FALSE;

				xwcsncpy(ShortStr,Str,StrSize+1);
				Len=StrLength(RemoveTrailingSpaces(ShortStr));
				delete[] ShortStr;
			}
			else
				Len=StrSize;

			int LastCurPos=CurPos;

			while (CurPos<Len/*StrSize*/)
			{
				RecurseProcessKey(KEY_SHIFTRIGHT);

				if (LastCurPos==CurPos)break;

				LastCurPos=CurPos;
			}

			Unlock();
			Show();
			return TRUE;
		}
		case KEY_BS:
		{
			if (CurPos<=0)
				return FALSE;

			PrevCurPos=CurPos;
			CurPos--;

			if (CurPos<=LeftPos)
			{
				LeftPos-=15;

				if (LeftPos<0)
					LeftPos=0;
			}

			if (!RecurseProcessKey(KEY_DEL))
				Show();

			return TRUE;
		}
		case KEY_CTRLSHIFTBS:
		case KEY_RCTRLSHIFTBS:
		{
			DisableCallback();

			// BUGBUG
			for (int i=CurPos; i>=0; i--)
			{
				RecurseProcessKey(KEY_BS);
			}
			RevertCallback();
			Changed(true);
			Show();
			return TRUE;
		}
		case KEY_CTRLBS:
		case KEY_RCTRLBS:
		{
			if (CurPos>StrSize)
			{
				PrevCurPos=CurPos;
				CurPos=StrSize;
			}

			Lock();

			DisableCallback();

			// BUGBUG
			for (;;)
			{
				int StopDelete=FALSE;

				if (CurPos>1 && IsSpace(Str[CurPos-1])!=IsSpace(Str[CurPos-2]))
					StopDelete=TRUE;

				RecurseProcessKey(KEY_BS);

				if (!CurPos || StopDelete)
					break;

				if (IsWordDiv(WordDiv(),Str[CurPos-1]))
					break;
			}

			Unlock();
			RevertCallback();
			Changed(true);
			Show();
			return TRUE;
		}
		case KEY_CTRLQ:
		case KEY_RCTRLQ:
		{
			Lock();

			if (!Flags.Check(FEDITLINE_PERSISTENTBLOCKS) && (SelStart != -1 || Flags.Check(FEDITLINE_CLEARFLAG)))
				RecurseProcessKey(KEY_DEL);

			ProcessCtrlQ();
			Unlock();
			Show();
			return TRUE;
		}
		case KEY_OP_SELWORD:
		{
			int OldCurPos=CurPos;
			PrevSelStart=SelStart;
			PrevSelEnd=SelEnd;
#if defined(MOUSEKEY)

			if (CurPos >= SelStart && CurPos <= SelEnd)
			{ // �������� ��� ������ ��� ��������� ������� �����
				Select(0,StrSize);
			}
			else
#endif
			{
				int SStart, SEnd;

				if (CalcWordFromString(Str,CurPos,&SStart,&SEnd,WordDiv()))
					Select(SStart,SEnd+(SEnd < StrSize?1:0));
			}

			CurPos=OldCurPos; // ���������� �������
			Show();
			return TRUE;
		}
		case KEY_OP_PLAINTEXT:
		{
			if (!Flags.Check(FEDITLINE_PERSISTENTBLOCKS))
			{
				if (SelStart != -1 || Flags.Check(FEDITLINE_CLEARFLAG)) // BugZ#1053 - ���������� � $Text
					RecurseProcessKey(KEY_DEL);
			}

			const wchar_t *S = eStackAsString();

			ProcessInsPlainText(S);

			Show();
			return TRUE;
		}
		case KEY_CTRLT:
		case KEY_CTRLDEL:
		case KEY_CTRLNUMDEL:
		case KEY_CTRLDECIMAL:
		case KEY_RCTRLT:
		case KEY_RCTRLDEL:
		case KEY_RCTRLNUMDEL:
		case KEY_RCTRLDECIMAL:
		{
			if (CurPos>=StrSize)
				return FALSE;

			Lock();
			DisableCallback();
			if (Mask && *Mask)
			{
				int MaskLen=StrLength(Mask);
				int ptr=CurPos;

				while (ptr<MaskLen)
				{
					ptr++;

					if (!CheckCharMask(Mask[ptr]) ||
					        (IsSpace(Str[ptr]) && !IsSpace(Str[ptr+1])) ||
					        (IsWordDiv(WordDiv(), Str[ptr])))
						break;
				}

				// BUGBUG
				for (int i=0; i<ptr-CurPos; i++)
					RecurseProcessKey(KEY_DEL);
			}
			else
			{
				for (;;)
				{
					int StopDelete=FALSE;

					if (CurPos<StrSize-1 && IsSpace(Str[CurPos]) && !IsSpace(Str[CurPos+1]))
						StopDelete=TRUE;

					RecurseProcessKey(KEY_DEL);

					if (CurPos>=StrSize || StopDelete)
						break;

					if (IsWordDiv(WordDiv(), Str[CurPos]))
						break;
				}
			}

			Unlock();
			RevertCallback();
			Changed(true);
			Show();
			return TRUE;
		}
		case KEY_CTRLY:
		case KEY_RCTRLY:
		{
			if (Flags.Check(FEDITLINE_READONLY|FEDITLINE_DROPDOWNBOX))
				return (TRUE);

			PrevCurPos=CurPos;
			LeftPos=CurPos=0;
			*Str=0;
			StrSize=0;
			Str=(wchar_t *)xf_realloc(Str,1*sizeof(wchar_t));
			Select(-1,0);
			Changed();
			Show();
			return TRUE;
		}
		case KEY_CTRLK:
		case KEY_RCTRLK:
		{
			if (Flags.Check(FEDITLINE_READONLY|FEDITLINE_DROPDOWNBOX))
				return (TRUE);

			if (CurPos>=StrSize)
				return FALSE;

			if (!Flags.Check(FEDITLINE_EDITBEYONDEND))
			{
				if (CurPos<SelEnd)
					SelEnd=CurPos;

				if (SelEnd<SelStart && SelEnd!=-1)
				{
					SelEnd=0;
					SelStart=-1;
				}
			}

			Str[CurPos]=0;
			StrSize=CurPos;
			Str=(wchar_t *)xf_realloc(Str,(StrSize+1)*sizeof(wchar_t));
			Changed();
			Show();
			return TRUE;
		}
		case KEY_HOME:        case KEY_NUMPAD7:
		case KEY_CTRLHOME:    case KEY_CTRLNUMPAD7:
		case KEY_RCTRLHOME:   case KEY_RCTRLNUMPAD7:
		{
			PrevCurPos=CurPos;
			CurPos=0;
			Show();
			return TRUE;
		}
		case KEY_END:           case KEY_NUMPAD1:
		case KEY_CTRLEND:       case KEY_CTRLNUMPAD1:
		case KEY_RCTRLEND:      case KEY_RCTRLNUMPAD1:
		case KEY_CTRLSHIFTEND:  case KEY_CTRLSHIFTNUMPAD1:
		case KEY_RCTRLSHIFTEND: case KEY_RCTRLSHIFTNUMPAD1:
		{
			PrevCurPos=CurPos;

			if (Mask && *Mask)
			{
				wchar_t *ShortStr=new wchar_t[StrSize+1];

				if (!ShortStr)
					return FALSE;

				xwcsncpy(ShortStr,Str,StrSize+1);
				CurPos=StrLength(RemoveTrailingSpaces(ShortStr));
				delete[] ShortStr;
			}
			else
				CurPos=StrSize;

			Show();
			return TRUE;
		}
		case KEY_LEFT:        case KEY_NUMPAD4:        case KEY_MSWHEEL_LEFT:
		case KEY_CTRLS:       case KEY_RCTRLS:
		{
			if (CurPos>0)
			{
				PrevCurPos=CurPos;
				CurPos--;
				Show();
			}

			return TRUE;
		}
		case KEY_RIGHT:       case KEY_NUMPAD6:        case KEY_MSWHEEL_RIGHT:
		case KEY_CTRLD:       case KEY_RCTRLD:
		{
			PrevCurPos=CurPos;

			if (Mask && *Mask)
			{
				wchar_t *ShortStr=new wchar_t[StrSize+1];

				if (!ShortStr)
					return FALSE;

				xwcsncpy(ShortStr,Str,StrSize+1);
				int Len=StrLength(RemoveTrailingSpaces(ShortStr));
				delete[] ShortStr;

				if (Len>CurPos)
					CurPos++;
			}
			else
				CurPos++;

			Show();
			return TRUE;
		}
		case KEY_INS:         case KEY_NUMPAD0:
		{
			Flags.Swap(FEDITLINE_OVERTYPE);
			Show();
			return TRUE;
		}
		case KEY_NUMDEL:
		case KEY_DEL:
		{
			if (Flags.Check(FEDITLINE_READONLY|FEDITLINE_DROPDOWNBOX))
				return (TRUE);

			if (CurPos>=StrSize)
				return FALSE;

			if (SelStart!=-1)
			{
				if (SelEnd!=-1 && CurPos<SelEnd)
					SelEnd--;

				if (CurPos<SelStart)
					SelStart--;

				if (SelEnd!=-1 && SelEnd<=SelStart)
				{
					SelStart=-1;
					SelEnd=0;
				}
			}

			if (Mask && *Mask)
			{
				int MaskLen=StrLength(Mask);
				int i,j;
				for (i=CurPos,j=CurPos; i<MaskLen; i++)
				{
					if (CheckCharMask(Mask[i+1]))
					{
						while (!CheckCharMask(Mask[j]) && j<MaskLen)
							j++;

						Str[j]=Str[i+1];
						j++;
					}
				}

				Str[j]=L' ';
			}
			else
			{
				wmemmove(Str+CurPos,Str+CurPos+1,StrSize-CurPos);
				StrSize--;
				Str=(wchar_t *)xf_realloc(Str,(StrSize+1)*sizeof(wchar_t));
			}

			Changed(true);
			Show();
			return TRUE;
		}
		case KEY_CTRLLEFT:  case KEY_CTRLNUMPAD4:
		case KEY_RCTRLLEFT: case KEY_RCTRLNUMPAD4:
		{
			PrevCurPos=CurPos;

			if (CurPos>StrSize)
				CurPos=StrSize;

			if (CurPos>0)
				CurPos--;

			while (CurPos>0 && !(!IsWordDiv(WordDiv(), Str[CurPos]) &&
			                     IsWordDiv(WordDiv(), Str[CurPos-1]) && !IsSpace(Str[CurPos])))
			{
				if (!IsSpace(Str[CurPos]) && IsSpace(Str[CurPos-1]))
					break;

				CurPos--;
			}

			Show();
			return TRUE;
		}
		case KEY_CTRLRIGHT:   case KEY_CTRLNUMPAD6:
		case KEY_RCTRLRIGHT:  case KEY_RCTRLNUMPAD6:
		{
			if (CurPos>=StrSize)
				return FALSE;

			PrevCurPos=CurPos;
			int Len;

			if (Mask && *Mask)
			{
				wchar_t *ShortStr=new wchar_t[StrSize+1];

				if (!ShortStr)
					return FALSE;

				xwcsncpy(ShortStr,Str,StrSize+1);
				Len=StrLength(RemoveTrailingSpaces(ShortStr));
				delete[] ShortStr;

				if (Len>CurPos)
					CurPos++;
			}
			else
			{
				Len=StrSize;
				CurPos++;
			}

			while (CurPos<Len/*StrSize*/ && !(IsWordDiv(WordDiv(),Str[CurPos]) &&
			                                  !IsWordDiv(WordDiv(), Str[CurPos-1])))
			{
				if (!IsSpace(Str[CurPos]) && IsSpace(Str[CurPos-1]))
					break;

				CurPos++;
			}

			Show();
			return TRUE;
		}
		case KEY_SHIFTNUMDEL:
		case KEY_SHIFTDECIMAL:
		case KEY_SHIFTDEL:
		{
			if (SelStart==-1 || SelStart>=SelEnd)
				return FALSE;

			RecurseProcessKey(KEY_CTRLINS);
			DeleteBlock();
			Show();
			return TRUE;
		}
		case KEY_CTRLINS:     case KEY_CTRLNUMPAD0:
		case KEY_RCTRLINS:    case KEY_RCTRLNUMPAD0:
		{
			if (!Flags.Check(FEDITLINE_PASSWORDMODE))
			{
				if (SelStart==-1 || SelStart>=SelEnd)
				{
					if (Mask && *Mask)
					{
						wchar_t *ShortStr=new wchar_t[StrSize+1];

						if (!ShortStr)
							return FALSE;

						xwcsncpy(ShortStr,Str,StrSize+1);
						RemoveTrailingSpaces(ShortStr);
						CopyToClipboard(ShortStr);
						delete[] ShortStr;
					}
					else
					{
						CopyToClipboard(Str);
					}
				}
				else if (SelEnd<=StrSize) // TODO: ���� � ������ ������� �������� "StrSize &&", �� �������� ��� "Ctrl-Ins � ������ ������ ������� ��������"
				{
					int Ch=Str[SelEnd];
					Str[SelEnd]=0;
					CopyToClipboard(Str+SelStart);
					Str[SelEnd]=Ch;
				}
			}

			return TRUE;
		}
		case KEY_SHIFTINS:    case KEY_SHIFTNUMPAD0:
		{
			wchar_t *ClipText=nullptr;

			if (MaxLength==-1)
				ClipText=PasteFromClipboard();
			else
				ClipText=PasteFromClipboardEx(MaxLength);

			if (!ClipText)
				return TRUE;

			if (!Flags.Check(FEDITLINE_PERSISTENTBLOCKS))
			{
				DisableCallback();
				DeleteBlock();
				RevertCallback();
			}

			for (int i=StrLength(Str)-1; i>=0 && IsEol(Str[i]); i--)
				Str[i]=0;

			for (int i=0; ClipText[i]; i++)
			{
				if (IsEol(ClipText[i]))
				{
					if (IsEol(ClipText[i+1]))
						wmemmove(&ClipText[i],&ClipText[i+1],StrLength(&ClipText[i+1])+1);

					if (!ClipText[i+1])
						ClipText[i]=0;
					else
						ClipText[i]=L' ';
				}
			}

			if (Flags.Check(FEDITLINE_CLEARFLAG))
			{
				LeftPos=0;
				Flags.Clear(FEDITLINE_CLEARFLAG);
				SetString(ClipText);
			}
			else
			{
				InsertString(ClipText);
			}

			if (ClipText)
				xf_free(ClipText);

			Show();
			return TRUE;
		}
		case KEY_SHIFTTAB:
		{
			PrevCurPos=CurPos;
			CursorPos-=(CursorPos-1) % TabSize+1;

			if (CursorPos<0) CursorPos=0; //CursorPos=0,TabSize=1 case

			SetTabCurPos(CursorPos);
			Show();
			return TRUE;
		}
		case KEY_SHIFTSPACE:
			Key = KEY_SPACE;
		default:
		{
//      _D(SysLog(L"Key=0x%08X",Key));
			if (Key==KEY_NONE || Key==KEY_IDLE || Key==KEY_ENTER || Key==KEY_NUMENTER || Key>=65536)
				break;

			if (!Flags.Check(FEDITLINE_PERSISTENTBLOCKS))
			{
				if (PrevSelStart!=-1)
				{
					SelStart=PrevSelStart;
					SelEnd=PrevSelEnd;
				}
				DisableCallback();
				DeleteBlock();
				RevertCallback();
			}

			if (InsertKey(Key))
				Show();

			return TRUE;
		}
	}

	return FALSE;
}

// ��������� Ctrl-Q
int Edit::ProcessCtrlQ()
{
	INPUT_RECORD rec;
	DWORD Key;

	for (;;)
	{
		Key=GetInputRecord(&rec);

		if (Key!=KEY_NONE && Key!=KEY_IDLE && rec.Event.KeyEvent.uChar.AsciiChar)
			break;

		if (Key==KEY_CONSOLE_BUFFER_RESIZE)
		{
//      int Dis=EditOutDisabled;
//      EditOutDisabled=0;
			Show();
//      EditOutDisabled=Dis;
		}
	}

	/*
	  EditOutDisabled++;
	  if (!Flags.Check(FEDITLINE_PERSISTENTBLOCKS))
	  {
	    DeleteBlock();
	  }
	  else
	    Flags.Clear(FEDITLINE_CLEARFLAG);
	  EditOutDisabled--;
	*/
	return InsertKey(rec.Event.KeyEvent.uChar.AsciiChar);
}

int Edit::ProcessInsPlainText(const wchar_t *str)
{
	if (*str)
	{
		InsertString(str);
		return TRUE;
	}

	return FALSE;
}

int Edit::InsertKey(int Key)
{
	bool changed=false;
	wchar_t *NewStr;

	if (Flags.Check(FEDITLINE_READONLY|FEDITLINE_DROPDOWNBOX))
		return (TRUE);

	if (Key==KEY_TAB && Flags.Check(FEDITLINE_OVERTYPE))
	{
		PrevCurPos=CurPos;
		CursorPos+=TabSize - (CursorPos % TabSize);
		SetTabCurPos(CursorPos);
		return TRUE;
	}

	if (Mask && *Mask)
	{
		int MaskLen=StrLength(Mask);

		if (CurPos<MaskLen)
		{
			if (KeyMatchedMask(Key))
			{
				if (!Flags.Check(FEDITLINE_OVERTYPE))
				{
					int i=MaskLen-1;

					while (!CheckCharMask(Mask[i]) && i>CurPos)
						i--;

					for (int j=i; i>CurPos; i--)
					{
						if (CheckCharMask(Mask[i]))
						{
							while (!CheckCharMask(Mask[j-1]))
							{
								if (j<=CurPos)
									break;

								j--;
							}

							Str[i]=Str[j-1];
							j--;
						}
					}
				}

				PrevCurPos=CurPos;
				Str[CurPos++]=Key;
				changed=true;
			}
			else
			{
				// ����� ������� ��� "����� ������ �� �����", �������� ��� SetAttr - ������ '.'
				;// char *Ptr=strchr(Mask+CurPos,Key);
			}
		}
		else if (CurPos<StrSize)
		{
			PrevCurPos=CurPos;
			Str[CurPos++]=Key;
			changed=true;
		}
	}
	else
	{
		if (MaxLength == -1 || StrSize < MaxLength)
		{
			if (CurPos>=StrSize)
			{
				if (!(NewStr=(wchar_t *)xf_realloc(Str,(CurPos+2)*sizeof(wchar_t))))
					return FALSE;

				Str=NewStr;
				_snwprintf(&Str[StrSize],CurPos+2,L"%*s",CurPos-StrSize,L"");
				//memset(Str+StrSize,' ',CurPos-StrSize);Str[CurPos+1]=0;
				StrSize=CurPos+1;
			}
			else if (!Flags.Check(FEDITLINE_OVERTYPE))
				StrSize++;

			if (Key==KEY_TAB && (TabExpandMode==EXPAND_NEWTABS || TabExpandMode==EXPAND_ALLTABS))
			{
				StrSize--;
				InsertTab();
				return TRUE;
			}

			if (!(NewStr=(wchar_t *)xf_realloc(Str,(StrSize+1)*sizeof(wchar_t))))
				return TRUE;

			Str=NewStr;

			if (!Flags.Check(FEDITLINE_OVERTYPE))
			{
				wmemmove(Str+CurPos+1,Str+CurPos,StrSize-CurPos);

				if (SelStart!=-1)
				{
					if (SelEnd!=-1 && CurPos<SelEnd)
						SelEnd++;

					if (CurPos<SelStart)
						SelStart++;
				}
			}

			PrevCurPos=CurPos;
			Str[CurPos++]=Key;
			changed=true;
		}
		else if (Flags.Check(FEDITLINE_OVERTYPE))
		{
			if (CurPos < StrSize)
			{
				PrevCurPos=CurPos;
				Str[CurPos++]=Key;
				changed=true;
			}
		}
		else
			MessageBeep(MB_ICONHAND);
	}

	Str[StrSize]=0;

	if (changed) Changed();

	return TRUE;
}

void Edit::SetObjectColor(PaletteColors Color,PaletteColors SelColor)
{
	this->Color=ColorIndexToColor(Color);
	this->SelColor=ColorIndexToColor(SelColor);
}

void Edit::SetObjectColor(const FarColor& Color,const FarColor& SelColor)
{
	this->Color=Color;
	this->SelColor=SelColor;
}

void Edit::GetString(wchar_t *Str,int MaxSize)
{
	//xwcsncpy(Str, this->Str,MaxSize);
	wmemmove(Str,this->Str,Min(StrSize,MaxSize-1));
	Str[Min(StrSize,MaxSize-1)]=0;
	Str[MaxSize-1]=0;
}

void Edit::GetString(string &strStr)
{
	strStr = Str;
}


const wchar_t* Edit::GetStringAddr()
{
	return Str;
}



void  Edit::SetHiString(const wchar_t *Str)
{
	if (Flags.Check(FEDITLINE_READONLY))
		return;

	string NewStr;
	HiText2Str(NewStr, Str);
	Select(-1,0);
	SetBinaryString(NewStr, static_cast<int>(NewStr.GetLength()));
}

void Edit::SetString(const wchar_t *Str, int Length)
{
	if (Flags.Check(FEDITLINE_READONLY))
		return;

	Select(-1,0);
	SetBinaryString(Str,Length==-1?(int)StrLength(Str):Length);
}

void Edit::SetEOL(const wchar_t *EOL)
{
	EndType=EOL_NONE;

	if (EOL && *EOL)
	{
		if (EOL[0]==L'\r')
			if (EOL[1]==L'\n')
				EndType=EOL_CRLF;
			else if (EOL[1]==L'\r' && EOL[2]==L'\n')
				EndType=EOL_CRCRLF;
			else
				EndType=EOL_CR;
		else if (EOL[0]==L'\n')
			EndType=EOL_LF;
	}
}

const wchar_t *Edit::GetEOL()
{
	return EOL_TYPE_CHARS[EndType];
}

/* $ 25.07.2000 tran
   ����������:
   � ���� ������ DropDownBox �� ��������������
   ��� �� ���������� ������ �� SetString � �� ������ Editor
   � Dialog �� ����� �� ���������� */
void Edit::SetBinaryString(const wchar_t *Str,int Length)
{
	if (Flags.Check(FEDITLINE_READONLY))
		return;

	// ��������� ������������ �������, ���� ��������� MaxLength
	if (MaxLength != -1 && Length > MaxLength)
	{
		Length=MaxLength; // ??
	}

	if (Length>0 && !Flags.Check(FEDITLINE_PARENT_SINGLELINE))
	{
		if (Str[Length-1]==L'\r')
		{
			EndType=EOL_CR;
			Length--;
		}
		else
		{
			if (Str[Length-1]==L'\n')
			{
				Length--;

				if (Length > 0 && Str[Length-1]==L'\r')
				{
					Length--;

					if (Length > 0 && Str[Length-1]==L'\r')
					{
						Length--;
						EndType=EOL_CRCRLF;
					}
					else
						EndType=EOL_CRLF;
				}
				else
					EndType=EOL_LF;
			}
			else
				EndType=EOL_NONE;
		}
	}

	CurPos=0;

	if (Mask && *Mask)
	{
		RefreshStrByMask(TRUE);
		int maskLen=StrLength(Mask);

		for (int i=0,j=0; j<maskLen && j<Length;)
		{
			if (CheckCharMask(Mask[i]))
			{
				int goLoop=FALSE;

				if (KeyMatchedMask(Str[j]))
					InsertKey(Str[j]);
				else
					goLoop=TRUE;

				j++;

				if (goLoop) continue;
			}
			else
			{
				PrevCurPos=CurPos;
				CurPos++;
			}

			i++;
		}

		/* ����� ���������� ������� (!*Str), �.�. ��� ������� ������
		   ������ �������� ����� ����� SetBinaryString("",0)
		   �.�. ����� ������� �� ���������� "�������������" ������ � ������
		*/
		RefreshStrByMask(!*Str);
	}
	else
	{
		wchar_t *NewStr=(wchar_t *)xf_realloc_nomove(this->Str,(Length+1)*sizeof(wchar_t));

		if (!NewStr)
			return;

		this->Str=NewStr;
		StrSize=Length;
		wmemcpy(this->Str,Str,Length);
		this->Str[Length]=0;

		if (TabExpandMode == EXPAND_ALLTABS)
			ReplaceTabs();

		PrevCurPos=CurPos;
		CurPos=StrSize;
	}

	Changed();
}

void Edit::GetBinaryString(const wchar_t **Str,const wchar_t **EOL,int &Length)
{
	*Str=this->Str;

	if (EOL)
		*EOL=EOL_TYPE_CHARS[EndType];

	Length=StrSize; //???
}

int Edit::GetSelString(wchar_t *Str, int MaxSize)
{
	if (SelStart==-1 || (SelEnd!=-1 && SelEnd<=SelStart) ||
	        SelStart>=StrSize)
	{
		*Str=0;
		return FALSE;
	}

	int CopyLength;

	if (SelEnd==-1)
		CopyLength=MaxSize;
	else
		CopyLength=Min(MaxSize,SelEnd-SelStart+1);

	xwcsncpy(Str,this->Str+SelStart,CopyLength);
	return TRUE;
}

int Edit::GetSelString(string &strStr)
{
	if (SelStart==-1 || (SelEnd!=-1 && SelEnd<=SelStart) ||
	        SelStart>=StrSize)
	{
		strStr.Clear();
		return FALSE;
	}

	int CopyLength;
	CopyLength=SelEnd-SelStart+1;
	wchar_t *lpwszStr = strStr.GetBuffer(CopyLength+1);
	xwcsncpy(lpwszStr,this->Str+SelStart,CopyLength);
	strStr.ReleaseBuffer();
	return TRUE;
}



void Edit::AppendString(const wchar_t *Str)
{
	int LastPos = CurPos;
	CurPos = GetLength();
	InsertString(Str);
	CurPos = LastPos;
}

void Edit::InsertString(const wchar_t *Str)
{
	if (Flags.Check(FEDITLINE_READONLY|FEDITLINE_DROPDOWNBOX))
		return;

	if (!Flags.Check(FEDITLINE_PERSISTENTBLOCKS))
		DeleteBlock();

	InsertBinaryString(Str,StrLength(Str));
}


void Edit::InsertBinaryString(const wchar_t *Str,int Length)
{
	wchar_t *NewStr;

	if (Flags.Check(FEDITLINE_READONLY|FEDITLINE_DROPDOWNBOX))
		return;

	Flags.Clear(FEDITLINE_CLEARFLAG);

	if (Mask && *Mask)
	{
		int Pos=CurPos;
		int MaskLen=StrLength(Mask);

		if (Pos<MaskLen)
		{
			//_SVS(SysLog(L"InsertBinaryString ==> Str='%s' (Length=%d) Mask='%s'",Str,Length,Mask+Pos));
			int StrLen=(MaskLen-Pos>Length)?Length:MaskLen-Pos;

			/* $ 15.11.2000 KM
			   ������� ����������� ��� ���������� ������ PasteFromClipboard
			   � ������ � ������
			*/
			for (int i=Pos,j=0; j<StrLen+Pos;)
			{
				if (CheckCharMask(Mask[i]))
				{
					int goLoop=FALSE;

					if (j < Length && KeyMatchedMask(Str[j]))
					{
						InsertKey(Str[j]);
						//_SVS(SysLog(L"InsertBinaryString ==> InsertKey(Str[%d]='%c');",j,Str[j]));
					}
					else
						goLoop=TRUE;

					j++;

					if (goLoop) continue;
				}
				else
				{
					if(Mask[j] == Str[j])
					{
						j++;
					}
					PrevCurPos=CurPos;
					CurPos++;
				}

				i++;
			}
		}

		RefreshStrByMask();
		//_SVS(SysLog(L"InsertBinaryString ==> this->Str='%s'",this->Str));
	}
	else
	{
		if (MaxLength != -1 && StrSize+Length > MaxLength)
		{
			// ��������� ������������ �������, ���� ��������� MaxLength
			if (StrSize < MaxLength)
			{
				Length=MaxLength-StrSize;
			}
		}

		if (MaxLength == -1 || StrSize+Length <= MaxLength)
		{
			if (CurPos>StrSize)
			{
				if (!(NewStr=(wchar_t *)xf_realloc(this->Str,(CurPos+1)*sizeof(wchar_t))))
					return;

				this->Str=NewStr;
				_snwprintf(&this->Str[StrSize],CurPos+1,L"%*s",CurPos-StrSize,L"");
				//memset(this->Str+StrSize,' ',CurPos-StrSize);this->Str[CurPos+1]=0;
				StrSize=CurPos;
			}

			int TmpSize=StrSize-CurPos;
			wchar_t *TmpStr=new wchar_t[TmpSize+16];

			if (!TmpStr)
				return;

			wmemcpy(TmpStr,&this->Str[CurPos],TmpSize);
			StrSize+=Length;

			if (!(NewStr=(wchar_t *)xf_realloc(this->Str,(StrSize+1)*sizeof(wchar_t))))
			{
				delete[] TmpStr;
				return;
			}

			this->Str=NewStr;
			wmemcpy(&this->Str[CurPos],Str,Length);
			PrevCurPos=CurPos;
			CurPos+=Length;
			wmemcpy(this->Str+CurPos,TmpStr,TmpSize);
			this->Str[StrSize]=0;
			delete[] TmpStr;

			if (TabExpandMode == EXPAND_ALLTABS)
				ReplaceTabs();

			Changed();
		}
		else
			MessageBeep(MB_ICONHAND);
	}
}


int Edit::GetLength()
{
	return(StrSize);
}


// ������� ��������� ����� ����� � ������ Edit
void Edit::SetInputMask(const wchar_t *InputMask)
{
	if (Mask)
		xf_free(Mask);

	if (InputMask && *InputMask)
	{
		if (!(Mask=xf_wcsdup(InputMask)))
			return;

		RefreshStrByMask(TRUE);
	}
	else
		Mask=nullptr;
}


// ������� ���������� ��������� ������ ����� �� ����������� Mask
void Edit::RefreshStrByMask(int InitMode)
{
	if (Mask && *Mask)
	{
		int MaskLen=StrLength(Mask);

		if (StrSize!=MaskLen)
		{
			wchar_t *NewStr=(wchar_t *)xf_realloc(Str,(MaskLen+1)*sizeof(wchar_t));

			if (!NewStr)
				return;

			Str=NewStr;

			for (int i=StrSize; i<MaskLen; i++)
				Str[i]=L' ';

			StrSize=MaxLength=MaskLen;
			Str[StrSize]=0;
		}

		for (int i=0; i<MaskLen; i++)
		{
			if (InitMode)
				Str[i]=L' ';

			if (!CheckCharMask(Mask[i]))
				Str[i]=Mask[i];
		}
	}
}


int Edit::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
	if (!(MouseEvent->dwButtonState & 3))
		return FALSE;

	if (MouseEvent->dwMousePosition.X<X1 || MouseEvent->dwMousePosition.X>X2 ||
	        MouseEvent->dwMousePosition.Y!=Y1)
		return FALSE;

	//SetClearFlag(0); // ����� ������ ��� ��������� � ������ �����-������?
	SetTabCurPos(MouseEvent->dwMousePosition.X - X1 + LeftPos);

	if (!Flags.Check(FEDITLINE_PERSISTENTBLOCKS))
		Select(-1,0);

	if (MouseEvent->dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED)
	{
		static int PrevDoubleClick=0;
		static COORD PrevPosition={};

		if (GetTickCount()-PrevDoubleClick<=GetDoubleClickTime() && MouseEvent->dwEventFlags!=MOUSE_MOVED &&
		        PrevPosition.X == MouseEvent->dwMousePosition.X && PrevPosition.Y == MouseEvent->dwMousePosition.Y)
		{
			Select(0,StrSize);
			PrevDoubleClick=0;
			PrevPosition.X=0;
			PrevPosition.Y=0;
		}

		if (MouseEvent->dwEventFlags==DOUBLE_CLICK)
		{
			ProcessKey(KEY_OP_SELWORD);
			PrevDoubleClick=GetTickCount();
			PrevPosition=MouseEvent->dwMousePosition;
		}
		else
		{
			PrevDoubleClick=0;
			PrevPosition.X=0;
			PrevPosition.Y=0;
		}
	}

	Show();
	return TRUE;
}


/* $ 03.08.2000 KM
   ������� ������ �������� ��-�� �������������
   ���������� ������ ����� ����.
*/
int Edit::Search(const string& Str,string& ReplaceStr,int Position,int Case,int WholeWords,int Reverse,int Regexp, int *SearchLength)
{
	*SearchLength = 0;

	if (Reverse)
	{
		Position--;

		if (Position>=StrSize)
			Position=StrSize-1;

		if (Position<0)
			return FALSE;
	}

	if ((Position<StrSize || (!Position && !StrSize)) && !Str.IsEmpty())
	{
		if (Regexp)
		{
			string strSlash(Str);
			InsertRegexpQuote(strSlash);
			RegExp re;
			// Q: ��� ������: ����� ������� ��� ����� RegExp`�?
			if (!re.Compile(strSlash, OP_PERLSTYLE|OP_OPTIMIZE|(!Case?OP_IGNORECASE:0)))
				return FALSE;

			SMatch m[10*2], *pm = m;
			int n = re.GetBracketsCount();
			if (n > static_cast<int>(ARRAYSIZE(m)/2))
			{
				pm = (SMatch *)xf_malloc(2*n*sizeof(SMatch));
				if (!pm)
					return FALSE;
			}

			int found = FALSE;
			int pos, half = 0;

			if (!Reverse)
			{
				if (re.SearchEx(this->Str,this->Str+Position,this->Str+StrSize,pm,n))
					found = TRUE;
			}
			else
			{
				pos = 0;
				for (;;)
				{
					if (!re.SearchEx(this->Str,this->Str+pos,this->Str+StrSize,pm+half,n))
						break;
					pos = static_cast<int>(pm[half].start);
					if (pos > Position)
						break;

					found = TRUE;
					++pos;
					half = n - half;
				}
				half = n - half;
			}
			if (found)
			{
				*SearchLength = pm[half].end - pm[half].start;
				CurPos = pm[half].start;
				ReplaceStr=ReplaceBrackets(this->Str,ReplaceStr,pm+half,n);
			}
			if (pm != m)
				xf_free(pm);

			return found;
		}

		if (Position==StrSize) return FALSE;

		int Length = *SearchLength = (int)Str.GetLength();

		for (int I=Position; (Reverse && I>=0) || (!Reverse && I<StrSize); Reverse ? I--:I++)
		{
			for (int J=0;; J++)
			{
				if (!Str[J])
				{
					CurPos=I;
					return TRUE;
				}

				if (WholeWords)
				{
					wchar_t ChLeft,ChRight;
					int locResultLeft=FALSE;
					int locResultRight=FALSE;
					ChLeft=this->Str[I-1];

					if (I>0)
						locResultLeft=(IsSpace(ChLeft) || wcschr(WordDiv(),ChLeft));
					else
						locResultLeft=TRUE;

					if (I+Length<StrSize)
					{
						ChRight=this->Str[I+Length];
						locResultRight=(IsSpace(ChRight) || wcschr(WordDiv(),ChRight));
					}
					else
					{
						locResultRight=TRUE;
					}

					if (!locResultLeft || !locResultRight)
						break;
				}

				wchar_t Ch=this->Str[I+J];

				if (Case)
				{
					if (Ch!=Str[J])
						break;
				}
				else
				{
					if (Upper(Ch)!=Upper(Str[J]))
						break;
				}
			}
		}
	}

	return FALSE;
}

void Edit::InsertTab()
{
	wchar_t *TabPtr;
	int Pos,S;

	if (Flags.Check(FEDITLINE_READONLY))
		return;

	Pos=CurPos;
	S=TabSize-(Pos % TabSize);

	if (SelStart!=-1)
	{
		if (Pos<=SelStart)
		{
			SelStart+=S-(Pos==SelStart?0:1);
		}

		if (SelEnd!=-1 && Pos<SelEnd)
		{
			SelEnd+=S;
		}
	}

	int PrevStrSize=StrSize;
	StrSize+=S;
	CurPos+=S;
	Str=(wchar_t *)xf_realloc(Str,(StrSize+1)*sizeof(wchar_t));
	TabPtr=Str+Pos;
	wmemmove(TabPtr+S,TabPtr,PrevStrSize-Pos);
	wmemset(TabPtr,L' ',S);
	Str[StrSize]=0;
	Changed();
}


bool Edit::ReplaceTabs()
{
	wchar_t *TabPtr;
	int Pos=0,S;

	if (Flags.Check(FEDITLINE_READONLY))
		return false;

	bool changed=false;

	while ((TabPtr=(wchar_t *)wmemchr(Str+Pos,L'\t',StrSize-Pos)))
	{
		changed=true;
		Pos=(int)(TabPtr-Str);
		S=TabSize-((int)(TabPtr-Str) % TabSize);

		if (SelStart!=-1)
		{
			if (Pos<=SelStart)
			{
				SelStart+=S-(Pos==SelStart?0:1);
			}

			if (SelEnd!=-1 && Pos<SelEnd)
			{
				SelEnd+=S-1;
			}
		}

		int PrevStrSize=StrSize;
		StrSize+=S-1;

		if (CurPos>Pos)
			CurPos+=S-1;

		Str=(wchar_t *)xf_realloc(Str,(StrSize+1)*sizeof(wchar_t));
		TabPtr=Str+Pos;
		wmemmove(TabPtr+S,TabPtr+1,PrevStrSize-Pos);
		wmemset(TabPtr,L' ',S);
		Str[StrSize]=0;
	}

	if (changed) Changed();
	return changed;
}


int Edit::GetTabCurPos()
{
	return(RealPosToTab(CurPos));
}


void Edit::SetTabCurPos(int NewPos)
{
	int Pos;

	if (Mask && *Mask)
	{
		wchar_t *ShortStr=new wchar_t[StrSize+1];

		if (!ShortStr)
			return;

		xwcsncpy(ShortStr,Str,StrSize+1);
		Pos=StrLength(RemoveTrailingSpaces(ShortStr));
		delete[] ShortStr;

		if (NewPos>Pos)
			NewPos=Pos;
	}

	CurPos=TabPosToReal(NewPos);
}


int Edit::RealPosToTab(int Pos)
{
	return RealPosToTab(0, 0, Pos, nullptr);
}


int Edit::RealPosToTab(int PrevLength, int PrevPos, int Pos, int* CorrectPos)
{
	// ������������� �����
	bool bCorrectPos = CorrectPos && *CorrectPos;

	if (CorrectPos)
		*CorrectPos = 0;

	// ���� � ��� ��� ���� ������������� � �������, �� ������ ��������� ����������
	if (TabExpandMode == EXPAND_ALLTABS)
		return PrevLength+Pos-PrevPos;

	// ������������� �������������� ����� ���������� ���������
	int TabPos = PrevLength;

	// ���� ���������� ������� �� ������ ������, �� ����� ��� ����� ��� �
	// ��������� ����� ������ �� ����, ����� ���������� ����������
	if (PrevPos >= StrSize)
		TabPos += Pos-PrevPos;
	else
	{
		// �������� ���������� � ���������� �������
		int Index = PrevPos;

		// �������� �� ���� �������� �� ������� ������, ���� ��� ��� � �������� ������,
		// ���� �� ����� ������, ���� ������� ������ �� ��������� ������
		for (; Index < Min(Pos, StrSize); Index++)

			// ������������ ����
			if (Str[Index] == L'\t')
			{
				// ���� ���� ������������� ������ ������������� ����� � ��� ������������
				// ��� �� �����������, �� ����������� ����� �������������� ������ �� �������
				if (bCorrectPos)
				{
					++Pos;
					*CorrectPos = 1;
					bCorrectPos = false;
				}

				// ����������� ����� ���� � ������ �������� � ������� ������� � ������
				TabPos += TabSize-(TabPos%TabSize);
			}
		// ������������ ��� ��������� �������
			else
				TabPos++;

		// ���� ������� ��������� �� ��������� ������, �� ��� ����� ��� ����� � �� ������
		if (Pos >= StrSize)
			TabPos += Pos-Index;
	}

	return TabPos;
}


int Edit::TabPosToReal(int Pos)
{
	if (TabExpandMode == EXPAND_ALLTABS)
		return Pos;

	int Index = 0;

	for (int TabPos = 0; TabPos < Pos; Index++)
	{
		if (Index > StrSize)
		{
			Index += Pos-TabPos;
			break;
		}

		if (Str[Index] == L'\t')
		{
			int NewTabPos = TabPos+TabSize-(TabPos%TabSize);

			if (NewTabPos > Pos)
				break;

			TabPos = NewTabPos;
		}
		else
		{
			TabPos++;
		}
	}

	return Index;
}


void Edit::Select(int Start,int End)
{
	SelStart=Start;
	SelEnd=End;

	/* $ 24.06.2002 SKV
	   ���� ������ ��������� �� ������ ������, ���� ��������� �����.
	   17.09.2002 ��������� �������. ���������.
	*/
	if (SelEnd<SelStart && SelEnd!=-1)
	{
		SelStart=-1;
		SelEnd=0;
	}

	if (SelStart==-1 && SelEnd==-1)
	{
		SelStart=-1;
		SelEnd=0;
	}

//  if (SelEnd>StrSize)
//    SelEnd=StrSize;
}

void Edit::AddSelect(int Start,int End)
{
	if (Start<SelStart || SelStart==-1)
		SelStart=Start;

	if (End==-1 || (End>SelEnd && SelEnd!=-1))
		SelEnd=End;

	if (SelEnd>StrSize)
		SelEnd=StrSize;

	if (SelEnd<SelStart && SelEnd!=-1)
	{
		SelStart=-1;
		SelEnd=0;
	}
}


void Edit::GetSelection(int &Start,int &End)
{
	/* $ 17.09.2002 SKV
	  ���� ����, ��� ��� ��������� ������ OO design'�,
	  ��� ��� ��� � �������� �����.
	*/
	/*  if (SelEnd>StrSize+1)
	    SelEnd=StrSize+1;
	  if (SelStart>StrSize+1)
	    SelStart=StrSize+1;*/
	/* SKV $ */
	Start=SelStart;
	End=SelEnd;

	if (End>StrSize)
		End=-1;//StrSize;

	if (Start>StrSize)
		Start=StrSize;
}


void Edit::GetRealSelection(int &Start,int &End)
{
	Start=SelStart;
	End=SelEnd;
}


void Edit::DeleteBlock()
{
	if (Flags.Check(FEDITLINE_READONLY|FEDITLINE_DROPDOWNBOX))
		return;

	if (SelStart==-1 || SelStart>=SelEnd)
		return;

	PrevCurPos=CurPos;

	if (Mask && *Mask)
	{
		for (int i=SelStart; i<SelEnd; i++)
		{
			if (CheckCharMask(Mask[i]))
				Str[i]=L' ';
		}

		CurPos=SelStart;
	}
	else
	{
		int From=SelStart,To=SelEnd;

		if (From>StrSize)From=StrSize;

		if (To>StrSize)To=StrSize;

		wmemmove(Str+From,Str+To,StrSize-To+1);
		StrSize-=To-From;

		if (CurPos>From)
		{
			if (CurPos<To)
				CurPos=From;
			else
				CurPos-=To-From;
		}

		Str=(wchar_t *)xf_realloc(Str,(StrSize+1)*sizeof(wchar_t));
	}

	SelStart=-1;
	SelEnd=0;
	Flags.Clear(FEDITLINE_MARKINGBLOCK);

	// OT: �������� �� ������������ �������� ������ ��� �������� � �������
	if (Flags.Check((FEDITLINE_PARENT_SINGLELINE|FEDITLINE_PARENT_MULTILINE)))
	{
		if (LeftPos>CurPos)
			LeftPos=CurPos;
	}

	Changed(true);
}

static int _cdecl SortColors(const void *el1,const void *el2)
{
	ColorItem *item1=(ColorItem *)el1,*item2=(ColorItem *)el2;
	if (item1->Priority > item2->Priority)
		return 1;
	if (item1->Priority < item2->Priority)
		return -1;
	return item1->SubPriority - item2->SubPriority;
}

void Edit::AddColor(ColorItem *col,bool skipsort)
{
	if (ColorCount==MaxColorCount)
	{
		if (ColorCount<256)
		{
			if (!(ColorCount & 15))
			{
				MaxColorCount=ColorCount+16;
				ColorList=(ColorItem *)xf_realloc(ColorList,(MaxColorCount)*sizeof(*ColorList));
			}
		}
		else if (ColorCount<2048)
		{
			if (!(ColorCount & 255))
			{
				MaxColorCount=ColorCount+256;
				ColorList=(ColorItem *)xf_realloc(ColorList,(MaxColorCount)*sizeof(*ColorList));
			}
		}
		else if (ColorCount<65536)
		{
			if (!(ColorCount & 2047))
			{
				MaxColorCount=ColorCount+2048;
				ColorList=(ColorItem *)xf_realloc(ColorList,(MaxColorCount)*sizeof(*ColorList));
			}
		}
		else if (!(ColorCount & 65535))
		{
			MaxColorCount=ColorCount+65536;
			ColorList=(ColorItem *)xf_realloc(ColorList,(MaxColorCount)*sizeof(*ColorList));
		}
	}

	#ifdef _DEBUG
	//Maximus: ��� �������
	_ASSERTE(ColorCount<MaxColorCount);
	#endif

	if (skipsort && !ColorListNeedSort && ColorCount && ColorList[ColorCount-1].Priority>col->Priority)
		ColorListNeedSort=true;


	ColorList[ColorCount++]=*col;

	if (!skipsort)
	{
		for(int ii=0;ii<ColorCount;++ii) ColorList[ii].SubPriority=ii;
		far_qsort(ColorList,ColorCount,sizeof(*ColorList),SortColors);
	}
}

void Edit::SortColorUnlocked()
{
	if (ColorListNeedFree)
	{
		ColorListNeedFree=false;
		if (!ColorCount)
		{
			xf_free(ColorList);
			ColorList=nullptr;
			MaxColorCount = 0;
		}
	}

	if (ColorListNeedSort)
	{
		ColorListNeedSort=false;
		for(int ii=0;ii<ColorCount;++ii) ColorList[ii].SubPriority=ii;
		far_qsort(ColorList,ColorCount,sizeof(*ColorList),SortColors);
	}
}

int Edit::DeleteColor(int ColorPos,const GUID& Owner,bool skipfree)
{
	int Src;

	if (!ColorCount)
		return FALSE;

	int Dest=0;

	if (ColorPos!=-1)
	{
		for (Src=0; Src<ColorCount; Src++)
		{
			if ((ColorList[Src].StartPos!=ColorPos) || (!IsEqualGUID(Owner,ColorList[Src].Owner)))
			{
				if (Dest!=Src)
					ColorList[Dest]=ColorList[Src];

				Dest++;
			}
		}
	}
	else
	{
		for (Src=0; Src<ColorCount; Src++)
		{
			if (!IsEqualGUID(Owner,ColorList[Src].Owner))
			{
				if (Dest!=Src)
					ColorList[Dest]=ColorList[Src];

				Dest++;
			}
		}
	}

	int DelCount=ColorCount-Dest;
	ColorCount=Dest;

	if (!ColorCount)
	{
		if (skipfree)
		{
			ColorListNeedFree=true;
		}
		else
		{
			xf_free(ColorList);
			ColorList=nullptr;
			MaxColorCount = 0;
		}
	}

	return(DelCount);
}


int Edit::GetColor(ColorItem *col,int Item)
{
	if (Item >= ColorCount)
		return FALSE;

	*col=ColorList[Item];
	return TRUE;
}


void Edit::ApplyColor()
{
	// ��� ����������� ��������� ����������� ������� ����� ���������� �����
	int Pos = INT_MIN, TabPos = INT_MIN, TabEditorPos = INT_MIN;

	int XPos = 0;
	if(Flags.Check(FEDITLINE_EDITORMODE))
	{
		EditorInfo ei={};
		CtrlObject->Plugins->CurEditor->EditorControl(ECTL_GETINFO, &ei);
		XPos = ei.CurTabPos - ei.LeftPos;
	}

	// ������������ �������� ��������
	for (int Col = 0; Col < ColorCount; Col++)
	{
		ColorItem *CurItem = ColorList+Col;

		// ���������� �������� � ������� ������ ������ �����
		if (CurItem->StartPos > CurItem->EndPos)
			continue;

		// �������� �������� �������� �� ���������� �� �����
		if (CurItem->StartPos-LeftPos > X2 && CurItem->EndPos-LeftPos < X1)
			continue;

		int Length = CurItem->EndPos-CurItem->StartPos+1;

		if (CurItem->StartPos+Length >= StrSize)
			Length = StrSize-CurItem->StartPos;

		// �������� ��������� �������
		int RealStart, Start;

		// ���� ���������� ������� ����� �������, �� ������ �� ���������
		// � ����� ���� ����� ����������� ��������
		if (Pos == CurItem->StartPos)
		{
			RealStart = TabPos;
			Start = TabEditorPos;
		}
		// ���� ���������� ��� ������ ��� ��� ���������� ������� ������ �������,
		// �� ���������� ���������� � ������ ������
		else if (Pos == INT_MIN || CurItem->StartPos < Pos)
		{
			RealStart = RealPosToTab(CurItem->StartPos);
			Start = RealStart-LeftPos;
		}
		// ��� ������������ ������ ���������� ������������ ���������� �������
		else
		{
			RealStart = RealPosToTab(TabPos, Pos, CurItem->StartPos, nullptr);
			Start = RealStart-LeftPos;
		}

		// ���������� ����������� �������� ��� �� ����������� ���������� �������������
		Pos = CurItem->StartPos;
		TabPos = RealStart;
		TabEditorPos = Start;

		// ���������� �������� ��������� � ������� ��������� ������� �� �������
		if (Start > X2)
			continue;

		// ������������� ������������ ����� (�����������, ���� ���������� ���� ECF_TABMARKFIRST)
		int CorrectPos = CurItem->Flags & ECF_TABMARKFIRST ? 0 : 1;

		// �������� �������� �������
		int EndPos = CurItem->EndPos;
		int RealEnd, End;

		bool TabMarkCurrent=false;

		// ������������ ������, ����� ���������� ������� ����� �������, �� ����
		// ����� �������������� ������� ����� 1
		if (Pos == EndPos)
		{
			// ���� ���������� ������ ������������ ������������ ����� � ������������
			// ������ ������ -- ��� ���, �� ������ ������ � ����� �������������,
			// ����� ������ �� ��������� � ���� ������ ��������
			if (CorrectPos && EndPos < StrSize && Str[EndPos] == L'\t')
			{
				RealEnd = RealPosToTab(TabPos, Pos, ++EndPos, nullptr);
				End = RealEnd-LeftPos;
				TabMarkCurrent = (CurItem->Flags & ECF_TABMARKCURRENT) && XPos>=Start && XPos<End;
			}
			else
			{
				RealEnd = TabPos;
				CorrectPos = 0;
				End = TabEditorPos;
			}
		}
		// ���� ���������� ������� ������ �������, �� ���������� ����������
		// � ������ ������ (� ������ ������������� ������������ �����)
		else if (EndPos < Pos)
		{
			// TODO: �������� ��� �� ����� ��������� � ������ ����� (�� ������� Mantis#0001718)
			RealEnd = RealPosToTab(0, 0, EndPos, &CorrectPos);
			EndPos += CorrectPos;
			End = RealEnd-LeftPos;
		}
		// ��� ������������ ������ ���������� ������������ ���������� ������� (� ������
		// ������������� ������������ �����)
		else
		{
			// Mantis#0001718: ���������� ECF_TABMARKFIRST �� ������ ��������� ������������
			// ��������� � ������ ���������� ����
			if (CorrectPos && EndPos < StrSize && Str[EndPos] == L'\t')
				RealEnd = RealPosToTab(TabPos, Pos, ++EndPos, nullptr);
			else
			{
				RealEnd = RealPosToTab(TabPos, Pos, EndPos, &CorrectPos);
				EndPos += CorrectPos;
			}
			End = RealEnd-LeftPos;
		}

		// ���������� ����������� �������� ��� �� ����������� ���������� �������������
		Pos = EndPos;
		TabPos = RealEnd;
		TabEditorPos = End;

		// ���������� �������� ��������� � ������� �������� ������� ������ ����� ������� ������
		if (End < X1)
			continue;

		// �������� ��������� �������� �� ������
		if (Start < X1)
			Start = X1;

		if (End > X2)
			End = X2;

		if(TabMarkCurrent)
		{
			Start = XPos;
			End = XPos+1;
		}

		// ������������� ����� ��������������� ��������
		Length = End-Start+1;

		if (Length < X2)
			Length -= CorrectPos;

		// ������������ �������, ���� ���� ��� ������������
		if (Length > 0)
		{
			ScrBuf.ApplyColor(
			    Start,
			    Y1,
			    Start+Length-1,
			    Y1,
			    CurItem->Color,
			    // �� ������������ ���������
			    SelColor,
			    true
			);
		}
	}
}

/* $ 24.09.2000 SVS $
  ������� Xlat - ������������� �� �������� QWERTY <-> ������
*/
void Edit::Xlat(bool All)
{
	//   ��� CmdLine - ���� ��� ���������, ����������� ��� ������
	if (All && SelStart == -1 && !SelEnd)
	{
		::Xlat(Str,0,StrLength(Str),Opt.XLat.Flags);
		Changed();
		Show();
		return;
	}

	if (SelStart != -1 && SelStart != SelEnd)
	{
		if (SelEnd == -1)
			SelEnd=StrLength(Str);

		::Xlat(Str,SelStart,SelEnd,Opt.XLat.Flags);
		Changed();
		Show();
	}
	/* $ 25.11.2000 IS
	 ���� ��� ���������, �� ���������� ������� �����. ����� ������������ ��
	 ������ ����������� ������ ������������.
	*/
	else
	{
		/* $ 10.12.2000 IS
		   ������������ ������ �� �����, �� ������� ����� ������, ��� �� �����, ���
		   ��������� ����� ������� ������� �� 1 ������
		*/
		int start=CurPos, end, StrSize=StrLength(Str);
		bool DoXlat=true;

		if (IsWordDiv(Opt.XLat.strWordDivForXlat,Str[start]))
		{
			if (start) start--;

			DoXlat=(!IsWordDiv(Opt.XLat.strWordDivForXlat,Str[start]));
		}

		if (DoXlat)
		{
			while (start>=0 && !IsWordDiv(Opt.XLat.strWordDivForXlat,Str[start]))
				start--;

			start++;
			end=start+1;

			while (end<StrSize && !IsWordDiv(Opt.XLat.strWordDivForXlat,Str[end]))
				end++;

			::Xlat(Str,start,end,Opt.XLat.Flags);
			Changed();
			Show();
		}
	}
}


/* $ 15.11.2000 KM
   ���������: �������� �� ������ � �����������
   �������� ��������, ������������ ������
*/
int Edit::KeyMatchedMask(int Key)
{
	int Inserted=FALSE;

	if (Mask[CurPos]==EDMASK_ANY)
		Inserted=TRUE;
	else if (Mask[CurPos]==EDMASK_DSS && (iswdigit(Key) || Key==L' ' || Key==L'-'))
		Inserted=TRUE;
	else if (Mask[CurPos]==EDMASK_DIGITS && (iswdigit(Key) || Key==L' '))
		Inserted=TRUE;
	else if (Mask[CurPos]==EDMASK_DIGIT && (iswdigit(Key)))
		Inserted=TRUE;
	else if (Mask[CurPos]==EDMASK_ALPHA && IsAlpha(Key))
		Inserted=TRUE;
	else if (Mask[CurPos]==EDMASK_HEX && (iswdigit(Key) || (Upper(Key)>=L'A' && Upper(Key)<=L'F') || (Upper(Key)>=L'a' && Upper(Key)<=L'f')))
		Inserted=TRUE;

	return Inserted;
}

int Edit::CheckCharMask(wchar_t Chr)
{
	return (Chr==EDMASK_ANY || Chr==EDMASK_DIGIT || Chr==EDMASK_DIGITS || Chr==EDMASK_DSS || Chr==EDMASK_ALPHA || Chr==EDMASK_HEX)?TRUE:FALSE;
}

void Edit::SetDialogParent(DWORD Sets)
{
	if ((Sets&(FEDITLINE_PARENT_SINGLELINE|FEDITLINE_PARENT_MULTILINE)) == (FEDITLINE_PARENT_SINGLELINE|FEDITLINE_PARENT_MULTILINE) ||
	        !(Sets&(FEDITLINE_PARENT_SINGLELINE|FEDITLINE_PARENT_MULTILINE)))
		Flags.Clear(FEDITLINE_PARENT_SINGLELINE|FEDITLINE_PARENT_MULTILINE);
	else if (Sets&FEDITLINE_PARENT_SINGLELINE)
	{
		Flags.Clear(FEDITLINE_PARENT_MULTILINE);
		Flags.Set(FEDITLINE_PARENT_SINGLELINE);
	}
	else if (Sets&FEDITLINE_PARENT_MULTILINE)
	{
		Flags.Clear(FEDITLINE_PARENT_SINGLELINE);
		Flags.Set(FEDITLINE_PARENT_MULTILINE);
	}
}

void Edit::FixLeftPos(int TabCurPos)
{
	if (TabCurPos<0) TabCurPos=GetTabCurPos(); //�����������, ����� ��� ���� �� ������
	if (TabCurPos-LeftPos>ObjWidth-1)
		LeftPos=TabCurPos-ObjWidth+1;

	if (TabCurPos<LeftPos)
		LeftPos=TabCurPos;
}

EditControl::EditControl(ScreenObject *pOwner,Callback* aCallback,bool bAllocateData,History* iHistory,FarList* iList,DWORD iFlags):Edit(pOwner,bAllocateData)
{
	if (aCallback)
	{
		m_Callback=*aCallback;
	}
	else
	{
		m_Callback.Active=true;
		m_Callback.m_Callback=nullptr;
		m_Callback.m_Param=nullptr;
	}

	MacroAreaAC=MACRO_DIALOGAUTOCOMPLETION;

	ECFlags=iFlags;
	pHistory=iHistory;
	pList=iList;
	Selection=false;
	SelectionStart=-1;
	ACState=ECFlags.Check(EC_ENABLEAUTOCOMPLETE)!=FALSE;
	ColorUnChanged = ColorIndexToColor(COL_DIALOGEDITUNCHANGED);
}

void EditControl::Show()
{
	if(X2-X1+1>StrSize)
	{
		SetLeftPos(0);
	}
	if (pOwner->IsVisible())
	{
		Edit::Show();
	}
}

void EditControl::Changed(bool DelBlock)
{
	if(m_Callback.Active)
	{
		if(m_Callback.m_Callback)
		{
			m_Callback.m_Callback(m_Callback.m_Param);
		}
		AutoComplete(false, DelBlock);
	}
}

void EditControl::SetMenuPos(VMenu& menu)
{
	if(ScrY-Y1<Min(Opt.Dialogs.CBoxMaxHeight,menu.GetItemCount())+2 && Y1>ScrY/2)
	{
		menu.SetPosition(X1,Max(0,Y1-1-Min(Opt.Dialogs.CBoxMaxHeight,menu.GetItemCount())-1),Min(ScrX-2,X2),Y1-1);
	}
	else
	{
		menu.SetPosition(X1,Y1+1,X2,0);
	}
}

void EnumFiles(VMenu& Menu, const wchar_t* Str)
{
	if(Str && *Str)
	{
		string strStr(Str);

		bool OddQuote = false;
		for(size_t i=0; i<strStr.GetLength(); i++)
		{
			if(strStr.At(i) == L'"')
			{
				OddQuote = !OddQuote;
			}
		}

		size_t Pos = 0;
		if(OddQuote)
		{
			strStr.RPos(Pos, L'"');
		}
		else
		{
			for(Pos=strStr.GetLength()-1; Pos!=static_cast<size_t>(-1); Pos--)
			{
				if(strStr.At(Pos)==L'"')
				{
					Pos--;
					while(strStr.At(Pos)!=L'"' && Pos!=static_cast<size_t>(-1))
					{
						Pos--;
					}
				}
				else if(strStr.At(Pos)==L' ')
				{
					Pos++;
					break;
				}
			}
		}
		if(Pos==static_cast<size_t>(-1))
		{
			Pos=0;
		}
		bool StartQuote=false;
		if(strStr.At(Pos)==L'"')
		{
			Pos++;
			StartQuote=true;
		}
		string strStart(strStr,Pos);
		strStr.LShift(Pos);
		Unquote(strStr);
		if(!strStr.IsEmpty())
		{
			FAR_FIND_DATA_EX d;
			string strExp;
			apiExpandEnvironmentStrings(strStr,strExp);
			FindFile Find(strExp+L"*");
			bool Separator=false;
			while(Find.Get(d))
			{
				const wchar_t* FileName=PointToName(strStr);
				bool NameMatch=!StrCmpNI(FileName,d.strFileName,StrLength(FileName)),AltNameMatch=NameMatch?false:!StrCmpNI(FileName,d.strAlternateFileName,StrLength(FileName));
				if(NameMatch || AltNameMatch)
				{
					strStr.SetLength(FileName-strStr);
					string strAdd (strStr + (NameMatch ? d.strFileName : d.strAlternateFileName));
					if (!StartQuote)
						QuoteSpace(strAdd);

					string strTmp(strStart+strAdd);
					if(StartQuote)
						strTmp += L'"';

					if(!Separator)
					{
						if(Menu.GetItemCount())
						{
							MenuItemEx Item={};
							Item.strName = MSG(MCompletionFilesTitle);
							Item.Flags=LIF_SEPARATOR;
							Menu.AddItem(&Item);
						}
						else
						{
							Menu.SetTitle(MSG(MCompletionFilesTitle));
						}
						Separator=true;
					}
					Menu.AddItem(strTmp);
				}
			}
		}
	}
}

int EditControl::AutoCompleteProc(bool Manual,bool DelBlock,int& BackKey, int Area)
{
	int Result=0;
	static int Reenter=0;

	if(ECFlags.Check(EC_ENABLEAUTOCOMPLETE) && *Str && !Reenter && (CtrlObject->Macro.GetCurRecord(nullptr,nullptr) == MACROMODE_NOMACRO || Manual))
	{
		Reenter++;

		VMenu ComplMenu(nullptr,nullptr,0,0);
		string strTemp=Str;

		if(Opt.AutoComplete.ShowList)
			CtrlObject->Macro.SetMode(Area);

		if(pHistory)
		{
			if(pHistory->GetAllSimilar(ComplMenu,strTemp))
			{
				ComplMenu.SetTitle(MSG(MCompletionHistoryTitle));
			}
		}
		else if(pList)
		{
			for(size_t i=0;i<pList->ItemsNumber;i++)
			{
				if (!StrCmpNI(pList->Items[i].Text, strTemp, static_cast<int>(strTemp.GetLength())) && StrCmp(pList->Items[i].Text, strTemp))
				{
					ComplMenu.AddItem(pList->Items[i].Text);
				}
			}
		}
		if(ECFlags.Check(EC_ENABLEFNCOMPLETE))
		{
			EnumFiles(ComplMenu,strTemp);
		}
		if(ComplMenu.GetItemCount()>1 || (ComplMenu.GetItemCount()==1 && StrCmpI(strTemp,ComplMenu.GetItemPtr(0)->strName)))
		{
			ComplMenu.SetFlags(VMENU_WRAPMODE|VMENU_NOTCENTER|VMENU_SHOWAMPERSAND);

			if(!DelBlock && Opt.AutoComplete.AppendCompletion && (!Flags.Check(FEDITLINE_PERSISTENTBLOCKS) || Opt.AutoComplete.ShowList))
			{
				int SelStart=GetLength();

				// magic
				if(IsSlash(Str[SelStart-1]) && Str[SelStart-2] == L'"' && IsSlash(ComplMenu.GetItemPtr(0)->strName.At(SelStart-2)))
				{
					Str[SelStart-2] = Str[SelStart-1];
					StrSize--;
					SelStart--;
					CurPos--;
				}

				AppendString(ComplMenu.GetItemPtr(0)->strName+SelStart);
				Select(SelStart, GetLength());
				Show();
			}
			if(Opt.AutoComplete.ShowList)
			{
				MenuItemEx EmptyItem={};
				ComplMenu.AddItem(&EmptyItem,0);
				SetMenuPos(ComplMenu);
				ComplMenu.SetSelectPos(0,0);
				ComplMenu.SetBoxType(SHORT_SINGLE_BOX);
				ComplMenu.ClearDone();
				ComplMenu.Show();
				Show();
				int PrevPos=0;

				while (!ComplMenu.Done())
				{
					INPUT_RECORD ir;
					ComplMenu.ReadInput(&ir);
					if(!Opt.AutoComplete.ModalList)
					{
						int CurPos=ComplMenu.GetSelectPos();
						if(CurPos>=0 && PrevPos!=CurPos)
						{
							PrevPos=CurPos;
							SetString(CurPos?ComplMenu.GetItemPtr(CurPos)->strName:strTemp);
							Show();
						}
					}
					if(ir.EventType==WINDOW_BUFFER_SIZE_EVENT)
					{
						SetMenuPos(ComplMenu);
						ComplMenu.Show();
					}
					else if(ir.EventType==KEY_EVENT || ir.EventType==FARMACRO_KEY_EVENT)
					{
						unsigned MenuKey=InputRecordToKey(&ir);

						// ����
						if((MenuKey>=L' ' && MenuKey<=static_cast<int>(WCHAR_MAX)) || MenuKey==KEY_BS || MenuKey==KEY_DEL || MenuKey==KEY_NUMDEL)
						{
							string strPrev;
							DeleteBlock();
							GetString(strPrev);
							ProcessKey(MenuKey);
							GetString(strTemp);
							if(StrCmp(strPrev,strTemp))
							{
								ComplMenu.DeleteItems();
								PrevPos=0;
								if(!strTemp.IsEmpty())
								{
									if(pHistory)
									{
										if(pHistory->GetAllSimilar(ComplMenu,strTemp))
										{
											ComplMenu.SetTitle(MSG(MCompletionHistoryTitle));
										}
									}
									else if(pList)
									{
										for(size_t i=0;i<pList->ItemsNumber;i++)
										{
											if (!StrCmpNI(pList->Items[i].Text, strTemp, static_cast<int>(strTemp.GetLength())) && StrCmp(pList->Items[i].Text, strTemp))
											{
												ComplMenu.AddItem(pList->Items[i].Text);
											}
										}
									}
								}
								if(ECFlags.Check(EC_ENABLEFNCOMPLETE))
								{
									EnumFiles(ComplMenu,strTemp);
								}
								if(ComplMenu.GetItemCount()>1 || (ComplMenu.GetItemCount()==1 && StrCmpI(strTemp,ComplMenu.GetItemPtr(0)->strName)))
								{
									if(MenuKey!=KEY_BS && MenuKey!=KEY_DEL && MenuKey!=KEY_NUMDEL && Opt.AutoComplete.AppendCompletion)
									{
										int SelStart=GetLength();

										// magic
										if(IsSlash(Str[SelStart-1]) && Str[SelStart-2] == L'"' && IsSlash(ComplMenu.GetItemPtr(0)->strName.At(SelStart-2)))
										{
											Str[SelStart-2] = Str[SelStart-1];
											StrSize--;
											SelStart--;
											CurPos--;
										}

										DisableCallback();
										AppendString(ComplMenu.GetItemPtr(0)->strName+SelStart);
										if(X2-X1>GetLength())
											SetLeftPos(0);
										Select(SelStart, GetLength());
										RevertCallback();
									}
									ComplMenu.AddItem(&EmptyItem,0);
									SetMenuPos(ComplMenu);
									ComplMenu.SetSelectPos(0,0);
									ComplMenu.Redraw();
								}
								else
								{
									ComplMenu.SetExitCode(-1);
								}
								Show();
							}
						}
						else
						{
							switch(MenuKey)
							{
							// "������������" �������
							case KEY_CTRLEND:
							case KEY_RCTRLEND:
								{
									ComplMenu.ProcessKey(KEY_DOWN);
									break;
								}

							case KEY_SHIFTDEL:
							case KEY_SHIFTNUMDEL:
								{
									if(ComplMenu.GetItemCount()>1)
									{
										unsigned __int64 CurrentRecord = *static_cast<unsigned __int64*>(ComplMenu.GetUserData(nullptr, 0));
										if (pHistory->DeleteIfUnlocked(CurrentRecord))
										{
											ComplMenu.DeleteItem(ComplMenu.GetSelectPos());
											if(ComplMenu.GetItemCount()>1)
											{
												SetMenuPos(ComplMenu);
												ComplMenu.Redraw();
												Show();
											}
											else
											{
												ComplMenu.SetExitCode(-1);
											}
										}
									}
								}
								break;

							// ��������� �� ������ �����
							case KEY_LEFT:
							case KEY_NUMPAD4:
							case KEY_CTRLS:     case KEY_RCTRLS:
							case KEY_RIGHT:
							case KEY_NUMPAD6:
							case KEY_CTRLD:     case KEY_RCTRLD:
							case KEY_CTRLLEFT:  case KEY_RCTRLLEFT:
							case KEY_CTRLRIGHT: case KEY_RCTRLRIGHT:
							case KEY_CTRLHOME:  case KEY_RCTRLHOME:
								{
									if(MenuKey == KEY_LEFT || MenuKey == KEY_NUMPAD4)
									{
										MenuKey = KEY_CTRLS;
									}
									else if(MenuKey == KEY_RIGHT || MenuKey == KEY_NUMPAD6)
									{
										MenuKey = KEY_CTRLD;
									}
									pOwner->ProcessKey(MenuKey);
									ComplMenu.Show();
									Show();
									break;
								}

							// ��������� �� ������
							case KEY_SHIFT:
							case KEY_ALT:
							case KEY_RALT:
							case KEY_CTRL:
							case KEY_RCTRL:
							case KEY_HOME:
							case KEY_NUMPAD7:
							case KEY_END:
							case KEY_NUMPAD1:
							case KEY_IDLE:
							case KEY_NONE:
							case KEY_ESC:
							case KEY_F10:
							case KEY_ALTF9:
							case KEY_RALTF9:
							case KEY_UP:
							case KEY_NUMPAD8:
							case KEY_DOWN:
							case KEY_NUMPAD2:
							case KEY_PGUP:
							case KEY_NUMPAD9:
							case KEY_PGDN:
							case KEY_NUMPAD3:
							case KEY_ALTLEFT:
							case KEY_ALTRIGHT:
							case KEY_ALTHOME:
							case KEY_ALTEND:
							case KEY_RALTLEFT:
							case KEY_RALTRIGHT:
							case KEY_RALTHOME:
							case KEY_RALTEND:
							case KEY_MSWHEEL_UP:
							case KEY_MSWHEEL_DOWN:
							case KEY_MSWHEEL_LEFT:
							case KEY_MSWHEEL_RIGHT:
								{
									ComplMenu.ProcessInput();
									break;
								}

							case KEY_ENTER:
							case KEY_NUMENTER:
							{
								if(Opt.AutoComplete.ModalList)
								{
									ComplMenu.ProcessInput();
									break;
								}
							}

							// �� ��������� ��������� ������ � ��� ���������
							default:
								{
									ComplMenu.Hide();
									ComplMenu.SetExitCode(-1);
									BackKey=MenuKey;
									Result=1;
								}
							}
						}
					}
					else
					{
						ComplMenu.ProcessInput();
					}
				}
				int ExitCode=ComplMenu.GetExitCode();

				// mouse click
				if(ExitCode>0)
				{
					if(Opt.AutoComplete.ModalList)
					{
						SetString(ComplMenu.GetItemPtr(ExitCode)->strName);
					}
					else
					{
						ComplMenu.Hide();
						BackKey = KEY_ENTER;
						Result=1;
					}
				}
			}
		}

		Reenter--;
	}
	return Result;
}

void EditControl::AutoComplete(bool Manual,bool DelBlock)
{
	int Key=0;
	int PrevMacroMode=CtrlObject->Macro.GetMode();
	if(Opt.AutoComplete.ShowList)
		CtrlObject->Macro.SetMode(MacroAreaAC);
	if(AutoCompleteProc(Manual,DelBlock,Key,MacroAreaAC))
	{
		// BUGBUG, hack
		int Wait=WaitInMainLoop;
		WaitInMainLoop=1;
		struct FAR_INPUT_RECORD irec={(DWORD)Key,*FrameManager->GetLastInputRecord()};
		if(!CtrlObject->Macro.ProcessEvent(&irec))
			pOwner->ProcessKey(Key);
		WaitInMainLoop=Wait;
		Show();
	}
	CtrlObject->Macro.SetMode(PrevMacroMode);
}

int EditControl::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
	if(Edit::ProcessMouse(MouseEvent))
	{
		while(IsMouseButtonPressed()==FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			Flags.Clear(FEDITLINE_CLEARFLAG);
			SetTabCurPos(IntKeyState.MouseX - X1 + LeftPos);
			if(IntKeyState.MouseEventFlags&MOUSE_MOVED)
			{
				if(!Selection)
				{
					Selection=true;
					SelectionStart=-1;
					Select(SelectionStart,0);
				}
				else
				{
					if(SelectionStart==-1)
					{
						SelectionStart=CurPos;
					}
					Select(Min(SelectionStart,CurPos),Min(StrSize,Max(SelectionStart,CurPos)));
					Show();
				}
			}
		}
		Selection=false;
		return TRUE;
	}
	return FALSE;
}

void EditControl::EnableAC(bool Permanent)
{
	ACState=Permanent?true:ECFlags.Check(EC_ENABLEAUTOCOMPLETE)!=FALSE;
	ECFlags.Set(EC_ENABLEAUTOCOMPLETE);
}

void EditControl::DisableAC(bool Permanent)
{
	ACState=Permanent?false:ECFlags.Check(EC_ENABLEAUTOCOMPLETE)!=FALSE;
	ECFlags.Clear(EC_ENABLEAUTOCOMPLETE);
}

void EditControl::SetObjectColor(PaletteColors Color,PaletteColors SelColor,PaletteColors ColorUnChanged)
{
	Edit::SetObjectColor(Color, SelColor);
	this->ColorUnChanged=ColorIndexToColor(ColorUnChanged);
}

void EditControl::SetObjectColor(const FarColor& Color,const FarColor& SelColor, const FarColor& ColorUnChanged)
{
	Edit::SetObjectColor(Color, SelColor);
	this->ColorUnChanged=ColorUnChanged;
}

void EditControl::GetObjectColor(FarColor& Color, FarColor& SelColor, FarColor& ColorUnChanged)
{
	Edit::GetObjectColor(Color, SelColor);
	ColorUnChanged = this->ColorUnChanged;
}

void EditControl::SetUnchangedColor()
{
	SetColor(ColorUnChanged);
}
