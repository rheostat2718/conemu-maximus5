﻿=Migration to [conemu.github.io](http://conemu.github.io/)=
Google has declared [shutdown](http://google-opensource.blogspot.ru/2015/03/farewell-to-google-code.html) of googlecode service.

New versions of this file will be available on [conemu.github.io](http://conemu.github.io/en/ConEmuC.html).


---


This is console part of ConEmu. It serves all console requests.

There are two versions - ConEmuC.exe and ConEmuC64.exe.

In the 32-bit operation systems only ConEmuC.exe (and ConEmuHk.dll library) is used.

64-bit systems are more complicated, so both versions must exists in the ConEmu folder.
ConEmuC.exe is used, when You run 32bit applications from started console,
ConEmuC64.exe (and ConEmuHk64.dll) for 64bit applications.

Root console process will be ConEmuC.exe, when You are using ConEmu.exe,
and ConEmuC64.exe, when You are using ConEmu64.exe.

### Font in real console ###
ConEmu use small _Lucida console_ font in real console by default,
cause of this font is shipped with Windows and it is unicode
(unicode font must be selected in real console to allow ConEmu
retrieve unicode characters from real console).

You may specify font face name and size to the real console.

**Warning**.
  * This font must be preinstalled in the system.
  * It **must** be **unicode** (means TTF).
  * It **must** be smaller than main font (selected for ConEmu).

### ConEmuC.exe command line switches ###

**Note** From build 140106 you may use '-'style switches with ConEmuC too,
for example, following commands acts the same.
```
ConEmuC /IsConEmu
ConEmuC -IsConEmu
```

**Show help screen**
```
ConEmuC /?
```

**Useful checks**
```
ConEmuC /IsConEmu
    returns 1 as errorlevel if running in ConEmu tab, 2 if not
ConEmuC /IsAnsi
    returns 1 as errorlevel if ANSI are processed, 2 if not
ConEmuC /IsTerm
    returns 1 as errorlevel if running in telnet, 2 if not
```

**Attach consoles to ConEmu**
```
ConEmuC /AUTOATTACH [/GHWND=NEW|<HWND>]
    asynchronous attach to ConEmu GUI (for batches)
    always returns 0 as errorlevel on exit (Issue 1003)
ConEmuC /ATTACH /NOCMD [/GHWND=NEW|<HWND>]
    asynchronous attach current (existing) console to ConEmu
ConEmuC /ATTACH [/GHWND=NEW|<HWND>] /[FAR|CON|TRM]PID=<PID>
    synchronous attach specified console process to ConEmu
  Switches
    /GHWND        - you may force new ConEmu window or attach to specific one
    /PID=<PID>    - use <PID> as root process
    /FARPID=<PID> - for internal use from Far plugin
    /CONPID=<PID> - 'soft' mode, don't inject ConEmuHk into <PID>
    /TRMPID=<PID> - called from *.vshost.exe when 'AllocConsole' just created
```

**[ConEmu GUI macro command](GuiMacro.md)**
```
ConEmuC /GUIMACRO Function([Arg1[,Arg2[,...]])
```

**Echo and Type**
These two was implemented for test purposed mostly. But may be useful.
Use any of `"-e"`, `"-echo"`, `"/echo"` to echo.
```
ConEmuC -e [-r] [-n] [-b] "String to echo"
  Switches
    -r  - do not replace "^e^r^n^t^a^b" with ASCII equivalents
          to print single "^" char - double it "^^"
          escape char alias - "^["
    -n  - do not add CRLF after printed line
    -b  - scroll to bottom of the buffer before printing (TrueColor buffer compatible)
```
Use any of `"-t"`, `"-type"`, `"/type"` to type text file.
```
ConEmuC -t [-b] [-CP] "Path to text file to echo"
  Switches
    -b  - scroll to bottom of the buffer before printing (TrueColor buffer compatible)
    -CP - consider that text file code page is "CP"
          ConEmuC -t -65001 "utf8-file.txt"
          Without specified code page BOM may be used for utf-8 and CP1200
          Otherwise the file will be threated as ANSI
```
Both commands are able to process [ANSI sequences](http://conemu.github.io/en/AnsiEscapeCodes.html).
But take into account, that `"ConEmuC -t ..."` does not do replacements with ASCII equivalents.
```
ConEmuC -e "^[[1;33;45mqwerty^[[m"
```

**Debug and MiniDump**
```
ConEmuC /DEBUGPID=<Debugging PID> [/DUMP | /MINI | /FULL]
    start debugger or create memory dump
ConEmuC /DEBUGEXE <command line>
    start <command line> under debugger
ConEmuC /DEBUGTREE <command line>
    start <command line> under debugger, debug all children
```

**[Export environment variables](ConEmuEnvironment#Export_variables.md) to the parent processes**
```
ConEmuC /EXPORT[=CON|ALL] [Var1 [Var2 [...]]]
```

**Download from http/https/ftp**
```
ConEmuC /download [-login <name> -password <pwd>]
    [-proxy <address:port> [-proxylogin <name> -proxypassword <pwd>]]
    [-async Y|N] [-otimeout <ms>] [-timeout <ms>]
    "full_url_to_file" "local_path_name"
```

**Execute commands and create tabs (Internal use!)**
```
ConEmuC [switches] /ROOT <program with arguments, far.exe for example>
ConEmuC [switches] [/U | /A] [/Async | /Fork] /C <command line>
  Switches
    /[NO]CONFIRM - [don't] confirm closing console on program termination
    /B{W|H|Z}    - define window width, height and buffer height
    /F{N|W|H}    - define console font name, width, height
    /LOG[N]      - create (debug) log file, N is number from 0 to 3
```

**Show errorlevel of `<command>`**
```
ConEmuC /Result /C <command>
```

**Return errorlevel = `<number>`**
```
ConEmuC /ErrorLevel <number>
```

### -new\_console and -cur\_console switches ###
When you run application from ConEmu console, you may use **[-new\_console](NewConsole.md)** or **[-cur\_console](NewConsole.md)** switches.

Btw, this is one of the ways to start **GUI** application in ConEmu tab.

**Note** These are NOT a switches of ConEmuC, specify them as application (far, vim, putty, etc.) switches.

**Warning** Option 'Inject ConEmuHk' must be enabled in ConEmu settings!

**Examples**
```
  dir "-new_console:bh9999c" c:\ /s
  vim.exe -new_console:nh0 c:\sources\1.cpp
  hiew.exe -cur_console:h0 c:\tools\far.exe
```