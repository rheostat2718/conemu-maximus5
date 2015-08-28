### Why they are needed ###

When user reports a problem, frequently, developer needs more information than user may tell. ConEmu may write internal information in text form into log files.

### How to start logging ###

Run `ConEmu.exe` or `ConEmu64.exe` (whatever you are using) with special switch `/log`. How to do that? Simpliest way is to press **Win+R**, type (or browse) in edit field full path to `ConEmu.exe` or `ConEmu64.exe` and append `/log` switch. There are also `/log2`, `/log3` and `/log4` alternatives. Larger digit - more information will be written - ConEmu will run slower.

### Where log files are created ###

This depends of when program folder is write-allowed. For example, running as normal user any program can't write to `C:\Program Files`.

**If program folder is locked**, ConEmu will create on your desktop new folder named **ConEmuLogs** and write information to `ConEmu*.log` files there.

**If program folder is write-allowed**, ConEmu will create `ConEmu*.log` files near to executable file. Note, that there are `ConEmuC.exe` and `ConEmuC64.exe` in the `ConEmu` subfolder.