
/*
Copyright (c) 2009-2012 Maximus5
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


#define SHOWDEBUGSTR

#include <shlobj.h>
#include "Header.h"
#include "ScreenDump.h"

#define MAX_OVERLAY_WIDTH    300
#define MAX_OVERLAY_HEIGHT   300
#define OVERLAY_TEXT_SHIFT   (gpSet->isDragShowIcons ? 18 : 0)
#define OVERLAY_COLUMN_SHIFT 5

#define DEBUGSTROVL(s) //DEBUGSTR(s)
#define DEBUGSTRBACK(s) //DEBUGSTR(s)


CDragDrop::CDragDrop()
{
	//m_hWnd = hWnd;
	mb_DragDropRegistered = FALSE;
	m_lRefCount = 1;
	m_pfpi = NULL;
	mp_DataObject = NULL;
	mb_selfdrag = NULL;
	mp_DesktopID = NULL;
	mh_Overlapped = NULL; mh_BitsDC = NULL; mh_BitsBMP = mh_BitsBMP_Old = NULL;
	mp_Bits = NULL;
	mn_AllFiles = 0; mn_CurWritten = 0; mn_CurFile = 0;
	mb_DragWithinNow = FALSE;
	mn_ExtractIconsTID = 0;
	mh_ExtractIcons = NULL;
	mh_DragThread = NULL; mn_DragThreadId = 0;
	InitializeCriticalSection(&m_CrThreads);
}

BOOL CDragDrop::Init()
{
	BOOL lbRc = FALSE;
	HRESULT hr = S_OK;
	wchar_t szDesktopPath[MAX_PATH+1];
	hr = SHGetFolderPath(ghWnd, CSIDL_DESKTOP, NULL, SHGFP_TYPE_CURRENT, szDesktopPath);

	if (hr == S_OK)
	{
		DWORD dwAttrs = GetFileAttributes(szDesktopPath);

		if (dwAttrs == (DWORD)-1)
		{
			// ����� ����������
		}
		else if ((dwAttrs & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
		{
			// mp_DesktopID
			// [out] The address of a pointer to an item identifier list structure that specifies
			// the folder's location relative to the root of the namespace (the desktop). The
			// ppidl parameter is set to NULL on failure. The calling application is responsible for
			// freeing this resource by calling ILFree.
			hr = SHGetFolderLocation(ghWnd, CSIDL_DESKTOP, NULL, 0, &mp_DesktopID);

			if (hr != S_OK)
			{
				DisplayLastError(L"Can't get desktop folder", hr);
			}
		}
	}

	if (mp_DesktopID == NULL)
	{
		wchar_t szError[MAX_PATH*2];
		wcscpy(szError, L"Desktop folder not found:\n");
		wcscat(szError, szDesktopPath);
		wcscat(szError, L"\nSome Drag-n-Drop features will be disabled!");
		DisplayLastError(szError);
	}

	hr = RegisterDragDrop(ghWnd, this);

	if (hr != S_OK)
	{
		DisplayLastError(L"Can't register Drop target", hr);
		goto wrap;
	}
	else
	{
		mb_DragDropRegistered = TRUE;
	}

	lbRc = TRUE;
wrap:
	return lbRc;
}

CDragDrop::~CDragDrop()
{
	DestroyDragImageBits();
	DestroyDragImageWindow();

	if (mb_DragDropRegistered && ghWnd)
	{
		mb_DragDropRegistered = FALSE;
		RevokeDragDrop(ghWnd);
	}

	TryEnterCriticalSection(&m_CrThreads);

	if (!m_OpThread.empty())
	{
		LeaveCriticalSection(&m_CrThreads);

		if (MessageBox(ghWnd, _T("Not all shell operations was finished!\r\nDo You want to terminate them (it's may be harmful)?"),
		              gpConEmu->GetDefaultTitle(), MB_YESNO|MB_ICONEXCLAMATION) == IDYES)
		{
			// Terminate all thread
			TryEnterCriticalSection(&m_CrThreads);
			std::vector<ThInfo>::iterator iter = m_OpThread.begin();

			while(iter != m_OpThread.end())
			{
				HANDLE hThread = iter->hThread;
				TerminateThread(hThread, 100);
				iter = m_OpThread.erase(iter);
			}

			LeaveCriticalSection(&m_CrThreads);
		}
		else
		{
			// ����� ����� �� ���������
			BOOL lbActive = TRUE;

			while(TRUE)
			{
				Sleep(100);
				TryEnterCriticalSection(&m_CrThreads);
				lbActive = !m_OpThread.empty();
				LeaveCriticalSection(&m_CrThreads);
			}
		}
	}
	else
	{
		// ������������� ����� ���
		LeaveCriticalSection(&m_CrThreads);
	}

	// ���� ���� ��������� ��������� �� �������
	if (mh_DragThread && mn_DragThreadId)
	{
		TerminateThread(mh_DragThread, 100);
	}

	SafeCloseHandle(mh_DragThread); mn_DragThreadId = 0;

	if (m_pfpi) free(m_pfpi); m_pfpi=NULL;

	if (mp_DesktopID) { CoTaskMemFree(mp_DesktopID); mp_DesktopID = NULL; }

	DeleteCriticalSection(&m_CrThreads);
}

void CDragDrop::Drag(BOOL abClickNeed, COORD crMouseDC)
{
	BOOL lbNeedLBtnUp = !abClickNeed;
	CConEmuPipe pipe(gpConEmu->GetFarPID(), CONEMUREADYTIMEOUT);

	if (!gpSet->isDragEnabled /*|| isInDrag */|| gpConEmu->isDragging())
	{
		gpConEmu->DebugStep(_T("DnD: Drag disabled"));
		goto wrap;
	}

	if (mh_DragThread)
	{
		gpConEmu->DebugStep(L"DnD: Drag thread already created!");
		goto wrap;
	}

	_ASSERTE(!gpConEmu->isPictureView());
	//gpConEmu->isDragProcessed=true; // ����� �� ��������� ��� ���� �� ���� ����
	gpConEmu->mouse.state |= (gpConEmu->mouse.state & DRAG_R_ALLOWED) ? DRAG_R_STARTED : DRAG_L_STARTED;
	MCHKHEAP

	if (m_pfpi) {free(m_pfpi); m_pfpi=NULL;}

	if (pipe.Init(_T("CDragDrop::Drag")))
	{
		//DWORD cbWritten=0;
		struct
		{
			BOOL  bClickNeed;
			COORD crMouse;
		} DragArg;
		DragArg.bClickNeed = abClickNeed;
		DragArg.crMouse = gpConEmu->ActiveCon()->ClientToConsole(crMouseDC.X, crMouseDC.Y);

		if (pipe.Execute(CMD_DRAGFROM, &DragArg, sizeof(DragArg)))
		{
			lbNeedLBtnUp = FALSE; // ����� ��� ���������� � �������
			gpConEmu->DebugStep(_T("DnD: Recieving data"));
			DWORD cbBytesRead=0;
			int nWholeSize=0;
			pipe.Read(&nWholeSize, sizeof(nWholeSize), &cbBytesRead);

			if (nWholeSize==0)    // ������ ����� �������
			{
				if (!pipe.Read(&nWholeSize, sizeof(nWholeSize), &cbBytesRead))
					nWholeSize = 0;
			}
			else
			{
				nWholeSize=0;
			}

			MCHKHEAP

			if (nWholeSize<=0)
			{
				gpConEmu->DebugStep(_T("DnD: Data is empty"));
			}
			else
			{
				gpConEmu->DebugStep(_T("DnD: Recieving data..."));
				wchar_t *szDraggedPath=NULL; //ASCIIZZ
				szDraggedPath=new wchar_t[nWholeSize/*(MAX_PATH+1)*FilesCount*/+1];
				ZeroMemory(szDraggedPath, /*((MAX_PATH+1)*FilesCount+1)*/(nWholeSize+1)*sizeof(wchar_t));
				wchar_t  *curr=szDraggedPath;
				UINT nFilesCount = 0;

				for(;;)
				{
					int nCurSize=0;

					if (!pipe.Read(&nCurSize, sizeof(nCurSize), &cbBytesRead)) break;

					if (nCurSize==0) break;

					pipe.Read(curr, sizeof(WCHAR)*nCurSize, &cbBytesRead);
					_ASSERTE(*curr);
					curr+=_tcslen(curr)+1;
					nFilesCount ++;
				}

				int cbStructSize=0;

				if (m_pfpi) {free(m_pfpi); m_pfpi=NULL;}

				if (pipe.Read(&cbStructSize, sizeof(int), &cbBytesRead))
				{
					if ((DWORD)cbStructSize>sizeof(ForwardedPanelInfo))
					{
						m_pfpi = (ForwardedPanelInfo*)calloc(cbStructSize, 1);
						pipe.Read(m_pfpi, cbStructSize, &cbBytesRead);
						m_pfpi->pszActivePath = (WCHAR*)(((char*)m_pfpi)+m_pfpi->ActivePathShift);
						//Slash �� ����� ��� �� �����
						int nPathLen = lstrlenW(m_pfpi->pszActivePath);

						if (nPathLen>0 && m_pfpi->pszActivePath[nPathLen-1]==_T('\\'))
							m_pfpi->pszActivePath[nPathLen-1] = 0;

						m_pfpi->pszPassivePath = (WCHAR*)(((char*)m_pfpi)+m_pfpi->PassivePathShift);
						//Slash �� ����� ��� �� �����
						nPathLen = lstrlenW(m_pfpi->pszPassivePath);

						if (nPathLen>0 && m_pfpi->pszPassivePath[nPathLen-1]==_T('\\'))
							m_pfpi->pszPassivePath[nPathLen-1] = 0;
					}
				}

				// ����� �������
				pipe.Close();
				int size=(curr-szDraggedPath)*sizeof(wchar_t)+2;
				MCHKHEAP
				HGLOBAL file_nameW = NULL, file_PIDLs = NULL;

				if (nFilesCount==1 && mp_DesktopID)
				{
					HRESULT hr = S_OK;

					try
					{
						file_nameW = GlobalAlloc(GPTR, sizeof(WCHAR)*(lstrlenW(szDraggedPath)+1));
						lstrcpyW(((WCHAR*)file_nameW), szDraggedPath);
						//TODO: ��������� ������ hr!
						WCHAR* pszSlash = wcsrchr(szDraggedPath, L'\\');

						if (pszSlash)
						{
							// ������� ����� �������� PIDL ��� ������������ �����
							SFGAOF tmp;
							LPITEMIDLIST pItem=NULL;
							DWORD nParentSize=0, nItemSize=0;
							IShellFolder *pDesktop=NULL;
							MCHKHEAP
							//hr = SHGetSpecialFolderLocation ( ghWnd, CSIDL_DESKTOP, &pParent );
							//hr = SHGetFolderLocation ( ghWnd, CSIDL_DESKTOP, NULL, 0, &pParent );
							//hr = SHGetFolderLocation ( ghWnd, CSIDL_DESKTOP, NULL, 0, &pItem );
							hr = SHGetDesktopFolder(&pDesktop);

							if (hr == S_OK && pDesktop)
							{
								MCHKHEAP
								// ����� � ��� ���������� �����/�����...
								ULONG nEaten=0;
								hr = pDesktop->ParseDisplayName(ghWnd, NULL, szDraggedPath, &nEaten, &pItem, &(tmp=0));
								pDesktop->Release(); pDesktop = NULL;

								if (hr == S_OK && pItem && mp_DesktopID)
								{
									SHITEMID *pCur = (SHITEMID*)mp_DesktopID;

									while(pCur->cb)
									{
										pCur = (SHITEMID*)(((LPBYTE)pCur) + pCur->cb);
									}

									nParentSize = ((LPBYTE)pCur) - ((LPBYTE)mp_DesktopID)+2;
									pCur = (SHITEMID*)pItem;

									while(pCur->cb)
									{
										pCur = (SHITEMID*)(((LPBYTE)pCur) + pCur->cb);
									}

									nItemSize = ((LPBYTE)pCur) - ((LPBYTE)pItem)+2;
									pCur = NULL;
									MCHKHEAP
									file_PIDLs = GlobalAlloc(GPTR, 3*sizeof(UINT)+nParentSize+nItemSize);

									if (file_PIDLs)
									{
										CIDA* pida = (CIDA*)file_PIDLs;
										pida->cidl = 1;
										pida->aoffset[0] = 3*sizeof(UINT);
										pida->aoffset[1] = pida->aoffset[0]+nParentSize;
										memmove((((LPBYTE)pida)+(pida)->aoffset[0]), mp_DesktopID, nParentSize);
										memmove((((LPBYTE)pida)+(pida)->aoffset[1]), pItem, nItemSize);
									}

									MCHKHEAP
								}

								if (pItem)
									CoTaskMemFree(pItem);
							}
						}
					}
					catch(...)
					{
						hr = -1;
					}

					if (hr == -1)
					{
						gpConEmu->DebugStep(_T("DnD: Exception in shell"));
						goto wrap;
					}
				}

				MCHKHEAP
				IDropSource *pDropSource = NULL;
				DROPFILES drop_struct = { sizeof(drop_struct), { 0, 0 }, 0, 1 };
				HGLOBAL drop_data = GlobalAlloc(GPTR, size+sizeof(drop_struct));
				_ASSERTE(drop_data);

				if (!drop_data)
				{
					gpConEmu->DebugStep(_T("DnD: Memory allocation failed!"));
					goto wrap;
				}

				memcpy(drop_data, &drop_struct, sizeof(drop_struct));
				memcpy(((byte*)drop_data) + sizeof(drop_struct), szDraggedPath, size);
				//Maximus5 - � ������ ����������� ��� �����?
				delete szDraggedPath; szDraggedPath=NULL;
				HGLOBAL drop_preferredeffect = GlobalAlloc(GPTR, sizeof(DWORD));

				if (gpConEmu->mouse.state & DRAG_R_STARTED)
					*((DWORD*)drop_preferredeffect) = DROPEFFECT_LINK;
				else if (gpSet->isDefCopy)
					*((DWORD*)drop_preferredeffect) = DROPEFFECT_COPY;
				else
					*((DWORD*)drop_preferredeffect) = DROPEFFECT_MOVE;

				MCHKHEAP
				HGLOBAL drag_loop = GlobalAlloc(GPTR, sizeof(DWORD));
				*((DWORD*)drag_loop) = 1;
				DragImageBits *pDragBits = CreateDragImageBits((wchar_t*)(((byte*)drop_data) + sizeof(drop_struct)));
				FORMATETC       fmtetc[] =
				{
					{ CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
					{ RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT), 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
					{ RegisterClipboardFormat(L"InShellDragLoop"), 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
					{ RegisterClipboardFormat(L"DragImageBits"), 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
					// ��� ������ ���� ����������
					{ RegisterClipboardFormat(CFSTR_SHELLIDLIST), 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL},
					{ RegisterClipboardFormat(L"FileNameW"), 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL }
				};
				STGMEDIUM       stgmed[] =
				{
					{ TYMED_HGLOBAL, { (HBITMAP)drop_data }, 0 }, //����� �� ���������� � ������������ ������������
					{ TYMED_HGLOBAL, { (HBITMAP)drop_preferredeffect }, 0 },
					{ TYMED_HGLOBAL, { (HBITMAP)drag_loop }, 0 },
					{ TYMED_HGLOBAL, { (HBITMAP)pDragBits }, 0 },
					// ��� ������ ���� ����������, ����� ������ ��� �������� ������� � ������� Alt
					{ TYMED_HGLOBAL, { (HBITMAP)file_PIDLs }, 0},
					{ TYMED_HGLOBAL, { (HBITMAP)file_nameW }, 0 }
				};
				//stgmed.hGlobal = drop_data ;
				//--stgmed.lpszFileName=szDraggedPath;
				CreateDropSource(&pDropSource, this);
				MCHKHEAP
				int nCount = sizeof(fmtetc)/sizeof(*fmtetc);

				if (!pDragBits)
				{
					nCount --;
					stgmed[nCount-2] = stgmed[nCount-1]; fmtetc[nCount-2] = fmtetc[nCount-1];
					stgmed[nCount-1] = stgmed[nCount];   fmtetc[nCount-1] = fmtetc[nCount];
				}

				if (!file_PIDLs) nCount -= 2;

				CreateDataObject(fmtetc, stgmed, nCount, &mp_DataObject) ;//   |   ���������� ����...
				// ��������� ���
				DWORD dwAllowedEffects = (file_PIDLs ? DROPEFFECT_LINK : 0)|DROPEFFECT_COPY|DROPEFFECT_MOVE;
				MCHKHEAP

				//CFSTR_PREFERREDDROPEFFECT, FD_LINKUI

				// ����� ������� ���������� � ����� �������...
				if (!m_pfpi)  // ���� ��� ��� �� �������
					RetrieveDragToInfo(mp_DataObject);

				if (LoadDragImageBits(mp_DataObject))
				{
					mb_DragWithinNow = TRUE;
					CreateDragImageWindow();
				}

				//gpConEmu->DebugStep(_T("DnD: Finally, DoDragDrop"));
				gpConEmu->DebugStep(NULL);
				MCHKHEAP
				//// 2009-08-20 ������� � ��������� ����, ����� ��� ��������� �������� Drop - ���� ConEmu �����������
				//DragThreadArg *pArg = new DragThreadArg;
				//if (!pArg) {
				//	DisplayLastError(L"Memory allocation error (DragThreadArg)", -1);
				//} else {
				//	pArg->pThis = this;
				//	pArg->pDataObject = mp_DataObject;
				//	pArg->pDropSource = pDropSource;
				//	pArg->dwAllowedEffects = dwAllowedEffects;
				//	// ���������� ������ �����
				//	mh_DragThread = CreateThread(NULL, 0, CDragDrop::DragOpThreadProc, pArg, 0, &mn_DragThreadId);
				//}
				DWORD dwResult = 0, dwEffect = 0;
				dwResult = DoDragDrop(mp_DataObject, pDropSource, dwAllowedEffects, &dwEffect);
				MCHKHEAP
				mp_DataObject->Release(); mp_DataObject = NULL;
				pDropSource->Release();
				////isLBDown=false; -- � ReleaseCapture ��� ����� ������?
			}

			//}
			//}
		}
	}

	//isInDrag=false;
	//isDragProcessed=false; -- ����� ��� �������� � ��������� ������ ������� ������ ���� ����� ��������� ��� ���???
wrap:

	if (lbNeedLBtnUp)
	{
		//�.�. D&D �� ������ - ����� ���� "���������" ����� ������ ����� � ����
		gpConEmu->ActiveCon()->RCon()->OnMouse(WM_LBUTTONUP, 0, crMouseDC.X, crMouseDC.Y);
	}

	return;
}

DWORD CDragDrop::DragOpThreadProc(LPVOID lpParameter)
{
	DragThreadArg *pArg = (DragThreadArg*)lpParameter;
	_ASSERTE(pArg);

	if (!pArg)
	{
		DisplayLastError(L"DragThreadArg is NULL", -1);
		return 0;
	}

	DWORD dwResult = 0, dwEffect = 0;
	HRESULT hr = S_OK;
	hr = OleInitialize(NULL);  // ��� �� ����������� �������� Ole ������ �� ����� �����. ������� ��� ��-�� ���� ������ ������������ �����
	dwResult = DoDragDrop(pArg->pDataObject, pArg->pDropSource, pArg->dwAllowedEffects, &dwEffect);
	MCHKHEAP

	if (pArg->pThis->mp_DataObject == pArg->pDataObject)
	{
		pArg->pThis->mp_DataObject = NULL;
	}

	pArg->pDataObject->Release();
	pArg->pDataObject = NULL;
	pArg->pDropSource->Release();
	pArg->pDropSource = NULL;

	if (pArg->pThis->mn_DragThreadId == GetCurrentThreadId())
	{
		SafeCloseHandle(pArg->pThis->mh_DragThread);
		pArg->pThis->mn_DragThreadId = 0;
	}

	delete pArg;
	return 0;
}

wchar_t* CDragDrop::FileCreateName(BOOL abActive, BOOL abWide, LPVOID asFileName)
{
	if (!asFileName ||
	        (abWide && ((wchar_t*)asFileName)[0] == 0) ||
	        (!abWide && ((char*)asFileName)[0] == 0))
	{
		MBoxA(_T("Can't drop file! Filename is empty!"));
		return NULL;
	}

	wchar_t *pszFullName = NULL;
	wchar_t* pszNameW = (wchar_t*)asFileName;
	char* pszNameA = (char*)asFileName;
	int nSize = 0;
	if (abActive) nSize = _tcslen(m_pfpi->pszActivePath)+1; else nSize = _tcslen(m_pfpi->pszPassivePath)+1;

	int nPathLen = nSize;

	if (abWide)
	{
		wchar_t* pszSlash = wcsrchr(pszNameW, L'\\');

		if (pszSlash) pszNameW = pszSlash+1;

		if (!*pszNameW)
		{
			MBoxA(_T("Can't drop file! Filename is empty (W)!"));
			return NULL;
		}

		nSize += _tcslen(pszNameW)+1;
	}
	else
	{
		char* pszSlash = strrchr(pszNameA, '\\');

		if (pszSlash) pszNameA = pszSlash+1;

		if (!*pszNameA)
		{
			MBoxA(_T("Can't drop file! Filename is empty (A)!"));
			return NULL;
		}

		nSize += strlen(pszNameA)+1;
	}

	pszFullName = (wchar_t*)calloc(nSize, 2);
	wcscpy(pszFullName, abActive ? m_pfpi->pszActivePath : m_pfpi->pszPassivePath);
	wcscat(pszFullName, L"\\");

	if (abWide)
	{
		wcscpy(pszFullName+nPathLen, pszNameW);
	}
	else
	{
		MultiByteToWideChar(CP_ACP, 0, pszNameA, -1, pszFullName+nPathLen, nSize - nPathLen);
	}

	// ���� �����, ��������� ������� �����?
	WIN32_FIND_DATAW fnd = {0};
	HANDLE hFind = FindFirstFile(pszFullName, &fnd);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		FindClose(hFind);
		wchar_t* pszMsg = NULL;

		if (fnd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			pszMsg = (wchar_t*)calloc(nSize + 100,2);
			wcscpy(pszMsg, L"Can't create file! Same name folder exists!\n");
			wcscat(pszMsg, pszFullName);
			MessageBox(ghWnd, pszMsg, gpConEmu->GetDefaultTitle(), MB_ICONSTOP);
			free(pszMsg);
			free(pszFullName);
			return NULL;
		}
		else
		{
			pszMsg = (wchar_t*)calloc(nSize + 255,2);
			LARGE_INTEGER liSize;
			liSize.LowPart = fnd.nFileSizeLow; liSize.HighPart = fnd.nFileSizeHigh;
			FILETIME ftl;
			FileTimeToLocalFileTime(&fnd.ftLastWriteTime, &ftl);
			SYSTEMTIME st; FileTimeToSystemTime(&ftl, &st);
			wsprintf(pszMsg, L"File already exists!\n\n%s\nSize: %I64i\nDate: %02i.%02i.%i %02i:%02i:%02i\n\nOverwrite?",
			         pszFullName, liSize.QuadPart, st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond);
			int nRc = MessageBox(ghWnd, pszMsg, gpConEmu->GetDefaultTitle(), MB_ICONEXCLAMATION|MB_YESNO);
			free(pszMsg);

			if (nRc != IDYES)
			{
				free(pszFullName);
				return NULL;
			}

			// ����� ReadOnly, ��� �������������
			if (fnd.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM|FILE_ATTRIBUTE_READONLY))
				SetFileAttributes(pszFullName, FILE_ATTRIBUTE_NORMAL);
		}
	}

	return pszFullName;
}

