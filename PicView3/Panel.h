
#pragma once

/**************************************************************************
Copyright (c) 2010 Maximus5
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
**************************************************************************/


// ����������� � OpenFrom (CPicViewPanel::Initialize) ��� ������� �� ������� VE_READ ��� EE_READ.
// OpenFrom ����� ���� ����� OPEN_VIEWER ��� OPEN_EDITOR, ���� ������ ������ ���� ����� F11 �� �������/���������
	#define OPEN_BY_USER 0x80000000

enum PLUGINPANELITEMFLAGS_PV{
  PPIF__WASNOTCHECKED          = 0x0100,
  PPIF__APPROVED               = 0x0200,
  PPIF__INVALID                = 0x0400,
  PPIF__ALLMASK                = 0xFF00,
};

//{
//  PicPanelItem* pPanelItem;
//  Image* pImage; // ����� ���� NULL'��
//  CPVDModuleBase *pDecoder; // ������ ����������� �������, ������� ������ ������� ����
//                            // WARNING! ��� �������� �������� ����� �������� ��� �������� � ����������
//                            // � NULL ���� �� ���� ��� ��������
//  int nRefCount; // �������� ������� ����� ������ ���� �� ��� ����������� (�������+�����)
//  LPVOID pDecoderContext;
//  LPVOID pDisplayContext;
//  DWORD nLastUsed; // ����� ���������������� ������, �� �������� ����������� �������� "������" �������� ��� ������������ ������
//}

class CImage;

struct PicViewItem
{
	BOOL bPicDetected; // ���� ������� ��� ������ ���� �� ���-��, ��� ��� "����� �����"
	BOOL bPicDisabled; // ��� ���� ����� "����� �����" �������� � TRUE, ���� ���������� �� ��������������
	BOOL bTempCopy;    // ���� ��� ���������� �� ��������� ����� (�� ������, ��������, ��� � ��������� ����)
	int  nRawIndex;    // ��� ����������. ������������ ������� ����� �������� � mpvi_Items
	int  nDisplayItemIndex; // ������ ��� ����������� � ��������� � � OSD, 1-based!
	wchar_t sFileExt[10];       // ���������� ����� - ����� ��� �������������� �������� � �������� ���������� �����
	CUnicodeFileName* sFile;
	CUnicodeFileName* sTempFile; // ������������� � Temp ���� �� ������ ��� ���������� ��������� (����)
	PluginPanelItem* pItem;
	
	// ������ ����������� �������, ������� ������ ������� ����
	// WARNING! ��� �������� �������� ����� �������� ��� ��������
	// � ���������� � NULL ���� �� ���� ��� ��������
	//CPVDModuleBase *pDecoder;   
	

	// ���������� ��������, ������� ���������� ������ ����
	// ��������� ���� (�� ����) ������ �������, ���� (nFileRefCount>0)
	int nFileRefCount;
	
	// ������, ���������� ����������� ��������,
	// �������, � ���������� �� �����������
	CImage *pImage;

	// ����� ���������������� ������, �� �������� �����������
	// �������� "������" �������� ��� ������������ ������
	DWORD nLastUsed;
	
	//TODO. ������ ����� � ����/���������(removable) ������
	// ����� � ���, ����� ������� ������� ������ �����, �� ����� ��� �� ��������� ������?
	// ������ �� �� 32Mb? �� ����� ����� ���� �������� ������, ���� ��� ��������� (� ���������� ftp?).
	DWORD  nBufferSize; // ������ ���������� ������, �� ����, ����� ������� �����
	DWORD  nBufferRead; // ������ ��������� ������
	LPBYTE ptrBuffer;   // ���������� ������ ��� ������ �����

};

class CPicViewPanel
{
protected:
	// Work mode
	bool mb_PanelExists;
	bool mb_PluginMode;
	bool mb_RealFilesMode;
	// Items
	int mn_ItemsCount, mn_ReadyItemsCount;
	// �������� �����������. == m_Panel.CurrentItem
	int mn_ActiveRawIndex;
	// ������������ � ������ ������ �����������
	int mn_DecodeRawIndex;
	// ��� �������� ����������� - ��� ������������ (����� ����������) ����� ������ � ����� �� ���������
	// [0] - ���������� �����������, [1] - ���������. ���� "-1" - ���� � ��� �������� �� ��������� �� �������������
	int mn_LastOpenedRawIndex[2];

