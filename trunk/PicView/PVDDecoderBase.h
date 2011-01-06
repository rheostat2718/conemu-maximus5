
#pragma once

/**************************************************************************
Copyright (c) 2009 Maximus5
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

class ModuleData;

class PVDDecoderBase {
public:
	HMODULE hPlugin;
	ModuleData* pData;
	const wchar_t *pModulePath;  // ������ ���� � ���������� (pvd ����)
	const wchar_t *pRegPath;     // ������ ���� � ����� ���������� � ������� (���������, ����������, ���������, � �.�.)
	wchar_t *pRegSettings;       // ������ ���� � ����� ���������� � ��� ������� ����������� (������� pRegPath)
	DWORD nDefPriority;
	wchar_t *pDefActive;   // ������ �������������� ���������� ����� �������.
                           // ��� ����������, ������� ������ ����� "������" ���������.
	                       // ����� ����������� �������� "*" ����������, ���
	                       // ��������� �������� �������������.
                           // ���� ��� ������������� �� ���� �� ����������� �� ������� �� ���������� -
                           // PicView ��� ����� ���������� ������� ���� �����������, ���� ����������
                           // �� ������� � ������ ��� ������������.
    wchar_t *pDefInactive; // ������ ���������� ���������� ����� �������.
                           // ����� ����������� ����������, ������� ������ ����� �������
                           // "� ��������", �� ��������, � ����������.
    wchar_t *pDefForbidden;// ������ ������������ ���������� ����� �������.
                           // ��� ������ � ���������� ������������ ��������� ��
                           // ����� ���������� ������. ������� "." ��� �������������
	                       // ������ ��� ����������.
	UINT   nVersion;
	UINT32 nFlags;         // ��������� ����� PVD_IP_xxx (��� �������� �� pvdPluginInfo)
	UINT32 nCurrentFlags;  // erased unsupported flags from nFlags
	wchar_t *pName;        // ��� ����������
	wchar_t *pVersion;     // ������ ����������
	wchar_t *pComments;    // ����������� � ����������: ��� ���� ������������, ��� ����� ����������, ...
	bool bPrepared;        // ��������� ������� ��������� (�������� - ������ �� �������)
	bool bInitialized;     // pvd ���������� ��������� � ������� ����������������
	bool bDisplayInitialized;
	wchar_t szLastError[512];
	//
	bool bCancelled;
public:
	PVDDecoderBase(ModuleData* apData);
	virtual ~PVDDecoderBase();
	bool InitPrepare(FILETIME ftRegModified, BOOL abPluginChanged);
	bool LoadFormatsFromReg(FILETIME ftRegModified, BOOL abForceWriteReg);
	virtual bool LoadFormatsFromPvd() = 0;
	void SaveFormatsToReg(FILETIME *pftRegModified=NULL);
	virtual bool InitPlugin(bool bAllowResort=false) = 0; // ���������, ����������������, �������� ������ ��������
	wchar_t* LoadRegValue(HKEY hkey, LPCWSTR asName) const;
	bool IsAllowed(const wchar_t* asExt, bool abAllowAsterisk, bool abAllowInactive, bool abAssumeActive=false);
	void SetStatus(LPCWSTR asStatus, BOOL abSaveInReg=FALSE);
	void SetException(LPCWSTR asFunction);
public:
	virtual BOOL FileOpen2(const wchar_t *pFileName, INT64 lFileSize, const BYTE *pBuf, UINT32 lBuf, pvdInfoImage2 *pImageInfo) = 0;
	virtual BOOL PageInfo2(void *pImageContext, pvdInfoPage2 *pPageInfo) = 0;
	virtual BOOL PageDecode2(void *pImageContext, pvdInfoDecode2 *pDecodeInfo, pvdDecodeCallback2 DecodeCallback, void *pDecodeCallbackContext) = 0;
	virtual void PageFree2(void *pImageContext, pvdInfoDecode2 *pDecodeInfo) = 0;
	virtual void FileClose2(void *pImageContext) = 0;
	virtual BOOL TranslateError2(DWORD nErrNumber, wchar_t *pszErrInfo, int nBufLen);
	virtual void ReloadConfig2() {};
	virtual void Cancel2() {};
	BOOL DisplayCheck();
	BOOL DisplayAttach();
	BOOL DisplayDetach();
	virtual BOOL DisplayInit2(pvdInfoDisplayInit2* pDisplayInit) { return FALSE; };
protected:
	virtual BOOL DisplayAttach2(HWND hWnd, BOOL bAttach) { return FALSE; };
	BOOL LoadExtensionsFromReg(HKEY hkey, LPCWSTR asName, wchar_t** rpszExt);
public:
	virtual BOOL DisplayCreate2(pvdInfoDisplayCreate2* pDisplayCreate) { return FALSE; };
	virtual BOOL DisplayPaint2(void* pDisplayContext, pvdInfoDisplayPaint2* pDisplayPaint) { return FALSE; };
	virtual void DisplayClose2(void* pDisplayContext) {};
	virtual void DisplayExit2() {};
protected:
	virtual bool PluginInfo2() = 0;
	virtual BOOL Init2() = 0;
	virtual void Exit2() {};
	void* mp_Context;
	BOOL  mb_DisplayAttached;
};

class PVDDecoderConfig;

class PVDDecoderConfigDlg : public MPicViewDlg
{
protected:
	PVDDecoderConfig* mp_Config;
	int iActiveExt, iInactiveExt, iForbiddenExt, iResetExt, iOk, iCancel;
	int /*iPName, iPVer, iPComm,*/ iPLog, iReload, iUnload, iAbout /*, iPFlags*/;
	FarList ModuleFlags; FarListItem ModuleFlagItems[30]; wchar_t szFlagsLeft[20];
