
#pragma once

#define IMAGE_SUBSYSTEM_DOS_EXECUTABLE  255

// ���� GetImageSubsystem ������ true - �� ����� ����� ��������� ��������� ��������
// IMAGE_SUBSYSTEM_WINDOWS_CUI    -- Win Console (32/64)
// IMAGE_SUBSYSTEM_DOS_EXECUTABLE -- DOS Executable (ImageBits == 16)
bool GetImageSubsystem(const wchar_t *FileName,DWORD& ImageSubsystem,DWORD& ImageBits/*16/32/64*/);

// ���������� ����� ��������� LoadLibraryW ��� ����������� ��������
int FindKernelAddress(HANDLE ahProcess, DWORD anPID, DWORD* pLoadLibrary);
