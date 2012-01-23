
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

#include <windows.h>
#include "common.hpp"
#include "RgnDetect.h"
#include "UnicodeChars.h"

#ifndef RGN_ERROR
#define RGN_ERROR 0
#endif

WARNING("�������� *mp_FarInfo �� instance ����� m_FarInfo");


static bool gbInTransparentAssert = false;

//#ifdef _DEBUG
//	#undef _ASSERTE
//	#define _ASSERTE(expr) gbInTransparentAssert=true; _ASSERT_EXPR((expr), _CRT_WIDE(#expr)); gbInTransparentAssert=false;
//	#undef _ASSERT
//	#define _ASSERT(expr) gbInTransparentAssert=true; _ASSERT_EXPR((expr), _CRT_WIDE(#expr)); gbInTransparentAssert=false;
//#endif


CRgnDetect::CRgnDetect()
{
	mb_SelfBuffers = FALSE;
	mn_DetectCallCount = 0;
	memset(&m_DetectedDialogs, 0, sizeof(m_DetectedDialogs));
	mp_FarInfo = NULL;
	// �����
	mn_NextDlgId = 0; mb_NeedPanelDetect = TRUE;
	memset(&mrc_LeftPanel,0,sizeof(mrc_LeftPanel));
	memset(&mrc_RightPanel,0,sizeof(mrc_RightPanel));
	memset(&mrc_FarRect, 0, sizeof(mrc_FarRect)); // �� ��������� - �� ������������
	//
	mpsz_Chars = NULL;
	mp_Attrs = mp_AttrsWork = NULL;
	mn_CurWidth = mn_CurHeight = mn_MaxCells = 0;
	mb_SBI_Loaded = false;
	mb_TableCreated = false;
	mb_NeedTransparency = false;
	//
	nUserBackIdx = nMenuBackIdx = 0;
	crUserBack = crMenuTitleBack = 0;
	nPanelTabsBackIdx = nPanelTabsForeIdx = 0; bPanelTabsSeparate = TRUE;
	bShowKeyBar = true; nBottomLines = 2;
	bAlwaysShowMenuBar = false; nTopLines = 0;
}

CRgnDetect::~CRgnDetect()
{
	if (mpsz_Chars)
	{
		free(mpsz_Chars); mpsz_Chars = NULL;
	}

	if (mp_Attrs)
	{
		free(mp_Attrs); mp_Attrs = NULL;
	}

	if (mp_AttrsWork)
	{
		free(mp_AttrsWork); mp_AttrsWork = NULL;
	}
}

//#ifdef _DEBUG
const DetectedDialogs* CRgnDetect::GetDetectedDialogsPtr() const
{
	return &m_DetectedDialogs;
}
//#endif

int CRgnDetect::GetDetectedDialogs(int anMaxCount, SMALL_RECT* rc, DWORD* rf, DWORD anMask/*=-1*/, DWORD anTest/*=-1*/) const
{
	if (!this) return 0;

	int nCount = m_DetectedDialogs.Count;

	if (nCount > 0 && anMaxCount > 0 && (rc || rf))
	{
		_ASSERTE(sizeof(*rc) == sizeof(m_DetectedDialogs.Rects[0]));
		_ASSERTE(sizeof(*rf) == sizeof(m_DetectedDialogs.DlgFlags[0]));

		if (anMask == (DWORD)-1)
		{
			if (nCount > anMaxCount)
				nCount = anMaxCount;

			if (rc)
				memmove(rc, m_DetectedDialogs.Rects, nCount*sizeof(*rc));

			if (rf)
				memmove(rf, m_DetectedDialogs.DlgFlags, nCount*sizeof(*rf));
		}
		else
		{
			nCount = 0; DWORD nF;

			for(int i = 0; i < m_DetectedDialogs.Count && nCount < anMaxCount; i++)
			{
				nF = m_DetectedDialogs.DlgFlags[i];

				if ((nF & anMask) == anTest)
				{
					if (rc)
						rc[nCount] = m_DetectedDialogs.Rects[i];

					if (rf)
						rf[nCount] = m_DetectedDialogs.DlgFlags[i];

					nCount++;
				}
			}
		}
	}

	return nCount;
}

DWORD CRgnDetect::GetDialog(DWORD nDlgID, SMALL_RECT* rc) const
{
	if (!nDlgID)
	{
		_ASSERTE(nDlgID!=0);
		return 0;
	}

	//DWORD nMask = nDlgID;
	//if ((nDlgID & FR_FREEDLG_MASK) != 0) {
	//	nMask |= FR_FREEDLG_MASK;
	//}

	for(int i = 0; i < m_DetectedDialogs.Count; i++)
	{
		if ((m_DetectedDialogs.DlgFlags[i] & FR_ALLDLG_MASK) == nDlgID)
		{
			if (rc) *rc = m_DetectedDialogs.Rects[i];

			_ASSERTE(m_DetectedDialogs.DlgFlags[i] != 0);
			return m_DetectedDialogs.DlgFlags[i];
		}
	}

	// ������ ����
	return 0;
}

DWORD CRgnDetect::GetFlags() const
{
	// ����� �� ������� � ������� ������ �� ����� ������� ���������� ����������� mn_AllFlagsSaved
	// � �� �� ��� ����� � m_DetectedDialogs.AllFlags (��� ����� ������ ��������)
	//return m_DetectedDialogs.AllFlags;
	return mn_AllFlagsSaved;
}







/* ****************************************** */
/* ����� �������� � ������� "����������" ���� */
/* ****************************************** */


// ����� ������ �������
bool CRgnDetect::FindFrameRight_ByTop(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostRight)
{
	wchar_t wcMostRight = 0;
	int n;
	int nShift = nWidth*nFromY;
	wchar_t wc = pChar[nShift+nFromX];
	nMostRight = nFromX;

	if (wc != ucBoxSinglDownRight && wc != ucBoxDblDownRight)
	{
		// ������������� �������� - �������� ������ �� ������ ��������
		int nMostTop = nFromY;

		if (FindFrameTop_ByLeft(pChar, pAttr, nWidth, nHeight, nFromX, nFromY, nMostTop))
			nFromY = nMostTop; // ���� ������ ������������ ������
	}

	if (wc != ucBoxSinglDownRight && wc != ucBoxDblDownRight)
	{
		wchar_t c;

		// �������� ����������� ���� ������ �� �������
		if (wc == ucBoxSinglVert || wc == ucBoxSinglVertRight)
		{
			while(++nMostRight < nWidth)
			{
				c = pChar[nShift+nMostRight];

				if (c == ucBoxSinglVert || c == ucBoxSinglVertLeft)
				{
					nMostRight++; break;
				}
			}
		}
		else if (wc == ucBoxDblDownRight)
		{
			while(++nMostRight < nWidth)
			{
				c = pChar[nShift+nMostRight];

				if (c == ucBoxDblVert || c == ucBoxDblVertLeft || c == ucBoxDblVertSinglLeft)
				{
					nMostRight++; break;
				}
			}
		}
	}
	else
	{
		if (wc == ucBoxSinglDownRight)
		{
			wcMostRight = ucBoxSinglDownLeft;
		}
		else if (wc == ucBoxDblDownRight)
		{
			wcMostRight = ucBoxDblDownLeft;
		}

		// ����� ������ �������
		while(++nMostRight < nWidth)
		{
			n = nShift+nMostRight;

			//if (pAttr[n].crBackColor != nBackColor)
			//	break; // ����� ����� ���� �������
			if (pChar[n] == wcMostRight)
			{
				nMostRight++;
				break; // ����������� ������� �����
			}
		}
	}

	nMostRight--;
	_ASSERTE(nMostRight<nWidth);
	return (nMostRight > nFromX);
}

// ����� ����� �������
bool CRgnDetect::FindFrameLeft_ByTop(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostLeft)
{
	wchar_t wcMostLeft;
	int n;
	int nShift = nWidth*nFromY;
	wchar_t wc = pChar[nShift+nFromX];
	nMostLeft = nFromX;

	if (wc != ucBoxSinglDownLeft && wc != ucBoxDblDownLeft)
	{
		// ������������� �������� - �������� ������ �� ������ ��������
		int nMostTop = nFromY;

		if (FindFrameTop_ByRight(pChar, pAttr, nWidth, nHeight, nFromX, nFromY, nMostTop))
			nFromY = nMostTop; // ���� ������ ������������ ������
	}

	if (wc != ucBoxSinglDownLeft && wc != ucBoxDblDownLeft)
	{
		wchar_t c;

		// �������� ����������� ���� ������ �� �������
		if (wc == ucBoxSinglVert || wc == ucBoxSinglVertLeft)
		{
			while(--nMostLeft >= 0)
			{
				c = pChar[nShift+nMostLeft];

				if (c == ucBoxSinglVert || c == ucBoxSinglVertRight)
				{
					nMostLeft--; break;
				}
			}
		}
		else if (wc == ucBoxDblDownRight)
		{
			while(--nMostLeft >= 0)
			{
				c = pChar[nShift+nMostLeft];

				if (c == ucBoxDblVert || c == ucBoxDblVertLeft || c == ucBoxDblVertSinglLeft)
				{
					nMostLeft--; break;
				}
			}
		}
	}
	else
	{
		if (wc == ucBoxSinglDownLeft)
		{
			wcMostLeft = ucBoxSinglDownRight;
		}
		else if (wc == ucBoxDblDownLeft)
		{
			wcMostLeft = ucBoxDblDownRight;
		}
		else
		{
			_ASSERTE(wc == ucBoxSinglDownLeft || wc == ucBoxDblDownLeft);
			return false;
		}

		// ����� ����� �������
		while(--nMostLeft >= 0)
		{
			n = nShift+nMostLeft;

			//if (pAttr[n].crBackColor != nBackColor)
			//	break; // ����� ����� ���� �������
			if (pChar[n] == wcMostLeft)
			{
				nMostLeft--;
				break; // ����������� ������� �����
			}
		}
	}

	nMostLeft++;
	_ASSERTE(nMostLeft>=0);
	return (nMostLeft < nFromX);
}

bool CRgnDetect::FindFrameRight_ByBottom(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostRight)
{
	wchar_t wcMostRight;
	int n;
	int nShift = nWidth*nFromY;
	wchar_t wc = pChar[nShift+nFromX];
	nMostRight = nFromX;

	if (wc == ucBoxSinglUpRight || wc == ucBoxSinglHorz || wc == ucBoxSinglUpHorz || wc == ucBoxDblUpSinglHorz)
	{
		wcMostRight = ucBoxSinglUpLeft;
	}
	else if (wc == ucBoxDblUpRight || wc == ucBoxSinglUpDblHorz || wc == ucBoxDblUpDblHorz || wc == ucBoxDblHorz)
	{
		wcMostRight = ucBoxDblUpLeft;
	}
	else
	{
		return false; // ����� ������ ������ ���� �� �� ������
	}

	// ����� ������ �������
	while(++nMostRight < nWidth)
	{
		n = nShift+nMostRight;

		//if (pAttr[n].crBackColor != nBackColor)
		//	break; // ����� ����� ���� �������
		if (pChar[n] == wcMostRight)
		{
			nMostRight++;
			break; // ����������� ������� �����
		}
	}

	nMostRight--;
	_ASSERTE(nMostRight<nWidth);
	return (nMostRight > nFromX);
}

bool CRgnDetect::FindFrameLeft_ByBottom(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostLeft)
{
	wchar_t wcMostLeft;
	int n;
	int nShift = nWidth*nFromY;
	wchar_t wc = pChar[nShift+nFromX];
	nMostLeft = nFromX;

	if (wc == ucBoxSinglUpLeft || wc == ucBoxSinglHorz || wc == ucBoxSinglUpHorz || wc == ucBoxDblUpSinglHorz)
	{
		wcMostLeft = ucBoxSinglUpRight;
	}
	else if (wc == ucBoxDblUpLeft || wc == ucBoxSinglUpDblHorz || wc == ucBoxDblUpDblHorz || wc == ucBoxDblHorz)
	{
		wcMostLeft = ucBoxDblUpRight;
	}
	else
	{
		return false; // ����� ����� ������ ���� �� �� ������
	}

	// ����� ����� �������
	while(--nMostLeft >= 0)
	{
		n = nShift+nMostLeft;

		//if (pAttr[n].crBackColor != nBackColor)
		//	break; // ����� ����� ���� �������
		if (pChar[n] == wcMostLeft)
		{
			nMostLeft--;
			break; // ����������� ������� �����
		}
	}

	nMostLeft++;
	_ASSERTE(nMostLeft>=0);
	return (nMostLeft < nFromX);
}

