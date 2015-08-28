
## Более новые записи ##
  * [Builds 140104 .. текущая](Whats_New.md)


## Build 131225 ##
  * [Issue 1406](https://code.google.com/p/conemu-maximus5/issues/detail?id=1406): Sources was converted to utf-8 (mostly by Rick Sladkey).
  * GCC makefiles and source fixes.
  * DllMain was not called when compiled with GCC.
  * InjectHookDLL cause crash in hooked process when compiled with GCC.
  * File ".editorconfig" added into "src" folder.
  * [Issue 1404](https://code.google.com/p/conemu-maximus5/issues/detail?id=1404): Workaround for deadlock between AltServerStop and ReloadFullConsoleInfo (by Rick Sladkey).
  * [Issue 1404](https://code.google.com/p/conemu-maximus5/issues/detail?id=1404): Fix overlapped I/O for ReadFile like TransactNamePipe (by Rick Sladkey).
  * [Issue 1405](https://code.google.com/p/conemu-maximus5/issues/detail?id=1405): Don't wait for overlapped event unless the API assures us that I/O is pending (by Rick Sladkey).
  * If no desktop dir is available - save dump into %TEMP%.
  * Some internal code changes.


## Build 131223 ##
  * [Issue 1379](https://code.google.com/p/conemu-maximus5/issues/detail?id=1379): Disable fade in effect on startup.
  * Console font dialog - 'restart may be required'.
  * [Issue 1402](https://code.google.com/p/conemu-maximus5/issues/detail?id=1402): Prevent RM\_ALTSERVER mode from warping CurrentDirectory (by Rick Sladkey).
  * New ANSI sequences supported:
```
"ESC [ n X" - erase n characters
"ESC [ > c" --> Report "ESC > 67 ; build ; 0 c"
"ESC [ c"   --> Report "ESC [ ? 1 ; 2 c"
"ESC [ 5 n" -> Report status as "CSI 0 n" (OK)
"ESC [ 6 n" -> Report Cursor Position as "ESC [ row ; col R"
"ESC [ 1 8 t" --> Report the size of the text area in characters as "ESC [ 8 ; height ; width t"
"ESC [ 1 9 t" --> Report the size of the screen in characters as "ESC [ 9 ; height ; width t"
"ESC [ 2 1 t" --> Report window’s title as "ESC ] l title ESC \"
```
  * ANSI xterm256. When line was wrapped (long prompt or output)
    * extended atributes was not shifted properly;
    * rest of line was printed char-by-char.
  * [Issue 1403](https://code.google.com/p/conemu-maximus5/issues/detail?id=1403): Seamless start with default ConEmu config in WinPE.
  * Ready to create MiniDump in WinPE.
  * Try to use 'Courier New' as border font if 'Lucida Console' fails
  * [Issue 1399](https://code.google.com/p/conemu-maximus5/issues/detail?id=1399): Angular quotation marks in the English translation.
  * [Issue 1400](https://code.google.com/p/conemu-maximus5/issues/detail?id=1400): Environment variables dont't work in update path to 7zip
  * Portable\_Test.cmd - creates simple one-file portable executable using WinRar.
  * Some internal changes.


## Build 131219 ##
  * ConEmuHk linked with flags `/DYNAMICBASE:NO /FIXED:NO /BASE:0x7E110000`.
  * GuiMacro Transparent & HighlightMouse described in the About dialog.
  * Hide cursor during mouse-selection, show half-horz cursor during keyboard selection.
  * Copy, Detect line ends. Allow one space on the next line beginning.
  * Some hotkeys descriptions was changed
    * Highlighting: hyperlinks and compiler errors (FarGotoEditor modifier);
    * Highlighting: Switch ‘Highlight row under mouse cursor’.
  * ANSI & xterm256 improvements:
    * Try to deal with ReadLine scrolling and xterm256;
    * Clear extened attributes after cmd's cls.
    * `"ESC [ 0 m"` set original colors instead of gray-on-black; ‘Original’ means colors was set in console before first ANSI color change (first write to console actually) in the current application (e.g. cmd.exe).
  * Addons\git.cmd -> Addons\git\_sample.cmd.
  * Some internal and debug changes.


## Build 131215 ##
  * [Issue 1362](https://code.google.com/p/conemu-maximus5/issues/detail?id=1362): Hotkeys for inc/dec transparency
    * GuiMacro: `Transparency(<Cmd>,<Value>)`
```
Cmd = 0, Value = 40..255 (255==Opaque)
Cmd = 1, Value = <relative inc/dec>
```
    * Hotkeys (not assigned by default) for inc/dec transparency by 20 'points'.
  * [Issue 1391](https://code.google.com/p/conemu-maximus5/issues/detail?id=1391): Extended fonts not applied to first visible line of buffer (by rick.sladkey).
  * Highlighting row/col under mouse cursor.
    * Checkboxes on 'Highlight' settings page.
    * GuiMacro `HighlightMouse(<What>[,<Act>])`. Affect ACTIVE console only.
```
HighlightMouse(0) switch off/row/col/row+col/off/...
otherwise
  What: 1 - row, 2 - col, 3 - row+col
  Act:  0 - off, 1 - on,  2 - switch (default)
```
    * New hotkey Apps+L switch row highlighting: HighlightMouse(1)
    * Some bugfixes
  * Many changes in mark/paste/highlighting setting pages.
    * Page 'Mark & Paste' renamed to 'Mark/Copy'
    * New pages 'Paste' and 'Highlight'
    * Group 'Mouse button actions' moved to 'Controls' page
    * Group 'Copying format' added to 'Mark/Copy' page
    * Group 'Pasting text behavior' moved to 'Paste' page
    * Group 'Hyperlinks and compiler errors' moved to 'Highlight' page
  * [Issue 1392](https://code.google.com/p/conemu-maximus5/issues/detail?id=1392): Allow drop simple text from other apps (browsers, e.g.)
  * Cmd line parsing, quotation issues.
  * Internal changes. ConEmuHk, ShellProcessor.
  * Internal changes. RealConsole: isGuiForceConView, ResetVarsOnStart
  * Internal changes. Allow DontEnable in MainThread only


## Build 131211 ##
  * For life simplification and remove excess 'cmd' from process tree, ConEmu will process 'set' commands internally (ConEmuC) before starting root process. Example:
```
set term=msys & "set PATH=C:\Program Files\GIT\bin;%PATH%" & powershell
```
  * [Issue 1387](https://code.google.com/p/conemu-maximus5/issues/detail?id=1387). Garbage in console if UpRight/DownLeft background placement.
  * Internal changes. Support variable length in NextArg.
  * Internal changes. ExtendedConsole was added to exclusion list of ConEmuHk.
  * Internal changes. gbDontEnable -> DontEnable::isDontEnable.


## Build 131207 ##
  * [Issue 1385](https://code.google.com/p/conemu-maximus5/issues/detail?id=1385). Fix wrong coloring of DBCS chars in 936/932 cp.


## Build 131205 ##
  * Some fixes in pipe processing.
  * Some codeanalyze fixes.
  * Some more debug info in MPipe.h.
  * `IsConEmu.cmd` updated, returns errorlevel 0,1,2.
  * [Issue 881](https://code.google.com/p/conemu-maximus5/issues/detail?id=881): ConEmuHk causes IBM Java applications to crash.
  * [Issue 1236](https://code.google.com/p/conemu-maximus5/issues/detail?id=1236): Far Manager. Pathname not fit in left panel causes right mouse click failure.
  * [Issue 1358](https://code.google.com/p/conemu-maximus5/issues/detail?id=1358): Allow multiple selections in 'Attach To' window.


## Build 131202 ##
  * Trying to fix MS bug with kernel crash in ReadConsole.
  * Some fixes in pipe processing.
  * [Issue 1381](https://code.google.com/p/conemu-maximus5/issues/detail?id=1381). Portable distribution contains `*.map` files (`ConEmu.Gui.map` and `ConEmu64.Gui.map`).
  * [Issue 1253](https://code.google.com/p/conemu-maximus5/issues/detail?id=1253): Add selection information to status bar (stream/block, coords, length).
  * GuiMacro. "Close(2,1)" will close ConEmu window, same as 'Cross' click, bug without confirmation.


## Build 131129 ##
  * [Issue 1375](https://code.google.com/p/conemu-maximus5/issues/detail?id=1375). Xml file search priority changed. First found from following locations will be used:
```
%ConEmuDir%\ConEmu.xml
%ConEmuBaseDir%\ConEmu.xml
%APPDATA%\ConEmu.xml
```
  * [Issue 1368](https://code.google.com/p/conemu-maximus5/issues/detail?id=1368). ConEmuBg. Suppress SYNCHRO in viewer/editor.


## Build 131125 ##
  * [Issue 1367](https://code.google.com/p/conemu-maximus5/issues/detail?id=1367): `ConEmuArgs`, `ConEmuBaseDir`, `ConEmuConfig`, `ConEmuDir` was not initialized.
  * Excess confirmations when closing Far consoles.


## Build 131124 ##
  * Crash fix on tabs closing.
  * Initialize predefined palettes with "/reset" or "/basic" switches.
  * Setupper. Featute name ‘clink readme’ and ‘Show git branch’.
  * [Issue 1354](https://code.google.com/p/conemu-maximus5/issues/detail?id=1354): Checkbox ‘Process -new\_console and -cur\_console switches’.
  * New possible env values `"ConEmuHooks=OFF,NOARG"` to temporarily disable processing of -new\_console and -cur\_console switches.
  * Clear `"ConEmuFakeDT"`, `"ConEmuHooks"` on ConEmu.exe startup.
  * `ConEmuRelyDT` renamed to `ConEmuFakeDT`.
  * Some fixes in CSettings::GetString.
  * Settings, FarMacro. Edits -> ComboBoxes.
  * Contributors. ForNeVeR.
  * Fix stack overflow using `#Close(0)` in SafeFarClose.
  * Internal. Storing event handles in ConEmuHk.DllMain.
  * Internal. Some changed for Wine (dwLayoutName).


## Build 131119 ##
  * (Pull#2, ForNeVeR) ConEmuBg: Fit background text width.
  * `VConGroup.SetConsoleSizes` fails on more than 2 splits.
  * Fix close confirmation msg on WinXP.
  * Default button set to Cancel in Assertion dialog.
  * [Issue 1345](https://code.google.com/p/conemu-maximus5/issues/detail?id=1345): Menu shortcuts &Edit and Sett&ings.


## Build 131117 ##
  * Live preview peeks were broken in Windows 8.
  * New ConEmu specific ANSI OSC.
```
ESC ] 9 ; 7 ; "cmd" ST - Run some process with arguments
ESC ] 9 ; 8 ; "env" ST - Output value of environment variable
Usage example:
prompt $P$E]9;7;"cmd /cGitExportBranch.cmd"$e\$E]9;8;"gitbranch"$e\$g
```
  * Don't print "nothing to export" when exported variable is not defined.
```
ConEmuC /export=CON gitbranch
```
  * 60 sec delay between each OnInfo\_ReportCrash.
  * Helper batch file ‘cecho.cmd’ may be used to colored print from batches. Call "cecho /?" for usage examples.


## Build 131115 ##
  * [Issue 1339](https://code.google.com/p/conemu-maximus5/issues/detail?id=1339): Crash on close ConEmu window (multiple assertions actually, regression of 131114).
  * ConEmuSetup.exe: `/p:x[86|64],adm` starts MSI as admin right away.


## Build 131114 ##
  * [Issue 1333](https://code.google.com/p/conemu-maximus5/issues/detail?id=1333): Internal changes in WndProc, GWLP\_USERDATA is not used anymore.
  * Minor fix in "ConEmuC /export". Don't try to export to (a) direct parent of ConEmuC.exe process and (b) conhost.exe.
  * [Issue 581](https://code.google.com/p/conemu-maximus5/issues/detail?id=581): Task "/dir" switch was ignored when starting second tab.
  * Far Manager macros settings are moved to ‘Far macros’ page.
  * [Issue 1325](https://code.google.com/p/conemu-maximus5/issues/detail?id=1325) (Neverthness): ConEmuBg Far Plugin: VolumeFree may be used in Background.xml.
  * ConEmuBg: Force check xml file on plugin enabled.
  * GuiMacro: `WindowMaximize(<Cmd>)` - maximize window by width (Cmd==1) or height (Cmd==2). Same as doubleclick on the window frame edges.
  * New hotkeys on ‘Keys & Macro’ page: Maximize width and Maximize height.
  * [Issue 1329](https://code.google.com/p/conemu-maximus5/issues/detail?id=1329): Support env.vars in the "/icon" task argument when creating Jump lists.
  * [Issue 1165](https://code.google.com/p/conemu-maximus5/issues/detail?id=1165): Scroll console (don't send wheel events) if console mode has ENABLE\_QUICK\_EDIT\_MODE.
  * New Environment variable "ConEmuRelyDT". That will cheat system and local current time for console processes. Use carefully! Console applications will think that time was freezed! ‘Inject ConEmuHk’ must be on! Some examples (cmd):
```
set ConEmuRelyDT=%DATE% %TIME%
set ConEmuRelyDT=2013-12-31T23:59:59.99
set ConEmuRelyDT=2013-01-31 9:0:0
set ConEmuRelyDT=2013-11-30
```


## Build 131107 ##
  * Now it is possible to execute GuiMacro from outside of ConEmu window. Would remind that you may call several macros, delimited with semicolon, in a time.
```
Syntax: ConEmuC /GUIMACRO[:PID|HWND] <GuiMacro>
PID is a ConEmu.exe process identifier. HWND is a %ConEmuHWND%.
Also, you may call "/GUIMACRO:0" to affect first found instance.
```
  * Split created with command "cmd -new\_console:bsV" was left invisible.
  * Compatibility section for Windows 8.1 added to the ConEmu manifest.
  * [Issue 1277](https://code.google.com/p/conemu-maximus5/issues/detail?id=1277): Far Manager shows ‘Copy’ in the window title instead of panel path.
  * [Issue 1312](https://code.google.com/p/conemu-maximus5/issues/detail?id=1312): Default Terminal: Visual Studio 2013 console window not hooked.
  * [Issue 1317](https://code.google.com/p/conemu-maximus5/issues/detail?id=1317): Task GuiMacro was ignored directory parameter.
  * [Issue 1319](https://code.google.com/p/conemu-maximus5/issues/detail?id=1319): Crashes as soon as opened.


## Build 131105 ##
  * [Issue 1312](https://code.google.com/p/conemu-maximus5/issues/detail?id=1312):	Visual Studio 2013 console window not hooked
  * Regression. ‘Always on top’ was not cleared after exiting FullScreen mode (Alt+Enter).
  * [Issue 218](https://code.google.com/p/conemu-maximus5/issues/detail?id=218): Far Manager panels flickering while maximize/restore.


## Build 131103 ##
  * [Issue 1079](https://code.google.com/p/conemu-maximus5/issues/detail?id=1079): Console hangs when running PHP console scripts.
  * [Issue 1195](https://code.google.com/p/conemu-maximus5/issues/detail?id=1195): Invalid size locking region was used after resizing window when Far is active.
  * [Issue 1196](https://code.google.com/p/conemu-maximus5/issues/detail?id=1196): ConEmu's Far plugin language resources.
  * [Issue 1300](https://code.google.com/p/conemu-maximus5/issues/detail?id=1300): Failed (hanged) while closing Far Manager using ‘Safe Far close macro’ in some cases.
  * [Issue 1301](https://code.google.com/p/conemu-maximus5/issues/detail?id=1301): Clink initialization method was changed.
  * [Issue 1307](https://code.google.com/p/conemu-maximus5/issues/detail?id=1307): Some resize fixes in Quake mode.
  * [Issue 1308](https://code.google.com/p/conemu-maximus5/issues/detail?id=1308): Copy text without HTML formatting by default.
  * GuiMacro: `Copy(<What>[,<Format>])`. User may specify format regardless of current setting (choosed from menu).
```
0: plain text, not formatting; 1: copy HTML format; 2: copy as HTML.
```
  * New hotkeys (active while selection is present), Ctrl+C (plain text), Ctrl+Shift+C (HTML format) and unassigned for (as HTML).
  * Regression: Task hotkeys was broken.
  * On the first run (with Fast configuration dialog) task list will be filled with some predefined shells found on PC.
  * File `"AnsiColors16t.ans"` was updated with "How to use ANSI" recommendations.
  * Batch file `"SetEscChar.cmd"` provided for use with cmd.exe scripts.
  * [Issue 1308](https://code.google.com/p/conemu-maximus5/issues/detail?id=1308): Don't use copy with HTML formatting (by default).
  * [Issue 367](https://code.google.com/p/conemu-maximus5/issues/detail?id=367): New background placements - Center, Stretch-Fit, Stretch-Fill.
  * Debugging purposes: new ConEmu.exe switches "/nomacro" and "/nohotkey".
  * cmd.exe users: option ‘Support UNC Paths’ on ‘ComSpec’ allows "cd" to network location. This works in prompt only! And only in newly created consoles after enabling feature.
```
Example: cd \\server\share\folder
```


## Build 131026 ##
  * Showing Task hotkeys in the start menu (Win+N, [+] drop down).
  * Task hotkeys may be configured on ‘Keys & Macro’ settings page, too.
  * Regression. Changes in ‘Default terminal’ hooked applications was ignored.
  * Shift+Home and Shift+End start text selection if ‘Start selection with Shift+Arrows’ is enabled.
  * Trim trailing ‘.,;’ from hyperlinks (Ctrl+LClick).


## Build 131025 ##
  * [Issue 1304](https://code.google.com/p/conemu-maximus5/issues/detail?id=1304): Fail to Run history item from dropdown menu.
  * [Issue 1298](https://code.google.com/p/conemu-maximus5/issues/detail?id=1298): ConEmu may crash when starting some incorrect tasks.
  * [Issue 1269](https://code.google.com/p/conemu-maximus5/issues/detail?id=1269): Win+Left, Win+Right once more.
  * GuiMacro: SetOption("AlwaysOnTop",Value). Where Value: 2 - switch AlwaysOnTop, 1 - enable, 0 - disable.
  * [Issue 675](https://code.google.com/p/conemu-maximus5/issues/detail?id=675): When both "Reset selection on input" and "any key" are checked selection was dropped when Shift (e.g.) was released.
  * [Issue 685](https://code.google.com/p/conemu-maximus5/issues/detail?id=685): Allow using simple modifiers for text selection (Shift, Alt, Control).
  * [Issue 836](https://code.google.com/p/conemu-maximus5/issues/detail?id=836): Allow to set tasks hotkey.
  * Anyone still using WinXP? ‘Enter TEXT fullscreen mode, when available’ hotkey is available. This is a global hotkey, you may (try to) press it even if ConEmu has no focus.


## Build 131020 ##
  * [Issue 1267](https://code.google.com/p/conemu-maximus5/issues/detail?id=1267): Chinese message of previous command crash.
  * [Issue 1291](https://code.google.com/p/conemu-maximus5/issues/detail?id=1291): Python fails to print string sequence with ASCII character followed by Chinese character.
  * [Issue 1292](https://code.google.com/p/conemu-maximus5/issues/detail?id=1292): While selection is active, Shift-Home/Shift-End keys extend selection to line begin/end.
  * [Issue 1293](https://code.google.com/p/conemu-maximus5/issues/detail?id=1293): Regression. `"-new_console:d:<path>"` and whitespace in `<path>` = parser issue.
  * [Issue 1289](https://code.google.com/p/conemu-maximus5/issues/detail?id=1289): Regression. No background image at start.


## Build 131017 ##
  * New switch "-new\_console:W:Background" allows set background image per tab or split. Option ‘Background image’ **must be enabled** in main settings (main image path may be empty).
  * Regression. When prompting for password (-new\_console:u:User) command field was not filled.
  * Internal changes of "-new\_console" parser.
  * Minor internal changes in closing procedure.
  * About dialog, "-new\_console" page was added. May be called from Tasks setup ('?' button in title bar and click on tasks edit field).
  * [Issue 823](https://code.google.com/p/conemu-maximus5/issues/detail?id=823): Shift+left mouse selection does not select above visible windows (has to scroll buffer up).


## Build 131015 ##
  * Environment variables are allowed in tab icon paths.
  * [Issue 1275](https://code.google.com/p/conemu-maximus5/issues/detail?id=1275): ConEmu window was restored from TSA unexpectedly after RClick-Esc.
  * Menu item ‘Store history’ in the create new console dialog (right-click on title bar).
  * Minor changes in ‘New console’ dialog.
  * Environment export was not working in Far Manager. Example:
```
vcvarsall.bat x86 & ConEmuC /export=CON
```
  * Regression: crash of ConEmuC while expanding environment variables.


## Build 131010 ##
  * ConEmuC will expand environment variables before running commands.
  * Support PortableApps.com folder structure from the box. No need for explicit `'/LoadCfgFile "%PAL:DataDir%\settings\ConEmu.xml"'`.
  * [Issue 1274](https://code.google.com/p/conemu-maximus5/issues/detail?id=1274): Hang on "cmd --new\_console".
  * Switch "/icon" was ignored for startup task.
  * Trailing spaces was increased when used ‘Auto save/restore of opened tabs’.
  * [Issue 1275](https://code.google.com/p/conemu-maximus5/issues/detail?id=1275): When minimized into TSA and all VCon are closed we need to restore and run new tab.
  * New color scheme: ‘`<Monokai>`’.
  * [Issue 1277](https://code.google.com/p/conemu-maximus5/issues/detail?id=1277): Far Manager shows ‘DrawWND’ in the window title instead of panel path.
  * [Issue 1279](https://code.google.com/p/conemu-maximus5/issues/detail?id=1279): Copy to clipboard fails if "Plain text only" is the active option.
  * New environment variables `"ConEmuDrive"` and `"ConEmuWorkDrive"`.


## Build 131006 ##
  * New option on ‘Startup’ page: ‘Auto save/restore of opened tabs’.
  * New environment variable `"ConEmuWorkDir"`, it contains folder from where your shell was started. User may use it when starting shell, with ‘ConEmu Here’ feature for example:
```
"C:\Program Files\Far Manager\far.exe" "!ConEmuWorkDir!"
```
  * ‘Default terminal’ settings are moved to own page. ‘Integration’ page contains only ‘ConEmu Here’ and ‘ConEmu Inside’.
  * After unregistering ‘ConEmu Here’ or ‘ConEmu Inside’ item drop down was refreshed but not edit fields.
  * [Issue 1269](https://code.google.com/p/conemu-maximus5/issues/detail?id=1269): More debug logging of Win+Left/Win+Right.
  * [Issue 1262](https://code.google.com/p/conemu-maximus5/issues/detail?id=1262): ConEmu.Editor.lua bug.
  * Debug purposes: "/basic" and "/resetdefault" switch.


## Build 130929 ##
  * [Issue 714](https://code.google.com/p/conemu-maximus5/issues/detail?id=714): Font size fixed when copying using CF\_HTML formatting. You may choose from Edit submenu one of the following options:
  * Plain text only (don't copy HTML formatting);
  * Copy HTML format (text will be ready for pasting in Word, Outlook etc.);
  * Copy as HTML (you want RAW HTML? choose this option).
    * Send cursor keys to vim with xterm control sequences.


## Build 130928 ##
  * Default anti-aliasing method will be ‘ClearType’ if it is turned on in your OS.
  * Infinite run loop when ‘Single instance’ was ON and ‘Multiple consoles’ was OFF.
  * GuiMacro: Close(8) - close all consoles.
  * New menu items in ‘Close or kill submenus’: ‘Close all consoles’ and ‘Close except active’.
  * [Issue 714](https://code.google.com/p/conemu-maximus5/issues/detail?id=714): Copying text to clipboard using CF\_HTML formatting. May be disabled from Edit submenu (Copy HTML format).


## Build 130923 ##
  * [Issue 1234](https://code.google.com/p/conemu-maximus5/issues/detail?id=1234): Tasks menu changes (Win+N, button `[+]`)
    * by default - does not expand to submenus, RClick on task - show submenu with task commands;
    * Shift+Click on `[+]` arrow - creates submenus for tasks, RClick on task - run all task commands;
    * also, user may set new hotkey for ‘Show create new console popup menu with task submenus’.
  * [Issue 1238](https://code.google.com/p/conemu-maximus5/issues/detail?id=1238): Running particular command from task submenus - was ignored ‘/dir ...’ switch.
  * Vista and higher: Default console font - ‘Consolas’ (if ClearType is enabled in system).
  * Vista and higher: Default tab and status bars font - ‘Segoe UI’.
  * _Default_ font height (console, tab and status bars) takes into account monitor DPI (e.g. 150%).
  * [Issue 1240](https://code.google.com/p/conemu-maximus5/issues/detail?id=1240): Window size wasn't saved on exit (sizing by grip in status bar).


## Build 130921 ##
  * [Issue 1242](https://code.google.com/p/conemu-maximus5/issues/detail?id=1242): DPI aware dialogs and font inconsistency.
  * App distinct palette settings was ignored when starting GuiMacro: `shell("new_console:In","powershell")`.
  * Crash in ConEmuC when attaching wide buffer consoles.
  * Some improvements on Default Terminal feature (to be continued).


## Build 130918 ##
  * [Issue 1239](https://code.google.com/p/conemu-maximus5/issues/detail?id=1239): Regression: commands with redirection fails in Far Manager.
  * [Issue 1137](https://code.google.com/p/conemu-maximus5/issues/detail?id=1137): Don't warn about Ctrl+Tab duplicate if internal Ctrl+Tab processing is disabled.
  * Regression: ‘Activate split on mouse over’ was not working.
  * [Issue 1125](https://code.google.com/p/conemu-maximus5/issues/detail?id=1125): Default terminal not activated when starting cmd from Windows 7 start menu Ctrl+Shift+Enter.


## Build 130917 ##
  * [Issue 1211](https://code.google.com/p/conemu-maximus5/issues/detail?id=1211): Redirection heuristic disabled. If one needs redirection in root command
```
call "cmd /c your-command-line"
```
  * [Issue 1228](https://code.google.com/p/conemu-maximus5/issues/detail?id=1228): In some cases ConEmu fails to display DBCS characters with "ANSI X3.64" enabled. Need to rewrite ANSI processor... Temporary workaround was created.
  * [Issue 1220](https://code.google.com/p/conemu-maximus5/issues/detail?id=1220): When starting "far /co" plugin ConEmu background fails to initialize.
  * [Issue 1230](https://code.google.com/p/conemu-maximus5/issues/detail?id=1230): Far plugin ConEmu background stops viewer autoupdating.
  * When starting "far /w" plugin ConEmu background fails to draw icons.
  * You may force Far Manager background plugins to update using Far macro. This may be useful for minimizing lags after Ctrl-O, Ctrl-O for example.
```
Plugin.Call("4b675d80-1d4a-4ea9-8436-fdc23f2fc14b", 99)
```
  * Some clarifications in ‘ConEmu received wrong version packet’ message.
  * Some fixes in padding painting.
  * Update Windows 7 jump list controls are moved to ‘Task bar’ settings page.
  * Far Manager tab templates are moved to ‘Far Manager’ settings page.


## Build 130913 ##
  * [Issue 1229](https://code.google.com/p/conemu-maximus5/issues/detail?id=1229): Special prefix symbols `'>'` and `'*'` (task item) was not stripped when starting new console from task menu.
  * Some changes in color palette processing.
  * New switch `"-new_console:P:PaletteName"` allows set fixed color palette for specified tab.
```
Example: ConEmu /cmd cmd -cur_console:P:^<PowerShell^>
```


## Build 130912 ##
  * Show icons in tab bar. By default, root application (your shell) icon is displayed. Also, you may specify any special icon when starting tab with -new\_console switch.
  * New switch `"-new_console:C:IconFile"` set icon for a tab. Example:
```
ConEmu.exe /cmd C:\GIT\git-bash.bat -cur_console:C:C\GIT\etc\git.ico
```
  * Changes in ‘Win+N’ popup menu (plus sign on toolbar). Tasks menu items are true submenu now. When task contains more than one item, ‘All task tabs’ sub-item appears to run all task tabs. You may quickly run all task tabs with RightClick on Task menu item itself (don't wait for submenu appears). Also, if you define more than 10 tasks, all sequent tasks are available via ‘More tasks’ submenus.
  * Default terminal: `ConEmuHk.dll` will be reloaded after updating to new ConEmu build (logoff/logon is not needed anymore).
  * Switch tabs by clicking over tab in fullscreen mode (actually, when caption is hidden).
  * Saving start command line history was disabled by default (130827). Enabled now.
  * Do you want to get your cmd.exe prompt bottom aligned? On ConEmu startup and after "cls"? It is possible, run this file as your shell:
```
cmd.exe /k ClsEx.cmd
```


## Build 130827 ##
  * New search sequence for `ConEmu.xml` file:
```
* %APPDATA%\ConEmu.xml
* %ConEmuBaseDir%\ConEmu.xml
* %ConEmuDir%\ConEmu.xml
```
  * New user may choose settings storage location on the first start (Fast configuration dialog).
  * New option `"SaveCmdHistory"`, may be changed via reg/xml configuration.
  * [Issue 1210](https://code.google.com/p/conemu-maximus5/issues/detail?id=1210): Don't reset default command specified with "/cmd" argument in Quake mode.
  * Wrong Quake position (top) when "Pad size" was set.
  * When running "-new\_console:sVb" - "b" was ignored.
  * [Issue 1090](https://code.google.com/p/conemu-maximus5/issues/detail?id=1090): Win+Left/Right on three monitors once again.


## Build 130822 ##
  * [Issue 1195](https://code.google.com/p/conemu-maximus5/issues/detail?id=1195): Assertion.
  * [Issue 1198](https://code.google.com/p/conemu-maximus5/issues/detail?id=1198): Crash when exporting xml configuration to read-only folder.
  * Split was erroneously activated by mouse hover while any menu (tab, system, etc.) was active.
  * Trying to search "msxml3.dll" in %PATH% when it is not available via COM and is not found in the ConEmu's folder.
  * Fails to typing japanese/chinese in the python prompt.


## Build 130815 ##
  * [Issue 1183](https://code.google.com/p/conemu-maximus5/issues/detail?id=1183): Sometimes Far x64 crashes on context menu.


## Build 130814 ##
  * Sometimes 0-size crash dump files were created.


## Build 130813a ##
  * [Issue 1193](https://code.google.com/p/conemu-maximus5/issues/detail?id=1193): PowerShell: Get-Credential crashes. Bug appears in 130807.
  * ConEmu About dialog was broken.


## Build 130813 ##
  * Crashed sometimes with `"ConEmu /log"`.


## Build 130812 ##
  * [Issue 1191](https://code.google.com/p/conemu-maximus5/issues/detail?id=1191): Due to Explorer bug (or weird feature?) ConEmu was launched instead of explorer from taskbar pinned library icon. Registry keys (related to "ConEmu here") will be fixed on ConEmu launch.
  * New option on "Features" page - "Exception handler (debug)". When checked - unhandled exception filter will be installed in alternative servers.


## Build 130809 ##
  * [Issue 1179](https://code.google.com/p/conemu-maximus5/issues/detail?id=1179): Can't disable text cursor blinking when ConEmu window is inactive.
  * conemu\_ml: TCC fails while executing: `""F:\program files\take command\tcc.exe" /C "alias where" "`.
  * [Issue 1090](https://code.google.com/p/conemu-maximus5/issues/detail?id=1090): Tile left/right fails to move ConEmu window between monitors.


## Build 130808 ##
  * [Issue 1181](https://code.google.com/p/conemu-maximus5/issues/detail?id=1181): Activate pane by mouse over works now with GUI child applications.
  * Option "Activate split on mouse over" is disabled by default.
  * Internal log changing: `"ConEmu /log /single"` will affect existing instance.


## Build 130807 ##
  * Forbid Far Manager plugins to change window styles of ConEmu windows.
  * Some fixes in Far Manager command execution interceptor.
  * New color scheme: ‘`<Solarized (Luke Maciak)>`’.
  * New switch "-new\_console:f" forces starting console active. May be useful when starting several consoles simultaneously. Example:
```
ConEmu.exe /cmdlist powershell -new_console:f ||| cmd -new_console:sH
```
  * Wrong size and pos of child GUI windows in split.
  * Hotkey ‘Close current tab’ must close group when option ‘One tab per group’ is enabled.
  * GuiMacro: Close(6) - close active tab or group, Close(7) - all active processes of the active group.
  * Activate pane by mouse over works now with GUI child applications.


## Build 130726 ##
  * [Issue 1133](https://code.google.com/p/conemu-maximus5/issues/detail?id=1133): New option on "Status" page: "Use vertical padding". With unchecked you may use same status bar height and font as font in console.
  * [Issue 1158](https://code.google.com/p/conemu-maximus5/issues/detail?id=1158): Provide "ConEmu Here" menu item for Windows 7 libraries. Have no sense for library root - select smth or open library folder.
  * [Issue 1159](https://code.google.com/p/conemu-maximus5/issues/detail?id=1159): Crash when trying to put focus in bottom split while there was horizonal split only.
  * [Issue 1159](https://code.google.com/p/conemu-maximus5/issues/detail?id=1159): Split to bottom and split to right hotkeys was messed up.


## Build 130725 ##
  * GuiMacro: new function
```
Split(<Cmd>,<Horz>,<Vert>)
    Cmd=0, Horz=1..99, Vert=0: Duplicate active ‘shell’ split to right, same as Shell("new_console:s50Hn")
    Cmd=0, Horz=0, Vert=1..99: Duplicate active ‘shell’ split to bottom, same as Shell("new_console:s50Vn")
    Cmd=1, Horz=-1..1, Vert=-1..1: Move splitter between panes (aka resize panes)
    Cmd=2, Horz=-1..1, Vert=-1..1: Put cursor to the nearest pane (preferred direction may be choosed).
```
  * New hotkeys: Apps+Shift+Arrows - resize splits, Apps+Arrows - put focus to next split.
  * Far 3 macro `"ConEmu.ShiftEnter.lua"` updated.


## Build 130722 ##
  * Build number stored in xml-configuration file when saving settings.
  * Keys & Macros: "Drag ConEmu window by client area" renamed to "Move ConEmu window by dragging client area".
  * Settings->Mark & Paste: New option "Show IBeam cursor".
  * [Issue 1152](https://code.google.com/p/conemu-maximus5/issues/detail?id=1152): Hungs on wheel when "sreen per time" was choosed in mouse settings.
  * Issue ???: Don't resize real console until mouse button released (Far Manager still resized ‘on the fly’).
  * [Issue 1141](https://code.google.com/p/conemu-maximus5/issues/detail?id=1141): Tab bar position fails after restoring ConEmu when ‘Auto show’ (tabs) is selected.
  * Some more internal logging.
  * Macro ConEmu.ShiftEnter.lua fails in Far 3.


## Build 130708 ##
  * Far Macro `"ConEmu.Editor.fml"` was changed! Update your copies in MacroLib!
  * [Issue 1126](https://code.google.com/p/conemu-maximus5/issues/detail?id=1126): ConEmu macros adopted for Far Manager 3 (tested in build 3502). New macros located in `"%ConEmuBaseDir\Far3_lua"`. You need to copy selected files to your `"%FARPROFILE%\Macros\scripts"` folder.
  * Right click on ‘Minimize’ button hides ConEmu to TSA.
  * [Issue 1123](https://code.google.com/p/conemu-maximus5/issues/detail?id=1123): "Close(5)" crashes.


## Build 130703 ##
  * GuiMacro delimiting fixed. Examples:
```
print("qqq"); WindowMinimize();
print("qqq") WindowMinimize()
print: "qqq"; WindowMinimize
```
  * GuiMacro: Close(5) - close all tabs but active.
  * [Issue 1130](https://code.google.com/p/conemu-maximus5/issues/detail?id=1130): Decreasing font may stuck on some values.
  * Some fixes in build files.


## Build 130702 ##
  * GuiMacro: new option "I" (capital 'i', case sensitive) for "shell" function. It forces inheriting of root process contents, like "Duplicate root" feature.
```
Example: shell("new_console:Ibn","cmd","/k start explorer \"%CD%\" && exit")
```
  * Debug logging of macro function "FontSetSize".
  * [Issue 1113](https://code.google.com/p/conemu-maximus5/issues/detail?id=1113): New option "Tab button double click action".
  * 130621 regression: all group tabs was closed even if "One tab per group" was not checked.
  * [Issue 929](https://code.google.com/p/conemu-maximus5/issues/detail?id=929): New option "Activate split on mouse over".


## Build 130625 ##
  * [Issue 1096](https://code.google.com/p/conemu-maximus5/issues/detail?id=1096): Possible crash while executing some GuiMacro.
  * [Issue 1107](https://code.google.com/p/conemu-maximus5/issues/detail?id=1107): Some unassigned hotkeys can't be hidden in Options dialog.
  * Some internal changes in GUI resize.


## Build 130621 ##
  * Show IBeam cursor when mouse is used for selection.
  * Checking conflicts between key modifiers for text selection and Far Manager D&D.
  * Close confirmation from tab belonging to group (splits) will suggest closing whole group.
  * [Issue 978](https://code.google.com/p/conemu-maximus5/issues/detail?id=978): New option "One tab per group", disabled by default. Weird with Far Manager editor windows yet.
  * [Issue 1095](https://code.google.com/p/conemu-maximus5/issues/detail?id=1095): Transparency was not changed after Quake activation.
  * Incorrect cursor position in console after vim exiting. Don't forget enable ‘Inject ConEmuHk’!
  * [Issue 1103](https://code.google.com/p/conemu-maximus5/issues/detail?id=1103): GuiMacro: `SetOption("bgImageDarker",<value>)` - change darkening of background image, 0..255.


## Build 130612 ##
  * [Issue 1090](https://code.google.com/p/conemu-maximus5/issues/detail?id=1090): Regression: Win+Left/Right does not move ConEmu anymore between multiple monitors (beginning).
  * Regression: Wrong working directory in recreate dialog.
  * Automatic crashdump creation when exception occures in ConEmuC.
  * ConEmu may close unexpectedly when clicking on ConEmu system icon while GUI child application is running in tab.


## Build 130604 ##
  * [Issue 1080](https://code.google.com/p/conemu-maximus5/issues/detail?id=1080), [Issue 1082](https://code.google.com/p/conemu-maximus5/issues/detail?id=1082): Regression: 5 sec delay when starting new console.
  * [Issue 1083](https://code.google.com/p/conemu-maximus5/issues/detail?id=1083): Process Win+Left/Win+Right internally (tile to left/right).
  * [Issue 1087](https://code.google.com/p/conemu-maximus5/issues/detail?id=1087): cmd autoattach ignores disabled multi console option.
  * Some fixes in "ConEmuC /export ...".
  * Debug purposes: new environment variable (may be set by user) `ConEmuSleepIndicator=NUM`.


## Build 130530 ##
  * One more fix for "Sleep in backgound".
  * Wheel was not working while mouse was over scrollbar.
  * Buffer (scrolling and contents) was not restored after Vim exit. Don't forget enable ‘Inject ConEmuHk’!
  * When Vim started current buffer output stored, and may be viewed by as Alternative buffer (‘lock’ button on toolbar or Win+A by default).
  * [Issue 1077](https://code.google.com/p/conemu-maximus5/issues/detail?id=1077): Sometimes assertion (keyboard layout related) may occurs on startup.


## Build 130526 ##
  * One more fix for "Sleep in backgound".
  * [Issue 1076](https://code.google.com/p/conemu-maximus5/issues/detail?id=1076): Regression: Second instance (with single instance mode) always starts in `%ConEmuDir%`.
  * New GuiMacro: `Copy(<What>)` - Copy active console contents
  * What==0: current selection
  * What==1: all buffer contents


## Build 130523 ##
  * [Issue 1049](https://code.google.com/p/conemu-maximus5/issues/detail?id=1049): Don't lock current (startup) directory.
  * Width and Height may be set up in cells/pixels/percents.
    * For example, Width = "90%", Height = "400px".
    * Without modifier - treated as cells.
  * New option ‘Align ConEmu window size to cells’ on ‘Size & Pos’ page.
  * Some changes in ANSI processor.
  * Minor changes in mouse-over-scrollbar processing.
  * [Issue 1004](https://code.google.com/p/conemu-maximus5/issues/detail?id=1004): overflow of SetTitle when "/title" option was specified.
  * [Issue 1067](https://code.google.com/p/conemu-maximus5/issues/detail?id=1067): missing linking dependency for ConEmuDW project.
  * [Issue 1068](https://code.google.com/p/conemu-maximus5/issues/detail?id=1068): UI interface for ‘Default tab click action’ (patch from peter.sunde).
  * [Issue 1069](https://code.google.com/p/conemu-maximus5/issues/detail?id=1069): better clink error message (check box ‘Use clink in prompt’).
  * Some more internal logging.
  * [Issue 868](https://code.google.com/p/conemu-maximus5/issues/detail?id=868): Allow change quake-mode window size from Settings dialog.
  * Allow set position (X-coord) of quake-mode window size. Choose radio "Fixed" instead of "Cascade" before.
  * [Issue 852](https://code.google.com/p/conemu-maximus5/issues/detail?id=852): New menu item ‘Copy all’, this will copy all text from the beginning of the buffer to the cursor line (including).


## Build 130427 ##
  * [Issue 1035](https://code.google.com/p/conemu-maximus5/issues/detail?id=1035), [Issue 1050](https://code.google.com/p/conemu-maximus5/issues/detail?id=1050): Keyboard layouts once more.
  * [Issue 1047](https://code.google.com/p/conemu-maximus5/issues/detail?id=1047): Assertion on start file dragging.
  * [Issue 1052](https://code.google.com/p/conemu-maximus5/issues/detail?id=1052): New option ‘Show balloon help tooltips’ on ‘Appearance’ page.

## Build 130425 ##
  * [Issue 1040](https://code.google.com/p/conemu-maximus5/issues/detail?id=1040): Resize Far 3 panels macro error.
  * [Issue 1035](https://code.google.com/p/conemu-maximus5/issues/detail?id=1035): Assertion on startup with some keyboard layouts (again).


## Build 130422 ##
  * [Issue 1035](https://code.google.com/p/conemu-maximus5/issues/detail?id=1035): Assertion on startup with some keyboard layouts.


## Build 130421 ##
  * Option ‘Inject ConEmuHk’ is enabled by default (for new configurations).
  * Some more internal logging.
  * Three-level autoupdate: Stable, Preview, Developer.
  * [Issue 944](https://code.google.com/p/conemu-maximus5/issues/detail?id=944): With some keyboard layouts ConEmu starting may cause loading "fantom" language.
  * Options ‘Focus in child windows’ was ignored when activating ConEmu.


## Build 130411 ##
  * Quit on close once again... If no opened consoles and cross clicking - ConEmu will terminates.
  * [Issue 1014](https://code.google.com/p/conemu-maximus5/issues/detail?id=1014): ‘Extend Fonts’ feature does not work (was locked to far.exe only).
  * [Issue 1011](https://code.google.com/p/conemu-maximus5/issues/detail?id=1011): Typo in installer.
  * Some more logging on keyboard input.
  * Allow to set tab templates without "%s" and "%n".
  * Far 3 (Lua) macro fails: Plugin.Call("bd454d48-448e-46cc-909d-b6cf789c2d65",1).
  * [Issue 1020](https://code.google.com/p/conemu-maximus5/issues/detail?id=1020): Far 3 build 3277 supported.


## Build 130401 ##
  * Some more internal diagnostics.
  * [Issue 1000](https://code.google.com/p/conemu-maximus5/issues/detail?id=1000): Gestures was not working on Windows x64.
  * [Issue 1003](https://code.google.com/p/conemu-maximus5/issues/detail?id=1003): ConEmu autorun command processor disables testing movie in flash.


## Build 130320 ##
  * [Issue 998](https://code.google.com/p/conemu-maximus5/issues/detail?id=998): Default terminal may fails with error code 31 when starting debugging session in VS.


## Build 130319 ##
  * [Issue 924](https://code.google.com/p/conemu-maximus5/issues/detail?id=924): ConHost.exe crashed when maximizing.
  * [Issue 974](https://code.google.com/p/conemu-maximus5/issues/detail?id=974): ‘Leave ConEmu opened’ blocks autoupdate.
  * [Issue 990](https://code.google.com/p/conemu-maximus5/issues/detail?id=990): Regression. ANSI processor was not initialized properly.
  * [Issue 994](https://code.google.com/p/conemu-maximus5/issues/detail?id=994): Regression. Clink loading failed.


## Build 130318 ##
  * [Issue 984](https://code.google.com/p/conemu-maximus5/issues/detail?id=984): Option "Add %ConEmuDir% to %PATH%" on ComSpec page. Turned ON by default.
  * [Issue 915](https://code.google.com/p/conemu-maximus5/issues/detail?id=915): Closing options redesigned. Taskbar page, checkboxes
    * Close ConEmu with last tab
    * Quit on close (e.g. caption bar cross clicking)
    * Minimize on closing last tab
    * Hide to the TSA
  * Some changes in console handling.
  * Failed to disable "Use clink in prompt" in active session (ConEmu restart was needed).


## Build 130314 ##
  * [Issue 858](https://code.google.com/p/conemu-maximus5/issues/detail?id=858): Drop files to Far panels failed second time.
  * Some internal changes.


## Build 130313 ##
  * Scrollbar was not appears/works when mouse was over padding on right edge.
  * Updated debug switch: `"ConEmuC /debugtree <command_line>"`.
  * Flickering when starting Quake mode in TSA.
  * Drawing artefacts in the Status bar when resizing window.
  * [Issue 969](https://code.google.com/p/conemu-maximus5/issues/detail?id=969): Disabling "Multiple consoles in one ConEmu window" has no effect for "Default terminal".
  * The continue of "/debugtree".


## Build 130305 ##
  * Scrollbar was not appears/works when mouse was over padding on right edge.
  * Updated debug switch: `"ConEmuC /debugtree <command_line>"`.


## Build 130304 ##
  * Regression: GUI applications attach failed.
  * [Issue 971](https://code.google.com/p/conemu-maximus5/issues/detail?id=971): Sending dead chars to child GUI applications.
  * [Issue 972](https://code.google.com/p/conemu-maximus5/issues/detail?id=972): Regression: Duplicate root failed.


## Build 130228 ##
  * [Issue 964](https://code.google.com/p/conemu-maximus5/issues/detail?id=964): Duplicate Root shows an assert.
  * [Issue 965](https://code.google.com/p/conemu-maximus5/issues/detail?id=965): Regression: stored fixed position for normal mode was ignored.
  * Settings->Info: showing "conhost.exe" in processes list for Windows 8 and higher.
  * New debug switch: `"ConEmuC /debugtree <command_line>"`.


## Build 130227 ##
  * Win8: Resize bugfix.
  * [Issue 592](https://code.google.com/p/conemu-maximus5/issues/detail?id=592): Quake-style autohide when clicking on taskbar.
  * [Issue 888](https://code.google.com/p/conemu-maximus5/issues/detail?id=888): Fixing MinTTY bug.
  * [Issue 960](https://code.google.com/p/conemu-maximus5/issues/detail?id=960): Sometimes ConEmu crashes while attaching external console (in progress).
  * Crash on GuiMacro WindowMaximize().


## Build 130223 ##
  * Regression: Hotkey names not displayed on ‘Keys & Macro’ page.
  * Some focus-switch hotkeys was not working.
  * Switch "/demote" for ConEmu.exe allows to run commands de-elevated via Task Sheduler.
  * AutoUpdate fixes.
  * Don't execute ‘ConEmu autorun’ when console window is minimized (AutoUpdate issue).
  * ‘Duplicate root’ fails sometimes with error code 0xC0000148.


## Build 130220 ##
  * [Issue 949](https://code.google.com/p/conemu-maximus5/issues/detail?id=949): Selection failed with buffer and ‘Freeze console’ checked.


## Build 130219 ##
  * [Issue 592](https://code.google.com/p/conemu-maximus5/issues/detail?id=592): Quake activation/deactivation changes.
  * [Issue 912](https://code.google.com/p/conemu-maximus5/issues/detail?id=912): Internal processing of Win+Shift+Left/Right - move ConEmu window to previous/next monitor.
  * [Issue 939](https://code.google.com/p/conemu-maximus5/issues/detail?id=939): Update failes if full path to 7za.exe was not specified, but 7za.exe is located in %PATH%.
  * [Issue 946](https://code.google.com/p/conemu-maximus5/issues/detail?id=946): Exception in some cases running with "/Log2" switch.
  * [Issue 946](https://code.google.com/p/conemu-maximus5/issues/detail?id=946): Blinking when ConEmu starting minimized.
  * Update: Failed with proxy requires authentification.


## Build 130212 ##
  * [Issue 912](https://code.google.com/p/conemu-maximus5/issues/detail?id=912): Resize changes (Full Screen Does Not Allow Win+Shift+Arrow to Move Monitors).
  * [Issue 937](https://code.google.com/p/conemu-maximus5/issues/detail?id=937): Drag&Drop once more (AIMP3).
  * [Issue 938](https://code.google.com/p/conemu-maximus5/issues/detail?id=938): Double click for word selection stops working after a while.


## Build 130210 ##
  * Regression. Sometimes ConEmu.exe may crash on startup or quake-sliding.
  * [Issue 932](https://code.google.com/p/conemu-maximus5/issues/detail?id=932): Support UltraEdit decimal color format.


## Build 130208 ##
  * Interprocess communication changes.
  * [Issue 926](https://code.google.com/p/conemu-maximus5/issues/detail?id=926): Pad around console redraw lag.
  * [Issue 928](https://code.google.com/p/conemu-maximus5/issues/detail?id=928): Regression of 130205: Restart of console does not works.
  * Preparing for multi-page selection.


## Build 130205 ##
  * Some improvements for ‘stress mode’.
  * ‘Disabled’ will be shown in ‘Keys & Macro’ if specified action is disabled in settings.
  * Some bugfixes in Drag&Drop to Far panels from some storages.
  * Checking 7zip/WinRar existence before updating with 7z package.
  * Some changes in Status bar drawing.
  * Removed all references to std::vector.
  * [Issue 885](https://code.google.com/p/conemu-maximus5/issues/detail?id=885): Attach of ‘As Administrator’ console fixed.
  * [Issue 886](https://code.google.com/p/conemu-maximus5/issues/detail?id=886): Button ‘?’ in the Title Bar of ‘Settings’ dialog.
  * Minor changes of resources in ‘Settings’ dialog.
  * Tooltips in ‘Settings’ dialogs was not visible with this registry value:
```
[HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced]
"EnableBalloonTips"=0
```



## Build 130128 ##
  * Refactoring of drag from Far Manager panels is started.
  * Changes in self-update procedure (elevation related).
  * [Issue 899](https://code.google.com/p/conemu-maximus5/issues/detail?id=899): Default position of ‘Find text’ dialog is upper-right corner.
  * Some changes in Drag&Drop items drawing.
  * Bugfixes for Reload/Reset Settings (buttons in ‘Settings’ dialog).
  * Regression. Can't select some modifiers on ‘Keys & Macro’ page in ‘Settings’ dialog.
  * ConEmu.exe switch changed to avoid ambiguity. The following command line will run 3 tabs on ConEmu startup:
```
"ConEmu.exe /cmdlist cmd1 ||| cmd2 ||| cmd3".
```
  * Use Vista+ ‘Task dialog’ for closing confirmation.


## Build 130120 ##
  * Settings in ‘Create new console’ dialog was ignored for Tasks.
  * [Issue 706](https://code.google.com/p/conemu-maximus5/issues/detail?id=706): ConEmu Inside: You must use in Sync command "\1" and "\2" instead of "%1" and "%2". Also, you need to re-register menu items.
  * Vim 256 colors next try. Hope, it will be usable now. You need to configure your vimrc for enabling 256 colors. Sample lines are here: http://conemu-maximus5.googlecode.com/svn/files/Temp/vimrc_add


## Build 130117 ##
  * Excess console window flickering removed, when elevated tab is starting from elevated ConEmu.
  * Minor bugfix for ‘Single instance mode’.
  * Quake animation when hide to TSA is disabled.
  * New switch `"-new_console:w"` turns on overwrite mode in console (for cmd, powershell, etc.)
  * One step closer to Vim 256 colors.


## Build 130116 ##
  * [Issue 883](https://code.google.com/p/conemu-maximus5/issues/detail?id=883): PuTTY resize again.
  * Failed to change cursor ‘Blinking’ for Inactive console.
  * Two new compelental hotkeys: ‘Set focus to ConEmu’, ‘Set focus to child GUI application’.
  * New item in tab menu: ‘Child system menu’.
  * Removing excess redraws.
  * New switch: `"ConEmu.exe /nosingle"`.
  * New option in ‘Fast configuration’ and ‘Appearance’ page: ‘Single instance mode (use existing window instead of running new instance)’.
  * Minor changes in ‘Mark & Paste’ page: new checkbox ‘copy before reset’ instead of third state of ‘Reset selection on input’.


## Build 130113 ##
  * Searching in ‘Settings’ dialog now available. Type text in ‘Find’ editbox (upper-left corner) and wait a little. You may continue search by pressing ">" button.
  * New option ‘Minimize on focus lose’ on ‘Task bar’ page. This is used only for ‘normal’ style, ‘quake’ style has separate option.
  * New switch. Now it is possible to start several tabs from the command line or shortcut without creating tasks.
```
ConEmu.exe /cmdlist cmd1 | cmd2 | cmd3
```
  * [Issue 864](https://code.google.com/p/conemu-maximus5/issues/detail?id=864): MiddleButtonClick on free tabbar area creates new tab.
  * [Issue 871](https://code.google.com/p/conemu-maximus5/issues/detail?id=871): Sometimes inactive consoles was not resized properly.
  * URL-detector does not strip single-quote from the end of URL.
  * Dropping long filepathnamed to prompt sometimes fails (pasted paths was trimmed).
  * [Issue 878](https://code.google.com/p/conemu-maximus5/issues/detail?id=878): Can't select in putty after moving ConEmu window.
  * New option ‘Focus in child windows’. Enabled by default. It means set focus in xshell/putty/notepad/etc. when ConEmu is activated or tab switched.
  * New hotkey (not set by default): Switch focus between ConEmu and child GUI application (xshell/putty/notepad/etc.)
  * Now you may export your current configuration to xml-file directly from Settings dialog.
  * Current settings storage location displayed on the top of Settings dialog.


## Build 130109 ##
  * Sometimes, trap occures when switching to tab which is in ‘closing state’.
  * ‘Resize mark’ renamed to ‘Size grip’.
  * ‘Default terminal’ fails to execute smth like "cmd /k copy aaa bbb".


## Build 130108 ##
  * New ‘column’ in Status bar - ‘Resize mark’. You can change ConEmu size by dragging this ‘column’ even when frame is hidden.
  * Starting shells takes into account paths defined in "SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths".
  * Minor changes in starting root shell.
  * Toolbar: when last console is closed - ConEmu hides button with console number.
  * Toolbar buttons order was changed. [+] button on first place now.
  * Option ‘Hide caption of child GUI windows’ is enabled by default.
  * Quake-style options moved to ‘Appearance’ page.
  * New Quake-style option for animation duration. Any value between 0 and 2000 ms allowed.
  * Minor fixes in "/reset" switch.
  * Fixed focus of child GUI windows after ConEmu window restore or tab switch.


## Build 130105 ##
  * [Issue 865](https://code.google.com/p/conemu-maximus5/issues/detail?id=865): Frame width option is broken.
  * [Issue 864](https://code.google.com/p/conemu-maximus5/issues/detail?id=864): MiddleButtonClick on free tabbar area creates new tab.
  * Debug purposes: new ConEmuC argument `/DEBUGEXE <command line>`.
  * Due to large amount of ANSICON-related reports its loading will be blocked in ConEmu.
  * Don't show console font warning in ReactOS.


## Build 130104 ##
  * Fixed Maximize/Restore regression. On Restore console size was not reduced.
  * [Issue 806](https://code.google.com/p/conemu-maximus5/issues/detail?id=806): GVim started as `"gvim --windowid !ConEmuBackHWND!"` does not appears when switching tabs.
  * Fixed regression in hotkeys processing. Far received RAlt after Shift+Arrow.
  * Changes in Quake-style animation. Slide window when possible.
  * PuTTY dialogs appears beneath ConEmu window, then "Always on top" is checked.
  * xterm 24-bit colors supported. Same restrictions as xterm 256 colors.


## Build 130102 ##
  * GuiMacro parser regression. Only one string argument was processed.


## Build 130101 ##
  * When unhandled exception occurred in ConEmu - full memory dump will be created on desktop in "ConEmuTrap" folder.
  * New options for ‘Esc’ key or ‘Task bar’ page: ‘Minimize ConEmu when all consoles closed’, ‘Minimize always’ and ‘Never’. May be useful in Quake mode.
  * When ‘Esc’ is always used for minimize ConEmu, you may use Shift+Esc to send simple Esc to console. Uncheck ‘Map Shift+Esc to Esc’ on ‘Task bar’ page if you don't need this mapping.
  * Resize bugfixes in Quake mode.
  * [Issue 853](https://code.google.com/p/conemu-maximus5/issues/detail?id=853): Default terminal fails in TotalCmd ‘Open command prompt window’.
  * New toolbar images by Grzegorz Kozub.
  * New option on ‘Appearance’ page: ‘Hide caption of child GUI windows started in ConEmu’. Turned ON by default.
  * Regression. Tab activation failed when it is inactive split.
  * New environment variable "ConEmuHooks". You may set it to "OFF" to disable injects temporarily. Example:
```
cmd /k set ConEmuHooks=OFF & "C:\Program Files\Microsoft SDKs\Windows\v7.1\Bin\SetEnv.cmd" -new_console:t:SDK & set ConEmuHooks=
```
  * Some changes in drag&drop files to Far panels.


## Build 121227 ##
  * New SystemMenu item: About -> Whats new (local, web).
  * Far 3 plugins updated to new API (some changes was missed).


## Build 121225 ##
  * GuiMacro engine upgraded to avoid ambiguity. Now there are C-Strings and Verbatim-Strings.
    * C-Strings: allows backslash-escape sequences, for example `print("\ecd /d \"C:\\Users\"\n")`
    * Verbatim-Strings: allows using backslashes without escapes, for example `print(@"cd /d ""C:\Users""")`
  * Due to a bug(?) in MS DOM saving GuiMacro with (Esc), for example `print("\eexit\n")`, was caused erasing of `ConEmu.xml` file contents.
  * Group ‘Multi console’ moved from ‘Tabs’ page to ‘Appearance’ page.
  * New option ‘Show buttons (toolbar) in tab bar’ on ‘Appearance’ page.
  * [Issue 849](https://code.google.com/p/conemu-maximus5/issues/detail?id=849): Transparency breaks on startup when ‘Hide caption’ is on.
  * Debug build. Menu item ‘Dump used memory blocks’ in ‘Debug’ submenu.
  * Some internal changes.
  * Some Attach from Far Plugin bugfixes.
  * ‘App distinct’ updated with new Cursor controls.
  * Shift+Arrows may start selection (new option on ‘Mark & Paste’ and ‘App distinct’ pages). Shift+Left/Right starts text selection, Shift+Up/Down starts block selection.
    * Note! Shift+Arrows don't start selection in Far Manager.


## Build 121216 ##
  * [Issue 846](https://code.google.com/p/conemu-maximus5/issues/detail?id=846): New predefined color palette `<Solarized>`.
  * Sometimes console was not resized after maximizing ConEmu window. TechInfo: because of larget font size in **real console**. Auto-reducing automatically when possible.
  * Sometimes close console confirmation ‘Press Enter or Esc to close console’ was bypassed.
  * Cursor setting page was redesigned. Inactive cursor may be fully configured now. To be continued, some combinations not works yet.
  * New settings page ‘Task bar’. Some controls from ‘Appearance’ page are here now.
  * New option ‘Auto hide ConEmu after last console close’. Must be used together with ‘Leave ConEmu after last console close’.
  * New `ConEmu.exe` switches:
    * `/MinTSA` - almost like combination of ‘/Min /TSA’ but implies ‘Leave ...’ and ‘Auto hide after last console close’.
    * `/SetDefTerm` - forces installation of ConEmu as default terminal. May be useful with ‘/MinTSA /Detached’.
  * Now you can turn on ‘Register on OS startup’ for ‘Default terminal’ feature. ‘Integration’ page.


## Build 121211 ##
  * [Issue 824](https://code.google.com/p/conemu-maximus5/issues/detail?id=824): Ctrl+BackSpace deletes word leftward to the cursor. Ignored in Far Manager. Hotkey is configurable. ATM takes care of ‘Change prompt with Left Click’ third state.
  * [Issue 707](https://code.google.com/p/conemu-maximus5/issues/detail?id=707): ConEmu Inside: WinXP: Trying to show ‘Tip pane’ automatically and close it after ConEmu exits.
  * [Issue 838](https://code.google.com/p/conemu-maximus5/issues/detail?id=838): New cursor type ‘Block’. With ‘Color’ checked ConEmu draws ‘empty’ rectangle around cell under cursor.
  * [Issue 842](https://code.google.com/p/conemu-maximus5/issues/detail?id=842): Far 3 build 3000: `[string "if APanel.Left Keys("Tab") end"]:1: 'then' expected near 'Keys'`.
  * Some resize bugfixes.


## Build 121206 ##
  * Timeout for downloading files (Auto-Update) increased to 180 seconds.
  * [Issue 743](https://code.google.com/p/conemu-maximus5/issues/detail?id=743): Sometimes Auto-Update fails via proxy.
  * Refactoring of menus processing.
  * Shell style Drag&Drop allows drops into inactive panes (split-screen). To be more exact pane will be activated on drag-over.
  * Sometimes ‘Quake-style slide down’ shows window without sliding.
  * New file in ConEmu folder `"TypeBuildErrors.cmd"`. This may be used for colorization errors and warnings in the compiler outputs.
  * Some more information with "/log2".
  * [Issue 834](https://code.google.com/p/conemu-maximus5/issues/detail?id=834): Too large FPS while using Far 2 truemod.
  * Redesign of ‘Mark & Paste’ settings page.


## Build 121128 ##
  * Some changes in console server & ExtendedConsole.dll.
  * -new\_console:z
  * ‘Default terminal’ - more flexible:
    * active config name used;
    * close confirmation configurable;
    * don't inject hooks configurable;
    * take care about started with debugging;
    * still available after GUI was closed;
    * "-new\_console:z" may be used when you need to force real console instead of replacing as ‘default terminal’.
  * [Issue 828](https://code.google.com/p/conemu-maximus5/issues/detail?id=828): Auto-hidden taskbar and maximized ConEmu without Caption.


## Build 121125 ##
  * GCC warnings and errors.
  * Empty passwords are allowed when ‘Run as user’ (if they are allowed by local policy of course).
  * When ‘Run as user’ trying to use %USERPROFILE% instead of "System32".


## Build 121124 ##
  * Experimental! ConEmu may set up itself as default terminal for console applications.
    * Warning! This feature may raise false alarms in some antiviral programs!
    * Warning! Autoupdate with 7z package may fails if this feature is enabled!
    * Settings -> Integration -> Force ConEmu...
    * Note! In the current build active ConEmu window required for processing interception.
    * TechInfo: When enabling this feature ConEmu calls CreateRemoteThread for injecting ConEmuHk.dll into selected processes (explorer.exe by default). This allows to intercept CreateProcess WinAPI function and replace it with ConEmu.exe...
    * There is new ConEmu.exe switch "/nodefterm" - don't start initialization procedure.
  * [Issue 825](https://code.google.com/p/conemu-maximus5/issues/detail?id=825): ANSI X3.64 sequences don't works with Gow ls.
  * [Issue 819](https://code.google.com/p/conemu-maximus5/issues/detail?id=819): Regressions in hooks (build 121028).
  * Options ‘Skip click on activation’ applies to the click on the inactive split-pane.
  * [Issue 667](https://code.google.com/p/conemu-maximus5/issues/detail?id=667): (May be fixed) ConEmu has incorrect Z-order in 'Vista Switcher'.
  * [Issue 827](https://code.google.com/p/conemu-maximus5/issues/detail?id=827): Can't disable ‘Sleep in background’.


## Build 121121 ##
  * Settings -> Keys & Macro. Checkbox ‘Hide unassigned’ temporarily hides from list actions and macros without hotkeys.
  * [Issue 700](https://code.google.com/p/conemu-maximus5/issues/detail?id=700): WinXP, Default history buffers count too small.


## Build 121119 ##
  * Bugfix. 10 sec delay when starting Quake-style or "/single" ConEmu.
  * Apps+Tab and Apps+Shift+Tab switches between visible split-panes.
  * New item in tab popup menu: Terminate -> Active group. Also available from System menu -> Active console.
  * New hotkey (unassigned by default) for close of active group.


## Build 121118 ##
  * For debug purposes. User may define environment variable `"ConEmuReportExe"`. ConEmuHk (when it is injected in the process with defined name) will show message box. It is right moment to take memory dump of this infiltrated process. For example (for cmd.exe command prompt):
```
    set ConEmuReportExe=sh.exe
    "C:\Program Files (x86)\Git\bin\sh.exe" --login -i
```
  * [Issue 811](https://code.google.com/p/conemu-maximus5/issues/detail?id=811): Unable to change Highlight & goto shortcut.
  * When ‘Don't close ConEmu on last console close’ is on it was impossible to close empty ConEmu window.
  * Split key macros (default as Ctrl+Shift+O & Ctrl+Shift+E) works now like ‘Duplicate root...’ menu item.
  * Settings page ‘Status bar’ redesigned: two listboxes (available and selected) instead of checkboxes.
  * [Issue 814](https://code.google.com/p/conemu-maximus5/issues/detail?id=814): highlight & goto Far Manager macro error bugfix.
  * New switch: "-new\_console:t:tabname". Allows rename new created tab.
  * GuiMacro: Tab(10,1) and Tab(10,-1) cycle switches visible split-panes.
  * [Issue 818](https://code.google.com/p/conemu-maximus5/issues/detail?id=818): More DBCS console bugfixes.


## Build 121109 ##
  * Fast access to ‘Keys & Macro’ (hotkeys) settings page. Win+Alt+K or SystemMenu -> Help -> Hotkeys.
  * New hotkeys: Win+Alt+Del - close active tab, Win+F4 - close all tabs (same as ‘Cross’ click).
  * When closing tab from Win7 taskbar thumbnail - closing confirmation was not appears onscreen.
  * New menu item ‘Duplicate root...’ in tab menu. It tries to create new tab with root process and same directory/environment. Hotkey available but not assigned by default.


## Build 121108 ##
  * Settings: Group of controls ‘Automatic attach of cmd ...’ moved to ‘ComSpec’ page.
  * New environment variables: "ConEmuDrawHWND" and "ConEmuBackHWND".
  * HWND environment variables ("ConEmuHWND", "ConEmuDrawHWND", "ConEmuBackHWND") are processed when creating new console. For example, you may start gvim from cmd running in ConEmu:
```
gvim --windowid !ConEmuBackHWND! -new_console:sH
```


## Build 121106 ##
  * New ConEmuC command: Export environment variables to parent processes. Syntax (case insensitive):
```
ConEmuC /export[=CON|ALL] [Var1 [Var2 [...]]]
  VarN        - may contains one trailing/middle asterisk (sort of filemask support).
  /export     - export to all processes of current console and ConEmu GUI
  /export=CON - export to all processes of current console only
  /export=ALL - export to all processes of opened tabs and ConEmu GUI
```
    * Note! This is experimental, ‘Inject ConEmuHk’ required to set variables.
    * Some shells may ignores environment changes because of their features.
    * Shells comments: ‘Far Manager’ & ‘TCC/LE’ - OK; ‘cmd’ - fails partially (required to run smth from active cmd, for example "cmd /k exit", to ‘apply’ changes); bash - fails completely (seems, it does not support outside changing of env.vars).
  * When ‘Hide caption’ and ‘Frame width’ >= resizable frame width - resizable frame used.


## Build 121104 ##
  * [Issue 786](https://code.google.com/p/conemu-maximus5/issues/detail?id=786): Failed to input Chinese in PuTTY.
  * Resize bugfix: when maximizing scrolling window (e.g. "dir c:\ /s") ‘viewport’ enlarges after delay.


## Build 121101 ##
  * Searching in full console (not only visible area). Checkbox ‘Freeze console’ must be turned on. To be continued...
  * Sometimes DosBox do not starts from Far command prompt (doom.exe -warp 10).
  * Background: you may set ‘solid color’. Just type in ‘Path’ field ColorRef value (e.g. ‘#300A24’ without quotas) instead of background picture file. Supported formats are: ‘#RRGGBB’ (hex), ‘0xBBGGRR’ (hex), ‘xBBGGRR’ (hex), ‘RRR,GGG,BBB’ (dec), ‘RRR GGG BBB’ (dec).
  * New predefined color scheme: ‘`<Ubuntu>`’. If you need purple background, set background color to #300A24 on ‘Main’ page.
  * Decided to change default hotkey for ‘Minimize/Restore’ - it's now `Ctrl+~`.
  * [Issue 795](https://code.google.com/p/conemu-maximus5/issues/detail?id=795): Reducing window size on Hide/Restore.


## Build 121029 ##
  * New option: Settings -> Status bar -> System colors.
  * [Issue 791](https://code.google.com/p/conemu-maximus5/issues/detail?id=791): Rollback changes in ‘Run As Administrator’.
  * [Issue 700](https://code.google.com/p/conemu-maximus5/issues/detail?id=700): Forcing ConEmu's console to larger history buffer count. Small count causes ‘Up arrow don't work in prompt’ for long console trees, for example: ‘cmd -> Far -> ConEmuC -> cmd -> vim -> cmd -> ...’ Forcing values in `HKEY_CURRENT_USER\Console\ConEmu`: History buffer size - min=16, default=50; Count - min=16, default=32.
  * In the Tasks popup menu (Win+N or down arrow near to `[+]`) you may RClick on the task and get submenu with task contents. You may click task command to start particular command.
  * [Issue 794](https://code.google.com/p/conemu-maximus5/issues/detail?id=794): Names of renamed tabs should appear on taskbar.


## Build 121028 ##
  * [Issue 526](https://code.google.com/p/conemu-maximus5/issues/detail?id=526): Optimization of ‘Inject ConEmuHk’. Hope on great speedup and no new bugs.
  * (Rolled back) Changed ‘Run As Administrator’ startup style to avoid flickering of real console.
  * Fixed default Far Manager macros for Lua (Far 3 build 2851).
  * Sometimes "-new\_console" was removed with excess spaces.
  * `*`nix like ‘sudo’ command. Batch file "%ConEmuBaseDir%\csudo.cmd". When checkbox ‘Add %ConEmuBaseDir% to %PATH%’ is On (‘ComSpec’ page), you may just type csudo in non-elevated prompt, for example:
```
csudo dism /online /enable-feature /featurename:NetFX3 /All /Source:D:\sources\sxs /LimitAccess
```
  * Some fixes in closing ConEmu window.


## Build 121022 ##
  * New default hotkeys: Ctrl+RClick - system menu, Shift+RClick - active tab menu.
  * New GuiMacro.
```
Menu(<Type>) - pop up System (Type=0) or Tab (Type=1) menu.
```
  * Now you may use LButton, RButton, MButton in "Keys & Macro". Note. When you use Alt+Btn for Menu() - release Alt before Btn to avoid menu cancelling.
  * Added default support of GIT-bash directory structure, read [MinGW structure](ConEmuFolders#MinGW_structure.md) for details.
  * Hiew32 fails to paste more than 1 char at once.


## Build 121020 ##
  * Filter had not works as expected on ‘Keys & Macro’ page.
  * Minor changes in ConEmu initialization process.
  * Create new console dialog, field ‘Another user’. Preferring build-in Administrator, another user from Administrators group, or just another user.
  * Recreating console as another user did not works.
  * [Issue 767](https://code.google.com/p/conemu-maximus5/issues/detail?id=767): ‘Tabs’ page, if ‘Multiple consoles in one ConEmu window’ is off then creating new tasks in new ConEmu window by default.
  * New `ConEmu.exe` command line switch:
```
/Palette <Name> - Choose named color palette.
```
  * [Issue 764](https://code.google.com/p/conemu-maximus5/issues/detail?id=764): Enable same FG/BG color when BG color is background image.
  * [Issue 777](https://code.google.com/p/conemu-maximus5/issues/detail?id=777): Resizing only works right from corners when ‘Pad size’ is set.


## Build 121018 ##
  * Removing GCC warnings.
  * Tasks page: New button ‘Active tabs’ will add all current tabs into selected task.
  * Button ‘No’ in close confirmation dialog. It will close active console only.
  * Confirmation dialog was not shown when single console was opened.
  * Don't show ‘Press Enter or Esc to close console’ after killing root process.
  * [Issue 774](https://code.google.com/p/conemu-maximus5/issues/detail?id=774): ConEmu window size breaks when moved across monitors.
  * [Issue 775](https://code.google.com/p/conemu-maximus5/issues/detail?id=775): Failed to update Windows 7 Jump list when no tasks are left in the list.


## Build 121016 ##
  * ConEmu on Reddit! http://redd.it/11k5oy Please, vote ;)
  * Internal resize changes.
  * [Issue 736](https://code.google.com/p/conemu-maximus5/issues/detail?id=736): ConEmu crashes on saving a task when creating the new one.
  * Pasting CygWin paths fails.
  * Fixes in ‘Extend foreground color with background’.
  * User palettes was not saved with ‘Save settings’ button.
  * Apps+Insert - paste file pathname from cliboard, converted to CygWin style.
  * App distinct: When there is no explicit settings for current application - root application settings applied.


## Build 121015 ##
  * Option ‘Use Clink in prompt’ supports loading of clink version 0.2 (recommended). There is no need to set up ‘clink autorun’ when this option is checked.
  * [Issue 763](https://code.google.com/p/conemu-maximus5/issues/detail?id=763): Maximizing puts ConEmu on the second monitor.
  * [Issue 764](https://code.google.com/p/conemu-maximus5/issues/detail?id=764): Enable same FG/BG color when BG color is background image.


## Build 121014 ##
  * Far 3 build 2851 supported. Builds from 2799 to 2850 (inclusively) not supported.
  * Incorrect startup position when maximized and hide caption.
  * AnsiColors16.ans changed, AnsiColors16t.ans created.
  * Some resize bugfixes.
  * Minor changes in Settings dialog.
  * New ConEmu.exe command line switches (some exists already but not described).
```
/Reset - Don't load settings from registry/xml.
/UpdateJumpList - Update Windows 7 taskbar jump list.
/LoadCfgFile <file> - Use specified xml file as configuration storage.
/SaveCfgFile <file> - Save configuration to the specified xml file.
/Exit - Don't create ConEmu window, exit after actions.
```
  * [Issue 760](https://code.google.com/p/conemu-maximus5/issues/detail?id=760): Shield overlay icon appears only on tab-change on Taskbar.
  * New option: ‘Appearance’ -> ‘Show Shield overlay (Win7 or higher)’.


## Build 121009 ##
  * Changes of numbers on ‘Appearance’ page was ignored.
  * Far Manager: when "far /w" and mouse over visible scrollbar - mouse wheel scrolls console.


## Build 121008 ##
  * Changes in ‘Sleep in background’.
  * Some more AI in ‘Press Enter or Esc to close console’ (successfull ReadLn takes into account).
  * Changes of ‘Pad size’ was ignored.
  * !Bugfix: Restart as Administrator fails.


## Build 121004 ##
  * BugFix. "`%ConEmuDir%`" does not works for elevated consoles.
  * BugFix. Maximize fails on WinXP.


## Build 121002 ##
  * Far 3 build 2848 supported.
  * Two elevated in task.
  * DBCS bugfixes (default font in real console).
  * More information in logs.


## Build 120930 ##
  * GuiMacro: Paste(8) - paste path from clipboard converted to CygWin style.
  * When started with "`/logN`" switch and program folder is write-protected, log files will be created on `Desktop\ConEmuLogs`.
  * Bugfix: "`/single`" switch.


## Build 120926 ##
  * [Issue 727](https://code.google.com/p/conemu-maximus5/issues/detail?id=727): Shortcut opens multiple instances.
  * [Issue 724](https://code.google.com/p/conemu-maximus5/issues/detail?id=724): Internal window resize changes (continuation).
  * When can't create font (specified font not found in system) defaulting to monospace fonts.
  * Ctrl+Shift+D, Ctrl+Shift+F - select and paste to console path to a folder or a file.


## Build 120925 ##
  * [Issue 721](https://code.google.com/p/conemu-maximus5/issues/detail?id=721): Window is not resizable in Normal mode with caption hidden.


## Build 120924 ##
  * Internal window resize changes (continuation).
  * ConEmuHk compiled with "/DYNAMICBASE".
  * Switch "/single" takes care of the path, from existing ConEmu instance was started.
  * For ‘Quake style’ switch "/single" implied.


## Build 120923 ##
  * Internal window resize changes.
  * Touchscreen. Ignore SPI\_GETWHEELSCROLLLINES.
  * IME. Composition window placed inside ConEmu.
  * RClick & Far 3 & "Far /w" does not select panel items.
  * Minor changes in splitter`s painting.
  * [Issue 689](https://code.google.com/p/conemu-maximus5/issues/detail?id=689): Progress stuck at 100% in powershell.
  * New switch: "ConEmu.exe /reset" starts without loading settings (a'la fresh config).
  * New option on ‘Show’ page: ‘Don't show ConEmu window on Taskbar’.
  * New option on ‘Caption’ page: ‘Always show numbers [n/m]’.
  * New option on ‘Tabs’ page: ‘Tabs on bottom’.
  * [Issue 716](https://code.google.com/p/conemu-maximus5/issues/detail?id=716): GuiMacro.
```
SetOption("QuakeAutoHide",Value).
  Value: 2 - switch autohide, 1 - enable, 0 - disable.
```
  * [Issue 717](https://code.google.com/p/conemu-maximus5/issues/detail?id=717): New global hotkey for ‘Restore (bring to front)’. Unassigned by default.


## Build 120916 ##
  * Settings dialog redesigned. New pages created. Some options renamed.
  * Regression fixed. Can't activate alternative mode for tab #1.
  * Alternative buffer of "far /w" was not scrollable.


## Build 120913 ##
  * I'm bored to tell ‘Read Disclaimer #2’. Option ‘Inject ConEmuHk’ is off for now by default.
  * Some more diagnostic info on failed to start root command (shell).
  * Some default hotkeys are moved from Macros to User area, not to occupy Macros cells.
  * GuiMacro.
```
    Paste(4[,<Title>]) - choose file and paste full pathname
    Paste(5[,<Title>]) - choose folder and paste full path
```
  * New switches in "-new\_console".
```
    "-new_console:o" - don't enable 'Long console output' when starting command from Far Manager.
    "-new_console:i" - don't inject ConEmuHk into starting process.
```
  * ConEmu hungs on exit, when F-Secure is installed.
  * Attach does not catch console windows (regression of 120909).
  * Hyperlinks detector fixes.
  * In some cases (e.g. "far /x"), split console (Ctrl+Shift+O/E) starts program via "cmd.exe" as root console app.


## Build 120909 ##
  * Settings -> Tasks: more intuitive configuration for Tasks edit box, new buttons.
    * "Add Tab" - opens standard dialog with "Save" button, which append new line to Tasks edit box.
      * Add startup dir" - simplifying insertion of `"-new_console:d:<dir>"` with GUI dialog.
      * Add file path" - inserts path to any file (GUI dialog too).
  * Settings -> ComSpec: ‘Support UNC paths in cmd.exe’. The beginning. Checkbox turn on "!DisableUNCCheck".
  * GCC Build updated. ConEmu, ConEmuC, ConEmuCD, ConEmuHk. 32-bit versions only was tested in GCC.
  * MinGW. ConEmu files MAY be stored in MinGW compatible locations:
```
    $MINGW_ROOT/bin/
      ConEmu.exe, ConEmu64.exe
    $MINGW_ROOT/libexec/conemu/
      ConEmuC.exe, ConEmuHk.dll and others
      ConEmu.xml - contains some defaults for MinGW environment
    $MINGW_ROOT/share/conemu/
      WhatsNew-ConEmu.txt and others
```
  * AutoUpdate is disabled in MinGW mode.
  * Minor changes in command line parser.
  * Selection ended unexpectedly when Modifier for selection start was set and


## Build 120904 ##
  * When ‘Auto tabs’ is on, creating new console shrinked ConEmu size.
  * SplitScreen, dialog fixes.


## Build 120903 ##
  * SplitScreen options in ‘Create new console’ dialog.


## Build 120902 ##
  * Fixed regression of ‘Auto show scrollbar’.
  * [Issue 696](https://code.google.com/p/conemu-maximus5/issues/detail?id=696): ‘Frame width’ does not works (with ‘Hide caption always’).
  * Some excess and wrong console resizes removed on window state change.
  * Successive show/hide tab bar descrease window height.
  * Installer creates "`ConEmu.exe`" and "`ConEmu64.exe`" in AppPaths (HKLM).


## 120830 ##
Wikipedia page [User:Maximus7792/ConEmu](http://en.wikipedia.org/wiki/User:Maximus7792/ConEmu) in progress. Any comments, suggestions or corrections?


## Build 120830 ##
  * Internal changes in child windows.
  * Some fixes for DBCS.
  * GuiMacro. Changes in Shell function. All following examples runs "cmd" in new tab: ‘`Shell("new_console","cmd")`’ or ‘`Shell("","cmd")`’ or ‘`Shell("","","cmd")`’. **Memorial**. Other verbs, like ‘`Shell("open","cmd")`’ runs "cmd" outside of ConEmu window.
  * Safe Far close. From current build, this option may be used for ‘Unsafe’ console closing. Type "`#Close(1,1)`" (without quotas) in Safe Far close macro field and turn on checkbox. Now, click on the cross-button in ConEmu title provokes ‘Terminate active process’ in all opened consoles, regardless of ‘Far’ it is or not. **Warning!** Use on your own responsibility, this method may damage console applications data!
  * Far Manager & [TabletPC](TabletPC.md). Tap on clock+1row (upper-right corner) show/hide panels.
  * DoubleClick on TabBar free area acts as click on `[+]` button.


## Build 120826 ##
  * GuiMacro.
```
    Print(["<Text>"])
      Alias for Paste(2,"<Text>")
    Rename(<Type>,["<Title>"])
      Rename tab (Type=0) or console title (Type=1)
    Close(<What>[,<Flags>])
      close current console (0), without confirmation (0,1),
      terminate active process (1), without confirmation (1,1),
      close ConEmu window (2)
```
  * Internal changes in child windows.
  * When ‘Show & store current window size and position’ is off, size and pos was not displayed on ‘Size & Pos’ page.
  * CmdAutoAttach failed, if ConEmu.exe not installed.
  * ‘Show was hidden warning’ option on ‘Features’ page. ‘On’ by default. May be turned ‘Off’ for use with some desktop switchers (Dexpot e.g.).
  * [Issue 674](https://code.google.com/p/conemu-maximus5/issues/detail?id=674): Tab macro function going to console doesn't handle 1-based offsets properly.
  * После показа/скрытия TabBar (3-е состояние флажка) или StatusBar менялась высота окна ConEmu.
  * ‘Twilight’ and ‘Zenburn’ color palettes.


## Build 120806 ##
  * Bugfixes and regressions of SplitScreen.
  * Failed to store height less than 9 lines.
  * [Issue 668](https://code.google.com/p/conemu-maximus5/issues/detail?id=668): "-new\_console:d:C:\Program Files (x86)" fails.
  * GuiMacro.
```
    Close(<What>[,<Flags>])
      close current console (0), without confirmation (0,1),
      terminate active process (1), without confirmation (1,1)
    Shell. You may omit both <File> and <Parms>. This means ‘Duplicate tab’.
      Shell("new_console:sV") - create same tab, split current tab to bottom;
      Shell("new_console:sH") - create same tab, split current tab to right.
```


## Build 120803 ##
  * Bugfixes and regressions of SplitScreen.


## Build 120803 ##
  * Bugfixes and regressions of SplitScreen. Now you can create more than two splits.
  * Some changes in ‘Long console output’ (plugin required in Far Manager).
  * Minimal console size now is 4 cols 2 lines.


## Build 120802 ##
  * Experimental: `-new_console:s[<SplitTab>T][<Percents>](H|V)`. Examples.
    * `cmd -new_console:s` - split current tab, new tab and old (current) tab become 50% width of current tab.
    * `cmd -new_console:s50H` - same as `cmd -new_console:s`.
    * `cmd -new_console:sV` - split current tab, create new tab to the bottom.
    * `cmd -new_console:s3T30H` - split 3-d tab, create new tab to the right with 30% width.
  * Regression, Build 120727a: xterm-256 was broken.
  * [Issue 651](https://code.google.com/p/conemu-maximus5/issues/detail?id=651): 7-zip/WinRar was not elevated, when updating in "Program Files".
  * New option ‘Add %ConEmuBaseDir% to %PATH%’ on page ‘ComSpec’. Turned on by default.


## Build 120727a ##
  * Now you may use `ConEmu.exe` switches in the ‘Command’ field on ‘Integration’ page. For example: `/single /cmd powershell.exe`.
  * Minor hooks optimization.
  * [Issue 651](https://code.google.com/p/conemu-maximus5/issues/detail?id=651): Sometimes, ConEmu choose incorrect update method (msi instead 7z and vice versa). Cause of previously saved value "Update.DownloadSetup".


## Build 120727 ##
  * [Issue 643](https://code.google.com/p/conemu-maximus5/issues/detail?id=643): Windows from powershell/.net does not appears on screen.
  * [Issue 652](https://code.google.com/p/conemu-maximus5/issues/detail?id=652): New predefined palette ‘`<Solarized Light>`’.
  * Speedup of ecompl Far Manager plugin.
  * Color values fields (Settings dialog) allows next formats: ‘#RRGGBB’ (hex), ‘0xBBGGRR’ (hex), ‘xBBGGRR’ (hex), ‘RRR,GGG,BBB’ (dec), ‘RRR GGG BBB’ (dec).
  * New minimal value for ‘transparency’ is 0. Now on focus lose window may completely disappears from screen.


## Build 120726 ##
  * Mailing list created: http://groups.google.com/group/conemu_ml. Welcome!


## Build 120722 ##
  * [Issue 348](https://code.google.com/p/conemu-maximus5/issues/detail?id=348): Maximize errors with Hide caption.
  * [Issue 641](https://code.google.com/p/conemu-maximus5/issues/detail?id=641): Show current console title in the status bar.
  * [Issue 645](https://code.google.com/p/conemu-maximus5/issues/detail?id=645): PowerShell cmdlet Write-Progress detection.
  * [Issue 646](https://code.google.com/p/conemu-maximus5/issues/detail?id=646): ConEmu eats dead keys.
  * [Issue 647](https://code.google.com/p/conemu-maximus5/issues/detail?id=647): Mouse wheel does not respect Control Panel settings.
  * [Issue 648](https://code.google.com/p/conemu-maximus5/issues/detail?id=648): Progress detection AI fixes.
  * Alternative buffer was scrolled twice by arrows.
  * Status bar appearance. Options ‘Vertical separators’ and ‘Horizontal separator’.
  * Status will be shown on startup in Status bar only (if enabled).
  * ‘Sleep in background’ fixes.
  * Keyboard logging fixes (`ConEmu.exe /log`).


## Build 120719 ##
  * [Issue 645](https://code.google.com/p/conemu-maximus5/issues/detail?id=645): PowerShell cmdlet Write-Progress detection.
```
0..100 | foreach {Write-Progress Test Progress -PercentComplete $_; sleep -m 100}
```
  * ‘On the fly’ changing of text and popup-s console colors (cmd, powershell, etc.)
    * ‘On the fly’ changing of popup-s allowed in Vista or higher. Win2k and WinXP - after start of new console.
    * While changing popup-s colors RealConsole may flicker on the screen.


## Build 120717 ##
  * Do not run clink in background threads.
  * Sometimes ConEmu may crash while initialzing Far Manager.
  * New item ‘Console information’ in the ConEmu Far Manager plugin menu.


## Build 120716 ##
  * Window size and position moved to separate page ‘Size & Pos’.
    * Option ‘Long console output’ moved to the page ‘Size & Pos’.
    * Option ‘Cmd output codepage’ moved to the page ‘ComSpec’.
  * New option ‘Show & store current window size and position’. Turned On by default. When turned Off field values X/Y/Width/Height are not updated automatically (they displays user saved values).
  * Button ‘Apply’ in the window mode ‘Normal’ was not changed window position (X/Y).
  * [Issue 638](https://code.google.com/p/conemu-maximus5/issues/detail?id=638): bincmp fails.
  * After Apps+F context menu unexpectedly appears in Find dialog.
  * New color palette ‘`<PowerShell>`’.
  * ConEmu Inside: Windows 2000 and Windows 2003 supported.
  * Windows 2000 supported again. Alternative mode attach in Windows 2000 is not available.
  * While alternative buffer is active (Win+A) keys PgUp/PgDn/Up/Down will scroll console (holding Ctrl is not necessary).
  * New feature on ‘Features’ page: ‘Use Clink in prompt’. Clink - bash style completion.
    * This is **experimental** feature, it may be changed or removed in future builds.
    * Download here version 0.1.1: http://clink.googlecode.com
    * Unpack here: `%ConEmuBaseDir%\clink`
    * Please note, clink archive contains subfolder. Files `clink_dll_x86.dll` and `clink_dll_x64.dll` must be located in `ConEmu\clink`.


## Build 120710 ##
  * Far Manager. Do not turn on long buffer while searching in archives via MultiArc.
  * StatusBar.
    * New column ‘[+]’ - shows create new console popup menu.
    * Right click on ‘[+]’ or ‘ActiveCon/TotalCon’ pop up current tab menu.
  * DblClick on system icon had not closed ConEmu window.
  * Injects bugfixes.
  * [Issue 639](https://code.google.com/p/conemu-maximus5/issues/detail?id=639): `/cmd` can't find `far.cmd` or `far.bat` without the extension.
  * ConEmu Inside. When ConEmu already exists subsequence calls of menu item creates new console (tab) in existing window.


## Build 120710 ##
  * Reorder tabs. New hotkeys. Win+Alt+Left/Win+Alt+Right move tab (definitely all tab of current console) leftward/rightward.
  * [Issue 637](https://code.google.com/p/conemu-maximus5/issues/detail?id=637): ConEmu recursively runs itself in an endless loop.
  * Error when "Download path" = "C:\".
  * Changes in hooks.


## Build 120709 ##
  * Minimize/Restore takes into account ‘Always on top’ option.
  * ‘Copy on LButton up’ refined.
  * New hotkey: Minimize/Restore (alternative).


## Build 120708 ##
  * GuiMacro: теперь можно позвать через ANSI коды. Пример в файле ‘IsConEmu.cmd’.
  * Вкладка ‘Keys & Macro’: В поле редактирования GuiMacro можно использовать \e (ESC) и \a (BELL).
  * Cmd\_Autorun.cmd fixed.
  * ConEmuC switches. "`/IsConEmu`", "`/IsTerm`", "`/IsAnsi`". `ErrorLevel 1` - ON, `ErrorLevel 2` - OFF.
  * [Issue 633](https://code.google.com/p/conemu-maximus5/issues/detail?id=633): Cannot specifiy environment variables for startup dir.
  * Mark & Copy.
    * Уточнение DoubleClick, TripleClick.
    * Ctrl+C/Ctrl+Ins при наличии выделения выполняют копирование и завершают режим выделения.
  * [Issue 634](https://code.google.com/p/conemu-maximus5/issues/detail?id=634): Опция ‘End selection on typing’. 3-е состояние - завершить без копирования.
  * Во время выделения текста не работали хоткеи (типа Win+Alt+P).
  * [Issue 635](https://code.google.com/p/conemu-maximus5/issues/detail?id=635): На вкладке ‘Mark & Paste’ показываем хинт, если модификаторы Text/Block/Prompt совпадают.
  * [Issue 631](https://code.google.com/p/conemu-maximus5/issues/detail?id=631): Italic drawing bug.
  * [Issue 593](https://code.google.com/p/conemu-maximus5/issues/detail?id=593): Minimize/Restore hotkey fix. Now activating ConEmu if not focused.
  * ConEmu Inside: Добавлена поддержка Windows XP. В проводнике должна быть показана панель ‘Полезный совет’.
  * Доработана вкладка ‘Integration’.
    * Группы ‘ConEmu Inside’ и ‘ConEmu Here’ отвечают за регистрацию/удаление пунктов контекстного меню проводника для дисков/папок/файлов. Выпадающие списки ‘Menu item’ показывют текущие пункты меню (то что сейчас зарегистрировано в реестре). Любой пункт можно просмотреть (выбрав его в списке) и отредактировать (нажав кнопку ‘Register’).
    * Группа ‘Automatic attach of cmd...’ позволяет установить (или просмотреть текущую) команду, выполняемую процессором (cmd и tcc) при старте. Это позволяет автоматически подцепить запущенный cmd/tcc в новую вкладку ConEmu.


## Build 120705 ##
  * Settings tree redesigned.
  * [Issue 630](https://code.google.com/p/conemu-maximus5/issues/detail?id=630): ‘Snap to desktop edges’ and multi-monitor layout.
  * ConEmuC: "`/ATTACH /CONPID=<pid> /GHWND=NEW`" - Alternative nonintrusive attach of existing console to the NEW instance of ConEmu.exe.
  * [Issue 623](https://code.google.com/p/conemu-maximus5/issues/detail?id=623): Continue...


## Build 120704 ##
  * FindEditor/FindViewer/FindFarWindow возвращал "NotFound".
  * Empty lines sometimes appear on Debug page even if ‘Shell activity’ = ‘Disabled’.
  * [Issue 623](https://code.google.com/p/conemu-maximus5/issues/detail?id=623): Continue...
  * [Issue 629](https://code.google.com/p/conemu-maximus5/issues/detail?id=629): Crash on saving color palette.
  * Alternative nonintrusive attach mode for console windows.


## Build 120703 ##
  * Now DoubleClick (for ‘words’) and TripleClick (for ‘lines’) both works for text selection.
  * Extended ANSI codes (`ESC ] 9 ; ...`) - http://code.google.com/p/conemu-maximus5/wiki/AnsiEscapeCodes#ConEmu_specific_OSC.
  * [Issue 623](https://code.google.com/p/conemu-maximus5/issues/detail?id=623): Checking...


## Build 120702 ##
  * Drag window by any point of client area (configurable modifier, Ctrl+Alt by default).
  * ‘Features’ -> ‘Snap to desktop edges’.
  * [Issue 626](https://code.google.com/p/conemu-maximus5/issues/detail?id=626): /dir arg is ignored for startup task.
  * ‘Colors’ -> Text/Back. The beginning.


## Build 120701 ##
  * Для облегчения отладки и сообщений об ошибках.
    * Создать mini-dump: `ConEmuC.exe /DEBUGPID=<PID> /MINI`
    * Создать full-dump: `ConEmuC.exe /DEBUGPID=<PID> /FULL`
    * О соответствии битности беспокоиться не нужно, все сделает сам ConEmuC.exe.
    * Также, добавлен пункт в системном меню ConEmu: Debug -> Active process memory dump.
  * Tasks: `/DIR "<dir>"` учитывается при запуске вкладки в уже открытом ConEmu.
  * Также, рабочую папку можно указать с помощью параметров "`-new_console:d[:<dir>]`" и "`-cur_console:d[:<dir>]`".
  * Немного AI в ‘Change prompt text cursor position with Left Click’ - не пытаться вывести курсор за пределы строки.
  * [Issue 575](https://code.google.com/p/conemu-maximus5/issues/detail?id=575): Confirm or abort setting ConEmu OnTop from external program.


## Build 120630 ##
  * Page ‘Transparency’, ‘Use separate value’.
  * После закрытия консоли (не последней) на статусной строке не обновлялось поле ‘ActiveCon/TotalCount’.
  * ConEmu inside mode.
    * Синхронизация текущей папки: Explorer -> ConEmu. На статусной строке добавлена кликабельная колонка ‘Sync’.
    * Адаптировано для Vista.
  * More ANSI examples. RenameTab.cmd, SetConTitle.cmd, SetProgress.cmd.
  * GuiMacro.
```
Progress(<Type>[,<Value>])
  Set progress state on taskbar and ConEmu title.
    Type=0: remove progress.
    Type=1: set progress value to <Value> (0-100).
    Type=2: set error state in progress.
```
  * На ярлык ConEmu (или сам ConEmu.exe в проводнике) можно набросить один или несколько ярлыков программ. Каждая запустится в своей вкладке. Исключение - если в в настройке ConEmu ‘Startup’ -> ‘Command line’ указан "`far.exe`". В этом случае строка запуска ‘подклеивается’ к "`far.exe`".
  * При быстром закрытии ConEmu ‘крестиком’ в консоли успевало мелькнуть ‘Press Enter or Esc to close console...’.
  * [Issue 575](https://code.google.com/p/conemu-maximus5/issues/detail?id=575): Confirm or abort setting ConEmu OnTop from external program.
  * [Issue 624](https://code.google.com/p/conemu-maximus5/issues/detail?id=624): Menu was not available in Notepad++ with SourceCookifier plugin.


## Build 120626 ##
  * Again ConEmu inside mode.
    * AutoResize
    * ‘As Administrator’ supported: use "`-cur_console:a`" or hold Shift when selecting menu item
    * For debug purposes: "`/insidewnd <HWND>`" & "`/insidepid <PID>`".
    * ConEmu\_Inside.reg file updated.
  * Right click on leftmost part of status bar pop up system menu (useful in ‘ConEmu inside’ mode).
  * Minor fix in Close Far editor/viewer confirmation.


## Build 120625 ##
  * New option 'Min cursor size (pix)' on pages 'Text cursor' & 'App distinct'.
  * [Issue 619](https://code.google.com/p/conemu-maximus5/issues/detail?id=619): Sometimes Assertion appears after closing Far Manager.
  * Minor fix in 'Detect text line ends', 'Trim trailing spaces', 'Bash margin'.
  * New ConEmu.exe switch - "`/inside`". Sample usage in "`ConEmu\Addons\ConEmu_Inside.reg`". In breaf - create a pane in Explorer window and start there, for example, powershell. Experimental, was tested in Windows 7 and Windows 8, other systems was not tested yet. Before calling context menu item you owe to increase height of lower pane (Windows 7) of display right pane (Windows 7 & 8).


## Build 120624 ##
  * On the page 'Mark & Paste' option 'Auto copy' was renamed to 'Copy on LButton up'. Turned on by default.
  * Failed to save settings 'Highlight and goto' and 'External editor'.
  * [Issue 587](https://code.google.com/p/conemu-maximus5/issues/detail?id=587), [Issue 605](https://code.google.com/p/conemu-maximus5/issues/detail?id=605): Don't show empty clipboard message on Paste.
  * New settings: scrollbar appear/disappear delay.
  * Quake mode Again: 'Pad size (pix)' - pad around console field, works only with checked 'Try to center'.
  * 'App distinct' settings page redesigned.
  * Now are configurable: Apps+Space, Alt+F9, Alt+Enter, Alt+Space, Ctrl+Up, Ctrl+Down, Ctrl+PgUp, Ctrl+PgDn и PicView.SlideShow.
  * Checkbox removed (was in 'Send to console' group): Alt+F9, Alt+Enter, Alt+Space.
  * Win+Down does not increased height of console (option ‘Win+Arrows - resize window’).
  * [Issue 606](https://code.google.com/p/conemu-maximus5/issues/detail?id=606): Warnings was not appears in TSA in Windows XP.
  * New settings page 'Text cursor'.
  * [Issue 612](https://code.google.com/p/conemu-maximus5/issues/detail?id=612): Option to change cursor size ('Fixed cursor size' & '(5-100) %').
  * [Issue 608](https://code.google.com/p/conemu-maximus5/issues/detail?id=608): Fixed display problems after closing telnet and returning to calling app (cmd, far).
  * cmd/powershell/tcc: Switch "-new\_console" (for starting console applications in new ConEmu tab)
> now may be used without "ConEmuC.exe". For example, you may type in cmd.exe prompt:
```
powershell -new_console
```
  * Some fixes in Detach of GUI applications.
  * [Issue 607](https://code.google.com/p/conemu-maximus5/issues/detail?id=607): 'Mark & Paste' and 'App distinct' -> 'Detect text line ends', 'Bash margin', 'Trim trailing spaces', 'EOL'.
  * 'Mark & Paste' and 'App distinct' -> 'Change prompt text cursor position with Left Click'.


## Build 120618 ##
  * [Issue 582](https://code.google.com/p/conemu-maximus5/issues/detail?id=582): Change cursor position with mouse click (cmd, powershell, tcc/le, ...)


## Build 120617a ##
  * Small lag removed when activating (restoring) ConEmu with Far 3.
  * Putty does not received mouse wheel events.
  * GUI applications fix (broken in 120617).


## Build 120617 ##
  * 'Quake style slide down' third state - auto hide on lose focus.
  * Do not catch focus when starting with argument "/min".
  * [Issue 599](https://code.google.com/p/conemu-maximus5/issues/detail?id=599): 'Auto copy' on 'Mark & Paste' - copy selected text immediately on left mouse button up (when selecting with mouse).
  * [Issue 599](https://code.google.com/p/conemu-maximus5/issues/detail?id=599), [Issue 570](https://code.google.com/p/conemu-maximus5/issues/detail?id=570): Small lags removed when pasting large amounts of text from clipboard.
  * 'Main' settings page redesigned.
  * Chinese (DBCS) improvements.
  * Some Wine related fixes, but still not working ;)
  * Message balloon is shown in TSA when registration of global hotkey (Minimize/Restore) failed.
  * New default for Minimize/Restore - Win+Shift+C.


## Build 120614 ##
  * DownLeft and DownRight options for background image alignment.
  * [Issue 577](https://code.google.com/p/conemu-maximus5/issues/detail?id=577): Chinese font display error on Chinese Win XP.
  * [Issue 587](https://code.google.com/p/conemu-maximus5/issues/detail?id=587): Right click gives an error box.


## Build 120613 ##
  * After deleting Task chosen for Startup, ConEmu stopped starting.
  * 120612: Executing a command with "-new\_console" argument (from an existing tab) caused ConEmu to minimize.
  * Tab menu (Rename tab) and Apps+R. Only panels can be renamed in Far (as of yet).
  * UpRight option for background image alignment.
  * [Issue 590](https://code.google.com/p/conemu-maximus5/issues/detail?id=590): Ctrl+C/Ctrl+Break fixes.
  * Some Quake style improvements.


## Build 120612 ##
  * Sample ‘ColorPrompt.cmd’ added to distribution. Run it in cmd.exe or tcc.exe for change command prompt text color to yellow.
  * New ComEmu.exe arguments - /ShowHide and /ShowHideTSA. When ConEmu is not yet started - starts it (with optional /cmd ... argument), when ConEmu window exists - minimize it or hide to TSA. Similar to global hotkey Win+C, but hotkey wouldn't work if ConEmu was not been started yet. These keys may be used for a desktop shortcut.
  * New ConEmu.exe argument - /dir "Folder". Same as ‘Working folder’ in shortcut properties.
  * Added ‘Quake style slide down’ switch on the ‘Features’ page. Sample settings are in ‘ConEmu\ConEmu\_Tilde.xml’ (something like [this](http://tech-bytes.co.uk/2008/05/23/a-quake-style-console-for-your-gnome-desktop/)). Rename sample ‘ConEmu\_Tilde.xml’ to ‘ConEmu.xml’, run ConEmu on startup with /min switch, and ConEmu will slide down/up by Tilde key.
  * [Issue 576](https://code.google.com/p/conemu-maximus5/issues/detail?id=576): Trap when updating Jump Lists/Task while history is empty.
  * Copy to clipboard did not work from alternative console (Win+A).
  * [Issue 584](https://code.google.com/p/conemu-maximus5/issues/detail?id=584): App distinct color palette not working.
  * [Issue 576](https://code.google.com/p/conemu-maximus5/issues/detail?id=576): ConEmu.exe /updatejumplist.
  * When Maximized, checking/unchecking ‘Hide caption always’ lost caption.
  * GuiMacro additions:
```
    Paste(2[,"<Text>"]) - paste all lines without confirmation
    Paste(3[,"<Text>"]) - paste first line without confirmation
    Task(Index[,"Dir"]) - start task by 1-based index
    Task("Name"[,"Dir"]) - start task with the specified name
```
  * GuiMacro: The "Macro" setting on the ‘Keys’ page now supports "\\", "\r", "\n", "\t" escape sequences. Example: ‘Paste(2,"exit\n")’ will close cmd.exe.


## 120611 ##
  * [Article](http://www.hanselman.com/blog/ConEmuTheWindowsTerminalConsolePromptWeveBeenWaitingFor.aspx) in Scott Hanselman's blog :)


## Build 120609 ##
  * Injects code was rewritten (new ASLR in Windows 8 RC x64).
  * [Issue 571](https://code.google.com/p/conemu-maximus5/issues/detail?id=571): Added ‘New window’ check box in the creation confirmation dialog. If you choose ‘Another user’, ConEmu itself will be started under the chosen account. You may set a separate hotkey for the dialog with the check box turned on.
  * Some drawing artifacts may appears on window edges in Maximized and Fullscreen modes.
  * Refinement of screenshots (Maximized/Fullscreen window modes).
  * Attaching did not work for GUI applications (notepad, PuTTY, ...)
  * The cursor was not moved in the console while dragging the scrollbar thumb area.


## Long time ago? ##
Lifehacker published [an article about Console, PowerCmd, Take Command and Mintty](http://lifehacker.com/5857540/the-best-terminal-emulator-for-windows). Unfortunately, ConEmu, once again, was not mentioned :(
All my attempts to post comments and contact the article author were ignored :(


## Build 120608 ##
  * New wiki page: http://code.google.com/p/conemu-maximus5/wiki/RoadMap
  * Refinement of console closing confirmation.
  * GuiMacro: WindowFullscreen, WindowMaximize, WindowMode, Status.
  * Keys: Always on Top, Show/hide status bar (Apps+S), Show/hide tab bar (Apps+T), About (Win+Alt+A).
  * About: Pages added width arguments of ConEmuC, -new\_console and GuiMacro.
  * Added external editor startup line option in the ‘Mark & Paste’ page. By default: `far.exe /e%1:%2 "%3"`. '%1' - row, '%2' column, '%3' - filename.
  * Refinement of screenshot creation (Win+H). New Win+Shift+H - entire monitor area screenshot.
  * [Issue 570](https://code.google.com/p/conemu-maximus5/issues/detail?id=570): Pasting into iPython (in fact, any console app except Far) does not work as expected.
  * [Issue 558](https://code.google.com/p/conemu-maximus5/issues/detail?id=558): Some more color anomalies.


## 120605 ##
  * An answer was posted on [stackoverflow.com](http://stackoverflow.com/questions/60950/is-there-a-better-windows-console-window/10904494#10904494). Q: Is there a better Windows Console Window.
  * Advertising application on [meta.stackoverflow.com](http://meta.stackoverflow.com/questions/132988/open-source-advertising-sidebar-2012/134414#134414).


## Build 120604 ##
  * Error ‘Can't free hooks in module ...’ in ConEmuHk.dll
  * [Issue 557](https://code.google.com/p/conemu-maximus5/issues/detail?id=557): Assertion в RealBuffer.cpp
  * [Issue 568](https://code.google.com/p/conemu-maximus5/issues/detail?id=568): First call of 'View console output' (Far plugin) opens viewer in dump-mode
  * Fixed occasional exception in Far after execution of console command


## Build 120603 ##
  * Internal interprocess communication changes.
  * Status bar: New column 'ConEmu GUI PID'.
  * [Issue 566](https://code.google.com/p/conemu-maximus5/issues/detail?id=566): Hotkey `<<None>>` is not unique.
  * 'Settings'-> 'Tasks' -> 'ConEmu arguments for Jump list'. You may specify here additional arguments for ConEmu.exe, when starting from taskbar (Jump list). E.g.: `/icon "cmd.exe"`.


## Build 120602 ##
  * Internal interprocess communication changes.
  * Windows 7 or higher. Jump list support: ConEmu may add its 'Predefined tasks' and commands from its history into the Windows 'Tasks' section. Jump list population takes some time, therefore the button 'Update Now!' was added to the 'Tasks' page of the 'Settings' dialog.
  * ConEmu.exe: Added command line argument /title "Template". The specified text will be shown in ConEmu's window title instead of the active console's title. You may use environment variables in the "Template" string.
  * 'Create new console' dialog: Added 'Reset command history...' item to the system menu.
  * Win+H: Make screenshot of active ConEmu window.
  * Tabs: Separate tab templates for Panels (Far) and other console applications.
  * Tabs: You may specify word list, excluded from the title of consoles (with exception of Far). Tired of the 'Administrator:' prefix of the consoles, runned as Administrator (Win7).
  * Take away lags while closing ConEmu.
  * Sometimes, Shell-dialogs (starting files, EMenu items) does not appears in front of ConEmu.
  * Minor change in (isMonitorConsoleLang == 2).
  * 'Fade when inactive' affects Status bar.
  * Command line, specified by '/cmd' argument (ConEmu.exe) will be added to the history.
  * [Issue 561](https://code.google.com/p/conemu-maximus5/issues/detail?id=561): Sometimes, ConEmuHk64.dll cause exception in the console application.
  * [Issue 564](https://code.google.com/p/conemu-maximus5/issues/detail?id=564): Mouse events does not sent to Far, when 'Inject ConEmuHk' is off.
  * Changes in close tabs confirmation. Single dialog for all console, when closing ConEmu by ‘clicking on the cross’.


## Build 120527a ##
  * Window mode changes (Normal/Maximized/Full screen) was not saved, if button 'Apply' was not clicked before 'Save settings'.
  * Fixed Status bar setup menu hints.


## Build 120527 ##
  * Current accelerators are shown in system and tab menu.
  * Mouse events was not sent to console apps (except Far), when scrolling is on.
  * 'Settings' -> 'Features' -> 'Mouse options' -> 'Enable mouse'. Enabled by default. May be disabled to forbid sending mouse event to console. However, when disabled, you still can use mouse for selecting and pasting text.
  * New transparency option - ColorKey. However, 'UserScreen transparency' is still more useful for Far Manager.
  * Win+G - new hotkey for 'Attach...' system menu item.
  * Status Bar. Enabled by default. Font, color and column list (any of 20-th) are configurable in 'Settings' -> 'Status bar'. You may show/hide column from status bar popup menu, too. Column descriptions displayed, while hover mouse over it. Some columns process left clicks:
    * columns 'CAPS'/'NUM'/'SCRL' - switching;
    * column 'Active VCon' - active console selection popup menu (same as from toolbar);
    * column 'Transparency' - changing on the fly (100%..40%, UserScreen, ColorKey).


## Build 120524a ##
  * ConEmuC.exe/ConEmuC64.exe sets console text attributes to 7 on startup.
  * 'Settings' -> 'Mark & Paste' -> 'Freeze console before selection'.
  * 'Find test' -> 'Freeze console'.
  * Lags in ConEmu plugin.


## 120524 ##
  * [Facebook](http://www.facebook.com/ConEmu.Maximus5) page was created.


## Build 120523a ##
  * [Issue 554](https://code.google.com/p/conemu-maximus5/issues/detail?id=554): Fixed handling of `^[#G` (ANSI X3.64).


## Build 120523 ##
  * 'Auto scroll' item removed from the system menu. If the console must be paused - use - Win+A (alternative buffer).
  * Ctrl+Win+Alt+Enter - forced activation of ConEmu in FullScreen. Like Win+C, the configurable hotkey will work regardless of the active application.
  * Win+Shift+Del - Terminate the active process in the current console.
  * The "ConEmuANSI" environment variable was added. Possible values are "ON" and "OFF".
  * [Issue 554](https://code.google.com/p/conemu-maximus5/issues/detail?id=554): Cursor positioning and text output errors (ANSI X3.64).
  * [Issue 553](https://code.google.com/p/conemu-maximus5/issues/detail?id=553): Console scrolling rollback.


## Build 120522 ##
  * New option: ‘[ANSI X3.64 / 256color](AnsiEscapeCodes.md)’ (‘Features’ tab). The entire console's area is used (incl. scroll buffer), however Xterm 256color only applies to the "working" area (lowest part of the console, when using the scroll buffer). The option "Inject ConEmuHk" must be enabled as well. If 256color does not seem to work, enable the ‘Colorer TrueMod support’ flag, and check that buffer/scrolling is disabled. Example: "`256colors2.pl -cur_console:h0`". Or, to scroll the console to the bottom: "`echo ^[[9999;1H`" (replace ‘^[’ with the real ESC symbol (char with code \x1B)). Two examples are included in `ConEmu\Addons\AnsiColors16.ans` and `ConEmu\Addons\AnsiColors256.ans`.
  * Win+A - alternative buffer (toolbar button - black square). When used in Far Manager, it displays the output of the last command. Otherwise, freezes the current state of the console (incl. scroll buffer). The alternative console can be closed by a repeated Win+A or Esc.
  * Win+S - enable/disable scroll buffer (BufferHeight). Ignored in Win7. Toolbar button - vertical arrows.
  * Win+N - Show create new console popup menu.
  * [Issue 551](https://code.google.com/p/conemu-maximus5/issues/detail?id=551): Updater, Invalid unpack command line, while using environment variables.
  * New ConEmu.exe command-line options:
```
    /min - run minimized;
    /tsa - minimize to tray (to minimize to TSA on start, use with /min);
    /icon "<file>" - use icon from <file> (<file> can be exe, dll or ico);
    /noupdate - disable automatic update check on start.
```
  * 'About' dialog updated.
  * Fixed exception when lowering font size in Maximized window mode.
  * Added three system palettes: Standard VGA, Terminal.app, xterm.
  * Injects fixes.
  * Fixed incorrect size of fast configuration dialog when ConEmu starts maximized (shortcut property).
  * Fixed incorrect rendering when the flag 'Auto' on the 'Main' tab was enabled in Maximized window mode.
  * Minor changes in ConEmuC.exe.


## 120520 ##
  * ConEmu is mentioned in Wikipedia: http://en.wikipedia.org/wiki/Comparison_of_file_managers#cite_ref-20


## Build 120515 ##
  * 'App distinct' loading was broken in previous build.
  * Maximum number of allowed consoles is 30 now.
  * Only active console 'button' is shown on toolbar. Left click on active console button pop up all opened tabs menu.
  * Console activation with numbers.
    * Win+F11 and Win+F12 not used.
    * When console count less then 11'th - behaviour as before.
    * Otherwise - one-number and two-number activations are allowed. Example:
    * ‘Win(down) 1 5’ - activates 15'th console;
    * ‘Win(down) 1 Win(up)’ - activates 1'th console.
  * Once more 'Sleep in background' bugfix.
  * [Issue 541](https://code.google.com/p/conemu-maximus5/issues/detail?id=541): Incorrect RClick file selection in PanelViews.


## Build 120514 ##
  * [Issue 543](https://code.google.com/p/conemu-maximus5/issues/detail?id=543), [Issue 544](https://code.google.com/p/conemu-maximus5/issues/detail?id=544): Fixing bugs of unlucky version :)
  * [Issue 56](https://code.google.com/p/conemu-maximus5/issues/detail?id=56): Catch focus on drag over.
  * Drag&Drop: Menu on drop in Far command line.
  * Drag&Drop: Internal changes in overlay image.


## Build 120513 ##
  * 'Ignore console cursor size' instead of not worked 'Ignore telnet cursor size'. Option exists in 'App distinct' also.
  * Fixed unworked option 'Send Alt+Space to console' (page 'Keys').
  * Checkbox 'Run as adminstrator' was ignored in create new console dialog, when starting Task.
  * [Issue 532](https://code.google.com/p/conemu-maximus5/issues/detail?id=532): Option 'Show TSA balloon' (affects automatic mode only).
  * [Issue 537](https://code.google.com/p/conemu-maximus5/issues/detail?id=537): WinRar option check before updating: [HKCU\Software\WinRAR\Extraction\Profile] "UnpToSubfolders"
  * [Issue 538](https://code.google.com/p/conemu-maximus5/issues/detail?id=538): Wrong icons in Thumbnails/Tiles modes in root of archive.
  * [Issue 539](https://code.google.com/p/conemu-maximus5/issues/detail?id=539): Wrong drawing of Thumbnails/Tiles over QView (Ctrl+Q) and Info (Ctrl+L) panels.
  * Shell Drop allowed to Far editor and dialogs. Drop as pasting dragged file path names.
  * Once more 'Sleep in background' bugfix.
  * 'Command line' option moved to new 'Startup' page of 'Settings' dialog.
  * New items 'New console dialog' and 'Setup tasks' in popup menu (`[+]` button on toolbar).


## Build 120510 ##
  * ConEmu internal paste locks clipboard (Shift+Ins, Ctrl+V).
  * Far macro sample: ConEmu\Far1\_reg\TabList.reg
  * AnsiCon raise exception in cmd/tcc.


## Build 120509a ##
  * [Issue 534](https://code.google.com/p/conemu-maximus5/issues/detail?id=534): ConEmuSetup: Bugfix in default path display.
  * Find text in consoles. Hotkey Apps+F or Edit -> Find text.
  * Telnet and 'Sleep in background' bugfix.


## Build 120508 ##
  * [Issue 65](https://code.google.com/p/conemu-maximus5/issues/detail?id=65): Checkbox ‘Inject ConEmuHk’ must be turned on for correct displaying console applications, used SetConsoleActiveScreenBuffer (telnet.exe etc.).
  * ExtendedConsole files moved to ConEmu subfolder.
  * HotKeys processing was fully remaked.
    * ‘HostKey’ term was eliminated.
    * Almost all hotkeys may use own modifiers.
    * Exceptions are: system hotkeys, console activation by number (mutual modifier) and window resize by arrows (mutual modifier).
    * Now you may use ‘keys’ WheelUp, WheelDown, WheelLeft, WheelRight (mouse wheel).
  * Up to 32 GUI Macro may be configured and assigned to HotKeys. By default, CtrlWheelUp/CtrlWheelDown will modify font height.
  * Now you can use environment variables in "`ConEmu:run:`" (Far Manager plugin). For example, you may use file association for "`*.cpp,*.hpp`" (e.g. AltF4):
```
    ConEmu:Run:"%FARHOME%\Tools\Notepad++\notepad++.exe" -new_console "!\!.!"
```
  * Optional confirmation for closing console from ConEmu (Features -> Confirm close console).
  * Portable: You may place files MsXml3.dll and MsXml3r.dll near to ConEmu.exe, for eliminating xml-problems on Live (PE) and Windows 2000. I recommend to take files from Windows XP (they have less dependencies).
  * Console recreation small bugfixes.
  * Shift was not works for menu items 'Restart', 'Restart as', 'New console'.
  * System and tab menu always pop up on RClick on the tab.
  * GUI Apps in tabs: 'Attach...' menu item works now.
  * GUI Macro: `Tab(<Cmd>[,<Parm>])`
```
    <Cmd>=0 - show/hide tabs
    <Cmd>=1 - commit lazy changes
    <Cmd>=2 - switch next (eq. CtrlTab)
    <Cmd>=3 - switch prev (eq. CtrlShiftTab)
    <Cmd>=4 - switch tab direct (no recent mode), Parm=(1,-1)
    <Cmd>=5 - switch tab recent, Parm=(1,-1)
    <Cmd>=6 - switch console direct (no recent mode), Parm=(1,-1)
    <Cmd>=7 - activate console by number, Parm=(1-based console index)
    <Cmd>=8 - show tabs list menu (indiffirent Far/Not Far)
```
  * GUI Macro: `Paste(<Cmd>[,"<Text>"])`
```
    <Cmd>=0 - paste all lines, <Cmd>=1 - paste first line
    When <Text> is omitted - paste from Windows clipboard, otherwise - paste specified text.
```
  * Keyboard: 'Show opened tabs list (does not work in Far - use macro instead)'. Default - F12.
  * ConEmu plugin: new plugin menu item - 'Show all tabs list', you may use Far macro 'ConEmu.TabList.fml'
  * [Issue 530](https://code.google.com/p/conemu-maximus5/issues/detail?id=530): Bugfix in Thumbnails/Tiles colors when ‘Optional marking character’ is turned on.
  * Вкладка настроек 'Text selection' переименована в 'Mark & Paste'.
  * ConEmu may process internally pasting text from clipboard to console applications. By default, Ctrl+V pastes first line of text, Shift+Ins pastes all lines (`<Enter>` keypress are generated between lines). Optional multiline paste confirmation and large text confirmation (200 chars by default).
  * Pasting options on 'Mark & Paste' and 'App distinct' pages of 'Settings' dialog.
  * Preferred command processor (bits) is taked in account while starting "cmd" in new tab.
  * New Tab popup menu hotkey - Apps+Space (previously Win+Apps).


## Build 120422 ##
  * [Issue 527](https://code.google.com/p/conemu-maximus5/issues/detail?id=527): Bugfixes in panel detector.
  * Incorrect window placement on startup when 'Hide caption' and 'Maximized' in shortcut properties.
  * Old width/height appears in 'Settings' dialog after 'Apply' depressed.
  * Buttons +/-/Up/Down does not work on 'App distinct' page.


## Build 120421 ##
  * **Warning**. Changes in command processor using.
  * New page 'ComSpec' in 'Settings' dialog. 'Long console output' was moved here.
  * On 'ComSpec' page located used command processor settings. Default is autoselection of tcc.exe (if installed) and cmd.exe. You can force used processor (x32/x64) in 64-bit OS. Default is the same as OS (x64 cmd.exe in Windows x64).
  * Option 'Leave on close' revealed on 'Tabs' page.


## Build 120418 ##
  * Hyperlinks to file/line does not works in buffer consoles.
  * Some speed up of command execution.


## Build 120417 ##
  * [Issue 511](https://code.google.com/p/conemu-maximus5/issues/detail?id=511): bdf-fonts may be used for Fix Far boders.
  * [Issue 523](https://code.google.com/p/conemu-maximus5/issues/detail?id=523): Edit->Mark fails on scrolled consoles.


## Build 120416 ##
  * Some settings (Palette, Cursor, Extend Fonts) may be changed separately by application name and Elevation flag. Page 'App distinct' of 'Settings' window. Separate settings will be stored in "Apps" subkey of reg/xml settings.


## Build 120414 ##
  * Checkbox 'Long console output' was not checked in 'Settings' dialog.
  * Error in Plugin.Call ("4b675d80-1d4a-4ea9-8436-fdc23f2fc14b", "...") result for Far3.
  * After Far3 build 2576 was broken macros containing $Text. E.g. 'long right-click'.
  * GUI Apps in tabs: "Notepad" in Win-W don't works.
  * Removed a small delay before switching tabs (editor/viewer) via ConEmu.
  * [Issue 516](https://code.google.com/p/conemu-maximus5/issues/detail?id=516) : When you click on the taskbar to minimize ConEmu, Win-Number was not turned off.
  * [Issue 518](https://code.google.com/p/conemu-maximus5/issues/detail?id=518) : If the Far 'hangs', then when you try to access it (switching tabs, etc.) ConEmu hang for a while.
  * [Issue 521](https://code.google.com/p/conemu-maximus5/issues/detail?id=521) : Editor/Viewer tabs disappearing.
  * [Issue 248](https://code.google.com/p/conemu-maximus5/issues/detail?id=248) : Added ability to save / load custom color palette. Сolor palette contains 32 (16+16) colors and setting 'Extend foreground colors'. Named palettes are stored in the "Colors" subkey of reg/xml settings.
  * [Issue 445](https://code.google.com/p/conemu-maximus5/issues/detail?id=445) : Added ability to create custom 'Tasks'. In fact, this is an alias to run one or more applications in new tabs ConEmu. May be configured in 'Tasks' page of 'Settings' dialog, and stored in "Tasks" subkey subkey of reg/xml settings. 'Task' may be used (for example of the {Shells}):
    * when you start by specifying the properties of the shortcut: "ConEmu.exe / cmd {Shells}";
    * specifying {Shells} in the 'Command line' page 'Main' of 'Settings' dialog;
    * when you create a new console interface ConEmu (`+` on the toolbar, a list of Recreate-dialog);
    * from the command line (cmd.exe): "%ConEmuBaseDir%\ConEmuC.exe" / c {Shells} -new\_console.
    * from the command line (far.exe): conemu:run:{Shells} -new\_console
  * Minor fix in debug switch "/ detached".
  * Vista+: When the console is set detache acceptable to the user font size.


## Build 120408 ##
  * ConEmu was mentioned on http://jpsoft.com/blogs/2012/04/windows-console-replacements-part-6-take-command-and-conemu/


## Build 120401 ##
  * Settings->Far Manager->KeyBar RClick: при подсветке пункта меню на кейбаре отображается выполняемое действие.
  * [Issue 514](https://code.google.com/p/conemu-maximus5/issues/detail?id=514): Доработана опция 'Extend foreground colors with background'. Работает для любого приложения, а не только для Far Manager. Например, для Far Manager, в качестве 'заменяемого цвета фона' можно выбрать цвет панелей (синий по умолчанию), тогда для Far получится как бы своя собственная палитра цветов. Или наоборот, можно настроить цвета для внешнего редактора (Vim), выбрав в качестве цвета расширения его цвет фона. Для Far Manager имеет смысл только для веток 1.7x и 2.x. Для ветки 3.x любые TrueColor цвета можно настроить через интерфейс Far Manager.
  * [Issue 319](https://code.google.com/p/conemu-maximus5/issues/detail?id=319): !Hyperlinks. Разрешено для любого приложения (не только в Far). Но для открытия редактора (LClick по ошибке из консоли с cmd.exe) Far должен быть запущен. При наведении меняется курсор мышки.


## Build 120327 ##
  * Far3: после выполнения макроса ShiftEnter курсор прыгал к ".."
  * Hyperlink detector (e-mails).
  * [Issue 505](https://code.google.com/p/conemu-maximus5/issues/detail?id=505): Reattach fixed.
  * Settings->Far Manager->KeyBar RClick: Правый клик по кейбару отображает меню модификаторов. Т.е. щелкнув правой кнопкой по "7Search" в редакторе откроется всплывающее меню, в котором можно выбрать любой F7 с модификатором, например Ctrl-F7.
> > Примечание. При выборе пункта меню в консоль посылается именно нажатие, т.е. если на Ctrl-F7 висит макрос - будет выполнен макрос.


## Build 120326 ##
  * Far3 build 2573.
  * Еще раз табы для модальных окон (Far3).


## Build 120325 ##
  * Текущая версия НЕ совместима с Far3 build 2567 и выше (пока).
  * Еще раз табы для модальных окон (Far3).
  * [Issue 507](https://code.google.com/p/conemu-maximus5/issues/detail?id=507): В некоторых случаях при запуске консольных приложений не отключались расширенные атрибуты символов (color/italic/...).
  * [Issue 509](https://code.google.com/p/conemu-maximus5/issues/detail?id=509): В некоторых случаях (смена режима дисплея) в консоли мог самопроизвольно включаться режим прокрутки.
  * [Issue 510](https://code.google.com/p/conemu-maximus5/issues/detail?id=510): Отключен AI приводивший к невозможности сохранить заданную пользователем высоту шрифта (сохранялась высота, с которой был создан реальный шрифт).
  * [Issue 511](https://code.google.com/p/conemu-maximus5/issues/detail?id=511): Rudimentary BDF support (from thecybershadow).
  * Не работали жесты в release сборке.
  * Touchpad & Touchscreen: после двойного тапа в консоль мог проваливаться 'лишний' LBtnClick.
  * Добавлен макрос Editor.MsRClick.fml - удобен для работы в редакторе и диалогах  на Touchpad & Touchscreen.
  * В режимах 'Hide caption...' & Maximized 'светилась' рамка окна.


## Build 120315 ##
  * Far3 build 2546.
  * Исправлено отображение табов для Far3.
  * Продолжение 'гиперссылок'. Теперь обрабатываются и URL & e-mail (открытие через ShellExecute).
  * [Issue 499](https://code.google.com/p/conemu-maximus5/issues/detail?id=499): В архив 7z добавлена 64битная версия GUI, соответственно снято ограничение на обновление 64битной версии только инсталлятором.


## Build 120314 ##
  * Far3 build 2545, рекомендуется bis-сборка.
  * Не работали жесты в release сборке.
  * Еще раз детектор 'гиперссылок' на ошибки компиляции (добавлена поддержка Delphi/FPC).


## Build 120304 ##
  * Немного поправлен детектор 'гиперссылок' на ошибки компиляции.
  * Убираем OutputDebugString.


## Build 120227 ##
  * Правки межпроцессного взаимодействия.
  * Ошибка в умолчаниях шаблонов табов для вьювера и редактора с изменениями.
  * Vista и выше. При невозможности изменить размер консоли из-за пределов по размеру консольного шрифта, ConEmu пытается увеличить/уменьшить размер шрифта в консоли. Для пользователя это 'прозрачно', шрифт в окне ConEmu не меняется.
  * Windows 7 и выше. Поддержка тачскринов и жестов.
    * Длинный тап - аналог длинного RClick, открытие EMenu.
    * Перетаскивание одним пальцем - аналог перетаскивания с нажатой левой кнопкой мышки (например, для Drag&Drop).
    * Тап двумя пальцами - аналог короткого RClick, выделение файла. Позиция - центр между пальцами.
    * Нажатие и тап (опустить первый палец, тап вторым пальцем, отпустить первый палец) - аналог короткого RClick, выделение файла. Позиция - первый палец.
    * Поворот (опустить первый палец, крутить вторым пальцем вокруг первого, отпустить первый палец) - переключение табов, по часовой стрелке - CtrlTab, против часовой стрелки - CtrlShiftTab. Активация таба - после отпускания первого пальца.
    * Прокрутка (опустить два пальца, двигать их вверх/вниз, отпустить пальцы) - аналог прокрутки колесом мышки (инверсно).
    * Зум (опустить два пальца, сблизить/раздвинуть, отпустить пальцы) - изменить размер шрифта в ConEmu.
  * [Issue 493](https://code.google.com/p/conemu-maximus5/issues/detail?id=493): При включении табов размер консоли не меняется - увеличивается высота окна ConEmu.
  * [Issue 494](https://code.google.com/p/conemu-maximus5/issues/detail?id=494): Hotkeys for starts text and block selection from keyboard.
  * [Issue 495](https://code.google.com/p/conemu-maximus5/issues/detail?id=495): Option for send 'Alt-F9' to console.
  * [Issue 496](https://code.google.com/p/conemu-maximus5/issues/detail?id=496): Edit -> Mark text doesn't start selection sometimes.
  * Опять работаем в Win2k.


## Build 120215 ##
  * Experimental: Внутренние изменения межпроцессного взаимодействия.
  * Installer. Подхватывается путь установки Far 3.x, в том числе и для компонентов «Far Manager plugins» и «Far3 ExtendedConsole».
  * Правки в injects.
  * [Issue 296](https://code.google.com/p/conemu-maximus5/issues/detail?id=296): Исправлена работа в режиме HideCaption & Maximized.
  * Префикс плагина "ConEmu:run:" теперь поддерживает и "-new\_console" для убирания мерцания панелей.
  * Доработаны макросы для Far3.
  * В заголовке таба с консольным выводом (макрос Far на CtrlO) отображается 'Console output' вместо имени временного файла.
  * Для 'Resize panels by mouse' добавлен флажок 'use both panel edges'.
  * [Issue 491](https://code.google.com/p/conemu-maximus5/issues/detail?id=491): ConEmu changes window size on F3 or F4 in Far.
  * Win+Apps - active tab popup menu.
  * Продолжение GUI-приложений во вкладках.


## Build 120205 ##
  * EMenu по правому клику (опция 'RightClick 4 EMenu') всегда отображается в позиции курсора мыши.
  * Откат поддержки injects в UPX модулях.


## Build 120204 ##
  * [Issue 485](https://code.google.com/p/conemu-maximus5/issues/detail?id=485): Проблема при запуске с включенным Desktop mode.
  * [Issue 489](https://code.google.com/p/conemu-maximus5/issues/detail?id=489): Не работает dism (правки в injects).


## Build 120203 ##
  * Продолжение GUI-приложений во вкладках. Теперь при запуске консольной утилиты/команды из GUI-приложения во вкладке - утилита/команда стартует в новой вкладке ConEmu.
  * [Issue 486](https://code.google.com/p/conemu-maximus5/issues/detail?id=486): После запуска новой вкладки в фоне - не обновлялись кнопки на тулбаре.


## Build 120201 ##
  * AutoUpdate. Сообщение о невозможности обновления 64битной версии GUI показываем только при ручном вызове.
  * Не работал Terminate (из меню) для активного процесса.
  * [Issue 484](https://code.google.com/p/conemu-maximus5/issues/detail?id=484): Не восстанавливается иконка в трее после краха проводника.
  * В меню для активной консоли (таба) вместо 'Terminate' отображается 'Terminate->Console' и 'Terminate->Active process'.


## Build 120129a ##
  * [Issue 479](https://code.google.com/p/conemu-maximus5/issues/detail?id=479): Tiles/Thumbnails не обновлялись в Far 2.x при открытом QSearch.
  * [Issue 482](https://code.google.com/p/conemu-maximus5/issues/detail?id=482): RightClick и EMenu на границе панели.


## Build 120129 ##
  * Продолжение GUI приложений во вкладках.
  * Правки в injects.
  * Настройки выделения текста и скрытия заголовка перенесены из в доп.диалогов в окно 'Settings'.
  * Far 3 build 2426 (не работал ConEmu Panel Views).
  * [Issue 481](https://code.google.com/p/conemu-maximus5/issues/detail?id=481): ConEmu.120118 — ошибка переключения табов кликами.


## Build 120118 ##
  * [Issue 337](https://code.google.com/p/conemu-maximus5/issues/detail?id=337): Attach не-фаровских консолей: System menu -> Attach to... Проверялось в Windows 7. **ТехИнфо**. Для аттача необходимо загрузить в выбранный процесс модуль ConEmuHk.dll (ConEmuHk64.dll), что выполняется посредством CreateRemoteThread. В принципе, некоторые антивирусы могут ругаться при попытке использования этой функции. Вызывается CreateRemoteThread из ConEmuC.exe (ConEmuC64.exe).
  * [Issue 480](https://code.google.com/p/conemu-maximus5/issues/detail?id=480): Answering "Detach console" with "no" still detaches console.
  * В 120115 для Far 3.x отвалился правый клик на keybar.
  * Консольным программа разрешено 'рисовать' в DC ConEmu функцией StretchDIBits (Far плагины ImageView и PicViewAdv). **ТехИнфо**. Из отрисовки в ConEmu исключается прямоугольник, указанный в последнем вызове StretchDIBits. Его отрисовка восстанавливается при изменении размеров окна или при изменении текста в консоли в этом прямоугольнике.
  * Еще немного по пропорциональным шрифтам.


## Build 120115a ##
  * Параметр "`-cur_console[:n]`" можно указать после "`ConEmu.exe /cmd`" чтобы отключить 'Press Enter or Esc to close console'. Пример для UserMenu в Far 3.x, открывает файл под курсором в hiew, запущенном в новом экземпляре ConEmu.
```
macro:post $If (Apanel.Folder || Apanel.Plugin && !!(APanel.OPIFlags & 0x20)) msgbox("Warning!!","Panel or current item type disallows Hiew execution",0) $Else print(@"conemu.exe /config hiew /cmd T:\Utils\Files\Hiew32\hiew32.exe -cur_console:n ") CtrlF Enter $End
```


## Build 120115 ##
  * Detach GUI окон.
  * Правки детектора панелей.
  * Отладочная опция: System menu -> Debug -> Load screen dump.
  * Windows 7: Overlay icon 'Shield' on taskbar for elevated tabs.
  * Far 3 build 2381: Исправим безобразие с RightClick :) при включенных 'Right selection fix' и 'RightClick 4 EMenu'.
  * При ошибках запуска корневой команды ConEmuC.exe отображает текущую директорию (Can't create process, ErrCode, Current directory, Command to be executed).
  * [Issue 477](https://code.google.com/p/conemu-maximus5/issues/detail?id=477): 'Ломка' шрифта в диалоговых окнах.
  * [Issue 478](https://code.google.com/p/conemu-maximus5/issues/detail?id=478): Отображение рамок с растровыми шрифтами при отключенном 'Fix Far borders'.


## Build 120110 ##
  * Зарегистрирован аккаунт в твиттере :) http://twitter.com/ConEmuMaximus5
  * Автообновление: описано в FAQ-ConEmu.txt
  * Автообновление: ошибки скачивания 'version.ini' отображаются только при вызове из меню 'Help' -> 'Check for updates'.
  * Подтверждение перед Detach.
  * В 'Settings' -> 'Tabs' добавлена настройка шрифта табов.
  * Внутренние изменения. Окна VirtualConsoleClassBack и VirtualConsoleClassScroll более не используются.
  * Не работало отключение флажка 'Long console output'.
  * При запуске команд из Far Manager можно использовать параметр "`-cur_console:h[N]`". "`h[N]`" аналогичен параметру в "`-new_console`" но команда будет запущена в текущей а не новой консоли.
  * [Issue 474](https://code.google.com/p/conemu-maximus5/issues/detail?id=474): ConEmu not accepting mouse wheel input when mouse coords are negative.
  * [Issue 476](https://code.google.com/p/conemu-maximus5/issues/detail?id=476): New checkbox 'Settings'->'Tabs'->'Far windows'.


## Build 120105c ##
  * [Issue 295](https://code.google.com/p/conemu-maximus5/issues/detail?id=295): Улучшена отрисовка диалогов при использовании пропорциональных шрифтов.
  * AutoUpdate. Перед запуском обновления (после закрытия окна ConEmu) выполняется проверка отсутствия других запущенных экземпляров ConEmu.


## Build 120105 ##
  * [Issue 461](https://code.google.com/p/conemu-maximus5/issues/detail?id=461): В некоторых случаях ConEmu падал при закрытии GUI приложения во вкладке.
  * [Issue 469](https://code.google.com/p/conemu-maximus5/issues/detail?id=469): YAC (Yet Another Completion) doesn't work. Для GetConsoleWindow (возвращающего окно отрисовки) GetClassName перехватывается и возвращает "ConsoleWindowClass".
  * [Issue 472](https://code.google.com/p/conemu-maximus5/issues/detail?id=472): Падение при выборе меню дисков (Alt+F1).
  * При отрисовке TrueColor через ExtendedConsole.dll сбивался цвет фона пробельных символов при совпадении с 'цветом текста'.
  * Все HotKeys теперь можно задать на вкладке 'Keys' окна настроек. Настройка клавиш с вкладки 'Tabs' убрана. На вкладке 'Tabs' добавлен флажок '«Host-key»+Number iterates Far windows'.


## Build 120102 ##
  * На главной странице проекта ( http://code.google.com/p/conemu-maximus5/ ) добавлена кнопочка '+1'. Щелкните, кому не жалко :) Ну, и тут ( http://sourceforge.net/projects/conemu/ ) тоже.
  * Far3 build 2311 (build 2343 supported).
  * [Issue 102](https://code.google.com/p/conemu-maximus5/issues/detail?id=102): AutoUpdate. Умеем обновляться как через инсталлятор, так и через 7z-архив. Для обновления через архив - нужен архиватор (7z или WinRar).
  * При запуске в новой конфигурации вместо нескольких диалогов отображается один общий диалог 'быстрой настройки'.
  * Параметр '-new\_console' не запускал во вкладке GUI приложения.
  * В системное меню добавлены пункты 'Visit home page' и 'Report a bug' (Win+Alt+Space или клик по иконке в заголовке ConEmu).
  * Еще немного FAQ-ConEmu.txt
  * [Issue 465](https://code.google.com/p/conemu-maximus5/issues/detail?id=465), [Issue 466](https://code.google.com/p/conemu-maximus5/issues/detail?id=466): Правки в ConEmuHk.
  * Часть настроек с вкладки 'Features' перенесена в новую 'Far Manager'.
  * На вкладке настроек 'Far Manager' добавлены поля для макросов. Вернуть макрос 'по умолчанию' можно очистив поле макроса.
  * [Issue 468](https://code.google.com/p/conemu-maximus5/issues/detail?id=468): System's «Copy File» window (by Ctrl+V) appears behind ConEmu.
  * При запуске ConEmu.exe выполняется проверка наличия необходимых файлов (ConEmuC.exe и т.п.)


## Build 120101 ##
  * Same as build 120102. May be used for AutoUpdate testing.




## Более старые записи ##
  * [Builds 090707 .. 111220](Whats_New_1.md)
  * [Полный лог изменений на SVN](https://conemu-maximus5.googlecode.com/svn/trunk/ConEmu-alpha/Release/ConEmu/WhatsNew-ConEmu.txt)