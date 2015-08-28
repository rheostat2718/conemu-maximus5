#summary This page was generated automatically from ConEmu sources
<a href='Hidden comment:  IDD_SPG_COMSPEC '></a>
# Settings: ComSpec #
<img src='http://conemu-maximus5.googlecode.com/svn/files/Settings-Comspec.png' title='ConEmu Settings: ComSpec'>



<h2>Used command processor</h2>



<ul><li><b>Auto (tcc - if installed, %ComSpec% - otherwise)</b> Start tcc - if installed, %ComSpec% - otherwise (used for ‘Create new ‘cmd.exe’ console’)<br>
</li><li><b>%ComSpec% (env.var.)</b> Use %ComSpec% environment variable (used for ‘Create new ‘cmd.exe’ console’)<br>
</li><li><b>cmd</b> Use cmd.exe only (used for ‘Create new ‘cmd.exe’ console’)<br>
</li><li><b>Explicit executable (env.var. of ConEmu.exe allowed)</b> Specified command processor (used for ‘Create new ‘cmd.exe’ console’)</li></ul>






<h2>Choose preferred command processor platform (bits)</h2>



<ul><li><b>Same as OS</b> 64bit OS only, (System32 or SysWOW64) - use System32<br>
</li><li><b>Same as running application</b> 64bit OS only, (System32 or SysWOW64) - same bits as application (ConEmu, Far Manager, ...)<br>
</li><li><b>x32 always</b> 64bit OS only, (System32 or SysWOW64) - use SysWOW64</li></ul>




<b>Test</b> Show ‘calculated’ ComSpec path<br>
<br>
<b>Set ComSpec environment variable for child processes to selected value</b> ConEmu may update %ComSpec% variable to selected command processor<br>
<br>
<b>Cmd.exe output codepage</b> Windows command processor (cmd.exe) may cause then output of internal commands to be OEM or Unicode. You may force this selection, or use automatic selection (FAR2 -<code>&gt;</code> Unicode).<br>
<br>
<b>Support UNC paths in cmd.exe (\\server\share\folder)</b>

<h2>Add to %PATH% environment variable</h2>

<b>Add %ConEmuDir% to %PATH%</b>

<b>Add %ConEmuBaseDir% to %PATH%</b>





<h2>Choose preferred command processor platform (bits)</h2>





<h2>Add to %PATH% environment variable</h2>





<h2>Automatic attach of cmd & Tcc/Le to ConEmu (new window will be started if not found)</h2>



<b>[HKEY_CURRENT_USER\Software\Microsoft\Command Processor] Current command stored in registry ‘AutoRun’ (HKLM value is not processed here)</b>

<b>Register ConEmu autorun</b>

<b>Unregister</b>

<b>Clear</b>

<b>Force new ConEmu window</b>



