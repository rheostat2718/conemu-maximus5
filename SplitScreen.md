﻿=ConEmu has moved to conemu.github.io=
New versions of this file are available on [conemu.github.io](http://conemu.github.io/en/SplitScreen.html).

## Splits or panes ##
ConEmu may split any tab into several panes with free (user choice) grid configuration.
Unlike many other terminals (who can start in the split only the same shell as original one), you may
  * create new pane (split) with any shell of your choice, with different credentials or elevated (as Administrator);
  * or just duplicate shell from your active pane (classic way) with Ctrl+Shift+O or Ctrl+Shift+E.

## Ways to split ##
As usual, ConEmu provides more than a one way to do any action.

### SplitScreen options in ‘Create new console’ dialog ###
Open ‘Create new console’ dialog by pressing Win+W or `[+]` toolbar button
(note, if you have disabled ‘Create confirmation’ press Win+Shift+W or Shift+`[+]`)
and set up ‘New console split’ options.

### From your shell prompt ###
You may use `-new_console:s[<SplitTab>T][<Percents>](H|V)` switch appended to any command typed in your prompt.
Option ‘Inject ConEmuHk’ must be enabled to use this feature. Examples:
  * `cmd -new_console:s` - split current pane, run `cmd` shell, new pane and old (current) pane become 50% width of current pane.
  * `cmd -new_console:s50H` - same as `cmd -new_console:s`.
  * `powershell -NoProfile -new_console:sV` - split current pane, run `powershell -NoProfile`, create new pane to the bottom.
  * `sh --login -i -new_console:s3T30H` - split 3-d pane, create new pane to the right with 30% width.

### Default hotkeys ###
Split key macros (default as Ctrl+Shift+O & Ctrl+Shift+E) works now like ‘Duplicate root...’ menu item.
These macros will duplicate shell from your active pane!

### Start several consoles in 2x2 grid from the named Task ###
The question from [superuser.com](http://superuser.com/q/473807/139371).
ConEmu (build 120909 or higher recommended) provides SplitScreen feature.
You may set up named task to open several consoles on startup in the grid. Here the example for 2x2 grid.

```
>cmd -cur_console:n
powershell -cur_console:s1TVn
sh --login -i -cur_console:s1THn
putty -load mysrv -cur_console:s2THn
```


## Working with splits ##
  * Cycle switche visible split-panes: Apps+Tab, Apps+Shift+Tab {GuiMacro: Tab(10,...)}.
  * Put focus to nearest pane: Apps+Up, Apps+Down, Apps+Left, Apps+Right {GuiMacro: Split(2,...)}.
  * Move splitter (resize panes): Apps+Shift+Up, Apps+Shift+Down, Apps+Shift+Left, Apps+Shift+Right {GuiMacro: Split(1,...)}.

Wondering what is the ‘Apps’ key? This key usually located between RightWin and RightCtrl, defined as `VK_APPS` in `WinUser.h` ;)