## Process this file with automake to produce Makefile.in

SUBDIRS = po src

EXTRA_DIST =            \
        gpaint.desktop \
        gpaint.spec        \
	gpaint.glade \
	gpaint.gladep

install-data-local:
	@$(NORMAL_INSTALL)

dist-hook:
	if test -d $(srcdir)/pixmaps; then \
	  mkdir $(distdir)/pixmaps; \
	  for pixmap in $(srcdir)/pixmaps/*; do \
	    if test -f $$pixmap; then \
	      cp -p $$pixmap $(distdir)/pixmaps; \
	    fi \
	  done \
	fi

