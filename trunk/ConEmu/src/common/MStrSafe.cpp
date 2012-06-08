
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

#define HIDE_USE_EXCEPTION_INFO
#include <windows.h>
#include "MAssert.h"
#include "MStrSafe.h"

// ������������ � ConEmuC.exe, � ��� ����������� ����
// �������� ������ ��� �������������� �� ������!
#undef malloc
#define malloc "malloc"
#undef calloc
#define calloc "calloc"


LPCWSTR msprintf(LPWSTR lpOut, size_t cchOutMax, LPCWSTR lpFmt, ...)
{
	va_list argptr;
	va_start(argptr, lpFmt);
	
	const wchar_t* pszSrc = lpFmt;
	wchar_t* pszDst = lpOut;
	wchar_t  szValue[16];
	wchar_t* pszValue;

	while (*pszSrc)
	{
		if (*pszSrc == L'%')
		{
			pszSrc++;
			switch (*pszSrc)
			{
			case L'%':
				{
					*(pszDst++) = L'%';
					pszSrc++;
					continue;
				}
			case L'c':
				{
					// GCC: 'wchar_t' is promoted to 'int' when passed through '...'
					wchar_t c = va_arg( argptr, int );
					*(pszDst++) = c;
					pszSrc++;
					continue;
				}
			case L's':
				{
					pszValue = va_arg( argptr, wchar_t* );
					if (pszValue)
					{
						while (*pszValue)
						{
							*(pszDst++) = *(pszValue++);
						}
					}
					pszSrc++;
					continue;
				}
			case L'S':
				{
					char* pszValueA = va_arg( argptr, char* );
					if (pszValueA)
					{
						// �� ��������, ��� �� MultiByteToWideChar �����, ��
						// ��� ����� ������ �� ���� ������ ��� ������� ��������������
						while (*pszValueA)
						{
							*(pszDst++) = (wchar_t)*(pszValueA++);
						}
					}
					pszSrc++;
					continue;
				}
			case L'u':
			case L'i':
				{
					UINT nValue = 0;
					if (*pszSrc == L'i')
					{
						int n = va_arg( argptr, int );
						if (n < 0)
						{
							*(pszDst++) = L'-';
							nValue = -n;
						}
						else
						{
							nValue = n;
						}
					}
					else
					{
						nValue = va_arg( argptr, UINT );
					}
					pszSrc++;
					pszValue = szValue;
					while (nValue)
					{
						WORD n = (WORD)(nValue % 10);
						*(pszValue++) = (wchar_t)(L'0' + n);
						nValue = (nValue - n) / 10;
					}
					if (pszValue == szValue)
						*(pszValue++) = L'0';
					// ������ ���������� � szGuiPipeName
					while (pszValue > szValue)
					{
						*(pszDst++) = *(--pszValue);
					}
					continue;
				}
			case L'0':
			case L'X':
			case L'x':
				{
					int nLen = 0;
					DWORD nValue;
					wchar_t cBase = L'A';
					if (pszSrc[0] == L'0' && pszSrc[1] == L'8' && (pszSrc[2] == L'X' || pszSrc[2] == L'x'))
					{
						if (pszSrc[2] == L'x')
							cBase = L'a';
						memmove(szValue, L"00000000", 16);
						nLen = 8;
						pszSrc += 3;
					}
					else if (pszSrc[0] == L'0' && pszSrc[1] == L'2' && (pszSrc[2] == L'X' || pszSrc[2] == L'x'))
					{
						if (pszSrc[2] == L'x')
							cBase = L'a';
						memmove(szValue, L"00", 4);
						nLen = 2;
						pszSrc += 3;
					}
					else if (pszSrc[0] == L'X' || pszSrc[0] == L'x')
					{
						if (pszSrc[0] == L'x')
							cBase = L'a';
						pszSrc++;
					}
					else
					{
						_ASSERTE(*pszSrc == L'u' || *pszSrc == L's' || *pszSrc == L'c' || *pszSrc == L'i' || *pszSrc == L'X');
						goto wrap;
					}
					
					
					nValue = va_arg( argptr, UINT );
					pszValue = szValue;
					while (nValue)
					{
						WORD n = (WORD)(nValue & 0xF);
						if (n <= 9)
							*(pszValue++) = (wchar_t)(L'0' + n);
						else
							*(pszValue++) = (wchar_t)(cBase + n - 10);
						nValue = nValue >> 4;
					}
					if (!nLen)
					{
						nLen = (int)(pszValue - szValue);
						if (!nLen)
						{
							*(pszValue++) = L'0';
							nLen = 1;
						}
					}
					else
					{
						pszValue = (szValue+nLen);
					}
					// ������ ���������� � szGuiPipeName
					while (pszValue > szValue)
					{
						*(pszDst++) = *(--pszValue);
					}
					continue;
				}
			default:
				_ASSERTE(*pszSrc == L'u' || *pszSrc == L's' || *pszSrc == L'c' || *pszSrc == L'i' || *pszSrc == L'X');
			}
		}
		else
		{
			*(pszDst++) = *(pszSrc++);
		}
	}
wrap:
	*pszDst = 0;
	_ASSERTE(lstrlen(lpOut) < (int)cchOutMax);
	return lpOut;
}

