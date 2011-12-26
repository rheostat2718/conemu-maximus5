
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
#include "header.h"
#include "registry.h"
#include "ConEmu.h"

#ifdef _DEBUG
#define HEAPVAL //HeapValidate(GetProcessHeap(), 0, NULL);
#else
#define HEAPVAL
#endif

#ifndef __GNU__
const CLSID CLSID_DOMDocument30 = {0xf5078f32, 0xc551, 0x11d3, {0x89, 0xb9, 0x00, 0x00, 0xf8, 0x1f, 0xe2, 0x21}};
#endif



SettingsRegistry::SettingsRegistry()
{
	regMy = NULL;
	lstrcpy(Type, L"[reg]");
}
SettingsRegistry::~SettingsRegistry()
{
	CloseKey();
}



bool SettingsRegistry::OpenKey(HKEY inHKEY, const wchar_t *regPath, uint access, BOOL abSilent /*= FALSE*/)
{
	bool res = false;

	if ((access & KEY_WRITE) == KEY_WRITE)
		res = RegCreateKeyEx(inHKEY, regPath, 0, NULL, 0, access, 0, &regMy, 0) == ERROR_SUCCESS;
	else
		res = RegOpenKeyEx(inHKEY, regPath, 0, access, &regMy) == ERROR_SUCCESS;

	return res;
}
bool SettingsRegistry::OpenKey(const wchar_t *regPath, uint access, BOOL abSilent /*= FALSE*/)
{
	return OpenKey(HKEY_CURRENT_USER, regPath, access);
}
void SettingsRegistry::CloseKey()
{
	if (regMy)
	{
		RegCloseKey(regMy);
		regMy = NULL;
	}
}



bool SettingsRegistry::Load(const wchar_t *regName, LPBYTE value, DWORD nSize)
{
	_ASSERTE(nSize>0);

	DWORD nNewSize = nSize;
	LONG lRc = RegQueryValueEx(regMy, regName, NULL, NULL, (LPBYTE)value, &nNewSize);
	if (lRc == ERROR_SUCCESS)
		return true;
	if (lRc == ERROR_MORE_DATA && nSize == sizeof(BYTE) && nNewSize == sizeof(DWORD))
	{
		// ���� ��� ������ ��� DWORD � ���� - BYTE
		DWORD nData = 0;
		lRc = RegQueryValueEx(regMy, regName, NULL, NULL, (LPBYTE)value, &nNewSize);
		if (lRc == ERROR_SUCCESS)
		{
			*value = (BYTE)(nData & 0xFF);
			return true;
		}
	}
	return false;
}
// ����� ������ ���� ������� - ��� ������ �������� � *value ������ ��������
bool SettingsRegistry::Load(const wchar_t *regName, wchar_t **value)
{
	DWORD len = 0;

	if (RegQueryValueExW(regMy, regName, NULL, NULL, NULL, &len) == ERROR_SUCCESS && len)
	{
		int nChLen = len/2;
		if (*value) {free(*value); *value = NULL;}
		*value = (wchar_t*)malloc((nChLen+2)*sizeof(wchar_t));

		bool lbRc = (RegQueryValueExW(regMy, regName, NULL, NULL, (LPBYTE)(*value), &len) == ERROR_SUCCESS);

		if (!lbRc)
			nChLen = 0;
		(*value)[nChLen] = 0; (*value)[nChLen+1] = 0;

		return lbRc;
	}
	else if (!*value)
	{
		*value = (wchar_t*)malloc(sizeof(wchar_t)*2);
		(*value)[0] = 0; (*value)[1] = 0; // �� ������ REG_MULTI_SZ
	}

	return false;
}
// � ��� �������, ���� �������� ��� (��� ��� ������������) value �� �������
bool SettingsRegistry::Load(const wchar_t *regName, wchar_t *value, int maxLen)
{
	_ASSERTE(maxLen>1);
	DWORD len = 0, dwType = 0;

	if (RegQueryValueExW(regMy, regName, NULL, &dwType, NULL, &len) == ERROR_SUCCESS && dwType == REG_SZ)
	{
		len = maxLen*2; // max size in BYTES

		if (RegQueryValueExW(regMy, regName, NULL, NULL, (LPBYTE)value, &len) == ERROR_SUCCESS)
		{
			value[maxLen-1] = 0; // �� ������ ������, ����� ASCIIZ ��� ����������
			return true;
		}
	}

	return false;
}



void SettingsRegistry::Delete(const wchar_t *regName)
{
	RegDeleteValue(regMy, regName);
}



void SettingsRegistry::Save(const wchar_t *regName, LPCBYTE value, DWORD nType, DWORD nSize)
{
	_ASSERTE(value && nSize);
	RegSetValueEx(regMy, regName, NULL, nType, (LPBYTE)value, nSize);
}
void SettingsRegistry::Save(const wchar_t *regName, const wchar_t *value)
{
	if (!value) value = _T("");  // ���� ��� ������ � NULL

	RegSetValueEx(regMy, regName, NULL, REG_SZ, (LPBYTE)value, (lstrlenW(value)+1) * sizeof(wchar_t));
}









/* *************************** */
#ifndef __GNUC__
SettingsXML::SettingsXML()
{
	mp_File = NULL; mp_Key = NULL;
	lstrcpy(Type, L"[xml]");
	mb_Modified = false;
	mi_Level = 0;
	mb_Empty = false;
	mb_KeyEmpty = false;
}
SettingsXML::~SettingsXML()
{
	CloseKey();
}

