﻿[Download (Far3 x86/x64)](http://plugring.farmanager.com/plugin.php?pid=948)

### About ###
Resolve plugin for Far Manager 2.x and 3.x

This plugin may be useful for developers, performs two functions:

1. 'Error lookup' analog - for Windows error code shows name, description, severity/facility, and so on...
```
╔══════════════════ CPP Resolve ══════════════════╗
║                                                 ║
║ Expr:   0xC0000022                             ↓║
║ Source: ntdll.dll                              ↓║
║                       [ Resolve ]  [ ErrLook ]  ║
╟─ ╔════════════════ Resolve ═════════════════╗ ──╢
║  ║ Name=STATUS_ACCESS_DENIED                ║   ║
║ T║ Err=0xC0000022, Dec=-1073741790, Word=34 ║   ║
║ V║ SEVERITY_ERROR, FACILITY_NULL            ║   ║
║  ║                                          ║   ║
╚══║ {Access Denied}                          ║═══╝
   ║ A process has requested access to an ob… ║                                                               
   ║                                          ║
   ╟──────────────────────────────────────────╢
   ║                  { OK }                  ║
   ╚══════════════════════════════════════════╝
```

2. 'Resolve' - show Windows API macro value, i.e. values of WM\_SIZE, CFSTR\_FILECONTENTS and so on...
```
╔══════════════════ CPP Resolve ══════════════════╗
║                                                 ║
║ Expr:   CFSTR_FILECONTENTS                     ↓║
║ Source:                                        ↓║
║                       [ Resolve ]  [ ErrLook ]  ║
╟─────────────────────────────────────────────────╢
║                                                 ║
║ Type:   char const [13]                         ║
║ Value:  FileContents                            ║
║         [ 1 Copy ]  [ 2 Copy # ]  [ 3 Copy /* ] ║
╚═════════════════════════════════════════════════╝
```

Plugin defaults assumes Visual Studio 9.0 and SDK 7.0.

All PATH`s, INCLUDE`s and LIB`s may be changes in plugin settings.

'Resolve' function requires 'cl.exe' and 'link.exe'.

By Enter depress plugin choose function (Resolve/ErrLook) automatically.