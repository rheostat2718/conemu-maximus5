﻿=ConEmu has moved to [conemu.github.io](http://conemu.github.io/)=
**New versions** of this file are available on **[conemu.github.io](http://conemu.github.io/en/Whats_New.html)**.

Google has declared [shutdown](http://google-opensource.blogspot.ru/2015/03/farewell-to-google-code.html) of googlecode service.


## Support development ##
You can show your appreciation and support future development by [donation](http://conemu.github.io/donate.html).


## Build 150319 ##
  * Some required switches were not used while creating jump lists (`/FontDir`, `/FontFile`, etc.)
  * `PostPromptCmd` was not working with Far 3.
  * Added hotkey to activate ConEmu window and do ‘CD’ to the last (top in Z-order) Explorer window path. GuiMacro is available:
```
PasteExplorerPath (<DoCd>,<SetFocus>)
  DoCd: 1 - ‘CD’, 0 - paste path only
  SetFocus: 1 - bring ConEmu to the top, 0 - don't change active window
```


## Build 150316 ##
  * [Issue 1931](https://code.google.com/p/conemu-maximus5/issues/detail?id=1931): Fix crash and broken Far frames after resize (regression).
  * Inform user about some project events using TaskBar status area notification. Click on the icon or information balloon will show extended information dialog where user may stop showing this notification.
  * Translate mouse wheel into escape sequences when ConEmu emulates xterm (vim especially). If you want to use mouse wheel in Vim (official version was tested), just add to vimrc:
```
""""""""""""""""""""""""""""""""""""""
" let mouse wheel scroll file contents
""""""""""""""""""""""""""""""""""""""
if !has("gui_running")
    set term=xterm
    set mouse=a
    set nocompatible
    inoremap <Esc>[62~ <C-X><C-E>
    inoremap <Esc>[63~ <C-X><C-Y>
    nnoremap <Esc>[62~ <C-E>
    nnoremap <Esc>[63~ <C-Y>
endif
```
  * Let user set up hotkey for ‘Detach’ action. Add GuiMacro ‘Detach’.
  * All HotKeys are stored now in the "`HotKeys`" settings subkey.
  * If anyone like to hide cursor in the inactive consoles just have to set inactive ‘Fixed cursor size’ to ‘0%’.


## Build 150311 ##
  * Try to fix full-width hieroglyphs drawing.
  * Hangul syllables were not defined in CJK range.
  * We need real width of CJK hieroglyphs, they might not be double as large.
  * Don't try to use invalid fonts from registry (TrueTypeFont) on DBCS systems.
  * Current unicode range was doubled in drop-down.
  * Apply changes in unicode ranges immediately if was selected from drop-down.
  * Do not uncheck ‘Monospace’ on DBCS systems by default.


## Build 150310 ##
  * Mouse events were sometimes unexpectedly sent to the console during selection.
  * 150309 regression. "`ConEmu.ico`" was looked in wrong folder.
  * Overlay icon on Win7 TaskBar was not shown in some cases.
  * Name checkbox ‘Alternative font ...’ instead of ‘... pseudographics font’.
  * TaskBar aero thumbnail of active console was not live-updated if ConEmu was minimized.
  * Don't use ‘Enhance progressbars and scrollbars’ for cells with `bg==gf` unless it is `100%` filled block.
  * Search panel was too small in some cases.
  * [Issue 1939](https://code.google.com/p/conemu-maximus5/issues/detail?id=1939): Tab bar height too big since version 150307.


## Build 150309 ##
  * [Issue 1937](https://code.google.com/p/conemu-maximus5/issues/detail?id=1937): ‘Admin shield’ in TabBar was broken.
  * Only use "`ConEmu.ico`" name as possible external icon.
  * Logging. Log ConEmu icon loading, created startup consoles, overlay icon actions.


## Build 150307a ##
  * [Issue 1931](https://code.google.com/p/conemu-maximus5/issues/detail?id=1931), gh#67: ConEmu's output was blocked after maximizing window.


## Build 150307 ##
  * gh#65: Min size for active cursor changes were not saved.
  * Create toolbar buttons as large as the size of the tabbar.
  * Take current dpi into account while creating tab icons.
  * [Issue 1929](https://code.google.com/p/conemu-maximus5/issues/detail?id=1929): Quake position was changed unexpectedly after Win+D / restore.
  * [Issue 1792](https://code.google.com/p/conemu-maximus5/issues/detail?id=1792): Unexpected console resize happens after Win+D Win+D.
  * gh#66: New tab template ‘%f’ shows shell's current folder name. Check wiki ShellWorkDir.


## Build 150305 ##
  * Wrong tab was activated sometimes by mouse click if there was a scroll buttons on the TabBar.
  * Do not do tab flickering while switching them by mouse clicks.
  * White background flickering was occured sometimes during ConEmu startup.
  * Default tasks were created sometimes not properly quoted.
  * Add ‘Ranges’ drop-down list to pseudographics font settings with some predefines.
  * [Issue 1927](https://code.google.com/p/conemu-maximus5/issues/detail?id=1927): Key-Up event for ‘Space’ and ‘Enter’ in ReadConsoleInputA.
  * gh#63: Switch `-cur_console:d:...` was not stripped from Task while running shell sometimes.
  * While creating 'Auto save/restore' task add all `-cur_console` switches before the shell itself.


## Build 150303 ##
  * [Issue 1925](https://code.google.com/p/conemu-maximus5/issues/detail?id=1925): Single character between two gray blocks was obscured (ex. OEM ‘B1 C1 B1’).


## Build 150302b ##
  * Far Tasks. Show bitness x86/x64 in the task name.
  * Far Tasks. If only one Far installation found - just use {Far} as task name.
  * Far Tasks. Sort default Far tasks by version.
  * When dropping files in the Far Manager prompt, show ‘+ Enter’ menu items first.
  * TCC tasks were not created by default.
  * [Issue 1923](https://code.google.com/p/conemu-maximus5/issues/detail?id=1923): ‘ConEmu here’ was forced to "`%USERPROFILE%`" from certain folders. If user starts ‘ConEmu here’ from "`%ConEmuDir%`", "`%windir%`" or "`%windir%\system32`" ConEmu was switched to "`%USERPROFILE%`". That's because these folders may be used if user starts ConEmu (or just cmd as DefTerm) from bare Win+R. So, the switch "`/here`" was implemented, it's not shown in the Settings dialog. User needs to re-register ‘ConEmu here’ existing commands.
  * Process ‘`ConEmu[64].exe`’ returns exit code of last terminated shell.
  * Switch "`/QuitOnClose`" forces ConEmu window closing with last tab.


## Build 150302a ##
  * [Issue 1921](https://code.google.com/p/conemu-maximus5/issues/detail?id=1921): Regression: Fix a crash on last tab close (WinXP).


## Build 150302 ##
  * Show current tab icon as Windows 7 taskbar overlay icon.
  * Visible region lock (scrolling) was unexpectedly dropped sometimes.
  * Add all installed (and found) Far Manager installations while creating default tasks.
  * [Issue 1920](https://code.google.com/p/conemu-maximus5/issues/detail?id=1920): Fix visual defect in window padding area during Quake style slide down animation.
  * Allow Quake window resizing and moving.
  * Fixed resize issues after Quake slide up/down.
  * Add to the TabMenu items ‘Split to right’ and ‘Split to bottom’.
  * Untick ‘Auto minimize to TSA’ when turning Quake mode off.
  * ToolBar buttons min/max/close were not properly shown/hidden on some options changing.
  * gh#57: Cannot restore prevoisly opened tabs if there was a task marked as ‘Default for new console’.
  * Consoles were not resized sometimes after Win+Left/Right (snap window to left/right monitor edge).
  * After restoring snapped window the snapped state was dropped (window was put to ‘Normal’ position).


## Build 150224 ##
  * One more correction of keyboard input processing.
  * Option ‘Treat font height as device units’ was not saved in new config.
  * Pseudographics font was incorrectly sized on some dpi-s.
  * HotKeys. Implemented ‘Kill all but shell’ function. Default hotkey Win+Shift+Delete for new configs. Also it is accessible from TabMenu or SystemMenu and GuiMacro: terminate all but shell process `Close(10)`, no confirm `Close(10,1)`. **Note**. If there is a shell only - confirm user to kill the shell.
  * Don't do "ConEmuCpCvt" conversion if some chars were failed.
  * Ability to change ‘processor affinity’ and ‘process priority’ for all running processes. The function affects all running processes in the current console (tab/pane). Available from TabMenu and SystemMenu item ‘Affinity/priority...’, HotKey and GuiMacro `AffinityPriority([Affinity,Priority])`. Unless you specify ‘Affinity’ or ‘Priority’ in GuiMacro function ConEmu will show dialog with affinity/priority options.
  * [Issue 1911](https://code.google.com/p/conemu-maximus5/issues/detail?id=1911): Do not scroll out of found position after clicking in the console to allow select text there.
  * Option ‘Skip click on activation’ was not working sometimes.
  * Allow physical scroll down of the console until the cursor remains visible.
  * Internal. Fix wrong logging message for WA\_ACTIVE
  * During excessing keyboard activity console contents was updated slowly.
  * Switch "/bypass" creates processes with normal priority.
  * gh#59: Message ‘Failed to start task in user mode, timeout’ was appeared after updating.
  * gh#56: Do not use RVAL\_REF with GCC
  * LogFiles were not created for far.exe sometimes.
  * Do not flush logs after each line to avoid lags.

## Build 150224a ##
  * [Issue 1917](https://code.google.com/p/conemu-maximus5/issues/detail?id=1917): Avoid ‘Win+Shift+Delete is not unique’ error.


## Build 150218 ##
  * [Issue 1817](https://code.google.com/p/conemu-maximus5/issues/detail?id=1817): Fix dropped or garbled keypresses sent from AHK.
  * Don't suggest by default xml storage on Windows 2000.


## Build 150216 ##
  * GuiMacro. [Issue 1411](https://code.google.com/p/conemu-maximus5/issues/detail?id=1411), gh#58. `WindowPosSize("<X>","<Y>","<W>","<H>")`. It changes window pos and size, same as ‘Apply’ button in the ‘Size & Pos’ page of the Settings dialog.
  * GuiMacro. Simplify passing string arguments. For example, the following commands will be equal now:
```
ConEmuC -GuiMacro WindowPosSize "0" "0" "800px" "600px"
ConEmuC -GuiMacro WindowPosSize 0 0 800px 600px
```


## Build 150215 ##
  * Add menu item ‘Online &Help’ to the SystemMenu.
  * After AppDistinct settings change console was not redrawn properly sometimes.
  * Sometimes true colors were not used while executing smth like "`edit:<git log`".
  * Outscreen regions were not cleared by "cls" executed in "`Far /w`" prompt.
  * [Issue 1881](https://code.google.com/p/conemu-maximus5/issues/detail?id=1881): Newly catched admin consoles were unexpectedly scrolled to the bottom (DefTerm).
  * Catched RealConsole (DefTerm) was hidden even if ‘Show real console’ was ticked.
  * Use bitmap resource for ‘Search’ icon.


## Build 150209 ##
  * [Issue 1897](https://code.google.com/p/conemu-maximus5/issues/detail?id=1897): Tear off started "ConEmu.exe" from "taskeng.exe" (switch /demote). Let process created from task sheduler runs indefinitely.
  * Auto save window size & pos is turned on by default.
  * Add test batch "utf-8-test.cmd" (check console unicode capabilities).
  * Suggest ConEmuXml storage for new configurations by default.
  * [Issue 1870](https://code.google.com/p/conemu-maximus5/issues/detail?id=1870): Rename dialog was shown out of screen if tabs were on bottom.


## Build 150206 ##
  * CP hack: Use ConEmuCpCvt to force output CP conversion. If some command uses wrong CP while converting ANSI to Unicode (the example may be perl.exe from "git add -p") set variable to correct its output. Use asterisk for all apps. This hack affects ONLY WriteConsoleW function calls.
```
Format:
  <exename>:<badcp>:<goodcp>[;<exename>:<badcp>:<goodcp>[...]]
Example:
  set ConEmuCpCvt=perl.exe:1252:1251;*:850:866;
```
  * CP hack: Use ConEmuDefaultCp to change output CP if you don't want to use chcp. If you can't use "`chcp <codepage>`" to change whole console CP output, you may change it using environment variable ConEmuDefaultCp. It will affect only WriteFile and WriteConsoleA functions. So, if you need to run several apps in one console simultaneously...
```
set ConEmuDefaultCp=1251
ConEmuC -fork -c App1.exe
set ConEmuDefaultCp=866
ConEmuC -fork -c App2.exe
set ConEmuDefaultCp=
```
  * Add information about "-Async", "-Fork", "-Download" switches.
  * Switch "-noquake" implies "-nosingle".
  * TaskBar jump list was created without directly specified xml configuration file.
  * Button ‘Save’ instead of ‘Start’ was shown if bad startup Task was detected.
  * The menu item ‘Clear history...’ was not working (the one in the `[+]` drop-down menu).
  * Sometimes ‘Console script are not supported here’ error appeared after running a task from NewConsole dialog.
  * Tasks may be organized with folders. Delimit task name from folder name with "`::`". For example, type "`Shells::PowerShell`" instead of "`PowerShell`", and so on. Only one level of folders is allowed. No need to reorder tasks of one folder continuously, folders are filled by name. Generally this was implemented for drop-down `[+]` menu.
  * Create default tasks with folders (Shells, Bash, SDK, and so on).
  * Find task by name with or without folder part (Startup page).
  * Button 'Add default tasks' will not create task duplicates (checking by task name).
  * Use self-implemented UnExpandEnvStrings while creating default tasks.
  * Allow switch '/reuse' in task parameters (alias for '/single' yet).


## Build 150203 ##
  * [Issue 1886](https://code.google.com/p/conemu-maximus5/issues/detail?id=1886): Change text cursor position with ‘LeftClick’ was not working when PSReadLine was loaded in your `$profile`. Look at `RevokeMouseInput.ps1`.
  * github#20: ‘Ctrl-Left’ and ‘Ctrl-Right’ was not working in Vim/xterm.
  * Create new console confirmation now if disabled by default. So ‘Win+W’ or ‘LClick’ on the `[+]` button creates new default task. The default task may be choosed in the ‘Tasks’ settings page. New console dialog (command, directory, as admin, and so on) may be shown with:
    * ‘Win+Shift+W’;
    * ‘Shift+LClick’, ‘RClick’ or ‘LongTap’ on the `[+]`;
    * ‘New console dialog...’ menu item (down arrow about `[+]`).
  * Window and tab closing confirmations now are disabled by default. But if there are more than a two user processes running in the console - close confirmation may be shown. So, there are thee checkboxes now:
    * Confirm window closing (unchecked);
    * Confirm tab closing (unchecked);
    * When running process was detected (checked).


## Build 150128 ##
  * Add missed GuiMacro GetOption description.
  * Debug. Show ConEmu build number in the `_ASSERT` dialog
  * [Issue 1878](https://code.google.com/p/conemu-maximus5/issues/detail?id=1878): GuiMacro. Add ability to get some environment from ConEmu.
```
GetInfo("PID"[,"HWND"[,...]])
  - Returns values of some ConEmu environment variables
    GetInfo("PID") returns %ConEmuPID% and so on
    It's processed in GUI so the result may differs from RealConsole
```
  * Search field sometimes disappeared after playing with toolbar visibility.
  * Add small gap between search field and right window frame edge if toolbar is hidden.
  * HighlightAndGoto: Add examples with 'cmd.exe /c ...'.
```
cmd.exe /c "echo Executing "%3" & "%3""

After Ctrl+Click on highlighted (underlined) file
ConEmu will start cmd.exe in new tab and highlighted file will be executed.
Documents will be opened using associated application.
Add "-new_console:n" suffix to suppress "Press Enter or Esc to close console".
```
  * [Issue 1883](https://code.google.com/p/conemu-maximus5/issues/detail?id=1883): Search field activation hotkey was not working if toolbar buttons were hidden.
  * http://conemu.github.io/en/AnsiEscapeCodes.html AnsiEscapeCodes]: Change behavior on wrong ST terminator of OSC detected (allowed 'ESC \' or 'BEL').
  * [Issue 1865](https://code.google.com/p/conemu-maximus5/issues/detail?id=1865): Deadlocks were happened sometimes, PipeServer termination sequence was changed.


## Build 150119a ##
  * [Issue 1876](https://code.google.com/p/conemu-maximus5/issues/detail?id=1876): Revert msvcrt.dll linkage.


## Build 150119 ##
  * [Issue 1864](https://code.google.com/p/conemu-maximus5/issues/detail?id=1864): ExportEnvVar. Drop variable instead of setting it to empty string.
  * Don't show both small and large icons in the confirmation/about dialogs.
  * Internal. GuiMacro "Context" was not returned result as expected.
```
BTW, usage example: print("abc"); Split(2); Context(); print("def")
```
  * Minor change in 16x16 icon, add 256x256 icon, update icons in setupper.
  * Allow ‘Debug active process’ while ChildGui is stopped by ConEmuReportExe on loading.
  * Remove GetVersionEx warnings (internal).
  * Debug purposes switch: ConEmuC -OsVerInfo. Sets errorlevel to OsVer. Ex.: Win7 -> 1537 (0x601)
  * Link ConEmuHk[64](64.md).dll and ConEmuCD[64](64.md).dll statically with msvcrt.dll.
  * [Issue 1865](https://code.google.com/p/conemu-maximus5/issues/detail?id=1865): Fix weird hungs in LdrpAcquireLoaderLock (cmd.exe).
  * Emulate AnsiCon environment variables.
```
"ANSICON" = "WxH (wxh)", where W,H size of the buffer, w,h - window.
"ANSICON_DEF" = "BF", (B)ackground and (F)oreground color indexes (hex).
"ANSICON_VER" = "170", does not exist in the environment block.
```
  * Support some more ANSI SGR codes (22, 23, 24).
```
"ESC [ 22 m" - Unset BrightOrBold
"ESC [ 23 m" - Unset ItalicOrInverse
"ESC [ 24 m" - Unset BackOrUnderline
```
  * [Issue 1865](https://code.google.com/p/conemu-maximus5/issues/detail?id=1865): Allow ANSI escape sequences when redirecting to device CON.


## Build 150111 ##
  * New ConEmu icon.
  * HighDpi fixes (if primary mon is 192dpi and secondary is 96dpi).
    * Search icon size on secondary monitor was huge.
    * Settings dialog controls were sized too large.
  * DefTerm. Aggressive mode was not be able to disabled actually.
  * GuiMacro. Allow to execute some GuiMacro after ConEmu window creation. Example:
```
ConEmu -Detached -GuiMacro "Task(""{cmd}""); Settings"
```
  * [Issue 1861](https://code.google.com/p/conemu-maximus5/issues/detail?id=1861): GuiMacro. `Context([<Tab>[,<Split>]])`.
  * Don't try to start clink if cmd is started just for single command execution.
  * Fix incorrect buffer address passed when getting AutoRun reg key value.
  * Small improvements for build settings in VS12 project.


## Build 141221 ##
  * [Issue 1846](https://code.google.com/p/conemu-maximus5/issues/detail?id=1846): Script is not supported here (5909d9f regression).
  * Split options set in the ‘Create new console’ dialog not take precedence of any specified in the Task itself (first task's command actually).
  * CmdInit.cmd: Print Windows version while initializing.
  * Add "`ConEmuC /EXPORT=GUI [Var1 [Var2 [...]]]`" for exporting vars to ConEmu GUI only.
  * DefaultTasks. Use registry's ‘App Paths’ while searching for exe files.
  * Let Git bash use git.ico and try to find it using registry.
  * In some cases ConEmuHk was not properly deinitialized within cygwin/msys.
  * Turn on ‘Activate split on mouse hover’ for new config by default.
  * "ConEmuC /export" was unexpectedly exporting to all tabs.
  * Change finalization sequence of ConEmuHk.


## Build 141217 ##
  * Reorder create console hotkeys.
  * Now it's possible to choose ‘Default task for new console (Win+W)’ and ‘Default shell (Win+X)’.
  * Let's choose which tasks add to Windows 7 jump lists (check box ‘Taskbar jump lists’).


## Build 141216 ##
  * [Issue 1705](https://code.google.com/p/conemu-maximus5/issues/detail?id=1705): In some cases ConEmuBg config xml file was not loaded properly.
  * Prefill MemoryDump Save as dialog with default dump file name.
  * Debugger. Fix creation of several memory dumps at a time.
  * Debug. Add menu item ‘SystemMenu\Debug\Active tree memory dump’.
  * Tab was sometimes closed unexpectedly while recreating as Admin.
  * Inherit environment variables while starting new tab from existing one (-new\_console, start).
  * [Issue 1840](https://code.google.com/p/conemu-maximus5/issues/detail?id=1840): Administrator privilege was not persisting to child tabs.
  * Fix ‘Numpad with NumLock is ON’, ‘Tab’ and ‘Shift-Tab’ processing within xterm keyboard emulation (Vim).
  * LogFiles ‘`ConEmu-Input-*.log`’ were not created in some cases.


## Build 141208 ##
  * [Issue 980](https://code.google.com/p/conemu-maximus5/issues/detail?id=980): Far Manager was not resized properly after exiting NTVDM application.
  * Status `[Disabled]` was not shown for ‘Delete word leftward to the cursor’ hotkey.
  * While selection is present - force RClick to copy/paste internally if ‘Intelligent mode’ is on.
  * Add status bar columns for VCon window sizes in pixels.


## Build 141206 ##
  * [Issue 980](https://code.google.com/p/conemu-maximus5/issues/detail?id=980): Fix resize issues with NTVDM (old DOS) applications.
  * Some more internal logging.


## Build 141204 ##
  * [Issue 1832](https://code.google.com/p/conemu-maximus5/issues/detail?id=1832): Some file size zeros was missed in Far's Tiles mode.
  * Update counters FPS/RPS calculation.
  * Add to counters keyboard performance (delay between keypress and console reaction).


## Build 141203 ##
  * Powershell's "`get-help Get-ChildItem -full | out-host -paging`" did not react to Space/Enter.
  * LogFiles: Log ‘`IsAdmin`’ state and `ServerPID` changes.
  * Fix Coverity warnings (from Michael Lukashov).


## Build 141201 ##
  * In some cases using ‘Cmd\_Autorun’ was causing crashes.
  * Far active directory was not refresed in Tab labels if macro ‘%d’ was used in template.
  * Use simple "`ConEmuC -IsConEmu`" instead of "`ConEmuC -GuiMacro IsConEmu`" in ‘Cmd\_Autorun’.
  * Was not warned about wrong place (invalid switch) of "`-new_console:a`" with ConEmu.exe.
  * [Issue 1828](https://code.google.com/p/conemu-maximus5/issues/detail?id=1828): ‘Cmd\_Autorun’ fails if processes were started under different credentials.
  * Command was forced into new ConEmu tab even if `-new_console` was quoted with non-double-quotes.


## Build 141130 ##
  * Redesign ‘Features’ settings page.
  * Sleep in background: don't force inactive tabs to high activity state on startup.
  * More server-side logging of ‘sleep in background’.
  * Fix multiple [Sleep](Sleep.md)-s appeared in tabs (ConEmuSleepIndicator=TITLE).
  * In some cases wrong installer bitness was used with autoupdate. If ConEmu.exe was started but both x86 and x64 versions were installed.
  * [Issue 1812](https://code.google.com/p/conemu-maximus5/issues/detail?id=1812): Bypass Windows 10 bug with GetConsoleDisplayMode.
  * Changes in the setting ‘tab flash count’ field were ignored.
  * [Issue 1826](https://code.google.com/p/conemu-maximus5/issues/detail?id=1826): Changes of the checkbox ‘Aggressive mode’ were ignored.
  * Let inform user about unhandles checkbox clicks.
  * Don't leave flashed tab in highlighted state by default. If one need to leave it highlighted set flash counter to ODD value.
  * InactiveFlash: Running Far ignore top visible line (with clock).
  * InactiveFlash: Don't flash while Far is doing something with progress.
  * InactiveFlash: Support flashing for Far viewers also.
  * InactiveFlash: Use tab suffix instead of separate template.
  * [Issue 1825](https://code.google.com/p/conemu-maximus5/issues/detail?id=1825): Don't try to force Macro execution.


## Build 141126 ##
  * [Issue 1590](https://code.google.com/p/conemu-maximus5/issues/detail?id=1590), [Issue 1793](https://code.google.com/p/conemu-maximus5/issues/detail?id=1793): Slow input response in some cases (after startup or tab switching).
  * Some controls on the ‘Features’ page were overlapped.


## Build 141125 ##
  * [Issue 1821](https://code.google.com/p/conemu-maximus5/issues/detail?id=1821): Don't force "start /min ..." to be new ConEmu tab.
  * [Issue 1822](https://code.google.com/p/conemu-maximus5/issues/detail?id=1822): Option to disable forcing "start" to be new ConEmu tab.
  * Don't block row/col highlight updating if mouse is inside VCon even if ConEmu is not a foreground window.
  * Let switch and update row/col highlight simultaneously for all panes in group.
  * Don't flash tab created in background.
  * Don't flash just deactivated tab.
  * Shorten color scheme name <Default Windows scheme>.
  * Move ‘Schemes’ drop down to the top of ‘Colors’ settings page.
  * Redesign ‘Fast configuration’ dialog.


## Build 141123 ##
  * Add `<Base16>` color scheme.
  * LogFiles: Log Far plugin PID changes.
  * LogFiles: Log Far macro execution.
  * Show in the window title ‘`{%%}`’ if the progress was set to indeterminated state.
  * Remove lag switching to inactive cmd's tab after window resize.
  * Apps+G must group input only active panes.
  * [Issue 1161](https://code.google.com/p/conemu-maximus5/issues/detail?id=1161): Maximize/restore active pane. Apps+Enter, GuiMacro Split(3), optional DblClick on tab.
  * Try to force set CF\_LOCALE (optional setting "`CTS.ForceLocale`") while copying.
  * Make GitShowBranch.cmd multi-process aware.
  * Make close confirmation texts clearer.
  * Win+Ctrl+Del closes active group.
  * [Issue 1807](https://code.google.com/p/conemu-maximus5/issues/detail?id=1807): Allow GuiMacro execution in specified tab or split.
```
ConEmuC /GUIMACRO[:PID|HWND][:T<tab>][:S<split>] <GuiMacro command>
<tab> is 1-based tab index
<split> is 1-based pane index of selected/active group of panes
```
  * [Issue 1819](https://code.google.com/p/conemu-maximus5/issues/detail?id=1819): ConEmu broke cygwin's pipeline in some rare cases.
  * [Issue 1809](https://code.google.com/p/conemu-maximus5/issues/detail?id=1809): Inactive (invisible) tab flash on activity.
    * Setting "`TabFlashChanged`". 0 - disabled, >0 - infinite, ODD - leave highlighted, EVEN - turn off after flashing.
    * Setting "`TabModified`" set template for those modified tabs.
    * Both settings are available on ‘Tabs’ settings page.
  * Use keyboard hooks to process ‘Switch focus between ConEmu and ChildGui’.
  * Use Win+Z (by default) for ‘Switch focus between ConEmu and ChildGui’.
  * Was not able to set focus into ConEmu after `CtrlWinAltSpace` with ChildGui.


## Build 141117 ##
  * Default color scheme set back to `<Solarized>`. To get git's proper colored output for `<Solarized>` scheme run:
```
git config --global color.diff.new "green bold"
```
  * Extend fonts listboxes were not enabled properly.
  * Esc was not working in the ‘Commands’ field on the ‘Tasks’ settings page.
  * Apply button on ‘Size & Pos’ page was not working, size fields were rolled back after ‘Apply’ was pressed.
  * Command "start cmd" from ConEmu tab must start new tab.
  * [Issue 653](https://code.google.com/p/conemu-maximus5/issues/detail?id=653): New ConEmu.exe switches: `-Wnd{X|Y|W|H} <Value>`. This will force ConEmu window to position/size. Size supports cells (`-WndW 80`), percents (`-WndW 50%`) and pixels (`-WndW 100px`).
  * [Issue 1815](https://code.google.com/p/conemu-maximus5/issues/detail?id=1815): ConEmu panel view plugin does not despect AppDistinct palette overrides.


## Build 141116 ##
  * [Issue 1812](https://code.google.com/p/conemu-maximus5/issues/detail?id=1812): Vertical scroll bar disappeared in latest Win10 build.
  * Use modified `<SolarMe>` palette by default (for new configs).
  * Show error box if Server and GUI has wrong protocol versions. Log errors also.
  * LogFiles: Add root shell's start/exit logging.
  * LogFiles: Log ‘CMD's AutoRuns’ strings from registry.
  * LogFiles: Log Prompt/HookServer starts.
  * Show actual 4-bit colors in the ExtendedConsole color selection dialog.


## Build 141115 ##
  * Show asterisk insted of minus in the ‘Visible region lock’ StatusBar column.
  * AutoUpdate: Postpone was not working.
  * AutoUpdate: Don't show spare close confirmation after ‘Close & Update’ confirmation.
  * More logging of console buffer changes.


## Build 141114 ##
  * Scroll position was ignored when starting selection with ‘Freeze console...’ enabled option.
  * Don't reset scroll position on KeyUp, only KeyDown are taken into account.
  * New switch "`-cur_console:R`" to force start of hooks server in the parent process.
  * AnsiEscapeCodes: Color change was ignored specified before ‘`$E]9;8;"env"$E`’.
  * Add "`CmdInit.cmd`" to "`%ConEmuBaseDir%`". Set up pretty cmd prompt with optional git info.
  * GitShowBranch: Take into account PROMPT settings was set in the "`CmdInit.cmd`".
  * Redesign About dialog (tabs on the right).
  * Add ‘SysInfo’ page to the About dialog.
  * [Issue 1799](https://code.google.com/p/conemu-maximus5/issues/detail?id=1799): Log clipboard operations.
  * Fix dialog flickering while dragging to monitor with another dpi value.


## Build 141113 ##
  * [Issue 1745](https://code.google.com/p/conemu-maximus5/issues/detail?id=1745): Fix overscaled dialogs.


## Build 141112 ##
  * [Issue 1789](https://code.google.com/p/conemu-maximus5/issues/detail?id=1789): Far Thumbnails - current item was not changed if QSearch was active (Far 3 api break). Patch works with Far3bis 4188+ only.
  * Far Thumbnails - fix arrow move if QSearch is active.
  * Force using "`%ConEmuDrive%`" instead of "`%SystemDrive%`" when populating default tasks.


## Build 141111 ##
  * Group keyboard input for visible splits: Apps+G.
  * [Issue 1803](https://code.google.com/p/conemu-maximus5/issues/detail?id=1803): Unable to check quake-style setting ‘Auto-hide on focus lost’.
  * Fix Coverity warnings (from Michael Lukashov).
  * Fix broken theme on search control after quake activation.
  * FileLineDetector: Fix broken "`abc.py (1): ...`".
  * FileLineDetector: Fix broken mail detection, support stop on punctuations.


## Build 141110 ##
  * [Issue 1802](https://code.google.com/p/conemu-maximus5/issues/detail?id=1802): GuiMacro: Add ‘Pause’ ability. When you press ‘Pause’ RCon is forced into ‘internal selection’ state which cause pausing of most console writing applications.
  * StatusBar text color was broken after ‘Visible region lock’.
  * Old ‘PicViewSlide’ hotkeys were removed.
  * Donate link was changed.
  * FileLineDetector: Fix broken "`file.cpp(123) : error...`".
  * FileLineDetector: Support quotation trimming.
  * Fix warnings and typos (from Michael Lukashov).


## Build 141109 ##
  * [Issue 1801](https://code.google.com/p/conemu-maximus5/issues/detail?id=1801): FileLineDetector: In addition to a/ b/ support i/ c/ w/ o/ also.
  * Move StatusBar's ‘Visible region lock’ column near to ‘Active console buffer’.
  * GuiMacro. Add description of ‘Progress 3’.
  * If ‘Copy on Left Button release’ was unchecked console was unexpectedly scrolled even if LButton was Up.
  * Fix admin consoles initialization.
  * Fix PVS warnings (from Michael Lukashov).


## Build 141107 ##
  * FileLineDetector. Fix false detection of "`index 621e2f9..d0abbf9 100644`".
  * FileLineDetector. Support git diffs style: "`a/src/ConEmu.cpp`" and "`b/src/ConEmu.cpp`".
  * Splits was not taken into account when resizing window by top/bottom edges.
  * Hotkey Win+Alt+T shows ‘Tasks’ settings page.
  * After palette change selected color option was ignored, was used ‘RRR GGG BBB’.
  * Fix search pane size/pos (theming).
  * Don't beep on Esc press in search pane.
  * Lets stop search by F10 keypress.
  * If search pane was empty clicking in the workspace was not stop searching.
  * Lets drag window by free place of the TabBar.
  * Fix Coverity warning (from Michael Lukashov).


## Build 141106 ##
  * When text is pasted immediately with RClick while selection is active - do not use Windows clipboard.
  * Search control position fix.
  * Rename tabs in the About dialog.
  * Switch cross-highlighting under mouse cursor with Apps+X (row highlighting - Apps+L).


## Build 141105 ##
  * Some splits were not displayed on startup (d592a05 regression).
  * ConEmuEnvironment: `set ConEmuIsAdmin=ADMIN` for those tabs/splits which were started elevated (UAC).
  * Add colored prompt to default cmd's tasks.
  * TabMenu was reorganized. Some menu items were moved into submenus.
  * Internal. Try to wait a little if GUI was not in time with RCon server initialization.


## Build 141104a ##
  * [Issue 1792](https://code.google.com/p/conemu-maximus5/issues/detail?id=1792): RBTray caused console crop (ff25dbc regression).
  * Use `<Solarized>` palette by default.


## Build 141104 ##
  * [Issue 1790](https://code.google.com/p/conemu-maximus5/issues/detail?id=1790): Minimize/restore crops console size (ff25dbc regression).
  * Fix Coverity warnings and leaks (from Michael Lukashov).


## Build 141103 ##
  * Don't snap-jump by Win+Arrow if About dialog is at foreground.
  * Search for opened tab by full path instead of name only while Ctrl+Click on hyperlink (compiler errors).
  * Far Macro: Use ‘`elseif`’ keyword.
  * Far Macro: Use ‘`ACTL_SETCURRENTWINDOW`’ in ‘`ConEmu.Editor.lua`’.
  * Use solarized colors for StatusBar by default.
  * fix PVS warnings (from Michael Lukashov).
  * [Issue 1784](https://code.google.com/p/conemu-maximus5/issues/detail?id=1784): Ctrl+Click did not recognize: C:\Work\Test.php:36
  * Change autoupdate source information to github.
  * Fix wrong per-monitor-dpi jumps.


## Build 141101 ##
  * Was failed with creating new xml config if empty xml file with BOM was exists.
  * Fix hung up after changing dpi with GuiMacro from command line.
  * Fix messed Far window number in the ‘ConEmu - tabs list’ (another Far api break).
  * Predefined palettes `<Solarized>` and `<Solarized Light>` were changed.
  * Init by default extended colors (16..31) with standard windows palette.
  * Support new parsing format in the colors edit fields ‘00BBGGRR’ (just for copying from "`*.reg`" files).
  * Increase max text length in colors edit fields (copy/paste purposes).
  * Add button ‘Default’ to the ‘Colors’ settings page. It resets colors 16..31 to standard windows palette.
  * Add radiobutton styles of color formats.
  * Minor RunQueue optimizations and other internal changes.


## Build 141029 ##
  * Show menu item hints in the StatusBar instead of tool tips.
  * [Issue 1731](https://code.google.com/p/conemu-maximus5/issues/detail?id=1731): Fix assertion on cancel split window dialog (user/password confirmation).
  * Fix resize/minimize/restore lags and asserts.
  * Don't create new console if user cancels new console dialog (split window, user/password confirmation).
  * Between-monitor jump fixes (per-monitor dpi related).


## Build 141028 ##
  * Some listboxes in the Settings dialog was not filled properly. ‘EOL’ and ‘Back color’ were messed up.
  * Foreground color for selected text was ignored.
  * Show selection colors preview on the ‘Mark/Copy’ settings page.
  * Far's ‘User screen transparency’ was not working properly.
  * Selection status message was not dropped when ‘Find text’ dialog was closed.
  * Create search control in the Tab bar. It may be focused by old shortcut (‘`Apps+F`’ by default). When search control is focused, one may open search menu by ‘`Apps`’ or RClick. Control may be shown/hid with checkbox on the ‘`Appearance`’ settings page.
  * [Issue 1777](https://code.google.com/p/conemu-maximus5/issues/detail?id=1777): Skip Far macro execution errors in some cases.
  * Internal fixes of wrong `MF_SEPARATOR` usage.
  * Force search scroll-buffer on search repeat (‘Freeze console’ must be on).


## Build 141025 ##
  * [Issue 1321](https://code.google.com/p/conemu-maximus5/issues/detail?id=1321): Splitter calculation changed.
  * Try to move splitter with mouse dragging.
  * Consider monitor dpi in splitter size.
  * [Issue 1747](https://code.google.com/p/conemu-maximus5/issues/detail?id=1747): ‘Menu’ `->` ‘Close or kill’ `->` ‘Close all zombies’. This will close all dummy consoles, waiting for "`Press Enter or Esc ...`". Hotkey is available and new GuiMacro `Close(9[,1])`.
  * [Issue 1734](https://code.google.com/p/conemu-maximus5/issues/detail?id=1734): Try not to call `FCTL_GETPANELDIRECTORY` to get panel directories (ftp plugin issues).
  * Fix wrong StatusBar's first column width evaluation.
  * Due to useless Far3's window number let use 0-based window indexes as Far3build2798 and lower.
  * Adapt "`ConEmu.ShiftEnter.lua`" to broken Far windows behavior.
  * Add "`%ConEmuExeDir%`" before "`%ConEmuBaseDir%`" in "`%PATH%`". So user may copy batch from "`%ConEmuBaseDir%`" to "`%ConEmuExeDir%`", make desired changes and be sure it will not be overwrited after new update. For example, move "`csudo.cmd`" from subfolder with "`ConEmuC.exe`" to the folder with "`ConEmu.exe`" and change the option in the beginning to: "`set ConEmuSplit=NO`".
  * Don't show "`mp_RCon->m_GetDataPipe.Transact failed`" errors, log them only.


## Build 141022 ##
  * Correct snap/tile ConEmu window to left/right/by-width/by-height.
  * Show snap/tile status on the Settings/Info page.
  * Rename maximize/tile hotkeys' descriptions. ‘Maximize height / width’ `->` ‘Snap .. to top/bottom / left/right edges ...’ and ‘Tile ConEmu ...’ `->` ‘Snap ConEmu ...’
  * Update Far3/lua headers to build 4040.
  * Let `Plugin.SyncCall("4B675D80-1D4A-4EA9-8436-FDC23F2FC14B","...")` returns result as string if succeeded.
```
lua:=Plugin.SyncCall("4B675D80-1D4A-4EA9-8436-FDC23F2FC14B","IsConEmu")
lua:=Plugin.SyncCall("4B675D80-1D4A-4EA9-8436-FDC23F2FC14B","FindEditor: C:\\autoexec.bat")
```
  * Hyperlink detector false detect: full directory paths.
  * GuiMacro: New function `GetOption("<Name>")` return string representation of ConEmu option. Example for "Internal CtrlTab" option check (Far 3 lua macro)
```
if Plugin.SyncCall("4B675D80-1D4A-4EA9-8436-FDC23F2FC14B","GetOption TabSelf") == "1" then
  far.Message("ConEmu 'Internal CtrlTab' is ON")
end
```
  * [Issue 1772](https://code.google.com/p/conemu-maximus5/issues/detail?id=1772): Scroll page up/down stopped to work.
  * Support another Far 3 API breaking change (tabs).


## Build 141020 ##
  * Try to hilight "`folder\file`" and do not hilight fakes and dirs.
  * Internal. Helper script `NewClass.ps1` for creating in-project cpp/h pairs
  * Make possible to use "`\e]0;text\a`".
  * Let process "`^a`" in "`ConEmuC -e <string>`" (`->` "`\x07`").
  * [Issue 660](https://code.google.com/p/conemu-maximus5/issues/detail?id=660): Switch "`/AUTOATTACH`" was not run asynchronously in some cases.
  * Attach: Switch "`/GHWND=NEW`" may create garbaged command line.
  * Attach: If "`/AUTOATTACH`" is specified, run server of same bitness as root/parent process.
  * Scroll. Changing scrolling style. All scrollings in ConEmu are now processed virtually, RealConsole is not touched anymore.
  * Add StatusBar column: Visible region lock.
  * Favor first and grip status columns to be fully visible.
  * Log tab activation errors (Far windows).


## Build 141017 ##
  * Far editor was opened at 1-st line instead of stored cursor position on `Ctrl+LClick` on filename without line number.
  * [Issue 657](https://code.google.com/p/conemu-maximus5/issues/detail?id=657): Eager+recent tab switching doesn't cycle all tabs.
  * [Issue 1763](https://code.google.com/p/conemu-maximus5/issues/detail?id=1763): Assertion while starting something with redirection.
  * [Issue 1764](https://code.google.com/p/conemu-maximus5/issues/detail?id=1764): Some files/hyperlinks detector issues.


## Build 141016 ##
  * [Issue 1758](https://code.google.com/p/conemu-maximus5/issues/detail?id=1758): Support file/line format for php: "`C:\..\test.php:28`".
  * Hyperlinks: Check file existence before opening.
  * Hyperlinks: If clicked file was not found, try to find it in subfolders.
  * Hyperlinks: Try to make file path properly cased before opening (`realconsole.cpp` `->` `RealConsole.cpp`).
  * [Issue 1754](https://code.google.com/p/conemu-maximus5/issues/detail?id=1754): SystemMenu was opened by RClick on the title bar was not working on some XP machines.
  * [Issue 166](https://code.google.com/p/conemu-maximus5/issues/detail?id=166): Tab switch was not working from ‘ConEmu plugin `->` T’.
  * Do not turn on ‘Long console output’ during "`edit:<command`" operations.
  * Tabs. SetTabs may fails for new Editor if it was opened from "`edit:<command`".
  * Mouse wheel logging.
  * Buffer was dropped (scrolling disabled) unexpectedly after "`ConEmuC -c <command>`" execution.
  * Just for fun, add "`-fork`"/"`-async`" switch (sort of trailing ‘&’ in bash). For example, run from cmd.exe prompt and observe weird results :)
```
ConEmuC -async -c dir & ConEmuC -async -c dir
```


## Build 141013 ##
  * [Issue 1753](https://code.google.com/p/conemu-maximus5/issues/detail?id=1753): Cursor goes out of screen while executing commands from "far" without "/w".
  * [Issue 1754](https://code.google.com/p/conemu-maximus5/issues/detail?id=1754): Some changes with using WM\_SYSCOMMAND.
  * More logging of SystemMenu processing.
  * If SystemMenu was opened by `LClick` on the title bar icon and closed by Esc promptly ConEmu window may be closed.
  * `ConEmu.ShiftEnter.*`: Check if plugin is ready to process "`ConEmu:run:`" prefix.
  * Do not request synchronous paint of the status bar from background threads.


## Build 141012 ##
  * Ensure that just installed (new config) ConEmu's window will not be larger that monitor working area.
  * While converting old config do not get data from LogFont if it was not created yet.
  * Use wiki page ‘Issues’ instead of ‘/issues/entry’ to avoid bad authorization redirection.
  * Fix `Apps+PgDn` description misprint.
  * [Issue 1334](https://code.google.com/p/conemu-maximus5/issues/detail?id=1334): ConEmu plugin was not realized the attach is finished (call from plugin menu).
  * Tab initialization fix for "far.exe /e ...".
  * `Ctrl+LClick` was failed in some cases (Far editor, Lua macro).
  * [Issue 338](https://code.google.com/p/conemu-maximus5/issues/detail?id=338): Fix attach external Far Manager console from plugin (not recommended though).
  * Update application manifests to Windows 10.
  * Attach list (`Win+G`) was failed sometimes with showing correct bitness of processes.
  * Show new console size in the status bar during resizing with mouse.
  * [Issue 1568](https://code.google.com/p/conemu-maximus5/issues/detail?id=1568): Try to hold cursor position (bottom visible line) in the bottom after resizing.
  * Restrict minimal GUI height to ({4,2} x {splits count}).
  * Macro: `ConEmu.CtrlO.*`: the Far default behavior (`CtrlO`) mapped to `CtrlAltO` because `CtrlShiftO` is used for splitting.
  * [Issue 1752](https://code.google.com/p/conemu-maximus5/issues/detail?id=1752): ConEmuTh: add ‘Turn off’ to the plugin menu. Macro option: `Plugin.Call(ConEmuTh,256)`.
  * `ConEmu.Editor.lua`: Fix auto switch to Editor Tab macro.
  * `ConEmu.ShiftEnter.lua`: Fix `ShiftEnter` (run command in new tab) macro.
  * Fix some EmergencyShow multi-process issues.
  * [Issue 1745](https://code.google.com/p/conemu-maximus5/issues/detail?id=1745): DPI scaling issues with font's combo boxes.
  * Many internal changes.


## Build 141004 ##
  * Auto switch hovered tabs while dragging files from/to Far panel.
  * Don't try to copy files from "C:\1\" to "C:\1\" (same src and dst folders).
  * [Issue 1740](https://code.google.com/p/conemu-maximus5/issues/detail?id=1740): D&D was not started from Far 1.75.
  * When creating default tasks, show WinSDK version in the tab label.
  * Autocreate VS (vcvarsall.bat) tasks for new configs.


## Build 141002 ##
  * Preparing for Windows 10.


## Build 141001 ##
  * ConEmuPlugin was rewritten using classes (Far versioning).
  * Don't activate Far Panels while Ctrl+LClick in Far Editor window.
  * [Issue 1717](https://code.google.com/p/conemu-maximus5/issues/detail?id=1717): Auto save/restore opened tabs renamed labels.
  * AnsiEscapeCodes: 'ESC [ 90..97 m' high intensity foreground color, 'ESC [ 100..107' high intensity background color.
  * [Issue 1715](https://code.google.com/p/conemu-maximus5/issues/detail?id=1715): 'New window'+'Run as administrator' was created non-elevated window.
  * Don't autoshow scrollbar on mouse hover while ConEmu is not in foreground.
  * Far plugin. F11 -> 'ConEmu' -> 'Show all panels list'. Press enter to 'print' selected path into current Far window.
```
Use macro (ConEmu.PanelsList.lua) to simplify menu call.
Plugin.Call("4b675d80-1d4a-4ea9-8436-fdc23f2fc14b",11)
```
  * [Issue 1590](https://code.google.com/p/conemu-maximus5/issues/detail?id=1590): (workaround) Parameter 'StartCreateDelay' may be changed via settings (xml/reg). DWORD 10..10000, default 100.
  * Debug. Some improvement in showing CP and console modes.
  * Debug. Verbose WM\_DEADCHAR warning.


## Build 140923 ##
  * Debug purpose switch: `ConEmu.exe -LoadRegistry`.
  * Support automatic find/loading of "`.ConEmu.xml`" (dot prefixed) config file.
  * Wrong dpi was used if starting on second monitor (9df4391 regression).


## Build 140922 ##
  * Internal: CSettings decomposition.
  * GuiMacro may be used for changing Checkbox/Radio ConEmu's options.
```
SetOption("Check",<ID>,<Value>)
  ID: numeric identifier of checkbox (ConEmu.rc, resource.h)
  Value: 0 - off, 1 - on, 2 - third state
Example, turn scrollbar on: ConEmuC -GuiMacro SetOption Check 2488 1
```
  * [Issue 1703](https://code.google.com/p/conemu-maximus5/issues/detail?id=1703): 'file.ext:line:' in hyperlink detector error.
  * File[:line:] hyperlink was not opened in Far editor sometimes (one more 'Desktop' drawback).
  * It was impossible to resize splitters in some configurations. For example, area 3 was not available for resize:
```
+-----+-----+-----+
¦  1  ¦     ¦  4  ¦
+-----+  3  +-----+
¦  2  ¦     ¦  5  ¦
+-----+-----+-----+
```
  * Minor xml formatting fix (new line after last MSZ value).
  * Fix false xml modified timestamps updates for Tasks and Palettes.
  * Option ‘KeyBar RClick’ was not working with "Far /w".
  * Wrong DPI was used if creating small window attached to the monitor edge.
  * github#19: Don't post Enter/Space KeyUp events to the console input buffer.


## Build 140914 ##
  * Wrong macro was sent to Far 3 (Lua) on hyperlink click if file was already opened.
  * Fix drop files to Far command line. Add menu items to press ‘Enter’ after goto/edit/view.
  * Fix wrong ‘sleep/speed’ written to the log.
  * Avoid numerous writing of equal lines to log (console size).
  * Internal changes.
  * [Issue 1704](https://code.google.com/p/conemu-maximus5/issues/detail?id=1704): GuiMacro. User may change option "FarGotoEditorPath" on the fly. Multiple Highlight editors are available now. Just call GuiMacro `SetOption` function with preferred editor command line. Few examples with ConEmuC
```
conemuc -guimacro setoption FarGotoEditorPath @"far.exe /e%1:%2 ""%3"""
conemuc -guimacro setoption FarGotoEditorPath @"#notepad ""%3"""
```
  * [Issue 1711](https://code.google.com/p/conemu-maximus5/issues/detail?id=1711): Cut start/end quotes from double-quoted commands.


## Build 140909 ##
  * Verbose tab activation error information.
  * Wrong hotkey was **displayed** on Tasks page.
  * Debug log was not disabled on Settings window closing.
  * ConEmu frame was not set to inactive state with ChildGui in the active tab.
  * Log current session state changes (`/log` switch, [Issue 1689](https://code.google.com/p/conemu-maximus5/issues/detail?id=1689) related).
  * Wrong dpi value may be used in some cases (dialog sizes, etc.)
  * If ChildGui was in focus, cross clicking was not closed all ConEmu tabs but only active ChildGui.
  * Return focus to ChildGui on ‘Create new console’ dialog cancelling and tab switching.
  * Do not do quake-flicker on Settings dialog opening.


## Build 140908 ##
  * [Issue 1690](https://code.google.com/p/conemu-maximus5/issues/detail?id=1690): Give more weight to "`/dir`" `ConEmu.exe` switch than the same from Task parameter.
  * [Issue 1703](https://code.google.com/p/conemu-maximus5/issues/detail?id=1703): Force CD refresh before starting hyperlink (editor).
  * Spare "`cmd.exe`" was appeared in the process root in some cases.


## Build 140905 ##
  * [Issue 1684](https://code.google.com/p/conemu-maximus5/issues/detail?id=1684): Long startup of documents from network drives.
  * [Issue 1703](https://code.google.com/p/conemu-maximus5/issues/detail?id=1703): Prepare FarPlugin to send both panel CD's to GUI.
  * When exporting settings suggest default for `ConEmu.xml` folder.
  * Fix indents when creating new xml file.
  * Do not update modified (timestamp) if xml was not really changed during config save.


## Build 140903 ##
  * Move Background image settings to separate page.
  * In some rare cases background was not drawn.
  * Background was not updated after some changes of ‘Placement’ field.
  * Add dummy (yet) chord hotkey controls to settings.
  * Explain LDrag/RDrag modifiers in hotkey list, show `[Disabled]` if it LDrag/RDrag is unchecked.
  * [Issue 1657](https://code.google.com/p/conemu-maximus5/issues/detail?id=1657): Treat Far space-visualizations as space symbols.
  * [Issue 1691](https://code.google.com/p/conemu-maximus5/issues/detail?id=1691): Make simple filenames Ctrl+LClick'able (open file in the editor).
  * Reveal ConEmuC switches in release (`/ErrorLevel` and `/Result`).
  * [Issue 1696](https://code.google.com/p/conemu-maximus5/issues/detail?id=1696): GuiMacro. `Attach([PID[,Alternative]])` allows attaching external console/ChildGui by PID.
  * Keyboard hooks were not disabled when ChildGui has focus and user switching to another app.
  * `[`Re`]`store tab working directories using option ‘Auto save/restore opened tabs’. Read more about ShellWorkDir.
  * [Issue 1643](https://code.google.com/p/conemu-maximus5/issues/detail?id=1643): gh#10: Try do not change ‘modified’ timestamp in xml if nothing was changed (Auto save window size and position on exit).


## Build 140819 ##
  * [Issue 1666](https://code.google.com/p/conemu-maximus5/issues/detail?id=1666): Fix black squares in RDP sessions (non true-color display modes actually).
  * [Issue 1688](https://code.google.com/p/conemu-maximus5/issues/detail?id=1688): Crash while updating Task bar jump list (6307574 regression).
  * Add ‘Remote session’ and ‘ScreenDC BITSPIXEL’ detection to Log.


## Build 140818 ##
  * New status bar columns are available: Zoom and DPI. Zoom is clickable.
  * Let statusbar columns Focus/Foreground show process information.
  * Ctrl+WheelClick reset font size to 100, same as GuiMacro Zoom(100).
  * Make all ConEmu's dialogs per-monitor dpi aware.
  * Fix console recreation and termination. If user cancel the UAC dialog during console [re](re.md)creation ConEmu fails to close the window by cross-click.
  * Implement ‘%CD%’ maintaining in the RCon.
    * Automatic detection of ‘%CD%’ in "cmd.exe" or "tcc.exe";
    * Ability to inform ConEmu GUI about shell ‘CD’ using ANSI (\e]9;9;"CD"\e\\) or "ConEmuC -StoreCWD [dir](dir.md)";
    * Display **CD** in the tab title using ‘%d’ tab template;
    * Reuse **CD** in the Restart and Create new console dialogs;
    * At last you may use "**%CD%**" variable with Shell macro function in the `Dir` parameter. Example, start new 'cmd' session:
```
Shell("", "cmd", "", "%CD%")
```
  * [Issue 1680](https://code.google.com/p/conemu-maximus5/issues/detail?id=1680): Use ‘%CD%’ within hyperlink detector, warn if file not found.


## Build 140815 ##
  * [Issue 1660](https://code.google.com/p/conemu-maximus5/issues/detail?id=1660): Support `"/NoSingle"`, `"/Quake"`, `"/NoQuake"` switches in Task parameters. They used while starting new Task from existing ConEmu instance.
  * Quake window can't be hidden by taskbar button click.
  * Some internal changes.


## Build 140814 ##
  * [Issue 1672](https://code.google.com/p/conemu-maximus5/issues/detail?id=1672): Attach to a console window clears its window title.
  * DefTerm. Use switch `"/ROOTEXE"` internally to let ConEmuC know the name of root executable.
  * More tab switch fixed for Far Manager 4040 (‘Desktop’ is now has ‘0’ index).
  * New status column for debugging purposes - ‘Keyboard hooks’.
  * Advanced logging of monitor dpi.
  * Wrong values was written to log in ‘Loaded pos’.


## Build 140812 ##
  * [Issue 1669](https://code.google.com/p/conemu-maximus5/issues/detail?id=1669): Internal tab switching was broken from Far 4040.
  * [Issue 1670](https://code.google.com/p/conemu-maximus5/issues/detail?id=1670): Powershell + PSReadLine makes progress get stuck.


## Build 140811 ##
  * [Issue 1667](https://code.google.com/p/conemu-maximus5/issues/detail?id=1667): Drag&Drop was broken from Far 4040.
  * [Issue 1666](https://code.google.com/p/conemu-maximus5/issues/detail?id=1666): Windows XP: Black squares on the toolbar (3fb706a regression)
  * [Issue 1334](https://code.google.com/p/conemu-maximus5/issues/detail?id=1334): New buffer scrolling hotkeys.
    * by half-screen - Apps+PgUp/PgDn
    * to the top/bottom - Apps+Home/End
    * to the cursor row - Apps+Backspace
  * GuiMacro: `Scroll(<Type>,<Direction>,<Count=1>)`
```
Do buffer scrolling actions
   Type: 0; Value: ‘-1’=Up, ‘+1’=Down
   Type: 1; Value: ‘-1’=PgUp, ‘+1’=PgDown
   Type: 2; Value: ‘-1’=HalfPgUp, ‘+1’=HalfPgDown
   Type: 3; Value: ‘-1’=Top, ‘+1’=Bottom
   Type: 4; No arguments; Go to cursor line
```


## Build 140810 ##
  * With courtesy of Certum, ConEmu's binaries now are signed as ‘Open Source Developer, ConEmu-Maximus5’.
  * Per-monitor dpi awareness implemented (Windows 8.1). The checkbox ‘Admit monitor dpi with font size’ is turned on by default. It will affect the height of Tabs, Status and Main console fonts.
  * Ability to ‘Treat font height as device units’ implemented. The checkbox is turned OFF by default for existing configs, but it is turned ON by default for new configs and after config reset.
  * Releases builded via VisualStudio projects was not dpi-aware.
  * [Issue 1653](https://code.google.com/p/conemu-maximus5/issues/detail?id=1653): In some rare cases ConEmu.exe was locking shell working directory.
  * New ConEmu.exe switches: -quake, -quakeauto (autohide on focus lose) and -noquake.
  * Checkbox ‘New window’ was ignored if dialog was started from tasks menu with Shift+Click.
  * Ensure that console started in the ‘New window’ will not be Quake-moded.
  * Ctrl+Wheel changes the zoom value but not a font height from the ‘Main’ settings page.


## Build 140723 ##
  * [Issue 1608](https://code.google.com/p/conemu-maximus5/issues/detail?id=1608): DefTerm. /config parameter registered for startup was cleared after OS restart.
  * DefTerm. Started debugging session of Win32 app from VS was ignored DefTerm options.
  * DefTerm. Support flickerless start of Win32 console app debugging session (same bitness as devenv).
  * Make Win+S hotkey for ‘Duplicate root’ by default.
  * Ensure cascaded window will not pass outside of monitor working area.
  * Allow ‘Duplicate root’ even if injects were disabled.
  * Do not leave ‘hunging’ tab if duplicating was failed.
  * Tab title was not changed sometimes during command execution from Far panels.
  * Due to many reports crashdump message was changed (aka MicrosoftBugs, ConEmuHk).
  * Immediate reaction on "`ConEmuFakeDT`" changing in batch files (7a8e61f regression).
  * [Issue 1379](https://code.google.com/p/conemu-maximus5/issues/detail?id=1379): Use ‘Appearance -> Animation’ in ‘normal’ mode too.
  * Many internal changes.


## Build 140707 ##
  * Show SystemMenu button on the toolbar (RClick opens Settings dialog).
  * [Issue 1125](https://code.google.com/p/conemu-maximus5/issues/detail?id=1125): ‘Run as’ was hooked in DefTerm if ConEmu window is already opened only.
  * [Issue 1628](https://code.google.com/p/conemu-maximus5/issues/detail?id=1628): Detach/Re-Attach was failed.
  * Option to disable console detach confirmation.
  * Fix `[App Path]` search sequence (`far` was not started from ConEmu folder).
  * Error was occured while starting admin console (32779f4 regression).
  * Internal changes.


## Build 140703 ##
  * Support tabs dragging (reordering).
  * Highlight&Goto: [Issue 1624](https://code.google.com/p/conemu-maximus5/issues/detail?id=1624): Don't warn after starting external editor successfully.
  * Highlight&Goto: External editor may fails if located in system32 while ConEmu is 32bit on 64bit OS.
  * Highlight&Goto: Start editor after `LButtonUp`.
  * Highlight&Goto: Ensure started external editor get focus.
  * New settings page ‘Confirm’, some confirmation options moved here.
  * New setting ‘Confirm tab duplicating (Duplicate root menu item)’ on ‘Confirm’ page.


## Build 140701 ##
  * Wrong deinitialization ConEmuHk sequence may cause crash on application exit.
  * HTML format copy: Eliminate line spacing and try to match font height.
  * Stack overflow may occurs sometimes on ConEmu dialog popup.
  * New switch: `ConEmuC /IsAdmin` - returns 1 as errorlevel if current user has elevated privileges, 2 if not.
  * [Issue 1626](https://code.google.com/p/conemu-maximus5/issues/detail?id=1626): Fix focus restore with some ChildGui on ConEmu window activation.
  * [Issue 1617](https://code.google.com/p/conemu-maximus5/issues/detail?id=1617): Fix performance drawback on GetSystemTime.


## Build 140628 ##
  * Fix attaching (`Win+G` from ConEmu) of `*.vshost.exe` console.
  * GuiMacro. `PasteFile` able to skip lines started with `"<CommentMark>"` prefix.
```
PasteFile(<Cmd>[,"<File>"[,"<CommentMark>"]])
```
  * [Issue 1620](https://code.google.com/p/conemu-maximus5/issues/detail?id=1620): Support PuTTY's duplicate session in ‘Duplicate root’ function.
  * `GitShowBranch.cmd`: drop remote name from branch name (drop tail after first ‘.’).
  * When reopening About dialog, show last selected section.
  * Search in about dialog was on first page with found text.
  * Update confirmation dialogs was not visible if ConEmu was ‘Always on top’.
  * Do not hook `ssh-agent.exe`, console-outside process was blocking updating of `ConEmuHk.dll`.
  * Move process creation to separate thread (`ConEmu.exe`).
  * Added ‘Tomorrow’ color schemes (by Michael Mims).
  * Support ‘Script.ps1:35 char:23’ error format. However, column is ignored as yet.
  * Status ‘Console server started...’ sometimes was not cleared on startup.
  * DefTerm. Feature was rewritten almost from scratch.
    * For accessibility reasons DefTerm settings are mirrored in registry `[HKCU\Software\ConEmu]`. That means, for example, user may change ‘DefTerm-Enabled’ value to 0 to disable feature any time, even if ConEmu is not running at the moment.
    * Save to registry only from Settings window (DefTerm page clicks) or ConEmu startup (if DefTerm is enabled).
    * Update status bar during DefTerm installation.
    * It is possible to hook processes by their window class name.
    * New ‘Agressive’ mode of DefTerm hooking. Sample usecase:
      * The `explorer.exe` was hooked (for example with ‘Register on OS startup’ option), when you start new `devenv.exe`, started process will be hooked from `explorer.exe`, even if you have not have ConEmu loaded at the moment of `devenv.exe` start.
    * Allow windowless ‘Register on OS startup’.
    * Server startup fails sometimes if console window was hidden.
    * Seamless startup from VC# debugger, no RealConsole flickering.
    * Many internal changes.


## Build 140615 ##
  * GuiMacro. `Keys("Enter")`, `Keys("Tab")`, `Keys("Backspace")` was not working.
  * GuiMacro. Support alternative notation, e.g. `Keys("{Enter}")`.
  * [Issue 1125](https://code.google.com/p/conemu-maximus5/issues/detail?id=1125): cmd.exe in administrator mode was not autoattached to ConEmu (Ctrl+Shift+Enter from start menu)
  * Force working directory to `%UserProfile%` if `ConEmu.exe` was started from `%WinDir%`, `%WinDir%\system32` or `%ConEmuDir%`. If you really need to use these working directories - force them with ‘/dir’ switch:
```
ConEmu.exe /dir C:\Windows\System32 /cmd PowerShell.exe
```
  * Do not run spare `cmd.exe` when running `""cmd""` (`cmd` from Windows start menu).
  * [Issue 1598](https://code.google.com/p/conemu-maximus5/issues/detail?id=1598): Let `Ctrl+Shift+[F|D]` start from current console startup directory (current directory in future).
  * [Issue 1564](https://code.google.com/p/conemu-maximus5/issues/detail?id=1564): Add tasks to history too.
  * [Issue 1615](https://code.google.com/p/conemu-maximus5/issues/detail?id=1615): Op ‘New console’ fails when used startup task.


## Build 140612 ##
  * VC14 solution files.
  * GuiMacro. Two new options in `Copy(<What>[,<Format>[,"<File>"]])` function.
```
What==2: copy visible area contents.
File: if specified, save to file instead of clipboard.
```
  * GuiMacro: New function `PasteFile(<Cmd>[,"<File>"])`, for pasting `<File>` contents.
```
Paste <File> contents, omit <File> to show selection dialog
Cmd==0: paste all lines
Cmd==1: paste first line
Cmd==2: paste all lines, without confirmations
Cmd==3: paste first line, without confirmations
Cmd==9: paste all lines space-separated
Cmd==10: paste all lines space-separated, without confirmations
```
  * [Issue 1611](https://code.google.com/p/conemu-maximus5/issues/detail?id=1611): GuiMacro. Optional `<IsRelative>` argument in `SetOption` function. Set `IsRelative=1` to use relative instead of absolute for some options, for example, `SetOption("bgImageDarker",-10,1)` to make background darker.
  * Add search ability to the About dialog.
    * Do not autoselect text in the About dialog (on Tab press).
  * New design of search control in the Settings dialog.
    * Add ability to search within hotkeys in the Settings dialog.
    * Display found-hint in place of listbox/listview item.
    * Add ‘(Quake-style hotkey also)’ to ‘Minimize/Restore’ hotkey description.
  * DefTerm. Unchecked option ‘Use existing ConEmu window if available’ was ignored in C# VS debugger.
  * DefTerm. Fix external console attach (Win+G) from ConEmu window.
  * DefTerm. Hook ‘Command prompt (Administrator)’ too.
  * Admin sign was not shown after auto-attach of external console.
  * Show version state in the About dialog (stable/preview/alpha/obsolete).
  * Do not show ‘Starting attach autorun’ for default terminal consoles (cmd as Admin).
  * Speed up xml load/save.


## Build 140602 ##
  * [Issue1598](https://code.google.com/p/conemu-maximus5/issues/detail?id=1598): When reopening Settings dialog, show last selected section.
  * Allow prefix '^' in Far macros (through ConEmu) do not send keys to plugins (KMFLAGS\_NOSENDKEYSTOPLUGINS).
  * Opening compiler error (hyperlink) in Far editor fails if active plugin panel was processing ShiftF4 itself.
  * Some new features in ‘Ctrl+LClick’ on compiler error (Highlight and goto)
    * Use dropdown for external editor (Ctrl+LClick on compiler errors).
    * New macro ‘%4’ and ‘%5’ for ‘slashed’ paths, so the full macro list is:
```
‘%1’ - line number, ‘%2’ - column number, ‘%3’ - C:\\Path\\File, ‘%4’ - C:/Path/File, ‘%5’ - /C/Path/File
```
    * New predefined (available in list) editor command lines for Vim, SciTE, Notepad++ and Sublime.
    * Prefix ‘#’ means ‘run editor outside of ConEmu tab’.
    * Check registry for ‘App Paths’ when no path was specified for editor executable.
    * Force run specified external editor instead of open file in the existing Far instance.
  * GitHub#13: Was unable to save 'Highlight and goto' modifier.
  * [Issue1596](https://code.google.com/p/conemu-maximus5/issues/detail?id=1596): Rust-compiled apps were crashed in ConEmu.
  * Use ‘PATH’ defined in the ‘App Paths’ registry key if running application without full path.
  * Internal. Do not turn on session.SetSessionNotification on startup.


## Build 140529 ##
  * Hide tab error tooltip after receiving tabs from Far plugin.
  * Active tab was not changed if Far window was switched from ConEmu plugin (F12 macro).


## Build 140528 ##
  * [Issue 1588](https://code.google.com/p/conemu-maximus5/issues/detail?id=1588): Crash when setting ‘Settings > Main > Cell = -1’.
  * Show Far editors/viewers tabs from all panes, but not active VCon only.
  * [Issue 1594](https://code.google.com/p/conemu-maximus5/issues/detail?id=1594): Do not expect space after ‘file.ext:line:’ in hyperlink detector.
  * Update Far panel tab title immediately after pressing Ctrl-F10 in editor.
  * Do not add trailing space while pasting with RBtnClick.


## Build 140523 ##
  * Do not push ‘background’ tabs on top of recent stack during task creation.
  * Do not block tab activation (VCon actually) if VCon has the only tab or that tab already active.
  * [Issue 1582](https://code.google.com/p/conemu-maximus5/issues/detail?id=1582): Ctrl+Tab was broken if Tab bar is not visible (NewTabs regression).
  * NewTabs: Fix empty tab title in some cases (VConMenu, etc.)
  * Fix wrong behavior of right clicks on tool bar buttons


## Build 140522 ##
  * Some fixes related to new tabs internals.
  * [Issue 1583](https://code.google.com/p/conemu-maximus5/issues/detail?id=1583): Fix `ConEmu.Editor.lua` bug.
  * Wrong startup directory used when starting at the drive root.
  * Allow rename any Far tab, not only panels.
  * Two new switches (commands actually) in ConEmuC: "-e" and "-t". Read more in wiki: http://code.google.com/p/conemu-maximus5/wiki/ConEmuC#Echo_and_Type
```
ConEmuC -e [-n] [-r] [-b] "string" for printing some string
ConEmuC -t [-r] [-b] "file" for printing some text file
```
  * Force "CHERE\_INVOKING" in default tasks for cygwin.


## Build 140519 ##
  * **Experimental build**
  * Refactoring of Server startup.
  * Refactoring of Tabs-related internal code.
  * Typo: overrided -> overriden.
  * Debug. Switches in ConEmuC: `/Args`, `/CheckUnicode`, `/ErrorLevel`, `/Result`.
  * New environment variable `ConEmuServerPID` set during server init.
  * [Issue 1505](https://code.google.com/p/conemu-maximus5/issues/detail?id=1505): Fix attach of several ChildGui at the same time.
  * [Issue 1214](https://code.google.com/p/conemu-maximus5/issues/detail?id=1214): Support overtab clicks for activate/close/menu
  * Fix panel tab title blinking during edit/view opening.
  * [Issue 436](https://code.google.com/p/conemu-maximus5/issues/detail?id=436): Do not lose recent tab (editor/viewer) history after cmd execution from Far.


## Build 140505 ##
  * Limited logging of console contents.
    * Option ‘[Inject ConEmuHk](ConEmuHk.md)’ must be enabled to get that feature!
    * Only ‘Standard output’ will be logged (the same activity as processed with ‘ANSI X3.64’ processor). So, certain Windows console API functions will not be logged (Far Manager panels, for example).
    * You may choose folder for storing log-files, default is `"%ConEmuDir%\Logs\"`.
    * **Note!** Default folder may be write-protected, if you've installed ConEmu to `"Program Files"`. So, you may need to choose write-enabled folder.
    * Each console will share its own log-file, example: `"ConEmu-YYYY-MM-DD-pPID.log"`.
    * So, if several processes will write to console output simultaneously, you may get unpredictable result in your log-file.
    * By default, logging is **disabled**.
  * ANSI. Fix ‘no-color’ on new line.
  * ANSI. Fix ‘bad color cell’ on EOL.
  * ANSI. Do not break `"\r\n"` in two physical writes.


## Build 140429 ##
  * [Issue 1557](https://code.google.com/p/conemu-maximus5/issues/detail?id=1557): Switch `-new_console:u:"other_user:password"` may lock the account of other\_user (another way). Also, bad behavior of `-cur_console` reverted (15cff21 regression)
  * Some internal changes.


## Build 140428 ##
  * Shield was not set on ‘Restart’ button in ‘Recreate’ dialog.
  * github`#`5: Console was not started as admin when ‘Create new console’ dialog was skipped.
  * Don't hide row/col highlight on focus loose.
  * [Issue 1557](https://code.google.com/p/conemu-maximus5/issues/detail?id=1557): Switch `-new_console:u:"other_user:password"` may lock the account of other\_user. In some cases starting new console without directly specified working directory (with `-cur_console:d:...` for example) may lock other\_user account with reason ‘Bad password count exceeded’.
  * [Issue 1516](https://code.google.com/p/conemu-maximus5/issues/detail?id=1516): ConEmu fails to duplicate root when cmd.exe was catched as ‘Default terminal’.


## Build 140422 ##
  * GuiMacro. `Sleep(Milliseconds)`. Because it is processed inside GUI process, Milliseconds are limited to 10000 (10 sec) max.
  * GuiMacro. `Break()` for `Ctrl+C` and `Break(1)` for `Ctrl+Break`. However, these function may fails in some cases (in RealConsole for example). Also, they are processed by Windows asynchronously, so you may need to `Sleep(ms)` after them if your want to `print("something")` to console after.
  * Clipboard use optimization - RClick Paste may fails sometimes.
  * Fix hyperlinks underlining errors (c31f3cb regression).


## Build 140416 ##
  * Don't use hooks inside shell started from mintty (6b514d0 regression).
  * [Issue 1553](https://code.google.com/p/conemu-maximus5/issues/detail?id=1553): Mouse button actions modifier was not saved (vkCTSVkAct).
  * [Issue 1556](https://code.google.com/p/conemu-maximus5/issues/detail?id=1556): Ctrl+L not working in cygwin.


## Build 140414 ##
  * [Issue 1544](https://code.google.com/p/conemu-maximus5/issues/detail?id=1544): Cygwin ssh forking fails.


## Build 140412 ##
  * [Issue 1532](https://code.google.com/p/conemu-maximus5/issues/detail?id=1532): Show only critical status messages in the top-left terminal corner.
  * [Issue 1534](https://code.google.com/p/conemu-maximus5/issues/detail?id=1534): Use About() and Settings() GuiMacro for system hotkeys (Win+Alt+A, Win+Alt+P, Win+Alt+K). System hotkeys are fixed by design. But user can use GuiMacro to open About or Settings dialogs. Moreover, if you need to open dialogs with special page activated (if you need some page often).
  * Add [ConEmuEnvironment#Export\_variables](ConEmuEnvironment#Export_variables.md) link to About dialog (Console tab).
  * [Issue 1546](https://code.google.com/p/conemu-maximus5/issues/detail?id=1546): Mark/copy with mouse appears very slow in some cases.


## Build 140404 ##
  * Show 1-based console coordinates in the status bar (was 0-based).
  * Don't show size of the console visible rect by default in the status bar.
  * Win8. Failed to set console width less than 19 after Ctrl+Shift+O (in progress).


## Build 140403 ##
  * [Issue 1527](https://code.google.com/p/conemu-maximus5/issues/detail?id=1527): Regression 140327. PowerShell was not hooked.
  * [Issue 1528](https://code.google.com/p/conemu-maximus5/issues/detail?id=1528): Infinite mouse messages `ChildGui<-->ConEmu` in some cases.
  * Settings Main page redesigned.
  * Change clink homepage url, update `ConEmu\clink\Readme.txt`.
  * Remove possible deadlock after closing AltServer.
  * Some internal changes.


## Build 140327 ##
  * Console. Increase max buffer height to 32766 lines.
  * [Issue 1504](https://code.google.com/p/conemu-maximus5/issues/detail?id=1504): Parenthesis issue in `csudo.cmd`.
  * Ensure `Cmd_Autorun.cmd` always returns errorlevel==0.
  * Quotation marks changes in `Cmd_Autorun.cmd` (possible parenthesis issues?)
  * Use LdrDllNotification (Win8+). Will set up hooks before DllMain called (LoadLibrary).
  * Ctrl+O Far macro was failed in some cases - `Shell("new_console:b")`.
  * [Issue 1509](https://code.google.com/p/conemu-maximus5/issues/detail?id=1509): Arg parsing failed for ‘`/dir "C:\" /single`’ (01ed281 regression).
  * Allow string arguments of `-new_console` to be resetted. For example, you are running task with "/icon nice.ico" in Task parameters. You may reset it in task commands if you are running several tabs at once:
```
cmd -new_console:C:
powershell -new_console:C:twice.ico
```
  * Hook mouse\_event and SendInput API.
  * Allow "/icon" task parameter to change tab icon too.
  * Restrict "/icon" task parameter to change already started instance window icon.
  * New ConEmu.exe switch ‘`/FontDir "YourDir"`’ allows to register fonts from several specified folders
  * [Issue 1258](https://code.google.com/p/conemu-maximus5/issues/detail?id=1258): `Ctrl+BackSpace` delete words to the left including spaces.
> > Actually, not only spaces are taken to account: ‘`>])}$.,/\"`’.
  * About dialog `-new_console` fixes.
  * No need to unset hooks if our process is to be force killed by TerminateProcess (minor speed up).
  * [Issue 1512](https://code.google.com/p/conemu-maximus5/issues/detail?id=1512): Warn on invalid prefix-switch in task command. Valid switches currently are
```
/bufferheight <lines>
/dir <workingdir>
/icon <tabicon>
/tab <tabtitle>
```
  * Warn on invalid switches in task parameters. Valid switches currently are
```
/dir <WorkingDir>
/icon <ConEmuOrTabIcon>
/single
```
  * RClick on Close button - minimize to TSA again (02bf905 regression).
  * Don't hook LoadLibraryW if possible. Till now LoadLibrary will be hooked in:
    * Windows2k, WindowsXP - all processes
    * Far Manager (far.exe, far32.exe, far64.exe)
  * Vim xterm arrow key lags happen sometimes. When Vim was switched to xterm mode, ConEmu sends corresponding xterm ESC-sequences instead of simple VK\_xxx keypresses. Sometimes Vim don't catch whole sequence before any other event occures. For example, you press Right arrow and Vim does nothing before you just move mouse over console.
  * [Issue 1511](https://code.google.com/p/conemu-maximus5/issues/detail?id=1511): Url was not detected after ‘port’: "http://example:8080/TestTest/test?test=test".
  * [Issue 1516](https://code.google.com/p/conemu-maximus5/issues/detail?id=1516): Duplicate root failed for consoles started from TaskMgr via Default terminal feature.
  * Consider console is to be started hidden if start X/Y==32767 (Hooks).
  * Tooltip msg fix.
  * Quake settings. Name ‘Cascade’ as ‘Centered’, enable ‘Apply’ on change.
  * New option ‘Retard inactive panes’ on Features page. There is already ‘Sleep in background’ option which retard ConEmu window and its tabs when ConEmu is minimized/inactive. New option ‘Retard inactive panes’, disabled by default, retard inactive but visible split-panes in the active ConEmu window.
  * [Issue 1518](https://code.google.com/p/conemu-maximus5/issues/detail?id=1518): Remove delay before being able interact with quake style console after slide-down.
  * Wheel was not working while ConEmu's internal text selecting in Far.
  * [Issue 823](https://code.google.com/p/conemu-maximus5/issues/detail?id=823): Out of screen (upper border) selected text was not copied to clipboard.
  * Fix wrong hotkey for menu item ‘Close except active’.
  * [Issue 1100](https://code.google.com/p/conemu-maximus5/issues/detail?id=1100): Don't show our scrollbar while GuiChild is visible.
  * Allow RealConsole scrolling while GuiChild is hidden.
  * Fix position of ChildGui (cut more pixels of the child frame).
  * Fix position of ChildGui (chrome.exe, firefox.exe).
  * ChildGui. Some applications can ‘disable’ Maximize button but they are still ‘resizeable’.
  * [Issue 1282](https://code.google.com/p/conemu-maximus5/issues/detail?id=1282): PortableApps. Seamless run of paf-applications, console like tcc.exe or GUI like KiTTY.
  * Tab icon was not initialized properly if executable file was not in %PATH%.
  * Many internal changes.


## Build 140310 ##
  * [Issue 1498](https://code.google.com/p/conemu-maximus5/issues/detail?id=1498): Prettify update confirmation dialogs.
  * Indeterminated taskbar status during update confirmation.
  * ConEmu's debugger was failed to start on elevated consoles.
  * Internet errors was not displayed sometimes during update.
  * Workaround for ssh crash when third-party thread terminated before ssh init finished.
  * [Issue 1502](https://code.google.com/p/conemu-maximus5/issues/detail?id=1502): GuiMacro new window transparency functions.
```
SetOption AlphaValue <40..255>
    same as Transparency 0 <40..255>
SetOption AlphaValueInactive <0..255>
    same as Transparency 2 <0..255>
SetOption AlphaValueSeparate <0..1>
    same as Transparency 4 <0..1>
Also, new mode in Transparency for inactive value relative change
    Transparency 3 <-255..+255>
```
  * Show actual (active/inactive) transparency value in status bar.
  * [Issue 1042](https://code.google.com/p/conemu-maximus5/issues/detail?id=1042): Quake, return focus to window which was active before showing ConEmu.


## Build 140304 ##
  * [Issue 1490](https://code.google.com/p/conemu-maximus5/issues/detail?id=1490): Fix crash on ANSI scrolling sequences.
  * [Issue 1491](https://code.google.com/p/conemu-maximus5/issues/detail?id=1491): Don't erase customized directories when updating ConEmu PortableApps.
  * Overflow of internally registered windows messages.
  * AnsiDbg. Send predefined `*.ans` files to srv by Ctrl+1/2/3.
  * Overwrite file confirmation in SaveAs dialog
  * ‘Import...’ button in the Settings dialog.
    * **Note!** Export AND Import settings (xml files) now works with unnamed (.Vanilla) configuration to unify settings exchange.


## Build 140302 ##
  * Regression 131211. ([Issue 158](https://code.google.com/p/conemu-maximus5/issues/detail?id=158)) ConEmu PanelView was not updated after panel change.
  * Internal. AnsiDebugger added to src folder.
  * Regression 140227. "`-GuiMacro:0`" was executed in the first tab but not an active one.
  * GUI child application fails to start inside ConEmu if ‘tab’ startup command begins with "set", "chcp", "title" (processed by ConEmuC before starting shell). Than, following GUI executable was started outside of ConEmu window.
  * More logging of Mouse Wheel events
  * Wheel over inactive mintty pane unexpectedly scrolls active pane below too.


## Build 140227 ##
  * Don't unlock working directory until ConEmu initialization finished. So, behavior of "-loadcfgfile" and "-savecfgfile" changed. If you specify xml file without path, if will be searched in the working (`ConEmu.exe` startup) directory, but not in the `ConEmu.exe` folder.
  * In some cases Tab labels was not updated with console titles.
  * Switch "`-NoCloseConfirm`" described in the About dialog.
  * Don't highlight row/col during any popup menus (system, tab, etc.)
  * [Issue 1444](https://code.google.com/p/conemu-maximus5/issues/detail?id=1444): Put focus to left/right/top/bottom split was not worked as expected.
  * Result of "`ConEmuC.exe -GuiMacro:0`" was not printed to StdOut.
  * Don't try to export "`ConEmuMacroResult`" when "`ConEmuC.exe -GuiMacro:0`" was called.
  * Fix "`ConEmuC.exe -GuiMacro:PID|HWND`" to exec macro in the exact instance or tab/split.
  * Colorization test in "`ConEmuC.exe /CHECKUNICODE`".
  * Wrong startup directory was displayed in console if startup command fails.
  * Some internal changes.


## Build 140225 ##
  * Log Lock/Unlock/Logoff events.
  * GuiMacro. `Keys("Combo1"[,"Combo2"[,...]])`. Combo syntax is mostly similar to AutoHotKey syntax: `[Mod1[Mod2[Mod3]]]Key`. Where ‘Mod’ may be: `^ - LCtrl, >^ - RCtrl, ! - LAlt, >! - RAlt, + - Shift`. For example, if you want to ‘`Ctrl+/`’ post ‘`Ctrl+_`’ - use macro: `Keys("^_")`.
  * Support "`\b`", "`\xFF`" and "`\xFFFF`" in GuiMacro strings. Hexadecimal parser stops on first NON hexadecimal character. So, avoid ambiguous strings like "`\x05five`", this will be parsed as two strings "`\x05f`" and "`ive`"
  * Support more that one string argument in Paste and Print GuiMacro functions. All strings will be concatenated before pasting into console.
```
Syntax:
Paste (<Cmd>[,"<Text>"[,"<Text2>"[...]]])
Print("<Text>"[,"<Text2>"[...]])
Example:
print("123","\x1F","abc","def")
```
  * Command line switch "`ConEmu.exe /NoCloseConfirm`" to disable confirmation of ConEmu's window closing.
  * [Issue 1488](https://code.google.com/p/conemu-maximus5/issues/detail?id=1488): Ctrl+Shift+O fails in ‘`Cmd_Autorun`’ consoles.


## Build 140223 ##
  * Hook abnormal termination of child processes (cygwin, mintty, vim, etc.)
  * [Issue 713](https://code.google.com/p/conemu-maximus5/issues/detail?id=713): Far. After ConsoleDetach, 1st console does not work correctly.


## Build 140220 ##
  * [Issue 1477](https://code.google.com/p/conemu-maximus5/issues/detail?id=1477): Fix Far panels detection (No column titles and No sort letter).
  * Many changes within "-new\_console" and "-cur\_console" switches processor.
    * Allow multiple quoted switches in one -new\_console:d:"C:\My folder":t:"My title"
    * Simple switches may be separated too: -new\_console:c:b:a
    * [Issue 1354](https://code.google.com/p/conemu-maximus5/issues/detail?id=1354): Switches will not be processed AFTER ConEmu's executables, read more in [wiki](NewConsole#Exclusions.md)
    * Internal refactoring (allow ‘concatenate’ or ‘apply’ them correctly) and unit tests.
  * [Issue 1478](https://code.google.com/p/conemu-maximus5/issues/detail?id=1478): Allow ChildGui activation with LClick.
  * Some internal changes.


## Build 140216 ##
  * Fix crash after resizing ConEmu window in some cases.


## Build 140214 ##
  * RClick on Max caption button - Fullscreen switch.
  * Restore default split mouse hover activation timeout to 500ms.
  * Pressing Alt+F4 enexpectedly close all ConEmu's tabs instead of active (PuTTY) only
  * Support "title" directive in Tasks and cmdline. **Note**! "title" is useless with most of shells like cmd, powershell or Far! You need to change title within your shell!
```
cmd /k title Your title
powershell -noexit -command "$host.UI.RawUI.WindowTitle='Your title'"
```
  * Add ‘Tasks’ page to the ‘About’ dialog.
  * Fix caption icon in the close confirmation TaskDialogIndirect.
  * Force show icon in the TSA if taskbar icon is hidden (‘Desktop’ and ‘Quake’ modes).
  * Fix ‘Intelligent selection’ conditions
    * Don't use in Far Manager viewers
    * Use in ‘Terminated’ console state
  * Optional ‘Use existing ConEmu window’ with Default terminal feature.
    * New checkbox on the ‘Default term’ Settings page
    * Optional switch "-new\_console:N"
  * Far. Sometimes scrolling (buffer) was not removed after command finished.
  * Optimize loading Far plugin (ConEmu.dll). Occasionally assertion box was appeared.
```
---------------------------
ConEmuC: CheckResources started
---------------------------
ConEmu ?????? [??].??: CreateFile(\\.\pipe\ConEmuPlugin????) failed, code=0x00000002, Timeout
```
  * Some internal changes.


## Build 140205 ##
  * Was not working: `"set ConEmuFakeDT=2013-11-30"`.
  * New status line cursor info format: `‘(col,row) height [V|H]’`.
  * Show one info col instead of three cursor columns (by default).
  * [Issue 1431](https://code.google.com/p/conemu-maximus5/issues/detail?id=1431): Auto disable ‘Desktop’ mode when enabling ‘Quake’ mode.
  * Long console output may fails in Windows 8 (Far Manager).
  * Use static link of user32 in ConEmuHk.
  * After `"cmd -new_console"` from `"far /w"` prompt, buffer was disabled unexpectedly.
  * [Issue 1464](https://code.google.com/p/conemu-maximus5/issues/detail?id=1464): Takes care of system ‘Active window tracking’ on 3rd-state of ‘Activate split on mouse over’.
  * [Issue 1165](https://code.google.com/p/conemu-maximus5/issues/detail?id=1165): Scrolling console buffer with mouse wheel in Far only works when mouse is over scroll bar.


## Build 140203 ##
  * Vim fails to react for window size change in the xterm terminal mode.


## Build 140202 ##
  * New ‘Intelligent selection’ mode.
    * Now, you may start both block or text selection with mouse without modifier pressed.
    * Type of selection will be determined by the direction of LButton mouse drag, when you start drag vertically - block selection, horizontally - text selection.
    * If you set up and press selection modifier - mouse selection will start even if ‘Intelligent selection’ is disabled with exceptions.
    * New mode may be disabled ‘totally’ or for specified processes only. For example, Vim knows about text selection and can process mouse internally, and Far Manager can use mouse for dragging files or text selection in Editors.
    * When "far" specified in the exceptions without extension, you can use new mode in the ‘User screen’ (panels are off) and in Viewers.
  * So, ‘Mark/Copy’ settings page redesigned
  * GuiMacro. Paste(9), Paste(10) - One-line paste with or without confirmation.
  * ‘Change prompt position with LClick’ works on left button release.
  * Some internal changes.


## Build 140128 ##
  * New pasting mode ‘One line’ (useful with "git status" & "git add"). When ‘Right mouse button action’ is set to ‘Auto’ you may press it during selection (when Left mouse button is still pressed) to paste all copies lines as a ONE space-delimited line.
  * [Issue 1416](https://code.google.com/p/conemu-maximus5/issues/detail?id=1416): ReAttach after Detach fails for non-console applications
  * One more icon fix in MessageBox dialogs


## Build 140127 ##
  * `AnsiColors24bit.ps1` - example how to print with 24bit color with PowerShell script.
  * Support cppcheck erors in hyperlink detector.
  * [Issue 992](https://code.google.com/p/conemu-maximus5/issues/detail?id=992): Support `‘file:///c:\...’` hyperlinks.
  * [Issue 1053](https://code.google.com/p/conemu-maximus5/issues/detail?id=1053): Disable hyperlinks detector on Far panels.
  * Settings. Mouse button action fails to be changed.
  * Allow ‘-’style switches in task params
  * Hints in ‘Settings\Tasks’ and ‘Settings\Integration’.
  * Small redesign of ‘Settings\Tasks’.
  * Fix icon in some dialogs captions.
  * Don't update xml if `<CmdLineHistory>` was not changed actually.
  * Some internal changes.


## Build 140124 ##
  * Checkbox 'New window' in create dialog was ignored in 'Single instance mode'.
  * Wrong error message was displayed if empty passwords are disabled in system policy.
  * Use 'Fade when inactive' option for inactive splits.
  * Ansi xterm 24-bit fails for RED=0..15.


## Build 140123 ##
  * Don't turn on ‘Quick edit mode’ in real console by default.
  * Underline hyperlinks internally (don't use fontmapper).
  * Some internal changes.


## Build 140122 ##
  * `SetOption(AlwaysOnTop)` described in the About dialog.
  * [Issue 1336](https://code.google.com/p/conemu-maximus5/issues/detail?id=1336): Vim, xterm256 drawing issues (T,S,K,J,L,M CSI commands).
  * Don't try to export environment to ServerPID==0.
  * [Issue 823](https://code.google.com/p/conemu-maximus5/issues/detail?id=823): Autoscroll console while select text with mouse.
  * Some fixes in `GitShowBranch.cmd`.
  * Console progress detector stucks sometimes.
  * [Issue 1441](https://code.google.com/p/conemu-maximus5/issues/detail?id=1441): Don't enable (ENABLE\_QUICK\_EDIT\_MODE|ENABLE\_INSERT\_MODE) by default in new configs.
  * Some internal changes.


## Build 140117 ##
  * New option ‘Show progress indicator’ on ‘Task bar’ page.
  * GuiMacro. Some function must be executed in main thread.
  * [Issue 1434](https://code.google.com/p/conemu-maximus5/issues/detail?id=1434): GuiMacro ‘Status(1,"TEST")’ toggles statusbar.
  * GuiMacro. Allow one-word string arguments with no quotas (powershell-style).
  * GuiMacro. ‘Flash’ function allows to flash taskbar icon and/or window caption.
```
Flash(<Cmd>[,<Flags>[,<Count>]])
  - Allows to flash taskbar icon and/or window caption
    Flash(0) - Stop all flashing
    Flash(0,1) - Simple flashing (see MSDN FlashWindow)
Flash(1,<Flags>,<Count>)
  - Special flashing (see MSDN FlashWindowEx)
    Flags: 0 - stop, 1 - caption, 2 - taskbar, 3 - caption+taskbar, etc.
    Count: the number of times to flash the window
```
  * Fix. ANSI. Wrong behavior of 'ESC [ n L' (VIM & xterm256)


## Build 140116 ##
  * Also show ANSI color indexes on ‘Colors’ settings page.
  * Donate/flattr buttons in the ‘About’ dialog.
  * Some more logging information to LogFiles.
  * [Issue 1430](https://code.google.com/p/conemu-maximus5/issues/detail?id=1430): Progress indicator for aria2 downloader (thecybershadow).
  * Tab Admin shield/suffix. Separate settings, Shield was not be able to disabled.
  * Tab templates. Avoid adding sequential spaces (e.g. if some vars was empty).
  * New tab template var: ‘%a’ - admin suffix.
    * Admin suffix must be enabled on ‘Tabs’ settings page;
    * If ‘%a’ was not specified explicitly, suffix appended at the end;
    * If you don't need suffix in tab at all - clear ‘suffix’ edit box.
  * Apply ‘Skip words from title’ to admin console titles, but not tabs only.
  * Some internal changes.


## Build 140114a ##
  * Fix. Autoupdate msg box info fixed.


## Build 140114 ##
  * Color artifacts in some cases after changing palette on-the-fly (cmd.exe).
  * Fix. ANSI. Wrong behavior of 'ESC [ n M' (VIM & xterm256).
  * GuiMacro. Add 'Palette' description to About dialog.
  * Some code refactoring.


## Build 140113 ##
  * Quake contents was not painted during animation (Revert 4d4fec3f35).
  * GuiMacro. Regression. Remove lags during Far F4 shell macro.
  * `ConEmu.Editor.*` - F4 in panels. Execute in QSearch too.
  * [Issue 1214](https://code.google.com/p/conemu-maximus5/issues/detail?id=1214): MClick & RClick above tabs in fullscreen mode.
  * Protect code from infinite close confirmations.


## Build 140112a ##
  * Just for testing autoupdate from SourceForge.

## Build 140112 ##
  * Allow change of active VCon palette on the fly from Tab menu.
  * GuiMacro. Palette function to change or get palette name
```
Palette([<Cmd>[,"<NewPalette>"]]) or Palette([<Cmd>[,<PaletteIndex>]])
  Cmd=0 - return palette from ConEmu settings
  Cmd=1 - change palette in ConEmu settings, returns prev palette
  Cmd=2 - return palette from current console
  Cmd=3 - change palette in current console, returns prev palette
```
  * Updater. Don't try to extract filename from URL, use predefined format.
  * Shutdown server logging fixes.
  * [Issue 1396](https://code.google.com/p/conemu-maximus5/issues/detail?id=1396): ConEmu was not closed if console was terminated too fast.
  * Remove warnings on GetVersionEx and other internal code changes.
  * Memory leak in macro execution.
  * GuiMacro. Parser refactoring, new syntax allowed.
    * Arguments may be delimited with spaces
    * Several macros in one cmdline may be delimited with "`-GuiMacro`"
```
Usage example:
ConEmuC.exe -GuiMacro palette 1 "<Solarized>" -guimacro WindowMaximize
```
  * ConEmuC. Add linefeed after GuiMacro output if ConEmuC is not redirected.
  * ConEmuC. Don't write terminating '\0' to redirected output.
  * Show all (stable/preview/devel) versions when "Check for updates" called and no newer version was detected.
  * [Issue 1424](https://code.google.com/p/conemu-maximus5/issues/detail?id=1424): Show time in status bar (optional).
  * Ready to new version.ini location. New (debug) switch "`-SetUpdateSrc <URL>`".


## Build 140109 ##
  * Palette was failed to be set properly for elevated consoles.
  * Elevated console was hidden even if "ConEmu /visible" was called.
  * Allow '-'style arguments in ConEmu.exe cmd line.
  * Default terminal. Remove real console flickering when starting console app from explorer.
  * [Issue 1340](https://code.google.com/p/conemu-maximus5/issues/detail?id=1340): Alternatively attached console was closed by Ctrl+C.
  * Macro `Shell("new_console:a")` allowed.
  * Update/fix 'RunAsAdmin' state on tab after attach.
  * Crash if user cancelled console creation in some cases.
  * Memory leak if user cancelled console creation in some cases.
  * Excess logon dialog when running `"cmd -new_console:u:<name>:"`.
  * Win v6.1: conhost PID detection improved.
  * Some internal changes.


## Build 140106 ##
  * Default terminal fixes.
  * Don't show excess paths and quotes in title.
  * Small memory leak while restaring tab on Win7.
  * Don't force ConEmuC64 as root server when using "set" before command.
  * Support `"chcp ansi|oem|utf-8|<CodePageNumber>"` internally by ConEmuC.
  * Support '-'style arguments in ConEmuC.
  * [Issue 1421](https://code.google.com/p/conemu-maximus5/issues/detail?id=1421): PowerShell coloursed prompt displayed incorrectly.
  * Downloader. New internal commands, default to async, and new switches:
    * `-otimeout <ms>` - Our wait for async operation timeout
    * `-timeout <ms>`  - WinInet data receive timeout
    * `-async Y|N`     - Turn On/Off asynchronous mode.


## Build 140104 ##
  * All internet code (Updater) moved to ConEmuCD.dll. Side effect - you may download from http/https/ftp without any external tools.
```
ConEmuC /download [-login <name> -password <pwd>]
    [-proxy <address:port> [-proxylogin <name> -proxypassword <pwd>]]
    "full_url_to_file" "local_path_name"
```
  * Fix DirectoryExist for symlinks.
  * Allow to choose another shell or directory if console startup failed. For example, if your saved named task was created for some directory, which was deleted at some moment. ConEmu will suggest to show standard 'Create new console' dialog where user can change command and directory.
  * ConEmu will check directory existence before creating new tab. If you type wrong startup directory for new process in the 'Create new console' dialog, ConEmu will check it, show warning and allows to change it to smth better.
  * [Issue 1410](https://code.google.com/p/conemu-maximus5/issues/detail?id=1410): Fix unexpected SetCurrentConsoleFontEx from PowerShell.
  * [Issue 1415](https://code.google.com/p/conemu-maximus5/issues/detail?id=1415): Some console output of ConEmuC was not able to redirecting.
  * More information in "ConEmuC /?".
  * Allow "ConEmuC /AUTOATTACH /GHWND=0x???" - attach to exact ConEmu instance.




## Older entries ##
  * [Builds 120101 .. 131225](Whats_New_2.md)
  * [WhatsNew autotranslated](http://translate.google.ru/translate?hl=ru&sl=ru&tl=en&u=http%3A%2F%2Fcode.google.com%2Fp%2Fconemu-maximus5%2Fwiki%2FWhats_New%3Fshow%3Dcontent)
  * [Full changelog on SVN](https://conemu-maximus5.googlecode.com/svn/trunk/ConEmu-alpha/Release/ConEmu/WhatsNew-ConEmu.txt)