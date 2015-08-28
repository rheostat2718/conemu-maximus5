#summary How to change the default terminal emulator on Windows

# ConEmu has moved to conemu.github.io #
New versions of this file are available on [conemu.github.io](http://conemu.github.io/en/DefaultTerminal.html).

Google has declared [shutdown](http://google-opensource.blogspot.ru/2015/03/farewell-to-google-code.html) of googlecode service.

# Introduction #

In fact, Windows does not provide official way to change default terminal application.
That is because console subsystem is a part of Windows kernel (`csrss.exe` up to Vista and `conhost.exe` in Windows 7 and higher).

Only use of hacking methods can ‘change’ default Windows terminal application. And ConEmu can do it!

**WARNING**! Enabling ‘Default terminal’ feature may cause false alarms in some antiviral programs!

## Inspired by questions ##

  * [How to change the default terminal emulator on Windows](http://superuser.com/q/509642/139371)
  * [Use custom console for Visual Studio console application debugging](http://stackoverflow.com/q/12602411/1405560)
  * [C# debug log console app](http://stackoverflow.com/q/19599107/1405560)

ConEmu build 121124 introduced unique feature: ‘Replace default Windows terminal’.

# Description #

<img src='http://conemu-maximus5.googlecode.com/svn/files/Settings-DefTerm.png' alt='ConEmu settings, DefTerm page' title='ConEmu settings, DefTerm page'>

Just turn on checkbox ‘Force ConEmu as default terminal for console applications’,<br>
optionally turn on checkbox ‘Register on OS startup’<br>
and point ‘parent’ applications from you like to start console tools (for example <code>explorer.exe|devenv.exe|totalcmd.exe</code>)<br>
and voila. You may press <b>Win</b>+<b>R</b> type <code>ipconfig /all</code> and press <b>Enter</b>.<br>
It will starts in the new ConEmu tab.<br>
<br>
When you run console application from specified ‘parent’ application,<br>
your console application will be started in the new ConEmu tab.<br>
<br>
The only exception - consoles started ‘As administrator’ can't be hooked yet.<br>
<br>
<b>Note</b>! To ensure that your console application will be seized, run ConEmu <b>before</b>
starting your consoles or debugging processes (from Visual Studio for example).<br>
However, that may be not required if you choose ‘Register on OS startup’.<br>
But in fact, ‘registering’ will affect, in most cases, only <code>explorer.exe</code> ‘parent’ process.<br>
So, recommendation is ‘run ConEmu before...’<br>
<br>
‘<a href='SettingsDefTerm.md'>Default terminal</a>’ feature is more powerful than<br>
‘Automatic attach of cmd’ (‘<a href='SettingsComspec.md'>ComSpec</a>’ page).<br>
‘Default terminal’ can seize starting of <b>any</b> console program, not only cmd.exe.<br>
For example, you may dblclick on <code>ipconfig.exe</code> in Total Commander panel and voila...<br>
Also there will be no flickering of real console window.<br>
<br>
Settings page wiki: <a href='DefaultTerminal.md'>Default term</a>.<br>
<br>
<br>
<h1>Tech info</h1>

The only real way to ‘change’ default Windows terminal:<br>
<br>
<ul><li>Hide RealConsole when your console application is to be started;<br>
</li><li>Run new ConEmu window if no one instance was running before;<br>
</li><li>Attach started console application into ConEmu instance.</li></ul>

Looks simple, right? But who can perform these actions?<br>
<br>
<h2>Hooks must be installed</h2>

ConEmuHk (<code>ConEmuHk.dll</code> or <code>ConEmuHk64.dll</code>) must be loaded into process space of<br>
‘parent’ application <b>from</b> which you are starting new console application:<br>
Explorer (explorer.exe), Task manager (taskmgr.exe), Visual Studio (devenv.exe or WDExpress.exe), and so on...<br>
<br>
When ConEmuHk is loaded into ‘parent’ application it will hook limited set of Windows API functions<br>
related to creating new processes. So, when you start console application from shortcut, <b>Win</b>+<b>R</b> or even from<br>
Visual Studio debugger, ConEmuHk will change application startup procedure to the described above.<br>
<br>
Actually, startup procedure slightly differs when you start debugging session.<br>
<br>
<h2>Normal console application startup</h2>

To avoid flickering of RealConsole window ConEmuHk will change command line.<br>
<br>
When you run "cmd" from <b>Win</b>+<b>R</b> and no ConEmu instance was runned:<br>
<pre><code>ConEmu.exe /single /cmd -new_console:n "C:\Windows\system32\cmd.exe" <br>
</code></pre>

And when ConEmu window already exists:<br>
<pre><code>ConEmuC64.exe [many internal switched] /HIDE /NOCONFIRM /ROOT "C:\Windows\system32\cmd.exe" <br>
</code></pre>

That is safe because ‘parent’ does not really need to get started process handle (<code>cmd.exe</code> in example).<br>
<br>
<h2>Starting debugging session</h2>

When you start Visual Studio debugger of your ‘<code>ConAppSample</code>’, ‘parent’ process<br>
(devenv.exe or WDExpress.exe) need to get the handle of your ‘<code>ConAppSample.exe</code>’<br>
to be able to call debugging routines.<br>
<br>
That is why RealConsole window will flickers on screen before alternative attach procedure completes.<br>
<br>
Note, when you are debugging .Net console application, Visual Studio will run ‘<code>ConAppSample.vshost.exe</code>’,<br>
if option ‘Enable the Visual Studio hosting process’ is checked in your project Properties\Debug.