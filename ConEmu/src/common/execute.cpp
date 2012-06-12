/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
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
#include <TlHelp32.h>
#include "../common/common.hpp"
#include "Execute.h"
#include "WinObjects.h"





// ��������� ����� �� �������� GetFileInfo, �������� ����������� ���������� � ���� PE-������

// ���������� ��������� IMAGE_SUBSYSTEM_* ���� ������� ��������
// ��� ������ �� ��������� IMAGE_SUBSYTEM_UNKNOWN ��������
// "���� �� �������� �����������".
// ��� DOS-���������� ��������� ��� ���� �������� �����.

// 17.12.2010 Maks
// ���� GetImageSubsystem ������ true - �� ����� ����� ��������� ��������� ��������
// IMAGE_SUBSYSTEM_WINDOWS_CUI    -- Win Console (32/64)
// IMAGE_SUBSYSTEM_DOS_EXECUTABLE -- DOS Executable (ImageBits == 16)

//#define IMAGE_SUBSYSTEM_DOS_EXECUTABLE  255

//struct IMAGE_HEADERS
//{
//	DWORD Signature;
//	IMAGE_FILE_HEADER FileHeader;
//	union
//	{
//		IMAGE_OPTIONAL_HEADER32 OptionalHeader32;
//		IMAGE_OPTIONAL_HEADER64 OptionalHeader64;
//	};
//};

