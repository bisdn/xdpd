MAINTAINERCLEANFILES = Makefile.in

if C_CLASSIFIER
CLASSIFIER_SRC=$(top_srcdir)/src/io/packet_classifiers/c_pktclassifier/c_pktclassifier.c \
		$(top_srcdir)/src/io/packet_classifiers/push_pop_operations.cc
else
CLASSIFIER_SRC=$(top_srcdir)/src/io/packet_classifiers/cpp_pktclassifier/cpp_pktclassifier.cc \
		$(top_srcdir)/src/io/packet_classifiers/cpp_pktclassifier/packetclassifier.cc \
		$(top_srcdir)/src/io/packet_classifiers/cpp_pktclassifier/rofl_pktclassifier.cc \
		$(top_srcdir)/src/io/packet_classifiers/cpp_pktclassifier/static_pktclassifier.cc \
		$(top_srcdir)/src/io/packet_classifiers/cpp_pktclassifier/cpp_headers/cpp_ethernet.cc \
		$(top_srcdir)/src/io/packet_classifiers/cpp_pktclassifier/cpp_headers/cpp_arpv4.cc \
		$(top_srcdir)/src/io/packet_classifiers/cpp_pktclassifier/cpp_headers/cpp_icmpv4.cc \
		$(top_srcdir)/src/io/packet_classifiers/cpp_pktclassifier/cpp_headers/cpp_icmpv6.cc \
		$(top_srcdir)/src/io/packet_classifiers/cpp_pktclassifier/cpp_headers/cpp_ipv4.cc \
		$(top_srcdir)/src/io/packet_classifiers/cpp_pktclassifier/cpp_headers/cpp_ipv6.cc \
		$(top_srcdir)/src/io/packet_classifiers/cpp_pktclassifier/cpp_headers/cpp_gtpu.cc \
		$(top_srcdir)/src/io/packet_classifiers/cpp_pktclassifier/cpp_headers/cpp_mpls.cc \
		$(top_srcdir)/src/io/packet_classifiers/cpp_pktclassifier/cpp_headers/cpp_tcp.cc \
		$(top_srcdir)/src/io/packet_classifiers/cpp_pktclassifier/cpp_headers/cpp_udp.cc \
		$(top_srcdir)/src/io/packet_classifiers/cpp_pktclassifier/cpp_headers/cpp_ppp.cc \
		$(top_srcdir)/src/io/packet_classifiers/cpp_pktclassifier/cpp_headers/cpp_pppoe.cc \
		$(top_srcdir)/src/io/packet_classifiers/cpp_pktclassifier/cpp_headers/cpp_vlan.cc
endif

SHARED_SRC=\
	../of1x_cmm_mockup.c \
	$(top_srcdir)/src/afa-imp/forwarding_module.cc\
	$(top_srcdir)/src/io/iface_utils.cc \
	$(top_srcdir)/src/pipeline-imp/memory.c \
	$(top_srcdir)/src/pipeline-imp/pthread_lock.c \
	$(top_srcdir)/src/pipeline-imp/pthread_atomic_operations.c \
	$(top_srcdir)/src/io/pktin_dispatcher.cc \
	$(top_srcdir)/src/io/iomanager.cc \
	$(top_srcdir)/src/io/bufferpool.cc \
	$(top_srcdir)/src/io/datapacketx86.cc \
	$(top_srcdir)/src/io/datapacket_storage.cc \
	$(top_srcdir)/src/io/ports/ioport.cc \
	$(top_srcdir)/src/io/ports/mockup/ioport_mockup.cc \
	$(top_srcdir)/src/io/ports/mmap/ioport_mmap.cc \
	$(top_srcdir)/src/io/ports/mmap/mmap_int.cc \
	$(top_srcdir)/src/io/ports/mmap/ioport_mmapv2.cc \
	$(top_srcdir)/src/io/ports/mmap/mmap_rx.cc \
	$(top_srcdir)/src/io/ports/mmap/mmap_tx.cc \
	$(top_srcdir)/src/io/ports/vlink/ioport_vlink.cc \
	$(top_srcdir)/src/io/scheduler/epoll_ioscheduler.cc \
	$(top_srcdir)/src/processing/processingmanager.cc \
	$(top_srcdir)/src/util/time_utils.c \
	$(top_srcdir)/src/bg_taskmanager.cc \
	$(top_srcdir)/src/bg_taskmanager.h \
	$(top_srcdir)/src/pipeline-imp/packet.cc \
	$(CLASSIFIER_SRC)
	

if DEBUG
processingmanagertest_SOURCES= $(SHARED_SRC)\
			../platform_hooks_of1x_mockup.cc\
			processingmanagertest.cc 
			
processingmanagertest_LDADD= -lrofl -lcppunit -lpthread -lrofl_pipeline


check_PROGRAMS = processingmanagertest
TESTS = processingmanagertest 

endif
