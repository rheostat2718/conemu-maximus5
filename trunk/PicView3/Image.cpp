
/**************************************************************************
Copyright (c) 2011 Maximus5
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


#include "PictureView.h"
#include "PictureView_Lang.h"
#include "PVDManager.h"
#include "Image.h"
#include "ImagePages.h"
#include "FileMap.h"
#include "DecoderHandle.h"
#include "DisplayHandle.h"

extern RECT GetDisplayRect();

CRITICAL_SECTION CImage::mcs_DecoderFailed;
bool CImage::mb_CSInitialized = false;

LPCSTR szImage = "CImage";

CImage::CImage(LPCSTR asFrom, const wchar_t *apFileName, int anPanelItemRaw)
	: CRefRelease(asFrom)
{
	if (!mb_CSInitialized)
	{
		mb_CSInitialized = TRUE;
		InitializeCriticalSection(&mcs_DecoderFailed);
	}

	FileName = apFileName;
	nPanelItemRaw = anPanelItemRaw;
	pszFileNameOnly = g_Panel.ItemPointToName((LPCWSTR)FileName);

	mp_Decoder = NULL;
	mp_File = NULL;
	//mp_Draw = NULL;

	mp_Pages = new CImagePages(this);

#if 0
	mi_DecoderIndex = -1;
	mp_ImageContext = NULL;
	mp_DecoderModule = NULL;
	mb_ImageOpened = false;
	//mps_FileName = NULL;
	mpb_DecoderFailed = NULL; mn_DecoderFailedCount = 0;

	//Decoder = NULL;
	pDraw = NULL; Display = NULL;
#endif

	//FileNamePrefix[0] = 0; // \\?\ ...
	//FileNameData[0] = 0;

	////bSelected = false;
	//lWidth = lHeight = nBPP = lDecodedWidth = lDecodedHeight = nDecodedBPP = 0;
	////OpenTimes[0] = 0;
	//lOpenTime = lStartOpenTime = 0;
	//lTimeOpen = lTimeDecode = lTimeTransfer = lTimePaint = 0;
	//bTimePaint = FALSE;
	//nPages = nPage = Animation = lFrameTime = 0;

	//DecoderName[0] = 0; FormatName[0] = 0; Compression[0] = 0; Comments[0] = 0;

	//Decoder = new CPVDManager(this);
	//_ASSERTE(Decoder);

	//dds = new DirectDrawSurface;
	
	//memset(&InfoPage, 0, sizeof(InfoPage));
}

CImage::~CImage()
{
	// ���������� ��������� �� �������
	SetDecoder(NULL);

	// � ���, ��� ��������
	Close();
	
	//_ASSERTE(mp_ImageContext == NULL);
	//_ASSERTE(mb_ImageOpened == FALSE);
	//_ASSERTE(pDraw == NULL);
	//if (mp_DecoderModule) // pData ��� ������ �� pPlugin, � ��� ������������� � CPVDManager::UnloadPlugins2
	//	mp_DecoderModule = NULL;

	//SafeFree(mpb_DecoderFailed); mn_DecoderFailedCount = 0;
}

void CImage::OnTerminate()
{
	if (mb_CSInitialized)
	{
		mb_CSInitialized = false;
		DeleteCriticalSection(&mcs_DecoderFailed);
	}
}

void CImage::Close()
{
	if (!gnDecoderThreadId || GetCurrentThreadId() == gnDecoderThreadId)
	{
		CloseImage();
	}
	
	CloseDisplay();

	// ������� ��������� �� ��������
	if (mp_Pages)
	{
		delete mp_Pages;
		mp_Pages = NULL;
	}
}

void CImage::CloseImage()
{
	SafeRelease(mp_File,szPVDManager);

	// mpb_DecoderFailed ������� �� �����!
	// �������� �������, ����� ��������� ������������� �� ���� �������� �� ���� ������ �������,
	// � ���� ������ ���������� ���������� ������� ���������, ���� ��� �� ��������,
	// ��� ���� �� ���������� �����-�� ������ �������.
}

void CImage::CloseDisplay()
{
	//SafeRelease(mp_Draw,szImage);
	if (mp_Pages)
		mp_Pages->CloseDisplay();
}

// � �������� ������������� ����� ��������� ������� ���������� �������, ���
// ���� ���������� ��� ��������� ����������� �����
void CImage::CheckPages(uint anNewPages)
{
	Info.UpdatePageCount(anNewPages);
}

bool CImage::IsValid()
{
	if (!this)
	{
		_ASSERTE(this!=NULL);
		return false;
	}
	TODO("������ �� ��� ��������� ������� ��������, ����� ������������ ������ ����������");
	return true;
}

bool CImage::IsActive()
{
	if (!IsValid())
		return false;
	
	return (nPanelItemRaw == g_Panel.GetActiveRawIdx());
}

int CImage::PanelItemRaw()
{
	_ASSERTE(nPanelItemRaw >= 0);
	return nPanelItemRaw;
}

//void CImage::SetPanelItemRaw(uint nRaw)
//{
//	TODO("��� ��������� ����� ���� -1 � g_Plugin.FarPanelInfo.ItemsNumber. � ���� ������ ��� ����� ��� ����� ��������");
//	if (nPanelItemRaw != nRaw) {
//		nPanelItemRaw = nRaw;
//		nPage = 0; // ����� ������ ��������
//	}
//}

//TODO("������ �������� piSubDecoder");
//bool CImage::ImageOpen(DecodeParams *apParams, const unsigned char *buf /*= NULL*/, int lBuf /*= 0*/)
//{
//	_ASSERTE(!FileName.IsEmpty());
//
//	// ������ ���� ��� ������, �� ��������
//	g_Plugin.InitCMYK(FALSE); // ��������� ��� ����������
//
//
//	i64 lFileSize;
//	
//	FileMap Map((LPCWSTR)FileName);
//	lFileSize = Map.lSize;
//	if (!buf || !lBuf)
//		if (Map.MapView())
//		{
//			buf = Map.pMapping;
//			lBuf = (int)Min<i64>(Map.lSize, ~0u);
//			//if (lBuf) -- IN_APL ����� �� �������
//			//	lFileSize = 0; // ��� ������ ��� � ������
//		}
//		else
//		{
//			buf = NULL;
//			lBuf = 0;
//		}
//
//	bool result = false;
//
//	TODO("��������� � ������ �������");
//	this->lStartOpenTime = timeGetTime();
//
//
//	// ���������� ������! �� ������ ����� ��������� � ������ AltPgUp �������� (����� ��������)
//	//Decoder->mi_SubDecoder = *piSubDecoder;
//	
//lReOpen2:
//	memset(&InfoPage, 0, sizeof(InfoPage)); // !!! cbSize �� ���������
//	result = g_Manager.Open(this, (LPCWSTR)FileName, lFileSize, buf, lBuf, apParams);
//
//	if (!result || (g_Plugin.FlagsWork & FW_TERMINATE))
//	{
//		return false;
//	}
//
//	// ��� ������ �������� ����� - �(�)������� (���� ���������) �����������
//	// ����������� ���������� PgUp/PgDn (������� �������� ��� �����)
//	if (apParams->Flags & eRenderFirstImage)
//	{
//		g_Manager.OnFirstImageOpened(this);
//		if ((g_Plugin.FlagsWork & FW_TERMINATE))
//			result = false;
//	}
//		
//
//	if (result)
//	{
//		TODO("������ �� ��������� ImageDecode � ������ ���� Display. ���������� ������������� ����� ������ ��� �� �������� DX � �������");
//		if (!ImageDecode())
//		{
//			WARNING("���� ImageDecode ������������ - �� �� �������������, �.�. ImageOpen �������� ��� �� �������!");
//			WARNING("� ����� ������ ����� �������������� �������? �� ��������� pPVDDecoder � ��� ��������� ���������� bProcessed");
//			uint nDecoders = (uint)CPVDManager::Decoders.size();
//			if (nDecoders <= 1)
//				return false;
//
//			if (g_Plugin.FlagsWork & FW_TERMINATE)
//				return false; // ������ �����������
//
//			mi_SubDecoder++;
//			if (mi_SubDecoder >= nDecoders)
//				mi_SubDecoder = 0;
//
//			goto lReOpen2;
//		}
//		else
//		{
//			// �������� �������������. ����� ������ "������� �������� ��� ����� �����������".
//			g_Manager.ResetProcessed(this);
//		}
//	}
//	
//	return result;
//}

