@echo off
setlocal

if "%WORKSPACE%"=="" (
  exit /b 0
)

set TargetFile=%~1

signtool.exe sign /a /sm /fd sha1 /t http://timestamp.comodoca.com "%TargetFile%"
if "%errorlevel%" == 1 (
  echo FATAL ERROR - could not sign %TargetFile%
  exit /b 1
)

signtool.exe sign /a /sm /as /fd sha256 /tr http://timestamp.comodoca.com /td sha256 "%TargetFile%"
if "%errorlevel%" == 1 (
  echo FATAL ERROR - could not sign %TargetFile%
  exit /b 1
)

signtool.exe verify /all /pa "%TargetFile%"
if "%errorlevel%" == 1 (
  echo FATAL ERROR - could not verify signature for %TargetFile%
  exit /b 1
)

endlocal
exit /b 0
