=ConEmu has moved to conemu.github.io=

New versions of this file will be available on [conemu.github.io](http://conemu.github.io/en/Tabs.html).

# Windows console with tabs #
ConEmu may visualize with tabs all opened consoles, GUI applications (PuTTY for example) and Far Manager windows (panels, editors, viewers). ConEmu can handle up to 30 consoles and much more Far Manager editor (viewer) windows.

<img src='http://conemu-maximus5.googlecode.com/svn/files/ConEmuTabs2.png' alt='ConEmu tabs' title='ConEmu tabs and multiconsole'>

Tab captions may be changed<br>
<ul><li>with templates on <a href='http://code.google.com/p/conemu-maximus5/wiki/Settings#Tabs'>Tabs page</a> of Settings dialog<br>
</li><li>manually from tab context menu or customizable hotkey (Apps+R by default)</li></ul>

Initially Far Manager Viewer windows are framed by square brackets. Editor windows are framed by curly brackets. Modified editors marked with asterisk. Asterisk immediately appears and disappears on editor status change. You may change tabs appearence with <a href='http://code.google.com/p/conemu-maximus5/wiki/Settings#Tabs'>templates</a>.<br>
<br>
<b>Note: ConEmu plugin is required to get tabs for Far Manager editors and viewers.</b>

Left click on a tab will activate it, except Far Manager can't switch to this window (dialog is opened, macro is recorded or played, Far Manager menu bar is active, and so on).<br>
<br>
Right click on a tab will activate it (when it is possible) and (on succeeded activation) immediately close this tab. Far Manager windows will be closed by sending key sequence (F10 by default). Key sequence may be changed via registry value TabCloseMacro. Right click on a tab, containing other console applications (cmd, powershell, etc.) brings Recreate dialog.<br>
<br>
Consoles, started under Administrator (look at Run as administrator checkbox), are marked with <i>Shield</i> icon. If You want, ConEmu may show " (Admin)" suffix instead of icon. You can change suffix text or completely remove it on <a href='http://code.google.com/p/conemu-maximus5/wiki/Settings#Tabs'>Tabs page</a> of Settings dialog.<br>
<br>
Also, you may strip superfluous "Administrator:" prefix from console title <a href='http://code.google.com/p/conemu-maximus5/wiki/Settings#Tabs'>ibid</a>.<br>
<br>
<h4>Switching between consoles</h4>
You may use <code>Win-Number</code> to activate consoles:<br>
<ul><li>When console count less then 11'th: Win+1 activates first console, Win+9 - ninth, Win+0 - tenth.<br>
</li><li>Otherwise - one-number and two-number activations are allowed. Example:<br>
</li><li>‘Win(down) 1 5’ - activates 15'th console;<br>
</li><li>‘Win(down) 1 Win(up)’ - activates 1'th console.</li></ul>

Win is default <a href='ConEmuTerms#Host_key.md'>HostKey</a>, You may select yours convinent combination (up to 3 keys) from Win, Ctrl, Alt, Shift and Apps.<br>
<br>
By default, ConEmu <a href='Settings#Tabs.md'>internally handles Ctrl+Tab</a> combination, which switch consoles too.<br>
<br>
Also, you may use Win+Tab to switch consoles (check box on <a href='Settings.md'>Keys</a> page).