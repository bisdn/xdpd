#Detecting crypto and ssl
ssl_detected="yes"

AC_CHECK_LIB(ssl, SSL_library_init, , ssl_detected="no")
AC_CHECK_LIB(crypto, ERR_get_error, , ssl_detected="no")
       
AC_MSG_CHECKING(for availabilty of openssl and crypto libraries(SSL/TLS))
if test "$ssl_detected" = "yes"; then
	AC_MSG_RESULT(found)
	AC_DEFINE(HAVE_OPENSSL, 1)
else
	AC_MSG_RESULT(not found)
fi
 
AM_CONDITIONAL([HAVE_OPENSSL],  [test "$ssl_detected" = yes])