public:
	PVDDecoderConfigDlg(PVDDecoderConfig* pConfig);
	~PVDDecoderConfigDlg();
	virtual LONG_PTR ConfigDlgProc(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2);
	bool Run();
protected:
	virtual void Reset();
	typedef struct tag_PvdItem {
		wchar_t* pszType;
		wchar_t szName[MAX_PATH];
		const wchar_t* pszLabel;
		wchar_t* pszData;
		DWORD nData;
		bool bData;
		DWORD nLen, nType;
		int nID, nCount; // nCount ������������ ��� �����-������
	} PvdItem;
	PvdItem m_PvdItems[MAX_CONFIG_ITEMS-20];
	int mn_PvdItems;
	int LoadItems();
	void FreeItems();
	wchar_t ms_Title[128];
	wchar_t* Title(const wchar_t* asModule);
	BOOL LoadExtensionsFromDlg(int nID, wchar_t** rpszExt);
};

class PVDDecoderAboutDlg : public MPicViewDlg
{
protected:
	PVDDecoderConfig* mp_Config;
	int iOk, iCancel;
	int iActiveExt, iInactiveExt, iForbiddenExt, iPriority, iReloadExt;
	wchar_t szPriority[32];
	int iPName, iPVer, iPComm, iPLog, iPFlags;
	FarList ModuleFlags; FarListItem ModuleFlagItems[30]; wchar_t szFlagsLeft[20];
public:
	PVDDecoderAboutDlg(PVDDecoderConfig* pConfig);
	~PVDDecoderAboutDlg();
	virtual LONG_PTR ConfigDlgProc(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2);
	bool Run();
protected:
	virtual void Reset();
	wchar_t ms_Title[128];
	wchar_t* Title(const wchar_t* asModule);
};

class ModuleData : public RefRelease
{
public:
	BOOL bPrepared; // ������������ � TRUE ����� �������� ������ � ��.
	FILETIME ftModified; // ���� ���������� ��������� �����
	DWORD nVersion;
	DWORD nFlags; // �����, ������� ����� ������ (��������� PicView ����� ��������)
	DWORD nCurrentFlags; // ������� ����� ������� (�������� PicView ���������� ���� DISPLAY ���� DX ������� � ���������)
	DWORD nActiveFlags; // ���� ������ PVD_IP_DISPLAY ��� ��������� �� ������������� ������ ������
	wchar_t *pModulePath;  // ������ ���� � ���������� (pvd ����)
	const wchar_t *pModuleFileName;
	wchar_t *pRegPath;     // ������ ���� � ����� ���������� � ������� (���������, ����������, ���������, � �.�.)
	HMODULE hPlugin;       // May be NULL
	DWORD nPriority;
	wchar_t *pActive;      // ������ �������������� ���������� ����� �������.
                           // ��� ����������, ������� ������ ����� "������" ���������.
	                       // ����� ����������� �������� "*" ����������, ���
	                       // ��������� �������� �������������.
                           // ���� ��� ������������� �� ���� �� ����������� �� ������� �� ���������� -
                           // PicView ��� ����� ���������� ������� ���� �����������, ���� ����������
                           // �� ������� � ������ ��� ������������.
    wchar_t *pInactive;    // ������ ���������� ���������� ����� �������.
                           // ����� ����������� ����������, ������� ������ ����� �������
                           // "� ��������", �� ��������, � ����������.
    wchar_t *pForbidden;   // ������ ������������ ���������� ����� �������.
                           // ��� ������ � ���������� ������������ ��������� ��
                           // ����� ���������� ������. ������� "." ��� �������������
	                       // ������ ��� ����������.
	PVDDecoderBase *pPlugin;
	// ���������� ������������ ��� ������� ��������
	//bool bProcessed;
	wchar_t szStatus[0x1000];

	ModuleData();
protected:
	virtual ~ModuleData();
public:

	//
	void SetStatus(LPCWSTR asStatus, BOOL abSaveInReg=FALSE);
	DWORD Priority();
	void SetPriority(DWORD n);
	//
	void Unload();
	bool Load(bool abPluginChanged); // ���������� LastWriteDate
};

// ������������ � ������� ��������� ����������� (����������, ���������, ���������)
class PVDDecoderConfig
{
public:
	//PVDDecoderBase* pPlugin;
	ModuleData* pData;
	wchar_t* pTitle;
	DWORD nPriority;
	wchar_t* pActive;
	wchar_t* pInactive;
	wchar_t* pForbidden;
	
	PVDDecoderConfig(/*PVDDecoderBase* p,*/ ModuleData* apData);
	~PVDDecoderConfig();

	//void Init(PVDDecoderBase* p, ModuleData* apData);
	void CreateTitle();
	void Free();
	bool Configure();
	void About();
};
