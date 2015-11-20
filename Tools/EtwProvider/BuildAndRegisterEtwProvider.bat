@echo off
REM Check whether we're running under admin first
REM Etw provider registration requires admin rights

net session >nul 2>&1
if %errorLevel% NEQ 0 (
	echo This script must be run as Administrator
	goto end
)

call "%VS120COMNTOOLS%vsvars32.bat"
msbuild "%~dp0EtwProvider.sln" /t:Rebuild /verbosity:minimal

set EtwProviderPath=%~dp0..\..\build\bin\Tools\EtwProvider
pushd %EtwProviderPath%
setx WhatIsMyIP_ETW_Provider_Path "%CD%"
popd

wevtutil.exe um "%~dp0EtwProvider.man"
wevtutil.exe im "%~dp0EtwProvider.man"
if %errorLevel% NEQ 0 (
	echo Failed to register EtwProvider
	goto end
)

echo Etw provider registration complete
:end
pause