// ������ ��� ���������. ��� ������ - ���� �� �����
bool CRgnDetect::FindDialog_TopLeft(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostRight, int &nMostBottom, bool &bMarkBorder)
{
	bMarkBorder = TRUE;
	#ifdef _DEBUG
	int nShift = nWidth*nFromY;
	#endif
	int nMostRightBottom;
	// ����� ������ ������� �� �����
	nMostRight = nFromX;
	FindFrameRight_ByTop(pChar, pAttr, nWidth, nHeight, nFromX, nFromY, nMostRight);
	_ASSERTE(nMostRight<nWidth);
	// ����� ������ �������
	nMostBottom = nFromY;
	FindFrameBottom_ByLeft(pChar, pAttr, nWidth, nHeight, nFromX, nFromY, nMostBottom);
	_ASSERTE(nMostBottom<nHeight);
	// ����� ������ ������� �� ������ �������
	nMostRightBottom = nFromY;

	if (FindFrameBottom_ByRight(pChar, pAttr, nWidth, nHeight, nMostRight, nFromY, nMostRightBottom))
	{
		_ASSERTE(nMostRightBottom<nHeight);

		// ����������� ������� - ���������� ������
		if (nMostRightBottom > nMostBottom)
			nMostBottom = nMostRightBottom;
	}

	return true;
}

// ������ ��� ���������. ��� ������ - ���� �� �����
bool CRgnDetect::FindDialog_TopRight(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostRight, int &nMostBottom, bool &bMarkBorder)
{
	bMarkBorder = TRUE;
	#ifdef _DEBUG
	int nShift = nWidth*nFromY;
	#endif
	int nX;
	nMostRight = nFromX;
	nMostBottom = nFromY;

	// ����� ����� ������� �� �����
	if (FindFrameLeft_ByTop(pChar, pAttr, nWidth, nHeight, nFromX, nFromY, nX))
	{
		_ASSERTE(nX>=0);
		nFromX = nX;
	}

	// ����� ������ �������
	nMostBottom = nFromY;
	FindFrameBottom_ByRight(pChar, pAttr, nWidth, nHeight, nMostRight, nFromY, nMostBottom);
	_ASSERTE(nMostBottom<nHeight);

	// ����� ����� ������� �� ����
	if (FindFrameLeft_ByBottom(pChar, pAttr, nWidth, nHeight, nMostRight, nMostBottom, nX))
	{
		_ASSERTE(nX>=0);

		if (nFromX > nX) nFromX = nX;
	}

	return true;
}

// ��� ����� ���� ������ ����� �������
// ����� ������� �����
// ������ ��� ���������, �� ���������� �� � ����
bool CRgnDetect::FindDialog_Left(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostRight, int &nMostBottom, bool &bMarkBorder)
{
	bMarkBorder = TRUE;
	//nBackColor = pAttr[nFromX+nWidth*nFromY].crBackColor; // �� �� ������ ������ �������� ���� ���� ��� ���������
	wchar_t wcMostRight, wcMostBottom, wcMostRightBottom, wcMostTop, wcNotMostBottom1;
	int nShift = nWidth*nFromY;
	int nMostTop, nY, nX;
	wchar_t wc = pChar[nShift+nFromX];

	if (wc == ucBoxSinglVert || wc == ucBoxSinglVertRight)
	{
		wcMostRight = ucBoxSinglUpLeft; wcMostBottom = ucBoxSinglUpRight; wcMostRightBottom = ucBoxSinglUpLeft; wcMostTop = ucBoxSinglDownLeft;

		// ���������� �� ������������ ����� �� ������
		if (wc == ucBoxSinglVert)
		{
			wcNotMostBottom1 = ucBoxSinglUpHorz; //wcNotMostBottom2 = ucBoxSinglUpDblHorz;
			nMostBottom = nFromY;

			while(++nMostBottom < nHeight)
			{
				wc = pChar[nFromX+nMostBottom*nWidth];

				if (wc == wcNotMostBottom1 || wc == ucBoxDblUpSinglHorz || wc == ucBoxSinglUpDblHorz || wc == ucBoxDblUpDblHorz)
					return false;
			}
		}
	}
	else
	{
		wcMostRight = ucBoxDblUpLeft; wcMostBottom = ucBoxDblUpRight; wcMostRightBottom = ucBoxDblUpLeft; wcMostTop = ucBoxDblDownLeft;
	}

	// ���������� ��������� �����, ����� ���� ���-���� ����?
	if (FindFrameTop_ByLeft(pChar, pAttr, nWidth, nHeight, nFromX, nFromY, nY))
	{
		_ASSERTE(nY >= 0);
		nFromY = nY;
	}

	// ����� ������ �������
	nMostBottom = nFromY;
	FindFrameBottom_ByLeft(pChar, pAttr, nWidth, nHeight, nFromX, nFromY, nMostBottom);
	_ASSERTE(nMostBottom<nHeight);
	// ����� ������ ������� �� �����
	nMostRight = nFromX;

	if (FindFrameRight_ByTop(pChar, pAttr, nWidth, nHeight, nFromX, nFromY, nX))
	{
		_ASSERTE(nX<nWidth);
		nMostRight = nX;
	}

	// ����������� ����� ������ ������� �� ����
	if (FindFrameRight_ByBottom(pChar, pAttr, nWidth, nHeight, nFromX, nMostBottom, nX))
	{
		_ASSERTE(nX<nWidth);

		if (nX > nMostRight) nMostRight = nX;
	}

	_ASSERTE(nMostRight>=0);

	// ���������� ��������� ����� �� ������ �������?
	if (FindFrameTop_ByRight(pChar, pAttr, nWidth, nHeight, nMostRight, nFromY, nMostTop))
	{
		_ASSERTE(nMostTop>=0);
		nFromY = nMostTop;
	}

	return true;
}

bool CRgnDetect::FindFrameTop_ByLeft(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostTop)
{
	wchar_t c;
	// ���������� ��������� ����� ����� ������ ����� �� ����
	int nY = nFromY;

	while(nY > 0)
	{
		c = pChar[(nY-1)*nWidth+nFromX];

		if (c == ucBoxDblDownRight || c == ucBoxSinglDownRight  // ������� � ��������� ���� (����� �������)
		        || c == ucBoxDblVertRight || c == ucBoxDblVertSinglRight || c == ucBoxSinglVertRight
		        || c == ucBoxDblVert || c == ucBoxSinglVert
		  ) // ����������� (������ �������)
		{
			nY--; continue;
		}

		// ������ ������� �����������
		break;
	}

	_ASSERTE(nY >= 0);
	nMostTop = nY;
	return (nMostTop < nFromY);
}

bool CRgnDetect::FindFrameTop_ByRight(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostTop)
{
	wchar_t c;
	// ���������� ��������� ����� ����� ������ ����� �� ����
	int nY = nFromY;

	while(nY > 0)
	{
		c = pChar[(nY-1)*nWidth+nFromX];

		if (c == ucBoxDblDownLeft || c == ucBoxSinglDownLeft  // ������� � ��������� ���� (������ �������)
		        || c == ucBoxDblVertLeft || c == ucBoxDblVertSinglLeft || c == ucBoxSinglVertLeft // ����������� (������ �������)
		        || c == ucBoxDblVert || c == ucBoxSinglVert
		        || c == L'}' // �� ������ ������� ������ ����� ���� �������� ������������ ����� �����
		        || (c >= ucBox25 && c <= ucBox75) || c == ucUpScroll || c == ucDnScroll) // ������ ��������� ����� ���� ������ ������
		{
			nY--; continue;
		}

		// ������ ������� �����������
		break;
	}

	_ASSERTE(nY >= 0);
	nMostTop = nY;
	return (nMostTop < nFromY);
}

bool CRgnDetect::FindFrameBottom_ByRight(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostBottom)
{
	// ���������� ���������� ����� ������ ����� �� ����
	int nY = nFromY;
	int nEnd = nHeight - 1;
	wchar_t c; //, cd = ucBoxDblVert;

	//// � ������ � ������ ������� � ������ ������� ���� ����� ���� ����, � �� ���������� ����
	//// "<=1" �.�. ���� ������ ����������� ����������� ����
	//c = pChar[nY*nWidth+nFromX];
	//if (nFromY <= 1 && nFromX == (nWidth-1)) {
	//	if (isDigit(c)) {
	//		cd = c;
	//	}
	//}
	while(nY < nEnd)
	{
		c = pChar[(nY+1)*nWidth+nFromX];

		if (c == ucBoxDblUpLeft || c == ucBoxSinglUpLeft  // ������� � ��������� ���� (������ ������)
		        //|| c == cd // ������ ���� �� ������ ������
		        || c == ucBoxDblVertLeft || c == ucBoxDblVertSinglLeft || c == ucBoxSinglVertLeft // ����������� (������ �������)
		        || c == ucBoxDblVert || c == ucBoxSinglVert
		        || (c >= ucBox25 && c <= ucBox75) || c == ucUpScroll || c == ucDnScroll) // ������ ��������� ����� ���� ������ ������
		{
			nY++; continue;
		}

		// ������ ������� �����������
		break;
	}

	_ASSERTE(nY < nHeight);
	nMostBottom = nY;
	return (nMostBottom > nFromY);
}

bool CRgnDetect::FindFrameBottom_ByLeft(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostBottom)
{
	// ���������� ���������� ����� ����� ����� �� ����
	int nY = nFromY;
	int nEnd = nHeight - 1;
	wchar_t c;

	while(nY < nEnd)
	{
		c = pChar[(nY+1)*nWidth+nFromX];

		if (c == ucBoxDblUpRight || c == ucBoxSinglUpRight  // ������� � ��������� ���� (����� ������)
		        || c == ucBoxDblVertRight || c == ucBoxDblVertSinglRight || c == ucBoxSinglVertRight
		        || c == ucBoxDblVert || c == ucBoxSinglVert
		  ) // ����������� (������ �������)
		{
			nY++; continue;
		}

		// ������ ������� �����������
		break;
	}

	_ASSERTE(nY < nHeight);
	nMostBottom = nY;
	return (nMostBottom > nFromY);
}

// �� �� ������ ������� �����. ����� ������ �����
bool CRgnDetect::FindDialog_Right(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostRight, int &nMostBottom, bool &bMarkBorder)
{
	bMarkBorder = TRUE;
	int nY = nFromY;
	int nX = nFromX;
	nMostRight = nFromX;
	nMostBottom = nFromY; // ����� ��������

	// ���������� ��������� ����� ����� ������ ����� �� ����
	if (FindFrameTop_ByRight(pChar, pAttr, nWidth, nHeight, nFromX, nFromY, nY))
		nFromY = nY;

	// ���������� ���������� ����� ������ ����� �� ����
	if (FindFrameBottom_ByRight(pChar, pAttr, nWidth, nHeight, nFromX, nMostBottom, nY))
		nMostBottom = nY;

	// ������ ����� ������ ������

	// �� �����
	if (FindFrameLeft_ByTop(pChar, pAttr, nWidth, nHeight, nFromX, nFromY, nX))
		nFromX = nX;

	// �� ����
	if (FindFrameLeft_ByBottom(pChar, pAttr, nWidth, nHeight, nFromX, nMostBottom, nX))
		if (nX < nFromX) nFromX = nX;

	_ASSERTE(nFromX>=0 && nFromY>=0);
	return true;
}

// ������ ����� ���� ��� �����, ��� � ������ �� ������������ �����
bool CRgnDetect::FindDialog_Any(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostRight, int &nMostBottom, bool &bMarkBorder)
{
	wchar_t c;
	wchar_t wc = pChar[nFromY*nWidth+nFromX];

	// ��� ��� ������� ����� ��������� (��� ����������) �� ����� �� ����
	for(int ii = 0; ii <= 1; ii++)
	{
		int nY = nFromY;
		int nYEnd = (!ii) ? -1 : nHeight;
		int nYStep = (!ii) ? -1 : 1;
		wchar_t wcCorn1 = (!ii) ? ucBoxSinglDownLeft : ucBoxSinglUpLeft;
		wchar_t wcCorn2 = (!ii) ? ucBoxDblDownLeft : ucBoxDblUpLeft;
		wchar_t wcCorn3 = (!ii) ? ucBoxDblDownRight : ucBoxDblUpRight;
		wchar_t wcCorn4 = (!ii) ? ucBoxSinglDownRight : ucBoxSinglUpRight;

		// TODO: ���� ����� - ��������� ����� ���� �� ���� (�������/���������)

		// �������
		while(nY != nYEnd)
		{
			c = pChar[nY*nWidth+nFromX];

			if (c == wcCorn1 || c == wcCorn2  // ������� � ��������� ���� (������ �������/������)
			        || c == ucBoxDblVertLeft || c == ucBoxDblVertSinglLeft || c == ucBoxSinglVertLeft // ����������� (������ �������)
			        || c == L'}' // �� ������ ������� ������ ����� ���� �������� ������������ ����� �����
			        || (c >= ucBox25 && c <= ucBox75) || c == ucUpScroll || c == ucDnScroll) // ������ ��������� ����� ���� ������ ������
			{
				if (FindDialog_Right(pChar, pAttr, nWidth, nHeight, nFromX, nY, nMostRight, nMostBottom, bMarkBorder))
				{
					nFromY = nY;
					return true;
				}

				return false; // ��������� ���...
			}

			if (c == wcCorn3 || c == wcCorn4  // ������� � ��������� ���� (����� �������/������)
			        || c == ucBoxDblVertRight || c == ucBoxDblVertSinglRight || c == ucBoxSinglVertRight) // ����������� (������ �������)
			{
				if (FindDialog_Left(pChar, pAttr, nWidth, nHeight, nFromX, nY, nMostRight, nMostBottom, bMarkBorder))
				{
					nFromY = nY;
					return true;
				}

				return false; // ��������� ���...
			}

			if (c != wc)
			{
				// ������ ������� �����������
				break;
			}

			// ���������� (����� ��� ����)
			nY += nYStep;
		}
	}

	return false;
}

