AM_CONDITIONAL([WITH_MGMT_REST], true)
AC_DEFINE(WITH_MGMT_REST)

BOOST_REQUIRE([1.54])
BOOST_ASIO
BOOST_SYSTEM([mt])
CPPFLAGS+="$BOOST_CPPFLAGS"
LDFLAGS+="$BOOST_SYSTEM_LDFLAGS"
LIBS+="$BOOST_SYSTEM_LIBS"

#Add files
AC_CONFIG_FILES([
	src/xdpd/management/plugins/rest/Makefile
  src/xdpd/management/plugins/rest/server/Makefile
])

