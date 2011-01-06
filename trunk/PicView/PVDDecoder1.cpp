
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

#include "PVDManager.h"
#include "PVDDecoderBase.h"
#include "PVDDecoder1.h"
#include "PictureView_Lang.h"
//#include <shlwapi.h>




/* ********************* */
/*  Interface Version 1  */
/* ********************* */
PVDDecoderVer1::PVDDecoderVer1(ModuleData* apData)
	: PVDDecoderBase(apData)
{
	fInit = NULL;
	fExit = NULL;
	fPluginInfo = NULL;
	fFileOpen = NULL;
	fPageInfo = NULL;
	fPageDecode = NULL;
	fPageFree = NULL;
	fFileClose = NULL;
	nVersion = 1;
	nFlags = nCurrentFlags = PVD_IP_DECODE;
}

PVDDecoderVer1::~PVDDecoderVer1()
{
	MCHKHEAP;

	if (bInitialized) {
		Exit2();
		_ASSERTE(!bInitialized);
	}

	MCHKHEAP;
}

bool PVDDecoderVer1::LoadFormatsFromPvd()
{
	if (!bInitialized) if (!InitPlugin()) return false;

	// � ������ ������ ���������� ���������� ������ ����������
	if (lstrcmpi(pName, L"DJVU") == 0 || lstrcmpi(pName, L"DJVU.pvd") == 0) {
		pDefActive = _wcsdup(L"DJV,DJVU");
	} else if (lstrcmpi(pName, L"PVD_APL") == 0 || lstrcmpi(pName, L"PVD_APL.PVD") == 0) {
		pDefActive = _wcsdup(L"032,ABR,ANI,APD,ARW,BAY,BMP,BW,CNV,CR2,CRW,CS1,CUR,CVX,DCR,DCX,DIB,DNG,EMF,EPS,ERF,FFF,GIF,HDR,ICL,ICN,ICO,IFF,ILBM,INT,INTA,IW4,J2C,J2K,JBR,JFIF,JIF,JP2,JPC,JPE,JPEG,JPG,JPK,JPX,KDC,LBM,MEF,MOS,MRW,NEF,NRW,ORF,PBM,PBR,PCD,PCT,PCX,PEF,PGM,PIC,PICT,PIX,PNG,PPM,PSD,PSP,PSPBRUSH,PSPIMAGE,RAF,RAS,RAW,RGB,RGBA,RLE,RSB,RW2,RWL,SGI,SR2,SRF,TGA,THM,TIF,TIFF,TTC,WBM,WBMP,WMF,XBM,XIF,XPM");
	} else {
		pDefActive = _wcsdup(L"");
	}
	pDefInactive = _wcsdup(L"");
	pDefForbidden = _wcsdup(L"");

	return true;
}

// ���������, ����������������, �������� ������ ��������
bool PVDDecoderVer1::InitPlugin(bool bAllowResort/*=false*/)
{
	if (bInitialized) return true;

	bool result = false;

	if (!hPlugin) {
		TRY {
			hPlugin = LoadLibrary(pModulePath);
			if (!hPlugin)
				SetStatus(GetMsg(MICantLoadLibrary),TRUE);
		} CATCH {
			SetException(L"LoadLibrary");
			hPlugin = NULL;
		}
		if (!hPlugin) {
			return false;
		}
	}

	fInit = (pvdInit_t)GetProcAddress(hPlugin, "pvdInit");
	fExit = (pvdExit_t)GetProcAddress(hPlugin, "pvdExit");
	fPluginInfo = (pvdPluginInfo_t)GetProcAddress(hPlugin, "pvdPluginInfo");
	fFileOpen = (pvdFileOpen_t)GetProcAddress(hPlugin, "pvdFileOpen");
	fPageInfo = (pvdPageInfo_t)GetProcAddress(hPlugin, "pvdPageInfo");
	fPageDecode = (pvdPageDecode_t)GetProcAddress(hPlugin, "pvdPageDecode");
	fPageFree = (pvdPageFree_t)GetProcAddress(hPlugin, "pvdPageFree");
	fFileClose = (pvdFileClose_t)GetProcAddress(hPlugin, "pvdFileClose");

	if (fInit && fExit && fPluginInfo && fFileOpen && fPageInfo && fPageDecode && fPageFree && fFileClose)
	{
		bool bNeedExit = true;
		TRY {
			if (!Init2()) {
				SetStatus(L"pvdInit failed",TRUE);
				bNeedExit = false;
				Exit2();
			}
			else
			{
				result = PluginInfo2();
				bInitialized = true;
			}
		} CATCH {
			if (bNeedExit) // 09.09.2009 Maks [+]
			TRY {
				Exit2();
			} CATCH {
			}
		}
	} else {
		wchar_t szMsg[128];
		wsprintf(szMsg, L"Required plugin function (%s) not found",
			!fInit ? L"pvdInit" :
			!fExit ? L"pvdExit" :
			!fPluginInfo ? L"pvdPluginInfo" :
			!fFileOpen ? L"pvdFileOpen" :
			!fPageInfo ? L"pvdPageInfo" :
			!fPageDecode ? L"pvdPageDecode" :
			!fPageFree ? L"pvdPageFree" :
			!fFileClose ? L"pvdFileClose" :	L"<Unlisted>" );
		SetStatus(szMsg,TRUE);
	}
	if (!result) {
		FreeLibrary(hPlugin);
		hPlugin = NULL;
	}
	
	if (result)
		pData->nCurrentFlags = nCurrentFlags = PVD_IP_DECODE;

	return result;
}

