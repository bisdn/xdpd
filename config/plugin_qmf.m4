#Define ALWAYS WITH_MGMT_$PLUGIN
AC_DEFINE(WITH_MGMT_QMF)

#TODO add here LIB checks
PKG_CHECK_MODULES([QMF2], [qmf2 >= 0.22], [have_qmf2=yes], [have_qmf2=no])
if (test "${have_qmf2}" = "yes"); then
        PKG_CHECK_EXISTS([qmf2 >= 0.24], AC_DEFINE(WITH_QMF2_024))
        CPPFLAGS+="$QMF2_CFLAGS"
        LIBS+="$QMF2_LIBS"
fi

#Add files
AC_CONFIG_FILES([
	src/xdpd/management/plugins/qmf/Makefile
])