//bool CImage::ImageDecode(void)
//{
//	//this->iPage = 0;
//	const uint nPages = this->nPages;
//	//_ASSERTE(nPage < nPages);
//	// ���������� ��������. ������� djvu ����� djvu.pvd
//	// ������������� �� ������ ������� (AltPgDn), ���������� Shell.pvd, � �� ����� ������
//	// ��������� - �� ���� ������ ���� ��������.
//	if (nPage >= nPages) 
//		nPage = 0;
//	
//	WARNING("!!! ������� � ������� ����� ��������� � ���� ������� !!!");
//		
//	// InfoPage - ��� ���� ��� �������, ����� ������� �� ����� �������� ������� pvdPageInfo
//	bool result = g_Manager.Decode(this, &pDraw, false);
//	// ���������� ������� (���� ��� ����������) ����������� ����� CheckPages(...)
//	// ���� AI �������. ����� PVD_IIF_ANIMATED ���������� ��� PVD
//	//if (nPages != this->nPages) // buggy GFL with animated GIF
//	//	this->Animation = 1;
//	// ���� ������ �� ���������
//	//TODO("����� ����������� ���������� ������������� �������� �������� �� ��������� � � ����� ���������");
//	//if (!result || this->nPages < 2)
//	//{
//	//	this->Decoder->Close();
//	//	//deletex(this->Decoder);
//	//}
//
//	if (!result)
//	{
//		g_Manager.Close(this);
//	} else {
//		//if (this == g_Plugin.Image[0])
//		if (IsActive())
//		{
//			// ����� � GDI ���-�� �� ����������������
//			TODO("�� �������. ��� (this != g_Plugin.Image[0])");
//			InvalidateRect(g_Plugin.hWnd, NULL, FALSE);
//		}
//
//		//if (g_Plugin.FlagsWork & FW_JUMP_DISABLED) {
//		//
//		//	this->SetPanelItemRaw(PANEL_ITEM_UNAVAILABLE);
//		//	
//		//}
//	}
//	
//	return result;
//}




