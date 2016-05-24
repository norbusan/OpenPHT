@echo off
setlocal

if "%WORKSPACE%"=="" (
	rem if we don't have a workspace let's just assume we are being executed from plex\scripts
	set WORKSPACE="%~dp0..\.."
)

rem start by downloading what we need.
cd %WORKSPACE%\project\BuildDependencies
call DownloadBuildDeps.bat || exit /b %ERRORLEVEL%
call DownloadMingwBuildEnv.bat || exit /b %ERRORLEVEL%
cd %WORKSPACE%\project\Win32BuildSetup
set NOPROMPT=1
call %WORKSPACE%\plex\scripts\buildmingwlibs.bat || exit /b %ERRORLEVEL%

rd /s /q %WORKSPACE%\upload
md %WORKSPACE%\upload

for /r %WORKSPACE%\system %%d in (*.dll) do (
	rem %WORKSPACE%\plex\scripts\WindowsSign.cmd %%d || exit /b %ERRORLEVEL%
)

..\..\project\BuildDependencies\msys\bin\sh --login /xbmc/plex/scripts/packdeps.sh || exit /b %ERRORLEVEL%

endlocal
exit /b 0
