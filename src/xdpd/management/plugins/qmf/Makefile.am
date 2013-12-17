MAINTAINERCLEANFILES = Makefile.in

noinst_LTLIBRARIES = libxdpd_mgmt_qmf.la

libxdpd_mgmt_qmf_la_SOURCES = \
	qmfagent.cc \
	qmfagent.h

libxdpd_mgmt_qmf_la_LIBADD = \
        -lqpidtypes \
        -lqpidmessaging \
        -lqmf2
