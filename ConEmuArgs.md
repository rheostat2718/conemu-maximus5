=Migration to [conemu.github.io](http://conemu.github.io/)=
Google has declared [shutdown](http://google-opensource.blogspot.ru/2015/03/farewell-to-google-code.html) of googlecode service.

New versions of this file will be available on [conemu.github.io](http://conemu.github.io/en/ConEmuArgs.html).

### ConEmu.exe command line switches ###

| **Argument** | **Description** |
|:-------------|:----------------|
| /?           | Show help screen |
| /Config _configname_ | Tells to use an existing or create a new configuration with the specified name. Each configuration keeps its own individual data in the registry or xml file (ConEmu.xml). |
| /Dir _workdir_ | Set working (startup) directory for ConEmu and console processes. This switch override ConEmu startup directory (defined in the ConEmu shortcut for example). |
| /FS | /Max | /Min | (Full screen), (Maximized) or (Minimized) mode |
| /TSA         | Override (enable) minimize to taskbar status area (TSA) |
| /MinTSA      | Start ConEmu in TSA, but minimize to taskbar after restoring. Also ConEmu will **not** be exited after last console close, but hided to TSA. |
| /StartTSA    | Start ConEmu in TSA, but minimize to taskbar after restoring. ConEmu will be **exited** after last console close. |
| /Icon _file_ | Take icon from file (exe, dll, ico) |
| /Title _title_ | Set **fixed** _title_ for ConEmu window. You may use environment variables in _title_. |
| /Multi | /NoMulti | Enable or disable multiconsole features |
| /Single | /Reuse | New console will be started in new tab of existing ConEmu |
| /NoSingle    | Force new ConEmu window even if single mode is selected in the Settings |
| /ShowHide | /ShowHideTSA | May be used with AutoHotKey or desktop shortcuts. Works like ‘Minimize/Restore’ global hotkey. |
| /NoUpdate    | Disable automatic checking for updates on startup |
| /CT`[0|1]`   | Anti-aliasing: /ct0 - off, /ct1 - standard, /ct - clear type |
| /Font _fontname_ | Specify the font name |
| /Size _fontsize_ | Specify the font size |
| /FontFile _fontfilename_ | Loads font from file for ConEmu process (multiple pairs allowed) |
| /FontDir _fontfolder_ | Loads all fonts from folder (multiple pairs allowed) |
| /BufferHeight _lines_ | Set console buffer height. May be used with cmd.exe, PowerShell, etc. |
| /Wnd{X|Y|W|H} _value_ | Set window position and size. |
| /Palette _Name_ | Choose named color palette. |
| /Log | /Log1 | /Log2 | Used to create debug log files |
| /Reset       | Don't load settings from registry/xml. |
| /UpdateJumpList | Update [Windows 7 taskbar jump list](Windows7Taskbar#Customizable_Jump_list.md). May be used for automated ConEmu set up from batches. |
| /LoadCfgFile _file_ | Use specified xml file as configuration storage. |
| /SaveCfgFile _file_ | Save configuration to the specified xml file. |
| /Exit        | Don't create ConEmu window, exit after actions. |
| -new\_console <br> -cur_console <table><thead><th> This <b>special</b> switches may be specified after <b>/cmd</b> switch. Read more about <a href='NewConsole.md'>-new_console and -cur_console</a> switches </th></thead><tbody>
<tr><td> /cmd <i>commandline</i> <br> /cmd @<i>taskfile</i> <br> /cmd <i>{taskname}</i> </td><td> Command line to start. This must be the last used switch (excepting -new_console and -cur_console). You may use ">" and "<code>*</code>" modifiers in <i>taskfile</i> and /BufferHeight argument. <br> <i>taskname</i> is one of the tasks specified on the "Tasks" page of "Settings" dialog. </td></tr>
<tr><td> /cmdlist <i>commands</i> </td><td> Run several tabs on startup. This must be the last used switch (excepting -new_console and -cur_console). Use the same syntax and abilities as on the "Tasks" page of "Settings" dialog. Delimit commands (tabs) with "<code>|</code><code>|</code><code>|</code>". Note, that you must escape delimiter "<code>^|^|^|</code>" when running from cmd-files. </td></tr></tbody></table>

ConEmuC.exe command line switches are described <a href='ConEmuC#ConEmuC.exe_command_line_switches.md'>here</a>.<br>
<br>
<h3>Examples</h3>

1.	ClearType is ON, using font face name "Lucida Console", font height 16. Starts FAR manager in the folder "C:\1 2".<br>
<br>
<pre><code>ConEmu.exe /ct /font "Lucida Console" /size 16 /cmd far.exe "c:\1 2\"<br>
</code></pre>

2.  Starts continuous operation minimized in taskbar status area (TSA), use icon from "cmd.exe".<br>
<pre><code>ConEmu.exe /tsa /min /icon "cmd.exe" /cmd cmd /c dir c:\ /s<br>
</code></pre>

3. Start four cmd tabs in a grid 2x2 (<b>Win+R</b> or <b>shortcut</b> syntax)<br>
<pre><code>ConEmu.exe /cmdlist cmd -cur_console:fn ||| cmd -cur_console:s1TVn ||| cmd -cur_console:s1THn ||| cmd -cur_console:s2THn<br>
</code></pre>

4.	Starts with four tabs: Far Manager, CMD, PowerShell and Bash.<br>
<ul><li>Far.exe will be active console;<br>
</li><li>Cmd.exe will be started with 400 lines buffer height in the <code>%ALLUSERSPROFILE%</code> directory and special color 4F (white on red);<br>
</li><li>PowerShell will be started as Administrator with 1000 lines buffer height in the <code>%USERPROFILE%</code> directory;<br>
</li><li>Bash (MinGW) will be started in the <code>C:\Source</code> directory.</li></ul>

<b>Win+R</b> or <b>shortcut</b> syntax (following is <b>one-line</b> command, splitted for clearness)<br>
<pre><code>ConEmu.exe /cmdlist<br>
  &gt;"C:\Program Files\Far\far.exe"<br>
  ||| cmd /k color 4F "-cur_console:h400d:%ALLUSERSPROFILE%"<br>
  ||| *powershell "-cur_console:h1000d:%USERPROFILE%"<br>
  ||| C:\MinGW\msys\1.0\bin\sh.exe --login -i "-cur_console:d:C:\Source"<br>
</code></pre>
batch files (bat, cmd) syntax (following is <b>one-line</b> command, splitted for clearness)<br>
<pre><code>rem Remove line breaks after paste!<br>
start "ConEmu" "C:\Program Files\ConEmu\ConEmu.exe" /cmdlist<br>
  ^&gt;"C:\Program Files\Far\far.exe"<br>
  ^|^|^| cmd /k color 4F "-cur_console:h400d:%ALLUSERSPROFILE%"<br>
  ^|^|^| *powershell "-cur_console:h1000d:%USERPROFILE%"<br>
  ^|^|^| C:\MinGW\msys\1.0\bin\sh.exe --login -i "-cur_console:d:C:\Source"<br>
</code></pre>
or<br>
<pre><code>ConEmu.exe /cmd @startfile.txt<br>
</code></pre>
or<br>
<pre><code>ConEmu.exe /cmd {taskname}<br>
</code></pre>

<h4>Sample startfile.txt or contents of {taskname}</h4>
<pre><code>&gt;"C:\Program Files\Far\far.exe"<br>
cmd /k color 4F "-cur_console:h400d:%ALLUSERSPROFILE%"<br>
*powershell "-cur_console:h1000d:%USERPROFILE%"<br>
C:\MinGW\msys\1.0\bin\sh.exe --login -i "-cur_console:d:C:\Source"<br>
</code></pre>

<pre><code>start "Three tabs" "C:\Program Files\ConEmu\ConEmu.exe" /cmdlist ^&gt; cmd /k color 4C -cur_console:d:x:\mercurial\blah\blah ^|^|^| cmd /k color 5D -cur_console:d:x:\mercurial\blah\blah\solr ^|^|^| cmd /k color 2A -cur_console:d:x:\mercurial\blah\blah\rep<br>
</code></pre>

<h3>Rarely used switches</h3>

<table><thead><th> /debug </th><th> Show message box immediately after ConEmu starts </th></thead><tbody>
<tr><td> /autosetup 1 | 0 </td><td> Install or remove ConEmu autostart with cmd.exe  </td></tr>
<tr><td> /visible </td><td> Starts with visible real console                 </td></tr>
<tr><td> /detached </td><td> Starts with no console, ConEmu is ready for attaching from another real console or FAR manager </td></tr>
<tr><td> /nocascade </td><td> Disable ‘Cascade’ option may be set in the Settings </td></tr>
<tr><td> /nodefterm </td><td> Don't start initialization procedure for setting up ConEmu as default terminal </td></tr>
<tr><td> /nokeyhooks </td><td> Disable SetWindowsHookEx and global hotkeys      </td></tr>
<tr><td> /nomacro </td><td> Disable hotkeys with GuiMacro actions            </td></tr>
<tr><td> /nohotkey </td><td> Disable all hotkeys                              </td></tr>
<tr><td> /noregfonts </td><td> Disable auto register fonts (font files from ConEmu folder) </td></tr>
<tr><td> /nocloseconfirm </td><td> Disable confirmation of ConEmu's window closing  </td></tr>
<tr><td> /inside<br>/insidepid <i>PID</i><br>/insidewnd x<i>HWND</i> </td><td> Starts ConEmu in ‘inside’ mode (act as child window of parent process) </td></tr>
<tr><td> /demote </td><td> Run command de-elevated, using Task Sheduler. May be useful in Vista and higher with UAC enabled. Example: ConEmu.exe /demote /cmd powershell.exe </td></tr>
<tr><td> /resetdefault </td><td> Same as /reset <b>and</b> don't show <a href='SettingsFast.md'>fast configuration</a> dialog. </td></tr>
<tr><td> /basic </td><td> Same as /resetdefault <b>and</b> disable ‘Save settings’ button in <a href='Settings.md'>Settings</a> dialog. </td></tr>