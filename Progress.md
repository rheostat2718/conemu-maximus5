﻿ConEmu use simple AI to detect operation progress by title bar or console text. So, You'll see current operation status of many programs, such as FAR, wget, nerocmd, 7-zip, chkdsk, and so on...

Operation progress, detected in any console (active console is preferred) is shown in window title.
```
{35%} C:\Arch\7z.exe a ...
```
ConEmu prepend percentage value with asterisk, when operation runs in background (inactive console) and active console have no operation.
```
{*35%} C:\Arch\7z.exe a ...
```

**Progress on Windows7 taskbar** <br>
<img src='http://conemu-maximus5.googlecode.com/svn/files/ConEmuProgress.png' alt='ConEmu progress in Windows7' title='ConEmu progress in Windows7'>