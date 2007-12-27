rem c:\qt-3\setenv.bat
rem c:\qt-3\setpath.bat
copy debug\OSIFdll.DLL .
gcc -o testapp.exe testapp.c