bool PVDDecoderVer1::PluginInfo2()
{
	_ASSERTE(hPlugin && fPluginInfo);
	pvdInfoPlugin pip = {0};
	bool lbRc = true;
	
	TRY{
		fPluginInfo(&pip);
		
	} CATCH {
		lbRc = false;
		SetException(L"pvdPluginInfo");
		pName = _wcsdup(L"Invalid");
		pVersion = _wcsdup(L"Invalid");
		pComments = _wcsdup(L"Invalid");
	}
	
	if (lbRc) {
		nFlags = nCurrentFlags = PVD_IP_DECODE; // � ������ ������ - ������ �������
		
		if (!nDefPriority) {
			HKEY hkey = 0; DWORD dwDisp;
			if (!RegCreateKeyEx(HKEY_CURRENT_USER, pRegPath, 0, 0, 0, KEY_ALL_ACCESS, 0, &hkey, &dwDisp)) {
				RegSetValueEx(hkey, L"DefaultPriority", 0, REG_DWORD, (LPBYTE)&pip.Priority, sizeof(pip.Priority));
				if (!pData->nPriority) {
					DWORD nSize = 4;
					if (RegQueryValueEx(hkey, L"Priority", 0, 0, (LPBYTE)&pData->nPriority, &nSize)
						|| !pData->nPriority)
					{
						// ���� ��������� ������ �� ���� �� ��������� - ��� ��� 0. ������� "�������������"
						pData->nPriority = pip.Priority;
						RegSetValueEx(hkey, L"Priority", 0, REG_DWORD, (LPBYTE)&pip.Priority, sizeof(pip.Priority));
					}
				}
				RegCloseKey(hkey); hkey = NULL;
			}
		}
		nDefPriority = max(1,pip.Priority); // 0 �� ������ ������ ���������� ����������

		SAFEFREE(pName);
		SAFEFREE(pVersion);
		SAFEFREE(pComments);
		
		TRY {
			pName = ConvertToWide(pip.pName);
			pVersion = ConvertToWide(pip.pVersion);
			pComments = ConvertToWide(pip.pComments);
		} CATCH {
			lbRc = false;
			SetException(L"ConvertToWide(name,version,comments)");
		}
		
		#ifdef _DEBUG
		wchar_t szDbg[128]; wsprintf(szDbg, L"PluginInfo> Priority:0x%04X, Flags:0x%04X", pip.Priority, nFlags);
		PVDManager::AddLogString(pName, szDbg);
		#endif
	}
	
	return lbRc;
}

BOOL PVDDecoderVer1::Init2()
{
	if (hPlugin && fInit) {
		UINT nRc = 0;
		TRY {
			nRc = fInit();
			if (nRc != PVD_CURRENT_INTERFACE_VERSION) {
				SetStatus(L"pvdInit returns an error",TRUE);
			}
		} CATCH {
			SetException(L"pvdInit"); nRc = 0;
		}
		return (nRc == PVD_CURRENT_INTERFACE_VERSION);
	} else SetStatus(L"PVDDecoderVer1::Init2 skipped, plugins was not loaded",TRUE);
	return FALSE;
}

