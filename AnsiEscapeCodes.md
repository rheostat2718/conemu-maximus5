﻿=Migration to [conemu.github.io](http://conemu.github.io/)=
Google has declared [shutdown](http://google-opensource.blogspot.ru/2015/03/farewell-to-google-code.html) of googlecode service.

New versions of this file will be available on [conemu.github.io](http://conemu.github.io/en/AnsiEscapeCodes.html).

<font size='5'><b>ANSI X3.64 and Xterm 256 colors</b></font>

ConEmu (from build 120520d) can process [ANSI X3.64](http://en.wikipedia.org/wiki/ANSI_X3.64)
and its extension [xterm 256 color mode](http://www.frexx.de/xterm-256-notes/).



# Description #
New option ‘ANSI X3.64 / xterm 256 colors’ on the ‘Features’ page, turned On by default.
It works with full console (including scrolling area - BufferHeight)
but xterm 256 color affects only on ‘working’ area (this is bottom part of console, if scrolling exists).
Outside (upper area) 256 colors will be approximated to console stanard 16-colors.

## ANSI sequences processing requirements ##
  * These checkboxes must be **On**
    * ‘ANSI X3.64 / xterm 256 colors’ on the [Features](Settings#Features.md) page
    * ‘Inject ConEmuHk’ on the [Features](Settings#Features.md) page (required for 2-nd level programs, aka started from your shell)

## xterm 256 color processing requirements ##
  * These checkboxes must be **On**
    * ‘TrueMod (24bit color) support’ on the [Colors](Settings#Colors.md) page
    * ‘ANSI X3.64 / xterm 256 colors’ on the [Features](Settings#Features.md) page
    * ‘Inject ConEmuHk’ on the [Features](Settings#Features.md) page (required for 2-nd level programs, aka started from your shell)
  * You need to ensure that buffer/scrolling is Off

### Example 1: Vim ###
```
vim.exe -cur_console:h0 <Vim arguments here>
```

### Example 2: 256colors2.pl ###
Perl script [256colors2.pl](http://www.frexx.de/xterm-256-notes/data/256colors2.pl) need to be runned as following:
```
256colors2.pl -cur_console:h0
```

### Example 3: scroll console to bottom ###
You may scroll console to the bottom (activate working area):
```
echo ^[[9999;1H
```
**Warning** You need to change ‘^[’ to ESC code before using this script (char with code \x1B).

## TechInfo ##
ANSI escape sequences will be processed when console application output text using
Windows API functions WriteConsoleA, WriteConsoleW or WriteFile. For example:
```
cmd /c type "Colors-256.ans"
```
Also, output with extended attributes (xterm 256 color) is available with functions
WriteConsoleOutputCharacterA and WriteConsoleOutputCharacterW.

## Environment variable ##
How can I check in cmd-file if ANSI x3.64 is supported and enabled?
You need to check ‘ConEmuANSI’ environment variable
```
if "%ConEmuANSI%"=="ON"  echo Enabled
if "%ConEmuANSI%"=="OFF" echo Disabled
```

# List of supported codes #
## CSI (Control Sequence Initiator) codes ##
| ESC [ _lines_ A | Moves cursor up by _lines_ lines (1 by default) |
|:----------------|:------------------------------------------------|
| ESC [ _lines_ B | Moves cursor down by _lines_ lines (1 by default) |
| ESC [ _cols_ C  | Moves cursor rightward by _cols_ columns (1 by default) |
| ESC [ _cols_ D  | Moves cursor leftward by _cols_ columns (1 by default) |
| ESC [ _lines_ E | Moves cursor to beginning of the line, _lines_ (default 1) lines down. |
| ESC [ _lines_ F | Moves cursor to beginning of the line, _lines_ (default 1) lines up. |
| ESC [ _col_ G   | Moves the cursor to column _col_ (absolute, 1-based). |
| ESC [ _row_ ; _col_ H | Set cursor position. The values _row_ and _col_ are 1-based. |
| ESC [ _n_ J     | Erase display. When _n_ is 0 or missing: from cursor to end of display). When _n_ is 1: erase from start to cursor. When _n_ is 2: erase whole display **and** moves cursor to upper-left corner. |
| ESC [ _n_ K     | Erase line. When _n_ is 0 or missing: from cursor to end of line. When _n_ is 1: erase from start of line to cursor. When _n_ is 2: erase whole line **and** moves cursor to first column. |
| ESC [ _n_ L     | Insert _n_ (default 1) lines before current, scroll part of screen from current line to bottom. |
| ESC [ _n_ M     | Delete _n_ (default 1) lines including current. |
| ESC [ _lines_ S | Scroll screen (whole buffer) up by _lines_. New lines are added at the bottom. |
| ESC [ _lines_ T | Scroll screen (whole buffer) down by _lines_. New lines are added at the top. |
| ESC [ _n_ X     | Erase _n_ (default 1) characters from cursor (fill with spaces and default attributes). |
| ESC [ `>` c     | Report `"ESC > 67 ; build ; 0 c"`               |
| ESC [ c         | Report `"ESC [ ? 1 ; 2 c"`                      |
| ESC [ _row_ ; _col_ f | Set cursor position (same as H). The values _row_ and _col_ are 1-based. |
| ESC [ _a_ ; _b_ h | Set mode ([see below](AnsiEscapeCodes#Terminal_modes.md)). |
| ESC [ _a_ ; _b_ l | Reset mode ([see below](AnsiEscapeCodes#Terminal_modes.md)). |
| ESC [ _a_ ; _b_ ; _c_ m | Set SGR attributes ([see below](AnsiEscapeCodes#SGR_(Select_Graphic_Rendition)_parameters.md)). |
| ESC [ 5 n       | Report status as "`CSI 0 n`" (OK)               |
| ESC [ 6 n       | Report Cursor Position as `"ESC [ row ; col R"` |
| ESC [ _a_ ; _b_ r | Set scrolling region from top=_a_ to bottom=_b_. The values _a_ and _b_ are 1-based. Omit values to reset region. |
| ESC [ s         | Save cursor position (can not be nested).       |
| ESC [ 1 8 t     | Report the size of the text area in characters as `"ESC [ 8 ; height ; width t"` |
| ESC [ 1 9 t     | Report the size of the screen in characters as `"ESC [ 9 ; height ; width t"` |
| ESC [ 2 1 t     | Report window’s title as `"ESC ] l title ESC \"` |
| ESC [ u         | Restore cursor position.                        |

### Terminal modes ###
| ESC [ 7 ; _col_ h | Enables line wrapping at column position. If _col_ (1-based) is absent, wrap at column 80. |
|:------------------|:-------------------------------------------------------------------------------------------|
| ESC [ 7 l         | Disables line wrapping. Lines wraps at the end of screen buffer.                           |
| ESC [ 25 h        | Show text cursor.                                                                          |
| ESC [ 25 l        | Hide text cursor.                                                                          |

### SGR (Select Graphic Rendition) parameters ###
| ESC [ 0 m | Reset current attributes |
|:----------|:-------------------------|
| ESC [ 1 m | Set BrightOrBold         |
| ESC [ 2 m | Unset BrightOrBold       |
| ESC [ 3 m | Set ItalicOrInverse      |
| ESC [ 4 m | Set BackOrUnderline      |
| ESC [ 5 m | Set BackOrUnderline      |
| ESC [ 22 m | Unset BrightOrBold       |
| ESC [ 23 m | Unset ItalicOrInverse    |
| ESC [ 24 m | Unset BackOrUnderline    |
| ESC [ 30…37 m | Set ANSI text color      |
| ESC [ 38 ; 5 ; _n_ m | Set xterm text color, _n_ is color index from 0 to 255 |
| ESC [ 38 ; 2 ; _r_ ; _g_ ; _b_ m | Set xterm 24-bit text color, _r_, _g_, _b_ are from 0 to 255 |
| ESC [ 39 m | Reset text color to defauls |
| ESC [ 40…47 m | Set ANSI background color |
| ESC [ 48 ; 5 ; _n_ m | Set xterm background color, _n_ is color index from 0 to 255 |
| ESC [ 48 ; 2 ; _r_ ; _g_ ; _b_ m | Set xterm 24-bit background color, _r_, _g_, _b_ are from 0 to 255 |
| ESC [ 49 m | Reset background color to defauls |
| ESC [ 30…37 m | Set ANSI text color      |

## OSC (Operating system commands) ##
**Note**. These codes may ends with ‘ESC\’ (two symbols - ESC and BackSlash) or ‘BELL’ (symbol with code \x07, same as ‘^a’ in `*`nix).
For simplifying, endings in the following table marked as ‘ST’.

| ESC ] 2 ; "_txt_" ST | Set console window title to _txt_. |
|:---------------------|:-----------------------------------|

### ConEmu specific OSC ###
| ESC ] 9 ; 1 ; _ms_ ST | Sleep. _ms_ - number, milliseconds. |
|:----------------------|:------------------------------------|
| ESC ] 9 ; 2 ; "_txt_" ST | Show GUI MessageBox ( _txt_ ) for any purposes. |
| ESC ] 9 ; 3 ; "_txt_" ST | Change ConEmu Tab to _txt_. Set empty string to return original Tab text. |
| ESC ] 9 ; 4 ; _st_ ; _pr_ ST | Set progress state on Windows 7 taskbar and ConEmu title. When _st_ is 0: remove progress. When _st_ is 1: set progress value to _pr_ (number, 0-100). When _st_ is 2: set error state in progress on Windows 7 taskbar |
| ESC ] 9 ; 5 ST        | Wait for Enter/Space/Esc. Set environment variable "ConEmuWaitKey" to "ENTER"/"SPACE"/"ESC" on exit. |
| ESC ] 9 ; 6 ; "_txt_" ST | Execute GuiMacro ( _txt_ ). Set EnvVar "ConEmuMacroResult" on exit. |
| ESC ] 9 ; 7 ; "_cmd_" ST | Run some process with arguments.    |
| ESC ] 9 ; 8 ; "_env_" ST | Output value of environment variable. |


# Examples #
## ANSI and xterm color maps ##
<img src='http://conemu-maximus5.googlecode.com/svn/files/ConEmuAnsi.png' title='ANSI X3.64 and Xterm 256 colors in ConEmu'>

<h3>Xterm 256 color map</h3>
Example from file: <code>ConEmu\Addons\AnsiColors256.ans</code>.<br>
<pre><code>^[[9999S^[[9999;1HSystem colors (0..15 from xterm palette):
^[[48;5;0m  ^[[48;5;1m  ^[[48;5;2m  ^[[48;5;3m  ^[[48;5;4m  ^[[48;5;5m  ^[[48;5;6m  ^[[48;5;7m  ^[[0m
^[[48;5;8m  ^[[48;5;9m  ^[[48;5;10m  ^[[48;5;11m  ^[[48;5;12m  ^[[48;5;13m  ^[[48;5;14m  ^[[48;5;15m  ^[[0m

Color cube, 6x6x6 (16..231 from xterm palette):
^[[48;5;16m  ^[[48;5;17m  ^[[48;5;18m  ^[[48;5;19m  ^[[48;5;20m  ^[[48;5;21m  ^[[0m ^[[48;5;52m  ^[[48;5;53m  ^[[48;5;54m  ^[[48;5;55m  ^[[48;5;56m  ^[[48;5;57m  ^[[0m ^[[48;5;88m  ^[[48;5;89m  ^[[48;5;90m  ^[[48;5;91m  ^[[48;5;92m  ^[[48;5;93m  ^[[0m ^[[48;5;124m  ^[[48;5;125m  ^[[48;5;126m  ^[[48;5;127m  ^[[48;5;128m  ^[[48;5;129m  ^[[0m ^[[48;5;160m  ^[[48;5;161m  ^[[48;5;162m  ^[[48;5;163m  ^[[48;5;164m  ^[[48;5;165m  ^[[0m ^[[48;5;196m  ^[[48;5;197m  ^[[48;5;198m  ^[[48;5;199m  ^[[48;5;200m  ^[[48;5;201m  ^[[0m 
^[[48;5;22m  ^[[48;5;23m  ^[[48;5;24m  ^[[48;5;25m  ^[[48;5;26m  ^[[48;5;27m  ^[[0m ^[[48;5;58m  ^[[48;5;59m  ^[[48;5;60m  ^[[48;5;61m  ^[[48;5;62m  ^[[48;5;63m  ^[[0m ^[[48;5;94m  ^[[48;5;95m  ^[[48;5;96m  ^[[48;5;97m  ^[[48;5;98m  ^[[48;5;99m  ^[[0m ^[[48;5;130m  ^[[48;5;131m  ^[[48;5;132m  ^[[48;5;133m  ^[[48;5;134m  ^[[48;5;135m  ^[[0m ^[[48;5;166m  ^[[48;5;167m  ^[[48;5;168m  ^[[48;5;169m  ^[[48;5;170m  ^[[48;5;171m  ^[[0m ^[[48;5;202m  ^[[48;5;203m  ^[[48;5;204m  ^[[48;5;205m  ^[[48;5;206m  ^[[48;5;207m  ^[[0m 
^[[48;5;28m  ^[[48;5;29m  ^[[48;5;30m  ^[[48;5;31m  ^[[48;5;32m  ^[[48;5;33m  ^[[0m ^[[48;5;64m  ^[[48;5;65m  ^[[48;5;66m  ^[[48;5;67m  ^[[48;5;68m  ^[[48;5;69m  ^[[0m ^[[48;5;100m  ^[[48;5;101m  ^[[48;5;102m  ^[[48;5;103m  ^[[48;5;104m  ^[[48;5;105m  ^[[0m ^[[48;5;136m  ^[[48;5;137m  ^[[48;5;138m  ^[[48;5;139m  ^[[48;5;140m  ^[[48;5;141m  ^[[0m ^[[48;5;172m  ^[[48;5;173m  ^[[48;5;174m  ^[[48;5;175m  ^[[48;5;176m  ^[[48;5;177m  ^[[0m ^[[48;5;208m  ^[[48;5;209m  ^[[48;5;210m  ^[[48;5;211m  ^[[48;5;212m  ^[[48;5;213m  ^[[0m 
^[[48;5;34m  ^[[48;5;35m  ^[[48;5;36m  ^[[48;5;37m  ^[[48;5;38m  ^[[48;5;39m  ^[[0m ^[[48;5;70m  ^[[48;5;71m  ^[[48;5;72m  ^[[48;5;73m  ^[[48;5;74m  ^[[48;5;75m  ^[[0m ^[[48;5;106m  ^[[48;5;107m  ^[[48;5;108m  ^[[48;5;109m  ^[[48;5;110m  ^[[48;5;111m  ^[[0m ^[[48;5;142m  ^[[48;5;143m  ^[[48;5;144m  ^[[48;5;145m  ^[[48;5;146m  ^[[48;5;147m  ^[[0m ^[[48;5;178m  ^[[48;5;179m  ^[[48;5;180m  ^[[48;5;181m  ^[[48;5;182m  ^[[48;5;183m  ^[[0m ^[[48;5;214m  ^[[48;5;215m  ^[[48;5;216m  ^[[48;5;217m  ^[[48;5;218m  ^[[48;5;219m  ^[[0m 
^[[48;5;40m  ^[[48;5;41m  ^[[48;5;42m  ^[[48;5;43m  ^[[48;5;44m  ^[[48;5;45m  ^[[0m ^[[48;5;76m  ^[[48;5;77m  ^[[48;5;78m  ^[[48;5;79m  ^[[48;5;80m  ^[[48;5;81m  ^[[0m ^[[48;5;112m  ^[[48;5;113m  ^[[48;5;114m  ^[[48;5;115m  ^[[48;5;116m  ^[[48;5;117m  ^[[0m ^[[48;5;148m  ^[[48;5;149m  ^[[48;5;150m  ^[[48;5;151m  ^[[48;5;152m  ^[[48;5;153m  ^[[0m ^[[48;5;184m  ^[[48;5;185m  ^[[48;5;186m  ^[[48;5;187m  ^[[48;5;188m  ^[[48;5;189m  ^[[0m ^[[48;5;220m  ^[[48;5;221m  ^[[48;5;222m  ^[[48;5;223m  ^[[48;5;224m  ^[[48;5;225m  ^[[0m 
^[[48;5;46m  ^[[48;5;47m  ^[[48;5;48m  ^[[48;5;49m  ^[[48;5;50m  ^[[48;5;51m  ^[[0m ^[[48;5;82m  ^[[48;5;83m  ^[[48;5;84m  ^[[48;5;85m  ^[[48;5;86m  ^[[48;5;87m  ^[[0m ^[[48;5;118m  ^[[48;5;119m  ^[[48;5;120m  ^[[48;5;121m  ^[[48;5;122m  ^[[48;5;123m  ^[[0m ^[[48;5;154m  ^[[48;5;155m  ^[[48;5;156m  ^[[48;5;157m  ^[[48;5;158m  ^[[48;5;159m  ^[[0m ^[[48;5;190m  ^[[48;5;191m  ^[[48;5;192m  ^[[48;5;193m  ^[[48;5;194m  ^[[48;5;195m  ^[[0m ^[[48;5;226m  ^[[48;5;227m  ^[[48;5;228m  ^[[48;5;229m  ^[[48;5;230m  ^[[48;5;231m  ^[[0m 
Grayscale ramp (232..255 from xterm palette):
^[[48;5;232m  ^[[48;5;233m  ^[[48;5;234m  ^[[48;5;235m  ^[[48;5;236m  ^[[48;5;237m  ^[[48;5;238m  ^[[48;5;239m  ^[[48;5;240m  ^[[48;5;241m  ^[[48;5;242m  ^[[48;5;243m  ^[[48;5;244m  ^[[48;5;245m  ^[[48;5;246m  ^[[48;5;247m  ^[[48;5;248m  ^[[48;5;249m  ^[[48;5;250m  ^[[48;5;251m  ^[[48;5;252m  ^[[48;5;253m  ^[[48;5;254m  ^[[48;5;255m  ^[[0m
</code></pre>
<b>Warning</b> You need to change ‘^[’ to ESC code before using this script (char with code \x1B).<br>
<br>
<h3>Standard ANSI color map</h3>
This example is from file: <code>ConEmu\Addons\AnsiColors16.ans</code>.<br>
<pre><code>System colors (Standard console 16 colors):
^[[0;30;40m  ^[[0;30;41m  ^[[0;30;42m  ^[[0;30;43m  ^[[0;30;44m  ^[[0;30;45m  ^[[0;30;46m  ^[[0;30;47m  ^[[0m
^[[0;30;4;40m  ^[[0;30;4;41m  ^[[0;30;4;42m  ^[[0;30;4;43m  ^[[0;30;4;44m  ^[[0;30;4;45m  ^[[0;30;4;46m  ^[[0;30;4;47m  ^[[0m
</code></pre>
<b>Warning</b> You need to change ‘^[’ to ESC code before using this script (char with code \x1B).<br>
<br>
<h2>sixteencolors.net</h2>
Large <a href='http://en.wikipedia.org/wiki/ANSI_art'>ANSI art</a> archive: <a href='http://sixteencolors.net/'>http://sixteencolors.net/</a>

Download ans file and execute it in ConEmu console, e.g. <code>type TK-FREES.ANS</code>.<br>
You can view and scroll last output switching into alternative mode - Win+A.<br>
<br>
<b>Note</b> Many arts was written for 80-chars console width.<br>
<br>
<h2>Compiler error highlighting</h2>
<pre><code>nmake | sed -e "s/.* : \bERR.*/^[[1;31;40m&amp;^[[0m/i" -e "s/.* : \bWARN.*/^[[1;36;40m&amp;^[[0m/i"
</code></pre>
or<br>
<pre><code>type "Errors.log" | sed -e "s/.* : \bERR.*/^[[1;31;40m&amp;^[[0m/i" -e "s/.* : \bWARN.*/^[[1;36;40m&amp;^[[0m/i"
</code></pre>
<b>Warning</b> Before execution replace ‘^[’ width real ESC character (symbol with code of \x1B).<br>
<br>
<h2>Text Progressbar in cmd-files</h2>
<a href='http://code.google.com/p/conemu-maximus5/issues/attachmentText?id=554&aid=5540007000&name=test_bar.cmd&token=X1cvdOQjxnWPTGqFCrzyH1M0DCs%3A1337810139732'>test_bar.cmd</a> @Artyom.Vorobets