﻿[Download (Far3 x86/x64)](http://code.google.com/p/conemu-maximus5/downloads/list?can=2&q=QSearchSwitch)

### About ###
QSearchSwitch plugin for Far Manager 3.x

Some users want to QSearch in panels without holding Alt key.

This mini-plugin invert Alt behaviour, when you type letters QSearch appears.

If you want to type in Far CommandLine - press Alt+‘letter’. After first symbol (when CommandLine is not empty)
you may release Alt.

### Installation ###
  * Unpack to `%FARHOME%\Plugins\QSearchSwitch` folder
  * Restart Far Manager

### Requirements ###
  * Far Manager 3.0 build 2630 bis or higher
  * Far3bis required cause of http://bugs.farmanager.com/view.php?id=1687

### Configuration ###
Use Plugin.Call("9d5d070e-42f3-4a10-82e3-b3c74830705b",Cmd) to control plugin, where Cmd is bitmask
```
  0x01 - switch (enable/disable)
  0x02 - enable
  0x04 - disable
  0x10 - save plugin settings
```

Look for example in `EnableDisable.farconfig`, import this file with command:
```
far /import EnableDisable.farconfig
```