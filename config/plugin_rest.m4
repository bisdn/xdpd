AM_CONDITIONAL([WITH_MGMT_REST], true)
AC_DEFINE(WITH_MGMT_REST)

BOOST_REQUIRE([1.54], [HAVE_BOOST=false])
if (test "$HAVE_BOOST" = "false"); then
  AC_ERROR("REST management plugin requires Boost")
fi

BOOST_ASIO
BOOST_THREADS([mt])
BOOST_SYSTEM([mt])
CPPFLAGS+="$BOOST_CPPFLAGS"
LDFLAGS+="$BOOST_SYSTEM_LDFLAGS $BOOST_THREAD_LDFLAGS"
LIBS+="$BOOST_SYSTEM_LIBS $BOOST_THREAD_LIBS"

AC_CONFIG_FILES([
	src/xdpd/management/plugins/rest/Makefile
  src/xdpd/management/plugins/rest/server/Makefile
])

