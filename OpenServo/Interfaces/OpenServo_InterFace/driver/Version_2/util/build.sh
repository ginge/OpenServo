PYTHONVER=python2.4
INCLUDEDIR=`$PYTHONVER -c "import distutils.sysconfig; print distutils.sysconfig.get_python_inc()"`

swig -python -I/usr/include/python2.4 libusb.i

gcc -fPIC -Wall -fno-strict-aliasing -O2 -g  -shared  -I $INCLUDEDIR -o _libusb.so libusb_wrap.c -lusb 
strip _libusb.so
