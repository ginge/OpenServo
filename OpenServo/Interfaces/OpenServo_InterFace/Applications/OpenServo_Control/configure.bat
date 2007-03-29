::
:: $Id$
::
@echo off

set QTDIR=c:\qt-3
set PATH=\Mingw\bin;%PATH%

set PATH=%QTDIR%\bin;%PATH%

@echo off
echo Setting QT Spec
set QMAKESPEC=win32-g++
