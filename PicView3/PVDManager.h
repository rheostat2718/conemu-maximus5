
#pragma once

/**************************************************************************
Copyright (c) 2009 Skakov Pavel
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

#include "PictureView.h"
#include "PVDInterface/PictureViewPlugin.h"
#include <vector>
#include "PictureView_Dlg.h"

#include "PVDModuleBase.h"
#include "PVDModule1.h"
#include "PVDModule2.h"

#include "DecodeParams.h"


#define PVD_IP_SUPPORTEDTYPES (PVD_IP_DECODE|PVD_IP_DISPLAY)

#define MAX_WHOLELOG_LINES 100

class CImage;
class CDecoderHandle;
class CDisplayHandle;
class CDecodeItem;

extern LPCSTR szPVDManager;

struct DecodeQueueItem
{
	// ������ ������� �������� �������: eItemEmpty/eItemFailed/eItemRequest/eItemDecoding/eItemReady
	DecodingStatus Status;
	
	//// ������ ��� ��������� � �������� ���� GetDecodedImage, IsImageReady
	//CImage* pImage;

	//// ����
	//LPCWSTR pFullFileName;

	//// ������ ����� �� ������
	//int      nRawPanelItem;

	// ��������� �������������
	DecodeParams Params;

	// ����������� �����������. � ���������, ��������� ��������� ����� ������ ���� � � �������
	// ��� �� ��������. ��� �������� GFL & GhostScript, GDI+, DjVu?.
	LPBYTE pBuf;
	DWORD  lBuf;
	

	//// �������������� � ������������ � ������ ������� ������
	//CModuleInfo *Display;
	//LPVOID pDraw;

	// Event, ������������ ����� ������� "�����"
	HANDLE hReady;
};

#ifdef _DEBUG
	#define DECODER_QUEUE_SIZE 3 // ����� � �������� ������� ��������� - ������ ������ �� ������� ��������������
#else
	#define DECODER_QUEUE_SIZE 128
#endif


// ����� �����, ����� ������� ���� ��������� � ������������
// ��������� ������ ��������� � ������ CImage, ������� ��� ��������� � ��������� ������ �����
// � static ���������� - ������ ����������� � ������� � �����
class CPVDManager
{
public:
	CPVDManager();
	~CPVDManager();

	static bool bCancelDecoding;
	static std::vector<CModuleInfo*> Plugins, Decoders, Displays;
	static bool CPVDManager::PluginCompare(CModuleInfo* p1, CModuleInfo* p2);
	static CModuleInfo* GetNextDecoder(const CModuleInfo* apDecoder, bool abForward);
	static CModuleInfo* pDefaultDisplay; // ��� ������� � ��������� ������� - ���� "Display module"
	static CModuleInfo* pActiveDisplay;  // � ��� ��, ����� ��� ���� ����� ������ (����� ���� NULL)
	static std::vector<wchar_t*> sWholeLog;
	static bool bWholeLogChanged;
	static void AddLogString(const wchar_t* asModule, const wchar_t* asMessage);

	static void LoadPlugins2();
	static void ScanFolder(const wchar_t* asFrom);
	static CModuleInfo* LoadPlugin(const wchar_t* asFrom, WIN32_FIND_DATAW& fnd, bool abShowErrors);
	static CModuleInfo* FindPlugin(const wchar_t* asFile, bool abShowErrors);
	static void SortPlugins2();
	static void UnloadPlugins2();
	static bool CreateVersion1(CModuleInfo* plug, bool lbForceWriteReg);
	static bool CreateVersion2(CModuleInfo* plug, bool lbForceWriteReg);
	static bool IsSupportedExtension(const wchar_t *pFileName);
	
	static bool DisplayAttach();
	static bool DisplayDetach(BOOL abErase=TRUE);
	static void DisplayExit();
	static void DisplayErase();

	static void DecoderExit();

	void OnTerminate();
	
	//const wchar_t* GetName();

	static BOOL __stdcall DecodeCallback2(void *pDecodeCallbackContext2, UINT32 iStep, UINT32 nSteps,
										  pvdInfoDecodeStep2* pImagePart);


	void OnFirstImageOpened(CImage *apImage);

	
	// ��������� ������� �������������
protected:
	// �� ��������� ������������� ������ ���������, ������� ��� �����������,
	// � ���� �� - ����������� Invalidate, ����� ����� ���������� �����������
	static DWORD CALLBACK DecodingThread(LPVOID pParam);
	BOOL DecodingThreadStep();
	HANDLE mh_Thread, mh_Request, mh_Alive; DWORD mn_ThreadId; DWORD mn_AliveTick;
	bool CheckDecodingThread();
	
	// 128 - ��� ���� �����. � ����� � ������� ����� �������� �������� (6..9):
	// ����� �������� ����������� + 2 ��� ��������� ������
	// ��������� ����������� (������ �����)
	// ���������� ����������� (������ �����)
	// + ���� �������������� ����� �� ������������� �������� - � ������� ����� ������� ��������� �����
	DecodeQueueItem m_DecoderQueue[DECODER_QUEUE_SIZE];
	CRITICAL_SECTION csDecoderQueue;
	
public:
	// �������� ����� ��� ���-�� ��� ����������� ������ �� �� ������ ������� ����.

	// ������� ������ �������� ��������� ������������� ���� ��������
	// �� ������������� � ��������� ��������� �������������
	CImage* GetDecodedImage(DecodeParams *apParams);
	
	// ��������� � ������� �������������
	BOOL RequestDecodedImage(DecodeParams *apParams);
	
	// ���������� ������������� ����������� � (���) ������� ��� �� �������
	// ��� �������� ����� ��� �������� ������ � ���������� (����� ����), �� 
	// ������������ ����� ����� ������ - ����� �������, ����������� � ����������
	// ��� ���������� ������������, � ������������� � ���� �������� ����� ��������
	// �� ��� ��� ������� �������� ���� eRenderActivateOnReady
	//void CancelImageDecoding(CImage* apImage, DecodeParams aParams);
	
	// ����� ������������� ��� ������ PAN �����������. �� ���� � ����
	// ������ ����� ������������� (... ?)
	bool IsImageReady(DecodeParams *apParams);

	// 1) ��������� ������� ������ �� ��������� �������� � �������� �������
	// 2) ���� ����� - ����������� ���� ���������
	bool OnItemReady(CDecodeItem* apItem);
	
protected:
	// ���� ������� ��������� - ����� ������ �� ������� ������� � ���������� �����������
	// ����������� ����� csDecoderQueue. ���������� ������ � m_DecoderQueue.
	int AppendDecoderQueue(DecodeParams *apParams);
	
	// ������� ������� "������" � ������, ��� � apParams ����� ���� "�������������" ������
	CImage* GetImageFromParam(DecodeParams *apParams);
	
	//
	bool ImageOpen(CImage* apImage, DecodeParams *apParams);
	bool Open(CImage* apImage, CDecoderHandle** ppFile, const wchar_t *pFileName, i64 lFileSize, const u8 *pBuffer, uint lBuffer, DecodeParams *apParams);
	bool OpenWith(CImage* apImage, CDecoderHandle** ppFile, CModuleInfo *pDecoder, const wchar_t *pFileName, i64 lFileSize, const u8 *pBuffer, uint lBuffer);
	void CloseData(CDecodeItem* apItem);
	void CloseDecoder(CDecodeItem* apItem);
	void CloseDisplay(CDecodeItem* apItem);
	//bool Decode(CImage *apImage, CDecoderHandle* pFile, CDisplayHandle** ppDraw, bool abResetTick);
	bool DecodePixels(CDecodeItem* apItem, bool abResetTick);
	bool PixelsToDisplay(CDecodeItem* apItem);
	//void ResetProcessed(CImage* apImage);
};

extern CPVDManager g_Manager;
