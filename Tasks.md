#summary Tasks in ConEmu

The ‘Task’ is one or more (tabbed or splitted) predefined commands (shells) which you may start anytime by name or hotkey.

<img src='http://conemu-maximus5.googlecode.com/svn/files/ConEmuStartTask.png' alt="ConEmu's tasks dropdown" title='Start task dropdown menu'>

Tasks may be configured in the <a href='SettingsTasks.md'>‘Settings’ dialog</a>.<br>
<br>
<h2>Creating new task</h2>
When you want to create new task absent in the default tasks list you need to know:<br>
<br>
<ul><li>The shell name. For example: cmd, powershell, bash and so on.<br>
</li><li>The shell arguments. For example: `"/k vcvarsall.bat x86"<br>
</li><li>Shell working directory. That may be very significant.<br>
</li><li>In some cases you need to know environment variables.</li></ul>

<h3>Where you may get this information?</h3>
In most cases you may open properties of shortcut created by any installer.<br>
Just find the shortcut, right-click it, and choose ‘Properties’.<br>
<br>
<img src='http://conemu-maximus5.googlecode.com/svn/files/ConEmuVsTask1.png' alt='VS tools prompt' title='Searching for VS tools prompt command'>

<ul><li>Copy ‘Target’ contents to the ‘Task Commands’;<br>
</li><li>Optionally set working directory with <code>/dir</code> switch in the ‘Task parameters’;<br>
</li><li>If you want to choose specific tab name use <a href='NewConsole.md'>-new_console:t:"VS 12.0"</a> switch;<br>
</li><li>Finally, if you need to define some environment variables, just specify them before the shell.<br>
<pre><code>set Var1=Value1 &amp; set "Var2=Value with spaces" &amp; cmd /k vcvarsall.bat x86 -new_console:t:"VS 12.0"<br>
</code></pre></li></ul>

Finally, the task for VS prompt is ready.<br>
<br>
<img src='http://conemu-maximus5.googlecode.com/svn/files/ConEmuVsTask2.png' alt='ConEmu VS tools prompt' title='ConEmu task for VS tools prompt'>

<h3>If there is no shortcut for that shell</h3>
Sometimes, if the shell is started from another program<br>
you may use ProcessExplorer or ProcessMonitor to detect<br>
which command is started, arguments and the working directory.<br>
<br>
If you can't do that, just google for alternatives.<br>
For example it is hard to find proper arguments for<br>
NuGet console started from Visual Studio because VS<br>
do not start <code>powershell.exe</code> but run powershell host<br>
internally. Just google! For example:<br>
<br>
<ul><li><a href='http://headsigned.com/article/using-nuget-standalone-command-line'>Using NuGet standalone command-line</a>;<br>
</li><li><a href='http://stackoverflow.com/a/13581202/1405560'>Download Nuget Packages Without VS/NuGet Package Manager</a>;<br>
</li><li><a href='http://stackoverflow.com/a/15000559/1405560'>Do an offline installation into Visual Studio</a>.