SIZE CImage::GetDefaultScaleSize()
{
	RECT ParentRect = GetDisplayRect(); //{0, 0, 1280, 960};
	
	//const �������� ������������� ���������� �� ����� � ������� (�� �����������). ��� ��� �� �������� - ������� ������ ������������������ ��������!
	int lScreenWidth = ParentRect.right - ParentRect.left;
	int lScreenHeight = ParentRect.bottom - ParentRect.top;

	RECT ImgRect = {0,0,lScreenWidth,lScreenHeight};
	
	// ��� ���������� �� this->lWidth & this->lHeight
	// � InfoPage.lWidth & InfoPage.lHeight. �.�. ������ ���
	// ����� ��������� ���������� ���������� � ������������ �������� ���������������� ���������
	
	u32 Zoom;
	if (g_Plugin.ZoomAuto || g_Plugin.FlagsWork & FW_QUICK_VIEW)
	{
		//const �������� ������������� ���������� �� ����� � ������� (�� �����������). ��� ��� �� �������� - ������� ������ ������������������ ��������!
		u32 ZoomW = MulDivU32(lScreenWidth, 0x10000, Info.lWidth /*dds->m_lWorkWidth*/);
		u32 ZoomH = MulDivU32(lScreenHeight, 0x10000, Info.lHeight /*dds->m_lWorkHeight*/);
		Zoom = (g_Plugin.ZoomAuto == ZA_FIT || g_Plugin.FlagsWork & FW_QUICK_VIEW) ? Min(ZoomW, ZoomH) : Max(ZoomW, ZoomH);
		if (!Zoom)
			Zoom = 1;
		if (g_Plugin.bAutoZoomMin && Zoom < g_Plugin.AutoZoomMin)
			Zoom = g_Plugin.AutoZoomMin;
		if (g_Plugin.bAutoZoomMax && Zoom > g_Plugin.AutoZoomMax)
			Zoom = g_Plugin.AutoZoomMax;
	}
	else
	{
		Zoom = g_Plugin.AbsoluteZoom ? g_Plugin.AbsoluteZoom : g_Plugin.Zoom;
	}

	
	//const �������� ������������� ���������� �� ����� � ������� (�� �����������). ��� ��� �� �������� - ������� ������ ������������������ ��������!
	int lWidth = MulDivU32R(Info.lWidth, Zoom, 0x10000);
	int lHeight = MulDivU32R(Info.lHeight, Zoom, 0x10000);
	
	SIZE sz = {lWidth,lHeight};
	
	return sz;
}


