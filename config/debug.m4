# Check for debug mode - MUST BE THE FIRST CHECK
AC_MSG_CHECKING(whether to enable debug mode)
debug_default="no"
AC_ARG_ENABLE(debug,
	AS_HELP_STRING([--enable-debug], [turn on debug mode [default=no]])
		, , enable_debug=$debug_default)
if test "$enable_debug" = "yes"; then
	CFLAGS="-g -O0 $CFLAGS"
	CXXFLAGS="-g -O0 -fno-inline $CXXFLAGS"
	AC_DEFINE(DEBUG)
	AC_MSG_RESULT(yes)
else
	CFLAGS="-O3 $CFLAGS" #--compiler-options -fno-strict-aliasing --compiler-options -fno-inline
	CXXFLAGS="-O3 $CXXFLAGS" #-fomit-frame-pointer"
	AC_DEFINE(NDEBUG)
	AC_MSG_RESULT(no)
fi
AM_CONDITIONAL(DEBUG, test "$enable_debug" = yes)