bool CRgnDetect::FindDialog_Inner(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY)
{
	// ���������� �� ������������ ����� �� ������
	int nShift = nWidth*nFromY;
	wchar_t wc = pChar[nShift+nFromX];

	if (wc != ucBoxSinglVert)
	{
		_ASSERTE(wc == ucBoxSinglVert);
		return false;
	}

	// ������ ������ ����� ���� ������... ������ �� ��� ���������
	int nY = nFromY;

	while(++nY < nHeight)
	{
		wc = pChar[nFromX+nY*nWidth];

		switch(wc)
		{
				// �� ������� ������������ ����� ����� ����������� '}' (����� ��� ����� � ������� �� �������)
			case ucBoxSinglVert:
			case L'}':
				continue;
				// ���� �� ���������� �� ������� �������� ������ - ������ ��� ����� �������. �������
			case ucBoxSinglUpRight:
			case ucBoxSinglUpLeft:
			case ucBoxSinglVertRight:
			case ucBoxSinglVertLeft:
				return false;
				// �������� ���� ������
			case ucBoxSinglUpHorz:
			case ucBoxDblUpSinglHorz:
			case ucBoxSinglUpDblHorz:
			case ucBoxDblUpDblHorz:
				nY++; // �������� ��� ������ (�������)
				// ����� - �������� ����� � �������� ��� ������ (�� �������)
			default:
				nY--;
				{
					// �������� ��� ����� �� ����� (���������� ���������� �������) ��� ����� �����
					CharAttr* p = pAttr+(nWidth*nY+nFromX);

					while(nY-- >= nFromY)
					{
						//_ASSERTE(p->bDialog);
						_ASSERTE(p >= pAttr);
						#if 0
						p->bDialogVBorder = true;
						#else
						p->Flags |= CharAttr_DialogVBorder;
						#endif
						p -= nWidth;
					}

					// �� ����� ������ �� � ����� ������
					while(nY >= 0)
					{
						wc = pChar[nFromX+nY*nWidth];

						if (wc != ucBoxSinglVert && wc != ucBoxSinglDownHorz 
							&& wc != ucBoxSinglDownDblHorz && wc != ucBoxDblDownDblHorz)
						{
							break;
						}

						//_ASSERTE(p->bDialog);
						_ASSERTE(p >= pAttr);
						#if 0
						p->bDialogVBorder = true;
						#else
						p->Flags |= CharAttr_DialogVBorder;
						#endif
						p -= nWidth;
						nY --;
					}
				}
				return true;
		}
	}

	return false;
}

// ���������� ����� �����?
bool CRgnDetect::FindFrame_TopLeft(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nFrameX, int &nFrameY)
{
	// ���������� ����� �����?
	nFrameX = -1; nFrameY = -1;
	int nShift = nWidth*nFromY;
	int nFindFrom = nShift+nFromX;
	int nMaxAdd = min(5,(nWidth - nFromX - 1));
	wchar_t wc;

	// � ���� �� ������
	for(int n = 1; n <= nMaxAdd; n++)
	{
		wc = pChar[nFindFrom+n];

		if (wc == ucBoxSinglDownRight || wc == ucBoxDblDownRight)
		{
			nFrameX = nFromX+n; nFrameY = nFromY;
			return true;
		}
	}

	if (nFrameY == -1)
	{
		// ������� ����
		nFindFrom = nShift+nWidth+nFromX;

		for(int n = 0; n <= nMaxAdd; n++)
		{
			wc = pChar[nFindFrom+n];

			if (wc == ucBoxSinglDownRight || wc == ucBoxDblDownRight)
			{
				nFrameX = nFromX+n; nFrameY = nFromY+1;
				return true;
			}
		}
	}

	return false;
}


bool CRgnDetect::ExpandDialogFrame(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int nFrameX, int nFrameY, int &nMostRight, int &nMostBottom)
{
	bool bExpanded = false;
	#ifdef _DEBUG
	int nStartRight = nMostRight;
	int nStartBottom = nMostBottom;
	#endif
	// ������ ��������� nMostRight & nMostBottom �� ���������
	int n, n2, nShift = nWidth*nFromY;
	wchar_t wc = pChar[nShift+nFromX];
	DWORD nColor = pAttr[nShift+nFromX].crBackColor;

	if (nFromX == nFrameX && nFromY == nFrameY)
	{
		if (wc != ucBoxDblDownRight && wc != ucBoxSinglDownRight)
			return false;

		//������� ����� ������ ����� � �����
		if (nFromY)    // ������� �����
		{
			n = (nFromY-1)*nWidth+nFromX;
			n2 = (nFromY-1)*nWidth+nMostRight;

			if ((pAttr[n].crBackColor == nColor && (pChar[n] == L' ' || pChar[n] == ucNoBreakSpace))
			        && (pAttr[n2].crBackColor == nColor && (pChar[n2] == L' ' || pChar[n2] == ucNoBreakSpace)))
			{
				nFromY--; bExpanded = true;
			}
		}

		if (nFromX)    // ������� �����
		{

			int nMinMargin = nFromX-3; if (nMinMargin<0) nMinMargin = 0;

			n = nFromY*nWidth+nFromX;
			n2 = nMostBottom*nWidth+nFromX;

			while(nFromX > nMinMargin)
			{
				n--; n2--;

				// ������� � ���� ����� "�������" � ���������� ����� CtrlF5. ���� ������� ���������� �������.
				if ((pAttr[n].crBackColor == nColor && (pChar[n] == L' ' || pChar[n] == ucNoBreakSpace) || pChar[n] == L'\\')
					&& (pAttr[n2].crBackColor == nColor && (pChar[n2] == L' ' || pChar[n2] == ucNoBreakSpace)))
				{
					nFromX--;
				}
				else
				{
					break;
				}
			}

			bExpanded = (nFromX<nFrameX);
		}

		_ASSERTE(nFromX>=0 && nFromY>=0);
	}
	else
	{
		if (wc != ucSpace && wc != ucNoBreakSpace)
			return false;
	}

	if (nMostRight < (nWidth-1))
	{
		int nMaxMargin = 3+(nFrameX - nFromX);

		if ((nMaxMargin+nMostRight) >= nWidth) nMaxMargin = nWidth - nMostRight - 1;

		int nFindFrom = nShift+nWidth+nMostRight+1;
		n = 0;
		wc = pChar[nShift+nFromX];

		while(n < nMaxMargin)
		{
			if (pAttr[nFindFrom].crBackColor != nColor || (pChar[nFindFrom] != L' ' && pChar[nFindFrom] != ucNoBreakSpace))
				break;

			n++; nFindFrom++;
		}

		if (n)
		{
			nMostRight += n;
			bExpanded = true;
		}
	}

	_ASSERTE(nMostRight<nWidth);

	// nMostBottom
	if (nFrameY > nFromY && nMostBottom < (nHeight-1))
	{
		n = (nMostBottom+1)*nWidth+nFrameX;

		if (pAttr[n].crBackColor == nColor && (pChar[n] == L' ' || pChar[n] == ucNoBreakSpace))
		{
			nMostBottom ++; bExpanded = true;
		}
	}

	_ASSERTE(nMostBottom<nHeight);
	return bExpanded;
}

// � ������ - ���� �������� �� ������. ��� ����� ���� ��� ����� �������
// ������ ����� �������� ��� ������, ��� �������� �� ������� ������
bool CRgnDetect::FindByBackground(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostRight, int &nMostBottom, bool &bMarkBorder)
{
	// �������� ���� ������ �� ����� ����
	// ��� ����� ���� ������, ����� �������� ������� ������ ��������,
	// ��� ������ ����� �������, � �������� ����� ������ ����� �����
	DWORD nBackColor = pAttr[nFromX+nWidth*nFromY].crBackColor;
	int n, nMostRightBottom, nShift = nWidth*nFromY;
	// ����� ������ �������
	nMostRight = nFromX;

	while(++nMostRight < nWidth)
	{
		n = nShift+nMostRight;

		if (pAttr[n].crBackColor != nBackColor)
			break; // ����� ����� ���� �������
	}

	nMostRight--;
	_ASSERTE(nMostRight<nWidth);
	//2010-06-27 ���-���� ������. ���� �� ����� "�� ����" �� �� ����� �� ����� ���������������, ����� ����������
#if 0
	wchar_t wc = pChar[nFromY*nWidth+nMostRight];

	if (wc >= ucBoxSinglHorz && wc <= ucBoxDblVertHorz)
	{
		switch(wc)
		{
			case ucBoxSinglDownRight: case ucBoxDblDownRight:
			case ucBoxSinglUpRight: case ucBoxDblUpRight:
			case ucBoxSinglDownLeft: case ucBoxDblDownLeft:
			case ucBoxSinglUpLeft: case ucBoxDblUpLeft:
			case ucBoxDblVert: case ucBoxSinglVert:
			{
				DetectDialog(pChar, pAttr, nWidth, nHeight, nMostRight, nFromY);

				if (pAttr[nShift+nFromX].bDialog)
					return false; // ��� ��� ����������
			}
		}
	}
	else if (nMostRight && ((wc >= ucBox25 && wc <= ucBox75) || wc == ucUpScroll || wc == ucDnScroll))
	{
		int nX = nMostRight;

		if (FindDialog_Right(pChar, pAttr, nWidth, nHeight, nX, nFromY, nMostRight, nMostBottom, bMarkBorder))
		{
			nFromX = nX;
			return false;
		}
	}

#endif
	// ����� ������ �������
	nMostBottom = nFromY;

	while(++nMostBottom < nHeight)
	{
		n = nFromX+nMostBottom*nWidth;

		if (pAttr[n].crBackColor != nBackColor)
			break; // ����� ����� ���� �������
	}

	nMostBottom--;
	_ASSERTE(nMostBottom<nHeight);
	// ����� ������ ������� �� ������ �������
	nMostRightBottom = nFromY;

	while(++nMostRightBottom < nHeight)
	{
		n = nMostRight+nMostRightBottom*nWidth;

		if (pAttr[n].crBackColor != nBackColor)
			break; // ����� ����� ���� �������
	}

	nMostRightBottom--;
	_ASSERTE(nMostRightBottom<nHeight);

	// ����������� ������� - ���������� ������
	if (nMostRightBottom > nMostBottom)
		nMostBottom = nMostRightBottom;

	return true;
}

