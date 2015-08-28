﻿‘Memory dump’ (or ‘Mini dump’, or ‘Crash dump’) is a binary file, containing full or brief information about program state at specified moment.
Usually, files has `.dmp` or `.mdmp` extension.

Following information is a copy of answers from [ConEmu FAQ](http://conemu.github.io/en/ConEmuFAQ.html).

# How to create a minidump #

There are several ways to create memory dumps.

## Process Explorer ##
[Process Explorer](ProcessExplorer.md) can create proper ‘Memory dumps’.
But be sure, you are using latest ‘Process Explorer’ version (15.40 is OK now).

Just locate process in the list, right click on it and choose ‘Create dump’ -> Mini or Full.

## ConEmu System menu ##
ConEmu GUI offers easy creation of ‘ActiveProcess memory dump’. Just choose menu item
from SystemMenu -> Debug.

**Note** ‘Active Process’ is **console** process or ChildGui application, but not a `ConEmu[64].exe` or `ConEmuC[64].exe`.

## Using ConEmuC command line ##
**Windows XP and above**

Press **Win+R** and run the following command, it will create a full process memory dump (it may be large enough).
```
"C:\Program Files\ConEmu\ConEmu\ConEmuC.exe" /DEBUGPID=Your_PID /FULL
```

Or, create a small process memory dump. Yes, it will be small, but may contains not enough information for detecting a problem.
```
"C:\Program Files\ConEmu\ConEmu\ConEmuC.exe" /DEBUGPID=Your_PID /MINI
```

Substitute _Your\_PID_ with the process ID, [read below](MiniDump#How_to_find_Process_ID_(PID).md).
You will be offered to choose filename for a minidump.

_**Warning!**_ Before creating dumps of Far Manager please ensure that "far.exe" was started with "/x" switch.

## Using Windows Task manager ##
**Windows 7 and up**

_Warning! This method only works for processes with the same bitness as the operating system.
Therefore, you won't be able to create a working minidump for 32-bit processes when using a 64-bit Windows version._

Open Windows' Task Manager, switch to the "Processes" tab, right-click the process, and select "Create dump file".

# How to find Process ID (PID) #
A. You can find the Process ID near to Process Name (ConEmu.exe, cmd.exe, etc.)
  * in Windows' Task Manager;
  * in the ConEmu status line;
  * in the Info page of ConEmu [Settings](Settings#Info.md) dialog.

**Note!** Sometimes PID column in the Task Manager window may be hidden, enable it in Task manager settings.
  * Windows 7 and below: Task Manager -> Processes tab -> Menu -> View -> Select Columns -> Check ‘PID’.
  * Windows 8: Task Manager -> Show ‘More details’ -> Processes tab -> Right click on column title -> Check ‘PID’.