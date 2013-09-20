
AC_CHECK_LIB(cli, cli_telnet_protocol,,[AC_MSG_ERROR([cli library not found])]) 

AC_DEFINE(HAVE_CONFIG_CLI)
AM_CONDITIONAL(ENABLE_CONFIG_CLI,[true])

AM_COND_IF(ENABLE_CONFIG_CLI,[AC_CONFIG_FILES([
	src/xdpd/management/adapter/cli/Makefile
])])


