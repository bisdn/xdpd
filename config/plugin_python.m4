#Define ALWAYS WITH_MGMT_$PLUGIN
AC_DEFINE(WITH_MGMT_PYTHON)

#TODO add here LIB checks
#AM_PATH_PYTHON(,, [:])
#AM_CONDITIONAL([HAVE_PYTHON], [test "$PYTHON" != :])

PKG_CHECK_MODULES([PYTHON], [python >= 2.7], [have_python=yes], [have_python=no])
if (test "${have_python}" = "yes"); then
        CPPFLAGS+="$PYTHON_CFLAGS"
        LIBS+="$PYTHON_LIBS"
fi

#Add files
AC_CONFIG_FILES([
	src/xdpd/management/plugins/python/Makefile
])

