MAINTAINERCLEANFILES = Makefile.in

ringbuffertest_SOURCES= \
	ringbuffertest.cc	

ringbuffertest_LDADD= \
	-lrofl_common \
	-lcppunit \
	-lpthread \
	 $(top_srcdir)/../../common/utils/libxdpd_common_utils.la

check_PROGRAMS=ringbuffertest

TESTS=ringbuffertest
