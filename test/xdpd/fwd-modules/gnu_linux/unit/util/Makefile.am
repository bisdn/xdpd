MAINTAINERCLEANFILES = Makefile.in

ringbuffertest_SOURCES= $(top_srcdir)/src/xdpd/fwd-modules/gnu_linux/util/ringbuffer.cc \
	$(top_srcdir)/src/xdpd/fwd-modules/gnu_linux/util/ringbuffer.h \
	ringbuffertest.cc	

ringbuffertest_LDADD= -lrofl \
	-lcppunit \
	-lpthread

check_PROGRAMS=ringbuffertest

TESTS=ringbuffertest
