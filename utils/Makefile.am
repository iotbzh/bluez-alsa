# BlueALSA - Makefile.am
# Copyright (c) 2016-2018 Arkadiusz Bokowy

bin_PROGRAMS =

if ENABLE_APLAY
bin_PROGRAMS += bluealsa-aplay
bluealsa_aplay_SOURCES = \
	aplay.c
bluealsa_aplay_CFLAGS = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/includes \
	@ALSA_CFLAGS@ \
	@BLUEZ_CFLAGS@ \
	@GIO2_CFLAGS@
bluealsa_aplay_LDADD = \
	@ALSA_LIBS@ \
	@BLUEZ_LIBS@ \
	@GIO2_LIBS@ \
	../src/bluealsalib/libbluealsa.la
endif

if ENABLE_RFCOMM
bin_PROGRAMS += bluealsa-rfcomm
bluealsa_rfcomm_SOURCES = \
	rfcomm.c
bluealsa_rfcomm_CFLAGS = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/includes \
	@BLUEZ_CFLAGS@
bluealsa_rfcomm_LDADD = \
	@BLUEZ_LIBS@ \
	../src/bluealsalib/libbluealsa.la \
	-lreadline
endif

if ENABLE_HCITOP
bin_PROGRAMS += hcitop
hcitop_CFLAGS = \
	@BLUEZ_CFLAGS@ \
	@LIBBSD_CFLAGS@ \
	@NCURSES_CFLAGS@
hcitop_LDADD = \
	@BLUEZ_LIBS@ \
	@LIBBSD_LIBS@ \
	@NCURSES_LIBS@
endif
