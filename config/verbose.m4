# Application verbosity 
AC_MSG_CHECKING(whether to print debug in verbose mode)
AC_ARG_ENABLE(verbose,
	AS_HELP_STRING([--verbose], [turn on verbose mode [default=no]])
		, verbose_debug="yes", verbose_debug="no")

AC_DEFINE(VERBOSE_DEBUG)
if test "$verbose_debug" = "yes"; then
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi
