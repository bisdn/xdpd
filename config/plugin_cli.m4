#Define ALWAYS WITH_MGMT_$PLUGIN
AC_DEFINE(WITH_MGMT_CLI)

#LIB checks
AC_CHECK_LIB(cli, cli_telnet_protocol,,[AC_MSG_ERROR([cli library not found])]) 

#Add files
AC_CONFIG_FILES([
	src/xdpd/management/plugins/cli/Makefile
])


