﻿
#pragma once

#define CERR_FIRSTEXITCODE 100

typedef int CERR_EXIT_CODES;
const CERR_EXIT_CODES
	// Для проверки ключей "/IsConEmu", "/IsTerm", "/IsAnsi" в ConEmuC.exe
    CERR_CHKSTATE_ON   = 1,
	CERR_CHKSTATE_OFF  = 2, // "OFF" > "ON", чтобы при любых ошибках .cmd не подумал, что все хорошо.
	// Далее - коды "ошибок"
	CERR_GETCOMMANDLINE = CERR_FIRSTEXITCODE+0,
	CERR_CARGUMENT = CERR_FIRSTEXITCODE+1,
	CERR_CMDEXENOTFOUND = CERR_FIRSTEXITCODE+2,
	CERR_NOTENOUGHMEM1 = CERR_FIRSTEXITCODE+3,
	CERR_CREATESERVERTHREAD = CERR_FIRSTEXITCODE+4, //-V112
	CERR_CREATEPROCESS = CERR_FIRSTEXITCODE+5,
	CERR_WINEVENTTHREAD = CERR_FIRSTEXITCODE+6,
	CERR_CONINFAILED = CERR_FIRSTEXITCODE+7,
	CERR_GETCONSOLEWINDOW = CERR_FIRSTEXITCODE+8,
	CERR_EXITEVENT = CERR_FIRSTEXITCODE+9,
	CERR_GLOBALUPDATE = CERR_FIRSTEXITCODE+10,
	CERR_WINHOOKNOTCREATED = CERR_FIRSTEXITCODE+11,
	CERR_CREATEINPUTTHREAD = CERR_FIRSTEXITCODE+12,
	CERR_CONOUTFAILED = CERR_FIRSTEXITCODE+13,
	CERR_PROCESSTIMEOUT = CERR_FIRSTEXITCODE+14,
	CERR_REFRESHEVENT = CERR_FIRSTEXITCODE+15,
	CERR_CREATEREFRESHTHREAD = CERR_FIRSTEXITCODE+16,
	CERR_HELPREQUESTED = CERR_FIRSTEXITCODE+18,
	CERR_ATTACHFAILED = CERR_FIRSTEXITCODE+19,
	CERR_RUNNEWCONSOLE = CERR_FIRSTEXITCODE+21,
	CERR_CANTSTARTDEBUGGER = CERR_FIRSTEXITCODE+22,
	CERR_CREATEMAPPINGERR = CERR_FIRSTEXITCODE+23,
	CERR_MAPVIEWFILEERR = CERR_FIRSTEXITCODE+24,
	CERR_COLORERMAPPINGERR = CERR_FIRSTEXITCODE+25,
	CERR_EMPTY_COMSPEC_CMDLINE = CERR_FIRSTEXITCODE+26,
	CERR_CONEMUHK_NOTFOUND = CERR_FIRSTEXITCODE+27,
	CERR_CONSOLEMAIN_NOTFOUND = CERR_FIRSTEXITCODE+28,
	CERR_HOOKS_WAS_SET = CERR_FIRSTEXITCODE+29,
	CERR_HOOKS_FAILED = CERR_FIRSTEXITCODE+30,
	CERR_DLLMAIN_SKIPPED = CERR_FIRSTEXITCODE+31,
	CERR_ATTACH_NO_CONWND = CERR_FIRSTEXITCODE+32, //-V112
	CERR_GUIMACRO_SUCCEEDED = CERR_FIRSTEXITCODE+33,
	CERR_GUIMACRO_FAILED = CERR_FIRSTEXITCODE+34,
	CERR_SERVER_ALREADY_EXISTS = CERR_FIRSTEXITCODE+35,
	CERR_RUNNEWCONSOLEFAILED = CERR_FIRSTEXITCODE+36,
	CERR_ATTACH_NO_GUIWND = CERR_FIRSTEXITCODE+37,
	CERR_SRVLOADFAILED = CERR_FIRSTEXITCODE+38,
	CERR_GUI_NOT_FOUND = CERR_FIRSTEXITCODE+39,
	CERR_MAINSRV_NOT_FOUND = CERR_FIRSTEXITCODE+40,
	CERR_UNICODE_CHK_FAILED = CERR_FIRSTEXITCODE+41,
	CERR_UNICODE_CHK_OKAY = CERR_FIRSTEXITCODE+42,
	CERR_AUTOATTACH_NOT_ALLOWED = CERR_FIRSTEXITCODE+43,
	CERR_HOOKS_WAS_ALREADY_SET = CERR_FIRSTEXITCODE+44,
	CERR_DOWNLOAD_FAILED = CERR_FIRSTEXITCODE+45,
	CERR_DOWNLOAD_SUCCEEDED = CERR_FIRSTEXITCODE+46,
	CERR_WRONG_GUI_VERSION = CERR_FIRSTEXITCODE+47,
	CERR_LAST = CERR_FIRSTEXITCODE+100
	;

