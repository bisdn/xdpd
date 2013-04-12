MAINTAINERCLEANFILES = Makefile.in

test_datapacket_storage_SOURCES= $(top_srcdir)/src/xdpd/fwd-modules/gnu_linux/io/datapacket_storage.cc\
	test_datapacket_storage.cc

test_datapacket_storage_LDADD= -lrofl -lcppunit -lpthread

check_PROGRAMS = test_datapacket_storage
TESTS = test_datapacket_storage
