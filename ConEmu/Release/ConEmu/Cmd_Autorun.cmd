@echo off

rem This (Cmd_Autorun /i) will install to the registry following keys
rem   [HKEY_CURRENT_USER\Software\Microsoft\Command Processor]
rem   "AutoRun"="\"C:\\Program Files\\FAR\\cmd_autorun.cmd\""
rem Which cause auto attaching to ConEmu each started cmd.exe or tcc.exe
rem If ConEmu (GUI) was not started yes, new instance will be started.

set FORCE_NEW_WND=NO
set FORCE_NEW_WND_CMD=

if "%~1"=="" goto noparm
goto checkparm

:noparm
rem �⮡� �� ࠧ�ࠦ��� ��譨� ⥪�⮬, ����� ConEmu 㦥 ����饭�...
rem ⠪ ��।����� �� ᮢᥬ ���४⭮. cmd ����� �������� �� ConEmu
rem � ०��� ᮧ����� ����� ���᮫�... �� �� �����, ���� ⠪...
if "%ConEmuHWND%"=="" goto noconemu
rem TODO: IsConEmu
goto conemufound


:checkparm
if "%~1"=="/?" goto help
if "%~1"=="-?" goto help
if "%~1"=="--?" goto help
if "%~1"=="-help" goto help
if "%~1"=="--help" goto help

if "%~1"=="/i" goto install
if "%~1"=="-i" goto install

if "%~1"=="/u" goto uninstall
if "%~1"=="-u" goto uninstall

call :check_new "%~1"

rem Memorize...
rem "TERM" variable may be defined in OS.
rem rem ��� ⥫��⮬ - �� ����᪠��!
rem rem TODO: IsTelnet
rem if not "%TERM%"=="" goto termfound

rem Memorize...
rem �஢����, ����� ConEmu 㦥 ����饭�?
rem Warning!!! �� �� ࠡ�⠥�, �᫨ � ConMan(!) 㪠���
rem "CreateInNewEnvironment"=dword:00000001
rem �㦭� ⠪:
rem REGEDIT4
rem [HKEY_CURRENT_USER\Software\HoopoePG_2x]
rem "CreateInNewEnvironment"=dword:00000000

:noconemu
if defined PROCESSOR_ARCHITEW6432 goto x64
if "%PROCESSOR_ARCHITECTURE%"=="AMD64" goto x64
goto x86
:x64
set ConEmuC1=%~dp0ConEmuC64.exe
set ConEmuC2=%~dp0ConEmuC.exe
set ConEmuC3=ConEmuC64.exe
goto con_find
:x86
set ConEmuC1=%~dp0ConEmuC.exe
set ConEmuC2=%~dp0ConEmuC.exe
set ConEmuC3=ConEmuC.exe
goto con_find
:con_find
set ConEmuPath=%ConEmuC1%
if exist "%ConEmuPath%" goto con_found
set ConEmuPath=%ConEmuC2%
if exist "%ConEmuPath%" goto con_found
set ConEmuPath=%ConEmuC3%
if exist "%ConEmuPath%" goto con_found
goto notfound
:con_found
rem Message moved to ConEmuC.exe
rem echo ConEmu autorun (c) Maximus5
rem echo Starting "%ConEmuPath%" in "Attach" mode (NewWnd=%FORCE_NEW_WND%)
call "%ConEmuPath%" /AUTOATTACH %FORCE_NEW_WND_CMD%
rem echo errorlevel=%errorlevel%
if errorlevel 132 if not errorlevel 133 goto rc_nocon
if errorlevel 121 if not errorlevel 122 goto rc_ok
echo ConEmu attach failed
goto fin
:rc_nocon
rem This occurs while VS build or run from telnet
rem echo ConEmu attach not available (no console window)
goto fin
:rc_ok
echo ConEmu attach succeeded
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
call :check_new "%~2"
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
echo Enabling autorun (NewWnd=%FORCE_NEW_WND%)
call "%ConEmuPath%" /autosetup 1 "%~0" %FORCE_NEW_WND_CMD%
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


:check_new
if "%~1"=="/GHWND=NEW" set FORCE_NEW_WND=YES
if "%~1"=="/NEW" set FORCE_NEW_WND=YES
if "%FORCE_NEW_WND%"=="YES" set FORCE_NEW_WND_CMD="/GHWND=NEW"
goto :EOF

:CheckRet
if errorlevel 2 echo ConEmu failed
if errorlevel 1 echo All done
goto fin

:fin