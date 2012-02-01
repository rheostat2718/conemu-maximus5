#pragma once

/*
mix.hpp

Mix
*/
/*
Copyright � 1996 Eugene Roshal
Copyright � 2000 Far Group
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
*/

#include "plugin.hpp"

int ToPercent(unsigned long N1,unsigned long N2);
int ToPercent64(unsigned __int64 N1,unsigned __int64 N2);

#ifdef __cplusplus
extern "C"
{
#endif

	typedef int (WINAPI *FRSUSERFUNC)(const PluginPanelItem *FData,const wchar_t *FullName,void *param);
	void WINAPI FarRecursiveSearch(const wchar_t *initdir,const wchar_t *mask,FRSUSERFUNC func,unsigned __int64 flags,void *param);

	size_t WINAPI FarMkTemp(wchar_t *Dest, size_t size, const wchar_t *Prefix);

#ifdef __cplusplus
};
#endif

string& FarMkTempEx(string &strDest, const wchar_t *Prefix=nullptr, BOOL WithTempPath=TRUE, const wchar_t *UserTempPath=nullptr);

void PluginPanelItemToFindDataEx(
    const PluginPanelItem *pSrc,
    FAR_FIND_DATA_EX *pDest
);

void FindDataExToPluginPanelItem(
    const FAR_FIND_DATA_EX *pSrc,
    PluginPanelItem *pDest
);

void FreePluginPanelItem(
    PluginPanelItem *pData
);

WINDOWINFO_TYPE ModalType2WType(const int fType);
