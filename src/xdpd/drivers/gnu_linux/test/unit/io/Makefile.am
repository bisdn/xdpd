MAINTAINERCLEANFILES = Makefile.in

test_pirl_SOURCES= $(top_srcdir)/src/io/pirl/pirl.c\
	test_pirl.cc

test_pirl_LDADD= -lrofl -lcppunit -lpthread -lrt

test_datapacket_storage_SOURCES= $(top_srcdir)/src/io/datapacket_storage.cc\
	test_datapacket_storage.cc

test_datapacket_storage_LDADD= -lrofl -lcppunit -lpthread

check_PROGRAMS = test_datapacket_storage test_pirl
TESTS = test_datapacket_storage test_pirl