bool GetImageSubsystem(const wchar_t *FileName,DWORD& ImageSubsystem,DWORD& ImageBits/*16/32/64*/,DWORD& FileAttrs)
{
	bool Result = false;
	ImageSubsystem = IMAGE_SUBSYSTEM_UNKNOWN;
	ImageBits = 0;
	FileAttrs = (DWORD)-1;
	wchar_t* pszExpand = NULL;
	// �������� � UNC? ���� ��� CreateProcess UNC �� ������������, ��� ��� ������ ���� ���
	HANDLE hModuleFile = CreateFile(FileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);

	// ���������� ���������
	if ((hModuleFile == INVALID_HANDLE_VALUE) && FileName && wcschr(FileName, L'%'))
	{
		pszExpand = ExpandEnvStr(FileName);
		if (pszExpand)
		{
			hModuleFile = CreateFile(pszExpand,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
			if (hModuleFile != INVALID_HANDLE_VALUE)
			{
				FileName = pszExpand;
			}
		}
	}

#if 0
	// ���� �� ������ ���� � ����� - ����������� ����� ��� � %PATH%
	if (hModuleFile != INVALID_HANDLE_VALUE && FileName && wcschr(FileName, L'\\') == NULL && wcschr(FileName, L'.') != NULL)
	{
		DWORD nErrCode = GetLastError();
		if (nErrCode)
		{
			wchar_t szFind[MAX_PATH], *psz;
			DWORD nLen = SearchPath(NULL, FileName, NULL, countof(szFind), szFind, &psz);
			if (nLen && nLen < countof(szFind))
				hModuleFile = CreateFile(szFind,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
		}
	}
#endif

	if (hModuleFile != INVALID_HANDLE_VALUE)
	{
		BY_HANDLE_FILE_INFORMATION bfi = {0};
		IMAGE_DOS_HEADER DOSHeader;
		DWORD ReadSize;

		if (GetFileInformationByHandle(hModuleFile, &bfi))
			FileAttrs = bfi.dwFileAttributes;

		// ��� ��� ������ - ����� ������� IMAGE_SUBSYSTEM_BATCH_FILE
		LPCWSTR pszExt = PointToExt(FileName);
		if (pszExt && (!lstrcmpi(pszExt, L".cmd") || !lstrcmpiW(pszExt, L".bat")))
		{
			CloseHandle(hModuleFile);
			ImageSubsystem = IMAGE_SUBSYSTEM_BATCH_FILE;
			ImageBits = IsWindows64() ? 64 : 32; //-V112
			Result = true;
			goto wrap;
		}

		if (ReadFile(hModuleFile,&DOSHeader,sizeof(DOSHeader),&ReadSize,NULL))
		{
			_ASSERTE(IMAGE_DOS_SIGNATURE==0x5A4D);
			if (DOSHeader.e_magic != IMAGE_DOS_SIGNATURE)
			{
				//const wchar_t *pszExt = wcsrchr(FileName, L'.');

				if (pszExt && lstrcmpiW(pszExt, L".com") == 0)
				{
					ImageSubsystem = IMAGE_SUBSYSTEM_DOS_EXECUTABLE;
					ImageBits = 16;
					Result = true;
				}
			}
			else
			{
				ImageSubsystem = IMAGE_SUBSYSTEM_DOS_EXECUTABLE;
				ImageBits = 16;
				Result = true;

				if (SetFilePointer(hModuleFile,DOSHeader.e_lfanew,NULL,FILE_BEGIN))
				{
					IMAGE_HEADERS PEHeader;

					if (ReadFile(hModuleFile,&PEHeader,sizeof(PEHeader),&ReadSize,NULL))
					{
						_ASSERTE(IMAGE_NT_SIGNATURE==0x00004550);
						if (PEHeader.Signature == IMAGE_NT_SIGNATURE)
						{
							switch(PEHeader.OptionalHeader32.Magic)
							{
								case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
								{
									ImageSubsystem = PEHeader.OptionalHeader32.Subsystem;
									_ASSERTE((ImageSubsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI) || (ImageSubsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI));
									ImageBits = 32; //-V112
								}
								break;
								case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
								{
									ImageSubsystem = PEHeader.OptionalHeader64.Subsystem;
									_ASSERTE((ImageSubsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI) || (ImageSubsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI));
									ImageBits = 64;
								}
								break;
								/*default:
									{
										// unknown magic
									}*/
							}
						}
						else if ((WORD)PEHeader.Signature == IMAGE_OS2_SIGNATURE)
						{
							ImageBits = 32; //-V112
							/*
							NE,  ���...  � ��� ���������� ��� ��� ������?

							Andrzej Novosiolov <andrzej@se.kiev.ua>
							AN> ��������������� �� ����� "Target operating system" NE-���������
							AN> (1 ���� �� �������� 0x36). ���� ��� Windows (�������� 2, 4) - �������������
							AN> GUI, ���� OS/2 � ������ �������� (��������� ��������) - ������������� �������.
							*/
							BYTE ne_exetyp = reinterpret_cast<PIMAGE_OS2_HEADER>(&PEHeader)->ne_exetyp;

							if (ne_exetyp==2||ne_exetyp==4) //-V112
							{
								ImageSubsystem = IMAGE_SUBSYSTEM_WINDOWS_GUI;
							}
							else
							{
								ImageSubsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI;
							}
						}

						/*else
						{
							// unknown signature
						}*/
					}

					/*else
					{
						// ������ ����� � ������� ���������� ��������� ;-(
					}*/
				}

				/*else
				{
					// ������ ������� ���� ���� � �����, �.�. dos_head.e_lfanew ������
					// ������� � �������������� ����� (�������� ��� ������ ���� DOS-����)
				}*/
			}

			/*else
			{
				// ��� �� ����������� ���� - � ���� ���� ��������� MZ, ��������, NLM-������
				// TODO: ����� ����� ��������� POSIX �������, �������� "/usr/bin/sh"
			}*/
		}

		/*else
		{
			// ������ ������
		}*/
		CloseHandle(hModuleFile);
	}

	/*else
	{
		// ������ ��������
	}*/
wrap:
	SafeFree(pszExpand);
	return Result;
}

bool GetImageSubsystem(DWORD& ImageSubsystem,DWORD& ImageBits/*16/32/64*/)
{
	HMODULE hModule = GetModuleHandle(NULL);

	ImageSubsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI;
	#ifdef _WIN64
	ImageBits = 64;
	#else
	ImageBits = 32; //-V112
	#endif

	IMAGE_DOS_HEADER* dos_header = (IMAGE_DOS_HEADER*)hModule;
	IMAGE_HEADERS* nt_header = NULL;

	_ASSERTE(IMAGE_DOS_SIGNATURE==0x5A4D);
	if (dos_header && dos_header->e_magic == IMAGE_DOS_SIGNATURE /*'ZM'*/)
	{
		nt_header = (IMAGE_HEADERS*)((char*)hModule + dos_header->e_lfanew);

		_ASSERTE(IMAGE_NT_SIGNATURE==0x00004550);
		if (nt_header->Signature != IMAGE_NT_SIGNATURE)
			return false;

		switch(nt_header->OptionalHeader32.Magic)
		{
			case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
			{
				ImageSubsystem = nt_header->OptionalHeader32.Subsystem;
				_ASSERTE((ImageSubsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI) || (ImageSubsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI));
				_ASSERTE(ImageBits == 32); //-V112
			}
			break;
			case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
			{
				ImageSubsystem = nt_header->OptionalHeader64.Subsystem;
				_ASSERTE((ImageSubsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI) || (ImageSubsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI));
				_ASSERTE(ImageBits == 64);
			}
			break;
			/*default:
				{
					// unknown magic
				}*/
		}
		return true;
	}

	return false;
}

bool GetImageSubsystem(PROCESS_INFORMATION pi,DWORD& ImageSubsystem,DWORD& ImageBits/*16/32/64*/)
{
	DWORD nErrCode = 0;
	DWORD nFlags = TH32CS_SNAPMODULE;
	
	ImageBits = 32; //-V112
	ImageSubsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI;
	
	#ifdef _WIN64
		HMODULE hKernel = GetModuleHandle(L"kernel32.dll");
		if (hKernel)
		{
			typedef BOOL (WINAPI* IsWow64Process_t)(HANDLE, PBOOL);
			IsWow64Process_t IsWow64Process_f = (IsWow64Process_t)GetProcAddress(hKernel, "IsWow64Process");

			if (IsWow64Process_f)
			{
				BOOL bWow64 = FALSE;
				if (IsWow64Process_f(pi.hProcess, &bWow64) && !bWow64)
				{
					ImageBits = 64;
				}
				else
				{
					ImageBits = 32;
					nFlags = TH32CS_SNAPMODULE32;
				}
			}
		}
	#endif
	
	HANDLE hSnap = CreateToolhelp32Snapshot(nFlags, pi.dwProcessId);
	if (hSnap == INVALID_HANDLE_VALUE)
	{
		nErrCode = GetLastError();
		return false;
	}
	IMAGE_DOS_HEADER dos;
	IMAGE_HEADERS hdr;
	SIZE_T hdrReadSize;
	MODULEENTRY32 mi = {sizeof(MODULEENTRY32)};
	BOOL lbModule = Module32First(hSnap, &mi);
	CloseHandle(hSnap);
	if (!lbModule)
		return false;
	
	// ������ ����� ������� ������ ��������
	if (!ReadProcessMemory(pi.hProcess, mi.modBaseAddr, &dos, sizeof(dos), &hdrReadSize))
		nErrCode = -3;
	else if (dos.e_magic != IMAGE_DOS_SIGNATURE)
		nErrCode = -4; // ������������ ��������� - ������ ���� 'MZ'
	else if (!ReadProcessMemory(pi.hProcess, mi.modBaseAddr+dos.e_lfanew, &hdr, sizeof(hdr), &hdrReadSize))
		nErrCode = -5;
	else if (hdr.Signature != IMAGE_NT_SIGNATURE)
		nErrCode = -6;
	else if (hdr.OptionalHeader32.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC
	        &&  hdr.OptionalHeader64.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC)
		nErrCode = -7;
	else
	{
		nErrCode = 0;
		
		switch (hdr.OptionalHeader32.Magic)
		{
			case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
			{
				_ASSERTE(ImageBits == 32); //-V112
				ImageBits = 32; //-V112
				ImageSubsystem = hdr.OptionalHeader32.Subsystem;
				_ASSERTE((ImageSubsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI) || (ImageSubsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI));
			}
			break;
			case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
			{
				_ASSERTE(ImageBits == 64);
				ImageBits = 64;
				ImageSubsystem = hdr.OptionalHeader64.Subsystem;
				_ASSERTE((ImageSubsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI) || (ImageSubsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI));
			}
			break;
			default:
			{
				nErrCode = -8;
			}
		}
	}
	
	return (nErrCode == 0);
}


/* ******************************************* */
/* PE File Info (kernel32.dll -> LoadLibraryW) */
/* ******************************************* */
struct IMAGE_MAPPING
{
	union
	{
		LPBYTE ptrBegin; //-V117
		PIMAGE_DOS_HEADER pDos; //-V117
	};
	LPBYTE ptrEnd;
	IMAGE_HEADERS* pHdr;
};
#ifdef _DEBUG
static bool ValidateMemory(LPVOID ptr, DWORD_PTR nSize, IMAGE_MAPPING* pImg)
{
	if ((ptr == NULL) || (((LPBYTE)ptr) < pImg->ptrBegin))
		return false;

	if ((((LPBYTE)ptr) + nSize) >= pImg->ptrEnd)
		return false;

	return true;
}
#endif

//================================================================================
//
// Given an RVA, look up the section header that encloses it and return a
// pointer to its IMAGE_SECTION_HEADER
//
PIMAGE_SECTION_HEADER GetEnclosingSectionHeader(DWORD rva, IMAGE_MAPPING* pImg)
{
	// IMAGE_FIRST_SECTION doesn't need 32/64 versions since the file header is the same either way.
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(pImg->pHdr); //-V220
	unsigned i;

	for(i = 0; i < pImg->pHdr->FileHeader.NumberOfSections; i++, section++)
	{
		// This 3 line idiocy is because Watcom's linker actually sets the
		// Misc.VirtualSize field to 0.  (!!! - Retards....!!!)
		DWORD size = section->Misc.VirtualSize;

		if (0 == size)
			size = section->SizeOfRawData;

		// Is the RVA within this section?
		if ((rva >= section->VirtualAddress) &&
		        (rva < (section->VirtualAddress + size)))
			return section;
	}

	return NULL;
}

LPVOID GetPtrFromRVA(DWORD rva, IMAGE_MAPPING* pImg)
{
	if (!pImg || !pImg->ptrBegin || !pImg->pHdr)
	{
		_ASSERTE(pImg!=NULL && pImg->ptrBegin!=NULL && pImg->pHdr!=NULL);
		return NULL;
	}

	PIMAGE_SECTION_HEADER pSectionHdr;
	INT delta;
	pSectionHdr = GetEnclosingSectionHeader(rva, pImg);

	if (!pSectionHdr)
		return NULL;

	delta = (INT)(pSectionHdr->VirtualAddress - pSectionHdr->PointerToRawData);
	return (LPVOID)(pImg->ptrBegin + rva - delta);
}

//int ParseExportsSection(IMAGE_MAPPING* pImg)
//{
//	PIMAGE_EXPORT_DIRECTORY pExportDir;
//	//PIMAGE_SECTION_HEADER header;
//	//INT delta;
//	//PSTR pszFilename;
//	DWORD i;
//	PDWORD pdwFunctions;
//	PWORD pwOrdinals;
//	DWORD *pszFuncNames;
//	DWORD exportsStartRVA; //, exportsEndRVA;
//	LPCSTR pszFuncName;
//
//	//exportsStartRVA = GetImgDirEntryRVA(pNTHeader,IMAGE_DIRECTORY_ENTRY_EXPORT);
//	//exportsEndRVA = exportsStartRVA +
//	//	GetImgDirEntrySize(pNTHeader, IMAGE_DIRECTORY_ENTRY_EXPORT);
//	if (pImg->pHdr->OptionalHeader64.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
//	{
//		exportsStartRVA = pImg->pHdr->OptionalHeader64.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
//		//exportDirSize = hdr.OptionalHeader64.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
//		//pExportDirAddr = GetPtrFromRVA(exportsStartRVA, (PIMAGE_NT_HEADERS64)&hdr, mi.modBaseAddr);
//	}
//	else
//	{
//		exportsStartRVA = pImg->pHdr->OptionalHeader32.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
//		//exportDirSize = hdr.OptionalHeader32.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
//		//pExportDirAddr = GetPtrFromRVA(exportsStartRVA, (PIMAGE_NT_HEADERS32)&hdr, mi.modBaseAddr);
//	}
//
//	// Get the IMAGE_SECTION_HEADER that contains the exports.  This is
//	// usually the .edata section, but doesn't have to be.
//	//header = GetEnclosingSectionHeader( exportsStartRVA, pNTHeader );
//	//if ( !header )
//	//	return -201;
//	//delta = (INT)(header->VirtualAddress - header->PointerToRawData);
//	pExportDir = (PIMAGE_EXPORT_DIRECTORY)GetPtrFromRVA(exportsStartRVA, pImg);
//
//	if (!pExportDir || !ValidateMemory(pExportDir, sizeof(IMAGE_EXPORT_DIRECTORY), pImg))
//		return -201;
//
//	//pszFilename = (PSTR)GetPtrFromRVA( pExportDir->Name, pNTHeader, pImageBase );
//	pdwFunctions =	(PDWORD)GetPtrFromRVA(pExportDir->AddressOfFunctions, pImg);
//	pwOrdinals =	(PWORD) GetPtrFromRVA(pExportDir->AddressOfNameOrdinals, pImg);
//	pszFuncNames =	(DWORD*)GetPtrFromRVA(pExportDir->AddressOfNames, pImg);
//
//	if (!pdwFunctions || !pwOrdinals || !pszFuncNames)
//		return -202;
//
//	for(i=0; i < pExportDir->NumberOfFunctions; i++, pdwFunctions++)
//	{
//		DWORD entryPointRVA = *pdwFunctions;
//
//		if (entryPointRVA == 0)      // Skip over gaps in exported function
//			continue;               // ordinals (the entrypoint is 0 for
//
//		// these functions).
//
//		// See if this function has an associated name exported for it.
//		for(unsigned j=0; j < pExportDir->NumberOfNames; j++)
//		{
//			if (pwOrdinals[j] == i)
//			{
//				pszFuncName = (LPCSTR)GetPtrFromRVA(pszFuncNames[j], pImg);
//
//				if (pszFuncName)
//				{
//					if (strcmp(pszFuncName, "LoadLibraryW"))
//					{
//						// �����
//						return entryPointRVA;
//					}
//				}
//			}
//		}
//	}
//
//	return -203;
//}

//static int FindLoadLibrary(LPCWSTR asKernel32)
//{
//	int nLoadLibraryOffset = 0;
//	MWow64Disable wow; wow.Disable(); // ��������� � Win64 ��������. ���� �� ����� - ������ �� ������.
//	HANDLE hMapping = NULL, hKernel = NULL;
//	LPBYTE ptrMapping = NULL;
//	LARGE_INTEGER nFileSize;
//	hKernel = CreateFile(asKernel32, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
//
//	if (!hKernel || (hKernel == INVALID_HANDLE_VALUE))
//		nLoadLibraryOffset = -101;
//	else if (!GetFileSizeEx(hKernel, &nFileSize) || nFileSize.HighPart)
//		nLoadLibraryOffset = -102;
//	else if (nFileSize.LowPart < (sizeof(IMAGE_DOS_HEADER) + sizeof(IMAGE_HEADERS)))
//		nLoadLibraryOffset = -103;
//	else if (!(hMapping = CreateFileMapping(hKernel, NULL, PAGE_READONLY, 0,0, NULL)) || (hMapping == INVALID_HANDLE_VALUE))
//		nLoadLibraryOffset = -104;
//	else if (!(ptrMapping = (LPBYTE)MapViewOfFile(hMapping, FILE_MAP_READ, 0,0,0)))
//		nLoadLibraryOffset = -105;
//	else // �������
//	{
//		IMAGE_MAPPING img;
//		img.pDos = (PIMAGE_DOS_HEADER)ptrMapping;
//		img.pHdr = (IMAGE_HEADERS*)(ptrMapping + img.pDos->e_lfanew);
//		img.ptrEnd = (ptrMapping + nFileSize.LowPart);
//
//		if (img.pDos->e_magic != IMAGE_DOS_SIGNATURE)
//			nLoadLibraryOffset = -110; // ������������ ��������� - ������ ���� 'MZ'
//		else if (!ValidateMemory(img.pHdr, sizeof(*img.pHdr), &img))
//			nLoadLibraryOffset = -111;
//		else if (img.pHdr->Signature != IMAGE_NT_SIGNATURE)
//			nLoadLibraryOffset = -112;
//		else if (img.pHdr->OptionalHeader32.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC
//		        &&  img.pHdr->OptionalHeader64.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC)
//			nLoadLibraryOffset = -113;
//		else // OK, ���� ��������� ��������� ������
//		{
//			nLoadLibraryOffset = ParseExportsSection(&img);
//		}
//	}
//
//	// ��������� �����������
//	if (ptrMapping)
//		UnmapViewOfFile(ptrMapping);
//
//	if (hMapping && (hMapping != INVALID_HANDLE_VALUE))
//		CloseHandle(hMapping);
//
//	if (hKernel && (hKernel != INVALID_HANDLE_VALUE))
//		CloseHandle(hKernel);
//
//	// Found result
//	return nLoadLibraryOffset;
//}

//// ���������� ����� ��������� LoadLibraryW ��� ����������� ��������
//int FindKernelAddress(HANDLE ahProcess, DWORD anPID, DWORD* pLoadLibrary)
//{
//	int iRc = -100;
//	*pLoadLibrary = NULL;
//	int nBits = 0;
//	SIZE_T hdrReadSize;
//	IMAGE_DOS_HEADER dos;
//	IMAGE_HEADERS hdr;
//	MODULEENTRY32 mi = {sizeof(MODULEENTRY32)};
//	// Must be TH32CS_SNAPMODULE32 for spy 32bit from 64bit process
//	// ������� ������� ��� Native, ���� ������� �������� ������ �������� - ����������� snapshoot
//	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, anPID);
//
//	if (!hSnap || hSnap == INVALID_HANDLE_VALUE)
//		iRc = -1;
//	else
//	{
//		WARNING("�� ���� ���������, 32-������ ������� �� ����� �������� ���������� � 64-������!");
//
//		// ������ ����� ���������� �������� ��������
//		if (!Module32First(hSnap, &mi))
//			iRc = -2;
//		else if (!ReadProcessMemory(ahProcess, mi.modBaseAddr, &dos, sizeof(dos), &hdrReadSize))
//			iRc = -3;
//		else if (dos.e_magic != IMAGE_DOS_SIGNATURE)
//			iRc = -4; // ������������ ��������� - ������ ���� 'MZ'
//		else if (!ReadProcessMemory(ahProcess, mi.modBaseAddr+dos.e_lfanew, &hdr, sizeof(hdr), &hdrReadSize))
//			iRc = -5;
//		else if (hdr.Signature != IMAGE_NT_SIGNATURE)
//			iRc = -6;
//		else if (hdr.OptionalHeader32.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC
//		        &&  hdr.OptionalHeader64.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC)
//			iRc = -7;
//		else
//		{
//			TODO("������������, ��� ����� ����������� ���������� IMAGE_OS2_SIGNATURE?");
//			nBits = (hdr.OptionalHeader32.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) ? 32 : 64;
//#ifdef WIN64
//
//			// ���� ahProcess - 64 ����, �� ����� ����������� snapshoot � ������ TH32CS_SNAPMODULE32
//			// � ��������, �� ���� �������� �� ������, �.�. ConEmuC.exe - 32������.
//			if (nBits == 32)
//			{
//				CloseHandle(hSnap);
//				hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE32, anPID);
//
//				if (!hSnap || hSnap == INVALID_HANDLE_VALUE)
//				{
//					iRc = -8;
//					hSnap = NULL;
//				}
//				else if (!Module32First(hSnap, &mi))
//				{
//					iRc = -9;
//					CloseHandle(hSnap);
//					hSnap = NULL;
//				}
//			}
//
//#endif
//
//			if (hSnap != NULL)
//			{
//				iRc = (hdr.OptionalHeader32.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) ? -20 : -21;
//
//				do
//				{
//					if (lstrcmpi(mi.szModule, L"kernel32.dll") == 0)
//					{
//						//if (!ReadProcessMemory(ahProcess, mi.modBaseAddr, &dos, sizeof(dos), &hdrReadSize))
//						//	iRc = -23;
//						//else if (dos.e_magic != IMAGE_DOS_SIGNATURE)
//						//	iRc = -24; // ������������ ��������� - ������ ���� 'MZ'
//						//else if (!ReadProcessMemory(ahProcess, mi.modBaseAddr+dos.e_lfanew, &hdr, sizeof(hdr), &hdrReadSize))
//						//	iRc = -25;
//						//else if (hdr.Signature != IMAGE_NT_SIGNATURE)
//						//	iRc = -26;
//						//else if (hdr.OptionalHeader32.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC
//						//	&&  hdr.OptionalHeader64.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC)
//						//	iRc = -27;
//						//else
//						iRc = 0;
//						break;
//					}
//				}
//				while(Module32Next(hSnap, &mi));
//			}
//		}
//
//		if (hSnap)
//			CloseHandle(hSnap);
//	}
//
//	// ���� kernel32.dll ����� � �������������� ��������
//	if (iRc == 0 && nBits)
//	{
//		BOOL lbNeedLoad = FALSE;
//		static DWORD nLoadLibraryW32 = 0;
//		static DWORD nLoadLibraryW64 = 0;
//		DWORD_PTR ptr = 0;
//		lbNeedLoad = (nBits == 64) ? (nLoadLibraryW32 == 0) : (nLoadLibraryW64 == 0);
//
//		if (lbNeedLoad)
//		{
//			iRc = FindLoadLibrary(mi.szExePath);
//
//			if (iRc > 0)
//			{
//				if (nBits == 64)
//					nLoadLibraryW64 = iRc;
//				else
//					nLoadLibraryW32 = iRc;
//
//				lbNeedLoad = FALSE; // OK
//			}
//
//			//LPVOID pExportDirAddr;
//			//DWORD exportsStartRVA;
//			//DWORD exportDirSize;
//			//if (nBits == 64)
//			//{
//			//	exportsStartRVA = hdr.OptionalHeader64.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
//			//	exportDirSize = hdr.OptionalHeader64.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
//			//	pExportDirAddr = GetPtrFromRVA(exportsStartRVA, (PIMAGE_NT_HEADERS64)&hdr, mi.modBaseAddr);
//			//}
//			//else
//			//{
//			//	exportsStartRVA = hdr.OptionalHeader32.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
//			//	exportDirSize = hdr.OptionalHeader32.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
//			//	pExportDirAddr = GetPtrFromRVA(exportsStartRVA, (PIMAGE_NT_HEADERS32)&hdr, mi.modBaseAddr);
//			//}
//			//if (!pExportDirAddr)
//			//	iRc = -30;
//			//else if (!ReadProcessMemory(ahProcess, pExportDirAddr,
//		}
//
//		if (lbNeedLoad)
//		{
//			// �� �������
//			if (iRc == 0)
//				iRc = -40;
//		}
//		else
//		{
//			ptr = ((DWORD_PTR)mi.modBaseAddr) + ((nBits == 64) ? nLoadLibraryW64 : nLoadLibraryW32);
//
//			if (ptr != (DWORD)ptr)
//			{
//				// BaseAddress ���� ��� 64-������� Kernel32 ���, � �������� � 32-������ ���������,
//				// �� ��� �� ����� ���������, � ���� �� "����" - �� ���������� ������.
//				iRc = -41;
//			}
//			else
//				*pLoadLibrary = (DWORD)ptr;
//		}
//	}
//
//	return iRc;
//}

bool FindImageSubsystem(const wchar_t *Module, /*wchar_t* pstrDest,*/ DWORD& ImageSubsystem, DWORD& ImageBits, DWORD& FileAttrs)
{
	if (!Module || !*Module)
		return false;

	bool Result = false;
	//ImageSubsystem = IMAGE_SUBSYSTEM_UNKNOWN;

	// ���������� ��� �� ���������� - ������� ��� ������������ � ������ � CreateProcess!
	//// ������� ������ - ������� ����������
	//// ����� "����������" �� �������, ������� ������ ����������� ��������,
	//// ��������, ��������� ���������� ������� ���. ����������.
	//string strExcludeCmds;
	//GetRegKey(strSystemExecutor,L"ExcludeCmds",strExcludeCmds,L"");
	//UserDefinedList ExcludeCmdsList;
	//ExcludeCmdsList.Set(strExcludeCmds);
	//while (!ExcludeCmdsList.IsEmpty())
	//{
	//	if (!StrCmpI(Module,ExcludeCmdsList.GetNext()))
	//	{
	//		ImageSubsystem=IMAGE_SUBSYSTEM_WINDOWS_CUI;
	//		Result=true;
	//		break;
	//	}
	//}

	//string strFullName=Module;
	LPCWSTR ModuleExt = PointToExt(Module);
	wchar_t *strPathExt/*[32767]*/ = NULL; //(L".COM;.EXE;.BAT;.CMD;.VBS;.JS;.WSH");
	wchar_t *strPathEnv/*[32767]*/ = NULL;
	wchar_t *strExpand/*[32767]*/ = NULL;
	wchar_t *strTmpName/*[32767]*/ = NULL;
	wchar_t *pszFilePart = NULL;
	DWORD nPathExtLen = 0;
	LPCWSTR pszPathExtEnd = NULL;
	LPWSTR Ext = NULL;

	typedef LONG (WINAPI *RegOpenKeyExW_t)(HKEY hKey, LPCTSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult);
	RegOpenKeyExW_t _RegOpenKeyEx = NULL;
	typedef LONG (WINAPI *RegQueryValueExW_t)(HKEY hKey, LPCTSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
	RegQueryValueExW_t _RegQueryValueEx = NULL;
	typedef LONG (WINAPI *RegCloseKey_t)(HKEY hKey);
	RegCloseKey_t _RegCloseKey = NULL;
	HMODULE hAdvApi = NULL;
	
	

	int cchstrPathExt = 32767;
	strPathExt = (wchar_t*)malloc(cchstrPathExt*sizeof(wchar_t)); *strPathExt = 0;
	int cchstrPathEnv = 32767;
	strPathEnv = (wchar_t*)malloc(cchstrPathEnv*sizeof(wchar_t)); *strPathEnv = 0;
	int cchstrExpand = 32767;
	strExpand = (wchar_t*)malloc(cchstrExpand*sizeof(wchar_t)); *strExpand = 0;
	int cchstrTmpName = 32767;
	strTmpName = (wchar_t*)malloc(cchstrTmpName*sizeof(wchar_t)); *strTmpName = 0;
		
	nPathExtLen = GetEnvironmentVariable(L"PATHEXT", strPathExt, cchstrPathExt-2);
	if (!nPathExtLen)
	{
		_wcscpy_c(strPathExt, cchstrPathExt, L".COM;.EXE;.BAT;.CMD;.VBS;.JS;.WSH");
		nPathExtLen = lstrlen(strPathExt);
	}
	pszPathExtEnd = strPathExt+nPathExtLen;
	// ������� �� ������
	strPathExt[nPathExtLen] = strPathExt[nPathExtLen+1] = 0;
	Ext = wcschr(strPathExt, L';');
	while (Ext)
	{
		*Ext = 0;
		Ext = wcschr(Ext+1, L';');
	}

	TODO("��������� �� ���������� ���� �����");

	// ������ ������ - � ������� ��������
	LPWSTR pszExtCur = strPathExt;
	while (pszExtCur < pszPathExtEnd)
	{
		Ext = pszExtCur;
		pszExtCur = pszExtCur + lstrlen(pszExtCur)+1;

		_wcscpyn_c(strTmpName, cchstrTmpName, Module, cchstrTmpName); //-V501

		if (!ModuleExt)
		{
			if (!*Ext)
				continue;
			_wcscatn_c(strTmpName, cchstrTmpName, Ext, cchstrTmpName);
		}

		if (GetImageSubsystem(strTmpName, ImageSubsystem, ImageBits/*16/32/64*/, FileAttrs))
		{
			Result = true;
			goto wrap;
		}

		if (ModuleExt)
		{
			break;
		}
	}

	// ������ ������ - �� �������� SearchPath

	// ����� �� ���������� PATH
	if (GetEnvironmentVariable(L"PATH", strPathEnv, cchstrPathEnv))
	{
		LPWSTR pszPathEnvEnd = strPathEnv + lstrlen(strPathEnv);

		LPWSTR pszPathCur = strPathEnv;
		while (pszPathCur && (pszPathCur < pszPathEnvEnd))
		{
			LPWSTR Path = pszPathCur;
			LPWSTR pszPathNext = wcschr(pszPathCur, L';');
			if (pszPathNext)
			{
				*pszPathNext = 0;
				pszPathCur = pszPathNext+1;
			}
			else
			{
				pszPathCur = pszPathEnvEnd;
			}
			if (!*Path)
				continue;

			pszExtCur = strPathExt;
			while (pszExtCur < pszPathExtEnd)
			{
				Ext = pszExtCur;
				pszExtCur = pszExtCur + lstrlen(pszExtCur)+1;
				if (!*Ext)
					continue;

				if (SearchPath(Path, Module, Ext, cchstrTmpName, strTmpName, &pszFilePart))
				{
					if (GetImageSubsystem(strTmpName, ImageSubsystem, ImageBits, FileAttrs))
					{
						Result = true;
						goto wrap;
					}
				}
			}
		}
	}

	pszExtCur = strPathExt;
	while (pszExtCur < pszPathExtEnd)
	{
		Ext = pszExtCur;
		pszExtCur = pszExtCur + lstrlen(pszExtCur)+1;
		if (!*Ext)
			continue;

		if (SearchPath(NULL, Module, Ext, cchstrTmpName, strTmpName, &pszFilePart))
		{
			if (GetImageSubsystem(strTmpName, ImageSubsystem, ImageBits, FileAttrs))
			{
				Result = true;
				goto wrap;
			}
		}
	}

	// ������ ������ - ����� � ������ � "App Paths"
	if (!wcschr(Module, L'\\'))
	{
		hAdvApi = LoadLibrary(L"AdvApi32.dll");
		if (!hAdvApi)
			goto wrap;
		_RegOpenKeyEx = (RegOpenKeyExW_t)GetProcAddress(hAdvApi, "RegOpenKeyExW");
		_RegQueryValueEx = (RegQueryValueExW_t)GetProcAddress(hAdvApi, "RegQueryValueExW");
		_RegCloseKey = (RegCloseKey_t)GetProcAddress(hAdvApi, "RegCloseKey");
		if (!_RegOpenKeyEx || !_RegQueryValueEx || !_RegCloseKey)
			goto wrap;

		LPCWSTR RegPath = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\";
		// � ������ Module �������� ����������� ������ �� ������ ����, �������
		// ������� �� SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths
		// ������� ������� � HKCU, ����� - � HKLM
		HKEY RootFindKey[] = {HKEY_CURRENT_USER,HKEY_LOCAL_MACHINE,HKEY_LOCAL_MACHINE};

		BOOL lbAddExt = FALSE;
		pszExtCur = strPathExt;
		while (pszExtCur < pszPathExtEnd)
		{
			if (!lbAddExt)
			{
				Ext = NULL;
				lbAddExt = TRUE;
			}
			else
			{
				Ext = pszExtCur;
				pszExtCur = pszExtCur + lstrlen(pszExtCur)+1;
				if (!*Ext)
					continue;
			}

			_wcscpy_c(strTmpName, cchstrTmpName, RegPath);
			_wcscatn_c(strTmpName, cchstrTmpName, Module, cchstrTmpName);
			if (Ext)
				_wcscatn_c(strTmpName, cchstrTmpName, Ext, cchstrTmpName);

			DWORD samDesired = KEY_QUERY_VALUE;
			DWORD RedirectionFlag = 0;
			// App Paths key is shared in Windows 7 and above
			OSVERSIONINFO osv = {sizeof(OSVERSIONINFO)};
			GetVersionEx(&osv);
			if (osv.dwMajorVersion < 6 || (osv.dwMajorVersion == 6 && osv.dwMinorVersion < 1))
			{
				#ifdef _WIN64
				RedirectionFlag = KEY_WOW64_32KEY;
				#else
				RedirectionFlag = IsWindows64() ? KEY_WOW64_64KEY : 0;
				#endif
			}
			for (size_t i = 0; i < countof(RootFindKey); i++)
			{
				if (i == (countof(RootFindKey)-1))
				{
					if (RedirectionFlag)
						samDesired |= RedirectionFlag;
					else
						break;
				}
				HKEY hKey;
				if (_RegOpenKeyEx(RootFindKey[i], strTmpName, 0, samDesired, &hKey) == ERROR_SUCCESS)
				{
					DWORD nType = 0, nSize = sizeof(strTmpName)-2;
					int RegResult = _RegQueryValueEx(hKey, L"", NULL, &nType, (LPBYTE)strTmpName, &nSize);
					_RegCloseKey(hKey);

					if ((RegResult == ERROR_SUCCESS) && (nType == REG_SZ || nType == REG_EXPAND_SZ || nType == REG_MULTI_SZ))
					{
						strTmpName[(nSize >> 1)+1] = 0;
						if (!ExpandEnvironmentStrings(strTmpName, strExpand, cchstrExpand))
							_wcscpy_c(strExpand, cchstrExpand, strTmpName);
						if (GetImageSubsystem(Unquote(strExpand), ImageSubsystem, ImageBits, FileAttrs))
						{
							Result = true;
							goto wrap;
						}
					}
				}
			}
		}
	}

wrap:
	if (strPathExt)
		free(strPathExt);
	if (strPathEnv)
		free(strPathEnv);
	if (strExpand)
		free(strExpand);
	if (strTmpName)
		free(strTmpName);
	if (hAdvApi)
		FreeLibrary(hAdvApi);
	return Result;
}