void PVDDecoderVer1::Exit2()
{
	if (hPlugin && fExit) {
		TRY {
			fExit();
			bInitialized = false;
		} CATCH {
			SetException(L"pvdExit");
		}
	}
}

wchar_t* PVDDecoderVer1::ConvertToWide(const char* psz)
{
	if (!psz || !*psz)
		return _wcsdup(L"");
	UINT nLen = lstrlenA(psz);
	wchar_t *pwsz = (wchar_t*)calloc(nLen+1,2);
	MultiByteToWideChar(CP_UTF8, 0, psz, -1, pwsz, nLen+1);
	return pwsz;
}

BOOL PVDDecoderVer1::FileOpen2(const wchar_t *pFileName, INT64 lFileSize, const BYTE *pBuf, UINT32 lBuf, pvdInfoImage2 *pImageInfo)
{
	if (!lFileSize && !lBuf) return FALSE;
	if (!bInitialized) if (!InitPlugin()) return FALSE;
	_ASSERTE(pImageInfo->cbSize == sizeof(pvdInfoImage2));
	
	pvdInfoImage pii = {0};
	void* pImgContext = NULL;
	UINT nLen = lstrlen(pFileName);
	char* pszUtf8 = (char*)calloc(nLen*4,1);
	WideCharToMultiByte(CP_UTF8, 0, pFileName, -1, pszUtf8, nLen*4, 0, 0);
	BOOL lbRc = FALSE;
	TRY {
		lbRc = fFileOpen(pszUtf8, lFileSize, pBuf, lBuf, &pii, &pImgContext);
	} CATCH {
		SetException(L"pvdFileOpen"); lbRc = FALSE;
	}
	free(pszUtf8);

	if (lbRc) {
		ImageContextWrap* pi = (ImageContextWrap*)calloc(sizeof(ImageContextWrap),1);
		pi->pImageContext = pImgContext;
		pImageInfo->pImageContext = pi;              // [OUT] ��������, ������������ ��� ��������� � �����
		pImageInfo->nPages = pii.nPages;             // [OUT] ���������� ������� �����������
		pImageInfo->Flags = pii.Flags;               // [OUT] ��������� �����: PVD_IIF_xxx
		pImageInfo->pFormatName = pi->pFormatName = ConvertToWide(pii.pFormatName);   // [OUT] �������� ������� �����
		pImageInfo->pCompression = pi->pCompression = ConvertToWide(pii.pCompression); // [OUT] �������� ������
		pImageInfo->pComments = pi->pComments = ConvertToWide(pii.pComments);       // [OUT] ��������� ����������� � �����
	}
	return lbRc;
}

BOOL PVDDecoderVer1::PageInfo2(void *pImageContext, pvdInfoPage2 *pPageInfo)
{
	if (!bInitialized) return FALSE;
	_ASSERTE(pPageInfo->cbSize == sizeof(pvdInfoPage2));
	
	ImageContextWrap* pi = (ImageContextWrap*)pImageContext;
	pvdInfoPage pip = {0};
	BOOL lbRc = FALSE;
	TRY {
		lbRc = fPageInfo(pi->pImageContext, pPageInfo->iPage, &pip);
	} CATCH {
		SetException(L"pvdPageInfo"); lbRc = FALSE;
	}
	if (lbRc) {
		pi->lWidth = pip.lWidth;                 // ���������, ������ ������ ���������� �� ����� ��������������
		pi->lHeight = pip.lHeight;               // ����� � ��������� pvdInfoDecode
		pPageInfo->lWidth = pip.lWidth;          // [OUT] ������ ��������
		pPageInfo->lHeight = pip.lHeight;        // [OUT] ������ ��������
		pPageInfo->nBPP = pip.nBPP;              // [OUT] ���������� ��� �� ������� (������ �������������� ���� - � �������� �� ������������)
		pPageInfo->lFrameTime = pip.lFrameTime;  // [OUT] ��� ������������� ����������� - ������������ ����������� �������� � �������� �������;
												 //       ����� - �� ������������
	}
	return lbRc;
}

BOOL PVDDecoderVer1::DecodeCallbackWrap(void *pDecodeCallbackContext, UINT32 iStep, UINT32 nSteps)
{
	DecodeCallbackArg* p = (DecodeCallbackArg*)pDecodeCallbackContext;
	return p->DecodeCallback(p->pDecodeCallbackContext, iStep, nSteps, NULL);
}

