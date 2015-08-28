﻿=Migration to [conemu.github.io](http://conemu.github.io/)=
Google has declared [shutdown](http://google-opensource.blogspot.ru/2015/03/farewell-to-google-code.html) of googlecode service.

New versions of this file will be available on [conemu.github.io](http://conemu.github.io/en/GuiMacro.html).


---


There is an easy way to control (a little) ConEmu window from console applications or additional hotkeys.
It's a GuiMacro.



## Abstract ##
You can execute ConEmu GUI macro from:
  * ConEmu hotkey - set up ‘Macro 01’ … ‘Macro 32’ on ‘Keys’ page of ‘Settings’ dialog.
  * [command line](GuiMacro#Command_line.md) (or batch file), format is flexible, some examples:
```
"%ConEmuBaseDir%\ConEmuC.exe" /GUIMACRO WindowMinimize
ConEmuC -GuiMacro palette 1 "<Solarized>" -guimacro WindowMaximize
ConEmuC -guimacro status 1 test
ConEmuC -Guimacro Flash(1,3,5) ; MsgBox("Notification text")
ConEmuC -GuiMacro:0 print: C:\Program Files (x86)\Adobe\Reader 10.0\Reader\AcroRd32
```
  * [ANSI X3.64](http://conemu.github.io/en/AnsiEscapeCodes.html) OSC code. The following example, is an example only (if you need to check ‘[IsConEmu](ConEmuC.md)’ it is better to call "`ConEmuC -IsConEmu`" and check %errorlevel%).
```
rem We need real ESC code (symbol with ASCII \x1B) in cmd scripts
call SetEscChar.cmd
echo %ESC%]9;6;"IsConEmu"%ESC%\
if "%ConEmuMacroResult%"=="Yes" echo ConEmu found!
```
  * via [Far Manager plugin](GuiMacro#Far_Manager_plugin.md) (Far1/Far2/Far3 are supported):
```
F11 -> ConEmu -> Execute ConEmu macro
```
  * you can use Far Manager macros to forward commands for ConEmu GUI.
    * **Far3-lua**: `Plugin.SyncCall("4b675d80-1d4a-4ea9-8436-fdc23f2fc14b","IsConEmu()")`
    * **Far3-pre-lua**: `Plugin.Call("4b675d80-1d4a-4ea9-8436-fdc23f2fc14b","IsConEmu()")`, _callplugin is alias of Plugin.Call_
    * **Far2**: `callplugin(0x43454D55,"IsConEmu()")`

## Syntax ##
ConEmu GuiMacro parser is flexible and support different styles.

### Styles of arguments specification ###

  * surrounded by brackets:
```
Function(Arguments)
```
  * delimited by space:
```
Function Arguments
```
  * delimited by colon (and space optionally):
```
Function: Arguments
```
    * **Note** Using this style you may easily pass **one-string-argument**. Other methods may require quotating, backslash-escaping or verbatim strings. Example:
```
print: C:\Program Files (x86)\Adobe\Reader 10.0\Reader\AcroRd32
```

### Specifying more that a one argument ###

  * delimited by commas:
```
Function(Arg1,Arg2,Arg3)
```
  * delimited by spaces:
```
Function Arg1 Arg2 Arg3
```

### Executing several functions at once ###

  * delimited by semicolons:
```
print("abc"); keys("^_"); print("def")
```
  * delimited by `-GuiMacro` when called from your shell prompt:
```
ConEmuC -GuiMacro print abc -GuiMacro keys "^_" -GuiMacro print def
```

### Numerical arguments ###

  * decimal form with leading optional ‘+’ or ‘-’: `FontSetSize(1,-1)`
  * hexadecimal form with ‘0x’ or ‘0X’ prefix: `MsgBox("Text","Title",0x132)`

### String arguments ###
It is recommended to put string arguments in doublequotes, but in fact that is not required for ‘one-word’ arguments.

#### C-style strings ####
Backslashes and doublequotes must be escaped with backslashes.

Special symbols can be specified using escaped form:
```
\\  --> \
\"  --> "
\r  --> Carriage return (ASCII 13)
\n  --> New line (ASCII 10)
\t  --> Horizontal tab (ASCII 9)
\b  --> Backspace (ASCII 8)
\e  --> Escape (ASCII 27)
\a  --> Alert (ASCII 7)
```

And hexadecimal form `"\xNNNN"` (up to 4 digits) may be used to specify exact unicode character:
```
\x1B   --> same as \e (ASCII 27)
\x044F --> russian ‘я’ letter
```

Example:
```
print("\eCD /d \"C:\\Users\"\n")
```

#### Verbatim strings ####
No ‘escaped’ symbols, with the only exception of doublequote - they must be doubled.

```
print(@"CD /d ""C:\Users""")
```

#### Far plugin notes ####
When you run some GuiMacro's from Far Manager macro and the string has backslashes ("`C:\\Users\\...`")
you need to use verbatim strings or double-double them because ConEmu GuiMacro parser always
parse escaped symbols itself. The GuiMacro may be called from many places (from command line for example)
and parser is the one for all inputs.

```
Plugin.SyncCall("4b675d80-1d4a-4ea9-8436-fdc23f2fc14b","shell(\"new_console\",@\""..win.GetEnv("FARHOME").."\\far.exe\")")
```

## Command line ##
Syntax (doesn't matter `ConEmuC.exe` or `ConEmuC64.exe` you are using):
```
ConEmuC /GuiMacro[:PID|HWND][:T<tab>][:S<split>] <GuiMacro command>
or
ConEmuC -GuiMacro[:PID|HWND][:T<tab>][:S<split>] <GuiMacro command>
```

| **PID** | _‘Process ID’, decimal number_ |
|:--------|:-------------------------------|
| 0       | Special value means ‘Use active tab/split of the first found ConEmu window |
| ConEmu GUI PID | `ConEmu.exe` or `ConEmu64.exe`, macro will be executed in the **active** tab/split |
| Console server PID | `ConEmuC.exe` or `ConEmuC64.exe` started with `/ROOT` argument |
| **HWND** | _Window descriptor, hexadecimal number with `x` prefix_ |
| ConEmu GUI HWND | Main ConEmu window, macro will be executed in the **active** tab/split |
| Console server HWND | The hidden real console window |
| **T`<`tab`>`** | Changes affected RealConsole, _‘tab’, decimal 1-based tab index_ |
| **S`<`split`>`** | Changes affected RealConsole, _‘split’, decimal 1-based pane index of selected/active group of panes_ |

When GUI macro executed, ConEmuC.exe set errorlevel to 0 and write result to StdOut.

When you execute macro without `PID` or `HWND`,
```
ConEmuC -GuiMacro print abc
```
your macro will be executed in the **current** console (tab/split), if it was started in ConEmu.
On the other hand, if you try to call that **outside** of ConEmu - execution
will fails with ‘%errorlevel% == 134’, because your console (where you are trying
to execute macro) is **not** a ConEmu console.

So, if you need to execute macro from any external application (Visual Studio, Sublime text, even from `Win+R`) you may
specify exact instance with `PID` or `HWND`.

`PID` must be decimal number. `HWND` must be specified as hexadecimal number (window descriptor) with ‘x’ prefix.
Exact value you may see in the ConEmu status bar, in the Windows Task Manager, or find yourself using Windows API.

Few examples:
```
ConEmuC -GuiMacro:0 print: C:\Program Files (x86)\Adobe\Reader 10.0\Reader\AcroRd32
ConEmuC -GuiMacro:8204 print "This is sample text"
ConEmuC -GuiMacro:x140F32 WindowMinimize
```

## Far Manager plugin ##
You may run GuiMacro from Far Plugin (`ConEmu.dll` / `ConEmu64.dll`) from:
  * Using menu command: `F11` -> `ConEmu` -> `Execute ConEmu macro`
  * Using Far Manager macro language:
    * **Far v3.x Lua**:
```
Plugin.SyncCall("4b675d80-1d4a-4ea9-8436-fdc23f2fc14b","IsConEmu()")
```
    * **Far v3.x pre Lua**:
```
Plugin.Call("4b675d80-1d4a-4ea9-8436-fdc23f2fc14b","IsConEmu()")
```
    * **Far v2.x**:
```
callplugin(0x43454D55,"IsConEmu()")
```

When you call plugin menu, result is show in the dialog field.

When you use Far Manager macro - result is placed to the ‘[ConEmuMacroResult](ConEmuEnvironment.md)’ environment variable.

## List of functions ##
<a href='Hidden comment:  !!! when modifying - add same functions to "src\ConEmu\About.h" !!! '></a>
```
    About(["<Page>"])
      - Show ‘About’ dialog with page activated, e.g. ‘About("-new_console")’
    Attach([<PID>[,<Alternative>]])
      - Attach console or ChildGui by PID
    Break([<Event>[,<ProcessGroupId>]])
      - Event==0: Generates a CTRL+C signal
        Event==1: Generates a CTRL+BREAK signal
        More info in GenerateConsoleCtrlEvent WinAPI function
    Close(<What>[,<Flags>])
      - Close current console (0), without confirmation (0,1),
        terminate active process (1), without confirmation (1,1)
        close ConEmu window (2)
        close active tab (3)
        close active group (4)
        close all tabs but active (5)
        close active tab or group (6)
        close all active processes of the active group (7)
        close all tabs (8)
    Copy(<What>[,<Format>[,"<File>"]])
      - Copy active console contents
        What==0: current selection
        What==1: all buffer contents
        What==2: visible area contents
        Format==0: plain text, not formatting
        Format==1: copy HTML format
        Format==2: copy as HTML
        File: if specified - save to file instead of clipboard
    FindEditor("<FullEditFileName>"),
    FindViewer("<FullViewerFileName>"),
    FindFarWindow(<WindowType>,"<WindowTitle>")
     - Returns "Found:<index of RealConsole>", "Active:<Far window number>", "Blocked", or "NotFound"
       <WindowType> is number from Far Manager enum WINDOWINFO_TYPE
    Flash(<Cmd>[,<Flags>[,<Count>]])
      - Allows to flash taskbar icon and/or window caption
        Flash(0) - Stop all flashing
        Flash(0,1) - Simple flashing (see MSDN FlashWindow)
    Flash(1,<Flags>,<Count>)
      - Special flashing (see MSDN FlashWindowEx)
        Flags: 0 - stop, 1 - caption, 2 - taskbar, 3 - caption+taskbar, etc.
        Count: the number of times to flash the window
    FontSetName("<FontFaceName>"[,FontHeight[,FontWidth]])
     - Change font face name and (optionally) height/width.
    FontSetSize(Relative,N)
     - Change font height in the ConEmu window
       Relative==0: N - required font height in points
       Relative==1: N (+-1, +-2) - increase/decrease font height
       Relative==2: N (per cents) - alias for Zoom(N)
       returns - "OK", or "InvalidArg"
    GetOption("<Name>")
     - Returns value of some ConEmu options (the set is limited)
    GroupInput([<Cmd>])
     - Group keyboard input for visible splits
       Cmd==0: switch mode (default)
       Cmd==1: group
       Cmd==2: un-group
    HighlightMouse(0)
     - Switch highlight under mouse cursor (off/row/col/row+col/...)
    HighlightMouse(<What>,<Act>)
     - What: 1 - row, 2 - col, 3 - row+col
       Act:  0 - off, 1 - on,  2 - switch (default)
    IsConEmu
     - Returns "Yes" when console was started under !ConEmu
       Alternative is "ConEmuC.exe -IsConEmu" and checking errorlevel
    IsConsoleActive
     - Check, is RealConsole active or not, "Yes"/"No"
    IsRealVisible
     - Check, is RealConsole visible or not, "Yes"/"No"
    Keys("<Combo1>"[,"<Combo2>"[,...]])
     - Post special keystrokes to the console (AutoHotKey syntax)
       "[Mod1[Mod2[Mod3]]]Key"
       Mod: ^ - LCtrl, >^ - RCtrl, ! - LAlt, >! - RAlt, + - Shift
       Example: Keys("^_") will post ‘Ctrl+_’
    MsgBox("<Text>","<Title>",<ButtonType>)
     - Show modal GUI MessageBox, returns clicked button name
       ("Ok", "Cancel", "Retry", etc.),
       ButtonType (number) is from Windows SDK.
    Paste(<Cmd>[,"<Text>"[,"<Text2>"[...]]])
     - When <Text> is omitted - paste from Windows clipboard,
       otherwise - paste specified text
       Cmd==0: paste all lines
       Cmd==1: paste first line
       Cmd==2: paste all lines, without confirmations
       Cmd==3: paste first line, without confirmations
       Cmd==4: select and parse file pathname, Text - default
       Cmd==5: select and parse folder pathname, Text - default
       Cmd==6: select and parse cygwin file pathname, Text - default
       Cmd==7: select and parse cygwin folder pathname, Text - default
       Cmd==8: paste path from clipboard converted to CygWin style
       Cmd==9: paste all lines space-separated
       Cmd==10: paste all lines space-separated, without confirmations
    PasteFile(<Cmd>[,"<File>"[,"<CommentMark>"]])
     - Paste <File> contents, omit <File> to show selection dialog
       Cmd==0: paste all lines
       Cmd==1: paste first line
       Cmd==2: paste all lines, without confirmations
       Cmd==3: paste first line, without confirmations
       Cmd==9: paste all lines space-separated
       Cmd==10: paste all lines space-separated, without confirmations
       If "<CommentMark>" specified - skip all lines starting with prefix
    Print(["<Text>"[,"<Text2>"[...]]])
     - Alias for Paste(2,"<Text>")
    Progress(<Type>[,<Value>])
     - Set progress state on taskbar and !ConEmu title
       Type=0: remove progress
       Type=1: set progress value to <Value> (0-100)
       Type=2: set error state in progress
       Type=3: set indeterminate state in progress
    Rename(<Type>,["<Title>"])
     - Rename tab (Type=0) or console title (Type=1)
    Scroll(<Type>,<Direction>,<Count=1>)
     - Do buffer scrolling actions
       Type: 0; Value: ‘-1’=Up, ‘+1’=Down
       Type: 1; Value: ‘-1’=PgUp, ‘+1’=PgDown
       Type: 2; Value: ‘-1’=HalfPgUp, ‘+1’=HalfPgDown
       Type: 3; Value: ‘-1’=Top, ‘+1’=Bottom
       Type: 4; No arguments; Go to cursor line
    Select(<Type>,<DX>,<DY>,<HE>)
     - Used internally for text selection
       Type: 0 - Text, 1 - Block
       DX: select text horizontally: -1/+1
       DY: select text vertically: -1/+1
       HE: to-home(-1)/to-end(+1) with text selection
    SetDpi(<DPI>)
     - Change effective dpi for ConEmu window: 96, 120, 144, 192
    SetOption("Check",<ID>,<Value>)
       ID: numeric identifier of checkbox (ConEmu.rc, resource.h)
       Value: 0 - off, 1 - on, 2 - third state
       Read about SetOption below
    SetOption("<Name>",<Value>[,<IsRelative>])
     - Name=one of allowed for changing ConEmu options
       IsRelative=1 to use relative instead of absolute for some options
       "AlphaValue": set transparency level of active window
         Value: 40 .. 255 (255==Opaque)
       "AlphaValueInactive": set transparency level of inactive window
         Value: 0 .. 255 (255==Opaque)
       "AlphaValueSeparate": enable separate active/inactive transparency level
         Value: 0(disable) .. 1(enable)
       "AlwaysOnTop": place ConEmu window above all non-topmost windows
         Value: 2 - toggle, 1 - enable, 0 - disable
       "bgImageDarker": darkening of background image
         Value: 0 .. 255
       "FarGotoEditorPath": path to the highlight/error editor
         Value: path to the executable with arguments (Issue 1704)
       "QuakeAutoHide": auto hide on focus lose, Quake mode
         Value: 2 - switch auto-hide, 1 - enable, 0 - disable
    Settings([<PageResourceId>])
     - Show ‘Settings’ dialog with specified page activated (optionally)
         PageResourceId: integer DialogID from ‘resource.h’
    Shell("<Verb>","<File>"[,"<Parms>"[,"<Dir>"[,<ShowCmd>]]])
     - Verb can be "open", "print" and so on, or special value "new_console",
       which starts File in the new tab of ConEmu window. Examples:
         Shell("open","cmd.exe","/k")
         Shell("","","cmd.exe /k -new_console:b"),
         Shell("new_console:b","","cmd.exe /k")
         Shell("new_console:sV")
         Shell("new_console:Ibn","cmd","/k start explorer \"%CD%\" && exit")
    Sleep(<Milliseconds>)
     - Milliseconds: max 10000 (10 seconds)
    Split(<Cmd>,<Horz>,<Vert>)
     - Cmd=0, Horz=1..99, Vert=0: Duplicate active ‘shell’ split to right, same as Shell("new_console:s50Hn")
       Cmd=0, Horz=0, Vert=1..99: Duplicate active ‘shell’ split to bottom, same as Shell("new_console:s50Vn")
       Cmd=1, Horz=-1..1, Vert=-1..1: Move splitter between panes (aka resize panes)
       Cmd=2, Horz=-1..1, Vert=-1..1: Put cursor to the nearest pane (preferred direction may be choosed)
       Cmd=3, Maximize/restore active pane
    Status(0[,<Parm>])
     - Show/Hide status bar, Parm=1 - Show, Parm=2 - Hide
    Status(1[,"<Text>"])
     - Set status bar text
    Tab(<Cmd>[,<Parm>])
     - Control ConEmu tabs
       Cmd==0:  show/hide tabs
       Cmd==1:  commit lazy changes
       Cmd==2:  switch next (eq. CtrlTab)
       Cmd==3:  switch prev (eq. CtrlShiftTab)
       Cmd==4:  switch tab direct (no recent mode), Parm=(1,-1)
       Cmd==5:  switch tab recent, Parm=(1,-1)
       Cmd==6:  switch console direct (no recent mode), Parm=(1,-1)
       Cmd==7:  activate console by number, Parm=(1-based console index)
       Cmd==8:  show tabs list menu (indiffirent Far/Not Far)
       Cmd==9:  close active tab, same as Close(3)
       Cmd==10: switches visible split-panes, Parm=(1,-1)
    Task(Index[,"Dir"])
     - start task with 1-based index
    Task("Name"[,"Dir"])
     - start task with specified name
    Transparency(<Cmd>,<Value>)
     - Change ConEmu window transparency level
       Cmd = 0, Value = 40..255 (255==Opaque)
       Cmd = 1, Value = <relative inc/dec>
       Cmd = 2, Value = 0..255 (255==Opaque) - for inactive window
       Cmd = 3, Value = <relative inc/dec> - for inactive window
       Cmd = 4, Value = <0/1> - AlphaValueSeparate
    Wiki(["PageName"])
     - Open online documentation
    WindowFullscreen()
     - Switch fullscreen window mode
       returns previous window mode
    WindowMaximize([<Cmd>])
     - Switch maximized window mode (Cmd==0)
     - Maximize window by width (Cmd==1) or height (Cmd==2)
       returns previous window mode
    WindowMinimize() or WindowMinimize(1)
     - Minimize ConEmu window ("1" means "minimize to the TSA")
    WindowMode([\"<Mode>\"])
     - Returns or set current window mode
       "NOR", "MAX", "FS" (fullscreen), "MIN", "TSA"
       "TLEFT", "TRIGHT" (tile to left/right), "THEIGHT",
       "MPREV", "MNEXT" (move ConEmu to prev/next monitor)
    Zoom(N)
     - Set zoom value, 100 - original font size
```

## SetOption ##
You may change some (or most) of ConEmu settings using `SetOption` macro function.

It may be used **by name** for several options, for example
```
SetOption("FarGotoEditorPath","far.exe /e%1:%2 \"%3\"")
```

Or **by CheckBox/RadioButton ID**. Of course you must know that number-ID. Look up the control
by its label in the [ConEmu.rc](https://github.com/Maximus5/ConEmu/blob/master/src/ConEmu/ConEmu.rc)
(don't forget about possible ‘`&`’) and then look up the exact number-ID by its string-ID
in the [resource.h](https://github.com/Maximus5/ConEmu/blob/master/src/ConEmu/resource.h).

Turn scrollbar on:
```
ConEmuC -GuiMacro SetOption Check 2488 1
```

Turn Quake style activation off
```
ConEmuC -GuiMacro SetOption Check 2333 0
```

## Examples ##
### Far3-Lua ###
```
local conemu = "4B675D80-1D4A-4EA9-8436-FDC23F2FC14B"

if Plugin.SyncCall(conemu,"IsConEmu")=="Yes" then
  far.Message("Far started under ConEmu","Macro")
end

local cmd = ('shell("open",@"%s")'):format(win.GetEnv"FARHOME"..[[\Encyclopedia\FarEncyclopedia.ru.chm]])
Plugin.SyncCall(conemu,cmd)

local cmd = ('shell("new_console",@"%s")'):format(win.GetEnv"FARHOME".."\\far.exe")
Plugin.SyncCall(conemu,cmd)
```
### Far3-pre-Lua ###
```
$if (callplugin("4b675d80-1d4a-4ea9-8436-fdc23f2fc14b","IsConEmu()")!=0 && env("ConEmuMacroResult")=="Yes")
  MsgBox("Macro", "Far started under ConEmu")
$end

callplugin("4b675d80-1d4a-4ea9-8436-fdc23f2fc14b","shell(\"open\",@\""+env("FARHOME")+"\\Encyclopedia\\FarEncyclopedia.ru.chm\")")

callplugin("4b675d80-1d4a-4ea9-8436-fdc23f2fc14b","shell(\"new_console\",@\""+env("FARHOME")+"\\far.exe\")")
```
### Far2 ###
```
$if (callplugin(0x43454D55,"IsConEmu()")!=0 && env("ConEmuMacroResult")=="Yes")
  MsgBox("Macro", "Far started under ConEmu")
$end
```
### Far1 ###
This is most complicated, cause of 'callplugin' absence in the Far 1.x
```
F11
$if (menu.Select("ConEmu",2)<=0) MsgBox("ConEmu","Plugin was not found in list!",1)
  $Exit
$end
Enter %r="";
$if (menu.select("~.",1)>0)
  ~ $Text "IsConEmu()" Enter %r=Dlg.GetValue(5,0);
$end
Esc
$if (%r=="Yes")
  MsgBox("Macro", "Far started under ConEmu")
$end
```