MAINTAINERCLEANFILES = Makefile.in

#Shared stuff
SHARED_SRC=\
	../of1x_cmm_mockup.c \
	$(top_srcdir)/src/afa-imp/forwarding_module.cc\
	$(top_srcdir)/src/io/iface_utils.cc \
	$(top_srcdir)/src/pipeline-imp/platform_hooks_of1x.cc \
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
	$(top_srcdir)/src/io/ports/mmap/mmap_rx.cc \
	$(top_srcdir)/src/io/ports/mmap/mmap_tx.cc \
	$(top_srcdir)/src/io/ports/vlink/ioport_vlink.cc \
	$(top_srcdir)/src/io/scheduler/epoll_ioscheduler.cc \
	$(top_srcdir)/src/processing/processingmanager.cc \
	$(top_srcdir)/src/util/time_utils.c \
	$(top_srcdir)/src/bg_taskmanager.cc \
	$(top_srcdir)/src/bg_taskmanager.h \
	$(top_srcdir)/src/pipeline-imp/memory.c \
	$(top_srcdir)/src/pipeline-imp/packet.cc 
	
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
	
datapacketx86test_SOURCES= $(top_srcdir)/src/io/datapacketx86.cc\
			$(top_srcdir)/src/io/bufferpool.cc\
			$(top_srcdir)/src/pipeline-imp/memory.c \
			$(CLASSIFIER_SRC) \
			datapacketx86test.cc 
			
datapacketx86test_LDADD= -lrofl -lcppunit -lpthread

#mmap_port_test_SOURCES = $(SHARED_SRC)\
#			$(CLASSIFIER_SRC) \
#			mmap_port_test.cc
	

			
#mmap_port_test_LDADD = -lrofl -lcppunit -lpthread -lrofl_pipeline 

#check_SCRIPTS = mmap_port_test.sh

#if DEBUG
#DEBUG_ONLY_TESTS= portmockuptest 
#endif

check_PROGRAMS = datapacketx86test $(DEBUG_ONLY_TESTS)

TESTS = datapacketx86test $(DEBUG_ONLY_TESTS) 
