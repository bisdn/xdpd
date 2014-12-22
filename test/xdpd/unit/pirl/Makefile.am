MAINTAINERCLEANFILES = Makefile.in

AUTOMAKE_OPTIONS = no-dependencies

test_pirl_SOURCES= $(top_srcdir)/src/xdpd/openflow/pirl/pirl.cc\
		test_pirl.cc

test_pirl_LDADD= -lrofl -lcppunit -lpthread -lrt

check_PROGRAMS = test_pirl
TESTS = test_pirl
