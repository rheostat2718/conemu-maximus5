=Unix like sudo command on Windows=

Common question: ‘Is there any 'sudo' command for Windows?’

One of the answers is here.

ConEmu package contains (started from build 121028) batch file
‘%ConEmuBaseDir%\[csudo.cmd](http://conemu-maximus5.googlecode.com/svn/trunk/ConEmu/Release/ConEmu/csudo.cmd)’.
When checkbox ‘Add %ConEmuBaseDir% to %PATH%’ is On (‘ComSpec’ page),
you may just type csudo in non-elevated prompt in your shell.

**Note** You may also rename ‘csudo.cmd’ to ‘sudo.cmd’ if you like simple ‘sudo’ notation.
And you must rename it if you like to change its contents to avoid loosing your changes
when new update of ConEmu arrives.

**Note** Elevated command will starts in new elevated ConEmu tab.

## Example ##
```
csudo dism /online /enable-feature /featurename:NetFX3 /All /Source:D:\sources\sxs /LimitAccess
```

## Screenshots ##
Let type in command prompt ‘csudo diskpart’

<img src='http://conemu-maximus5.googlecode.com/svn/files/ConEmuSuDo1.png' title='Running csudo command'>

Get UAC confirmation on Vista or later<br>
<br>
<img src='http://conemu-maximus5.googlecode.com/svn/files/ConEmuSuDo2.png' title='csudo UAC confirmation'>

Or choose another credentials on Windows 2000 or Windows XP<br>
<br>
<img src='http://conemu-maximus5.googlecode.com/svn/files/ConEmuSuDo4.png' title='csudo UAC confirmation'>

Here we are, diskpart started elevated in a split<br>
<br>
<img src='http://conemu-maximus5.googlecode.com/svn/files/ConEmuSuDo3.png' title='Running csudo command'>

<h2>Batch file contents</h2>
As you can see, batch contains two parameters, which you may redefine: ConEmuSplit and ConfirmClose.<br>
For example, if you want to start elevated command in new tab rather than in split - just<br>
change ‘<code>set ConEmuSplit=VERT</code>’ to ‘<code>set ConEmuSplit=NO</code>’.<br>
<br>
<b>csudo.cmd</b>

<pre><code>@echo off<br>
<br>
rem This is analogue for *nix "sudo" command<br>
rem You may rename this file to "sudo.cmd" or use it "as is"<br>
rem Example:<br>
rem csudo dism /online /enable-feature /featurename:NetFX3 /All /Source:D:\sources\sxs /LimitAccess<br>
<br>
setlocal<br>
<br>
rem Use split screen feature? Possible values: VERT, HORZ, NO<br>
set ConEmuSplit=VERT<br>
<br>
rem Show confirmation before closing SUDO tab<br>
rem You may set NO here, if confirmation is not needed<br>
set ConfirmClose=YES<br>
<br>
<br>
rem When possible - use Ansi Esc sequences to print errors<br>
rem Let set "ESC" variable to char with code \x1B<br>
set ESC=<br>
<br>
rem It is 64-bit OS?<br>
if not %PROCESSOR_ARCHITECTURE%==AMD64 goto x32<br>
<br>
rem First, try to use 64-bit ConEmuC<br>
if exist "%~dp0ConEmuC64.exe" (<br>
set ConEmuSrvPath=%~dp0ConEmuC64.exe<br>
goto run<br>
)<br>
<br>
:x32<br>
rem Let use 32-bit ConEmuC<br>
if exist "%~dp0ConEmuC.exe" (<br>
set ConEmuSrvPath=%~dp0ConEmuC.exe<br>
goto run<br>
)<br>
<br>
:not_found<br>
rem Oops, csudo located in wrong folder<br>
if %ConEmuANSI%==ON (<br>
echo %ESC%[1;31;40mFailed to find ConEmuC.exe or ConEmuC64.exe!%ESC%[0m<br>
) else (<br>
echo Failed to find ConEmuC.exe or ConEmuC64.exe<br>
)<br>
exit 100<br>
goto :EOF<br>
<br>
:run<br>
rem Preparing switches<br>
if %ConEmuSplit%==VERT (<br>
set SPLIT=sV<br>
) else if %ConEmuSplit%==HORZ (<br>
set SPLIT=sH<br>
) else (<br>
set SPLIT=<br>
)<br>
if %ConfirmClose%==NO (<br>
set ConEmuNewCon=-new_console:an%SPLIT%<br>
) else (<br>
set ConEmuNewCon=-new_console:ac%SPLIT%<br>
)<br>
<br>
if "%~1"=="" (<br>
rem There was no arguments, just start new ComSpec<br>
"%ConEmuSrvPath%" /c %ComSpec% %ConEmuNewCon%<br>
) else (<br>
rem Start requested command<br>
"%ConEmuSrvPath%" /c %* %ConEmuNewCon%<br>
)<br>
rem all done<br>
</code></pre>