LPCSTR msprintf(LPSTR lpOut, size_t cchOutMax, LPCSTR lpFmt, ...)
{
	va_list argptr;
	va_start(argptr, lpFmt);
	
	const char* pszSrc = lpFmt;
	char* pszDst = lpOut;
	char  szValue[16];
	char* pszValue;

	while (*pszSrc)
	{
		if (*pszSrc == '%')
		{
			pszSrc++;
			switch (*pszSrc)
			{
			case '%':
				{
					*(pszDst++) = '%';
					pszSrc++;
					continue;
				}
			case 'c':
				{
					// GCC: 'wchar_t' is promoted to 'int' when passed through '...'
					char c = va_arg( argptr, int );
					*(pszDst++) = c;
					pszSrc++;
					continue;
				}
			case 's':
				{
					pszValue = va_arg( argptr, char* );
					if (pszValue)
					{
						while (*pszValue)
						{
							*(pszDst++) = *(pszValue++);
						}
					}
					pszSrc++;
					continue;
				}
			case 'S':
				{
					char* pszValueA = va_arg( argptr, char* );
					if (pszValueA)
					{
						// �� ��������, ��� �� MultiByteToWideChar �����, ��
						// ��� ����� ������ �� ���� ������ ��� ������� ��������������
						while (*pszValueA)
						{
							*(pszDst++) = (char)*(pszValueA++);
						}
					}
					pszSrc++;
					continue;
				}
			case 'u':
			case 'i':
				{
					UINT nValue = 0;
					if (*pszSrc == 'i')
					{
						int n = va_arg( argptr, int );
						if (n < 0)
						{
							*(pszDst++) = '-';
							nValue = -n;
						}
						else
						{
							nValue = n;
						}
					}
					else
					{
						nValue = va_arg( argptr, UINT );
					}
					pszSrc++;
					pszValue = szValue;
					while (nValue)
					{
						WORD n = (WORD)(nValue % 10);
						*(pszValue++) = (char)('0' + n);
						nValue = (nValue - n) / 10;
					}
					if (pszValue == szValue)
						*(pszValue++) = '0';
					// ������ ���������� � szGuiPipeName
					while (pszValue > szValue)
					{
						*(pszDst++) = *(--pszValue);
					}
					continue;
				}
			case '0':
			case 'X':
			case 'x':
				{
					int nLen = 0;
					DWORD nValue;
					char cBase = 'A';
					if (pszSrc[0] == '0' && pszSrc[1] == '8' && (pszSrc[2] == 'X' || pszSrc[2] == 'x'))
					{
						if (pszSrc[2] == 'x')
							cBase = 'a';
						memmove(szValue, "00000000", 8);
						nLen = 8;
						pszSrc += 3;
					}
					else if (pszSrc[0] == 'X' || pszSrc[0] == 'x')
					{
						if (pszSrc[0] == 'x')
							cBase = 'a';
						pszSrc++;
					}
					else
					{
						_ASSERTE(*pszSrc == 'u' || *pszSrc == 's' || *pszSrc == 'c' || *pszSrc == 'i' || *pszSrc == 'X');
						goto wrap;
					}
					
					
					nValue = va_arg( argptr, UINT );
					pszValue = szValue;
					while (nValue)
					{
						WORD n = (WORD)(nValue & 0xF);
						if (n <= 9)
							*(pszValue++) = (char)('0' + n);
						else
							*(pszValue++) = (char)(cBase + n - 10);
						nValue = nValue >> 4;
					}
					if (!nLen)
					{
						nLen = (int)(pszValue - szValue);
						if (!nLen)
						{
							*pszValue = '0';
							nLen = 1;
						}
					}
					else
					{
						pszValue = (szValue+nLen);
					}
					// ������ ���������� � szGuiPipeName
					while (pszValue > szValue)
					{
						*(pszDst++) = *(--pszValue);
					}
					continue;
				}
			default:
				_ASSERTE(*pszSrc == 'u' || *pszSrc == 's' || *pszSrc == 'c' || *pszSrc == 'i' || *pszSrc == 'X');
			}
		}
		else
		{
			*(pszDst++) = *(pszSrc++);
		}
	}
wrap:
	*pszDst = 0;
	_ASSERTE(lstrlenA(lpOut) < (int)cchOutMax);
	return lpOut;
}
