#Define ALWAYS WITH_MGMT_$PLUGIN
AC_DEFINE(WITH_MGMT_CONFIG)

#LIB checks
AC_LANG_PUSH([C++])

AC_CHECK_HEADER([libconfig.h++],,
[AC_MSG_ERROR([libconfig C++ library not found])]
)

AC_LANG_POP([C++])

#Add files
AC_CONFIG_FILES([
	src/xdpd/management/plugins/config/Makefile
	src/xdpd/management/plugins/config/interfaces/Makefile
	src/xdpd/management/plugins/config/openflow/Makefile
])


