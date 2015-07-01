#Pipeline optimizations

AC_MSG_CHECKING(if user overrides default rofl-pipeline optimizations...)

#Detect if user is overriding our defaults
if test -z "`echo $AC_CONFIGURE_ARGS |  sed -e 's/--with\(out\)*-pipeline/USER_PIPELINE_CTL/' | grep USER_PIPELINE_CTL`"; then
	USER_OVERRIDES_PIPELINE_FLAGS="no"
	AC_MSG_RESULT([no])
else
	USER_OVERRIDES_PIPELINE_FLAGS="yes"
	AC_MSG_RESULT([yes])
fi

AM_CONDITIONAL([USER_OVERRIDES_PIPELINE_FLAGS], [test "$USER_OVERRIDES_PIPELINE_FLAGS" = "yes"])
