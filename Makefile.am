AUTOMAKE_OPTIONS = foreign


AM_CFLAGS = $(GUI_CFLAGS)
AM_VALAFLAGS = -g --save-temps --pkg "gtk+-3.0" --pkg "glib-2.0" --pkg gee-0.8 --pkg pango --pkg gmodule-2.0

bin_PROGRAMS = gpaint

gpaint_SOURCES = src/main.vala
gpaint_LDADD = $(GUI_LIBS)

EXTRA_DIST =            \
        gpaint.desktop \
        gpaint.spec        \
	gpaint.glade \
	gpaint.gladep

dist-hook:
	if test -d $(srcdir)/pixmaps; then \
	  mkdir $(distdir)/pixmaps; \
	  for pixmap in $(srcdir)/pixmaps/*; do \
	    if test -f $$pixmap; then \
	      cp -p $$pixmap $(distdir)/pixmaps; \
	    fi \
	  done \
	fi

ACLOCAL_AMFLAGS = -I m4
CLEANFILES = *.h *.c *.vapi *.stamp
