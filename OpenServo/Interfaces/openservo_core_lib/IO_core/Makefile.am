METASOURCES = AUTO
libio_core_so_LDFLAGS = -version-info 0:0:1 -shared
noinst_PROGRAMS = libio_core.so
AM_CFLAGS = -fPIC
libio_core_so_SOURCES = adapter.c adapter_enumerate.c adapter_parseconfig.c \
	adapters.ini libio_core.c
noinst_HEADERS = IO_error.h adapter.h adapter_enumerate.h adapter_parseconfig.h \
	libio_core.h
SUBDIRS = OSIF dummy linux
libio_core_so_LDADD = -lopenservoadapter_linux -ldl
