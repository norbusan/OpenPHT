@ECHO ON

SET LOC_PATH=%BUILD_DEPS_PATH%\scripts
SET FILES=%LOC_PATH%\win32depends_d.txt

CALL dlextract.bat win32depends %FILES%
IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

cd %TMP_PATH%

xcopy plex-depends\include\* "%BUILD_DEPS_PATH%\include" /E /Q /I /Y
xcopy plex-depends\lib\*.* "%BUILD_DEPS_PATH%\lib" /E /Q /I /Y
xcopy plex-depends\bin\*.pdb "%BUILD_DEPS_PATH%\lib" /E /Q /I /Y
copy plex-depends\bin\dump_syms.exe "%BUILD_DEPS_PATH%\bin\" /Y
copy plex-depends\bin\dnssd.dll "%APP_PATH%\system\" /Y
copy plex-depends\bin\libcec.dll "%APP_PATH%\system\" /Y
copy plex-depends\bin\libcurl.dll "%APP_PATH%\system\" /Y
copy plex-depends\bin\libeay32.dll "%APP_PATH%\system\" /Y
copy plex-depends\bin\libmicrohttpd-dll.dll "%APP_PATH%\system\webserver\" /Y
copy plex-depends\bin\libogg.dll "%APP_PATH%\system\cdrip\" /Y
copy plex-depends\bin\libshairplay-1.dll "%APP_PATH%\system\airplay\" /Y
copy plex-depends\bin\libvorbis.dll "%APP_PATH%\system\cdrip\" /Y
copy plex-depends\bin\libvorbisfile.dll "%APP_PATH%\system\players\paplayer\" /Y
copy plex-depends\bin\sqlite3.dll "%APP_PATH%\system\" /Y
copy plex-depends\bin\ssleay32.dll "%APP_PATH%\system\" /Y
copy plex-depends\bin\zlib1.dll "%APP_PATH%\system\" /Y
copy plex-depends\bin\zlib1.dll "%APP_PATH%\project\Win32BuildSetup\dependencies\" /Y

cd %LOC_PATH%