HANDLE CDragDrop::FileStart(LPCWSTR pszFullName)
{
	_ASSERTE(pszFullName && *pszFullName);

	if (!pszFullName || !*pszFullName)
		return INVALID_HANDLE_VALUE;

	// ������� ����
	HANDLE hFile = CreateFile(pszFullName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		DWORD dwErr = GetLastError();
		int nSize = _tcslen(pszFullName);
		wchar_t* pszMsg = (wchar_t*)calloc(nSize + 100,2);
		wcscpy(pszMsg, L"Can't create file!\n");
		wcscat(pszMsg, pszFullName);
		DisplayLastError(pszMsg, dwErr);
		free(pszMsg);
		//free(pszFullName);
		return INVALID_HANDLE_VALUE;
	}

	//free(pszFullName);
	mn_CurWritten = 0;
	return hFile;
}

HRESULT CDragDrop::FileWrite(HANDLE ahFile, DWORD anSize, LPVOID apData)
{
	DWORD dwWrite = 0;

	if (!WriteFile(ahFile, apData, anSize, &dwWrite, NULL) || dwWrite!=anSize)
	{
		DWORD dwLastError = GetLastError();

		if (!dwLastError) dwLastError = E_UNEXPECTED;

		return dwLastError;
	}

	mn_CurWritten += anSize;
	wchar_t KM = L'K';
	__int64 nPrepared = mn_CurWritten / 1000;

	if (nPrepared > 9999)
	{
		nPrepared /= 1000; KM = L'M';

		if (nPrepared > 9999)
		{
			nPrepared /= 1000; KM = L'G';
		}
	}

	wchar_t temp[64];
	wsprintf(temp, L"Copying %i of %i (%u%c)", (mn_CurFile+1), mn_AllFiles, (DWORD)nPrepared, KM);
	SetWindowText(ghWnd, temp);
	return S_OK;
}

