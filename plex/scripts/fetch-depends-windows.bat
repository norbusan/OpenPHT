@echo off
setlocal

if "%WORKSPACE%"=="" (
  rem if we don't have a workspace let's just assume we are being executed from plex\scripts
  set WORKSPACE="%~dp0..\.."
)

if not exist %WORKSPACE%\plex\Dependencies\windows-i386-xbmc-vc120.tar.gz (
  echo Downloading deps...
  %WORKSPACE%\project\BuildDependencies\bin\wget.exe -nv --no-check-certificate -O %WORKSPACE%\plex\Dependencies\windows-i386-xbmc-vc120.tar.gz http://sources.openpht.tv/plex-dependencies/windows-i386-xbmc-vc120.tar.gz || exit /b 1
)
echo Unpacking deps...
%WORKSPACE%\plex\scripts\tar.exe -C %WORKSPACE% -xzf %WORKSPACE%\plex\Dependencies\windows-i386-xbmc-vc120.tar.gz || exit /b 2

set DEPENDDIR="%WORKSPACE%\plex\build\dependencies"

if not exist %DEPENDDIR%\vcredist\2013 mkdir %DEPENDDIR%\vcredist\2013
if not exist %DEPENDDIR%\vcredist\2013\vcredist_x86.exe (
  echo Downloading vc120 redist...
  %WORKSPACE%\Project\BuildDependencies\bin\wget -nv --no-check-certificate -O %DEPENDDIR%\vcredist\2013\vcredist_x86.exe http://sources.openpht.tv/plex-dependencies/vcredist/2013/vcredist_x86.exe || exit /b 1
)

if not exist %DEPENDDIR%\dxsetup mkdir %DEPENDDIR%\dxsetup
for %%f in (DSETUP.dll dsetup32.dll dxdllreg_x86.cab DXSETUP.exe dxupdate.cab Jun2010_D3DCompiler_43_x86.cab Jun2010_d3dx9_43_x86.cab) do (
  if not exist %DEPENDDIR%\dxsetup\%%f (
    echo Downloading %%f...
    %WORKSPACE%\Project\BuildDependencies\bin\wget -nv --no-check-certificate -O %DEPENDDIR%\dxsetup\%%f http://sources.openpht.tv/build-deps/win32/dxsetup/%%f || exit /b 1
  )
)

endlocal
exit /b 0
