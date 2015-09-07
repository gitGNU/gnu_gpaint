AC_INIT(gpaint, 0.4, http://savannah.gnu.org/bugs/?group=gpaint)
AC_CONFIG_SRCDIR([src/main.vala])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE([subdir-objects])

AC_GNU_SOURCE

AC_PROG_MAKE_SET

AC_PROG_INSTALL
AC_LIBTOOL_DLOPEN
AC_LIBTOOL_WIN32_DLL

AC_PROG_LIBTOOL
AC_PROG_CC

AM_PROG_VALAC([0.21.2])

AC_DEFINE(_GNU_SOURCE, 1)

PKG_CHECK_MODULES(GUI, [gtk+-3.0 glib-2.0 gio-2.0 gmodule-2.0 gee-0.8 pango])

AC_SUBST(GUI_CFLAGS)

AC_CONFIG_FILES(Makefile)
AC_OUTPUT

echo "
$PACKAGE_NAME $PACKAGE_VERSION configuration:
-----------------------------
  Source code location:       ${srcdir}
  Host System Type:           ${host}
  Compiler:                   ${CC}
  Standard CFLAGS:            ${CFLAGS} ${ac_devel_default_warnings}
  Libraries:                  ${LIBDL_LIBS} ${LIBM_LIBS} ${LIBREADLINE_LIBS}
  Install path (prefix):      ${prefix}

  Now type 'make' to build $PACKAGE_NAME $PACKAGE_VERSION.
"
