# Check for debug mode - MUST BE THE FIRST CHECK
AC_MSG_CHECKING(whether to enable debug mode)
debug_default="no"
AC_ARG_ENABLE(debug,
	AS_HELP_STRING([--enable-debug], [turn on debug mode [default=no]])
		, , enable_debug=$debug_default)
if test "$enable_debug" = "yes"; then
	CFLAGS="$CFLAGS -g -O0"
	CXXFLAGS="$CXXFLAGS -g -O0 -fno-inline"
	AC_DEFINE(DEBUG)
	AC_MSG_RESULT(yes)
else
	CFLAGS="$CFLAGS -O3" #--compiler-options -fno-strict-aliasing --compiler-options -fno-inline
	CXXFLAGS="$CXXFLAGS -O3" #-fomit-frame-pointer"
	AC_DEFINE(NDEBUG)
	AC_MSG_RESULT(no)
fi
AM_CONDITIONAL(DEBUG, test "$enable_debug" = yes)


