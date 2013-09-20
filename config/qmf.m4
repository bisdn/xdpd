# Check for QMF support 
#FIXME Take this out of here
AC_MSG_CHECKING(whether to add Apache QMF support)
qmf_default="no"
AC_ARG_ENABLE(qmf,
	AS_HELP_STRING([--enable-qmf], [turn on Apache QMF support [default=no]])
		, , enable_qmf=$qmf_default)
if test "$enable_qmf" = "yes"; then 
	AC_DEFINE(HAVE_CONFIG_QMF)
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi
AM_CONDITIONAL(ENABLE_CONFIG_QMF, test "$enable_qmf" = yes)

AM_COND_IF(ENABLE_CONFIG_QMF,[AC_CONFIG_FILES([
	src/xdpd/management/adapter/qmf/Makefile
])])

