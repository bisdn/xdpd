# Nice message
AC_MSG_CHECKING(and enforcing rofl-pipeline is not compiled with packet processing functions inlined)
AC_MSG_RESULT(testing header)

#Check header
AC_CHECK_HEADER([rofl_datapath.h], pipeline_compiled="yes", pipeline_compiled="no")

#Remove cache
$as_unset ac_cv_header_rofl_datapath_h

#Check headers
AC_CHECK_HEADERS([rofl_datapath.h], [], pipeline_inline="yes", [#define ROFL_PIPELINE_ABORT_IF_INLINED 1])

if test "$pipeline_compiled" = "yes"; then
	if test "$pipeline_inline" = "yes"; then
	AC_ERROR([rofl-datapath is compiled with inlined platform functions, but this module does not support it]
	[               Force a recompilation of rofl libraries by triggering 'make clean-libs'])
	fi
fi
