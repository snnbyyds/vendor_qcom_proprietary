pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ../../omx.pc
EXTRA_DIST = $(pkgconfig_DATA)

AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic \
        -I ./inc/ \
        -I ./src/common/ \
         $(MM_CORE_CFLAGS)

AM_CPPFLAGS = -I ./inc/ \
              -I $(PKG_CONFIG_SYSROOT_DIR)/usr/include/audio-kernel \
              $(MM_CORE_CFLAGS)

c_sources = src/adec_svr.c \
            src/omx_alac_adec.cpp

#LIBRARY for Android apps
lib_LTLIBRARIES = libOmxAlacDec.la
libOmxAlacDec_la_CC = @CC@
libOmxAlacDec_la_SOURCES = $(c_sources)
libOmxAlacDec_la_CFLAGS = $(AM_CFLAGS) -DUSE_GLIB @GLIB_CFLAGS@ -DQC_MODIFIED -D_ENABLE_QC_MSG_LOG_ -DAUDIOV2 -Wconversion -DQCOM_AUDIO_USE_SYSTEM_HEAP_ID
libOmxAlacDec_la_CPPFLAGS = $(AM_CPPFLAGS) -Dstrlcpy=g_strlcpy -DUSE_GLIB @GLIB_CFLAGS@ -DQC_MODIFIED -D_ENABLE_QC_MSG_LOG_ -DAUDIOV2 -Wconversion -DQCOM_AUDIO_USE_SYSTEM_HEAP_ID -include glib.h
libOmxAlacDec_la_LDFLAGS = $(GLIB_LIBS) -shared -avoid-version
if USE_LIBION_LIBRARY
libOmxAlacDec_la_LDFLAGS += -lion
endif
