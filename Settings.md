# ConEmu has moved to conemu.github.io #
New versions of this file are available on [conemu.github.io](http://conemu.github.io/en/Settings.html).

Google has declared [shutdown](http://google-opensource.blogspot.ru/2015/03/farewell-to-google-code.html) of googlecode service.

<font size='5'><b>ConEmu setup</b></font>


# Where settings are stored #
ConEmu may store its settings in the Windows registry (HKCU), or in the ConEmu.xml file.

## Registry ##
All settings are stored in registry in the following key:

`[HKEY_CURRENT_USER\Software\ConEmu\.Vanilla]`

When You specify the [/config](Command_Line.md) switch in the ConEmu.exe command line settings will be stored here:

`[HKEY_CURRENT_USER\Software\ConEmu\«configname»]`

## ConEmu.xml ##
You may use ConEmu in «portable» mode, so all settings will be stored in the _ConEmu.xml_ file.
ConEmu search sequence of this file:

| `%ConEmuDir%\ConEmu.xml` | Folder with `ConEmu.exe` and `ConEmu64.exe` |
|:-------------------------|:--------------------------------------------|
| `%ConEmuBaseDir%\ConEmu.xml` | Folder with `ConEmuC.exe` and `ConEmuC64.exe` |
| `%APPDATA%\ConEmu.xml`   | I don't think this is really «portable», but many users was asked about `%APPDATA%` |

On [first time ConEmu run](SettingsFast.md), you may choose «portable» mode and location of xml file.

Or you may create manually a new empty _ConEmu.xml_ or just rename _ConEmu\_Sample.xml_ to _ConEmu.xml_
for engaging xml-mode. Template file _ConEmu\_Sample.xml_ is shipped with ConEmu.
You may use [named configuration (/config)](Command_Line.md) switch with xml-mode too.
Also, there are switches [/loadcfgfile and /safecfgfile](Command_Line.md) to use any special locations
(not so useful for daily using, but available).

Note. You may rename/create _ConEmu.xml_ file any time, even after ConEmu starts and loads its settings from registry.

# Manual change of settings #
So, You want to change setting, which is absent in the Settings dialog.
List and description of each setting You may find in the _Settings-ConEmu.reg_ file.
## Using registry ##
Go to Registry editor (be careful!) and change appropriate value in the ConEmu subkeys:
  * `[HKEY_CURRENT_USER\Software\ConEmu\.Vanilla]`, when [/config](Command_Line.md) switch is not using;
  * `[HKEY_CURRENT_USER\Software\ConEmu\«configname»]`, when /config switch specified in the ConEmu command line.
## Using ConEmu.xml ##
Open ConEmu.xml file using any text editor (Far Manager, Notepad, and so on) find appropriate value and change it.
Usually, this file is located near to the _ConEmuC.exe_.

# Settings dialog #
You may open settings dialog in several ways
  * Choosing «Settings...» menu item in ConEmu [system menu](SystemMenu.md), which can be opened
    * by right-clicking on the ‘Menu’ icon on the toolbar
    * by right-clicking on the window title
    * by left-clicking on the icon in the ConEmu title bar
    * by pressing Win+Alt+Space on the keyboard
    * by right-clicking on the ConEmu (running) icon in the taskbar (in Windows 7 you must hold down Shift key)
    * by right-clicking on the ConEmu icon in [TSA](ConEmuTerms#TSA.md) (if you enable TSA feature)
  * Or, you may press Win+Alt+P on the keyboard (predefined hotkey)

## Main ##
<img src='http://conemu-maximus5.googlecode.com/svn/files/SettingsMain.png' alt='ConEmu settings, Main page' title='ConEmu settings, Main page'>

<h2>Size and Pos</h2>
<img src='http://conemu-maximus5.googlecode.com/svn/files/SettingsSizeAndPos.png' alt='ConEmu settings, Size and position page' title='ConEmu settings, Size and position page'>

<h2>Appearance</h2>
<img src='http://conemu-maximus5.googlecode.com/svn/files/SettingsAppearance.png' alt='ConEmu settings, Appearance page' title='ConEmu settings, Appearance page'>

<h2>Task bar</h2>
<img src='http://conemu-maximus5.googlecode.com/svn/files/SettingsTaskBar.png' alt='ConEmu settings, Task bar page' title='ConEmu settings, Task bar page'>

<h2>Automatic update</h2>
<img src='http://conemu-maximus5.googlecode.com/svn/files/SettingsUpdate.png' alt='ConEmu settings, Update page' title='ConEmu settings, Update page'>

<h2>Startup</h2>
<img src='http://conemu-maximus5.googlecode.com/svn/files/SettingsStartup.png' alt='ConEmu settings, Startup page' title='ConEmu settings, Startup page'>

<h2>Tasks</h2>
<img src='http://conemu-maximus5.googlecode.com/svn/files/SettingsTasks.png' alt='ConEmu settings, Tasks page' title='ConEmu settings, Tasks page'>

Here you can configure a list of common tasks. In fact, this is an alias to run one or more applications in new tabs within ConEmu. These can be configured in the 'Tasks' page of the 'Settings' dialog, and stored in the "Tasks" subkey of reg/xml settings. 'Tasks' may be used as follows (as an example, we will use a task named {Shells}):<br>
<ul><li>when you start by specifying the properties of the shortcut: "ConEmu.exe / cmd {Shells}";<br>
</li><li>specifying {Shells} in the 'Command line' page 'Main' of 'Settings' dialog;<br>
</li><li>when you create a new console interface ConEmu (<code>+</code> on the toolbar, a list of Recreate-dialog);<br>
</li><li>from the command line (cmd.exe): "%ConEmuBaseDir%\ConEmuC.exe" / c {Shells} -new_console.<br>
</li><li>from the command line (far.exe): conemu:run:{Shells} -new_console</li></ul>

The ConEmu Jump list is set here too. Set up a list of tasks in the field of «ConEmu arguments for Jump list» You can optionally specify the icon that is displayed in the Jump list, for example<br>
<pre><code>/icon "cmd.exe"<br>
</code></pre>
и рабочую папку, в которой будет запущен указанный процесс, например<br>
<pre><code>/dir "c:\Program Files"<br>
</code></pre>
После настройки списка задач включите флажок «Add ConEmu tasks to taskbar» и (по желанию) «Add commands from history, too».<br>
Нажмите кнопку «Update Now!». В случае успеха вы увидите сообщение «Taskbar jump list was updated successfully», ну или сообщение об ошибке.<br>
Есть способ инициировать Jump list при запуске ConEmu (<a href='https://code.google.com/p/conemu-maximus5/issues/detail?id=576'>Issue 576</a>, может кому еще понадобится для автоматизации установки, например)<br>
для этого запустите (однократно) ConEmu.exe с аргументом "<code>/updatejumplist</code>".<br>
<br>
<h2>ComSpec</h2>
<img src='http://conemu-maximus5.googlecode.com/svn/files/SettingsComspec.png' alt='ConEmu settings, Comspec page' title='ConEmu settings, Comspec page'>

<h2>Features</h2>
<img src='http://conemu-maximus5.googlecode.com/svn/files/SettingsFeatures.png' alt='ConEmu settings, Features page' title='ConEmu settings, Features page'>

<h3>RealConsole font</h3>
Open this dialog from <a href='SettingsFeatures.md'>Features page</a> <code>-&gt;</code> button ‘<code>...</code>’ on the right of ‘Show real console’<br>
<img src='http://conemu-maximus5.googlecode.com/svn/files/SettingsMoreRealFont.png' alt='ConEmu settings, RealConsole font' title='ConEmu settings, RealConsole font'>

<h2>Text cursor</h2>
<img src='http://conemu-maximus5.googlecode.com/svn/files/SettingsTextCursor.png' alt='ConEmu settings, Text cursor page' title='ConEmu settings, Text cursor page'>

<h2>Colors</h2>
<img src='http://conemu-maximus5.googlecode.com/svn/files/SettingsColors.png' alt='ConEmu settings, Colors page' title='ConEmu settings, Colors page'>

<img src='http://conemu-maximus5.googlecode.com/svn/files/SettingsColors2.png' alt='ConEmu settings, Colors page' title='ConEmu settings, Colors page'>

<h2>Transparency</h2>
<a href='SettingsTransparency.md'>Read more</a><br>
<img src='http://conemu-maximus5.googlecode.com/svn/files/SettingsTransparency.png' alt='ConEmu settings, Transparency page' title='ConEmu settings, Transparency page'>

<h2>Tabs</h2>
<img src='http://conemu-maximus5.googlecode.com/svn/files/SettingsTabs.png' alt='ConEmu settings, Tabs page' title='ConEmu settings, Tabs page'>

<h2>Status bar</h2>
<img src='http://conemu-maximus5.googlecode.com/svn/files/SettingsStatus.png' alt='ConEmu settings, Status bar page' title='ConEmu settings, Status bar page'>

<h2>Integration</h2>
<img src='http://conemu-maximus5.googlecode.com/svn/files/SettingsIntegration.png' alt='ConEmu settings, Integration page' title='ConEmu settings, Integration page'>

<h2>App distinct</h2>
<img src='http://conemu-maximus5.googlecode.com/svn/files/SettingsDistinct.png' alt='ConEmu settings, App distinct page' title='ConEmu settings, App distinct page'>

<img src='http://conemu-maximus5.googlecode.com/svn/files/SettingsDistinct2.png' alt='ConEmu settings, App distinct page' title='ConEmu settings, App distinct page'>

<h2>Keys and Macro</h2>
<img src='http://conemu-maximus5.googlecode.com/svn/files/SettingsKeys.png' alt='ConEmu settings, Keys and Macro page' title='ConEmu settings, Keys and Macro page'>

<h2>Controls</h2>
<img src='http://conemu-maximus5.googlecode.com/svn/files/SettingsControls.png' alt='ConEmu settings, Controls page' title='ConEmu settings, Controls page'>

<h2>Mark & Paste</h2>
<a href='TextSelection.md'>Go to description</a>

<img src='http://conemu-maximus5.googlecode.com/svn/files/SettingsSelection.png' alt='ConEmu settings, Text selection' title='ConEmu settings, Text selection'>

<a href='Hidden comment: 
===Hide caption details===
<img src="http://conemu-maximus5.googlecode.com/svn/files/SettingsMoreHideCaption.png" title="ConEmu settings, Hide caption details" alt="ConEmu settings, Hide caption details">
'></a><br>
<br>
<h2>Far Manager</h2>
<img src='http://conemu-maximus5.googlecode.com/svn/files/SettingsFarManager.png' alt='ConEmu settings, Far Manager page' title='ConEmu settings, Far Manager page'>

<h2>Views</h2>
This is settings for 'Panel Views' Far Manager plugin.<br>
<img src='http://conemu-maximus5.googlecode.com/svn/files/SettingsViews.png' alt='ConEmu settings, Views page' title='ConEmu settings, Views page'>

<h2>Info</h2>
<img src='http://conemu-maximus5.googlecode.com/svn/files/SettingsInfo.png' alt='ConEmu settings, Info page' title='ConEmu settings, Info page'>

<h2>Debug</h2>
Enables advanced logging of console processes creation.<br>
<img src='http://conemu-maximus5.googlecode.com/svn/files/SettingsDebug.png' alt='ConEmu settings, Debug page' title='ConEmu settings, Debug page'>

<h1>Alphabetical</h1>
<table><thead><th> lbLDragKey </th><th> Drag with left mouse button, only when pressed... </th></thead><tbody>
<tr><td> cbDropEnabled </td><td> Off - ConEmu will NOT receive external drops<br>On - Drops allowed (FAR confirmation dialog)<br>3s - same as 'On', but w/o confirmation </td></tr>
<tr><td> lbRDragKey </td><td> Drag with right mouse button, only when pressed... </td></tr>
<tr><td> cbDragL    </td><td> Allow drag with left mouse button                 </td></tr>
<tr><td> cbDragR    </td><td> Allow drag with right mouse button                </td></tr>
<tr><td> cbMultiCon </td><td> Turn on MultiConsole<br>(several consoles in one ConEmu, alternative console)<br>ConEmu restart required </td></tr>
<tr><td> cbMinToTray </td><td> Auto minimize to Taskbar Status Area (TSA)<br>Right click on 'Close window button'<br>in ConEmu title for minimize to TSA,<br>when this checkbox is OFF.<br>3s - Always show notification icon </td></tr>
<tr><td> tsTopPID   </td><td> FAR PID and FAR with plugin PID                   </td></tr>
<tr><td> tFontFace  </td><td> Face name for main font                           </td></tr>
<tr><td> tFontSizeY </td><td> Main font height                                  </td></tr>
<tr><td> tFontCharset </td><td> Main font charset                                 </td></tr>
<tr><td> tFontFace2 </td><td> Font face used to draw FAR borders                </td></tr>
<tr><td> cbExtendColors </td><td> You can use up to 32 foreground colors<br>(instead of console standard 16) at the<br>expense of one of background colors </td></tr>
<tr><td> tRealFontMain </td><td> Real sizes of main font:<br>Height x Width x AveWidth </td></tr>
<tr><td> tRealFontBorders </td><td> Real sizes of 'Fix FAR borders' font:<br>Height x Width </td></tr>
<tr><td> cbBold     </td><td> Main font weight                                  </td></tr>
<tr><td> tFontSizeX </td><td> Main font width<br>0 - mean defailt width<br>for specified height </td></tr>
<tr><td> cbItalic   </td><td> Main font italic                                  </td></tr>
<tr><td> tWndWidth  </td><td> Width of console window<br>This is in characters  </td></tr>
<tr><td> tWndHeight </td><td> Height of console window<br>This is in characters </td></tr>
<tr><td> cbMonospace </td><td> Check it for non proportional (monospaced) fonts, 3d state means - center symbols in the cells </td></tr>
<tr><td> tFontSizeX2 </td><td> WIDTH for additional font.<br>It must be wider than main font,<br>otherwise, You get 'dashed' borders. </td></tr>
<tr><td> cbSendAE   </td><td> When is on, You can't enter fullscreen mode<br>from keyboard, but You can use 'Alt+Enter'<br>for macros in FAR </td></tr>
<tr><td> cbDnDCopy  </td><td> Forced 'Copy' action as default,<br>You can use Ctrl, Shift or Alt<br>during drag to change action </td></tr>
<tr><td> cbTabs     </td><td> Show tabs (all opened panels, editors and viewers)<br>on the top of ConEmu window </td></tr>
<tr><td> cbEnhanceGraphics </td><td> Enhance appearence of progressbars and scrollbars </td></tr>
<tr><td> cbRClick   </td><td> Off - all RClicks passed to console<br>On - EMenu called instead of RClick<br>3s - Short RClick passed, Long RClick -> EMenu<br>Warning! This may conflict with RDrag </td></tr>
<tr><td> cbBgImage  </td><td> Show background image instead standard<br>colors #0 and #1 (black and blue)<br>3d state - don't draw in viewer/editor </td></tr>
<tr><td> tThumbsX1  </td><td> Spacing in pixels from the left side of cell to the left side of preview </td></tr>
<tr><td> tThumbsY1  </td><td> Spacing in pixels from the top side of cell to the top side of preview </td></tr>
<tr><td> tThumbsX2  </td><td> Spacing in pixels from the right side of preview to the right side of cell </td></tr>
<tr><td> tThumbsY2  </td><td> Spacing in pixels from the bottom side of preview to the bottom side of cell (text label is placed here) </td></tr>
<tr><td> tThumbsSpacing </td><td> Vertical spacing in pixels (top and bottom) of text label </td></tr>
<tr><td> tThumbsPadding </td><td> Horizontal spacing in pixels (left and right) of text label </td></tr>
<tr><td> tTilesX1   </td><td> Spacing in pixels from the left side of cell to the left side of preview </td></tr>
<tr><td> tTilesY1   </td><td> Spacing in pixels from the top side of cell to the top side of preview </td></tr>
<tr><td> tTilesX2   </td><td> Spacing in pixels from the right side of preview to the right side of cell (text label is placed here) </td></tr>
<tr><td> cbRSelectionFix </td><td> Fix dashed selection with fast right mouse drag   </td></tr>
<tr><td> cbMonitorConsoleLang </td><td> Monitor input language change in real<br>console (i.e. XLat switching support) </td></tr>
<tr><td> cbSkipFocusEvents </td><td> Don't send focus events to console to disable<br>autoclosing of fast search dialog in panels </td></tr>
<tr><td> hkNewConsole </td><td> Hotkey for creating new console                   </td></tr>
<tr><td> hkSwitchConsole </td><td> Hotkey for switching between consoles             </td></tr>
<tr><td> cbSkipActivation </td><td> Skip mouse button (left/right/middle)<br>click, while activating window </td></tr>
<tr><td> cbSkipMove </td><td> Skip mouse move events, while<br>ConEmu is not foreground window </td></tr>
<tr><td> cbVisible  </td><td> Show real console on startup                      </td></tr>
<tr><td> cbNewConfirm </td><td> Confirm new console creation.<br>You may change cmd line for it, or turn on 'Run as...' feature.<br>Even if unchecked, You may hold 'Shift' to display dialog. </td></tr>
<tr><td> cbDragImage </td><td> Create and display overlay transparent snapshoot of files are dragged </td></tr>
<tr><td> cbAutoRegFonts </td><td> Search, register for process, and use first <code>*.ttf</code> file in conemu folder </td></tr>
<tr><td> cbDebugSteps </td><td> Show some debug information in ConEmu title<br>I.e. plugin communication steps. </td></tr>
<tr><td> hkCloseConsole </td><td> Hotkey for recreating (or closing)<br>active console </td></tr>
<tr><td> bRealConsoleSettings </td><td> You may change font face and size in the Real console. Console restart required. </td></tr>
<tr><td> cbCursorBlink </td><td> When 'Blinking' is ON - cursor blinks<br>with standard cursor blink rate. </td></tr>
<tr><td> cbTabSelf  </td><td> Handle CtrlTab and CtrlShiftTab internally<br>(by ConEmu). This keystrokes will not be sent<br>to console window, bu You can easily<br>switch between consoles (panels). </td></tr>
<tr><td> cbTabLazy  </td><td> When checked - real window switching<br>will be performed on Ctrl depress </td></tr>
<tr><td> cbTabRecent </td><td> Switch first between recent tabs.<br>You may still switch between tabs in<br>standard manner using Left/Right<br>(after CtrlTab), while Ctrl is still presses. </td></tr>
<tr><td> cbLongOutput </td><td> Autoexpand bufferheight to specified<br>number of lines, while FAR executes<br>console commands. Full console output<br>(up to specified count of lines) will be<br>available via FAR macro (CtrlO.reg) </td></tr>
<tr><td> tLongOutputHeight </td><td> Size of bufferheight, while FAR<br>executes console commands </td></tr>
<tr><td> tFontSizeX3 </td><td> Cell width for 'Monospace' mode                   </td></tr>
<tr><td> tWndX      </td><td> Upper left corner of ConEmu in Normal mode<br>This is in pixels </td></tr>
<tr><td> tWndY      </td><td> Upper left corner of ConEmu in Normal mode<br>This is in pixels </td></tr>
<tr><td> lbExtendIdx </td><td> Choose background color index, used to increase foreground color index </td></tr>
<tr><td> lbExtendFontItalicIdx </td><td> Choose background color index for which «Italic» font properties whill be inverted (default is Magenta #13) </td></tr>
<tr><td> lbExtendFontNormalIdx </td><td> When Bold or Italic font property was inverted, ConEmu may change background to default color (default is Blue #1) </td></tr>
<tr><td> cbFixFarBorders </td><td> You can specify additional font for drawing FAR borders.<br>I.e. Main font is 'Fixedsys', additional is 'Lucida Console'. </td></tr>
<tr><td> cbCursorColor </td><td> ON - cursor emulates console behaviour (sort of colors inversion)<br>OFF - cursor is white (color#15) on dark backgrounds,<br>and black (color#0) on light backgrounds </td></tr>
<tr><td> cbDragIcons </td><td> Show icons of dragged iterms                      </td></tr>
<tr><td> lbDefaultColors </td><td> You may choose one of predefined color schemes.<br>Each scheme customize first (main) 16 colors. </td></tr>
<tr><td> lbCmdOutputCP </td><td> Windows command processor (cmd.exe) may cause then output of internal commands to be OEM or Unicode.<br>You may force this selection, or use automatic selection (FAR2 -> Unicode). </td></tr>
<tr><td> lbNtvdmHeight </td><td> Old DOS programs may be runned under console sizes: 80x25, 80x28, 80x43 or 80x50. You may force window size selection, or left the automatic selection.<br>This feature is not available in x64 OS. </td></tr>
<tr><td> cbFARuseASCIIsort </td><td> Hook FAR string sort functions.<br>!!!HIGHLY EXPERIMENTAL!!! </td></tr>
<tr><td> cbFixAltOnAltTab </td><td> When You set a macro on Alt (RAlt) it can unexpectedly activates on AltTab or AltF9. This issue can be fixed by sending to console Control depress before Alt release. </td></tr>
<tr><td> cbFarHourglass </td><td> Show AppStarting cursor (arrow with a small hourglass) when FAR is not responding (during long operations) </td></tr>
<tr><td> slTransparent </td><td> Transparency of the main ConEmu window            </td></tr>
<tr><td> cbHideCaption </td><td> Hide main window caption, when maximized (Alt-F9) </td></tr>
<tr><td> cbFontAuto </td><td> Automatic font resize for the fixed real console width </td></tr>
<tr><td> cbLogs     </td><td> Write debug information to log files. Useful for creating bugreports. </td></tr>
<tr><td> cbDragPanel </td><td> Enable sizing of left and right panel by dragging with mouse.<br>3d state - resize on button release with macroses. </td></tr>
<tr><td> cbHideCaptionAlways </td><td> Remove window border and caption. Only console and tabs will be shown. </td></tr>
<tr><td> cbTryToCenter </td><td> Draw console content in center of ConEmu window   </td></tr>
<tr><td> cbDesktopMode </td><td> Act as a part of Windows Desktop                  </td></tr>
<tr><td> cbAlwaysOnTop </td><td> Places the ConEmu window above all non-topmost windows. The window maintains its topmost position even when it is deactivated </td></tr>
<tr><td> cbUserScreenTransparent </td><td> Turn on «User screen» transparency, when panel(s) is lifted up or hided. You may temporary reveal «User screen» by depressing Ctrl-Alt-Shift. </td></tr>
<tr><td> tBgImageColors </td><td> Choose background color indexes, which will be replaced with background image (default is «#0 #1») </td></tr>
<tr><td> cbExtendFonts </td><td> ConEmu is able to use normal, bold and italic fonts side by side. This feature is useful with Colorer FAR plugin. </td></tr>
<tr><td> lbExtendFontBoldIdx </td><td> Choose background color index for which «Bold» font properties whill be inverted (default is Red #12) </td></tr>
<tr><td> cbTrueColorer </td><td> Colorer truemod support                           </td></tr>
<tr><td> cbDropUnlocked </td><td> Unlock source and target window on Drop operation. This may cause unpredictable results, when DragSource creates temp dragged files ONLY for drag lifetime. </td></tr>
<tr><td> cbDropUseBCopy </td><td> Use BCopy service for drop operations («Unlocked drop» must be checked) </td></tr>
<tr><td> cbFadeInactive </td><td> When ConEmu looses focus, its contents may be faded. You may specify most bright color. </td></tr>
<tr><td> cbBlockInactiveCursor </td><td> Draw empty rectangle cursor while ConEmu has no focus </td></tr>
<tr><td> bHideCaptionSettings </td><td> Choose frame width, appearance and disappearance delays </td></tr>
<tr><td> tHideCaptionAlwaysFrame </td><td> While caption and frame are hidden, ConEmu may keep small part of frame (in pixels) around console part. Default is 1 pixel. </td></tr>
<tr><td> tHideCaptionAlwaysDelay </td><td> Delay in milliseconds, for Caption and Frame appearance </td></tr>
<tr><td> tHideCaptionAlwaysDissapear </td><td>                                                   </td></tr>
<tr><td> bResetSettings </td><td> Reset all settings to defaults                    </td></tr>
<tr><td> bReloadSettings </td><td> Reload all settings from registry/xml             </td></tr>
<tr><td> cbHandleFarDetach </td><td> Automatic attach to ConEmu new Far manager console, created on detach (CtrlAltTab) </td></tr>
<tr><td> cbHookFarRegistry </td><td> Enable portable mode for FAR manager. ConEmu plugin must be installed. </td></tr>
<tr><td> tFadeLow   </td><td> When ConEmu looses focus, its contents may be faded. You may specify here 'low shift' (wich makes dark colors lighter). </td></tr>
<tr><td> tFadeHigh  </td><td> When ConEmu looses focus, its contents may be faded. You may specify here 'high shift' (wich makes light colors darker). </td></tr>
<tr><td> tPerfFPS   </td><td> Frames per second                                 </td></tr>
<tr><td> tPerfData  </td><td> Average duration of transferring data<br>from CRealConsole to CVirtualConsole </td></tr>
<tr><td> tPerfRender </td><td> Average duration of rendering text to memory DC   </td></tr>
<tr><td> tPerfBlt   </td><td> Average duration of blit operation<br>from memory DC to screen DC </td></tr>
<tr><td> tPerfInterval </td><td> Average counts of reads real console data per second </td></tr>
<tr><td> cbUseWinNumber </td><td> Enables switching of tabs (30 consoles) by their numbers (1,2,...,9,0). «Host-key» is «Win» key, by default. </td></tr>
<tr><td> cbInstallKeybHooks </td><td> Allows «Host-key»+Number in Vista, Windows 7, or higher<br>3d state - confirm on startup </td></tr>
<tr><td> cbSafeFarClose </td><td> Try to close Far Manager softly, instead of closing console window </td></tr>
<tr><td> cbIgnoreTelnetCursorSize </td><td> Always show normal cursor in telnet               </td></tr>
<tr><td> tThumbsFontName </td><td> Text labels font name in Thumbnails mode          </td></tr>
<tr><td> tThumbsFontSize </td><td> Text labels font height in Thumbnails mode        </td></tr>
<tr><td> tTilesFontName </td><td> Text labels font name in Tiles mode               </td></tr>
<tr><td> tThumbMaxZoom </td><td> Maximal zoom for images smaller than preview      </td></tr>
<tr><td> cbThumbLoadFolders </td><td> Generate previews for folders (by first 4 files)  </td></tr>
<tr><td> cbThumbLoadFiles </td><td> Generate previews for files                       </td></tr>
<tr><td> tThumbsImgSize </td><td> Size in pixels of preview square in Thumbnails mode<br>Default is 96 </td></tr>
<tr><td> tTilesFontSize </td><td> Text labels font height in Tiles mode             </td></tr>
<tr><td> tTilesImgSize </td><td> Size in pixels of preview square in Tiles mode<br>Default is 48 </td></tr>
<tr><td> tThumbLoadingTimeout </td><td> Maximal duration of preview generation (per file) </td></tr>
<tr><td> tTilesY2   </td><td> Spacing in pixels from the bottom side of preview to the bottom side of cell </td></tr>
<tr><td> tTilesSpacing </td><td> Spacing in pixels on the left of text label       </td></tr>
<tr><td> tTilesPadding </td><td> Spacing in pixels on the right of text label      </td></tr>
<tr><td> cbThumbRestoreOnStartup </td><td> Restore panel views on FAR startup                </td></tr>
<tr><td> cbSleepInBackground </td><td> Reduce FPS when ConEmu loose focus                </td></tr>
<tr><td> cbExtendUCharMap </td><td> Show glyphs from selected font in «Unicode CharMap» plugin (FAR2 only) </td></tr>
<tr><td> cbShellNoZoneCheck </td><td> SEE_MASK_NOZONECHECKS. Same as appeared in Far 2 build 771 and disappeared after 1464. </td></tr>
<tr><td> cbThumbUsePicView2 </td><td> Try to use PicView2 plugin to generate previews (FAR2 only) </td></tr>
<tr><td> tTabConsole </td><td> Common tab template (Far panels or any other console program)<br>%s - Title, %i - Window number (always 0 here),<br>%% - '%' sign </td></tr>
<tr><td> tTabEditor </td><td> Tab template for Far internal editors<br>%s - Title, %i - Window number (same as F12 shows),<br>%% - '%' sign </td></tr>
<tr><td> tTabViewer </td><td> Tab template for Far internal viewers<br>%s - Title, %i - Window number (same as F12 shows),<br>%% - '%' sign </td></tr>
<tr><td> tTabEditorMod </td><td> Tab template for Far internal modified editors<br>%s - Title, %i - Window number (same as F12 shows),<br>%% - '%' sign </td></tr>
<tr><td> cbAdminShield </td><td> When this is checked - «Shield» icon will be shown in tabs, started «As administrator».<br>When unchecked - suffix will be appended for that tabs. You may clear suffix edit-field. </td></tr>
<tr><td> cbSendAltSpace </td><td> When is on, You can use 'Alt+Space' in FAR internally.<br>You can pop up system menu with Win+Alt+Space </td></tr>
<tr><td> cbBgAllowPlugin </td><td> Enable background Far plugins (i.e. Panel Colorer), 3d state means - don't draw in viewer/editor </td></tr>
<tr><td> cbAlwaysShowTrayIcon </td><td> Always show ConEmu icon in the Taskbar Status Area (TSA) </td></tr>
<tr><td> cbAlwaysShowScrollbar </td><td> Always show scrollbar on the right edge of window<br>3d state means - autoshow on scroll </td></tr>
<tr><td> Use ConEmuHk injects </td><td> Allow injecting ConEmuHk.dll (ConEmuHk64.dll) in every process of ConEmu console window </td></tr></tbody></table>
