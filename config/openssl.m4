# Check for debug mode - MUST BE THE FIRST CHECK
AC_MSG_CHECKING(whether to enable openssl for tls)
ssl_default="no"
AC_ARG_ENABLE(ssl,
	AS_HELP_STRING([--enable-ssl], [turn on tls via OpenSSL [default=no]])
		, , enable_ssl=$ssl_default)
AC_MSG_RESULT($ssl_default)
if test "$enable_ssl" = "yes"; then
	AC_CHECK_LIB(ssl, SSL_library_init,,AC_MSG_ERROR([OpenSSL ssl library not found])) 
	AC_CHECK_LIB(crypto, ERR_get_error,,AC_MSG_ERROR([OpenSSL crypto library not found]))
	AC_DEFINE(HAVE_OPENSSL, 1)
fi
AM_CONDITIONAL([HAVE_OPENSSL],  [test "$enable_ssl" = yes])

