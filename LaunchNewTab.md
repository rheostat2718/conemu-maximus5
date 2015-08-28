<font size='5'><b>Launching applications in ConEmu</b></font>


## [Using ConEmu switches](ConEmuArgs.md) ##
When you run something from `Win+R` or shortcut from your Desktop,
you may use `-cmd` or `-cmdlist` to run your application.
Remember, `-cmd` or `-cmdlist` will be the last ConEmu's GUI interpreted switch.
The rest of command line will be used to start your application.
Read more in the [wiki](ConEmuArgs.md). Example:

> ConEmu -reuse -dir "c:\projects" -cmd "set PATH=C:\MinGW\bin;%PATH%" & chcp 65001 & sh -l -i

## Create new console dialog ##
How to start application (tcc, powershell, far, putty) in ConEmu tab?

<img src='http://conemu-maximus5.googlecode.com/svn/files/ConEmuCreate.png' alt='ConEmu new console' title='ConEmu confirmation of new console creation'>

This dialog may be opened from:<br>
<ul><li>Keyboard (Win+W by default), or<br>
</li><li><img src='http://conemu-maximus5.googlecode.com/svn/files/ConEmuAddBtn.png' alt='Toolbar' title='Toolbar'>
</li><li><img src='http://conemu-maximus5.googlecode.com/svn/files/ConEmuAddSys.png' alt='System menu' title='System menu'></li></ul>

Well, what you can do with this dialog?<br>
<ul><li>Choose command line and startup directory for new process. You may use environment variables in both fields. Also, you may choose from drop down list either any of predefined <a href='Tasks.md'>tasks</a> or any command from history.<br>
</li><li>Choose another user name and password, ‘Restricted current user’ account or ‘Run as administrator’ feature.<br>
</li><li>You may split active tab vertically or horizontally (you need last alpha versions for this feature).<br>
</li><li>And it is possible to start new tab in the new ConEmu window (new <code>ConEmu.exe</code> process will be started).<br>
</li><li>Dialog's system menu contains ‘Reset command history’ menu item.</li></ul>

Similar dialog may be opened when you <a href='RestartTab.md'>‘Restart current console’</a>.<br>
<br>
<h3>Create new console without dialog</h3>
You may switch off create new console confirmation, just disable ‘Create confirmation’ checkbox<br>
on <a href='Settings#Appearance.md'>Appearance</a> settings page.<br>
<br>
<b>Note</b>. You may pop up confirmation dialog even if ‘Create confirmation’ unchecked.<br>
Just hold Shift while creating console from toolbar, System menu, or Keyboard (Win+Shift+W by default).<br>
<br>
<h2><a href='Tasks.md'>Tasks</a></h2>
You may define your own tasks or just use predefined one.<br>
Choose the task from drop down menu or run task by hotkey.<br>
You may choose task in the ‘Create new console’ dialog too.<br>
<br>
<h2>Create new tab from existing one</h2>
Use <a href='NewConsole.md'>-new_console</a> switch while executing command.<br>
For example, if you want to start PuTTY in new ConEmu tab,<br>
type in command prompt (cmd.exe started in ConEmu tab)<br>
<pre><code>putty -new_console<br>
</code></pre>
and it will be launched in the new tab of ConEmu.<br>
<br>
-new_console works even in batch files.<br>
<br>
<b>Note</b>. Option ‘Inject ConEmuHk’ must be turned on (Settings -> Features).<br>
<br>
<h2>Duplicate root</h2>
Well, this interesting feature offers a way to duplicate shell (root process of current tab) state<br>
to the new ConEmu tab.<br>
For example, you have started <code>cmd.exe</code>, set up environment variables, cd somewhere,<br>
run any program (vim for example) and call ‘Duplicate root’ from tab popup menu. This will<br>
create ‘copy’ of <code>cmd.exe</code> at most recent state (when possible).<br>
<br>
<h2>Default terminal feature</h2>
This allows to seize creation of console processes from many (selected by user) applications.<br>
For example, when you run ‘Command prompt’ shortcut from ‘Start menu’ cmd.exe may start in the new ConEmu tab.<br>
Read more about <a href='DefaultTerminal.md'>Default terminal feature</a>.<br>
<br>
<h2>Attach existing console or GUI application</h2>
Choose ‘Attach...’ from <a href='SystemMenu.md'>system menu</a>.<br>
<br>
<h2>Attach from command prompt</h2>
Batch file ‘Attach.cmd’ ships with ConEmu package. Run this file from any cmd.exe to attach this console<br>
into the new ConEmu tab. This batch just run following command<br>
<pre><code>ConEmuC.exe /ATTACH /NOCMD<br>
</code></pre>
which you may run from any other console shell or even from your own console application.<br>
<br>
<h2>Automatic attach of cmd</h2>
Mostly like as ‘Attach from command prompt’ but you may set up automatic execution of ‘Attach’ on cmd.exe startup.<br>
This also works with TCC/LE. Set up feature on ‘ComSpec’ page of ‘Settings’ dialog.<br>
<br>
<h2>Gui Macro</h2>
You may set up several most used shells or build environments via GuiMacro.<br>
Example: open ‘Keys & Macro’ settings page, click ‘Macros’ radio (it filters long key list)<br>
and in the free ‘Macro XX’ slot type<br>
<pre><code>shell("","C:\Windows\System32\cmd.exe /E:ON /V:ON /T:0E /K ""C:\\Program Files\\Microsoft SDKs\\Windows\\v7.1\\Bin\\SetEnv.cmd"" /Release /x86")<br>
</code></pre>
choose any wanted hotkey (<b>Ctrl</b>+<b>Shift</b>+<b>1</b> for example) and <b>Save settings</b>.