HRESULT CDragDrop::DropFromStream(IDataObject * pDataObject, BOOL abActive)
{
	STGMEDIUM stgMedium = { 0 };
	FORMATETC fmtetc = { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	HANDLE hFile = NULL;
#define BufferSize 0x10000
	BYTE cBuffer[BufferSize];
	DWORD dwRead = 0;
	BOOL lbWide = FALSE;
	HRESULT hr = S_OK;
	HRESULT hrStg = S_OK;
	// CF_HDROP � ��������� ����������!
	fmtetc.cfFormat = RegisterClipboardFormat(CFSTR_FILEDESCRIPTORW);
	TODO("�������������. ����� ���������� ��������� � �������� �����");

	// !! The caller then assumes responsibility for releasing the STGMEDIUM structure.
	if (S_OK == pDataObject->GetData(&fmtetc, &stgMedium))
	{
		lbWide = TRUE;
		HGLOBAL hDesc = stgMedium.hGlobal;
		FILEGROUPDESCRIPTORW *pDesc = (FILEGROUPDESCRIPTORW*)GlobalLock(hDesc);
		fmtetc.cfFormat = RegisterClipboardFormat(CFSTR_FILECONTENTS);
		mn_AllFiles = pDesc->cItems;

		for(mn_CurFile = 0; mn_CurFile<mn_AllFiles; mn_CurFile++)
		{
			fmtetc.lindex = mn_CurFile;
			wchar_t* pszNewFileName = FileCreateName(abActive, lbWide, pDesc->fgd[mn_CurFile].cFileName);

			if (pszNewFileName)
			{
				// !! The caller then assumes responsibility for releasing the STGMEDIUM structure.
				//���� if (S_OK == pDataObject->GetData(&fmtetc, &stgMedium) || !stgMedium.pstm)
				fmtetc.tymed = TYMED_FILE; // ������� ������� IStream
				stgMedium.tymed = TYMED_FILE;
				stgMedium.pstm = NULL;
				stgMedium.lpszFileName = ::SysAllocString(pszNewFileName);
				// ���������? �� ���� �� ��������. ���������� E_NOTIMPL
				hrStg = pDataObject->GetDataHere(&fmtetc, &stgMedium);
				::SysFreeString(stgMedium.lpszFileName);

				if (hrStg == S_OK)
				{
					free(pszNewFileName); pszNewFileName = NULL;
					continue;
				}

				fmtetc.tymed = TYMED_ISTREAM; // ������� ������� IStream
				stgMedium.tymed = TYMED_ISTREAM;
				stgMedium.pstm = NULL;
				hrStg = pDataObject->GetData(&fmtetc, &stgMedium);

				if (S_OK == hrStg && stgMedium.pstm)
				{
					IStream* pFile = stgMedium.pstm;

					if (!pszNewFileName)
						hFile = INVALID_HANDLE_VALUE;
					else
					{
						hFile = FileStart(pszNewFileName);
						free(pszNewFileName); pszNewFileName = NULL;
					}

					if (hFile != INVALID_HANDLE_VALUE)
					{
						// ����� ���������� � S_FALSE (����� �����?)
						while(SUCCEEDED(hr = pFile->Read(cBuffer, BufferSize, &dwRead)) && dwRead)
						{
							if (FileWrite(hFile, dwRead, cBuffer) != S_OK)
								break;
						}

						SafeCloseHandle(hFile);

						if (FAILED(hr))
							DisplayLastError(_T("Can't read medium!"), hr);
					}

					pFile->Release();
					continue;
				}

				if (pszNewFileName) free(pszNewFileName); pszNewFileName = NULL;
			}

			MBoxA(_T("Drag object does not contains known medium!"));
		}

		GlobalUnlock(hDesc);
		GlobalFree(hDesc);
		gpConEmu->DebugStep(NULL);
		return S_OK;
	}

	// Outlook 2k �������� ANSI!
	fmtetc.cfFormat = RegisterClipboardFormat(CFSTR_FILEDESCRIPTORA);

	// !! The caller then assumes responsibility for releasing the STGMEDIUM structure.
	if (S_OK == pDataObject->GetData(&fmtetc, &stgMedium))
	{
		lbWide = FALSE;
		HGLOBAL hDesc = stgMedium.hGlobal;
		FILEGROUPDESCRIPTORA *pDesc = (FILEGROUPDESCRIPTORA*)GlobalLock(hDesc);
		fmtetc.cfFormat = RegisterClipboardFormat(CFSTR_FILECONTENTS);
		mn_AllFiles = pDesc->cItems;

		for(mn_CurFile = 0; mn_CurFile<mn_AllFiles; mn_CurFile++)
		{
			fmtetc.lindex = mn_CurFile;
			wchar_t* pszNewFileName = FileCreateName(abActive, lbWide, pDesc->fgd[mn_CurFile].cFileName);

			if (pszNewFileName)
			{
				// !! The caller then assumes responsibility for releasing the STGMEDIUM structure.
				//���� if (S_OK == pDataObject->GetData(&fmtetc, &stgMedium) || !stgMedium.pstm)
				fmtetc.tymed = TYMED_ISTREAM; // ������� ������� IStream
				stgMedium.tymed = TYMED_ISTREAM;
				stgMedium.pstm = NULL;
				hrStg = pDataObject->GetData(&fmtetc, &stgMedium);

				if (S_OK == hrStg && stgMedium.pstm)
				{
					IStream* pFile = stgMedium.pstm;

					//hFile = FileStart(abActive, lbWide, pDesc->fgd[mn_CurFile].cFileName);
					if (!pszNewFileName)
						hFile = INVALID_HANDLE_VALUE;
					else
					{
						hFile = FileStart(pszNewFileName);
						free(pszNewFileName); pszNewFileName = NULL;
					}

					if (hFile != INVALID_HANDLE_VALUE)
					{
						// ����� ���������� � S_FALSE (����� �����?)
						while(SUCCEEDED(hr = pFile->Read(cBuffer, BufferSize, &dwRead)) && dwRead)
						{
							if (FileWrite(hFile, dwRead, cBuffer) != S_OK)
								break;
						}

						SafeCloseHandle(hFile);

						if (FAILED(hr))
							DisplayLastError(_T("Can't read medium!"), hr);
					}

					pFile->Release();
					continue;
				}

				if (pszNewFileName) free(pszNewFileName); pszNewFileName = NULL;
			}

			MBoxA(_T("Drag object does not contains known medium!"));
		}

		GlobalUnlock(hDesc);
		GlobalFree(hDesc);
		gpConEmu->DebugStep(NULL);
		return S_OK;
	}

	MBoxA(_T("Drag object does not contains known formats!"));
	return S_OK;
}

HRESULT CDragDrop::DropNames(HDROP hDrop, int iQuantity, BOOL abActive)
{
	wchar_t szMacro[MAX_DROP_PATH*2+50];
	wchar_t szData[MAX_DROP_PATH*2+10];
	wchar_t* pszText = NULL;
	// -- HRESULT hr = S_OK;
	DWORD dwStartTick = GetTickCount();
#define OPER_TIMEOUT 5000
	BOOL lbAddGoto = isPressed(VK_MENU);
	BOOL lbAddEdit = isPressed(VK_CONTROL);
	BOOL lbAddView = isPressed(VK_SHIFT);

	for(int i = 0 ; i < iQuantity; i++)
	{
		wcscpy(szMacro, L"$Text ");
		wcscpy(szData,  L"         ");
		pszText = szData + _tcslen(szData);
		int nLen = DragQueryFile(hDrop,i,pszText,MAX_DROP_PATH);

		if (nLen <= 0 || nLen >= MAX_DROP_PATH) continue;

		wchar_t* psz = pszText;
		// ��������� ������� � ������
		//while ((psz = wcschr(psz, L'"')) != NULL) {
		//	*psz = L'\'';
		//}
		nLen = _tcslen(psz);

		while(nLen>0 && *psz)
		{
			if (*psz == L'"' || *psz == L'\\')
			{
				wmemmove_s(psz+1, nLen+1, psz, nLen+1);

				if (*psz == L'"') *psz = L'\\';

				psz++;
			}

			psz++; nLen--;
		}

		if ((psz = wcschr(pszText, L' ')) != NULL)
		{
			// ��� ����� ���������
			*(--pszText) = L'\"';
			*(--pszText) = L'\\';
			wcscat(pszText, L"\\\"");
		}

		wcscat(szMacro, L"\"");

		if (lbAddGoto || lbAddEdit || lbAddView)
		{
			if (lbAddGoto)
				wcscat(szMacro, L"goto:");

			if (lbAddEdit)
				wcscat(szMacro, L"edit:");

			if (lbAddView)
				wcscat(szMacro, L"view:");

			lbAddGoto = FALSE; lbAddEdit = FALSE; lbAddView = FALSE;
		}

		wcscat(szMacro, pszText);
		wcscat(szMacro, L" \"");
		gpConEmu->PostMacro(szMacro);

		if (((i + 1) < iQuantity)
		        && (GetTickCount() - dwStartTick) >= OPER_TIMEOUT)
		{
			BOOL b = gbDontEnable; gbDontEnable = TRUE;
			int nBtn = MessageBox(ghWnd, L"Drop operation is too long. Continue?", gpConEmu->GetDefaultTitle(), MB_YESNO|MB_ICONEXCLAMATION);
			gbDontEnable = b;

			if (nBtn != IDYES) break;

			dwStartTick = GetTickCount();
		}
	}

	GlobalFree(hDrop);
	return S_OK;
}

HRESULT CDragDrop::DropLinks(HDROP hDrop, int iQuantity, BOOL abActive)
{
	TCHAR curr[MAX_DROP_PATH];
	LPCTSTR pszTo = abActive ? m_pfpi->pszActivePath : m_pfpi->pszPassivePath;
	int nToLen = _tcslen(pszTo);
	HRESULT hr = S_OK;

	for(int i = 0 ; i < iQuantity; i++)
	{
		int nLen = DragQueryFile(hDrop,i,curr,MAX_DROP_PATH);

		if (nLen <= 0 || nLen >= MAX_DROP_PATH) continue;

		LPCTSTR pszTitle = _tcsrchr(curr, _T('\\'));
		if (pszTitle) pszTitle++; else pszTitle = curr;

		nLen = nToLen+2+_tcslen(pszTitle)+4;
		TCHAR *szLnkPath = new TCHAR[nLen];
		_tcscpy(szLnkPath, pszTo);

		if (szLnkPath[_tcslen(szLnkPath)-1] != _T('\\'))
			_tcscat(szLnkPath, _T("\\"));

		_tcscat(szLnkPath, pszTitle);
		_tcscat(szLnkPath, _T(".lnk"));

		try
		{
			hr = CreateLink(curr, szLnkPath, pszTitle);
		}
		catch(...)
		{
			hr = E_UNEXPECTED;
		}

		if (hr!=S_OK)
			DisplayLastError(_T("Can't create link!"), hr);

		delete szLnkPath;
	}

	GlobalFree(hDrop);
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CDragDrop::Drop(IDataObject * pDataObject,DWORD grfKeyState,POINTL pt,DWORD * pdwEffect)
{
	mb_DragWithinNow = FALSE;
	DestroyDragImageBits();
	DestroyDragImageWindow();
	gpConEmu->SetDragCursor(NULL);
	//PostMessage(ghWnd, WM_SETCURSOR, -1, -1);
	DWORD dwAllowed = *pdwEffect;
	*pdwEffect = DROPEFFECT_COPY|DROPEFFECT_MOVE;

	if (S_OK != DragOver(grfKeyState, pt, pdwEffect) ||  *pdwEffect == DROPEFFECT_NONE)
	{
		return S_OK;
	}

	// ����������, �� ����� ������ �������
	ScreenToClient(ghWndDC, (LPPOINT)&pt);
	COORD cr = gpConEmu->ActiveCon()->ClientToConsole(pt.x, pt.y);
	pt.x = cr.X; pt.y = cr.Y;
	//pt.x/=gpSet->Log Font.lfWidth;
	//pt.y/=gpSet->Log Font.lfHeight;
	BOOL lbActive = PtInRect(&(m_pfpi->ActiveRect), *(LPPOINT)&pt);
	// ������ � ��������� ������ (��� � ��������, �����...)
	BOOL lbDropFileNamesOnly = FALSE;

	if ((m_pfpi->ActiveRect.bottom && pt.y > m_pfpi->ActiveRect.bottom) ||
	        (m_pfpi->PassiveRect.bottom && pt.y > m_pfpi->PassiveRect.bottom))
	{
		lbDropFileNamesOnly = TRUE;
	}

	// ���� ����� ������ ����� �������� - ����� � FAR
	if (!lbDropFileNamesOnly && !lbActive && mb_selfdrag
	        && (*pdwEffect == DROPEFFECT_COPY || *pdwEffect == DROPEFFECT_MOVE))
	{
		wchar_t *mcr = (wchar_t*)calloc(128, sizeof(wchar_t));
		//2010-02-18 �� ���� �������� '@'
		//2010-03-26 ������� '@' ������� ������, ��� ����� �������� ����������� ����� �� ����� ��� ���������� �������������
		// ���� ������ ".." �� ����� ������������ �� ������ ������ ������� ���������� ����� �� ������� �������
		lstrcpyW(mcr, L"$If (APanel.SelCount==0 && APanel.Current==\"..\") CtrlPgUp $End ");
		// ������ ���������� ������� �������
		lstrcatW(mcr, (*pdwEffect == DROPEFFECT_MOVE) ? L"F6" : L"F5");

		// � ���� ������� ���������� ����� ��� �������������
		if (gpSet->isDropEnabled==2)
			lstrcatW(mcr, L" Enter "); //$MMode 1");

		gpConEmu->PostCopy(mcr);
		return S_OK; // ����� ������ ����
	}

	STGMEDIUM stgMedium = { 0 };
	FORMATETC fmtetc = { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	// !! The caller then assumes responsibility for releasing the STGMEDIUM structure.
	HRESULT hr = pDataObject->GetData(&fmtetc, &stgMedium);
	HDROP hDrop = NULL;

	if (hr != S_OK || !stgMedium.hGlobal)
	{
		if (mb_selfdrag || lbDropFileNamesOnly)
		{
			MBoxA(_T("Drag object does not contains known formats!"));
			return S_OK;
		}

		// ��������� Drop �� Outlook, � ��. ShellExtensions (����� IStream)
		hr = DropFromStream(pDataObject, lbActive);
		return hr;
	}

	STGMEDIUM stgMediumMap = { 0 };
	FORMATETC fmtetcMap = { RegisterClipboardFormat(CFSTR_FILENAMEMAPW), 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	hDrop = (HDROP)stgMedium.hGlobal;
	int iQuantity = DragQueryFile(hDrop,0xFFFFFFFF,NULL,NULL);

	if (iQuantity < 1)
	{
		GlobalFree(stgMedium.hGlobal);
		return S_OK; // ������ ���, �������
	}

	gpConEmu->DebugStep(_T("DnD: Drop starting"));

	if (lbDropFileNamesOnly)
	{
		// GlobalFree �������� �������
		hr = DropNames(hDrop, iQuantity, lbActive);
		return hr;
	}

	// ���� ��������� ����� - ������ ����� � �������
	if (*pdwEffect == DROPEFFECT_LINK)
	{
		// GlobalFree �������� �������
		hr = DropLinks(hDrop, iQuantity, lbActive);
		return hr;
	}

	{
		//SHFILEOPSTRUCT *fop  = new SHFILEOPSTRUCT;
		ShlOpInfo *sfop = new ShlOpInfo;
		memset(sfop, 0, sizeof(ShlOpInfo));
		sfop->fop.hwnd = ghWnd;
		sfop->pDnD = this;
		hr = pDataObject->GetData(&fmtetcMap, &stgMediumMap);
		BOOL lbMultiDest = (hr == S_OK && stgMediumMap.hGlobal);
		TODO("���������� stgMediumMap");
		LPCWSTR pszFileMap = (LPCWSTR)GlobalLock(stgMediumMap.hGlobal);

		if (!pszFileMap) lbMultiDest = FALSE;

		LPCWSTR pszDropPath = lbActive ? m_pfpi->pszActivePath : m_pfpi->pszPassivePath;
		int nCount = MAX_DROP_PATH*iQuantity+iQuantity+1;
		int nDstCount = 0;
		int nDstFolderLen = 0;

		if (!lbMultiDest)
		{
			sfop->fop.pTo=new WCHAR[lstrlenW(pszDropPath)+3];
			wsprintf((LPWSTR)sfop->fop.pTo, _T("%s\\\0\0"), pszDropPath);
		}
		else
		{
			nDstFolderLen = lstrlenW(pszDropPath);
			nDstCount = iQuantity*(nDstFolderLen+3+MAX_PATH)+iQuantity+1;
			sfop->fop.pTo=new WCHAR[nDstCount];
			ZeroMemory((void*)sfop->fop.pTo,sizeof(WCHAR)*nDstCount);
		}

		sfop->fop.pFrom=new WCHAR[nCount];
		ZeroMemory((void*)sfop->fop.pFrom,sizeof(WCHAR)*nCount);
		WCHAR *curr = (WCHAR*)sfop->fop.pFrom;
		WCHAR *dst = lbMultiDest ? (WCHAR*)sfop->fop.pTo : NULL;

		for(int i = 0 ; i < iQuantity; i++)
		{
			DragQueryFile(hDrop,i,curr,MAX_DROP_PATH);
			curr+=_tcslen(curr)+1;

			if (lbMultiDest)
			{
				lstrcpy(dst, pszDropPath);
				dst += nDstFolderLen;

				if (*(dst-1) != L'\\')
				{
					*dst++ = L'\\'; *dst = 0;
				}

				if (pszFileMap && *pszFileMap)
				{
					int nNameLen = _tcslen(pszFileMap);
					lstrcpyn(dst, pszFileMap, MAX_PATH);
					pszFileMap += nNameLen+1;
					dst += _tcslen(dst)+1;
					MCHKHEAP;
				}
			}
		}

		MCHKHEAP;
		GlobalFree(stgMedium.hGlobal); hDrop = NULL;

		if (stgMediumMap.hGlobal)
		{
			if (pszFileMap) GlobalUnlock(stgMediumMap.hGlobal);

			GlobalFree(stgMediumMap.hGlobal);
			stgMediumMap.hGlobal = NULL;
		}

		if (*pdwEffect == DROPEFFECT_MOVE)
			sfop->fop.wFunc=FO_MOVE;
		else
			sfop->fop.wFunc=FO_COPY;

		sfop->fop.fFlags = lbMultiDest ? FOF_MULTIDESTFILES : 0;
		//sfop->fop.fFlags=FOF_SIMPLEPROGRESS; -- ����� ��������� ����������
		gpConEmu->DebugStep(_T("DnD: Shell operation starting"));
		ThInfo th;
		th.hThread = CreateThread(NULL, 0, CDragDrop::ShellOpThreadProc, sfop, 0, &th.dwThreadId);

		if (th.hThread == NULL)
		{
			DisplayLastError(_T("Can't create shell operation thread!"));
		}
		else
		{
			TryEnterCriticalSection(&m_CrThreads);
			m_OpThread.push_back(th);
			LeaveCriticalSection(&m_CrThreads);
		}

		gpConEmu->DebugStep(NULL);
	}

	return S_OK; //1;
}

DWORD CDragDrop::ShellOpThreadProc(LPVOID lpParameter)
{
	DWORD dwThreadId = GetCurrentThreadId();
	ShlOpInfo *sfop = (ShlOpInfo *)lpParameter;
	HRESULT hr = S_OK;
	CDragDrop* pDragDrop = sfop->pDnD;

	// ���������� �������� �����������/�����������...
	try
	{
		//-- �� ����������! ����� ���� ���������� FOF_MULTIDESTFILES
		//sfop->fop.fFlags = 0; //FOF_SIMPLEPROGRESS; -- ����� ������ ���� ����������
		hr = SHFileOperation(&(sfop->fop));
	}
	catch(...)
	{
		hr = E_UNEXPECTED;
	}

	if (hr != S_OK)
	{
		if (hr == 7 || hr == ERROR_CANCELLED)
			MBoxA(_T("Shell operation was cancelled"))
			else
				DisplayLastError(_T("Shell operation failed"), hr);
	}

	if (sfop->fop.pTo) delete sfop->fop.pTo;

	if (sfop->fop.pFrom) delete sfop->fop.pFrom;

	delete sfop;
	TryEnterCriticalSection(&pDragDrop->m_CrThreads);
	std::vector<ThInfo>::iterator iter = pDragDrop->m_OpThread.begin();

	while(iter != pDragDrop->m_OpThread.end())
	{
		if (dwThreadId == iter->dwThreadId)
		{
			CloseHandle(iter->hThread);
			iter = pDragDrop->m_OpThread.erase(iter);
			break;
		}
	}

	LeaveCriticalSection(&pDragDrop->m_CrThreads);
	return 0;
}

HRESULT STDMETHODCALLTYPE CDragDrop::DragOver(DWORD grfKeyState,POINTL pt,DWORD * pdwEffect)
{
	HRESULT hr = S_OK;
	DWORD dwAllowed = *pdwEffect;

	if (!gpSet->isDropEnabled && !gpConEmu->isDragging())
	{
		*pdwEffect = DROPEFFECT_NONE;
		gpConEmu->DebugStep(_T("DnD: Drop disabled"));
		hr = S_FALSE;
	}
	else if (m_pfpi==NULL)
	{
		*pdwEffect = DROPEFFECT_NONE;
	}
	else
	{
		TODO("���� drop ���� ��� ������ - ��������� ���� � ��������� ������");
		//gpConEmu->DebugStep(_T("DnD: DragOver starting"));
		POINT ptCur; ptCur.x = pt.x; ptCur.y = pt.y;
#ifdef _DEBUG
		GetCursorPos(&ptCur);
#endif
		RECT rcDC; GetWindowRect(ghWndDC, &rcDC);

		//HWND hWndFrom = WindowFromPoint(ptCur);
		if (/*(hWndFrom != ghWnd && hWndFrom != mh_Overlapped)
			||*/ !PtInRect(&rcDC, ptCur))
		{
			*pdwEffect = DROPEFFECT_NONE;
		}
		else
		{
			ScreenToClient(ghWndDC, (LPPOINT)&pt);
			COORD cr = gpConEmu->ActiveCon()->ClientToConsole(pt.x, pt.y);
			pt.x = cr.X; pt.y = cr.Y;
			//pt.x/=gpSet->Log Font.lfWidth;
			//pt.y/=gpSet->Log Font.lfHeight;
			BOOL lbActive = FALSE, lbPassive = FALSE;
			BOOL lbAllowToActive = // ����� �� ������� �� �������� ������
			    (!mb_selfdrag) // ���� ����� �������
			    || (grfKeyState & MK_ALT) // ��� ����� Alt
			    || (grfKeyState == MK_RBUTTON); // ��� ����� ������ ������� ��� �������������

			if (((lbActive = PtInRect(&(m_pfpi->ActiveRect), *(LPPOINT)&pt)) && m_pfpi->pszActivePath[0] && lbAllowToActive) ||
			        ((lbPassive = PtInRect(&(m_pfpi->PassiveRect), *(LPPOINT)&pt)) && (m_pfpi->pszPassivePath[0] || mb_selfdrag)))
			{
				if (grfKeyState & MK_CONTROL)
					*pdwEffect = DROPEFFECT_COPY;
				else if (grfKeyState & MK_SHIFT)
					*pdwEffect = DROPEFFECT_MOVE;
				else if (grfKeyState & (MK_ALT | MK_RBUTTON))
					*pdwEffect = DROPEFFECT_LINK;
				else if (gpConEmu->mouse.state & DRAG_R_STARTED)
					*pdwEffect = DROPEFFECT_LINK; // ��� Drop - ������ ������ ��� ��������
				else
				{
					// ������� �� ���������� �������, ������������ ���������� (����� ���� �� ������� �� ��������)
					*pdwEffect = (gpSet->isDefCopy && (dwAllowed&DROPEFFECT_COPY)==DROPEFFECT_COPY) ? DROPEFFECT_COPY : DROPEFFECT_MOVE;
				}

				if (*pdwEffect == DROPEFFECT_LINK && lbPassive && m_pfpi->pszPassivePath[0] == 0)
					*pdwEffect = DROPEFFECT_NONE;
			}
			else if ((m_pfpi->ActiveRect.bottom && pt.y > m_pfpi->ActiveRect.bottom) ||
			        (m_pfpi->PassiveRect.bottom && pt.y > m_pfpi->PassiveRect.bottom))
			{
				*pdwEffect = DROPEFFECT_COPY;
			}
			else
			{
				*pdwEffect = DROPEFFECT_NONE;
			}
		}
	}

	if (!mb_selfdrag)
	{
		if (*pdwEffect == DROPEFFECT_NONE)
		{
			MoveDragWindow(FALSE);
		}
		else if (mh_Overlapped && mp_Bits)
		{
			MoveDragWindow();
		}
		else if (mp_Bits)
		{
			CreateDragImageWindow();
		}
	}

	//gpConEmu->DebugStep(_T("DnD: DragOver ok"));
	return hr;
}


void CDragDrop::EnumDragFormats(IDataObject * pDataObject)
{
	//BOOL lbDoEnum = FALSE;
	//if (!lbDoEnum) return;
	HRESULT hr = S_OK;
	IEnumFORMATETC *pEnum = NULL;
	FORMATETC fmt[20];
	STGMEDIUM stg[20];
	LPCWSTR psz[20];
	SIZE_T memsize[20];
	TCHAR szName[20][MAX_PATH*2];
	ULONG nCnt = sizeof(fmt)/sizeof(*fmt);
	UINT i;
	memset(fmt, 0, sizeof(fmt));
	memset(stg, 0, sizeof(stg));
	memset(psz, 0, sizeof(psz));
	hr = pDataObject->EnumFormatEtc(DATADIR_GET,&pEnum);

	if (hr==S_OK)
	{
		hr = pEnum->Next(nCnt, fmt, &nCnt);
		pEnum->Release();

		/*
		CFSTR_PREFERREDDROPEFFECT ?
		*/
		for(i=0; i<nCnt; i++)
		{
			szName[i][0] = 0;

			if (!GetClipboardFormatName(fmt[i].cfFormat, szName[i], MAX_PATH))
			{
				switch(fmt[i].cfFormat)
				{
					case 1: lstrcpy(szName[i], L"CF_TEXT"); break;
					case 2: lstrcpy(szName[i], L"CF_BITMAP"); break;
					case 3: lstrcpy(szName[i], L"CF_METAFILEPICT"); break;
					case 4: lstrcpy(szName[i], L"CF_SYLK"); break;
					case 5: lstrcpy(szName[i], L"CF_DIF"); break;
					case 6: lstrcpy(szName[i], L"CF_TIFF"); break;
					case 7: lstrcpy(szName[i], L"CF_OEMTEXT"); break;
					case 8: lstrcpy(szName[i], L"CF_DIB"); break;
					case 9: lstrcpy(szName[i], L"CF_PALETTE"); break;
					case 10: lstrcpy(szName[i], L"CF_PENDATA"); break;
					case 11: lstrcpy(szName[i], L"CF_RIFF"); break;
					case 12: lstrcpy(szName[i], L"CF_WAVE"); break;
					case 13: lstrcpy(szName[i], L"CF_UNICODETEXT"); break;
					case 14: lstrcpy(szName[i], L"CF_ENHMETAFILE"); break;
					case 15: lstrcpy(szName[i], L"CF_HDROP"); break;
					case 16: lstrcpy(szName[i], L"CF_LOCALE"); break;
					case 17: lstrcpy(szName[i], L"CF_DIBV5"); break;
					case 0x0080: lstrcpy(szName[i], L"CF_OWNERDISPLAY"); break;
					case 0x0081: lstrcpy(szName[i], L"CF_DSPTEXT"); break;
					case 0x0082: lstrcpy(szName[i], L"CF_DSPBITMAP"); break;
					case 0x0083: lstrcpy(szName[i], L"CF_DSPMETAFILEPICT"); break;
					case 0x008E: lstrcpy(szName[i], L"CF_DSPENHMETAFILE"); break;
					default: wsprintf(szName[i], L"ClipFormatID=%i", fmt[i].cfFormat);
				}
			}

			stg[i].tymed = TYMED_HGLOBAL;
			// !! The caller then assumes responsibility for releasing the STGMEDIUM structure.
			hr = pDataObject->GetData(fmt+i, stg+i);

			if (hr == S_OK && stg[i].hGlobal)
			{
				psz[i] = (LPCWSTR)GlobalLock(stg[i].hGlobal);

				if (psz[i])
				{
					memsize[i] = GlobalSize(stg[i].hGlobal);
					wsprintf(szName[i]+_tcslen(szName[i]), L", DataSize=%i", memsize[i]);

					if (memsize[i] == 1)
					{
						wsprintf(szName[i]+_tcslen(szName[i]), L", Data=0x%02X", (DWORD)*((LPBYTE)(psz[i])));
					} if (memsize[i] == 4)

					{
						wsprintf(szName[i]+_tcslen(szName[i]), L", Data=0x%08X", (DWORD)*((LPDWORD)(psz[i])));
					}
					else if (memsize[i] == 8)
					{
						wsprintf(szName[i]+_tcslen(szName[i]), L", Data=0x%08X%08X", (DWORD)((LPDWORD)(psz[i]))[0], (DWORD)((LPDWORD)psz[i])[1]);
					}
					else
					{
						lstrcat(szName[i], L", ");
						const wchar_t* pwsz = (const wchar_t*)(psz[i]);
						const char* pasz = (const char*)(psz[i]);

						if (pasz[0] && pasz[1])
						{
							int nMaxLen = min(200,memsize[i]);
							wchar_t* pwszDst = szName[i]+_tcslen(szName[i]);
							MultiByteToWideChar(CP_ACP, 0, pasz, nMaxLen, pwszDst, nMaxLen);
							pwszDst[nMaxLen] = 0;
						}
						else
						{
							int nMaxLen = min(200,memsize[i]/2);
							lstrcpyn(szName[i]+_tcslen(szName[i]), pwsz, nMaxLen);
						}
					}
				}
				else
				{
					lstrcat(szName[i], L", hGlobal not available");
					stg[i].hGlobal = NULL;
				}
			}
			else
			{
				lstrcat(szName[i], L", hGlobal not available");
				stg[i].hGlobal = NULL;
			}

#ifdef _DEBUG

			if (wcscmp(szName[i], L"DragImageBits") == 0)
			{
				stg[i].tymed = TYMED_HGLOBAL;
			}

#endif
			lstrcat(szName[i], L"\n");
			OutputDebugStringW(szName[i]);
		}

		for(i=0; i<nCnt; i++)
		{
			if (psz[i] && stg[i].hGlobal)
			{
				GlobalUnlock(stg[i].hGlobal);
				GlobalFree(stg[i].hGlobal);
			}
		}
	}

	hr = S_OK;
}


HRESULT STDMETHODCALLTYPE CDragDrop::DragEnter(IDataObject * pDataObject,DWORD grfKeyState,POINTL pt,DWORD * pdwEffect)
{
	mb_selfdrag = (pDataObject == mp_DataObject);
	mb_DragWithinNow = TRUE;

	if (gbDebugLogStarted || IsDebuggerPresent())
		EnumDragFormats(pDataObject);

	if (gpSet->isDropEnabled || mb_selfdrag)
	{
		if (!mb_selfdrag)
		{
			// ��� "�����" ����� - ���������� ��� ��������
			RetrieveDragToInfo(pDataObject);

			if (LoadDragImageBits(pDataObject))
				CreateDragImageWindow();
		}
	}
	else
	{
		gpConEmu->DebugStep(_T("DnD: Drop disabled"));
	}

	return 0;
}

void CDragDrop::RetrieveDragToInfo(IDataObject * pDataObject)
{
	CConEmuPipe pipe(gpConEmu->GetFarPID(), CONEMUREADYTIMEOUT);
	gpConEmu->DebugStep(_T("DnD: DragEnter starting"));

	if (pipe.Init(_T("CDragDrop::DragEnter")))
	{
		if (pipe.Execute(CMD_DRAGTO))
		{
			DWORD cbBytesRead=0;
			int cbStructSize=0;

			if (m_pfpi) {free(m_pfpi); m_pfpi=NULL;}

			if (pipe.Read(&cbStructSize, sizeof(int), &cbBytesRead))
			{
				if ((DWORD)cbStructSize>sizeof(ForwardedPanelInfo))
				{
					m_pfpi = (ForwardedPanelInfo*)calloc(cbStructSize, 1);
					pipe.Read(m_pfpi, cbStructSize, &cbBytesRead);
					m_pfpi->pszActivePath = (WCHAR*)(((char*)m_pfpi)+m_pfpi->ActivePathShift);
					//Slash �� ����� ��� �� �����
					int nPathLen = lstrlenW(m_pfpi->pszActivePath);

					if (nPathLen>0 && m_pfpi->pszActivePath[nPathLen-1]==_T('\\'))
						m_pfpi->pszActivePath[nPathLen-1] = 0;

					m_pfpi->pszPassivePath = (WCHAR*)(((char*)m_pfpi)+m_pfpi->PassivePathShift);
					//Slash �� ����� ��� �� �����
					nPathLen = lstrlenW(m_pfpi->pszPassivePath);

					if (nPathLen>0 && m_pfpi->pszPassivePath[nPathLen-1]==_T('\\'))
						m_pfpi->pszPassivePath[nPathLen-1] = 0;
				}
			}
		}
	}

	gpConEmu->DebugStep(NULL);
}

HRESULT STDMETHODCALLTYPE CDragDrop::DragLeave(void)
{
	mb_DragWithinNow = FALSE;

	if (mb_selfdrag)
	{
		MoveDragWindow(FALSE);
	}
	else
	{
		DestroyDragImageBits();
		DestroyDragImageWindow();
	}

	return 0;
}

// CreateLink - uses the Shell's IShellLink and IPersistFile interfaces
//              to create and store a shortcut to the specified object.
//
// Returns the result of calling the member functions of the interfaces.
//
// Parameters:
// lpszPathObj  - address of a buffer containing the path of the object.
// lpszPathLink - address of a buffer containing the path where the
//                Shell link is to be stored.
// lpszDesc     - address of a buffer containing the description of the
//                Shell link.

HRESULT CDragDrop::CreateLink(LPCTSTR lpszPathObj, LPCTSTR lpszPathLink, LPCTSTR lpszDesc)
{
	HRESULT hres = S_OK;
	IShellLink* psl = NULL;
	// Get a pointer to the IShellLink interface.
	hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
	                        IID_IShellLink, (LPVOID*)&psl);

	if (SUCCEEDED(hres))
	{
		IPersistFile* ppf;
		// Set the path to the shortcut target and add the description.
		hres = psl->SetPath(lpszPathObj);
		hres = psl->SetDescription(lpszDesc);
		// Query IShellLink for the IPersistFile interface for saving the
		// shortcut in persistent storage.
		hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);

		if (SUCCEEDED(hres))
		{
			//WCHAR wsz[MAX_PATH];
			// Ensure that the string is Unicode.
			//MultiByteToWideChar(CP_ACP, 0, lpszPathLink, -1, wsz, MAX_PATH);
			// Add code here to check return value from MultiByteWideChar
			// for success.
			// Save the link by calling IPersistFile::Save.
			hres = ppf->Save(lpszPathLink, TRUE);
			ppf->Release();
		}

		psl->Release();
	}

	return hres;
}

BOOL CDragDrop::CreateDragImageBits(IDataObject * pDataObject)
{
	DEBUGSTROVL(L"CreateDragImageBits(IDataObject) - NOT IMPLEMENTED\n");
	return FALSE;
}

#include <pshpack1.h>
typedef struct tag_MyRgbQuad
{
	union
	{
		DWORD dwValue;
		struct
		{
			BYTE    rgbBlue;
			BYTE    rgbGreen;
			BYTE    rgbRed;
			BYTE    rgbAlpha;
		};
	};
} MyRgbQuad;
#include <poppack.h>

DragImageBits* CDragDrop::CreateDragImageBits(wchar_t* pszFiles)
{
	if (!pszFiles)
	{
		DEBUGSTROVL(L"CreateDragImageBits failed, pszFiles is NULL\n");
		_ASSERTE(pszFiles!=NULL);
		return NULL;
	}

	DestroyDragImageBits();
	DestroyDragImageWindow();
	DEBUGSTROVL(L"CreateDragImageBits()\n");
	DragImageBits* pBits = NULL;
	HDC hScreenDC = ::GetDC(0);
	HDC hDrawDC = CreateCompatibleDC(hScreenDC);
	HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, MAX_OVERLAY_WIDTH, MAX_OVERLAY_HEIGHT);

	if (hDrawDC && hBitmap)
	{
		HFONT hOldF = NULL, hf = CreateFont(14, 0, 0, 0, 400, 0, 0, 0, CP_ACP, 0, 0, 0, 0, L"Tahoma");

		if (hf) hOldF = (HFONT)SelectObject(hDrawDC, hf);

		int nMaxX = 0, nMaxY = 0, nX = 0;
		// GetTextExtentPoint32 ������-�� ������ DC, ������� ������ ������� �������
		wchar_t *psz = pszFiles; int nFilesCol = 0;

		while(*psz)
		{
			SIZE sz = {0};
			LPCWSTR pszText = wcsrchr(psz, L'\\');
			if (!pszText) pszText = psz; else psz++;

			GetTextExtentPoint32(hDrawDC, pszText, _tcslen(pszText), &sz);

			if (sz.cx > MAX_OVERLAY_WIDTH)
				sz.cx = MAX_OVERLAY_WIDTH;

			if (sz.cx > nMaxX)
				nMaxX = sz.cx;

			psz += _tcslen(psz)+1; // ����� ������� ���� � ����� ����� ����� ������ ;)
			nFilesCol ++;
		}

		nMaxX = min((OVERLAY_TEXT_SHIFT + nMaxX),MAX_OVERLAY_WIDTH);
		// ���� ����� ����� ������/����� - ����� ����������� ���������� �� � ��������� �������
		int nColCount = 1;

		if (nFilesCol > 3)
		{
			if (nFilesCol > 21 && (nMaxX * 3 + 32) <= MAX_OVERLAY_WIDTH)
			{
				nFilesCol = (nFilesCol+3) / (nColCount = 4);
			}
			else if (nFilesCol > 12 && (nMaxX * 2 + 32) <= MAX_OVERLAY_WIDTH)
			{
				nFilesCol = (nFilesCol+2) / (nColCount = 3);
			}
			else if ((nMaxX + 32) <= MAX_OVERLAY_WIDTH)
			{
				nFilesCol = (nFilesCol+1) / (nColCount = 2);
			}
		}

		HBITMAP hOldBitmap = (HBITMAP)SelectObject(hDrawDC, hBitmap);
		SetTextColor(hDrawDC, RGB(0,0,128)); // ����� ����� �����
		//SetTextColor(hDrawDC, RGB(255,255,255)); // ����� �����
		SetBkColor(hDrawDC, RGB(192,192,192)); // �� ����� ����
		SetBkMode(hDrawDC, OPAQUE);
		// DC ����� ���� �������� (������?), ������� ����� �������� ���
		RECT rcFill = MakeRect(MAX_OVERLAY_WIDTH,MAX_OVERLAY_HEIGHT);
#ifdef _DEBUG
		FillRect(hDrawDC, &rcFill, (HBRUSH)GetStockObject(BLACK_BRUSH/*WHITE_BRUSH*/));
#else
		FillRect(hDrawDC, &rcFill, (HBRUSH)GetStockObject(BLACK_BRUSH));
#endif
		//DumpImage(hDrawDC, MAX_OVERLAY_WIDTH, nMaxY, L"F:\\Dump.bmp");
		psz = pszFiles;
		int nFileIdx = 0, nColMaxY = 0, nAllFiles = 0;
		nMaxX = 0;
		int nFirstColWidth = 0;

		while(*psz)
		{
			if (!DrawImageBits(hDrawDC, psz, &nMaxX, nX, &nColMaxY))
				break; // ����� �� ������� MAX_OVERLAY_WIDTH x MAX_OVERLAY_HEIGHT (�� ������)

			psz += _tcslen(psz)+1;

			if (!*psz) break;

			nFileIdx ++; nAllFiles ++;

			if (nFileIdx >= nFilesCol)
			{
				if (!nFirstColWidth)
					nFirstColWidth = nMaxX + OVERLAY_TEXT_SHIFT;

				if ((nX + nMaxX + 32) >= MAX_OVERLAY_WIDTH)
					break;

				nX += nMaxX + OVERLAY_TEXT_SHIFT + OVERLAY_COLUMN_SHIFT;

				if (nColMaxY > nMaxY) nMaxY = nColMaxY;

				nColMaxY = nMaxX = nFileIdx = 0;
			}
		}

		if (nColMaxY > nMaxY) nMaxY = nColMaxY;

		if (!nFirstColWidth) nFirstColWidth = nMaxX + OVERLAY_TEXT_SHIFT;

		int nLineX = ((nX+nMaxX+OVERLAY_TEXT_SHIFT+3)>>2)<<2;

		if (nLineX > MAX_OVERLAY_WIDTH) nLineX = MAX_OVERLAY_WIDTH;

		SelectObject(hDrawDC, hOldF);
		GdiFlush();
#ifdef _DEBUG
		DumpImage(hDrawDC, MAX_OVERLAY_WIDTH, nMaxY, L"F:\\Dump.bmp");
#endif

		if (nLineX>2 && nMaxY>2)
		{
			HDC hBitsDC = CreateCompatibleDC(hScreenDC);

			if (hBitsDC && hDrawDC)
			{
				SetLayout(hBitsDC, LAYOUT_BITMAPORIENTATIONPRESERVED);
				BITMAPINFOHEADER bih = {sizeof(BITMAPINFOHEADER)};
				bih.biWidth = nLineX;
				bih.biHeight = nMaxY;
				bih.biPlanes = 1;
				bih.biBitCount = 24;
				bih.biCompression = BI_RGB;
				LPBYTE pSrc = NULL;
				HBITMAP hBitsBitmap = CreateDIBSection(hScreenDC, (BITMAPINFO*)&bih, DIB_RGB_COLORS, (void**)&pSrc, NULL, 0);

				if (hBitsBitmap)
				{
					HBITMAP hOldBitsBitmap = (HBITMAP)SelectObject(hBitsDC, hBitsBitmap);
					BitBlt(hBitsDC, 0,0,nLineX,nMaxY, hDrawDC,0,0, SRCCOPY);
					GdiFlush();
					DragImageBits* pDst = (DragImageBits*)GlobalAlloc(GPTR, sizeof(DragImageBits) + (nLineX*nMaxY - 1)*4);

					if (pDst)
					{
						pDst->nWidth = nLineX;
						pDst->nHeight = nMaxY;

						if (nColCount == 1)
							pDst->nXCursor = nMaxX / 2;
						else
							pDst->nXCursor = nFirstColWidth;

						pDst->nYCursor = 17; // ��� ������ �������
						pDst->nRes1 = GetTickCount(); // ���-�� ����������. Random?
						pDst->nRes2 = 0xFFFFFFFF;
						MyRgbQuad *pRGB = (MyRgbQuad*)pDst->pix;
						u32 nCurBlend = 0xAA, nAllBlend = 0xAA;

						for(int y = 0; y < nMaxY; y++)
						{
							for(int x = 0; x < nLineX; x++)
							{
								pRGB->rgbBlue = *(pSrc++);
								pRGB->rgbGreen = *(pSrc++);
								pRGB->rgbRed = *(pSrc++);

								if (pRGB->dwValue)
								{
									pRGB->rgbBlue = klMulDivU32(pRGB->rgbBlue, nCurBlend, 0xFF);
									pRGB->rgbGreen = klMulDivU32(pRGB->rgbGreen, nCurBlend, 0xFF);
									pRGB->rgbRed = klMulDivU32(pRGB->rgbRed, nCurBlend, 0xFF);
									pRGB->rgbAlpha = nAllBlend;
								}

								pRGB++;
							}
						}

						mp_Bits = (DragImageBits*)calloc(sizeof(DragImageBits)/*+(nCount-1)*4*/,1);

						if (mp_Bits)
						{
							*mp_Bits = *pDst;
							pBits = pDst; pDst = NULL; //OK
							mh_BitsDC = hBitsDC; hBitsDC = NULL;
							mh_BitsBMP = hBitsBitmap; hBitsBitmap = NULL;
						}
						else
						{
							if (pDst) GlobalFree(pDst); pDst = NULL;  // ������
						}
					}
					else
					{
						if (pDst) GlobalFree(pDst); pDst = NULL;
					}

					if (hBitsDC)
						SelectObject(hBitsDC, hOldBitsBitmap);

					if (hBitsBitmap)
					{
						DeleteObject(hBitsBitmap); hBitsBitmap = NULL;
					}
				}
			}

			if (hBitsDC)
			{
				DeleteDC(hBitsDC); hBitsDC = NULL;
			}
		}

		SelectObject(hDrawDC, hOldBitmap);
	}

	if (hDrawDC)
	{
		DeleteDC(hDrawDC); hDrawDC = NULL;
	}

	if (hBitmap)
	{
		DeleteObject(hBitmap); hBitmap = NULL;
	}

	if (hScreenDC)
	{
		::ReleaseDC(0, hScreenDC);
		hScreenDC = NULL;
	}

	return pBits;
}

BOOL CDragDrop::DrawImageBits(HDC hDrawDC, wchar_t* pszFile, int *nMaxX, int nX, int *nMaxY)
{
	if ((*nMaxY + 17) >= MAX_OVERLAY_HEIGHT)
		return FALSE;

	SHFILEINFO sfi = {0};
	DWORD nDrawRC = 0;
	wchar_t* pszText = wcsrchr(pszFile, L'\\');
	if (!pszText) pszText = pszFile; else pszText++;

	SIZE sz = {0};
	GetTextExtentPoint32(hDrawDC, pszText, _tcslen(pszText), &sz);

	if (sz.cx > MAX_OVERLAY_WIDTH)
		sz.cx = MAX_OVERLAY_WIDTH;

	if (sz.cx > *nMaxX)
		*nMaxX = sz.cx;

	// ������, ���� �������
	if (gpSet->isDragShowIcons)
	{
		UINT cbSize = sizeof(sfi);
		DWORD_PTR shRc = SHGetFileInfo(pszFile, 0, &sfi, cbSize,
		                               SHGFI_ATTRIBUTES|SHGFI_ICON/*|SHGFI_SELECTED*/|SHGFI_SMALLICON|SHGFI_TYPENAME);

		if (shRc)
		{
			nDrawRC = DrawIconEx(hDrawDC, nX, *nMaxY, sfi.hIcon, 16, 16, 0, NULL, DI_NORMAL);
		}
		else
		{
			// ���������� ����������� ������?
		}
	}

	// � ������ - ��� �����/�����
	RECT rcText = {nX+OVERLAY_TEXT_SHIFT, *nMaxY+1, 0, (*nMaxY + 16)};
	rcText.right = min(MAX_OVERLAY_WIDTH, (rcText.left + *nMaxX));
	wchar_t szText[MAX_PATH+1]; lstrcpyn(szText, pszText, MAX_PATH); szText[MAX_PATH] = 0;
	nDrawRC = DrawTextEx(hDrawDC, szText, _tcslen(szText), &rcText,
	                     DT_LEFT|DT_TOP|DT_NOPREFIX|DT_END_ELLIPSIS|DT_SINGLELINE|DT_MODIFYSTRING, NULL);

	if (*nMaxY < (rcText.bottom+1))
		*nMaxY = min(rcText.bottom+1,300);

	if (sfi.hIcon)
	{
		DestroyIcon(sfi.hIcon); sfi.hIcon = NULL;
	}

	return TRUE;
}

BOOL CDragDrop::LoadDragImageBits(IDataObject * pDataObject)
{
	if (/*mb_selfdrag ||*/ mh_Overlapped)
		return FALSE; // ���

	DestroyDragImageBits();
	DestroyDragImageWindow();

	if (!gpSet->isDragOverlay)
		return FALSE;

	STGMEDIUM stgMedium = { 0 };
	FORMATETC fmtetc = { 0, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	// ��� ���� ���-�� �� ��������
	//if (gpSet->isDragOverlay == 1) {
	//	wchar_t* pszFiles = NULL;
	//	fmtetc.cfFormat = RegisterClipboardFormat(L"FileNameW");
	//	TODO("� ����������� ���������� ����?");
	//	if (S_OK == pDataObject->QueryGetData(&fmtetc)) {
	//		if (S_OK == pDataObject->GetData(&fmtetc, &stgMedium) || stgMedium.hGlobal == NULL) {
	//			pszFiles = (wchar_t*)GlobalLock(stgMedium.hGlobal);
	//			if (pszFiles) {
	//				return (CreateDragImageBits(pszFiles) != NULL);
	//			}
	//		}
	//	}
	//}
	fmtetc.cfFormat = RegisterClipboardFormat(L"DragImageBits");

	if (S_OK != pDataObject->QueryGetData(&fmtetc))
	{
		return FALSE; // ������ �����������
	}

	// !! The caller then assumes responsibility for releasing the STGMEDIUM structure.
	if (S_OK != pDataObject->GetData(&fmtetc, &stgMedium) || stgMedium.hGlobal == NULL)
	{
		return FALSE; // ������ �����������
	}

	SIZE_T nInfoSize = GlobalSize(stgMedium.hGlobal);

	if (!nInfoSize)
	{
		GlobalFree(stgMedium.hGlobal);
		return FALSE; // �����
	}

	DragImageBits* pInfo = (DragImageBits*)GlobalLock(stgMedium.hGlobal);

	if (!pInfo)
	{
		GlobalFree(stgMedium.hGlobal);
		return FALSE; // �� ������� �������� ������
	}

	_ASSERTE(pInfo->nWidth>0 && pInfo->nWidth<=400);
	_ASSERTE(pInfo->nHeight>0 && pInfo->nHeight<=400);

	if (nInfoSize != (sizeof(DragImageBits)+(pInfo->nWidth * pInfo->nHeight - 1)*4))
	{
		_ASSERT(FALSE); // ����������� ������?
		GlobalFree(stgMedium.hGlobal);
		return FALSE;
	}

	BOOL lbRc = FALSE;
	//int nCount = pInfo->nWidth * pInfo->nHeight;
	mp_Bits = (DragImageBits*)calloc(sizeof(DragImageBits)/*+(nCount-1)*4*/,1);

	if (mp_Bits)
	{
		*mp_Bits = *pInfo;
		//memmove(mp_Bits->pix, pInfo->pix, nCount*4);
		MCHKHEAP
		HDC hdc = ::GetDC(ghWnd);
		mh_BitsDC = CreateCompatibleDC(hdc);

		if (mh_BitsDC)
		{
			SetLayout(mh_BitsDC, LAYOUT_BITMAPORIENTATIONPRESERVED);
			BITMAPINFOHEADER bih = {sizeof(BITMAPINFOHEADER)};
			bih.biWidth = mp_Bits->nWidth;
			bih.biHeight = mp_Bits->nHeight;
			bih.biPlanes = 1;
			bih.biBitCount = 32;
			bih.biCompression = BI_RGB;
			bih.biXPelsPerMeter = 96;
			bih.biYPelsPerMeter = 96;
			LPBYTE pDst = NULL;
			mh_BitsBMP_Old = NULL;
			mh_BitsBMP = CreateDIBSection(hdc, (BITMAPINFO*)&bih, DIB_RGB_COLORS, (void**)&pDst, NULL, 0);

			if (mh_BitsBMP && pDst)
			{
				mh_BitsBMP_Old = (HBITMAP)SelectObject(mh_BitsDC, mh_BitsBMP);
				int cbSize = pInfo->nWidth * pInfo->nHeight * 4;
				memmove(pDst, pInfo->pix, cbSize);
				GdiFlush();
#ifdef _DEBUG
				int nBits = GetDeviceCaps(mh_BitsDC, BITSPIXEL);
				//_ASSERTE(nBits == 32);
#endif
				lbRc = TRUE;
			}
		}

		if (hdc) ::ReleaseDC(ghWnd, hdc);
	}

	// ��������� ������
	GlobalUnlock(stgMedium.hGlobal);
	GlobalFree(stgMedium.hGlobal);

	if (!lbRc)
		DestroyDragImageBits();

	return lbRc;
}

void CDragDrop::DestroyDragImageBits()
{
	if (mh_BitsDC || mh_BitsBMP || mp_Bits)
	{
		DEBUGSTROVL(L"DestroyDragImageBits()\n");

		if (mh_BitsDC)
		{
			if (mh_BitsBMP_Old)
			{
				SelectObject(mh_BitsDC, mh_BitsBMP_Old); mh_BitsBMP_Old = NULL;
			}

			DeleteDC(mh_BitsDC);
			mh_BitsDC = NULL;
		}

		if (mh_BitsBMP)
		{
			DeleteObject(mh_BitsBMP);
			mh_BitsBMP = NULL;
		}

		if (mp_Bits)
		{
			free(mp_Bits);
			mp_Bits = NULL;
		}
	}
}

BOOL CDragDrop::CreateDragImageWindow()
{
	DEBUGSTROVL(L"CreateDragImageWindow()\n");
#define DRAGBITSCLASS L"ConEmuDragBits"
	static BOOL bClassRegistered = FALSE;

	if (!bClassRegistered)
	{
#ifdef _DEBUG
		DWORD dwLastError = 0;
#endif
		HBRUSH hBackBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
		WNDCLASS wc = {0/*CS_OWNDC*/ /*CS_SAVEBITS*/, DragBitsWndProc, 0, 0,
		               g_hInstance, NULL, NULL/*LoadCursor(NULL, IDC_ARROW)*/,
		               hBackBrush,  NULL, DRAGBITSCLASS
		              };

		if (!RegisterClass(&wc))
		{
#ifdef _DEBUG
			dwLastError = GetLastError();
#endif
			// �������� �� �����, ����� ���� �� ���������
			return FALSE;
		}

		bClassRegistered = TRUE; // �������������� ����� ���� ���
	}

	//int nCount = mp_Bits->nWidth * mp_Bits->nHeight;
	// |WS_BORDER|WS_SYSMENU - ������� ���������. ���������?
	//2009-08-20 [+] WS_EX_NOACTIVATE
	mh_Overlapped = CreateWindowEx(
	                    WS_EX_LAYERED|WS_EX_TRANSPARENT|WS_EX_PALETTEWINDOW|WS_EX_TOOLWINDOW|WS_EX_TOPMOST|WS_EX_NOACTIVATE,
	                    DRAGBITSCLASS, L"Drag", WS_POPUP|WS_VISIBLE|WS_CLIPSIBLINGS|WS_BORDER|WS_SYSMENU,
	                    0, 0, mp_Bits->nWidth, mp_Bits->nHeight, ghWnd, NULL, g_hInstance, (LPVOID)this);

	if (!mh_Overlapped)
	{
		DestroyDragImageBits();
		return NULL;
	}

	MoveDragWindow(FALSE);
	return TRUE;
}

void CDragDrop::MoveDragWindow(BOOL abVisible/*=TRUE*/)
{
	if (!mh_Overlapped)
	{
		DEBUGSTRBACK(L"MoveDragWindow skipped, Overlay was not created\n");
		return;
	}

	DEBUGSTRBACK(L"MoveDragWindow()\n");
	BLENDFUNCTION bf;
	bf.BlendOp = AC_SRC_OVER;
	bf.AlphaFormat = AC_SRC_ALPHA;
	bf.BlendFlags = 0;
	bf.SourceConstantAlpha = abVisible ? 255 : 0;
	POINT p = {0};
	GetCursorPos(&p);
	p.x -= mp_Bits->nXCursor; p.y -= mp_Bits->nYCursor;
	POINT p2 = {0, 0};
	SIZE  sz = {mp_Bits->nWidth,mp_Bits->nHeight};
#ifdef _DEBUG
	BOOL bRet =
#endif
	    UpdateLayeredWindow(mh_Overlapped, NULL, &p, &sz, mh_BitsDC, &p2, 0,
	                        &bf, ULW_ALPHA /*ULW_OPAQUE*/);
	//_ASSERTE(bRet);
	//apiSetForegroundWindow(ghWnd); // ����� �������� ���� ����� ������ �� GUI
}

void CDragDrop::DestroyDragImageWindow()
{
	if (mh_Overlapped)
	{
		DEBUGSTROVL(L"DestroyDragImageWindow()\n");
		DestroyWindow(mh_Overlapped);
		mh_Overlapped = NULL;
	}
}

LRESULT CDragDrop::DragBitsWndProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	if (messg == WM_CREATE)
	{
		SetWindowLongPtr(hWnd, GWLP_USERDATA,
		                 (LONG_PTR)((LPCREATESTRUCT)lParam)->lpCreateParams);
	}
	else if (messg == WM_SETFOCUS)
	{
		CDragDrop *pDrag = (CDragDrop*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

		if (pDrag && pDrag->mb_DragWithinNow)
		{
			TODO("���� ��������� ���� ����� - �� ����������� � ������� ��������");
			apiSetForegroundWindow(ghWnd); // ����� �������� ���� ����� ������ �� GUI
		}

		return 0;
	}

	return DefWindowProc(hWnd, messg, wParam, lParam);
}

BOOL CDragDrop::InDragDrop()
{
	if (mp_DataObject || mh_Overlapped)
		return TRUE;

	return FALSE;
}

void CDragDrop::DragFeedBack(DWORD dwEffect)
{
#ifdef _DEBUG
	wchar_t szDbg[128]; wsprintf(szDbg, L"DragFeedBack(%i)\n", (int)dwEffect);
	DEBUGSTRBACK(szDbg);
#endif

	// Drop ��� ������ �����, ����� �������� - ConEmu
	if (dwEffect == (DWORD)-1)
	{
		mb_DragWithinNow = FALSE;
		DestroyDragImageWindow();
		gpConEmu->SetDragCursor(NULL);
	}
	else if (dwEffect == DROPEFFECT_NONE)
	{
		MoveDragWindow(FALSE);
	}
	else
	{
		if (!mh_Overlapped && mp_Bits)
			CreateDragImageWindow();

		if (mh_Overlapped)
			MoveDragWindow();
	}

	//2009-08-20
	//MSG Msg = {NULL};
	//while (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
	//{
	//	TranslateMessage(&Msg);
	//	DispatchMessage(&Msg);
	//}
}
