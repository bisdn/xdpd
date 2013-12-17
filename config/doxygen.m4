#CHECK for doxygen
AC_CHECK_PROGS([DOXYGEN], [doxygen],)
AC_MSG_CHECKING(whether to compile documentation)

doc="no"
AC_ARG_WITH([doc],
	AS_HELP_STRING([--with-doc], [compile documentation if doxygen is available [default=no]])
		, doc="yes", )

if test -z "$DOXYGEN";then
	if test "$doc" == "yes"; then
		AC_MSG_ERROR([Unable to compile documentation. DOXYGEN not found!])
	fi
	AC_MSG_RESULT([no (doxygen not present!)])
	AM_CONDITIONAL(doc, [false])
else
	if test "$doc" = "yes"; then
		AM_CONDITIONAL(doc, [true])
		AC_CONFIG_FILES([doc/doxyfile.conf])	
		AC_MSG_RESULT(yes)
	else
		AC_MSG_RESULT(no)
		AM_CONDITIONAL(doc, [false])
	fi
fi

#Always add doc Makefile
AC_CONFIG_FILES([
	doc/Makefile
])
