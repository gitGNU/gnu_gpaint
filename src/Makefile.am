## Process this file with automake to produce Makefile.in


INCLUDES = \
        -DPACKAGE_DATA_DIR=\""$(datadir)"\" \
        -DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
        @GUI_CFLAGS@ \
        @PRINT_CFLAGS@


bin_PROGRAMS = gpaint-2

gpaint_2_SOURCES = \
about.c \
about.h \
brush.c \
brush.h \
callbacks.c \
callbacks.h \
canvas.c \
canvas.h \
color_palette.c \
color_palette.h \
debug.h \
drawing.c \
drawing.h \
file.c \
file.h \
fill.c \
fill.h \
freehand.c \
freehand.h \
gtkscrollframe.c \
gtkscrollframe.h \
image.c \
image.h \
image_processing.c \
image_processing.h \
lasso.c \
lasso.h \
main.c \
menu.c \
paste.c \
paste.h \
pen.c \
pen.h \
pixmaps.c \
pixmaps.h \
polyselect.c \
polyselect.h \
rectselect.c \
rectselect.h \
print.c \
print.h \
selection.c \
selection.h \
shape.c \
shape.h \
support.c \
support.h \
text.c \
text.h \
tool_palette.c \
tool_palette.h \
ui.c \
ui.h \
version.h


gpaint_2_CFLAGS = -I$(srcdir)
gpaint_2_LDADD = $(GUI_LIBS) $(PRINT_LIBS)

