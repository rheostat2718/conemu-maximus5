
/*
Copyright (c) 2009-2011 Maximus5
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
#include "defines.h"
#include "Memory.h"
#include "MStrSafe.h"

#ifdef _DEBUG
#include "MAssert.h"
#endif

#ifndef _ASSERTE
#define _ASSERTE(x)
#endif

HANDLE ghHeap = NULL;

#ifdef TRACK_MEMORY_ALLOCATIONS
static const char* PointToName(const char* asFileOrPath)
{
	if (!asFileOrPath)
	{
		_ASSERTE(asFileOrPath!=NULL);
		return NULL;
	}

	const char* pszSlash = strrchr(asFileOrPath, '\\');

	if (pszSlash)
		return pszSlash+1;

	return asFileOrPath;
}
#endif

bool HeapInitialize()
{
	if (ghHeap == NULL)
	{
		//#ifdef MVALIDATE_POINTERS
		//	ghHeap = HeapCreate(0, 200000, 0);
		//#else
		ghHeap = HeapCreate(HEAP_GENERATE_EXCEPTIONS, 200000, 0);
		//#endif
	}

	return (ghHeap != NULL);
}

void HeapDeinitialize()
{
	if (ghHeap)
	{
		HeapDestroy(ghHeap);
		ghHeap = NULL;
	}
}

//const char* PointToName(const char* asFileOrPath)
//{
//	if (!asFileOrPath)
//	{
//		_ASSERTE(asFileOrPath!=NULL);
//		return NULL;
//	}
//
//	const char* pszSlash = strrchr(asFileOrPath, L'\\');
//
//	if (pszSlash)
//		return pszSlash+1;
//
//	return asFileOrPath;
//}

void * __cdecl xf_malloc
(
    size_t _Size
#ifdef TRACK_MEMORY_ALLOCATIONS
    , LPCSTR lpszFileName, int nLine
#endif
)
{
	_ASSERTE(ghHeap);
	_ASSERTE(_Size>0);
#ifdef TRACK_MEMORY_ALLOCATIONS
#ifdef FORCE_HEAP_CHECK
	xf_dump_chk();
#endif
	xf_mem_block* p = (xf_mem_block*)HeapAlloc(ghHeap, 0, _Size+sizeof(xf_mem_block)+8);
	p->bBlockUsed = TRUE;
	p->nBlockSize = _Size;
#ifdef CONEMU_MINIMAL
	lstrcpynA(p->sCreatedFrom, PointToName(lpszFileName), sizeof(p->sCreatedFrom)/sizeof(p->sCreatedFrom[0]));
#else
	wsprintfA(p->sCreatedFrom, "%s:%i", PointToName(lpszFileName), nLine);
#endif
#ifdef _DEBUG

	if (_Size > 0) memset(p+1, 0xFD, _Size);

#endif
	memset(((LPBYTE)(p+1))+_Size, 0xCC, 8);
	return p?(p+1):p;
#else
	void* p = HeapAlloc(ghHeap, 0, _Size);
	return p;
#endif
}
void * __cdecl xf_calloc
(
    size_t _Count, size_t _Size
#ifdef TRACK_MEMORY_ALLOCATIONS
    , LPCSTR lpszFileName, int nLine
#endif
)
{
	_ASSERTE(ghHeap);
	_ASSERTE((_Count*_Size)>0);
#ifdef TRACK_MEMORY_ALLOCATIONS
#ifdef FORCE_HEAP_CHECK
	xf_dump_chk();
#endif
	xf_mem_block* p = (xf_mem_block*)HeapAlloc(ghHeap, HEAP_ZERO_MEMORY, _Count*_Size+sizeof(xf_mem_block)+8);
	p->bBlockUsed = TRUE;
	p->nBlockSize = _Count*_Size;
#ifdef CONEMU_MINIMAL
	lstrcpynA(p->sCreatedFrom, PointToName(lpszFileName), sizeof(p->sCreatedFrom)/sizeof(p->sCreatedFrom[0]));
#else
	wsprintfA(p->sCreatedFrom, "%s:%i", PointToName(lpszFileName), nLine);
#endif
	memset(((LPBYTE)(p+1))+_Count*_Size, 0xCC, 8);
	return p?(p+1):p;
#else
	void* p = HeapAlloc(ghHeap, HEAP_ZERO_MEMORY, _Count*_Size);
	return p;
#endif
}
void __cdecl xf_free
(
    void * _Memory
#ifdef TRACK_MEMORY_ALLOCATIONS
    , LPCSTR lpszFileName, int nLine
#endif
)
{
	if (!ghHeap || !_Memory)
	{
		//_ASSERTE(ghHeap && _Memory);
		#ifdef _DEBUG
		_CrtDbgBreak();
		#endif
		return;
	}
#ifdef TRACK_MEMORY_ALLOCATIONS
	xf_mem_block* p = ((xf_mem_block*)_Memory)-1;

	if (p->bBlockUsed == TRUE)
	{
		int nCCcmp = memcmp(((LPBYTE)_Memory)+p->nBlockSize, "\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC", 8);
		_ASSERTE(nCCcmp == 0);
		memset(_Memory, 0xFD, p->nBlockSize);
	}
	else
	{
		_ASSERTE(p->bBlockUsed == TRUE);
	}

	p->bBlockUsed = FALSE;
#ifdef CONEMU_MINIMAL
	lstrcpynA(p->sCreatedFrom, PointToName(lpszFileName), sizeof(p->sCreatedFrom)/sizeof(p->sCreatedFrom[0]));
#else
	wsprintfA(p->sCreatedFrom, "-- %s:%i", PointToName(lpszFileName), nLine);
#endif
	_Memory = (void*)p;
#endif
#ifdef _DEBUG
	__int64 _Size1 = HeapSize(ghHeap, 0, _Memory);
	_ASSERTE(_Size1 > 0);
#endif
	HeapFree(ghHeap, 0, _Memory);
#ifdef FORCE_HEAP_CHECK
	xf_dump_chk();
#endif
	//#ifdef _DEBUG
	//SIZE_T _Size2 = HeapSize(ghHeap, 0, _Memory);
	//if (_Size1 == _Size2) {
	//	_ASSERTE(_Size1 != _Size2);
	//}
	//#endif
}
#ifdef TRACK_MEMORY_ALLOCATIONS
#ifdef FORCE_HEAP_CHECK
void __cdecl xf_dump_chk()
{
#ifndef CONEMU_MINIMAL
	PROCESS_HEAP_ENTRY ent = {NULL};
	HeapLock(ghHeap);
	//HeapCompact(ghHeap,0);
	char sBlockInfo[255];
	PVOID pLast = NULL;

	while(HeapWalk(ghHeap, &ent))
	{
		if (pLast == ent.lpData)
		{
			wsprintfA(sBlockInfo, "!!! HeapWalk cycled at 0x%08X, size=0x%08X\n", (DWORD)ent.lpData, ent.cbData);
			OutputDebugStringA(sBlockInfo);
			_ASSERTE(pLast != ent.lpData);
			break;
		}

		if (((int)ent.cbData) < 0)
		{
			wsprintfA(sBlockInfo, "!!! Invalid memory block size at 0x%08X, size=0x%08X\n", (DWORD)ent.lpData, ent.cbData);
			OutputDebugStringA(sBlockInfo);
			_ASSERTE(((int)ent.cbData) >= 0);
			break;
		}
	}

	HeapUnlock(ghHeap);
#endif
}
#endif
void __cdecl xf_dump()
{
#ifndef CONEMU_MINIMAL
	PROCESS_HEAP_ENTRY ent = {NULL};
	HeapLock(ghHeap);
	//HeapCompact(ghHeap,0);
	char sBlockInfo[255];
	PVOID pLast = NULL;

	while(HeapWalk(ghHeap, &ent))
	{
		if (pLast == ent.lpData)
		{
			wsprintfA(sBlockInfo, "!!! HeapWalk cycled at 0x%08X, size=0x%08X\n", (DWORD)ent.lpData, ent.cbData);
			OutputDebugStringA(sBlockInfo);
			_ASSERTE(pLast != ent.lpData);
			break;
		}

		if (((int)ent.cbData) < 0)
		{
			wsprintfA(sBlockInfo, "!!! Invalid memory block size at 0x%08X, size=0x%08X\n", (DWORD)ent.lpData, ent.cbData);
			OutputDebugStringA(sBlockInfo);
			_ASSERTE(((int)ent.cbData) >= 0);
			break;
		}

		if (ent.wFlags & PROCESS_HEAP_ENTRY_BUSY)
		{
			xf_mem_block* p = (xf_mem_block*)ent.lpData;

			if (p->bBlockUsed==TRUE && p->nBlockSize==ent.cbData)
			{
				wsprintfA(sBlockInfo, "!!! Lost memory block at 0x" WIN3264TEST("%08X","%08X%08X") ", size %u\n    Allocated from: %s\n", WIN3264WSPRINT(ent.lpData), ent.cbData,
				          p->sCreatedFrom);
			}
			else
			{
				wsprintfA(sBlockInfo, "!!! Lost memory block at 0x" WIN3264TEST("%08X","%08X%08X") ", size %u\n    Allocated from: %s\n", WIN3264WSPRINT(ent.lpData), ent.cbData,
				          "<Header information broken!>");
			}

			pLast = ent.lpData;
			OutputDebugStringA(sBlockInfo);
		}
	}

	HeapUnlock(ghHeap);
#endif
}
#endif
bool __cdecl xf_validate(void * _Memory /*= NULL*/)
{
	_ASSERTE(ghHeap);
#ifdef TRACK_MEMORY_ALLOCATIONS

	if (_Memory)
	{
		xf_mem_block* p = ((xf_mem_block*)_Memory)-1;
		_ASSERTE(p->bBlockUsed == TRUE);
		_Memory = (void*)p;
	}

#endif
	BOOL b = HeapValidate(ghHeap, 0, _Memory);
	return (b!=FALSE);
}
void * __cdecl operator new(size_t _Size)
{
	void * p = xf_malloc(
	               _Size
#ifdef TRACK_MEMORY_ALLOCATIONS
	               ,__FILE__,__LINE__
#endif
	           );
#ifdef MVALIDATE_POINTERS
	_ASSERTE(p != NULL);

	if (p == NULL) InvalidOp();

#endif
	return p;
}
void * __cdecl operator new[](size_t _Size)
{
	void * p = xf_malloc(
	               _Size
#ifdef TRACK_MEMORY_ALLOCATIONS
	               ,__FILE__,__LINE__
#endif
	           );
#ifdef MVALIDATE_POINTERS
	_ASSERTE(p != NULL);

	if (p == NULL) InvalidOp();

#endif
	return p;
}
void __cdecl operator delete(void *p)
{
	xf_free(
	           p
#ifdef TRACK_MEMORY_ALLOCATIONS
	           ,__FILE__,__LINE__
#endif
	       );
}
void __cdecl operator delete[](void *p)
{
	xf_free(
	           p
#ifdef TRACK_MEMORY_ALLOCATIONS
	           ,__FILE__,__LINE__
#endif
	       );
}

char* lstrdup(const char* asText)
{
	int nLen = asText ? lstrlenA(asText) : 0;
	char* psz = (char*)malloc(nLen+1);

	if (nLen)
		StringCchCopyA(psz, nLen+1, asText);
	else
		psz[0] = 0;

	return psz;
}

wchar_t* lstrdup(const wchar_t* asText)
{
	int nLen = asText ? lstrlenW(asText) : 0;
	wchar_t* psz = (wchar_t*)malloc((nLen+1)*2);

	if (nLen)
		StringCchCopyW(psz, nLen+1, asText);
	else
		psz[0] = 0;

	return psz;
}
