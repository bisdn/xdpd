#Define ALWAYS WITH_MGMT_$PLUGIN
AM_CONDITIONAL([WITH_MGMT_RUN_PEX], true)
AC_DEFINE(WITH_MGMT_RUN_PEX)

#LIB checks e.g.
#AC_CHECK_LIB(pthread, pthread_kill,,AC_MSG_ERROR([pthread library not found])) 

#Add files
AC_CONFIG_FILES([
	src/xdpd/management/plugins/run_pex/Makefile
])


