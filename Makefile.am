AUTOMAKE_OPTIONS = foreign

GLIB_COMPILE_RESOURCES = `which glib-compile-resources`

AM_CFLAGS = $(GUI_CFLAGS)
AM_VALAFLAGS = -g --save-temps --pkg "gtk+-3.0" --pkg "glib-2.0" --pkg "gio-2.0" --pkg gee-0.8 --pkg pango --pkg gmodule-2.0

bin_PROGRAMS = gpaint

gpaint_SOURCES = src/main.vala
nodist_gpaint_SOURCES = res.c
gpaint_LDADD = $(GUI_LIBS)

GPAINT_RESOURCES = $(sh $(GLIB_COMPILE_RESOURCES) --sourcedir=$(top_srcdir) --generate-dependencies $(top_srcdir)/gpaint.gresource.xml)

res.c: res.h $(top_srcdir)/gpaint.gresource.xml $(GPAINT_RESOURCES)
	$(GLIB_COMPILE_RESOURCES) --c-name=gpaint --target=$@   --sourcedir=$(top_srcdir) --generate-source  $(top_srcdir)/gpaint.gresource.xml 	

res.h: $(top_srcdir)/gpaint.gresource.xml $(GPAINT_RESOURCES)
	$(GLIB_COMPILE_RESOURCES) --c-name=gpaint --target=$@   --sourcedir=$(top_srcdir) --generate-header  $(top_srcdir)/gpaint.gresource.xml  

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
