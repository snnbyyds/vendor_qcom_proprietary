
AM_CFLAGS = -Wall \
		-Wundef \
		-Wstrict-prototypes \
		-Wno-trigraphs	\
		-DUSE_LOGCAT_LOGGING

AM_CPPFLAGS = -D__packed__= \
		-DIMAGE_APPS_PROC \
		-DFEATURE_Q_SINGLE_LINK \
		-DFEATURE_Q_NO_SELF_QPTR \
		-DFEATURE_LINUX \
		-DFEATURE_NATIVELINUX \
		-DFEATURE_DSM_DUP_ITEMS \
		-DFEATURE_LE_DIAG \
		$(DIAG_CFLAGS) \
		-I$(WORKSPACE)/common/inc

bin_PROGRAMS = diag_dci_sample

diag_dci_sample_SOURCES = diag_dci_sample.c
diag_dci_sample_LDADD = $(srcdir)/../src/.libs/libdiag.so
if USE_GLIB
diag_dci_sample_CFLAGS =  -DUSE_GLIB @GLIB_CFLAGS@ $(DIAG_CFLAGS)
diag_dci_sample_LDFLAGS = -lpthread @GLIB_LIBS@ -llog $(DIAG_LIBS)
else
diag_dci_sample_LDFLAGS = -lpthread -lcutils -llog $(DIAG_LIBS)
endif
