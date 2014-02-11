# C classifier
enable_c_classifier=1
AM_CONDITIONAL(C_CLASSIFIER, test $enable_c_classifier -eq 1)
if test $enable_c_classifier -eq 1 ; then
	AC_DEFINE(C_PACKET_CLASSIFIER)
fi

