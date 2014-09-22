@echo off
call "%VS90COMNTOOLS%vsvars32.bat" x86
cd /d "%~dp0"

set PLATFORM=x86
set RELEASE=1

nmake /f makefile
