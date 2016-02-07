@ECHO ON

SET LOC_PATH=%CD%
SET FILES=%LOC_PATH%\gnutls_d.txt

CALL dlextract.bat gnutls %FILES%

cd %TMP_PATH%

copy gnutls-3.4.7-w32\bin\libgcc_s_sjlj-1.dll "%XBMC_PATH%\system\players\dvdplayer\" /Y
copy gnutls-3.4.7-w32\bin\libgmp-10.dll "%XBMC_PATH%\system\players\dvdplayer\" /Y
copy gnutls-3.4.7-w32\bin\libgnutls-30.dll "%XBMC_PATH%\system\players\dvdplayer\" /Y
copy gnutls-3.4.7-w32\bin\libhogweed-4-0.dll "%XBMC_PATH%\system\players\dvdplayer\" /Y
copy gnutls-3.4.7-w32\bin\libnettle-6-0.dll "%XBMC_PATH%\system\players\dvdplayer\" /Y
copy gnutls-3.4.7-w32\bin\libp11-kit-0.dll "%XBMC_PATH%\system\players\dvdplayer\" /Y

cd %LOC_PATH%
