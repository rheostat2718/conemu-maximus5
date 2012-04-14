if "%~1"=="" goto :EOF
"%~dp0timex"
echo Starting %~1>>times
"%~dp0timex">>times
dir /s c:\windows
"%~dp0timex"
"%~dp0timex">>times
echo .>>times
