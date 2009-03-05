METASOURCES = AUTO
noinst_PROGRAMS = libopenservo_lib.so
libopenservo_lib_so_LDFLAGS = -shared
AM_CFLAGS = -fPIC
libopenservo_lib_so_SOURCES = libopenservo_lib.c openservo_accessor_ptr.c \
	openservo_config.c openservo_func.c openservo_io.c openservo_optimise.c openservo_servo.c \
	openservo_update.c openservo_util.c
noinst_HEADERS = libopenservo_lib.h openservo_global.h openservo_io.h \
	openservo_lib_inc.h openservo_servo.h openservo_util.h register_feature_table.h \
	registers_features.h registers_v2.h registers_v3.h
libopenservo_lib_so_LDADD = -lio_core
_SOURCES = openservo_config.h