bool CRgnDetect::DetectDialog(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int nFromX, int nFromY, int *pnMostRight, int *pnMostBottom)
{
	if (nFromX >= nWidth || nFromY >= nHeight)
	{
		_ASSERTE(nFromX<nWidth);
		_ASSERTE(nFromY<nHeight);
		return false;
	}

#ifdef _DEBUG

	if (nFromX == 69 && nFromY == 12)
	{
		nFromX = nFromX;
	}

#endif

	// ������ �� ������������ ����� (���� �� ������)
	if (mn_DetectCallCount >= 3)
	{
		_ASSERTE(mn_DetectCallCount<3);
		return false;
	}

	/* *********************************************** */
	/* ����� ���� ������ 'return' ������������ ������! */
	/* *********************************************** */
	bool lbDlgFound = false;
	mn_DetectCallCount++;
	wchar_t wc; //, wcMostRight, wcMostBottom, wcMostRightBottom, wcMostTop, wcNotMostBottom1, wcNotMostBottom2;
	int nMostRight, nMostBottom; //, nMostRightBottom, nMostTop, nShift, n;
	//DWORD nBackColor;
	bool bMarkBorder = false;
	// ����� ��������� - ������ �������, ������� �������� �������� ������ ��������
	int nShift = nWidth*nFromY;
	wc = pChar[nShift+nFromX];
	WARNING("�������� detect");
	/*
	���� ������-����� ���� ������� �� ����� - �� ����� ���� ������ ������ ��������?
	���������� ����� ������-������ ����?
	*/

	/*
	���� � ������� ������� �������� ������ ����� (���� ��������) �� �������� ���������,
	��� ��� ���� ������� ������. � ���� ����� �� ����� ����������.
	*/

	if (wc >= ucBoxSinglHorz && wc <= ucBoxDblVertHorz)
	{
		switch(wc)
		{
				// ������� ����� ����?
			case ucBoxSinglDownRight: case ucBoxDblDownRight:
			{
				// ������ ��� ���������. ��� ������ - ���� �� �����
				if (!FindDialog_TopLeft(pChar, pAttr, nWidth, nHeight, nFromX, nFromY, nMostRight, nMostBottom, bMarkBorder))
					goto fin;

				goto done;
			}
			// ������ ����� ����?
			case ucBoxSinglUpRight: case ucBoxDblUpRight:
			{
				// ������� ����� ����� ��������� �� ����� �����
				if (!FindDialog_TopLeft(pChar, pAttr, nWidth, nHeight, nFromX, nFromY, nMostRight, nMostBottom, bMarkBorder))
					goto fin;

				goto done;
			}
			// ������� ������ ����?
			case ucBoxSinglDownLeft: case ucBoxDblDownLeft:
			{
				// ������ ��� ���������. ��� ������ - ���� �� �����
				if (!FindDialog_TopRight(pChar, pAttr, nWidth, nHeight, nFromX, nFromY, nMostRight, nMostBottom, bMarkBorder))
					goto fin;

				goto done;
			}
			// ������ ������ ����?
			case ucBoxSinglUpLeft: case ucBoxDblUpLeft:
			{
				// ������� ����� ����� ��������� �� ����� �����
				if (!FindDialog_Right(pChar, pAttr, nWidth, nHeight, nFromX, nFromY, nMostRight, nMostBottom, bMarkBorder))
					goto fin;

				goto done;
			}
			case ucBoxDblVert: case ucBoxSinglVert:
			{
				// ���������� �� ������������ ����� �� ������
				if (wc == ucBoxSinglVert)
				{
					if (FindDialog_Inner(pChar, pAttr, nWidth, nHeight, nFromX, nFromY))
						goto fin;
				}

				// ������ ����� ���� ��� �����, ��� � ������ �� ������������ �����
				if (FindDialog_Any(pChar, pAttr, nWidth, nHeight, nFromX, nFromY, nMostRight, nMostBottom, bMarkBorder))
					goto done;
			}
		}
	}

	if (wc == ucSpace || wc == ucNoBreakSpace)
	{
		// ���������� ����� �����?
		int nFrameX = -1, nFrameY = -1;

		if (FindFrame_TopLeft(pChar, pAttr, nWidth, nHeight, nFromX, nFromY, nFrameX, nFrameY))
		{
			// ���� ���� ����� - ���� ����� �� ����� :)
			DetectDialog(pChar, pAttr, nWidth, nHeight, nFrameX, nFrameY, &nMostRight, &nMostBottom);
			//// ������ ��������� nMostRight & nMostBottom �� ���������
			//ExpandDialogFrame(pChar, pAttr, nWidth, nHeight, nFromX, nFromY, nFrameX, nFrameY, nMostRight, nMostBottom);
			//
			goto done;
		}
	}

	// �������� ���� ������ �� ����� ����
	// ��� ����� ���� ������, ����� �������� ������� ������ ��������,
	// ��� ������ ����� �������, � �������� ����� ������ ����� �����
	// 100627 - �� ������ ���� ��� �� ������/��������. ��� ������� ����� ������ �����
	if ((m_DetectedDialogs.AllFlags & FR_VIEWEREDITOR))
		goto fin;

	if (!FindByBackground(pChar, pAttr, nWidth, nHeight, nFromX, nFromY, nMostRight, nMostBottom, bMarkBorder))
		goto fin; // ������ ��� ��� ��������, ��� ������� ���

done:
#ifdef _DEBUG

	if (nFromX<0 || nFromX>=nWidth || nMostRight<nFromX || nMostRight>=nWidth
	        || nFromY<0 || nFromY>=nHeight || nMostBottom<nFromY || nMostBottom>=nHeight)
	{
		//_ASSERT(FALSE);
		// ��� ����������, ���� ���������� ���������� ������� ��������� ��
		// ���������� ��������� �������� (������� ���� �������, ��������, � �.�.)
		goto fin;
	}

#endif
	// ������ ��������
	MarkDialog(pChar, pAttr, nWidth, nHeight, nFromX, nFromY, nMostRight, nMostBottom, bMarkBorder);
	lbDlgFound = true;

	// ������� �������, ���� �������
	if (pnMostRight) *pnMostRight = nMostRight;

	if (pnMostBottom) *pnMostBottom = nMostBottom;

fin:
	mn_DetectCallCount--;
	_ASSERTE(mn_DetectCallCount>=0);
	return lbDlgFound;
}

int CRgnDetect::MarkDialog(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int nX1, int nY1, int nX2, int nY2, bool bMarkBorder, bool bFindExterior /*= TRUE*/, DWORD nFlags /*= -1*/)
{
	if (nX1<0 || nX1>=nWidth || nX2<nX1 || nX2>=nWidth
	        || nY1<0 || nY1>=nHeight || nY2<nY1 || nY2>=nHeight)
	{
		_ASSERTE(nX1>=0 && nX1<nWidth);  _ASSERTE(nX2>=0 && nX2<nWidth);
		_ASSERTE(nY1>=0 && nY1<nHeight); _ASSERTE(nY2>=0 && nY2<nHeight);
		return -1;
	}

	int nDlgIdx = -1;
	DWORD DlgFlags = bMarkBorder ? FR_HASBORDER : 0;
	int nWidth_1 = nWidth - 1;
	int nHeight_1 = nHeight - 1;
	//RECT r = {nX1,nY1,nX2,nY2};

	if (nFlags != (DWORD)-1)
	{
		DlgFlags |= nFlags;
	}
	else if (!nX1 && !nY1 && !nY2 && nX2 == nWidth_1)
	{
		if ((mp_FarInfo->nFarInterfaceSettings & 0x10/*FIS_ALWAYSSHOWMENUBAR*/) == 0)
		{
			DlgFlags |= FR_ACTIVEMENUBAR;
		}
		else
		{
			DlgFlags |= FR_MENUBAR;
			// ���������� ���������� ���� FR_ACTIVEMENUBAR ���� ����� ���� ������ �����
			BYTE btMenuInactiveFore = (mp_FarInfo->nFarColors[col_HMenuText] & 0xF);
			BYTE btMenuInactiveBack = (mp_FarInfo->nFarColors[col_HMenuText] & 0xF0) >> 4;
			int nShift = nY1 * nWidth + nX1;

			for(int nX = nX1; nX <= nX2; nX++, nShift++)
			{
				if (pAttr[nShift].nForeIdx != btMenuInactiveFore
				        || pAttr[nShift].nBackIdx != btMenuInactiveBack)
				{
					DlgFlags |= FR_ACTIVEMENUBAR;
					break;
				}
			}
		}
	}
	else if (bMarkBorder && bFindExterior && mb_NeedPanelDetect) // ������ ���� ��� ��� ������� �������
		//&& !(m_DetectedDialogs.AllFlags & FR_FULLPANEL) // ���� ��� �� ����� ������������� ������
		//&& ((m_DetectedDialogs.AllFlags & (FR_LEFTPANEL|FR_RIGHTPANEL)) != (FR_LEFTPANEL|FR_RIGHTPANEL)) // � �� ����� ��� ������ ��� ����� ������
		//)
	{
		if ((!nY1 || ((m_DetectedDialogs.AllFlags & FR_MENUBAR) && (nY1 == 1)))  // ������� ��� ������� ������� �������
		        && (nY2 >= (nY1 + 3))) // � ����������� ������ �������
		{
			SMALL_RECT sr = {nX1,nY1,nX2,nY2};
			DWORD nPossible = 0;

			if (!nX1)
			{
				if (nX2 == nWidth_1)
				{
					nPossible |= FR_FULLPANEL;
					//r.Left = nX1; r.Top = nY1; r.Right = nX2; r.Bottom = nY2;
				}
				else if (nX2 < (nWidth-9))
				{
					nPossible |= FR_LEFTPANEL;
					//r.Left = nX1; r.Top = nY1; r.Right = nX2; r.Bottom = nY2;
				}
			}
			else if (nX1 >= 10 && nX2 == nWidth_1)
			{
				nPossible |= FR_RIGHTPANEL;
				//r.Left = nX1; r.Top = nY1; r.Right = nX2; r.Bottom = nY2;
			}

			if (nPossible)
			{
				// ����� ����� ���� �� � ����� ���� ����� �������������� ���� ����� ����� �������!
				// ����� - ������� ��� ��� ������ ��������� ��������� � �� ������
				BYTE btPanelFore = (mp_FarInfo->nFarColors[col_PanelBox] & 0xF);
				BYTE btPanelBack = (mp_FarInfo->nFarColors[col_PanelBox] & 0xF0) >> 4;
				int nShift = 0;

				for(int i = 0; i < 4; i++) //-V112
				{
					switch(i)
					{
						case 0: nShift = nY1 * nWidth + nX1; break;
						case 1: nShift = nY2 * nWidth + nX1; break;
						case 2: nShift = nY1 * nWidth + nX2; break;
						case 3: nShift = nY2 * nWidth + nX2; break;
					}

					// ���� ���� ���� ������ � ������ ������ - �� �������, ��� ��� ���
					if (pAttr[nShift].nForeIdx == btPanelFore && pAttr[nShift].nBackIdx == btPanelBack)
					{
						if ((nPossible & FR_RIGHTPANEL))
							mrc_RightPanel = sr; // ������ ������
						else
							mrc_LeftPanel = sr;  // ����� ��� �������������

						DlgFlags |= nPossible;

						// ����� ��� ������ ��� �����?
						if ((nPossible & FR_FULLPANEL))
							mb_NeedPanelDetect = FALSE;
						else if ((m_DetectedDialogs.AllFlags & (FR_LEFTPANEL|FR_RIGHTPANEL)) == (FR_LEFTPANEL|FR_RIGHTPANEL))
							mb_NeedPanelDetect = FALSE;

						break;
					}
				}
			}
		}
	}

	// ����� ���� ��� QSearch?
	if (!(DlgFlags & FR_COMMONDLG_MASK) && bMarkBorder && bFindExterior)
	{
		// QSearch ���������� ������ �� ������ ����� ������, �� ����������� ���� ������,
		// ����� KeyBar �������� � ������ �������� (nHeight-1) ������
		SMALL_RECT *prc = NULL;

		if ((m_DetectedDialogs.AllFlags & FR_FULLPANEL))
		{
			prc = &mrc_LeftPanel;
		}
		else if ((m_DetectedDialogs.AllFlags & FR_LEFTPANEL) && nX2 < mrc_LeftPanel.Right)
		{
			prc = &mrc_LeftPanel;
		}
		else if ((m_DetectedDialogs.AllFlags & FR_RIGHTPANEL) && nX1 > mrc_RightPanel.Left)
		{
			prc = &mrc_RightPanel;
		}

		// ���������
		if (prc)
		{
			if ((nY1+2) == nY2 && prc->Left < nX1 && prc->Right > nX2
			        && (nY1 == prc->Bottom || (nY1 == (prc->Bottom-1) && prc->Bottom == nHeight_1)))
			{
				DlgFlags |= FR_QSEARCH;
			}
		}

		if (!(DlgFlags & FR_QSEARCH))
		{
			// ������ �������:
			// �========== Unicode CharMap =======[�]=�  .
			if ((nX1+39) == nX2 && nY2 >= (nY1 + 23))
			{
				wchar_t* pchTitle = pChar + (nY1 * nWidth + nX1 + 12);

				if (!wmemcmp(pchTitle, L"Unicode CharMap", 15))
				{
					DlgFlags |= FR_UCHARMAP;
					int nSourceIdx = MarkDialog(pChar, pAttr, nWidth, nHeight, nX1+6, nY1+3, nX2-1, nY2-5, false/*bMarkBorder*/, false/*bFindExterior*/);

					if (nSourceIdx != -1)
					{
						m_DetectedDialogs.DlgFlags[nSourceIdx] |= FR_UCHARMAPGLYPH;
						m_DetectedDialogs.AllFlags |= FR_UCHARMAPGLYPH;
					}
				}
			}
		}
	}

	if (!(DlgFlags & FR_COMMONDLG_MASK))
	{
		mn_NextDlgId++;
		DlgFlags |= mn_NextDlgId<<8;
		// "�����������" ������?
		BYTE btWarnBack = (mp_FarInfo->nFarColors[col_WarnDialogBox] & 0xF0) >> 4;

		if (pAttr[nY1 * nWidth + nX1].nBackIdx == btWarnBack)
		{
			BYTE btNormBack = (mp_FarInfo->nFarColors[col_DialogBox] & 0xF0) >> 4;

			if (btNormBack != btWarnBack)
				DlgFlags |= FR_ERRORCOLOR;
		}
	}

	// ������� ���������� � ����� ������ ���������������, ������������ � �������
	if (m_DetectedDialogs.Count < MAX_DETECTED_DIALOGS)
	{
		nDlgIdx = m_DetectedDialogs.Count++;
		m_DetectedDialogs.Rects[nDlgIdx].Left = nX1;
		m_DetectedDialogs.Rects[nDlgIdx].Top = nY1;
		m_DetectedDialogs.Rects[nDlgIdx].Right = nX2;
		m_DetectedDialogs.Rects[nDlgIdx].Bottom = nY2;
		//m_DetectedDialogs.bWasFrame[nDlgIdx] = bMarkBorder;
		m_DetectedDialogs.DlgFlags[nDlgIdx] = DlgFlags;
	}

#ifdef _DEBUG

	if (nX1 == 57 && nY1 == 0)
	{
		nX2 = nX2;
	}

#endif

	if (bMarkBorder)
	{
		#if 0
		pAttr[nY1 * nWidth + nX1].bDialogCorner = TRUE;
		pAttr[nY1 * nWidth + nX2].bDialogCorner = TRUE;
		pAttr[nY2 * nWidth + nX1].bDialogCorner = TRUE;
		pAttr[nY2 * nWidth + nX2].bDialogCorner = TRUE;
		#else
		pAttr[nY1 * nWidth + nX1].Flags |= CharAttr_DialogCorner;
		pAttr[nY1 * nWidth + nX2].Flags |= CharAttr_DialogCorner;
		pAttr[nY2 * nWidth + nX1].Flags |= CharAttr_DialogCorner;
		pAttr[nY2 * nWidth + nX2].Flags |= CharAttr_DialogCorner;
		#endif
	}

	for (int nY = nY1; nY <= nY2; nY++)
	{
		#if 0
		int nShift = nY * nWidth + nX1;
		#else
		CharAttr* pAttrShift = pAttr + nY * nWidth + nX1;
		#endif

		if (bMarkBorder)
		{
			#if 0
			pAttr[nShift].bDialogVBorder = TRUE;
			pAttr[nShift+nX2-nX1].bDialogVBorder = TRUE;
			#else
			pAttrShift->Flags |= CharAttr_DialogVBorder;
			pAttrShift[nX2-nX1].Flags |= CharAttr_DialogVBorder;
			#endif
		}

		#if 0
		for(int nX = nX1; nX <= nX2; nX++, nShift++)
		#else
		for(int nX = nX2 - nX1 + 1; nX > 0; nX--, pAttrShift++)
		#endif
		{
			/*
			if (nY > 0 && nX >= 58)
			{
				nX = nX;
			}
			*/

			#if 0
			pAttr[nShift].bDialog = TRUE;
			pAttr[nShift].bTransparent = FALSE;
			#else
			pAttrShift->Flags = (pAttrShift->Flags | CharAttr_Dialog) & ~CharAttr_Transparent;
			#endif
		}

		//if (bMarkBorder)
		//	pAttr[nShift].bDialogVBorder = TRUE;
	}

	// ���� ���������� ������ �� ����� - ���������� ���������� ���������
	if (bFindExterior && bMarkBorder)
	{
		int nMostRight = nX2, nMostBottom = nY2;
		int nNewX1 = nX1, nNewY1 = nY1;
		_ASSERTE(nMostRight<nWidth);
		_ASSERTE(nMostBottom<nHeight);

		if (ExpandDialogFrame(pChar, pAttr, nWidth, nHeight, nNewX1, nNewY1, nX1, nY1, nMostRight, nMostBottom))
		{
			_ASSERTE(nNewX1>=0 && nNewY1>=0);
			DlgFlags |= FR_HASEXTENSION;

			if (nDlgIdx >= 0)
				m_DetectedDialogs.DlgFlags[nDlgIdx] = DlgFlags;

			//Optimize: �������� ����� ������ ��������� - ��� ������ ��� �������
			MarkDialog(pChar, pAttr, nWidth, nHeight, nNewX1, nNewY1, nMostRight, nMostBottom, true, false);
			// ��� ����� "��������" ���� ��� ��������
			if (((nMostBottom+1) < nHeight) && ((nNewX1+2) < nWidth))
				MarkDialog(pChar, pAttr, nWidth, nHeight, nNewX1+2, nMostBottom+1, min(nMostRight+2,nWidth-1), nMostBottom+1, true, false);
			// � ������ �� �������
			if (((nMostRight+1) < nWidth) && ((nNewY1+1) < nHeight))
				MarkDialog(pChar, pAttr, nWidth, nHeight, nMostRight+1, nNewY1+1, min(nMostRight+2,nWidth-1), nMostBottom, true, false);
		}
	}

	if ((DlgFlags & (FR_LEFTPANEL|FR_RIGHTPANEL|FR_FULLPANEL)) != 0)
	{
		// ��� ������� ������� PanelTabs
		bool bSeparateTabs = false;
		RECT r = {nX1,nY1,nX2,nY2};
		int nBottom = r.bottom;
		int nLeft = r.left;
		// SeparateTabs ����� ���� �� ������������������
		if (r.left /*&& mp_FarInfo->PanelTabs.SeparateTabs == 0*/
		        && (m_DetectedDialogs.AllFlags & FR_LEFTPANEL))
		{
			if (mrc_LeftPanel.Bottom > r.bottom
			        && nHeight > (nBottomLines+mrc_LeftPanel.Bottom+1))
			{
				nBottom = mrc_LeftPanel.Bottom;
				nLeft = 0;
			}
			else if (nHeight > (nBottomLines+mrc_LeftPanel.Bottom+1))
			{
				nLeft = 0;
			}
		}

		// SeparateTabs ����� ���� �� ������������������
		int nIdxLeft, nIdxRight;
		if (DlgFlags & FR_LEFTPANEL)
		{
			nIdxLeft = nWidth*(r.bottom+1)+r.right-1;
			nIdxRight = nWidth*(r.bottom+1)+(nWidth-2);
		}
		else
		{
			nIdxLeft = nWidth*(r.bottom+1)+r.left-2;
			nIdxRight = nWidth*(r.bottom+1)+r.right-1;
		}
		if ((pChar[nIdxRight-1] == 9616 && pChar[nIdxRight] == L'+' && pChar[nIdxRight+1] == 9616)
			&& !(pChar[nIdxLeft-1] == 9616 && pChar[nIdxLeft] == L'+' && pChar[nIdxLeft+1] == 9616))
			bSeparateTabs = false; // ���� ������ ���� ������ ����� (���� �� ��� ������)
		else
			bSeparateTabs = true;

		if (!bSeparateTabs)
		{
			r.left = nLeft;
			r.bottom = nBottom;
		}

		if (nHeight > (nBottomLines+r.bottom+1))
		{
			int nIdx = nWidth*(r.bottom+1)+r.right-1;

			if (pChar[nIdx-1] == 9616 && pChar[nIdx] == L'+' && pChar[nIdx+1] == 9616
				/* -- ����� ���� �� �������������������
			        && pAttr[nIdx].nBackIdx == nPanelTabsBackIdx
			        && pAttr[nIdx].nForeIdx == nPanelTabsForeIdx
				*/
					)
			{
				MarkDialog(pChar, pAttr, nWidth, nHeight, r.left, r.bottom+1, r.right, r.bottom+1,
				           false, false, FR_PANELTABS|(DlgFlags & (FR_LEFTPANEL|FR_RIGHTPANEL|FR_FULLPANEL)));
			}
			#ifdef _DEBUG
			else
			{
				int nDbg2 = 0;
			}
			#endif
		}
		#ifdef _DEBUG
		else if (!(DlgFlags & FR_PANELTABS))
		{
			int nDbg = 0;
		}
		#endif
	}

	m_DetectedDialogs.AllFlags |= DlgFlags;
	return nDlgIdx;
}

