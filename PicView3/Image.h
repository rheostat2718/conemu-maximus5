
#pragma once

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


class CPVDManager;
//class DirectDrawSurface;
class CModuleInfo;
class CDecoderHandle;
class CDisplayHandle;
struct DecodeParams;
class CImagePages;
class CDecodeItem;
class CDisplayHandlePtr;

#include "PictureView_FileName.h"
#include "PVDInterface/PictureViewPlugin.h"

#include "RefRelease.h"
#include "ImageInfo.h"

#include <vector>

// ����������� �� RefRelease, �.�. ������ �� ������ ����� ��������������
// � ������ �����, �������� � ������� �������������
class CImage : public CRefRelease
{
public:
	CImage(LPCSTR asFrom, const wchar_t *apFileName, int anPanelItemRaw);
	static void OnTerminate();
protected:
	virtual ~CImage();
public:
	//bool ImageOpen(DecodeParams *apParams, const unsigned char *buf /*= NULL*/, int lBuf /*= 0*/);
	void CheckPages(uint anNewPages);
	void Close();
	void CloseImage();
	void CloseDisplay();
	SIZE GetDefaultScaleSize();

#ifdef _DEBUG
	virtual void AddRef(LPCSTR asFrom)
	{
		CRefRelease::AddRef(asFrom);
	};
	virtual int Release(LPCSTR asFrom)
	{
		int i = CRefRelease::Release(asFrom);
		return i;
	};
#endif

private:
	CImagePages* mp_Pages;
//private:
//	bool ImageDecode(void);
public:
	void CheckPagesClosed();

	//CPVDManager *Decoder; -- ������ ��������������� � CImage

	//int mi_DecoderIndex;
private:
	// �������������, ����� ��������� �������(���������) � ��������� ���
	CModuleInfo* mp_Decoder; // RefRelease
public:
	void SetDecoder(CModuleInfo* apDecoder);
	CModuleInfo* GetDecoder();


	// ��� ���-���� ������ ���� � CImage, �.�. ���� ���� ���� ��� �����������
	// �������� �������, ����� ��������� ������������� �� ���� �������� �� ���� ������ �������,
	// � ���� ������ ���������� ���������� ������� ���������, ���� ��� �� ��������,
	// ��� ���� �� ���������� �����-�� ������ �������.
	WARNING("�������� ������ mpb_Processed ==> mpb_DecoderFailed");
	// true �������� �� ����� �������� �������� ���������,
	// � ����� ����, ��� �� ����������, ��� ���������� ��� ������ �������
	// �� ������, ������� ������ ���� �������.
private:
	//bool* mpb_DecoderFailed; uint mn_DecoderFailedCount; // ������ ���������� ��� �������� ������� ����
	static CRITICAL_SECTION mcs_DecoderFailed;
	static bool mb_CSInitialized;
	std::vector<const void*> mp_DecoderFailed;
public:
	//void CheckDecoderFailedSize(); -- �� ���������
	bool WasDecoderFailed(CModuleInfo* apDecoder);
	void SetDecoderFailed(CModuleInfo* apDecoder);
	void ResetDecoderFailed();


	WARNING("������� Private");
	CDecoderHandle* mp_File; // ��������, ������������(!) � ������ ������ � ���� �������
	void DecoderHandleReleased(CDecoderHandle* apFile);

	WARNING("������� Private");
	//CDisplayHandle* mp_Draw; // ��������, ������������(!) � ������ ������ � ���� �������

	CDecoderHandle* GetDecoderHandle(LPCSTR asFrom);
	bool GetDrawHandle(CDisplayHandlePtr& rDraw, DecodeParams *apParams = NULL);

	void StoreDecodedFrame(CDecodeItem* apFrame);

	//bool mb_ImageOpened;
	////wchar_t* mps_FileName;
	//void *mp_ImageContext;
	////TODO("������������� mp_Data � mp_DecoderModule, ����� �� ��� ������ ���� Decoder ������������� � g_Manager");
	//CModuleInfo* mp_DecoderModule;

	//TODO("������������� Display � mp_DisplayModule");
	//CModuleInfo *Display;
	//LPVOID pDraw;
	//pvdInfoPage2 InfoPage;

	CUnicodeFileName FileName;
	LPCWSTR pszFileNameOnly;

	
	ImageInfo Info;
	//uint lWidth, lHeight, nBPP;
	//uint lDecodedWidth, lDecodedHeight, nDecodedBPP;
	//DWORD lOpenTime, lStartOpenTime;
	//DWORD lTimeOpen, lTimeDecode, lTimeTransfer, lTimePaint;
	//BOOL  bTimePaint;
	//uint nPages, nPage;
	//uint Animation;
	//uint lFrameTime;
	//
	//wchar_t DecoderName[0x80], FormatName[0x80], Compression[0x80], Comments[0x100];

	bool IsValid();
	bool IsActive();
	int PanelItemRaw();
	//void SetPanelItemRaw(uint nRaw);
private:
	int nPanelItemRaw;
};
//typedef CImage* PImageInfo;
