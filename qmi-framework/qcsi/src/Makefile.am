AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic \
	-DQMI_CSI_ANDROID \
	-DQMI_FW_SYSLOG

AM_CPPFLAGS = \
        -I../../inc \
        -I../inc \
        -I../../common/inc \
        -I../../qsocket/inc \
	-I../../qrtr/lib \
	-I../../qrtr/src \
        $(QMIFV_CFLAGS)

if ENABLE_QMI_LOG
AM_CFLAGS += -DQMI_ANDROID_LOGGING_LE
endif

h_sources = ../inc/qmi_csi_common.h \
	../../qrtr/lib/libqrtr.h \
	../../qrtr/src/ns.h

requiredlibs = -lpthread \
        ../../encdec/libqmi_encdec.la

if ENABLE_QMI_LOG
requiredlibs += -llog
endif

library_includedir = $(pkgincludedir)
library_include_HEADERS = $(h_sources)

c_sources = qmi_csi_common.c \
	qmi_csi_target.c \
        qmi_csi_xport_ipc_router.c \
	qmi_csi_xport_qrtr.c \
        ../../common/src/common_v01.c

lib_LTLIBRARIES = libqmi_csi.la
libqmi_csi_la_CC = @CC@
libqmi_csi_la_SOURCES = $(c_sources) $(h_sources)
libqmi_csi_la_CFLAGS = $(AM_CFLAGS)
libqmi_csi_la_CPPFLAGS = $(AM_CPPFLAGS)
libqmi_csi_la_LDFLAGS = -shared -version-info 1:0:0
libqmi_csi_la_LIBADD = $(requiredlibs)