void CRgnDetect::OnWindowSizeChanged()
{
	mb_SBI_Loaded = false; // ������ ������ ���� ����������
}

//lpBuffer
//The data to be written to the console screen buffer. This pointer is treated as the origin of a
//two-dimensional array of CHAR_INFO structures whose size is specified by the dwBufferSize parameter.
//The total size of the array must be less than 64K.
//
//dwBufferSize
//The size of the buffer pointed to by the lpBuffer parameter, in character cells.
//The X member of the COORD structure is the number of columns; the Y member is the number of rows.
//
//dwBufferCoord
//The coordinates of the upper-left cell in the buffer pointed to by the lpBuffer parameter.
//The X member of the COORD structure is the column, and the Y member is the row.
//
//lpWriteRegion
//A pointer to a SMALL_RECT structure. On input, the structure members specify the upper-left and lower-right
//coordinates of the console screen buffer rectangle to write to.
//On output, the structure members specify the actual rectangle that was used.
void CRgnDetect::OnWriteConsoleOutput(const CHAR_INFO *lpBuffer,COORD dwBufferSize,COORD dwBufferCoord,PSMALL_RECT lpWriteRegion, const COLORREF *apColors)
{
	mp_Colors = apColors;

	if (!mb_SBI_Loaded)
	{
		if (!InitializeSBI(apColors))
			return;
	}

	if (!mpsz_Chars || !mp_Attrs || !mp_AttrsWork)
		return; // ����� ��� �� ��� �������

	if ((dwBufferCoord.X >= dwBufferSize.X) || (dwBufferCoord.Y >= dwBufferSize.Y))
	{
		_ASSERTE(dwBufferCoord.X < dwBufferSize.X);
		_ASSERTE(dwBufferCoord.Y < dwBufferSize.Y);
		return; // ������ � ����������
	}

	_ASSERTE(mb_TableCreated);
	SMALL_RECT rcRegion;
	rcRegion.Left = lpWriteRegion->Left - m_sbi.srWindow.Left;
	rcRegion.Right = lpWriteRegion->Right - m_sbi.srWindow.Left;
	rcRegion.Top = lpWriteRegion->Top - m_sbi.srWindow.Top;
	rcRegion.Bottom = lpWriteRegion->Bottom - m_sbi.srWindow.Top;

	// �� ������������ ����������� - ����� ������
	if (rcRegion.Left >= mn_CurWidth || rcRegion.Top >= mn_CurHeight
	        || rcRegion.Left > rcRegion.Right || rcRegion.Top > rcRegion.Bottom
	        || rcRegion.Left < 0 || rcRegion.Top < 0)
	{
		_ASSERTE(rcRegion.Left < mn_CurWidth && rcRegion.Top < mn_CurHeight);
		_ASSERTE(rcRegion.Left <= rcRegion.Right && rcRegion.Top <= rcRegion.Bottom);
		_ASSERTE(rcRegion.Left >= 0 && rcRegion.Top >= 0);
		return;
	}

	// ���������� ������ CHAR_INFO �� ����� � ��������
	int nX1 = max(0,rcRegion.Left);
	int nX2 = min(rcRegion.Right,(mn_CurWidth-1));
	int nY1 = max(0,rcRegion.Top);
	int nY2 = min(rcRegion.Bottom,(mn_CurHeight-1));

	if ((dwBufferSize.X - dwBufferCoord.X - 1) < (nX2 - nX1))
		nX2 = nX1 + (dwBufferSize.X - dwBufferCoord.X - 1);

	if ((dwBufferSize.Y - dwBufferCoord.Y - 1) < (nY2 - nY1))
		nY2 = nY1 + (dwBufferSize.Y - dwBufferCoord.Y - 1);

	const CHAR_INFO *pSrcLine = lpBuffer + dwBufferCoord.Y * dwBufferSize.X;

	for(int Y = nY1; Y <= nY2; Y++)
	{
		const CHAR_INFO *pCharInfo = pSrcLine + dwBufferCoord.X;
		wchar_t  *pChar = mpsz_Chars + Y * mn_CurWidth + rcRegion.Left;
		CharAttr *pAttr = mp_Attrs + Y * mn_CurWidth + rcRegion.Left;

		for(int X = nX1; X <= nX2; X++, pCharInfo++)
		{
			TODO("OPTIMIZE: *(lpAttr++) = lpCur->Attributes;");
			*(pAttr++) = mca_Table[pCharInfo->Attributes & 0xFF];
			TODO("OPTIMIZE: ch = lpCur->Char.UnicodeChar;");
			*(pChar++) = pCharInfo->Char.UnicodeChar;
		}

		pSrcLine += dwBufferSize.X;
	}
}

void CRgnDetect::SetFarRect(SMALL_RECT *prcFarRect)
{
	if (prcFarRect)
	{
		mrc_FarRect = *prcFarRect;

		// ����������� � m_sbi.srWindow
		if (mrc_FarRect.Bottom)
		{
			// ������� ��������� ���������
			if (mrc_FarRect.Bottom == m_sbi.srWindow.Bottom)
			{
				if (mrc_FarRect.Top != m_sbi.srWindow.Top)
					m_sbi.srWindow.Top = mrc_FarRect.Top;
			}
			else if (mrc_FarRect.Top == m_sbi.srWindow.Top)
			{
				if (mrc_FarRect.Bottom != m_sbi.srWindow.Bottom)
					m_sbi.srWindow.Bottom = mrc_FarRect.Bottom;
			}

			// ������ - �����������
			if (mrc_FarRect.Left == m_sbi.srWindow.Left)
			{
				if (mrc_FarRect.Right != m_sbi.srWindow.Right)
					m_sbi.srWindow.Right = mrc_FarRect.Right;
			}
			else if (mrc_FarRect.Right == m_sbi.srWindow.Right)
			{
				if (mrc_FarRect.Left != m_sbi.srWindow.Left)
					m_sbi.srWindow.Left = mrc_FarRect.Left;
			}

			// ����� - ������� ��� ��� ������� �� ����, � ������ ����� ������
		}
	}
	else
	{
		memset(&mrc_FarRect, 0, sizeof(mrc_FarRect));
	}
}

