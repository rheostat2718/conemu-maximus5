rem @echo off

set BUILD_RC=ERROR
if "%~3"=="" goto no_parms
set WIDE=1
cls
call "%VS90COMNTOOLS%vsvars32.bat" x86

rem Use:
rem vcexpress  [solutionfile | projectfile | anyfile.ext]  [switches]
rem 
rem Command line builds:
rem devenv solutionfile.sln /build [ solutionconfig ] [ /project projectnameorfile [ /projectconfig name ] ]
rem Available command line switches:
rem 
rem /Log	Logs IDE activity to the specified file for troubleshooting.

%~d0
cd "%~p0"
cd

set DEVEXE=.
if exist "%VS90COMNTOOLS%..\IDE\VCExpress.exe" set DEVEXE="%VS90COMNTOOLS%..\IDE\VCExpress.exe"
if exist "%VS90COMNTOOLS%..\IDE\devenv.exe" set DEVEXE="%VS90COMNTOOLS%..\IDE\devenv.exe"
if exist "%VS90COMNTOOLS%..\IDE\devenv.com" set DEVEXE="%VS90COMNTOOLS%..\IDE\devenv.com"
if %DEVEXE%==. goto nostudio

rem echo #pragma once > "%~dp0common\usetodo.hpp"
rem echo #define HIDE_TODO >> "%~dp0common\usetodo.hpp"

:LoopBuild
echo Building config: %3
if exist !Errors.log del !Errors.log
%DEVEXE% %2 %1 %3 /Out !Errors.log
if errorlevel 1 goto e1
rem if exist !Errors.log type !Errors.log
shift /3
if NOT "%~3"=="" goto LoopBuild
set BUILD_RC=OK

goto fin
:e1
set BUILD_RC=BUILD_ERRORS
:typeerr

rem echo #pragma once > "%~dp0common\usetodo.hpp"
rem echo //#define HIDE_TODO >> "%~dp0common\usetodo.hpp"

rem if exist !Errors.log type !Errors.log
goto fin
:nostudio
set BUILD_RC=VC_NOT_FOUND
Echo Visual Studio 9 Pro or Express not found!
goto fin
:no_parms
Echo Usage: "%~nx0" {/build ^| /rebuild} {sln_file} {config}
echo Exmpl: "%~nx0" /build ce.sln "Debug|Win32"
goto fin
:fin
