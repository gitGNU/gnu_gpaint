AUTOMAKE_OPTIONS = foreign

GLIB_COMPILE_RESOURCES = `which glib-compile-resources`

AM_CFLAGS = $(GUI_CFLAGS)
AM_VALAFLAGS = -g --pkg "gtk+-3.0" --pkg "glib-2.0" --pkg "gio-2.0" --pkg gee-0.8 --pkg pango --pkg gmodule-2.0 --gresources $(top_srcdir)/gpaint.gresource.xml

bin_PROGRAMS = gpaint

gpaint_SOURCES = src/config.vapi src/main.vala
nodist_gpaint_SOURCES = res.c
gpaint_CFLAGS = \
        -DLOCALEDIR=\"@localedir@\" \
        -DGETTEXT_PACKAGE=\"$(GETTEXT_PACKAGE)\" \
        -w \
        $(GUI_CFLAGS)

gpaint_LDADD = $(GUI_LIBS)


res.c res.h: $(top_srcdir)/gpaint.gresource.xml $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=$(top_srcdir) --generate-dependencies $(top_srcdir)/gpaint.gresource.xml)
	$(GLIB_COMPILE_RESOURCES) --c-name=gpaint --target=$@   --sourcedir=$(top_srcdir) --generate-source  $< 	

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
