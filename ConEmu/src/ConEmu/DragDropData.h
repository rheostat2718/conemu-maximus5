
/*
Copyright (c) 2009-2011 Maximus5
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

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY EXPRESS OR
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

#pragma once
//#include "..\common\common.hpp"
#include "BaseDragDrops.h"
//#include "virtualconsole.h"


//#define UNLOCKED_DRAG
#undef UNLOCKED_DRAG
//#define PERSIST_OVL
#undef PERSIST_OVL

#define MSG_STARTDRAG (WM_APP+10)

#define MAX_DROP_PATH 0x800

#include <pshpack1.h>
// This is representation of SHDRAGIMAGE
typedef struct tag_DragImageBits
{
	DWORD nWidth, nHeight; // XP max 301x301
	DWORD nXCursor, nYCursor; // ������� ������� �������, ������������ ��������� ��������
	DWORD nRes1; // HBITMAP hbmpDragImage;
	DWORD nRes2; // COLORREF crColorKey; ������ 0xffffffff
#ifdef WIN64
	DWORD nPad1, nPad2; // ������������ �� ������� QWORD
#endif
	RGBQUAD pix[1];
} DragImageBits;
#include <poppack.h>


#define HIDA_GetPIDLFolder(pida) (LPCITEMIDLIST)(((LPBYTE)pida)+(pida)->aoffset[0])
#define HIDA_GetPIDLItem(pida, i) (LPCITEMIDLIST)(((LPBYTE)pida)+(pida)->aoffset[i+1])

struct ForwardedPanelInfo;


class CDragDropData
{
	public:
		CDragDropData();
		~CDragDropData();
		// ����������� ���� ConEmu, ��� ��������������� D&D
		BOOL Register();
		// ��������� �� ���� ���������� ��� Drag
		BOOL PrepareDrag(BOOL abClickNeed, COORD crMouseDC, DWORD* pdwAllowedEffects);
		// ��������� �� ���� ���������� ��� Drop
		void RetrieveDragToInfo();
		// Callback
		void DragFeedBack(DWORD dwEffect);
		// ����� Drag, ��� ������� ���� mh_Overlapped
		BOOL InDragDrop();
		// Support for background D&D
		BOOL IsDragStarting();
		BOOL ForwardMessage(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
	private:
		//wchar_t *mpsz_DraggedPath; // ASCIIZZ
		int RetrieveDragFromInfo(BOOL abClickNeed, COORD crMouseDC, wchar_t** ppszDraggedPath, UINT* pnFilesCount);
		// ���������� � mp_DataObject ��������������� ��������
		BOOL AddFmt_FileNameW(wchar_t* pszDraggedPath, UINT nFilesCount, int cbSize);
		BOOL AddFmt_SHELLIDLIST(wchar_t* pszDraggedPath, UINT nFilesCount, int cbSize);
		BOOL AddFmt_PREFERREDDROPEFFECT(wchar_t* pszDraggedPath, UINT nFilesCount, int cbSize);
		BOOL AddFmt_InShellDragLoop(wchar_t* pszDraggedPath, UINT nFilesCount, int cbSize);
		BOOL AddFmt_HDROP(wchar_t* pszDraggedPath, UINT nFilesCount, int cbSize);
		BOOL AddFmt_DragImageBits(wchar_t* pszDraggedPath, UINT nFilesCount, int cbSize);
	protected:
		CDataObject *mp_DataObject;
		ForwardedPanelInfo *m_pfpi;
		BOOL mb_DragDropRegistered;
	protected:
		ITEMIDLIST m_DesktopID;
		void EnumDragFormats(IDataObject * pDataObject, HANDLE hDumpFile = NULL);
		//DragImageBits m_BitsInfo;
		HWND mh_Overlapped;
		HDC mh_BitsDC;
		HBITMAP mh_BitsBMP, mh_BitsBMP_Old;
		//int m_iBPP;
		static LRESULT CALLBACK DragBitsWndProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
		BOOL CreateDragImageWindow();
		void DestroyDragImageWindow();
		BOOL LoadDragImageBits(IDataObject * pDataObject);
		BOOL CreateDragImageBits(IDataObject * pDataObject);
		DragImageBits* CreateDragImageBits(wchar_t* pszFiles);
		BOOL DrawImageBits(HDC hDrawDC, wchar_t* pszFile, int *nMaxX, int nX, int *nMaxY);
		void DestroyDragImageBits();
		void MoveDragWindow(BOOL abVisible=TRUE);
		//DragImageBits m_ImgInfo;
		//LPBYTE mp_ImgData;
		DragImageBits *mp_Bits;
		BOOL mb_DragWithinNow;
		//
		static DWORD WINAPI ExtractIconsThread(LPVOID lpParameter);
		DWORD mn_ExtractIconsTID;
		HANDLE mh_ExtractIcons;

		/* Unlocked drag support */
	protected:
		typedef struct tag_CEDragSource
		{
			BOOL    bInDrag;
			CDragDropData* pDrag;
			DWORD   nTID;
			HANDLE  hReady;
			HANDLE  hThread;
			HWND    hWnd;
		} CEDragSource;
		std::vector<CEDragSource*> m_Sources;
		BOOL mb_DragStarting;
		void TerminateDrag();
		static LRESULT WINAPI DragProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
		static DWORD WINAPI DragThread(LPVOID lpParameter);
		CEDragSource* InitialCreateSource();
		CEDragSource* GetFreeSource();
		wchar_t ms_SourceClass[32];
		ATOM mh_SourceClass;
};
