=The detector=
ConEmu has ability to highlight hyperlinks, files and compiler errors (file+line\_number).
Just hover mouse with ‘Ctrl’ pressed.
May be configured on [Highlight](SettingsHighlight.md) settings page.

<img src='http://conemu-maximus5.googlecode.com/svn/files/ConEmuFileLineHL.png' title='ConEmu FileLineDetector'>

<h2>Compiler errors</h2>
Examples:<br>
<br>
<ul><li>macro.cpp(1820) : error C4716<br>
</li><li>NewClass.ps1:35 ...<br>
</li><li>abc.py (3): ...</li></ul>

<b>Note!</b> File will not be highlighted if ConEmu can't find it in the <a href='ShellWorkDir.md'>shell's current directory</a>.<br>
<br>
Action: File will be opened in the <a href='SettingsHighlight.md'>specified editor</a> on the proper line if possible.<br>
<br>
<h2>Files</h2>
File names or full paths produced from ‘dir’, ‘ls’, ‘git status’, ‘git diff’ and so on.<br>
<br>
<b>Note!</b> File will not be highlighted if ConEmu can't find it in the <a href='ShellWorkDir.md'>shell's current directory</a>.<br>
<br>
Action: will be opened in the default program (<a href='http://msdn.microsoft.com/en-us/library/windows/desktop/bb762153(v=vs.85).aspx'>ShellExecute</a>).<br>
<br>
<h2>Hyperlinks</h2>
Examples:<br>
<br>
<ul><li><a href='http://www.fosshub.com/ConEmu.html'>http://www.fosshub.com/ConEmu.html</a>
</li><li><a href='mailto:user@domain.com'>mailto:user@domain.com</a>
</li><li><a href='file:///c:\\sources\\abc.html'>file:///c:\\sources\\abc.html</a></li></ul>

Action: will be opened in the default program (<a href='http://msdn.microsoft.com/en-us/library/windows/desktop/bb762153(v=vs.85).aspx'>ShellExecute</a>).