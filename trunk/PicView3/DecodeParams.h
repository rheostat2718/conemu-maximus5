
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


class CDecodeItem;

enum DecodingFlags
{
	// ������� �����
	eRenderFull = 1,
	// ��� ������ ������
	eRenderScale = 2,
	// ������������� ������ ������� �����������
	eRenderRectScale = 4,
	// ������ �������� �����, ��� ����� ������ ���� ������� ��������.
	eRenderResetDecoder = 0x100,
	// ��������� ��������� �������������
	eRenderWaitOpened = 0x1000,
	// ����� �������� �������������
	eRenderNextDecoder = 0x10000,
	eRenderPrevDecoder = 0x20000,
	// "���������" ������ �������� (������ �������� ����� ��� ������ ������?)
	eRenderForceDecoder = 0x40000,
	// ������ �������� �����
	eRenderFirstImage = 0x80000,
	// �� ��������� ������������� - ���������� �� ������ ��� ����������
	eRenderActivateOnReady = 0x100000,
	// ��� PgUp/PgDn � RawIndex ����������� ����������� (-1, 1), � �� ������ 
	eRenderRelativeIndex   = 0x200000,
	// (PgUp/PgDn/Home/End) ������� �� ��� ���, ���� �� ���������� ����, ������� ������ ������������ � ��������
	eRenderFirstAvailable  = 0x400000,
	// ���������� �����������
	eRenderReleaseDescriptor = 0x800000,
};

enum DecodingPriority
{
	eCurrentImageCurrentPage = 0, // highest
	eCurrentImageNextPage,
	eDecodeNextImage,
	eDecodeAny,
};

enum DecodingStatus
{
	eItemEmpty = 0,
	eItemFailed,
	eItemRequest,
	eItemReady,
	eItemDecoding,
};

struct DecodeParams
{
public:
	// DecodingFlags: eRenderFull|eRenderScale|eRenderRectScale|eRenderWaitOpened
	//                eRenderNextDecoder|eRenderPrevDecoder|eRenderForceDecoder
	//                eRenderFirstImage|eRenderActivateOnReady|eRenderRelativeIndex|eRenderFirstAvailable
	DWORD Flags;

	// ������ ����� �� ������ (��� �������� ������, ��� "�������������" �������������)
	int   nRawIndex;
	// ��� ������������� ����������/����������� ����� (eRenderRelativeIndex)
	int   iDirection;
	// ����� ����� ����� ������������
	int   nPage;
	
	// ��������� �������������: eCurrentImageCurrentPage | eCurrentImageNextPage | eNextImage
	enum DecodingPriority Priority;

	// ��� ������ ������ �� OpenFilePluginW ������ ��� ����� ���� ���������
	const unsigned char *buf /*= NULL*/;
	int lBuf /*= 0*/;
	
	//TODO
	RECT rcSource;
	SIZE szTarget;
	
	
	// For internal use!
	enum DecodingStatus *pResult;
	int nFromRawIndex;
	CDecodeItem *pDecodeItem;
	

public:	
	// �����������, �������� ������������, ������
	DecodeParams()
	{
		memset(this, 0, sizeof(DecodeParams));
	}

	DecodeParams& operator=(DecodeParams& a);
	void OnItemReady();
	bool Compare(DecodeParams* pOther);
};
