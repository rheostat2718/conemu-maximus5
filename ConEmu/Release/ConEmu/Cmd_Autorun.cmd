@echo off

rem This (Cmd_Autorun /i) will install to the registry following keys
rem   [HKEY_CURRENT_USER\Software\Microsoft\Command Processor]
rem   "AutoRun"="\"C:\\Program Files\\FAR\\cmd_autorun.cmd\""
rem Which cause auto attaching to ConEmu each started cmd.exe or tcc.exe
rem If ConEmu (GUI) was not started yes, new instance will be started.


if "%1"=="" goto noparm
goto checkparm

:noparm
rem �⮡� �� ࠧ�ࠦ��� ��譨� ⥪�⮬, ����� ConEmu 㦥 ����饭�...
rem ⠪ ��।����� �� ᮢᥬ ���४⭮. cmd ����� �������� �� ConEmu
rem � ०��� ᮧ����� ����� ���᮫�... �� �� �����, ���� ⠪...
if "%ConEmuHWND%"=="" goto noconemu
goto conemufound


:checkparm
if "%1"=="/?" goto help
if "%1"=="-?" goto help

if "%1"=="/i" goto install
if "%1"=="-i" goto install

if "%1"=="/u" goto uninstall
if "%1"=="-u" goto uninstall

rem ��� ⥫��⮬ - �� ����᪠��!
if not "%TERM%"=="" goto termfound

rem �஢����, ����� ConEmu 㦥 ����饭�?
rem Warning!!! �� �� ࠡ�⠥�, �᫨ � ConMan(!) 㪠���
rem "CreateInNewEnvironment"=dword:00000001
rem �㦭� ⠪:
rem REGEDIT4
rem [HKEY_CURRENT_USER\Software\HoopoePG_2x]
rem "CreateInNewEnvironment"=dword:00000000

:noconemu
set ConEmuPath=%~dp0ConEmuC.exe
if not exist "%ConEmuPath%" set ConEmuPath=ConEmuC.exe
if not exist "%ConEmuPath%" goto notfound
echo ConEmu autorun (c) Maximus5
echo Starting "%ConEmuPath%" in "Attach" mode
call "%ConEmuPath%" /ATTACH /NOCMD
echo Done
goto fin

:notfound
Echo ConEmu not found! "%~dp0ConEmuC.exe"
goto fin
:gui_notfound
Echo ConEmu not found! "%~dp0..\ConEmu.exe"
goto fin
:termfound
Echo ConEmu can not be started under Telnet!
goto fin
:conemufound
rem Echo ConEmu already started (%ConEmuHWND%)
goto fin

:help
echo ConEmu autorun (c) Maximus5
echo Usage
echo    cmd_autorun.cmd /?  - displays this help
echo    cmd_autorun.cmd /i  - turn ON ConEmu autorun for cmd.exe
echo    cmd_autorun.cmd /u  - turn OFF ConEmu autorun
goto fin

:install
set ConEmuPath=%~dp0..\ConEmu.exe
if exist "%ConEmuPath%" goto install_found
set ConEmuPath=%~dp0ConEmu.exe
if exist "%ConEmuPath%" goto install_found
set ConEmuPath=%~dp0..\ConEmu64.exe
if exist "%ConEmuPath%" goto install_found
set ConEmuPath=%~dp0ConEmu64.exe
if exist "%ConEmuPath%" goto install_found
goto gui_notfound
:install_found
echo ConEmu autorun (c) Maximus5
echo Enabling autorun
call "%ConEmuPath%" /autosetup 1 "%~0"
goto CheckRet

:uninstall
set ConEmuPath=%~dp0..\ConEmu.exe
if exist "%ConEmuPath%" goto uninstall_found
set ConEmuPath=%~dp0ConEmu.exe
if exist "%ConEmuPath%" goto uninstall_found
set ConEmuPath=%~dp0..\ConEmu64.exe
if exist "%ConEmuPath%" goto uninstall_found
set ConEmuPath=%~dp0ConEmu64.exe
if exist "%ConEmuPath%" goto uninstall_found
goto gui_notfound
:uninstall_found
echo ConEmu autorun (c) Maximus5
echo Disabling autorun
call "%ConEmuPath%" /autosetup 0
goto CheckRet


:CheckRet
if errorlevel 2 echo ConEmu failed
if errorlevel 1 echo All done
goto fin

:fin
