﻿=ConEmu has moved to conemu.github.io=
New versions of this file are available on [conemu.github.io](http://conemu.github.io/en/CygwinHeap.html).

# Problems with Cygwin heap #
When you run any application compiled with cygwin or msys (that is
statically linked with cygwin1.dll or msys???.dll) you may,
sometimes, notice weird error in application console output, and it
will immediately exits after that.

```
0 [main] us 0 init_cheap: VirtualAlloc pointer is null, Win32 error 487 AllocationBase 0x68520000, BaseAddress 0x68570000, RegionSize 0x218000, State 0x1000 C:\Program Files (x86)\Git\bin\ls.exe: *** Couldn't reserve space for cygwin's heap, Win32 error 487
```

That is known cygwin problem mentioned in their FAQ (link?)

And what does that error means? Cygwin (or msys) initialization
routine tries to allocate memory (relatively large amount) at the
fixed address. And, yep, it is already used by some dll!

Is that a ConEmu problem?

No!

1) ConEmu is aware about cygwin and msys requirements and ConEmuHk compiled with link options ask Windows kernel to load ConEmuHk.dll (or ConEmuHk64.dll) in the memory address which must not has conflicts with cygwin or msys.
2) There is no one bug report which confirm conflict between ConEmu and cygwin/msys.

# How to fix that? #
I really can't help you because it is not a ConEmu bug...
But you may fix it yourself in your PC.
1) First of all, try to logoff/logon. If same here - try to restart your PC.
2) Same here? That means you must find problematic dll. Use ConEmuReportExe environment variable to stop execution of your application (sh.exe, ls.exe, and so on). And when message box arrears "sh.exe loaded" use ProcessMonitor to find dll which was loaded immediately after cygwin1.dll or msys???.dll. Press OK in the waiting message box and confirm that "failed" allocation memory address overlaps with dll your found. From my experience, that can be "[apphelp.dll](AppHelp.md)". When you find bad gay:
a) You may to PATCH this dll. Use rebase tool from Windows sdk for example. What new base address to choose? I can't tell you. Ideally, that address must not overlaps with any other libraries... but there are thousands... Good thing - must of them can be linked with dynamic base, bad thing - off you choose bad address, your system will be unstable (error code 0xC???).
b) And, if it possible, you may install the program which had installed that dll
c) Also, may be that program allows to set up exclusions. If you are working in the ConEmu consoles only, you may add to exclusions list only ConEmu's executables (ConEmuC.exe, ConEmuC64.exe, ConEmu.exe, ConEmu64.exe). Otherwise, you need to add to exclusions list all of your cygwin/msys applications (sh, ls, and on).

# Still not sure? #
Well, submit your report with MiniDump attached. All reports will be checked.

# How to create required dump #

Run your cmd prompt
set ConEmuReportExe=ls.exe
ls

Message box must appears "ls.exe loaded"
At this time, call ConEmu system menu > Debug > Active process memory dump
Attach here MiniDump (full dump is not required)

After pressing OK in "loaded" message box your error message expected.