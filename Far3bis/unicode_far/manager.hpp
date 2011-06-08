#pragma once

/*
manager.hpp

������������ ����� ����������� file panels, viewers, editors
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

class Frame;

class Manager
{
#if defined(SYSLOG)
		friend void ManagerClass_Dump(const wchar_t *Title,const Manager *m,FILE *fp);
#endif
	private:
		Frame **ModalStack;     // ���� ��������� �������
		int ModalStackCount;    // ������ ����� ��������� �������
		int ModalStackSize;     // ����� ����� ��������� �������

		int  FrameCount;         // ������ ����������� �������
		Frame **FrameList;       // ������� ��������� �������
		int  FrameListSize;      // ������ ������ ��� ����������� �������
		int  FramePos;           // ������ ������� ������������ ������. �� �� ������ ��������� � CurrentFrame
		// ������� ����������� ����� ����� �������� � ������� FrameManager->GetBottomFrame();

		/*$ ����������� �� ... */
		Frame *InsertedFrame;   // �����, ������� ����� �������� � ����� ����������� �������
		Frame *DeletedFrame;    // �����, �������������� ��� �������� �� ��������� �������, �� ���������� �����, ���� ��������� (�������� ��� �� ���, �� ���)
		Frame *ActivatedFrame;  // �����, ������� ���������� ������������ ����� ����� ������ ���������
		Frame *RefreshedFrame;  // �����, ������� ����� ������ ��������, �.�. ������������
		Frame *ModalizedFrame;  // �����, ������� ���������� � "�������" � �������� ������������ ������
		Frame *UnmodalizedFrame;// �����, ������������ �� "�������" ������������ ������
		Frame *DeactivatedFrame;// �����, ������� ��������� �� ���������� �������� �����
		Frame *ExecutedFrame;   // �����, �������� ��������� ����� ����� ��������� �� ������� ���������� ������

		Frame *CurrentFrame;     // ������� �����. �� ����� ����������� ��� � ����������� �������, ��� � � ��������� �����
		// ��� ����� �������� � ������� FrameManager->GetCurrentFrame();
		/*Frame **SemiModalBackFrames;
		int SemiModalBackFramesCount;
		int SemiModalBackFramesSize;*/

		/* $ 15.05.2002 SKV
		  ��� ��� ���� ����������, ��� � �� ���� ��������,
		  ������ ������� ��������� editor/viewer'��.
		  ĸ����� ���  ���� ������� ����� ������� ExecuteModal.
		  � ��������� ������, ��� ��� ExecuteModal ����������
		  1) �� ������ ��� ��������� ������� (��� ��� �� �������������),
		  2) �� ������ ��� editor/viewer'��.
		*/
		int ModalEVCount;

		int  EndLoop;            // ������� ������ �� �����
		int  StartManager;
		INPUT_RECORD LastInputRecord;

		int ModalExitCode;

	private:
		void StartupMainloop();
		Frame *FrameMenu(); //    ������ void SelectFrame(); // show window menu (F12)

		BOOL Commit();         // ��������� ���������� �� ���������� � ������� � ����� �������
		// ��� � ����� �������� ����, ���� ������ ���� �� ���������� ������� �� nullptr
		// �������, "����������� ����������" - Commit'a
		// ������ ���������� �� ������ �� ���� � �� ������ ����
		void RefreshCommit();  //
		void DeactivateCommit(); //
		void ActivateCommit(); //
		void UpdateCommit();   // ����������� �����, ����� ����� �������� ���� ����� �� ������
		void InsertCommit();
		void DeleteCommit();
		void ExecuteCommit();
		void ModalizeCommit();
		void UnmodalizeCommit();

		int GetModalExitCode();

		/*void AddSemiModalBackFrame(Frame* frame);
		BOOL IsSemiModalBackFrame(Frame *frame);
		void RemoveSemiModalBackFrame(Frame* frame);*/

	public:
		Manager();
		~Manager();

	public:
		// ��� ������� ����� ��������� �������� ����������� �� ������ ����� ����
		// ��� ��� �� ����������� ���������� � ���, ��� ����� ����� ������� � �������� ��� ��������� ������ Commit()
		void InsertFrame(Frame *NewFrame, int Index=-1);
		void DeleteFrame(Frame *Deleted=nullptr);
		void DeleteFrame(int Index);
		void DeactivateFrame(Frame *Deactivated,int Direction);
		void SwapTwoFrame(int Direction);
		void ActivateFrame(Frame *Activated);
		void ActivateFrame(int Index);
		void RefreshFrame(Frame *Refreshed=nullptr);
		void RefreshFrame(int Index);

		//! ������� ��� ������� ��������� �������.
		void ExecuteFrame(Frame *Executed);


		//! ������ � ����� ���� ��������� �������
		void ExecuteModal(Frame *Executed=nullptr);
		//! ��������� ����������� ����� � ��������� ������
		void ExecuteNonModal();
		//! �������� ����, ��� ����������� ����� ��������� ��� � �� ������� �����.
		BOOL ifDoubleInstance(Frame* frame);

		//!  �������, ������� �������� � �������� ���������� ������.
		//  ������ ������������ ������ ��� �������� ��������� � ������� ���������� �������� ���� VFMenu
		void ModalizeFrame(Frame *Modalized=nullptr, int Mode=TRUE);
		void UnmodalizeFrame(Frame *Unmodalized);

		void CloseAll();
		/* $ 29.12.2000 IS
		     ������ CloseAll, �� ��������� ����������� ����������� ������ � ����,
		     ���� ������������ ��������� ������������� ����.
		     ���������� TRUE, ���� ��� ������� � ����� �������� �� ����.
		*/
		BOOL ExitAll();
		BOOL IsAnyFrameModified(int Activate);

		int  GetFrameCount() {return(FrameCount);};
		int  GetFrameCountByType(int Type);

		/*$ 26.06.2001 SKV
		��� ������ ����� ACTL_COMMIT
		*/
		BOOL PluginCommit();

		int CountFramesWithName(const wchar_t *Name, BOOL IgnoreCase=TRUE);

		BOOL IsPanelsActive(); // ������������ ��� ������� WaitInMainLoop
		void SetFramePos(int NewPos);
		int  FindFrameByFile(int ModalType,const wchar_t *FileName,const wchar_t *Dir=nullptr);
		BOOL ShowBackground();

		void EnterMainLoop();
		void ProcessMainLoop();
		void ExitMainLoop(int Ask);
		int ProcessKey(DWORD key);
		int ProcessMouse(MOUSE_EVENT_RECORD *me);

		void PluginsMenu(); // �������� ���� �� F11
		void SwitchToPanels();

		INPUT_RECORD *GetLastInputRecord() { return &LastInputRecord; }
		void SetLastInputRecord(INPUT_RECORD *Rec);
		void ResetLastInputRecord() { LastInputRecord.EventType=0; }

		Frame *GetCurrentFrame() { return CurrentFrame; }

		Frame *operator[](size_t Index);

		int IndexOf(Frame *Frame);

		int IndexOfStack(Frame *Frame);
		int HaveAnyFrame();

		void ImmediateHide();
		/* $ 13.04.2002 KM
		  ��� ������ ResizeConsole ��� ���� NextModal �
		  ���������� ������.
		*/
		void ResizeAllModal(Frame *ModalFrame);

		Frame *GetBottomFrame() { return (*this)[FramePos]; }

		BOOL ManagerIsDown() {return EndLoop;}
		BOOL ManagerStarted() {return StartManager;}

		void InitKeyBar();

		/* $ 15.05.2002 SKV
		  ��� ��� ����� ��� � ������ ������,
		  � ���������� �������� �� �������������,
		  �� ����� ��� ������ ���.
		*/
		void EnterModalEV() {ModalEVCount++;}
		void ExitModalEV() {ModalEVCount--;}
		BOOL InModalEV() {return ModalEVCount;}

		void ResizeAllFrame();

		// ���������� top-����� ��� ��� �����, ���� � ������ ���� �������
		Frame* GetTopModal();
};

extern Manager *FrameManager;
extern long CurrentWindowType;
