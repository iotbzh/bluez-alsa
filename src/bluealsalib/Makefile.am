# BlueALSA - Makefile.am
# Copyright (c) 2018 Thiery Bultel (thierry.bultel@iot.bzh)

bluealsalib_LTLIBRARIES = libbluealsa.la

libbluealsa_la_SOURCES = \
	../shared/ctl-client.c \
	../shared/ffb.c \
	../shared/log.c \
	../shared/rt.c

EXTRA_DIST=bluealsa.pc.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = bluealsa.pc

AM_CFLAGS = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/includes \
	@ALSA_CFLAGS@ \
	@BLUEZ_CFLAGS@ \
	@GLIB2_CFLAGS@

libbluealsa_la_LIBADD = \
	@ALSA_LIBS@ \
	@BLUEZ_LIBS@

libbluealsa_la_LDFLAGS = \
	$(AM_LDFLAGS) \
	-version-info 1:0:0

bluealsalibdir = $(libdir)

# install path for published headers
bluealsalibincludedir=$(includedir)/bluealsa

bluealsalib_headers_dir=../../includes/bluealsa

bluealsalib_headers = \
	$(bluealsalib_headers_dir)/bluealsa.h \
	$(bluealsalib_headers_dir)/ctl-client.h \
	$(bluealsalib_headers_dir)/ctl-proto.h \
	$(bluealsalib_headers_dir)/defs.h \
	$(bluealsalib_headers_dir)/ffb.h \
	$(bluealsalib_headers_dir)/log.h \
	$(bluealsalib_headers_dir)/rt.h

libbluealsa_la_SOURCES += $(bluealsalib_headers)

bluealsalibinclude_HEADERS = $(bluealsalib_headers)
