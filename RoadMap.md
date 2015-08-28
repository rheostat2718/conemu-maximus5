﻿=ConEmu has moved to conemu.github.io=

[New official ConEmu website](http://conemu.github.io/)

<font size='5'><b>ConEmu road map</b></font>

Any help will be very much appreciated!



## Documentation ##
Unfortunately, I don't have enough time for maintaining documentation and screenshots.
Also, my english is not very well :)

## Optimization ##
Some portions of code must be optimized or totally rewritten :(

## Portable registry ##
ConEmu can store itself settings in xml file already.
However, it is possible to hook registry functions, so any (console or GUI) application,
started in ConEmu tab become portable.

This portion of code exists but not perfect. So, it is disabled in release versions.

## Tab groups and themes ##
[Issue 294](https://code.google.com/p/conemu-maximus5/issues/detail?id=294), [Issue 194](https://code.google.com/p/conemu-maximus5/issues/detail?id=194).
  * Tabs and toolbar themes
  * Tabs reordering
  * Tab groups (multiple visible consoles)
  * Custom icons (icon of active process) in tabs

## Advertising ##
I hope, ConEmu is stable enough and competitive with other console emulators.

## Virtual console buffer ##
In the current version, maximum buffer height (output history) is limited with 9999 lines (Windows console limit).

I want to eliminate this limitation :)

Also, restricting real console buffer to visible rect only greatly increase speed of command execution
(e.g. ‘`dir /s c:\windows`’).

## Horizontal scrolling ##
Some programs use not only vertical, but horizontal console buffer. I think, support of it will be useful.

## Far Manager related ##
### Icons in standard Far panels ###
Thumbnails and Tiles already works, but they are ConEmu windows which overlaps standard panels.

## Configurable panels and toolbars ##
Something like what is in TCMD or TC.

## RTL languages ##
RTL is beyond my comprehension :) If RTL users think ConEmu may be better - let me know.

## History in cmd and powershell ##
Some users asks about any way to get it to remember history from previous sessions.
This is complicated (but possible). For now, I can recommend:
  * Far Manager as framework for cmd.exe: http://stackoverflow.com/a/10921470/1405560
  * [Clink](ConEmu_Clink.md) for bash-style completion in cmd.exe.