
#pragma once

//#include <Objbase.h>
//#include <Objidl.h>
#include "Modules/ThumbSDK.h"
#include "QueueProcessor.h"

#define ImgCacheFileName L"ConEmuTh.cache"
#define ImgCacheListName L"#LFN"

extern HICON ghUpIcon;

struct CePluginPanelItem;

//typedef BOOL (WINAPI* AlphaBlend_t)(HDC hdcDest, int xoriginDest, int yoriginDest, int wDest, int hDest, HDC hdcSrc, int xoriginSrc, int yoriginSrc, int wSrc, int hSrc, BLENDFUNCTION ftn);

struct IMAGE_CACHE_INFO
{
	DWORD nAccessTick;
	union
	{
		struct
		{
			DWORD nFileSizeHigh;
			DWORD nFileSizeLow;
		};
		unsigned __int64 nFileSize;
	};
	FILETIME ftLastWriteTime;
	DWORD dwFileAttributes;
	wchar_t *lpwszFileName;
	BOOL bVirtualItem;
	DWORD_PTR UserData;
	UINT PreviewLoaded;  // �������� �� ��� ��������� ��������� (|1-ShellIcon, |2-Thumbnail, |4-Thumbnail ��� ������� ��������, � �� ������ ��������� ����������)
	//BOOL bPreviewExists; // � ���������� �� �� ��������� �������, ��� � ���� ������ ShellIcon?
	BOOL bIgnoreFileDescription; // ImpEx ���������� � �������� ������ �����������, ���������� ���������� ������������
	//int N,X,Y;
	struct ImageBits
	{
		COORD crSize; // ���������������, ������ ��������� � crLoadSize
		DWORD cbStride; // Bytes per line
		DWORD nBits; // 32 bit required!
		// [Out] Next fields MUST be LocalAlloc(LPTR)
		DWORD ColorModel; // One of CET_CM_xxx
		LPDWORD Pixels; // Alpha channel (highest byte) allowed.
		DWORD cbPixelsSize; // size in BYTES
	} Icon, Preview;
	wchar_t *pszComments; // This may be "512 x 232 x 32bpp"
	DWORD wcCommentsSize; // size in WORDS
	//// Module can place here information about original image (dimension, format, etc.)
	//// This must be double zero terminated string
	//wchar_t *pszInfo;
	//DWORD wcInfoSize; // size in WORDS
};

class CImgLoader;

class CImgCache
{
	protected:
		wchar_t ms_CachePath[MAX_PATH];
		wchar_t ms_LastStoragePath[32768];
		int nPreviewSize; // 96x96
		//int nXIcon, nYIcon, nXIconSpace, nYIconSpace;
		COLORREF crBackground;
		HBRUSH hbrBack;
		// ������ - ���������� ���� ����
#define FIELD_MAX_COUNT 1000
		//#define ITEMS_IN_FIELD 10 // ���������� � "������"
		//int nFieldX, nFieldY; // �������� ���������� � "������"/"�������" (�� ������ ITEMS_IN_FIELD)
		//HDC hField[FIELD_MAX_COUNT]; HBITMAP hFieldBmp[FIELD_MAX_COUNT], hOldBmp[FIELD_MAX_COUNT];
		IMAGE_CACHE_INFO CacheInfo[FIELD_MAX_COUNT];
		HDC mh_LoadDC, mh_DrawDC;
		HBITMAP mh_OldLoadBmp, mh_OldDrawBmp, mh_LoadDib, mh_DrawDib;
		COORD mcr_LoadDibSize, mcr_DrawDibSize;
		LPBYTE  mp_LoadDibBytes; DWORD mn_LoadDibBytes;
		LPBYTE  mp_DrawDibBytes; DWORD mn_DrawDibBytes;
		BOOL CheckLoadDibCreated();
		BOOL CheckDrawDibCreated();
		//void UpdateCell(struct IMAGE_CACHE_INFO* pInfo, BOOL abLoadPreview);
		BOOL FindInCache(CePluginPanelItem* pItem, int* pnIndex, BOOL abLoadPreview);
		void CopyBits(COORD crSrcSize, LPBYTE lpSrc, DWORD nSrcStride, COORD crDstSize, LPBYTE lpDst);

