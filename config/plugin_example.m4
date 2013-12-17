AM_CONDITIONAL([WITH_MGMT_EXAMPLE], true)
AC_DEFINE(WITH_MGMT_EXAMPLE)

#Add files
AC_CONFIG_FILES([
	src/xdpd/management/plugins/example/Makefile
])


