ConEmu has several required files (`exe` and `dll`) for normal operation.

This page describes possible variants of files layouts.

# Standard structure #
| `.\ConEmu.exe` <br> <code>.\ConEmu64.exe</code> <table><thead><th> Main ConEmu files (32bit and 64bit), usually located in <code>C:\Program Files\ConEmu</code>.<br> Far Manager users might install ConEmu to the folder, where <code>far.exe</code> is located.<br> In that case, default shell will be <code>far.exe</code>. </th></thead><tbody>
<tr><td> <code>.\ConEmu\ConEmuC.exe</code> <br> <code>.\ConEmu\ConEmuCD.dll</code> <br> <code>.\ConEmu\ConEmuHk.dll</code> </td><td> Required 32bit console part of ConEmu package.<br> These files are required in 64bit OS`s too, if you want to<br> run any 32bit application in ConEmu.                                                                                                       </td></tr>
<tr><td> <code>.\ConEmu\ConEmuC64.exe</code> <br> <code>.\ConEmu\ConEmuCD64.dll</code> <br> <code>.\ConEmu\ConEmuHk64.dll</code> </td><td> Required 64bit console part of ConEmu package. These files are required in 64bit OS`s only.                                                                                                                                                                  </td></tr>
<tr><td> <code>.\Plugins\ConEmu\</code>                  </td><td> This folder contains Far Manager plugins. If you are not using Far Manager, you may delete <code>Plugins</code> folder                                                                                                                                       </td></tr></tbody></table>

<h1>Plain structure</h1>
<table><thead><th> <code>.\ConEmu.exe</code> <br> <code>.\ConEmu64.exe</code> <br> <code>.\ConEmuC.exe</code> <br> <code>.\ConEmuCD.dll</code> <br> <code>.\ConEmuHk.dll</code> <br> <code>.\ConEmuC64.exe</code> <br> <code>.\ConEmuCD64.dll</code> <br> <code>.\ConEmuHk64.dll</code> </th><th> If you wish, you may put all ConEmu files in one folder.<br> But! You must write post-update command to move<br> console-part files to root folder,<br> if you want to use ‘Autoupdate’ feature. </th></thead><tbody></tbody></table>

<h1>MinGW structure</h1>
<table><thead><th> <code>.\bin\ConEmu.exe</code> <br> <code>.\bin\ConEmu64.exe</code> </th><th> MinGW <code>bin</code> folder is located usually in <code>C:\MinGW\bin</code>. </th></thead><tbody>
<tr><td> <code>.\libexec\conemu\ConEmuC.exe</code> <br> <code>.\libexec\conemu\ConEmuCD.dll</code> <br> <code>.\libexec\conemu\ConEmuHk.dll</code> <br> <code>.\libexec\conemu\ConEmuC64.exe</code> <br> <code>.\libexec\conemu\ConEmuCD64.dll</code> <br> <code>.\libexec\conemu\ConEmuHk64.dll</code> </td><td> Console-part files located here.                                               </td></tr>
<tr><td> <code>.\libexec\conemu\ConEmu.xml</code>                           </td><td> Configuration file located here.                                               </td></tr>
<tr><td> <code>.\msys\1.0\bin\sh.exe</code>                                 </td><td> Default shell for this mode (MinGW).                                           </td></tr>
<tr><td> <code>.\bin\sh.exe</code>                                          </td><td> Alternative default shell for this mode (GIT).                                 </td></tr>