BOOL CRgnDetect::InitializeSBI(const COLORREF *apColors)
{
	//if (mb_SBI_Loaded) - ������. ���� ������� - ������ ����� ��� ����������
	//	return TRUE;
	m_DetectedDialogs.AllFlags = 0;
	mp_Colors = apColors;
	//if (!mb_TableCreated) - ���� ������. ����� ����� ����������
	{
		mb_TableCreated = true;
		// ������������ ������������� ������, �� ��������� �������
		int  nColorIndex = 0;
		CharAttr lca;

		for(int nBack = 0; nBack <= 0xF; nBack++)
		{
			for(int nFore = 0; nFore <= 0xF; nFore++, nColorIndex++)
			{
				memset(&lca, 0, sizeof(lca));
				lca.nForeIdx = nFore;
				lca.nBackIdx = nBack;
				lca.crForeColor = lca.crOrigForeColor = mp_Colors[lca.nForeIdx];
				lca.crBackColor = lca.crOrigBackColor = mp_Colors[lca.nBackIdx];
				mca_Table[nColorIndex] = lca;
			}
		}
	}
	HANDLE hStd = GetStdHandle(STD_OUTPUT_HANDLE);
	mb_SBI_Loaded = GetConsoleScreenBufferInfo(hStd, &m_sbi)!=0;

	if (!mb_SBI_Loaded)
		return FALSE;

	if (mrc_FarRect.Right && mrc_FarRect.Bottom)
	{
		// FAR2 /w - ������ � ������� ����� ������
		TODO("��������, ����� � srWindow ���������� mrc_FarRect?");
	}
	else
	{
		// ����� ��������������� srWindow, �.�. ��� � ����� ������ �������� �� ������� ������ � ������ �� ���� �����.
		if (m_sbi.srWindow.Left)
			m_sbi.srWindow.Left = 0;

		if ((m_sbi.srWindow.Right+1) != m_sbi.dwSize.X)
			m_sbi.srWindow.Right = m_sbi.dwSize.X - 1;

		if (m_sbi.srWindow.Top)
			m_sbi.srWindow.Top = 0;

		if ((m_sbi.srWindow.Bottom+1) != m_sbi.dwSize.Y)
			m_sbi.srWindow.Bottom = m_sbi.dwSize.Y - 1;
	}

	int nTextWidth = TextWidth();
	int nTextHeight = TextHeight();

	// �������� ������, ��� ������������� ���������
	if (!mpsz_Chars || !mp_Attrs || !mp_AttrsWork || (nTextWidth * nTextHeight) > mn_MaxCells)
	{
		if (mpsz_Chars) free(mpsz_Chars);

		mn_MaxCells = (nTextWidth * nTextHeight);
		// ����� ��������� ������������ ��������� ������� - �������������� ������ ASCIIZ. ���� mpsz_Chars ����� � \0 ���������?
		mpsz_Chars = (wchar_t*)calloc(mn_MaxCells+1, sizeof(wchar_t));
		mp_Attrs = (CharAttr*)calloc(mn_MaxCells, sizeof(CharAttr));
		mp_AttrsWork = (CharAttr*)calloc(mn_MaxCells, sizeof(CharAttr));
	}

	mn_CurWidth = nTextWidth;
	mn_CurHeight = nTextHeight;

	if (!mpsz_Chars || !mp_Attrs || !mp_AttrsWork)
	{
		_ASSERTE(mpsz_Chars && mp_Attrs && mp_AttrsWork);
		return FALSE;
	}

	if (!mn_MaxCells)
	{
		_ASSERTE(mn_MaxCells>0);
		return FALSE;
	}

	CHAR_INFO *pCharInfo = (CHAR_INFO*)calloc(mn_MaxCells, sizeof(CHAR_INFO));

	if (!pCharInfo)
	{
		_ASSERTE(pCharInfo);
		return FALSE;
	}

	#ifdef _DEBUG
	CHAR_INFO *pCharInfoEnd = pCharInfo+mn_MaxCells;
	#endif
	BOOL bReadOk = FALSE;
	COORD bufSize, bufCoord;
	SMALL_RECT rgn;

	// ���� ���� ����� ������ 30� - � �������� �� �����
	if ((mn_MaxCells*sizeof(CHAR_INFO)) < 30000)
	{
		bufSize.X = nTextWidth; bufSize.Y = nTextHeight;
		bufCoord.X = 0; bufCoord.Y = 0;
		rgn = m_sbi.srWindow;

		if (ReadConsoleOutput(hStd, pCharInfo, bufSize, bufCoord, &rgn))
			bReadOk = TRUE;
	}

	if (!bReadOk)
	{
		// �������� ������ ���������
		bufSize.X = nTextWidth; bufSize.Y = 1;
		bufCoord.X = 0; bufCoord.Y = 0;
		CONSOLE_SCREEN_BUFFER_INFO sbi = m_sbi;
		_ASSERTE(sbi.srWindow.Right>sbi.srWindow.Left);
		_ASSERTE(sbi.srWindow.Bottom>sbi.srWindow.Top);
		rgn = sbi.srWindow;
		CHAR_INFO* pLine = pCharInfo;
		SMALL_RECT rcFar = mrc_FarRect;
		#ifdef _DEBUG
		int nFarWidth = rcFar.Right - rcFar.Left + 1;
		#endif

		for (SHORT y = sbi.srWindow.Top; y <= sbi.srWindow.Bottom; y++, rgn.Top++, pLine+=nTextWidth)
		{
			rgn.Bottom = rgn.Top;
			rgn.Right = rgn.Left+nTextWidth-1;
			ReadConsoleOutput(hStd, pLine, bufSize, bufCoord, &rgn);
		}
	}

	// ������ ����� ���������� ������ � mpsz_Chars & mp_Attrs
	COORD crNul = {0,0};
	COORD crSize = {nTextWidth,nTextHeight};
	OnWriteConsoleOutput(pCharInfo, crSize, crNul, &m_sbi.srWindow, mp_Colors);
	// ����� CHAR_INFO ������ �� �����
	free(pCharInfo);
	return TRUE;
}

int CRgnDetect::TextWidth()
{
	int nWidth = 0;

	if (mrc_FarRect.Right && mrc_FarRect.Bottom)
	{
		// FAR2 /w - ������ � ������� ����� ������
		nWidth = m_sbi.srWindow.Right - m_sbi.srWindow.Left + 1;
	}
	else
	{
		nWidth = m_sbi.dwSize.X;
	}

	return nWidth;
}

int CRgnDetect::TextHeight()
{
	int nHeight = 0;

	if (mrc_FarRect.Right && mrc_FarRect.Bottom)
	{
		// FAR2 /w - ������ � ������� ����� ������
		nHeight = m_sbi.srWindow.Bottom - m_sbi.srWindow.Top + 1;
	}
	else
	{
		nHeight = m_sbi.dwSize.Y;
	}

	return nHeight;
}

BOOL CRgnDetect::GetCharAttr(int x, int y, wchar_t& rc, CharAttr& ra)
{
	if (!mpsz_Chars || !mp_Attrs)
		return FALSE;

	if (x < 0 || x >= mn_CurWidth || y < 0 || y >= mn_CurHeight)
		return FALSE;

	rc = mpsz_Chars[x + y*mn_CurWidth];
	ra = mp_Attrs[x + y*mn_CurWidth];
	return TRUE;
}


// ��� ������� ���������� �� �������� (ConEmuTh)
void CRgnDetect::PrepareTransparent(const CEFAR_INFO_MAPPING *apFarInfo, const COLORREF *apColors)
{
	if (gbInTransparentAssert)
		return;

	mp_FarInfo = apFarInfo;
	mp_Colors = apColors;
	// ����� ������ � ��������������� �������
	m_DetectedDialogs.AllFlags = 0; mn_NextDlgId = 0; mb_NeedPanelDetect = TRUE;
	memset(&mrc_LeftPanel,0,sizeof(mrc_LeftPanel));
	memset(&mrc_RightPanel,0,sizeof(mrc_RightPanel));

	if (!mb_SBI_Loaded)
	{
		if (!InitializeSBI(apColors))
			return;
	}

	memmove(mp_AttrsWork, mp_Attrs, mn_CurWidth*mn_CurHeight*sizeof(*mp_AttrsWork));
	PrepareTransparent(apFarInfo, apColors, &m_sbi, mpsz_Chars, mp_AttrsWork, mn_CurWidth, mn_CurHeight);
}

