# Application verbosity 
AC_MSG_CHECKING(whether to print debug in verbose mode)
AC_ARG_ENABLE(verbose,
	AS_HELP_STRING([--enable-verbose], [turn on verbose mode [default=no]])
		, verbose_debug="yes", verbose_debug="no")

if test "$verbose_debug" = "yes"; then
	AC_MSG_RESULT(yes)
	AC_DEFINE(VERBOSE_DEBUG)
else
	AC_MSG_RESULT(no)
fi
