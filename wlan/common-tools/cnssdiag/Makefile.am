AM_CFLAGS = -Wall \
            $(DIAG_CFLAGS) \
	    $(LIBNL_CFLAGS)

if DEBUG
AM_CFLAGS += -g
else
AM_CFLAGS += -O2
endif

requiredlibs = -lnl-3 -lnl-genl-3 -lrt

if USE_GLIB
AM_CFLAGS += -DUSE_GLIB $(GLIB_CFLAGS)
requiredlibs += $(GLIB_LIBS)
endif

if CLD80211
AM_CFLAGS += -DCONFIG_CLD80211_LIB
requiredlibs += -lcld80211
endif

cnss_diag_SOURCES = cld-fwlog-netlink.c parser.c cld-diag-parser.c nan-parser.c
cnss_diag_LDADD = $(DIAG_LIBS) $(requiredlibs)

bin_PROGRAMS = cnss_diag

