@ECHO ON

SET LOC_PATH=%CD%
SET FILES=%LOC_PATH%\win32depends_d.txt

CALL dlextract.bat win32depends %FILES%

cd %TMP_PATH%

xcopy plex-depends\include\* "%CUR_PATH%\include" /E /Q /I /Y
xcopy plex-depends\lib\*.* "%CUR_PATH%\lib" /E /Q /I /Y
xcopy plex-depends\bin\*.pdb "%CUR_PATH%\lib" /E /Q /I /Y
copy plex-depends\bin\libcec.dll "%XBMC_PATH%\system\" /Y
copy plex-depends\bin\libcurl.dll "%XBMC_PATH%\system\" /Y
copy plex-depends\bin\libeay32.dll "%XBMC_PATH%\system\" /Y
copy plex-depends\bin\libogg.dll "%XBMC_PATH%\system\cdrip\" /Y
copy plex-depends\bin\libsamplerate-0.dll "%XBMC_PATH%\system\" /Y
copy plex-depends\bin\libshairplay-1.dll "%XBMC_PATH%\system\airplay\" /Y
copy plex-depends\bin\libvorbis.dll "%XBMC_PATH%\system\cdrip\" /Y
copy plex-depends\bin\libvorbisfile.dll "%XBMC_PATH%\system\players\paplayer\" /Y
copy plex-depends\bin\sqlite3.dll "%XBMC_PATH%\system\" /Y
copy plex-depends\bin\ssleay32.dll "%XBMC_PATH%\system\" /Y
copy plex-depends\bin\zlib1.dll "%XBMC_PATH%\system\" /Y
copy plex-depends\bin\zlib1.dll "%XBMC_PATH%\project\Win32BuildSetup\dependencies\" /Y

cd %LOC_PATH%
