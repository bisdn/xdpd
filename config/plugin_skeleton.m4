#Define ALWAYS WITH_MGMT_$PLUGIN
AM_CONDITIONAL([WITH_MGMT_PLUGIN_NAME], true)
AC_DEFINE(WITH_MGMT_PLUGIN_NAME)

#LIB checks e.g.
#AC_CHECK_LIB(pthread, pthread_kill,,AC_MSG_ERROR([pthread library not found])) 

#Add files
AC_CONFIG_FILES([
	src/xdpd/management/plugins/PLUGIN_NAME/Makefile
])