BOOL PVDDecoderVer1::PageDecode2(void *pImageContext, pvdInfoDecode2 *pDecodeInfo, pvdDecodeCallback2 DecodeCallback, void *pDecodeCallbackContext)
{
	if (!bInitialized) return FALSE;
	_ASSERTE(pDecodeInfo->cbSize == sizeof(pvdInfoDecode2));
	
	ImageContextWrap* pi = (ImageContextWrap*)pImageContext;
	pvdInfoDecode *pid = (pvdInfoDecode*)calloc(sizeof(pvdInfoDecode),1);
	DecodeCallbackArg arg = {DecodeCallback, pDecodeCallbackContext};
	BOOL lbRc = FALSE;
	TRY {
		lbRc = fPageDecode(pi->pImageContext, pDecodeInfo->iPage, pid, DecodeCallbackWrap, &arg);
	} CATCH {
		SetException(L"pvdPageDecode"); lbRc = FALSE;
	}
	if (lbRc) {
		pDecodeInfo->lWidth = pi->lWidth;              // [OUT] ������ �������������� ������� (pImage)
		pDecodeInfo->lHeight = pi->lHeight;            // [OUT] ������ �������������� ������� (pImage)
		pDecodeInfo->nBPP = pid->nBPP;                 // [IN]  PicView ����� ��������� ���������������� ������
								                       // [OUT] ���������� ��� �� ������� � �������������� �����������
								                       //       32 ���� ����� ���� ��� ������������� PVD_IDF_ALPHA
		pDecodeInfo->lImagePitch = pid->lImagePitch;   // [OUT] ������ - ����� ������ ��������������� ����������� � ������;
								                       //       ������������� �������� - ������ ���� ������ ����, ������������� - ����� �����
		pDecodeInfo->Flags = pid->Flags;               // [OUT] ��������� �����: PVD_IDF_xxx
			if (pDecodeInfo->Flags < 1 && pid->nBPP == 32) pDecodeInfo->Flags |= PVD_IDF_ALPHA;
		pDecodeInfo->nTransparentColor = -1;           // [OUT] ���� ������ PVD_IDF_TRANSPARENT - �������� ����, ������� ��������� ����������
								                       //       ��� ������� � �������� - �������� ������ ����������� �����
		pDecodeInfo->pImage = pid->pImage;             // [OUT] ��������� �� ������ ����������� � ���������� ������� (PVD_IDF_xxx, �� ��������� - RGB)
		pDecodeInfo->pPalette = pid->pPalette;         // [OUT] ��������� �� ������� �����������, ������������ � �������� 8 � ������ ��� �� �������
		pDecodeInfo->nColorsUsed = pid->nColorsUsed;   // [OUT] ���������� ������������ ������ � �������; ���� 0, �� ������������ ��� ��������� �����
		pDecodeInfo->lParam = (LPARAM)pid;             // [OUT] ��������� ����� ������������ ��� ���� �� ���� ����������
		pDecodeInfo->ColorModel = (pDecodeInfo->Flags & PVD_IDF_ALPHA) ? PVD_CM_BGRA : PVD_CM_BGR;

	} else {
		free(pid);
	}
	return lbRc;
}

void PVDDecoderVer1::PageFree2(void *pImageContext, pvdInfoDecode2 *pDecodeInfo)
{
	if (!bInitialized) return;
	ImageContextWrap* pi = (ImageContextWrap*)pImageContext;
	TRY {
		fPageFree(pi->pImageContext, (pvdInfoDecode*)pDecodeInfo->lParam);
	} CATCH {
		SetException(L"pvdPageFree");
	}
}

void PVDDecoderVer1::FileClose2(void *pImageContext)
{
	if (!bInitialized) return;
	ImageContextWrap* pi = (ImageContextWrap*)pImageContext;
	if (pi) {
		LPCWSTR pszFunc = L"pvdFileClose";
		TRY {
			fFileClose(pi->pImageContext);
			pszFunc = L"pvdFileClose.free";
			if (pi->pFormatName) free(pi->pFormatName);
			if (pi->pCompression) free(pi->pCompression);
			if (pi->pComments) free(pi->pComments);
			free(pi);
		} CATCH {
			SetException(pszFunc);
		}
	}
}
