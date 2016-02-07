@echo off
if "%WORKSPACE%"=="" (
	rem if we don't have a workspace let's just assume we are being executed from plex\scripts
	set WORKSPACE="%~dp0..\.."
)

echo Downloading deps
%WORKSPACE%\project\BuildDependencies\bin\wget.exe -nv --no-check-certificate -O %WORKSPACE%\plex\Dependencies\windows-i386-xbmc-deps.tar.gz http://sources.openpht.tv/plex-dependencies/windows-i386-xbmc-deps.tar.gz
echo Unpacking deps
%WORKSPACE%\plex\scripts\tar.exe -C %WORKSPACE% -xzf %WORKSPACE%\plex\Dependencies\windows-i386-xbmc-deps.tar.gz

set DEPENDDIR="%WORKSPACE%\plex\build\dependencies"

if not exist %DEPENDDIR%\vcredist\2012 mkdir %DEPENDDIR%\vcredist\2012
if not exist %DEPENDDIR%\vcredist\2012\vcredist_x86.exe (
  echo Downloading vc110 redist...
  %WORKSPACE%\Project\BuildDependencies\bin\wget -nv --no-check-certificate -O %DEPENDDIR%\vcredist\2012\vcredist_x86.exe http://sources.openpht.tv/plex-dependencies/vcredist/2012/vcredist_x86.exe
)

if not exist %DEPENDDIR%\dxsetup mkdir %DEPENDDIR%\dxsetup
for %%f in (DSETUP.dll dsetup32.dll dxdllreg_x86.cab DXSETUP.exe dxupdate.cab Jun2010_D3DCompiler_43_x86.cab Jun2010_d3dx9_43_x86.cab) do (
  if not exist %DEPENDDIR%\dxsetup\%%f (
    %WORKSPACE%\Project\BuildDependencies\bin\wget -nv -O %DEPENDDIR%\dxsetup\%%f http://mirrors.xbmc.org/build-deps/win32/dxsetup/%%f
  )
)