void CImage::SetDecoder(CModuleInfo* apDecoder)
{
	// mp_Decoder ������������ ������ �������������, ��� �������� ���������,
	// ������ � ����������� WasDecoderFailed/SetDecoderFailed/ResetDecoderFailed

	//_ASSERTE(!apDecoder || (mp_File && (apDecoder == mp_File->Decoder())));

	if (mp_Decoder != apDecoder)
	{
		if (apDecoder)
			apDecoder->AddRef(szImage);

		if (mp_Decoder)
			mp_Decoder->Release(szImage);

		mp_Decoder = apDecoder;
	}
}

CModuleInfo* CImage::GetDecoder()
{
	WARNING("������? ������ ���� ������ � mp_File?");
	return mp_Decoder;
}


// ��� ���-���� ������ ���� � CImage, �.�. ���� ���� ���� ��� �����������
// �������� �������, ����� ��������� ������������� �� ���� �������� �� ���� ������ �������,
// � ���� ������ ���������� ���������� ������� ���������, ���� ��� �� ��������,
// ��� ���� �� ���������� �����-�� ������ �������.
// WARNING!!! �������� ������ mpb_DecoderFailed ==> mpb_DecoderFailed
// true �������� �� ����� �������� �������� ���������,
// � ����� ����, ��� �� ����������, ��� ���������� ��� ������ �������
// �� ������, ������� ������ ���� �������.
//void CImage::CheckDecoderFailedSize()
//{
//	uint nDecoders = (uint)CPVDManager::Decoders.size();
//	if (!mpb_DecoderFailed || mn_DecoderFailedCount != nDecoders)
//	{
//		SafeFree(mpb_DecoderFailed);
//		mn_DecoderFailedCount = nDecoders;
//		mpb_DecoderFailed = (bool*)calloc(nDecoders,sizeof(*mpb_DecoderFailed));
//	}
//}

bool CImage::WasDecoderFailed(CModuleInfo* apDecoder)
{
	bool lbFailed = false;

	EnterCriticalSection(&mcs_DecoderFailed);
	for (std::vector<const void*>::iterator iter = mp_DecoderFailed.begin(); iter != mp_DecoderFailed.end(); ++iter)
	{
		const void* p = *iter;
		if (p == (const void*)apDecoder)
		{
			lbFailed = true;
			break;
		}
	}
	LeaveCriticalSection(&mcs_DecoderFailed);
	
	return lbFailed;
}

void CImage::SetDecoderFailed(CModuleInfo* apDecoder)
{
	EnterCriticalSection(&mcs_DecoderFailed);
	if (!WasDecoderFailed(apDecoder))
	{
		mp_DecoderFailed.push_back((const void*)apDecoder);
	}
	LeaveCriticalSection(&mcs_DecoderFailed);
}

void CImage::ResetDecoderFailed()
{
	EnterCriticalSection(&mcs_DecoderFailed);
	mp_DecoderFailed.clear();
	LeaveCriticalSection(&mcs_DecoderFailed);
}

// �� CDecoderHandle ����� �������� AddRef, ���������� ������� ������ ��� Release
CDecoderHandle* CImage::GetDecoderHandle(LPCSTR asFrom)
{
	if (mp_File)
		mp_File->AddRef(asFrom);
	return mp_File;
}

// �� CDisplayHandle ����� �������� AddRef, ���������� ������� ������ ��� Release
bool CImage::GetDrawHandle(CDisplayHandlePtr& rDraw, DecodeParams *apParams /*= NULL*/)
{
	//if (!Info.lDecodedWidth || !Info.lDecodedHeight)
	//	return NULL; // ��� �� ������������!
	if ((this == NULL) || (mp_Pages == NULL))
	{
		_ASSERTE(this && mp_Pages);
		return false;
	}

	bool bValid = false;

	// ������� ���������� ���������� ������� ��� �������� ������ (����������� NULL ���� ��� �� ������)
	if (apParams)
		bValid = mp_Pages->GetDrawHandle(rDraw, apParams->nPage, apParams);
	else
		bValid = mp_Pages->GetDrawHandle(rDraw, Info.nPage);

	return bValid;
}

void CImage::StoreDecodedFrame(CDecodeItem* apFrame)
{
	_ASSERTE(GetCurrentThreadId() == gnDisplayThreadId);
	if ((this == NULL) || (mp_Pages == NULL))
	{
		_ASSERTE(this && mp_Pages);
		return;
	}

	mp_Pages->Store(apFrame);
}

void CImage::CheckPagesClosed()
{
	if (this == NULL)
	{
		_ASSERTE(this);
		return;
	}

	if (mp_Pages)
		mp_Pages->CheckPagesClosed();
}