// ��� ������� ���������� �� GUI
void CRgnDetect::PrepareTransparent(const CEFAR_INFO_MAPPING *apFarInfo, const COLORREF *apColors, const CONSOLE_SCREEN_BUFFER_INFO *apSbi,
                                    wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight)
{
	_ASSERTE(pAttr!=mp_Attrs);
	mp_FarInfo = apFarInfo;
	mp_Colors = apColors;
	_ASSERTE(mp_Colors && (mp_Colors[1] || mp_Colors[2]));
	_ASSERTE(pChar[nWidth*nHeight] == 0); // ������ ���� ASCIIZ

	if (apSbi != &m_sbi)
		m_sbi = *apSbi;

	mb_BufferHeight = !mrc_FarRect.Bottom && (m_sbi.dwSize.Y > (m_sbi.srWindow.Bottom - m_sbi.srWindow.Top + 10));

	if (gbInTransparentAssert)
		return;

	// ����� ������ � ��������������� �������
	m_DetectedDialogs.AllFlags = 0; mn_NextDlgId = 0; mb_NeedPanelDetect = TRUE;
	memset(&mrc_LeftPanel,0,sizeof(mrc_LeftPanel));
	memset(&mrc_RightPanel,0,sizeof(mrc_RightPanel));
	// !!! ����� "m_DetectedDialogs.AllFlags = 0" - ������� return !!!
	m_DetectedDialogs.Count = 0;
	//if (!mp_ConsoleInfo || !gSet.NeedDialogDetect())
	//	goto wrap;
	// !!! � �������� ������ - ������� ������������, �� ������� - ��������, ���������� ��� ���������
	// � ����������-�������� ���� ����� ������
	//if (!mp_FarInfo->bFarPanelAllowed)
	//	goto wrap;
	//if (nCurFarPID && pRCon->mn_LastFarReadIdx != pRCon->mp_ConsoleInfo->nFarReadIdx) {
	//if (isPressed(VK_CONTROL) && isPressed(VK_SHIFT) && isPressed(VK_MENU))
	//	goto wrap;
	WARNING("���� ���������� � FarInfo �� ��������� - ����� ���������!");
	WARNING("��������� ����������� ������� ������� ����, ������� ������ 'R', � �� ��������, �/� ������");
	//COLORREF crColorKey = gSet.ColorKey;
	// �������� ����, �������� � ����
	nUserBackIdx = (mp_FarInfo->nFarColors[col_CommandLineUserScreen] & 0xF0) >> 4;
	crUserBack = mp_Colors[nUserBackIdx];
	nMenuBackIdx = (mp_FarInfo->nFarColors[col_HMenuText] & 0xF0) >> 4;
	crMenuTitleBack = mp_Colors[nMenuBackIdx];
	// COL_PANELBOX
	int nPanelBox = (mp_FarInfo->nFarColors[col_PanelBox] & 0xF0) >> 4;
	crPanelsBorderBack = mp_Colors[nPanelBox];
	nPanelBox = (mp_FarInfo->nFarColors[col_PanelBox] & 0xF);
	crPanelsBorderFore = mp_Colors[nPanelBox];
	// COL_PANELSCREENSNUMBER
	int nPanelNum = (mp_FarInfo->nFarColors[col_PanelScreensNumber] & 0xF0) >> 4;
	crPanelsNumberBack = mp_Colors[nPanelNum];
	nPanelNum = (mp_FarInfo->nFarColors[col_PanelScreensNumber] & 0xF);
	crPanelsNumberFore = mp_Colors[nPanelNum];
	// ����� ��������
	nDlgBorderBackIdx = (mp_FarInfo->nFarColors[col_DialogBox] & 0xF0) >> 4;
	nDlgBorderForeIdx = (mp_FarInfo->nFarColors[col_DialogBox] & 0xF);
	nErrBorderBackIdx = (mp_FarInfo->nFarColors[col_WarnDialogBox] & 0xF0) >> 4;
	nErrBorderForeIdx = (mp_FarInfo->nFarColors[col_WarnDialogBox] & 0xF);
	// ��� ������� ������� PanelTabs
	bPanelTabsSeparate = (mp_FarInfo->PanelTabs.SeparateTabs != 0);

	if (mp_FarInfo->PanelTabs.ButtonColor != -1)
	{
		nPanelTabsBackIdx = (mp_FarInfo->PanelTabs.ButtonColor & 0xF0) >> 4;
		nPanelTabsForeIdx = mp_FarInfo->PanelTabs.ButtonColor & 0xF;
	}
	else
	{
		nPanelTabsBackIdx = (mp_FarInfo->nFarColors[col_PanelText] & 0xF0) >> 4;
		nPanelTabsForeIdx = mp_FarInfo->nFarColors[col_PanelText] & 0xF;
	}

	// ��� bUseColorKey ���� ������ �������� (��� ������) ��
	// 1. UserScreen ��� ��� ���������� �� crColorKey
	// 2. � ����� - �� �������
	// ��������� ������� KeyBar �� ���������� (Keybar + CmdLine)
	bShowKeyBar = (mp_FarInfo->nFarInterfaceSettings & 8/*FIS_SHOWKEYBAR*/) != 0;
	nBottomLines = bShowKeyBar ? 2 : 1;
	// ��������� ������� MenuBar �� ����������
	// ��� ����� ���� ���� ������ ��������?
	// 1 - ��� ������� ������ ��� ��������� ����
	bAlwaysShowMenuBar = (mp_FarInfo->nFarInterfaceSettings & 0x10/*FIS_ALWAYSSHOWMENUBAR*/) != 0;

	if (bAlwaysShowMenuBar)
		m_DetectedDialogs.AllFlags |= FR_MENUBAR; // ������ �����, ����� �������� ������� �� ���������

	nTopLines = bAlwaysShowMenuBar ? 1 : 0;
	// �������� ������ � ������ ����� (�� �������), �� � ����� ������ ���� ����� ������� ��������...
	//// ��������, ��� ��� ����������
	//if (bShowKeyBar) {
	//	// � �����-������ ���� ������ ���� ����� 1
	//	if (pChar[nWidth*(nHeight-1)] != L'1')
	//		goto wrap;
	//	// ���������������� �����
	//	BYTE KeyBarNoColor = mp_FarInfo->nFarColors[col_KeyBarNum];
	//	if (pAttr[nWidth*(nHeight-1)].nBackIdx != ((KeyBarNoColor & 0xF0)>>4))
	//		goto wrap;
	//	if (pAttr[nWidth*(nHeight-1)].nForeIdx != (KeyBarNoColor & 0xF))
	//		goto wrap;
	//}
	// ������ ���������� � ������� ���������� ����������� ��������
	//if (mb_LeftPanel)
	//	MarkDialog(pAttr, nWidth, nHeight, mr_LeftPanelFull.left, mr_LeftPanelFull.top, mr_LeftPanelFull.right, mr_LeftPanelFull.bottom);
	//if (mb_RightPanel)
	//	MarkDialog(pAttr, nWidth, nHeight, mr_RightPanelFull.left, mr_RightPanelFull.top, mr_RightPanelFull.right, mr_RightPanelFull.bottom);
	// �������� ������������� ������
	RECT r;
	bool lbLeftVisible = false, lbRightVisible = false, lbFullPanel = false;

	// ���� ���������� � ������� ���������� ����������� ��������, �� ��� ����� � ���������,
	// �� � ��������� �� ������ ����� ��������� �����������

	//if (mb_LeftPanel) {
	if (mp_FarInfo->bFarLeftPanel)
	{
		lbLeftVisible = true;
		//r = mr_LeftPanelFull;
		r = mp_FarInfo->FarLeftPanel.PanelRect;
	}
	// -- ����� ������� �� ��������������
	_ASSERTE(mp_FarInfo->bFarLeftPanel || !mp_FarInfo->FarLeftPanel.Visible);
	//else
	//{
	//	// �� ���� ����� ������ ������ ��������� - ��� ������ ������ ��� �� ���������
	//	if (mp_FarInfo->bFarLeftPanel && mp_FarInfo->FarLeftPanel.Visible)
	//	{
	//		// � "��������" ������ ������ ������� ������� ������ ������
	//		lbLeftVisible = ConsoleRect2ScreenRect(mp_FarInfo->FarLeftPanel.PanelRect, &r);
	//	}
	//}

	//RECT rLeft = {0}, rRight = {0};

	if (lbLeftVisible)
	{
		if (r.right == (nWidth-1))
			lbFullPanel = true; // ������ ������ ���� �� �����

		if (r.right >= nWidth || r.bottom >= nHeight)
		{
			if (r.right >= nWidth) r.right = nWidth - 1;

			if (r.bottom >= nHeight) r.bottom = nHeight - 1;
		}

		MarkDialog(pChar, pAttr, nWidth, nHeight, r.left, r.top, r.right, r.bottom, true);
		//// ��� ������� ������� PanelTabs
		//if (nHeight > (nBottomLines+r.bottom+1)) {
		//	int nIdx = nWidth*(r.bottom+1)+r.right-1;
		//	if (pChar[nIdx-1] == 9616 && pChar[nIdx] == L'+' && pChar[nIdx+1] == 9616
		//		&& pAttr[nIdx].nBackIdx == nPanelTabsBackIdx
		//		&& pAttr[nIdx].nForeIdx == nPanelTabsForeIdx)
		//	{
		//		MarkDialog(pChar, pAttr, nWidth, nHeight, r.left, r.bottom+1, r.right, r.bottom+1, false, false, FR_PANELTABS|FR_LEFTPANEL);
		//	}
		//}
	}

	if (!lbFullPanel)
	{
		//if (mb_RightPanel) {
		if (mp_FarInfo->bFarRightPanel)
		{
			lbRightVisible = true;
			r = mp_FarInfo->FarRightPanel.PanelRect; // mr_RightPanelFull;
		}
		// -- ����� ������� �� ��������������
		_ASSERTE(mp_FarInfo->bFarRightPanel || !mp_FarInfo->FarRightPanel.Visible);
		//// �� ���� ����� ������ ������ ��������� - ��� ������ ������ ��� �� ���������
		//else if (mp_FarInfo->bFarRightPanel && mp_FarInfo->FarRightPanel.Visible)
		//{
		//	// � "��������" ������ ������ ������� ������� ������ ������
		//	lbRightVisible = ConsoleRect2ScreenRect(mp_FarInfo->FarRightPanel.PanelRect, &r);
		//}

		if (lbRightVisible)
		{
			if (r.right >= nWidth || r.bottom >= nHeight)
			{
				if (r.right >= nWidth) r.right = nWidth - 1;

				if (r.bottom >= nHeight) r.bottom = nHeight - 1;
			}

			MarkDialog(pChar, pAttr, nWidth, nHeight, r.left, r.top, r.right, r.bottom, true);
			//// ��� ������� ������� PanelTabs
			//if (nHeight > (nBottomLines+r.bottom+1)) {
			//	int nIdx = nWidth*(r.bottom+1)+r.right-1;
			//	if (pChar[nIdx-1] == 9616 && pChar[nIdx] == L'+' && pChar[nIdx+1] == 9616
			//		&& pAttr[nIdx].nBackIdx == nPanelTabsBackIdx
			//		&& pAttr[nIdx].nForeIdx == nPanelTabsForeIdx)
			//	{
			//		MarkDialog(pChar, pAttr, nWidth, nHeight, r.left, r.bottom+1, r.right, r.bottom+1, false, false, FR_PANELTABS|FR_RIGHTPANEL);
			//	}
			//}
		}
	}

	mb_NeedPanelDetect = TRUE;

	// ����� ���� ������ ������ - ����? ���������� ��� �������
	if (bAlwaysShowMenuBar  // ������
	        || (pAttr->crBackColor == crMenuTitleBack
	            && (pChar[0] == L' ' && pChar[1] == L' ' && pChar[2] == L' ' && pChar[3] == L' ' && pChar[4] != L' '))
	  )
	{
		MarkDialog(pChar, pAttr, nWidth, nHeight, 0, 0, nWidth-1, 0, false, false, FR_MENUBAR);
	}

	// ��������/������
	if (mp_FarInfo->bViewerOrEditor
	        && 0 == (m_DetectedDialogs.AllFlags & (FR_LEFTPANEL|FR_RIGHTPANEL|FR_FULLPANEL)))
	{
		// ������� ���������, ������� ���� � ������
		MarkDialog(pChar, pAttr, nWidth, nHeight, 0, 0, nWidth-1, nHeight-1, false, false, FR_VIEWEREDITOR);
	}

	if (mn_DetectCallCount != 0)
	{
		_ASSERT(mn_DetectCallCount == 0);
	}

	wchar_t* pszDst = pChar;
	CharAttr* pnDst = pAttr;
	
	const wchar_t szCornerChars[] = {
			ucBoxSinglDownRight,ucBoxSinglDownLeft,ucBoxSinglUpRight,ucBoxSinglUpLeft,
			ucBoxDblDownRight,ucBoxDblDownLeft,ucBoxDblUpRight,ucBoxDblUpLeft,
			0}; // ASCIIZ
	

	for (int nY = 0; nY < nHeight; nY++)
	{
		if (nY >= nTopLines && nY < (nHeight-nBottomLines))
		{
			// ! ������ cell - ����������� ��� �������/������ �������
			int nX1 = 0;
			int nX2 = nWidth-1; // �� ��������� - �� ��� ������

			//if (!mb_LeftPanel && mb_RightPanel) {
			if (!mp_FarInfo->bFarLeftPanel && mp_FarInfo->bFarRightPanel)
			{
				// �������� ������ ����� ������
				nX2 = /*mr_RightPanelFull*/ mp_FarInfo->FarRightPanel.PanelRect.left-1;
				//} else if (mb_LeftPanel && !mb_RightPanel) {
			}
			else if (mp_FarInfo->bFarLeftPanel && !mp_FarInfo->bFarRightPanel)
			{
				// �������� ������ ������ ������
				nX1 = /*mr_LeftPanelFull*/ mp_FarInfo->FarLeftPanel.PanelRect.right+1;
			}
			else
			{
				//��������! ������ ����� ����, �� ��� ����� ���� ��������� PlugMenu!
			}

#ifdef _DEBUG

			if (nY == 16)
			{
				nY = nY;
			}

#endif
			#if 0
			int nShift = nY*nWidth+nX1;
			#endif
			//int nX = nX1;

			if (nY == nTopLines
				&& (m_DetectedDialogs.AllFlags == 0 || m_DetectedDialogs.AllFlags == FR_MENUBAR)
		        && ((*pszDst == L'[' && pnDst->crBackColor == crPanelsNumberBack && pnDst->crForeColor == crPanelsNumberFore)
		        	||	(!nY
						&& ((*pszDst == L'P' && (pnDst->nBackIdx & 7) == 0x2 && pnDst->nForeIdx == 0xF)
							|| (*pszDst == L'R' && (pnDst->nBackIdx & 7) == 0x4 && pnDst->nForeIdx == 0xF))))
		        && (pszDst[nWidth] == ucBoxDblVert && pnDst[nWidth].crBackColor == crPanelsBorderBack && pnDst[nWidth].crForeColor == crPanelsBorderFore)
			  )
			{
				// ������������� ����������, ��� ������
				DetectDialog(pChar, pAttr, nWidth, nHeight, 0/*nX*/, nY+1);
			}

			//wchar_t* pszDstX = pszDst + nX;
			//CharAttr* pnDstX = pnDst + nX;

			wchar_t *pszFrom = pszDst;
			wchar_t *pszEnd = pszDst + nWidth;
			
			while (pszFrom < pszEnd)
			{
				//DWORD DstFlags = pnDst[nX].Flags;
				
				wchar_t cSave = pszDst[nWidth];
				pszDst[nWidth] = 0;
				wchar_t* pszCorner = wcspbrk(pszFrom, szCornerChars);
				// ���� �� ����� - ����� � ������� '\0' ����?
				while (!pszCorner)
				{
					pszFrom += lstrlen(pszFrom)+1;
					if (pszFrom >= (pszDst + nWidth))
					{
						break;
					}
					pszCorner = wcspbrk(pszFrom, szCornerChars);
				}
				pszDst[nWidth] = cSave;
				
				if (!pszCorner)
					break;
				pszFrom = pszCorner + 1; // ����� ��������, ����� �� ������
				int nX = (int)(pszCorner - pszDst);

				if (
						#if 0
						!pnDst[nX].bDialogCorner
						#else
						!(pnDst[nX].Flags/*DstFlags*/ & CharAttr_DialogCorner)
						#endif
					)
				{
					switch (pszDst[nX])
					{
						case ucBoxSinglDownRight:
						case ucBoxSinglDownLeft:
						case ucBoxSinglUpRight:
						case ucBoxSinglUpLeft:
						case ucBoxDblDownRight:
						case ucBoxDblDownLeft:
						case ucBoxDblUpRight:
						case ucBoxDblUpLeft:

							// ��� ������ ����� �������, ������� �� ��������� ���� �� �����
							// �������� "������" �� ��� ����
							// 100627 - �� �������(?) ����� ������������ ������ ����� �������, �� ��������� �������������
							if (!(m_DetectedDialogs.AllFlags & FR_VIEWEREDITOR) ||
							        (pnDst[nX].nBackIdx == nDlgBorderBackIdx && pnDst[nX].nForeIdx == nDlgBorderForeIdx) ||
							        (pnDst[nX].nBackIdx == nErrBorderBackIdx && pnDst[nX].nForeIdx == nErrBorderForeIdx)
							  )
								DetectDialog(pChar, pAttr, nWidth, nHeight, nX, nY);
					}
				}

				//nX++;
				//pszDstX++;
				//pnDstX++;
				#if 0
				nShift++;
				#endif
			}
		}

		pszDst += nWidth;
		pnDst += nWidth;
	}

	if (mn_DetectCallCount != 0)
	{
		_ASSERT(mn_DetectCallCount == 0);
	}

	if (mb_BufferHeight)  // ��� "far /w" mb_BufferHeight==false
		goto wrap; // � �������� ������ - ������� ������������

	if (!lbLeftVisible && !lbRightVisible)
	{
		if (isPressed(VK_CONTROL) && isPressed(VK_SHIFT) && isPressed(VK_MENU))
			goto wrap; // �� CtrlAltShift - �������� UserScreen (�� ������ ��� ����������)
	}

	// 0x0 ������ ���� ������������
	//pAttr[0].bDialog = TRUE;
	pszDst = pChar;
	pnDst = pAttr;

	// ������ ���� ��� ������� �����, �.�. �������� ������������ �����.
	// ��������, ����� ����������� �������� ��� ��� ��������� ���������
	if (mb_NeedTransparency)
	{
		for(int nY = 0; nY < nHeight; nY++)
		{
			if (nY >= nTopLines && nY < (nHeight-nBottomLines))
			{
				// ! ������ cell - ����������� ��� �������/������ �������
				int nX1 = 0;
				int nX2 = nWidth-1; // �� ��������� - �� ��� ������
				// ���-����, ���� ������ ���������� - ������ UserScreen ����������
				// ���� �������� ����������� ���������� ��� �� CtrlAltShift
				//if (!mb_LeftPanel && mb_RightPanel) {
				//	// �������� ������ ����� ������
				//	nX2 = mr_RightPanelFull.left-1;
				//} else if (mb_LeftPanel && !mb_RightPanel) {
				//	// �������� ������ ������ ������
				//	nX1 = mr_LeftPanelFull.right+1;
				//} else {
				//	//��������! ������ ����� ����, �� ��� ����� ���� ��������� PlugMenu!
				//}
				
				WARNING("�� ����� ������� ��������� �������� - ���� �� �������� ������ - ���������� ����������");
				#if 0
				int nShift = nY*nWidth+nX1;
				int nX = nX1;
				#else
				CharAttr* pnDstShift = pnDst + /*nY*nWidth +*/ nX1;
				CharAttr* pnDstEnd = pnDst + nX2 + 1;
				#endif

				#if 0
				while (nX <= nX2)
				#else
				while (pnDstShift < pnDstEnd)
				#endif
				{
					// ���� ��� �� ��������� ��� ���� �������
					#if 0
					if (!pnDst[nX].bDialog)
					#else
					if (!(pnDstShift->Flags & CharAttr_Dialog))
					#endif
					{
						#if 0
						if (pnDst[nX].crBackColor == crUserBack)
						#else
						if (pnDstShift->crBackColor == crUserBack)
						#endif
						{
							// �������� ����������
							#if 0
							pnDst[nX].bTransparent = TRUE;
							//pnDst[nX].crBackColor = crColorKey;
							#else
							pnDstShift->Flags |= CharAttr_Transparent;
							//pnDstShift->crBackColor = crColorKey;
							#endif
							//pszDst[nX] = L' ';
						}
					}

					#if 0
					nX++; nShift++;
					#else
					pnDstShift++;
					#endif
				}
			}

			pszDst += nWidth;
			pnDst += nWidth;
		}
	}

	// ���������...
	//// 0x0 ������ ���� ������������
	//pAttr[0].bTransparent = FALSE;
wrap:
	mn_AllFlagsSaved = m_DetectedDialogs.AllFlags;
}

