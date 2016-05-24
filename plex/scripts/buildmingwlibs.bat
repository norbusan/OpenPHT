rem batch file to compile mingw libs via BuildSetup

rem set M$ env
call "%VS110COMNTOOLS%vsvars32.bat" || exit /b 1

rem check for mingw env
IF EXIST ..\..\project\BuildDependencies\msys\bin\sh.exe (
  rem compiles a bunch of mingw libs and not more
  echo bla>..\..\project\Win32BuildSetup\noprompt
  ..\..\project\BuildDependencies\msys\bin\sh --login /xbmc/plex/scripts/buildmingwlibs.sh
) ELSE (
  ECHO bla>errormingw
  ECHO mingw environment not found
)  
