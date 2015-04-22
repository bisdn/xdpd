MAINTAINERCLEANFILES = Makefile.in

AUTOMAKE_OPTIONS = no-dependencies

test_checksums_SOURCES= $(top_srcdir)/src/io/packet_classifiers/c_pktclassifier/headers \
	test_checksums.cc

test_checksums_LDADD= -lrofl_common -lcppunit -lpthread

check_PROGRAMS = test_checksums
TESTS = test_checksums
