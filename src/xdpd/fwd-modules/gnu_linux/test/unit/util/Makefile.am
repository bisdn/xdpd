MAINTAINERCLEANFILES = Makefile.in

ringbuffertest_SOURCES= $(top_srcdir)/src/util/ringbuffer.cc \
	$(top_srcdir)/src/util/ringbuffer.h \
	ringbuffertest.cc	

ringbuffertest_LDADD= -lrofl \
	-lcppunit \
	-lpthread

check_PROGRAMS=ringbuffertest

TESTS=ringbuffertest
