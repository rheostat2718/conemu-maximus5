
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


/*

 ����� ������������ ���:
 1. ����������� ������������ ������� ��� �������
 2. �������� �������� ����������� � �������� �������� �����������
 
 ��������!
 ��� ����� �������� ����� ���� ��������� ������������ �������.
 ��� ������ ����� (�����, � �������� ������������� �����) ����� ����
 ��������� ������������ ��������� (���� ��� ������ �� �������������?)

 WARNING! ���������� ������ ������ � ���� �������!
 
*/

#include "PictureView.h"
#include "PVDManager.h"
#include "Image.h"
#include "ImagePages.h"
#include "DecodeItem.h"
#include "DecodeParams.h"
#include "DecoderHandle.h"
#include "DisplayHandle.h"

LPCSTR szImagePagesItem = "ImagePagesItem";

ImagePagesItem::ImagePagesItem()
{
	memset(this, 0, sizeof(*this));
}

ImagePagesItem::~ImagePagesItem()
{
	_ASSERTE(pDraw==NULL);
	SafeRelease(pDraw,szImagePagesItem);
}

void ImagePagesItem::Assign(CDecodeItem* apFrame)
{
	TODO("bScreenSize");
	nLastAccess = GetTickCount();
	nPage = apFrame->Params.nPage;
	Info = apFrame->Info;
	Params = apFrame->Params; // ���������� ������ ��������� ����
	if (pDraw != apFrame->pDraw)
	{
		SafeRelease(pDraw,szImagePagesItem);
		if (apFrame->pDraw)
			apFrame->pDraw->AddRef(szImagePagesItem);
		pDraw = apFrame->pDraw;
	}
}


CImagePages::CImagePages(CImage* apImage)
{
	//_ASSERTE(GetCurrentThreadId() == gnDisplayThreadId);
	InitializeCriticalSection(&csPages);

	//if (apImage)
	//	apImage->AddRef(szImagePagesItem);
	//mp_Image = apImage;

	// ��������� ��� �� ������ �������������, ��� ��� ��� �� �����
	mb_Animated = false;
	//mn_StoreSize = 0;
	//mp_Store = NULL;
}

CImagePages::~CImagePages()
{
	if (!m_Store.empty())
	{
		CloseDisplay();
	}
	DeleteCriticalSection(&csPages);
}

// � apFrame ������ ������ ���� ��� ������� - ���� ������ ���������� �� ��������
// ����� ����� ���� (�� �� ������) ���������� ����� � (��� �������) ���������� �������
void CImagePages::Store(CDecodeItem* apFrame)
{
	_ASSERTE(GetCurrentThreadId() == gnDisplayThreadId);
	if (!apFrame)
	{
		_ASSERTE(apFrame!=NULL);
		return;
	}

	EnterCriticalSection(&csPages);

	if (apFrame->Info.Animation)
		mb_Animated = true;

	bool bReady = false;
	std::vector<ImagePagesItem*>::iterator i;
	for (i = m_Store.begin(); i != m_Store.end(); ++i)
	{
		if ((*i)->nPage == apFrame->Params.nPage)
		{
			if (apFrame->Params.Compare(&(*i)->Params))
			{
				bReady = true;
				(*i)->Assign(apFrame);
			}
		}
	}

	if (!bReady)
	{
		ImagePagesItem* item = new ImagePagesItem;
		item->Assign(apFrame);
		m_Store.push_back(item);
	}

	LeaveCriticalSection(&csPages);
}

// ������� �������� ���������� ���������� �������
// �� CDisplayHandle ����� �������� AddRef, ���������� ������� ������ ��� Release
// CDisplayHandle �������� ��������� �� ������ ������� � �������� ��� ���������
bool CImagePages::GetDrawHandle(CDisplayHandlePtr& rDraw, uint anPage, DecodeParams *apParams /*= NULL*/)
{
	//_ASSERTE(GetCurrentThreadId() == gnDisplayThreadId);

	WARNING("��� �� �������� ������� '���������' �����������, ����� �� ��������� ������ ��� �� ������?");
	
	EnterCriticalSection(&csPages);

	// �� ������ �������� nLastAccess ��� ����������
	
	bool bFound = false;
	std::vector<ImagePagesItem*>::iterator i;
	for (i = m_Store.begin(); i != m_Store.end(); ++i)
	{
		if ((*i)->nPage == anPage)
		{
			if (!apParams || apParams->Compare(&(*i)->Params))
			{
				bFound = true;
				(*i)->nLastAccess = GetTickCount();
				rDraw.Assign((*i)->pDraw);
				break;
			}
		}
	}

	if (!bFound)
		rDraw.Assign(NULL);

	LeaveCriticalSection(&csPages);
	
	return rDraw.IsValid();
}

void CImagePages::CheckPagesClosed()
{
	EnterCriticalSection(&csPages);

	std::vector<ImagePagesItem*>::iterator i;
	for (i = m_Store.begin(); i != m_Store.end(); ++i)
	{
		_ASSERTE((*i)->pDraw == NULL);
	}

	LeaveCriticalSection(&csPages);
}

void CImagePages::CloseDisplay()
{
	EnterCriticalSection(&csPages);

	if (!m_Store.empty())
	{
		if ((GetCurrentThreadId() != gnDisplayThreadId) && (gnDisplayThreadId != 0))
		{
			_ASSERTE(GetCurrentThreadId() == gnDisplayThreadId || gnDisplayThreadId == 0);
		}

		std::vector<ImagePagesItem*>::iterator i;
		for (i = m_Store.begin(); i != m_Store.end(); i = m_Store.erase(i))
		{
			_ASSERTE(GetCurrentThreadId() == gnDisplayThreadId);

			ImagePagesItem* p = *i;
			SafeRelease(p->pDraw,szImagePagesItem);
			delete p;
		}
	}

	LeaveCriticalSection(&csPages);
}

void CImagePages::FreeUnused()
{
	if (!mb_Animated)
	{
		TODO("���������� ��������������");
	}
}
