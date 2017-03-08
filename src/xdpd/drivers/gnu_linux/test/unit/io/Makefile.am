MAINTAINERCLEANFILES = Makefile.in

AUTOMAKE_OPTIONS = no-dependencies

SUBDIRS=packet_classifiers

test_datapacket_storage_SOURCES= $(top_srcdir)/src/io/datapacket_storage.cc\
	test_datapacket_storage.cc

test_datapacket_storage_LDADD= -lcppunit -lpthread

test_bufferpool_SOURCES=$(top_srcdir)/src/io/bufferpool.cc\
	$(top_srcdir)/src/io/datapacketx86.cc\
	$(top_srcdir)/src/pipeline-imp/memory.c\
	test_bufferpool.cc

test_bufferpool_LDADD= -lrofl_common -lcppunit -lpthread $(top_srcdir)/../../common/utils/libxdpd_common_utils.la

check_PROGRAMS = test_datapacket_storage test_bufferpool
TESTS = test_datapacket_storage test_bufferpool
