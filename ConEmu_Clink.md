﻿=ConEmu and clink==
If you are using `cmd.exe` as shell you may try [clink](http://mridgers.github.io/clink/) (bash style completion and advanced cmd prompt).

  * Download latest [clink binaries](http://mridgers.github.io/clink/) or build it yourself from [sources](https://github.com/mridgers/clink#building-clink).
  * Unpack **files** (without subfolder) here: [%ConEmuBaseDir%\clink](https://github.com/Maximus5/ConEmu/tree/master/Release/ConEmu/clink) (this folder already created by ConEmu installer and contains `Readme.txt`)
  * Please note, clink archive contains subfolder. Files `clink_dll_x86.dll`, `clink_dll_x64.dll` (and others) must be located in `%ConEmuBaseDir%\clink`.
  * Turn on checkbox ‘Use Clink in prompt’ on ‘Features’ page, save settings and restart ConEmu.

OK, now bash style completion will be available in `cmd.exe` new session.

Note. Clink will be called in shell prompt (cmd) if `ConEmuHk.dll` or `ConEmuHk64.dll`
is loaded into shell process. This will be done for ‘root’ processes (that you specify when creating
console) or when ‘Inject ConEmuHk’ option is checked on ‘Features’ page (this is required, if you
start second-level cmd from root process, e.g. `cmd /c cmd /k …`).

Note. If you do not want to store clink in the ConEmu's subfolder, you may install it standalone
using `clink_setup.exe`. Actually, option ‘Use Clink in prompt’ is not required if clink was set
up using its installer.

Please refer to https://github.com/mridgers/clink for further information and bugreports.