	//struct PluginPanelItem** mp_Items;
	//BOOL *mpb_PicDetected; // ��� ��������� ����� �������, ��� ����� ������ �������, ��� ��� ��� ��������������� �����
	//CUnicodeFileName** ms_Files;
	PicViewItem* mpvi_Items;
	// Panel (or directory)
	struct PanelInfo m_Panel;     // FCTL_GETPANELINFO
	CUnicodeFileName ms_PanelDir;  // FCTL_GETPANELDIR
	CUnicodeFileName ms_TempDir;
	//CUnicodeFileName ms_PanelFile; // FCTL_GETPANELITEM(pi.CurrentItem)
public:
	static const struct PluginPanelItem* Invalid;
public:
	CPicViewPanel();
	~CPicViewPanel();
	void FreeAllItems();
	
public:
	enum MarkAction
	{
		ema_Mark = 1,
		ema_Unmark = 2,
		ema_Switch = 3,
	};
	
public:
	bool Initialize(int OpenFrom, LPCWSTR asFile);
	bool CatchNewFile(int OpenFrom, LPCWSTR asFile);
	bool IsInitialized();
	//bool RetrieveAllDirFiles(); // ����� ������ ���
	bool MarkUnmarkFile(enum MarkAction action);
	bool IsMarkAllowed();
	bool IsFileMarked();
	bool IsRealNames();
	//bool UpdatePanelDir();
	
	// ������� ��� ����������� �������� � ���������
	void CloseImageHandles();

	// ������� ������ ����������� ��������
	void CloseDisplayHandles();

	// Assertion purposes
	void CheckAllClosed();

	// ���������� ��� �������� ������� - ������ ��������� �����
	void OnTerminate();
	
	// ���������� � ������� ����, ����� ����� ������� �� �������
	void RedrawOnExit();

	int  GetActiveRawIdx(); // ������ � mp_Items, 0-based
	int  GetActiveDisplayIdx(); // ������ ��� ����������� � ��������� � � OSD, 1-based!
	void SetActiveRawIndex(int aiRawIndex);
	void SetDecodeRawIndex(int aiRawIndex);
	LPCWSTR GetItemFile(int aiRawIndex = -1); // ������� ������ ���� � ��������� (��������, �������������� � TEMP) �����
	const PicViewItem* GetItem(int aiRawIndex = -1);
	const PluginPanelItem* GetPanelItem(int aiRawIndex = -1);
	CImage* GetImage(int aiRawIndex = -1);
	//LPVOID GetImageDraw(int aiRawIndex = -1);

	//�������������� �������	
	//const wchar_t* GetCurrentItemName(); // ppi->FindData.lpwszFileName. !!!_���_����_!!!
	const wchar_t* GetCurrentDir();

	bool IsItemSupported(int aiRawIndex); // ������������� �� �������� � ����� �����������?
	void RescanSupported();
	void SetItemDisabled(int aiRawIndex); // ��� ����� �������� �� ������� ��������� �������
	
	int GetFirstItemRawIndex();
	int GetLastItemRawIndex();
	int GetNextItemRawIdx(int anStep = 1, int aiFromRawIndex = -1);
	int GetReadyItemsCount();
	
	void ShowError(const wchar_t* pszMsg, const wchar_t* pszMsg2=NULL, bool bSysError=false);

	void FreeUnusedDisplayData(int anForceLevel = 0);
	
	LPCWSTR ItemPointToName(LPCWSTR apszFull);
	
	bool StartItemExtraction();
	bool OnFileExtractedToViewer();
	
protected:
	bool InitListFromPanel();
	bool InitListFromFileSystem();
	void SortItems(); // ����� ������ ���
	int  CompareItems(PluginPanelItem* pItem1, PluginPanelItem* pItem2);

	wchar_t* ms_ExtractMacroText;
	PluginPanelItem* LoadPanelItem(int aiPanelItemIndex);
	bool CacheFile(int aiRawIndex, LPCWSTR asSrcFile);
	void EraseTempDir();
	bool InitExtractMacro();
	
	//void WaitLoadingFinished();
	bool AddItem(LPCWSTR asFilePathName, DWORD dwFileAttributes, ULONGLONG nFileSize, FILETIME ftLastWriteTime);
	
	int mn_LoadItemFromPlugin;
	HANDLE mh_LoadItemFromPlugin;
	bool LoadItemFromPlugin(int aiRawIndex);
	CImage* CreateImageData(int aiRawIndex);
	void FreeUnusedImage(int aiRawIndex);
};

extern CPicViewPanel g_Panel;
