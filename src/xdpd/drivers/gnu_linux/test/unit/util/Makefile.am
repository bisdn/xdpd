MAINTAINERCLEANFILES = Makefile.in

ringbuffertest_SOURCES= \
	../../../src/c_logger.h \
	../../../src/c_logger.c \
	ringbuffertest.cc	

ringbuffertest_LDADD= \
	-lrofl_common \
	-lcppunit \
	-lpthread

check_PROGRAMS=ringbuffertest

TESTS=ringbuffertest
