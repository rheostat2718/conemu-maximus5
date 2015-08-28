﻿#summary Run your shell naked

In some cases third party software or plugins may affect normal behaviour of your shell.
And that may be **not** a ConEmu issue at all.

Most of shells have switches to disable loading plugins or extensions.
Just use them. Easiest way to use "ConEmu /cmd ..." to force proper shell command.
For example:
```
ConEmu /cmd PowerShell -noprofile
```
Alternatively, you may change the ‘Command line’ on the ‘Startup’ settings page.

## cmd ##
Just run cmd using "/D" switch.
```
cmd /D
```

## PowerShell ##
Use "-noprofile" switch.
```
PowerShell -noprofile
```

## Far Manager ##
Use "/p" to disable plugins and "/m" to disable macros.
```
Far /p /m
```