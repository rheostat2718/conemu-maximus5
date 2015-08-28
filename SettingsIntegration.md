=Integration=
<img src='http://conemu-maximus5.googlecode.com/svn/files/Settings-Integration.png' title='ConEmu Settings: Integration'>

You may register here several menu items, which will be shown in the Explorer popup menu<br>
(used in folder background, folder items, libraries).<br>
<br>
Note! Your items are registered in Windows registry, they can't be ‘portable’.<br>
<br>
ConEmu offers two types of integration, registered separately:<br>
<br>
<ul><li>‘ConEmu Here’ (upper group) - classic mode, your shell will be started in normal ConEmu window;<br>
</li><li>‘ConEmu Inside’ (lower group) - your shell will be started in the pane of your Explorer window.</li></ul>

<h2>ConEmu Inside</h2>
When you run one of ‘ConEmu inside’ items, ConEmu window will be started <b>inside</b>
your Explorer window.<br>
<br>
<img src='http://conemu-maximus5.googlecode.com/svn/files/ConEmuInside.png' title='ConEmu inside Windows Explorer pane'>

But it is yours responsibility (on most of OS versions) to provide some free space<br>
in your Explorer. For example, turn on ‘View page’ or ‘Details pane’ in Win8's<br>
before calling one of ConEmu Inside items.<br>
<br>
In that mode, ConEmu can automatically sync directory (change folder in Explorer window and<br>
ConEmu will automatically ‘cd’ to this folder in your shell). To enable this mode - click<br>
on the ‘Sync’ item in the ConEmu status bar.<br>
<br>
To pop up ConEmu system menu - right click on the leftmost part of the ConEmu status bar.<br>
<br>
To pop up ConEmu ‘Tab’ menu - right click on the <code>[+]</code> item in the ConEmu status bar.<br>
<br>
<h2>‘Integration’ page items description</h2>

<img src='http://conemu-maximus5.googlecode.com/svn/files/SettingsIntegration2.png' title='Integration page items description'>

<h3>Menu item</h3>
What you will see in the Explorer popup menu.<br>
You can use <code>&amp;</code> sign to set hotkey for the menu item.<br>
For example, register <code>ConEmu &amp;Here</code> and you can press Alt+H<br>
to run it from keyboard. Of course, you need to pop up menu<br>
first. There is no way to register ‘global’ shortcut for these methods.<br>
<br>
<h3>Configuration</h3>
Especially for Inside mode. Allows to set different font, palette and other settings for your ConEmu instance.<br>
<br>
<h3>Command</h3>
Specify your shell here (cmd, powershell, Far Manager, etc.)<br>
<br>
Use <code>%1</code> macro for ‘clicked’ item (this may be folder or file!)<br>
<br>
If you need to pass directory instead of right-clicked file, you can use ‘ConEmuWorkDir!’ macro, example:<br>
<pre><code>"C:\Program Files\Far Manager\Far.exe" "!ConEmuWorkDir!"<br>
</code></pre>
Want to run shell in the existing window instead of creating new one? Use ‘/single’ switch with following ‘/cmd’, example:<br>
<pre><code>/single /cmd powershell<br>
</code></pre>

<h3>Icon file</h3>
Examples: ‘C:\Far\far.exe,0’, ‘powershell.exe’, ‘C:\Images\Icon.ico’.<br>
<br>
<h3>Sync dir (Inside mode only)</h3>
Here you can choose special command for ‘cd’ in your shell.<br>
You may use following macro here<br>
<pre><code>\e - Esc keypress<br>
\b - BackSpace keypress<br>
\n - Enter keypress<br>
\1 - "dir", for example "C:\Users\Max"<br>
\2 - "bash dir", for example "/C/Users/Max"<br>
</code></pre>
The default is<br>
<pre><code>PowerShell:<br>
  \ecd \1\n<br>
Bash (bash.exe, sh.exe):<br>
  \e\bcd \2\n<br>
CMD and others:<br>
  \ecd /d \1\n<br>
</code></pre>