// ������������� ���������� ���������� ������� � ���������� ������ ������
// (������� ����� ������� ������� ������ � ��������������� ������� ������)
bool CRgnDetect::ConsoleRect2ScreenRect(const RECT &rcCon, RECT *prcScr)
{
	if (!this) return false;

	*prcScr = rcCon;
	int nTopVisibleLine = m_sbi.srWindow.Top;
	_ASSERTE(m_sbi.srWindow.Left==0); // ��� ������ � ConEmu - ���� ������ ������� � ������ ����
	int nTextWidth = (m_sbi.srWindow.Right - m_sbi.srWindow.Left + 1);
	int nTextHeight = (m_sbi.srWindow.Bottom - m_sbi.srWindow.Top + 1);

	if (mb_BufferHeight && nTopVisibleLine)
	{
		prcScr->top -= nTopVisibleLine;
		prcScr->bottom -= nTopVisibleLine;
	}

	bool lbRectOK = true;

	if (prcScr->left == 0 && prcScr->right >= nTextWidth)
		prcScr->right = nTextWidth - 1;

	if (prcScr->left)
	{
		if (prcScr->left >= nTextWidth)
			return false;

		if (prcScr->right >= nTextWidth)
			prcScr->right = nTextWidth - 1;
	}

	if (prcScr->bottom < 0)
	{
		lbRectOK = false; // ��������� ����� �� ������� �����
	}
	else if (prcScr->top >= nTextHeight)
	{
		lbRectOK = false; // ��������� ����� �� ������� ����
	}
	else
	{
		// ��������������� �� �������� ��������������
		if (prcScr->top < 0)
			prcScr->top = 0;

		if (prcScr->bottom >= nTextHeight)
			prcScr->bottom = nTextHeight - 1;

		lbRectOK = (prcScr->bottom > prcScr->top);
	}

	return lbRectOK;
}

//void CRgnDetect::GetConsoleData(const CHAR_INFO *pCharInfo, const COLORREF *apColors, wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight)
//{
//    DWORD cwDstBufSize = nWidth * nHeight;
//
//    _ASSERT(nWidth != 0 && nHeight != 0);
//    if (nWidth == 0 || nHeight == 0)
//        return;
//
//
//    if (!mb_TableCreated) {
//    	mb_TableCreated = true;
//        // ������������ ������������� ������, �� ��������� �������
//	    int  nColorIndex = 0;
//	    CharAttr lca;
//
//	    for (int nBack = 0; nBack <= 0xF; nBack++) {
//	    	for (int nFore = 0; nFore <= 0xF; nFore++, nColorIndex++) {
//				memset(&lca, 0, sizeof(lca));
//	    		lca.nForeIdx = nFore;
//	    		lca.nBackIdx = nBack;
//		    	lca.crForeColor = lca.crOrigForeColor = mp_Colors[lca.nForeIdx];
//		    	lca.crBackColor = lca.crOrigBackColor = mp_Colors[lca.nBackIdx];
//		    	mca_Table[nColorIndex] = lca;
//			}
//		}
//    }
//
//
//	// ���������� ������ CHAR_INFO �� ����� � ��������
//	for (DWORD n = 0; n < CharCount; n++, pCharInfo++) {
//		TODO("OPTIMIZE: *(lpAttr++) = lpCur->Attributes;");
//		*(pAttr++) = mca_Table[pCharInfo->Attributes & 0xFF];
//
//		TODO("OPTIMIZE: ch = lpCur->Char.UnicodeChar;");
//		*(pChar++) = pCharInfo->Char.UnicodeChar;
//	}
//}




CRgnRects::CRgnRects()
{
	nRectCount = 0;
	nRgnState = NULLREGION;
	nFieldMaxCells = nFieldWidth = nFieldHeight = 0;
	pFieldCells = NULL;
}

CRgnRects::~CRgnRects()
{
	if (pFieldCells) free(pFieldCells);
}

// ����� ����� � NULLREGION
void CRgnRects::Reset()
{
	nRectCount = 0;
	nRgnState = NULLREGION;
}

// �������� ��� �������������� � ���������� rcRect[0]
void CRgnRects::Init(LPRECT prcInit)
{
	// ��������� ��������� �������������
	nRectCount = 1;
	rcRect[0] = *prcInit;
	nRgnState = SIMPLEREGION;
	// ������� ���� ��� ��������
	nFieldWidth = prcInit->right - prcInit->left + 1;
	nFieldHeight = prcInit->bottom - prcInit->top + 1;

	if (nFieldWidth<1 || nFieldHeight<1)
	{
		nRgnState = NULLREGION;
		return;
	}

	if (!nFieldMaxCells || !pFieldCells || nFieldMaxCells < (nFieldWidth*nFieldHeight))
	{
		if (pFieldCells) free(pFieldCells);

		nFieldMaxCells = nFieldWidth*nFieldHeight;
		_ASSERTE(sizeof(*pFieldCells)==1);
		pFieldCells = (bool*)calloc(nFieldMaxCells,1);

		if (!pFieldCells)
		{
			_ASSERTE(pFieldCells!=NULL);
			nRgnState = RGN_ERROR;
			return;
		}
	}
	else
	{
		memset(pFieldCells, 0, nFieldMaxCells);
	}
}

// Combines the parts of rcRect[..] that are not part of prcAddDiff.
int CRgnRects::Diff(LPRECT prcAddDiff)
{
	if (!pFieldCells || nRectCount>=MAX_RGN_RECTS || nRgnState <= NULLREGION)
		return nRgnState; // ������ ��� ������, ������ ������ �� �����

	if (prcAddDiff->left > prcAddDiff->right || prcAddDiff->top > prcAddDiff->bottom)
	{
		_ASSERTE(prcAddDiff->left <= prcAddDiff->right && prcAddDiff->top <= prcAddDiff->bottom);
		return nRgnState; // �� �������� prcAddDiff
	}

	// �������
	int X1 = rcRect[0].left, X2 = rcRect[0].right;

	if (prcAddDiff->left > X2 || prcAddDiff->right < X1)
		return nRgnState; // prcAddDiff �� ������������ � rcRect[0]

	int iX1 = max(prcAddDiff->left,X1);
	int iX2 = min(prcAddDiff->right,X2);

	if (iX2 < iX1)
	{
		_ASSERTE(iX2 >= iX1);
		return nRgnState; // ������ �������?
	}

	int Y1 = rcRect[0].top,  Y2 = rcRect[0].bottom;

	if (prcAddDiff->top > Y2 || prcAddDiff->bottom < Y1)
		return nRgnState; // prcAddDiff �� ������������ � rcRect[0]

	int iY1 = max(prcAddDiff->top,Y1);
	int iY2 = min(prcAddDiff->bottom,Y2);
	// �����, ������� ���� ������������� � ������ ����������
	rcRect[nRectCount++] = *prcAddDiff;
	int Y, iy = iY1 - rcRect[0].top;
	int nSubWidth = iX2 - iX1 + 1;
	_ASSERTE(iy>=0);

	for(Y = iY1; Y <= iY2; Y++, iy++)
	{
		int ix = iX1 - X1;
		_ASSERTE(ix>=0);
		int ii = (iy*nFieldWidth) + ix;
		memset(pFieldCells+ii, 1, nSubWidth);
		//for (int X = iX1; X <= iX2; X++, ii++) {
		//	if (!pFieldCells[ii]) {
		//		pFieldCells[ii] = true;
		//	}
		//}
	}

	// ������ ���������, � �������� �� ��� ���-��?
	Y2 = nFieldWidth * nFieldHeight;
	void* ptrCellLeft = memchr(pFieldCells, 0, Y2);
	//bool lbAreCellLeft = (ptrCellLeft != NULL);
	//for (Y = 0; Y < Y2; Y++) {
	//	if (!pFieldCells[ii]) {
	//		lbAreCellLeft = true;
	//		break;
	//	}
	//}

	if (ptrCellLeft != NULL)
	{
		nRgnState = COMPLEXREGION;
	}
	else
	{
		nRgnState = NULLREGION;
	}

	return nRgnState;
}

int CRgnRects::DiffSmall(SMALL_RECT *prcAddDiff)
{
	if (!prcAddDiff)
		return Diff(NULL);

	RECT rc = {prcAddDiff->Left,prcAddDiff->Top,prcAddDiff->Right,prcAddDiff->Bottom};
	return Diff(&rc);
}

// ����������� �� pRgn, ������� true - ���� ���� �������
bool CRgnRects::LoadFrom(CRgnRects* pRgn)
{
	bool lbChanges = false;

	if (!pRgn)
	{
		// ���� �� ����� ��� �� ������ ������
		lbChanges = (nRgnState >= SIMPLEREGION);
		// �����
		Reset();
	}
	else
	{
		if (nRectCount != pRgn->nRectCount || nRgnState != pRgn->nRgnState)
		{
			lbChanges = true;
		}
		else if (pRgn->nRectCount)
		{
			if (memcmp(rcRect, pRgn->rcRect, pRgn->nRectCount * sizeof(RECT)) != 0)
				lbChanges = true;
		}

		nRectCount = pRgn->nRectCount;
		nRgnState = pRgn->nRgnState;

		if (nRectCount > 0)
		{
			memmove(rcRect, pRgn->rcRect, nRectCount * sizeof(RECT));
		}
	}

	// ��� ���������� ������ ��������������
	if (pFieldCells)
	{
		free(pFieldCells); pFieldCells = NULL;
	}

	return lbChanges;
}

void CRgnDetect::SetNeedTransparency(bool abNeed)
{
	mb_NeedTransparency = abNeed;
}