@echo off

set ConEmuHttp=http://code.google.com/p/conemu-maximus5/wiki/ConEmu

if "%~1"=="" goto noparm
cd /d "%~dp0"

set path=%~d0\Utils\Lans\WiX\bin;%PATH%

echo Preparing version...
GenVersion.exe %1

set CAB_NAME=ConEmu.cab

if "%~2"=="no_msi" goto no_msi

echo Creating x86 installer
set MSI_PLAT=x86
set MSI_NAME=ConEmu.%1.%MSI_PLAT%.msi
call :create_msi

echo .
echo Creating x64 installer
set MSI_PLAT=x64
set MSI_NAME=ConEmu.%1.%MSI_PLAT%.msi
call :create_msi

call "%~dp0..\..\..\ConEmu-key\sign_any.bat" /d "ConEmu %~1 Installer" /du %ConEmuHttp% %~dp0%CAB_NAME%

:no_msi

echo .
echo Creating ConEmuSetup.%1.exe

call "%~dp0Executor\gccbuild.cmd" /nosign
if errorlevel 1 goto errs
if not exist "%~dp0Setupper\Executor.exe" goto errs
call "%~dp0..\..\..\ConEmu-key\sign_any.bat" /d "ConEmu %~1 Installer" /du %ConEmuHttp% "%~dp0Setupper\Executor.exe"

call "%~dp0Setupper\gccbuild.cmd" /nosign
if errorlevel 1 goto errs
if not exist "%~dp0Setupper\Setupper.exe" goto errs

move "%~dp0Setupper\Setupper.exe" "%~dp0ConEmuSetup.%1.exe"
call "%~dp0..\..\..\ConEmu-key\sign_any.bat" /d "ConEmu %~1 Installer" /du %ConEmuHttp% "%~dp0ConEmuSetup.%1.exe"

goto fin

:create_msi
echo Compiling...
candle.exe -nologo -dConfiguration=Release -dOutDir=bin\Release\ -dPlatform=%MSI_PLAT% -dProjectDir=%~dp0 -dProjectExt=.wixproj -dProjectFileName=ConEmu.wixproj -dProjectName=ConEmu -dProjectPath=%~dp0ConEmu.wixproj -dTargetDir=%~dp0bin\Release\ -dTargetExt=.msi -dTargetFileName=%MSI_NAME% -dTargetName=ConEmu -dTargetPath=%~dp0bin\Release\%MSI_NAME% -out obj\Release\Product.wixobj -arch %MSI_PLAT% Product.wxs
if errorlevel 1 goto err
echo Linking...
Light.exe -nologo -dcl:high  -cultures:null -out %~dp0bin\Release\%MSI_NAME% -pdbout %~dp0bin\Release\ConEmu.wixpdb -ext WixUIExtension obj\Release\Product.wixobj
if errorlevel 1 goto err
echo Signing...
call "%~dp0..\..\..\ConEmu-key\sign_any.bat" /d "ConEmu %~1 Installer" /du %ConEmuHttp% %~dp0bin\Release\%MSI_NAME%
if errorlevel 1 goto err
echo Succeeded.

move %~dp0bin\Release\%MSI_NAME% %MSI_NAME%
move %~dp0bin\Release\%CAB_NAME% %CAB_NAME%
rd /S /Q %~dp0bin
rd /S /Q %~dp0obj

goto :EOF


:noparm
echo Usage:    Release.cmd ^<BuildNo^> [no_msi]
echo Example:  Release.cmd 090121
pause
goto fin

:err
echo Error, while creating msi!
pause
goto fin

:errs
echo Error, while creating setupper!
pause
goto fin

:fin
