## Process this file with automake to produce Makefile.in

INCLUDES = \
	-I$(top_srcdir)/intl \
	$(GNOME_INCLUDEDIR)

bin_PROGRAMS = gpaint

gpaint_SOURCES = main.c \
	support.c support.h version.h\
	ui.c ui.h \
	callbacks.c callbacks.h pixmaps.c pixmaps.h image_buf.c image_buf.h rgb.c rgb.h util.c util.h \
	gtkscrollframe.c gtkscrollframe.h fileio.c fileio.h image_processing.c image_processing.h\
	clipboard.c clipboard.h print.c print.h

CFLAGS += $(EXTRA_GNOME_CFLAGS)
gpaint_LDADD = $(GNOME_LIBDIR) $(GNOMEUI_LIBS) $(INTLLIBS) $(EXTRA_GNOME_LIBS)