		CImgLoader *mp_ShellLoader;

#define MAX_MODULES 20
		struct tag_Module
		{
			HMODULE hModule;
			CET_Init_t Init;
			CET_Done_t Done;
			CET_Load_t LoadInfo;
			CET_Free_t FreeInfo;
			CET_Cancel_t Cancel;
			LPVOID pContext;
		} Modules[MAX_MODULES];
		int mn_ModuleCount;
		void LoadModules();
		void FreeModules();
		wchar_t ms_ModulePath[MAX_PATH], *mpsz_ModuleSlash;
		//// Alpha blending
		//HMODULE mh_MsImg32;
		//AlphaBlend_t fAlphaBlend;

	public:
		CImgCache(HMODULE hSelf);
		~CImgCache(void);
		void SetCacheLocation(LPCWSTR asCachePath);
		void Reset();
		void Init(COLORREF acrBack);
		BOOL RequestItem(CePluginPanelItem* pItem, BOOL abLoadPreview);
		BOOL PaintItem(HDC hdc, int x, int y, int nImgSize, CePluginPanelItem* pItem, /*BOOL abLoadPreview,*/ LPCWSTR* ppszComments, BOOL* pbIgnoreFileDescription);

	public:
		BOOL LoadThumbnail(struct IMAGE_CACHE_INFO* pItem);
		BOOL LoadShellIcon(struct IMAGE_CACHE_INFO* pItem);

	protected:
		BOOL mb_Quit;
		static DWORD WINAPI ShellExtractionThread(LPVOID apArg);
		IStorage *mp_RootStorage, *mp_CurrentStorage;
		BOOL LoadPreview();
};

extern CImgCache *gpImgCache;

class CImgLoader : public CQueueProcessor<IMAGE_CACHE_INFO*>
{
	public:
		// ��������� ��������. ������� ������ ����������:
		// S_OK    - ������� ������� ���������, ����� ���������� ������ eItemReady
		// S_FALSE - ������ ���������, ����� ���������� ������ eItemFailed
		// FAILED()- ������ eItemFailed � ���� ����������� ����� ���������
		virtual HRESULT ProcessItem(IMAGE_CACHE_INFO*& pItem, LONG_PTR lParam)
		{
			if (!gpImgCache)
			{
				return S_FALSE;
			}

			if (lParam == 1)
			{
				return gpImgCache->LoadShellIcon(pItem) ? S_OK : S_FALSE;
			}

			if (lParam == 2)
			{
				return gpImgCache->LoadThumbnail(pItem) ? S_OK : S_FALSE;
			}

			return S_FALSE;
		};

		//// ���������� ��� �������� ���������� ��������� �������� ��� ����������� ���������.
		//// ���� ������� ��������� ������� (Status == eItemReady), ���������� OnItemReady
		//virtual void OnItemReady(IMAGE_CACHE_INFO*& pItem, LONG_PTR lParam)
		//{
		//	return;
		//};
		//// ����� (Status == eItemFailed) - OnItemFailed
		//virtual void OnItemFailed(IMAGE_CACHE_INFO*& pItem, LONG_PTR lParam)
		//{
		//	return;
		//};
		//// ����� ���������� ���� ������� ������ ���������!

		// ���� ��������� ������� ���� �������� � ����� �� ���� ����������
		virtual bool IsTerminationRequested()
		{
			TODO("������� TRUE ��� ExitFar");
			return CQueueProcessor<IMAGE_CACHE_INFO*>::IsTerminationRequested();
		};
		// ����� ������� ����� ��������� CoInitialize ��������
		virtual HRESULT OnThreadStarted()
		{
			CoInitialize(NULL);
			return S_OK;
		}
		// ����� ������� ����� ��������� CoUninitialize ��������
		virtual void OnThreadStopped()
		{
			CoUninitialize();
			return;
		};
		// ����� �������������� ��� ��������� ������ ��������� (������������ ��� ������)
		virtual bool IsEqual(const IMAGE_CACHE_INFO*& pItem1, LONG_PTR lParam1, IMAGE_CACHE_INFO*& pItem2, LONG_PTR lParam2)
		{
			return (pItem1 == pItem2) && (lParam1 == lParam2);
		};
		// ���� ������� ������� ������������ - ���� �� ������������������
		virtual bool CheckHighPriority(const IMAGE_CACHE_INFO*& pItem)
		{
			// ��������� � ������� � ������� false, ����, ��������, ��� ������
			// ��� ������� ��������, �� ������������ ��� ������� � ��� �� ������
			return true;
		};
};
