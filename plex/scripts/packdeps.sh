cd /xbmc
files=$(find system -iname *.dll)
tar -czf /xbmc/upload/windows-i386-xbmc-vc120.tar.gz $files project/BuildDependencies/lib project/BuildDependencies/include project/Win32BuildSetup/dependencies project/BuildDependencies/bin/dump_syms.exe
