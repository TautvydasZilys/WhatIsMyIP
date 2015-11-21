@echo off

if defined VS140COMNTOOLS (
    call "%VS140COMNTOOLS%\vsvars32.bat"
) else (
    call "%VS120COMNTOOLS%\vsvars32.bat"
)

msbuild "%~dp0Tools\GenerateVcxItems\GenerateVcxItems.sln" /p:Configuration=Release /verbosity:minimal
call "%~dp0build\bin\Tools\GenerateVcxItems\Win32\Release\GenerateVcxItems.exe" "WhatIsMyIP"
if %errorlevel% NEQ 0 (
	echo Failed generating source files project.
	goto end
)

echo Successfully generated source files project.

if not defined WhatIsMyIP_ETW_Provider_Path (
	set RegisterProviderScript=\"%~dp0Tools\EtwProvider\BuildAndRegisterEtwProvider.bat\"
	call powershell.exe -executionpolicy bypass -command "start-process -verb 'runas' -filepath %RegisterProviderScript% -wait"
	
	if %errorlevel% NEQ 0 (
		echo Failed to register EtwProvider.
		goto end
	)
	
	echo Successfully registered EtwProvider.
)

echo Finished preparing workspace.

:end
pause