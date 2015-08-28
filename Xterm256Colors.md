=How to enable xterm 256 colors in ConEmu=

Note. If ‘Inject ConEmuHk’ is off, you can still use ANSI in vim (or any other application)
if you run them via ConEmuC.exe but not directly. For example, run vim from bash
```
conemuc //c C:\\GIT\\share\\vim\\vim73\\vim.exe 1.cpp
```
Of course, you may omit full path to vim.exe if it may be found in %PATH% environment variable.

Note. ConEmu will disable scroll buffer automatically, when vim.exe is started.
For any other application - you must do it yourself.

## xterm 256 color mode requirements ##
  * Options must be turned on
    * ‘TrueMod (24bit color) support’ on [Colors](Settings#Colors.md) page
    * ‘ANSI X3.64 / xterm 256 colors’ on [Features](Settings#Features.md) page
    * ‘Inject ConEmuHk’ on [Features](Settings#Features.md) page (required for second level programs)
  * Turn off scrolling (extended attributes works only in the ‘work’ area - the bottom of the console)

### Example 1: Vim ###
```
vim.exe -cur_console:h0 <Vim arguments here>
```

### Example 2: 256colors2.pl ###
Script [256colors2.pl](http://www.frexx.de/xterm-256-notes/data/256colors2.pl) must be executed as:
```
256colors2.pl -cur_console:h0
```

### Example 3: scroll console to the bottom ###
When you run **not** ‘fullscreen’ application (not Far/Vim/Hiew/...),
you may scroll console to the bottom (‘^[’ must be replaced with real ESC symbol, ASCII code \x1B):
```
echo ^[[9999;1H
```

## TechInfo ##
ANSI escape sequences are processed when console program uses functions WriteConsoleA, WriteConsoleW или WriteFile. Например:
```
cmd /c type "Colors-256.ans"
```
Text output with current extended attributes (xterm 256 color) is possible with
WriteConsoleOutputCharacterA and WriteConsoleOutputCharacterW functions.