bool SettingsXML::IsXmlAllowed()
{
	HRESULT hr;
	IXMLDOMDocument* pFile = NULL;
	hr = CoInitialize(NULL);
	hr = CoCreateInstance(CLSID_DOMDocument30, NULL, CLSCTX_INPROC_SERVER, //-V519
	                      IID_IXMLDOMDocument, (void**)&pFile);

	if (FAILED(hr) || !pFile)
	{
		wchar_t szErr[255];
		_wsprintf(szErr, SKIPLEN(countof(szErr))
		          L"XML setting file will be not used.\n\nCan't create IID_IXMLDOMDocument!\nErrCode=0x%08X", (DWORD)hr);
		MBoxA(szErr);
		return false;
	}

	pFile->Release();
	return true;
}

bool SettingsXML::OpenKey(const wchar_t *regPath, uint access, BOOL abSilent /*= FALSE*/)
{
	bool lbRc = false;
	HRESULT hr = S_OK;
	wchar_t szErr[512]; szErr[0] = 0;
	wchar_t szName[MAX_PATH];
	const wchar_t* psz = NULL;
	VARIANT_BOOL bSuccess;
	IXMLDOMParseError* pErr = NULL;
	//IXMLDOMNodeList* pList = NULL;
	IXMLDOMNode* pKey = NULL;
	IXMLDOMNode* pChild = NULL;
	VARIANT vt; VariantInit(&vt);
	bool bAllowCreate = (access & KEY_WRITE) == KEY_WRITE;
	CloseKey(); // �� ������

	if (!regPath || !*regPath)
	{
		return false;
	}

	HANDLE hFile = NULL;
	DWORD dwAccess = GENERIC_READ;

	if ((access & KEY_WRITE) == KEY_WRITE)
		dwAccess |= GENERIC_WRITE;

	LPWSTR pszXmlFile = gpConEmu->ConEmuXml();

	if (!pszXmlFile || !*pszXmlFile)
	{
		return false;
	}

	hFile = CreateFile(pszXmlFile, dwAccess, FILE_SHARE_READ|FILE_SHARE_WRITE,
	                   NULL, OPEN_EXISTING, 0, 0);

	// XML-���� ����������
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	else
	{
		BY_HANDLE_FILE_INFORMATION bfi = {0};
		if (GetFileInformationByHandle(hFile, &bfi))
			mb_Empty = (bfi.nFileSizeHigh == 0 && bfi.nFileSizeLow == 0);
		CloseHandle(hFile); hFile = NULL;
		if (mb_Empty && bAllowCreate)
		{
			hFile = CreateFile(pszXmlFile, GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE,
	                   NULL, OPEN_EXISTING, 0, 0);
			if (hFile != INVALID_HANDLE_VALUE)
			{
				LPCSTR pszDefault = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n<key name=\"Software\">\r\n\t<key name=\"ConEmu\">\r\n\t</key>\r\n</key>\r\n";
				DWORD nLen = lstrlenA(pszDefault);
				WriteFile(hFile, pszDefault, nLen, &nLen, NULL);
				CloseHandle(hFile);
			}
			hFile = NULL;
		}
	}

	if (mb_Empty && !bAllowCreate)
		return false;

	SAFETRY
	{
		hr = CoInitialize(NULL);
		hr = CoCreateInstance(CLSID_DOMDocument30, NULL, CLSCTX_INPROC_SERVER,
		IID_IXMLDOMDocument, (void**)&mp_File);

		if (FAILED(hr) || !mp_File)
		{
			_wsprintf(szErr, SKIPLEN(countof(szErr)) L"Can't create IID_IXMLDOMDocument!\nErrCode=0x%08X", (DWORD)hr);
			goto wrap;
		}

		hr = mp_File->put_preserveWhiteSpace(VARIANT_TRUE);
		hr = mp_File->put_async(VARIANT_FALSE);

		// ��������� xml-��
		bSuccess = VARIANT_FALSE;
		vt.vt = VT_BSTR; vt.bstrVal = ::SysAllocString(pszXmlFile);
		hr = mp_File->load(vt, &bSuccess);
		VariantClear(&vt);

		if (hr == S_FALSE)
		{
			mb_Empty = true; // ���� ���� (����� ������ ���������?)
		}
		else if (FAILED(hr) || !bSuccess)
		{
			_wsprintf(szErr, SKIPLEN(countof(szErr)) L"Failed to load ConEmu.xml!\nHR=0x%08X\n", (DWORD)hr);
			hr = mp_File->get_parseError(&pErr);

			if (pErr)
			{
				long errorCode = 0; // Contains the error code of the last parse error. Read-only.
				long line = 0; // Specifies the line number that contains the error. Read-only.
				long linepos  = 0; // Contains the character position within the line where the error occurred. Read-only.
				hr = pErr->get_errorCode(&errorCode);
				hr = pErr->get_line(&line);
				hr = pErr->get_linepos(&linepos);
				wsprintf(szErr+_tcslen(szErr), L"XmlErrCode=%i, Line=%i, Pos=%i", errorCode, line, linepos);
			}

			goto wrap;
		}

		hr = mp_File->QueryInterface(IID_IXMLDOMNode, (void **)&pKey);

		if (FAILED(hr))
		{
			_wsprintf(szErr, SKIPLEN(countof(szErr)) L"XML: Root node not found!\nErrCode=0x%08X", (DWORD)hr);
			goto wrap;
		}

		mi_Level = 0;

		while(*regPath)
		{
			// �������� ��������� �����
			psz = wcschr(regPath, L'\\');

			if (!psz) psz = regPath + _tcslen(regPath);

			lstrcpyn(szName, regPath, psz-regPath+1);
			// ����� � ��������� XML
			pChild = FindItem(pKey, L"key", szName, bAllowCreate);
			pKey->Release();
			pKey = pChild; pChild = NULL;
			mi_Level++;

			if (!pKey)
			{
				if (bAllowCreate)
				{
					_wsprintf(szErr, SKIPLEN(countof(szErr)) L"XML: Can't create key <%s>!", szName);
				}
				else
				{
					//_wsprintf(szErr, SKIPLEN(countof(szErr)) L"XML: key <%s> not found!", szName);
					szErr[0] = 0; // ������ �� ���������� - ��������� �� ���������
				}

				goto wrap;
			}

			if (*psz == L'\\')
			{
				regPath = psz + 1;
			}
			else
			{
				break;
			}
		}

		// �����, ���������
		mp_Key = pKey; pKey = NULL;

		if (mp_Key)
		{
			SYSTEMTIME st; wchar_t szTime[32];
			GetLocalTime(&st);
			_wsprintf(szTime, SKIPLEN(countof(szTime)) L"%04i-%02i-%02i %02i:%02i:%02i",
				st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
			SetAttr(mp_Key, L"modified", szTime);
		}

		lbRc = true;

	} SAFECATCH
	{
		lstrcpy(szErr, L"Exception in SettingsXML::OpenKey");
		lbRc = false;
	}
wrap:

	if (pErr) { pErr->Release(); pErr = NULL; }

	if (pChild) { pChild->Release(); pChild = NULL; }

	if (pKey) { pKey->Release(); pKey = NULL; }

	if (!lbRc && szErr[0] && !abSilent)
	{
		MBoxA(szErr);
	}

	return lbRc;
}

void SettingsXML::CloseKey()
{
	HRESULT hr = S_OK;
	HANDLE hFile = NULL;
	bool bCanSave = false;
	mi_Level = 0;

	if (mb_Modified && mp_File)
	{
		LPWSTR pszXmlFile = gpConEmu->ConEmuXml();

		if (pszXmlFile)
		{
			hFile = CreateFile(pszXmlFile, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE,
			                   NULL, OPEN_EXISTING, 0, 0);

			// XML-���� ����������, ��� ������ �������
			if (hFile == INVALID_HANDLE_VALUE)
			{
				DWORD dwErrCode = GetLastError();
				wchar_t szErr[MAX_PATH*2];
				_wsprintf(szErr, SKIPLEN(countof(szErr)) L"Can't open file for writing!\n%s\nErrCode=0x%08X",
				          pszXmlFile, dwErrCode);
				MBoxA(szErr);
			}
			else
			{
				CloseHandle(hFile); hFile = NULL;
				bCanSave = true;
			}

			if (bCanSave)
			{
				VARIANT vt; vt.vt = VT_BSTR; vt.bstrVal = ::SysAllocString(pszXmlFile);
				hr = mp_File->save(vt);
				VariantClear(&vt);
			}
		}
	}

	if (mp_Key) { mp_Key->Release(); mp_Key = NULL; }

	if (mp_File) { mp_File->Release(); mp_File = NULL; }

	mb_Modified = false;
	mb_Empty = false;
	mb_KeyEmpty = false;
}

BSTR SettingsXML::GetAttr(IXMLDOMNode* apNode, const wchar_t* asName)
{
	HRESULT hr = S_OK;
	BSTR bsValue = NULL;
	IXMLDOMNamedNodeMap* pAttrs = NULL;
	hr = apNode->get_attributes(&pAttrs);

	if (SUCCEEDED(hr) && pAttrs)
	{
		bsValue = GetAttr(apNode, pAttrs, asName);
		pAttrs->Release(); pAttrs = NULL;
	}

	return bsValue;
}

BSTR SettingsXML::GetAttr(IXMLDOMNode* apNode, IXMLDOMNamedNodeMap* apAttrs, const wchar_t* asName)
{
	HRESULT hr = S_OK;
	IXMLDOMNode *pValue = NULL;
	BSTR bsText = NULL;
	bsText = ::SysAllocString(asName);
	hr = apAttrs->getNamedItem(bsText, &pValue);
	::SysFreeString(bsText); bsText = NULL;

	if (SUCCEEDED(hr) && pValue)
	{
		hr = pValue->get_text(&bsText);
		pValue->Release(); pValue = NULL;
	}

	return bsText;
}

bool SettingsXML::SetAttr(IXMLDOMNode* apNode, const wchar_t* asName, const wchar_t* asValue)
{
	bool lbRc = false;
	HRESULT hr = S_OK;
	IXMLDOMNamedNodeMap* pAttrs = NULL;
	hr = apNode->get_attributes(&pAttrs);

	if (SUCCEEDED(hr) && pAttrs)
	{
		lbRc = SetAttr(apNode, pAttrs, asName, asValue);
		pAttrs->Release(); pAttrs = NULL;
	}

	return lbRc;
}

bool SettingsXML::SetAttr(IXMLDOMNode* apNode, IXMLDOMNamedNodeMap* apAttrs, const wchar_t* asName, const wchar_t* asValue)
{
	bool lbRc = false;
	HRESULT hr = S_OK;
	IXMLDOMNode *pValue = NULL;
	IXMLDOMAttribute *pIXMLDOMAttribute = NULL;
	BSTR bsText = NULL;
	bsText = ::SysAllocString(asName);
	hr = apAttrs->getNamedItem(bsText, &pValue);

	if (FAILED(hr) || !pValue)
	{
		hr = mp_File->createAttribute(bsText, &pIXMLDOMAttribute);
		::SysFreeString(bsText); bsText = NULL;

		if (SUCCEEDED(hr) && pIXMLDOMAttribute)
		{
			VARIANT vtValue; vtValue.vt = VT_BSTR; vtValue.bstrVal = ::SysAllocString(asValue);
			hr = pIXMLDOMAttribute->put_nodeValue(vtValue);
			VariantClear(&vtValue);
			hr = apAttrs->setNamedItem(pIXMLDOMAttribute, &pValue); //-V519
			lbRc = SUCCEEDED(hr);
		}
	}
	else if (SUCCEEDED(hr) && pValue)
	{
		::SysFreeString(bsText); bsText = NULL;
		bsText = ::SysAllocString(asValue);
		hr = pValue->put_text(bsText);
		lbRc = SUCCEEDED(hr);
		::SysFreeString(bsText); bsText = NULL;
	}

	if (pValue) { pValue->Release(); pValue = NULL; }

	if (pIXMLDOMAttribute) { pIXMLDOMAttribute->Release(); pIXMLDOMAttribute = NULL; }

	return lbRc;
}

// Indenting XML keys using DOM
void SettingsXML::AppendIndent(IXMLDOMNode* apFrom, int nLevel)
{
	if (nLevel<=0) return;

	int nMax = min(32,nLevel); //-V112
	wchar_t szIndent[34];

	for(int i=0; i<nMax; i++) szIndent[i] = L'\t';

	szIndent[nMax] = 0;
	BSTR bsText = ::SysAllocString(szIndent);
	AppendText(apFrom, bsText);
	SysFreeString(bsText);
}

void SettingsXML::AppendNewLine(IXMLDOMNode* apFrom)
{
	BSTR bsText = SysAllocString(L"\r\n");
	AppendText(apFrom, bsText);
	SysFreeString(bsText);
}

void SettingsXML::AppendText(IXMLDOMNode* apFrom, BSTR asText)
{
	if (!asText || !*asText)
		return;

	VARIANT vtType;
	IXMLDOMNode* pChild = NULL;
	IXMLDOMNode *pIXMLDOMNode = NULL;
	vtType.vt = VT_I4; vtType.lVal = NODE_TEXT;
	HRESULT hr = mp_File->createNode(vtType, L"", L"", &pChild);

	if (SUCCEEDED(hr) && pChild)
	{
		hr = pChild->put_text(asText);
		hr = apFrom->appendChild(pChild, &pIXMLDOMNode); //-V519
		pChild->Release(); pChild = NULL;

		if (SUCCEEDED(hr) && pIXMLDOMNode)
		{
			pIXMLDOMNode->Release(); pIXMLDOMNode = NULL;
		}
	}
}

IXMLDOMNode* SettingsXML::FindItem(IXMLDOMNode* apFrom, const wchar_t* asType, const wchar_t* asName, bool abAllowCreate)
{
	HRESULT hr = S_OK;
	IXMLDOMNodeList* pList = NULL;
	IXMLDOMNode* pChild = NULL;
	IXMLDOMNamedNodeMap* pAttrs = NULL;
	IXMLDOMAttribute *pIXMLDOMAttribute = NULL;
	IXMLDOMNode *pIXMLDOMNode = NULL;
	IXMLDOMNode *pName = NULL;
	BSTR bsText = NULL;
	BOOL lbEmpty = TRUE;

	// �������� ��� �������� �������� ������� ����
	if (apFrom == NULL)
	{
		hr = S_FALSE;
	}
	else
	{
		bsText = ::SysAllocString(asType);
		hr = apFrom->selectNodes(bsText, &pList);
		::SysFreeString(bsText); bsText = NULL;
	}

	if (SUCCEEDED(hr) && pList)
	{
		hr = pList->reset();

		while((hr = pList->nextNode(&pIXMLDOMNode)) == S_OK && pIXMLDOMNode)
		{
			lbEmpty = FALSE;
			hr = pIXMLDOMNode->get_attributes(&pAttrs);

			if (SUCCEEDED(hr) && pAttrs)
			{
				bsText = GetAttr(pIXMLDOMNode, pAttrs, L"name");

				if (bsText)
				{
					if (lstrcmpi(bsText, asName) == 0)
					{
						::SysFreeString(bsText); bsText = NULL;
						pChild = pIXMLDOMNode; pIXMLDOMNode = NULL;
						break;
					}

					::SysFreeString(bsText); bsText = NULL;
				}
			}

			pIXMLDOMNode->Release(); pIXMLDOMNode = NULL;
		}

		pList->Release();
		//pList = NULL; -- ��� �������
	}
	
	if (lbEmpty && abAllowCreate && (asType[0] == L'k'))
	{
		bsText = ::SysAllocString(L"value");
		hr = apFrom->selectNodes(bsText, &pList);
		::SysFreeString(bsText); bsText = NULL;
		if (SUCCEEDED(hr) && pList)
		{
			hr = pList->reset();
			if ((hr = pList->nextNode(&pIXMLDOMNode)) == S_OK && pIXMLDOMNode)
			{
				lbEmpty = FALSE;
				pIXMLDOMNode->Release(); pIXMLDOMNode = NULL;
			}
			pList->Release();
			//pList = NULL; -- ��� �������
		}
	}

	if (!pChild && abAllowCreate)
	{
		VARIANT vtType; vtType.vt = VT_I4;
		vtType.lVal = NODE_ELEMENT;
		bsText = ::SysAllocString(asType);
		hr = mp_File->createNode(vtType, bsText, L"", &pChild);
		::SysFreeString(bsText); bsText = NULL;

		if (SUCCEEDED(hr) && pChild)
		{
			if (SetAttr(pChild, L"name", asName))
			{
				if (asType[0] == L'k')
				{
					AppendNewLine(pChild);
					mb_KeyEmpty = true;
				}

				if (asType[0] == L'k')
				{
					//if (mb_KeyEmpty)
					//AppendIndent(apFrom, lbEmpty ? (mi_Level-1) : mi_Level);
					AppendIndent(apFrom, (mi_Level-1));
				}
				else if (mb_KeyEmpty)
				{
					AppendIndent(apFrom, !lbEmpty ? (mi_Level-1) : mi_Level);
				}
				else
				{
					AppendIndent(apFrom, 1);
				}
				hr = apFrom->appendChild(pChild, &pIXMLDOMNode);
				pChild->Release(); pChild = NULL;

				if (FAILED(hr))
				{
					pAttrs->Release(); pAttrs = NULL;
				}
				else
				{
					pChild = pIXMLDOMNode;
					pIXMLDOMNode = NULL;
				}

				AppendNewLine(apFrom);
				AppendIndent(apFrom, mi_Level-1);
				if ((asType[0] != L'k') && mb_KeyEmpty)
					mb_KeyEmpty = false;
			}
			else
			{
				pChild->Release(); pChild = NULL;
			}
		}
	}

	return pChild;
}

// ����� ������ ���� ������� - ��� ������ �������� � *value ������ ��������
bool SettingsXML::Load(const wchar_t *regName, wchar_t **value)
{
	bool lbRc = false;
	HRESULT hr = S_OK;
	IXMLDOMNode* pChild = NULL;
	IXMLDOMNamedNodeMap* pAttrs = NULL;
	IXMLDOMAttribute *pIXMLDOMAttribute = NULL;
	IXMLDOMNode *pNode = NULL;
	IXMLDOMNodeList* pList = NULL;
	BSTR bsType = NULL;
	BSTR bsData = NULL;
	size_t nLen = 0;

	if (*value) {free(*value); *value = NULL;}

	if (mp_Key)
		pChild = FindItem(mp_Key, L"value", regName, false);

	if (!pChild)
		return false;

	hr = pChild->get_attributes(&pAttrs);

	if (SUCCEEDED(hr) && pAttrs)
	{
		bsType = GetAttr(pChild, pAttrs, L"type");
	}

	if (SUCCEEDED(hr) && bsType)
	{
		if (!lstrcmpi(bsType, L"multi"))
		{
			// ��� �������� �������� ���:
			//<value name="CmdLineHistory" type="multi">
			//	<line data="C:\Far\Far.exe"/>
			//	<line data="cmd"/>
			//</value>
			wchar_t *pszData = NULL, *pszCur = NULL;
			size_t nMaxLen = 0, nCurLen = 0;
			long nCount = 0;

			if (pAttrs) { pAttrs->Release(); pAttrs = NULL; }

			// �������� ��� �������� �������� ������� ����
			bsData = ::SysAllocString(L"line");
			hr = pChild->selectNodes(bsData, &pList);
			::SysFreeString(bsData); bsData = NULL;

			if (SUCCEEDED(hr) && pList)
			{
				hr = pList->get_length(&nCount);

				if (SUCCEEDED(hr) && nCount > 0)
				{
					HEAPVAL;
					nMaxLen = ((MAX_PATH+1) * nCount) + 1;
					pszData = (wchar_t*)malloc(nMaxLen * sizeof(wchar_t));
					pszCur = pszData;
					pszCur[0] = 0; pszCur[1] = 0;
					nCurLen = 2; // ����� ��������� DoubleZero
					HEAPVAL;
				}
			}

			if (SUCCEEDED(hr) && pList)
			{
				hr = pList->reset();

				while((hr = pList->nextNode(&pNode)) == S_OK && pNode)
				{
					bsData = GetAttr(pNode, L"data");
					pNode->Release(); pNode = NULL;

					if (SUCCEEDED(hr) && bsData)
					{
						nLen = _tcslen(bsData) + 1;

						if ((nCurLen + nLen) > nMaxLen)
						{
							// ����� �����������!
							nMaxLen = nCurLen + nLen + MAX_PATH + 1;
							wchar_t *psz = (wchar_t*)malloc(nMaxLen * sizeof(wchar_t));
							_ASSERTE(psz);

							if (!psz) break;  // �� ������� �������� ������!

							wmemmove(psz, pszData, nCurLen);
							pszCur = psz + (pszCur - pszData);
							HEAPVAL;
							free(pszData);
							pszData = psz;
							HEAPVAL;
						}

						lstrcpy(pszCur, bsData);
						pszCur += nLen; // ��������� - �� ����� ��� ��������� ������
						nCurLen += nLen;
						*pszCur = 0; // ASCIIZZ
						HEAPVAL;
						::SysFreeString(bsData); bsData = NULL;
					}
				}

				pList->Release(); pList = NULL;
			}

			// ������ ���-�� ��������� �������
			if (pszData)
			{
				*value = pszData;
				lbRc = true;
			}
		}
		else if (!lstrcmpi(bsType, L"string"))
		{
			bsData = GetAttr(pChild, pAttrs, L"data");

			if (SUCCEEDED(hr) && bsData)
			{
				nLen = _tcslen(bsData);
				*value = (wchar_t*)malloc((nLen+2)*sizeof(wchar_t));
				lstrcpy(*value, bsData);
				(*value)[nLen] = 0; // ��� ������ ���� ����� lstrcpy
				(*value)[nLen+1] = 0; // ASCIIZZ
				lbRc = true;
			}
		}

		// ��� ��������� ���� - �� ����������. ��� ����� ������ ������
	}

	if (bsType) { ::SysFreeString(bsType); bsType = NULL; }

	if (bsData) { ::SysFreeString(bsData); bsData = NULL; }

	if (pChild) { pChild->Release(); pChild = NULL; }

	if (pAttrs) { pAttrs->Release(); pAttrs = NULL; }

	if (!lbRc)
	{
		_ASSERTE(*value == NULL);
		*value = (wchar_t*)malloc(sizeof(wchar_t)*2);
		(*value)[0] = 0; (*value)[1] = 0; // �� ������ REG_MULTI_SZ
	}

	return lbRc;
}
// � ��� �������, ���� �������� ��� (��� ��� ������������) value �� �������
bool SettingsXML::Load(const wchar_t *regName, wchar_t *value, int maxLen)
{
	_ASSERTE(maxLen>1);
	bool lbRc = false;
	wchar_t* pszValue = NULL;

	if (Load(regName, &pszValue))
	{
		if (pszValue)
			lstrcpyn(value, pszValue, maxLen);
		else
			value[0] = 0;

		lbRc = true;
	}

	if (pszValue) free(pszValue);

	return lbRc;
}
bool SettingsXML::Load(const wchar_t *regName, LPBYTE value, DWORD nSize)
{
	bool lbRc = false;
	HRESULT hr = S_OK;
	IXMLDOMNode* pChild = NULL;
	IXMLDOMNamedNodeMap* pAttrs = NULL;
	IXMLDOMAttribute *pIXMLDOMAttribute = NULL;
	IXMLDOMNode *pNode = NULL;
	BSTR bsType = NULL;
	BSTR bsData = NULL;

	if (!value || !nSize)
		return false;

	if (mp_Key)
		pChild = FindItem(mp_Key, L"value", regName, false);

	if (!pChild)
		return false;

	hr = pChild->get_attributes(&pAttrs);

	if (SUCCEEDED(hr) && pAttrs)
	{
		bsType = GetAttr(pChild, pAttrs, L"type");
	}

	if (SUCCEEDED(hr) && bsType)
	{
		bsData = GetAttr(pChild, pAttrs, L"data");
	}

	if (SUCCEEDED(hr) && bsData)
	{
		if (!lstrcmpi(bsType, L"string"))
		{
#ifdef _DEBUG
			DWORD nLen = _tcslen(bsData) + 1;
#endif
			DWORD nMaxLen = nSize / 2;
			lstrcpyn((wchar_t*)value, bsData, nMaxLen);
			lbRc = true;
		}
		else if (!lstrcmpi(bsType, L"ulong"))
		{
			wchar_t* pszEnd = NULL;
			DWORD lVal = wcstoul(bsData, &pszEnd, 10);

			if (nSize > 4) nSize = 4;

			if (pszEnd && pszEnd != bsData)
			{
				memmove(value, &lVal, nSize);
				lbRc = true;
			}
		}
		else if (!lstrcmpi(bsType, L"long"))
		{
			wchar_t* pszEnd = NULL;
			int lVal = wcstol(bsData, &pszEnd, 10);

			if (nSize > 4) nSize = 4;

			if (pszEnd && pszEnd != bsData)
			{
				memmove(value, &lVal, nSize);
				lbRc = true;
			}
		}
		else if (!lstrcmpi(bsType, L"dword"))
		{
			wchar_t* pszEnd = NULL;
			DWORD lVal = wcstoul(bsData, &pszEnd, 16);

			if (nSize > 4) nSize = 4;

			if (pszEnd && pszEnd != bsData)
			{
				memmove(value, &lVal, nSize);
				lbRc = true;
			}
		}
		else if (!lstrcmpi(bsType, L"hex"))
		{
			wchar_t* pszCur = bsData;
			wchar_t* pszEnd = NULL;
			LPBYTE pCur = value;
			wchar_t cHex;
			DWORD lVal = 0;
			lbRc = true;
			
			while(*pszCur && nSize)
			{
				lVal = 0;
				cHex = *(pszCur++);

				if (cHex >= L'0' && cHex <= L'9')
				{
					lVal = cHex - L'0';
				}
				else if (cHex >= L'a' && cHex <= L'f')
				{
					lVal = cHex - L'a' + 10;
				}
				else if (cHex >= L'A' && cHex <= L'F')
				{
					lVal = cHex - L'A' + 10;
				}
				else
				{
					lbRc = false; break;
				}

				cHex = *(pszCur++);

				if (cHex && cHex != L',')
				{
					lVal = lVal << 4;

					if (cHex >= L'0' && cHex <= L'9')
					{
						lVal |= cHex - L'0';
					}
					else if (cHex >= L'a' && cHex <= L'f')
					{
						lVal |= cHex - L'a' + 10;
					}
					else if (cHex >= L'A' && cHex <= L'F')
					{
						lVal |= cHex - L'A' + 10;
					}
					else
					{
						lbRc = false; break;
					}

					cHex = *(pszCur++);
				}

				*pCur = (BYTE)lVal;
				pCur++; nSize--;

				if (cHex != L',')
				{
					break;
				}
			}

			while(nSize--)  // �������� �����
				*(pCur++) = 0;
		}
	}

	// ��������� ���� (������) - �� ����������

	if (bsType) { ::SysFreeString(bsType); bsType = NULL; }

	if (bsData) { ::SysFreeString(bsData); bsData = NULL; }

	if (pChild) { pChild->Release(); pChild = NULL; }

	if (pAttrs) { pAttrs->Release(); pAttrs = NULL; }

	return lbRc;
}

// ������� ����������� ��������� REG_MULTI_SZ
void SettingsXML::Delete(const wchar_t *regName)
{
	Save(regName, NULL, REG_MULTI_SZ, 0);
}

void SettingsXML::Save(const wchar_t *regName, const wchar_t *value)
{
	if (!value) value = L"";  // ���� ��� ������ � NULL

	Save(regName, (LPCBYTE)value, REG_SZ, (_tcslen(value)+1)*sizeof(wchar_t));
}
void SettingsXML::Save(const wchar_t *regName, LPCBYTE value, DWORD nType, DWORD nSize)
{
	HRESULT hr = S_OK;
	IXMLDOMNamedNodeMap* pAttrs = NULL;
	IXMLDOMNodeList* pList = NULL;
	IXMLDOMAttribute *pIXMLDOMAttribute = NULL;
	IXMLDOMNode *pNode = NULL;
	IXMLDOMNode* pChild = NULL;
	IXMLDOMNode *pNodeRmv = NULL;
	BSTR bsValue = NULL;
	BSTR bsType = NULL;
	bool bNeedSetType = false;
	// nType:
	// REG_DWORD:    ���������� ����� � 16-������ ��� 10-���� �������, � ����������� �� ����, ��� ������ ������� � xml ("dword"/"ulong"/"long")
	// REG_BINARY:   ������ � hex (FF,FF,...)
	// REG_SZ:       ASCIIZ ������, ����� �����������������, ����� nSize/2 �� ��� ������ ����� ������
	// REG_MULTI_SZ: ASCIIZZ. ��� ������������ <list...> ����� ���������, ��� �� �� ������� �� ������� nSize
	pChild = FindItem(mp_Key, L"value", regName, true); // �������, ���� ��� ��� ����

	if (!pChild)
		goto wrap;

	hr = pChild->get_attributes(&pAttrs);

	if (FAILED(hr) || !pAttrs)
		goto wrap;

	bsType = GetAttr(pChild, pAttrs, L"type");

	switch(nType)
	{
		case REG_DWORD:
		{
			wchar_t szValue[32];

			if (bsType && (bsType[0] == L'u' || bsType[0] == L'U'))
			{
				_wsprintf(szValue, SKIPLEN(countof(szValue)) L"%u", *(LPDWORD)value);
			}
			else if (bsType && (bsType[0] == L'l' || bsType[0] == L'L'))
			{
				_wsprintf(szValue, SKIPLEN(countof(szValue)) L"%i", *(int*)value);
			}
			else
			{
				_wsprintf(szValue, SKIPLEN(countof(szValue)) L"%08x", *(LPDWORD)value);

				if (bsType) ::SysFreeString(bsType);

				// ����� ��������/���������� ���
				bsType = ::SysAllocString(L"dword"); bNeedSetType = true;
			}

			bsValue = ::SysAllocString(szValue);
		} break;
		case REG_BINARY:
		{
			if (nSize == 1 && bsType && (bsType[0] == L'u' || bsType[0] == L'U'))
			{
				wchar_t szValue[4];
				BYTE bt = *value;
				_wsprintf(szValue, SKIPLEN(countof(szValue)) L"%u", (DWORD)bt);
				bsValue = ::SysAllocString(szValue);
			}
			else if (nSize == 1 && bsType && (bsType[0] == L'l' || bsType[0] == L'L'))
			{
				wchar_t szValue[4];
				char bt = *value;
				_wsprintf(szValue, SKIPLEN(countof(szValue)) L"%i", (int)bt);
				bsValue = ::SysAllocString(szValue);
			}
			else
			{
				DWORD nLen = nSize*2 + (nSize-1); // �� 2 ������� �� ���� + ',' ����� ����
				bsValue = ::SysAllocStringLen(NULL, nLen);
				nLen ++; // ����� ����� �� ��������� WCHAR �� '\0'
				wchar_t* psz = (wchar_t*)bsValue;
				LPCBYTE  ptr = value;

				while(nSize)
				{
					_wsprintf(psz, SKIPLEN(nLen-(psz-bsValue)) L"%02x", (DWORD)*ptr);
					ptr++; nSize--; psz+=2;

					if (nSize)
						*(psz++) = L',';
				}

				if (bsType && lstrcmp(bsType, L"hex"))
				{
					// �������� ������ "hex"
					::SysFreeString(bsType); bsType = NULL;
				}

				if (!bsType)
				{
					// ����� ��������/���������� ���
					bsType = ::SysAllocString(L"hex"); bNeedSetType = true;
				}
			}
		} break;
		case REG_SZ:
		{
			wchar_t* psz = (wchar_t*)value;
			bsValue = ::SysAllocString(psz);

			if (bsType && lstrcmp(bsType, L"string"))
			{
				// �������� ������ "string"
				::SysFreeString(bsType); bsType = NULL;
			}

			if (!bsType)
			{
				// ����� ��������/���������� ���
				bsType = ::SysAllocString(L"string"); bNeedSetType = true;
			}
		} break;
		case REG_MULTI_SZ:
		{
			if (bsType && lstrcmp(bsType, L"multi"))
			{
				// �������� ������ "multi"
				::SysFreeString(bsType); bsType = NULL;
			}

			if (!bsType)
			{
				// ����� ��������/���������� ���
				bsType = ::SysAllocString(L"multi"); bNeedSetType = true;
			}
		} break;
		default:
			goto wrap; // �� ��������������
	}

	if (bNeedSetType)
	{
		_ASSERTE(bsType!=NULL);
		SetAttr(pChild, pAttrs, L"type", bsType);
		::SysFreeString(bsType); bsType = NULL;
	}

	// ������ ���������� ��������
	if (nType != REG_MULTI_SZ)
	{
		_ASSERTE(bsValue != NULL);
		SetAttr(pChild, pAttrs, L"data", bsValue);
		::SysFreeString(bsValue); bsValue = NULL;
	}
	else     // ��� ����� ����������� ������ ��������� <list>
	{
		VARIANT_BOOL bHasChild = VARIANT_FALSE;
		DOMNodeType  nodeType = NODE_INVALID;
		// ���� ����� ��� �������� "data" - ������� ��� �� ������ ���������
		hr = pAttrs->getNamedItem(L"data", &pNode);

		if (SUCCEEDED(hr) && pNode)
		{
			hr = pChild->removeChild(pNode, &pNodeRmv);
			pNode->Release(); pNode = NULL;

			if (pNodeRmv) { pNodeRmv->Release(); pNodeRmv = NULL; }
		}

		//TODO: ����� �������� ������� ������?
		// ������� ��������
#ifdef _DEBUG
		hr = pChild->get_nodeType(&nodeType);
#endif
		hr = pChild->hasChildNodes(&bHasChild);

		if (bHasChild)
		{
			while((hr = pChild->get_firstChild(&pNode)) == S_OK && pNode)
			{
				hr = pNode->get_nodeType(&nodeType);
#ifdef _DEBUG
				BSTR bsDebug = NULL;
				pNode->get_text(&bsDebug);

				if (bsDebug) ::SysFreeString(bsDebug); bsDebug = NULL;

#endif
				hr = pChild->removeChild(pNode, &pNodeRmv);

				if (pNodeRmv) { pNodeRmv->Release(); pNodeRmv = NULL; }

				pNode->Release(); pNode = NULL;
			}
		}

		// ������ - ��������� ������
		wchar_t* psz = (wchar_t*)value;
		BSTR bsNodeType = ::SysAllocString(L"line");
		VARIANT vtType; vtType.vt = VT_I4; vtType.lVal = NODE_ELEMENT;
		long nAllLen = nSize/2; // ����� � wchar_t
		long nLen = 0;

		while(psz && *psz && nAllLen > 0)
		{
			hr = mp_File->createNode(vtType, bsNodeType, L"", &pNode);

			if (FAILED(hr) || !pNode)
				break;

			if (!SetAttr(pNode, L"data", psz))
				break;

			hr = pChild->appendChild(pNode, &pNodeRmv);
			pNode->Release(); pNode = NULL;

			if (pNodeRmv) { pNodeRmv->Release(); pNodeRmv = NULL; }

			if (FAILED(hr))
				break;

			nLen = _tcslen(psz)+1;
			psz += nLen;
			nAllLen -= nLen;
		}

		_ASSERTE(nAllLen <= 1);
	}

	mb_Modified = true;
wrap:

	if (pIXMLDOMAttribute) { pIXMLDOMAttribute->Release(); pIXMLDOMAttribute = NULL; }

	if (pNode) { pNode->Release(); pNode = NULL; }

	if (pNodeRmv) { pNodeRmv->Release(); pNodeRmv = NULL; }

	if (pChild) { pChild->Release(); pChild = NULL; }

	if (pAttrs) { pAttrs->Release(); pAttrs = NULL; }

	if (bsValue) { ::SysFreeString(bsValue); bsValue = NULL; }

	if (bsType) { ::SysFreeString(bsType); bsType = NULL; }
}
#endif
