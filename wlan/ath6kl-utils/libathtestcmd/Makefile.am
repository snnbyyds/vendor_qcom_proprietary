AM_CFLAGS = -Wall \
           -g -O0 \
           -DLIBNL_2 \
           $(LIBNL_CFLAGS) \
           -I $(top_srcdir)/libtcmd \
           -I include \
           -I libtlv2 \
           -I libtlv2/include \
           -I libtlvutil \
           -I libtlvutil/common

SUBDIRS = libtlvutil libtlv2

if USE_GLIB
AM_CFLAGS += -DUSE_GLIB $(GLIB_CFLAGS)
endif

AM_CFLAGS += -D_HOST_SIM_TESTING -DLinux -DUSE_TLV2

c_sources = athtestcmdlib.c

libtestcmd6174_LIBADD := libtcmd.a libtlvutil.a libtlv2.a

lib_LIBRARIES  = libtestcmd6174.a
libtestcmd6174_a_SOURCES = $(c_sources)
libtestcmd6174_a_LDFLAGS = -static
pkginclude_HEADERS = testcmd6174.h \
		     include/testcmd.h \
		     include/art_utf_common.h
