# Nice message
AC_MSG_CHECKING(and enforcing rofl-pipeline is not compiled with packet processing functions inlined)
AC_MSG_RESULT(testing header)

#Check header
AC_CHECK_HEADERS([rofl_pipeline.h], [], pipeline_inline="yes", [#define ROFL_PIPELINE_ABORT_IF_INLINED 1])

if test "$pipeline_inline" = "yes"; then
	AC_ERROR([rofl-pipeline is compiled with inlined platform functions, but this module does not support it])
fi
