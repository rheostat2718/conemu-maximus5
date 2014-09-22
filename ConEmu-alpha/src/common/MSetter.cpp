﻿
/*
Copyright (c) 2009-2014 Maximus5
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

//#ifdef _DEBUG
//#define USE_LOCK_SECTION
//#endif

#define HIDE_USE_EXCEPTION_INFO
#include <windows.h>
#include "defines.h"
#include "MAssert.h"
#include "MSetter.h"

MSetter::MSetter(BOOL* st)
{
	DEBUGTEST(ZeroStruct(DataPtr));
	type = st_BOOL;
	mp_BOOLVal = st;

	if (mp_BOOLVal) *mp_BOOLVal = TRUE;
}
MSetter::MSetter(bool* st)
{
	DEBUGTEST(ZeroStruct(DataPtr));
	type = st_bool;
	mp_boolVal = st;

	if (mp_boolVal) *mp_boolVal = true;
}
MSetter::MSetter(DWORD* st, DWORD setValue)
{
	DEBUGTEST(ZeroStruct(DataPtr));
	type = st_DWORD; mdw_OldDwordValue = *st; *st = setValue;
	mdw_DwordVal = st;
}
MSetter::~MSetter()
{
	Unlock();
}
void MSetter::Unlock()
{
	if (type==st_BOOL)
	{
		if (mp_BOOLVal) *mp_BOOLVal = FALSE;
	}
	else if (type==st_bool)
	{
		if (mp_boolVal) *mp_boolVal = false;
	}
	else if (type==st_DWORD)
	{
		if (mdw_DwordVal) *mdw_DwordVal = mdw_OldDwordValue;
	}
}
