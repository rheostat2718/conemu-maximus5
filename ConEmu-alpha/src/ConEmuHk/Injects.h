﻿
#pragma once

extern UINT_PTR gfnLoadLibrary;
extern UINT_PTR gfnLdrGetDllHandleByName;

CINJECTHK_EXIT_CODES InjectHooks(PROCESS_INFORMATION pi, BOOL abLogProcess);
UINT_PTR GetLoadLibraryAddress();
UINT_PTR GetLdrGetDllHandleByNameAddress();