#define CERR_LASTEXITCODE (CERR_FIRSTEXITCODE+100)

// ServerInit | DoInjectRemote -> InjectRemote -> PrepareHookModule | [InfiltrateDll -> CreateRemoteThread]
typedef int CINFILTRATE_EXIT_CODES;
const CINFILTRATE_EXIT_CODES
	CIR_GeneralError = -1,
	// InfiltrateDll codes
	CIR_WrongBitness = -100,
	CIR_TooLongHookPath = -101,
	CIR_VirtualAllocEx = -102,
	CIR_WriteProcessMemory = -103,
	CIR_LoadKernel = -104,
	CIR_NoKernelExport = -105,
	CIR_VirtualAllocEx2 = -106,
	CIR_WriteProcessMemory2 = -107,
	CIR_CreateRemoteThread = -108,
	CIR_ReadProcessMemory = -109,
	CIR_InInjectedCodeError = -100,
	CIR_CheckKernelExportAddr = -111,
	CIR_OuterKernelAddr = -112,
	CIR_SnapshotCantBeOpened = -113,
	CIR_InfiltrateGeneral = -150,
	//
	CIR_GetModuleFileName = -200,
	// Different bitness, running matching ConEmuC[64]
	CIR_OpenProcess = -201,
	CIR_CreateProcess = -202,
	CIR_WrapperResult = -203,
	CIR_GetProcessBits = -204,
	//
	CIR_ConEmuHkNotFound = -250,
	// Prepare hook module (copy proper ConEmuHk[64] to %TEMP%)
	CIR_SHGetFolderPath = -251,
	CIR_TooLongTempPath = -252,
	CIR_CreateTempDirectory = -253,
	CIR_CopyHooksFile = -254,
	// Waiting thread result?
	CIR_DefTermWaitingFailed = -300,
	// Succeeded
	CIR_AlreadyInjected = 1,
	CIR_OK = 0
	;

// ConsoleMain2 | DoInjectHooks | OnCreateProcessFinished -> InjectHooks
typedef int CINJECTHK_EXIT_CODES;
const CINJECTHK_EXIT_CODES
	CIH_GeneralError = -1,
	//
	CIH_GetModuleFileName = -501,
	CIH_CreateProcess = -502,
	CIH_GetLoadLibraryAddress = -503,
	CIH_WrapperGeneral = -504,
	CIH_WrapperFailed = -505,
	CIH_ProcessWasTerminated = -506,
	CIH_WrongHandleBitness = -509,
	CIH_KernelNotLoaded = -510,
	CIH_OsVerFailed = -511,
	CIH_NtdllNotLoaded = -512,
	CIH_GetLdrHandleAddress = -514,
	//
	CIH_AsmBadCodePointer = -601,
	CIH_AsmMemBadSize = -602,
	CIH_AsmVirtualAllocEx = -703,
	CIH_AsmGetThreadContext = -710,
	CIH_AsmWriteProcessMemory = -730,
	CIH_AsmFlushInstructionCode = -731,
	CIH_AsmSetThreadContext = -732,
	CIH_AsmBitnessNot64 = -801,
	CIH_AsmBitmessNot32 = -802,
	CIH_AsmBadDllPathName = -803,
	CIH_AsmBadProcShift = -814,
	CIH_AsmGeneralError = -1000,
	//
	CIH_OK = 0
	;
