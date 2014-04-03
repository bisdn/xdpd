# Check for experimental 
AC_MSG_CHECKING(whether to compile experimental code)
experimental_default="no"
AC_ARG_ENABLE(experimental, AS_HELP_STRING([--enable-experimental], [compile experimental code [default=no]]), , enable_experimental=$experimental_default)
if test "$enable_experimental" = "yes"; then
	AC_DEFINE(EXPERIMENTAL)
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi

#Set automake conditional
AM_CONDITIONAL(EXPERIMENTAL, test "$enable_experimental" = yes)
