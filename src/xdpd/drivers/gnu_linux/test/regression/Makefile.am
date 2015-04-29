MAINTAINERCLEANFILES = Makefile.in

AUTOMAKE_OPTIONS = no-dependencies

SUBDIRS = io 

CLASSIFIER_SRC=$(top_srcdir)/src/io/packet_classifiers/c_types_pktclassifier/c_types_pktclassifier.c \
		$(top_srcdir)/src/io/packet_classifiers/c_types_pktclassifier/autogen_pkt_types.c \
		$(top_srcdir)/src/io/packet_classifiers/packet_operations.cc

#Shared stuff
SHARED_SRC=\
	of1x_cmm_mockup.c \
	$(top_srcdir)/src/hal-imp/driver.cc\
	$(top_srcdir)/src/io/iface_utils.cc \
	$(top_srcdir)/src/pipeline-imp/memory.c \
	$(top_srcdir)/src/pipeline-imp/pthread_lock.c \
	$(top_srcdir)/src/pipeline-imp/atomic_operations.c \
	$(top_srcdir)/src/pipeline-imp/timing.c \
	$(top_srcdir)/src/io/pktin_dispatcher.cc \
	$(top_srcdir)/src/io/iomanager.cc \
	$(top_srcdir)/src/io/bufferpool.cc \
	$(top_srcdir)/src/io/datapacketx86.cc \
	$(top_srcdir)/src/io/datapacket_storage.cc \
	$(top_srcdir)/src/io/ports/ioport.cc \
	$(top_srcdir)/src/io/ports/mockup/ioport_mockup.cc \
	$(top_srcdir)/src/io/ports/mmap/mmap_rx.cc \
	$(top_srcdir)/src/io/ports/mmap/mmap_tx.cc \
	$(top_srcdir)/src/io/ports/mmap/ioport_mmap.cc \
	$(top_srcdir)/src/io/ports/vlink/ioport_vlink.cc \
	$(top_srcdir)/src/io/scheduler/epoll_ioscheduler.cc \
	$(top_srcdir)/src/util/time_utils.c \
	$(top_srcdir)/src/bg_taskmanager.cc \
	$(top_srcdir)/src/bg_taskmanager.h \
	platform_hooks_of1x_mockup.cc \
	$(CLASSIFIER_SRC)
		
SHARED_LIBS=\
	-lrofl_common \
	-lrofl_datapath \
	-lcppunit \
	-lpthread
	

#Simple portmockup with mockup matches	
test_portmockup_matchesmockup_SOURCES = \
	$(SHARED_SRC) \
	pipeline_packet_mockup.c \
	test_portmockup.cc
	
test_portmockup_matchesmockup_LDADD = \
	$(SHARED_LIBS)

#Simple portmockup with non-mockup matches	
test_portmockup_SOURCES = \
	$(SHARED_SRC) \
	$(top_srcdir)/src/pipeline-imp/packet.cc \
	test_portmockup.cc
test_portmockup_LDADD = \
	$(SHARED_LIBS)
	

#Multiple portmockup with mockup matches	
test_portmockup_matchesmockup_multiport_SOURCES = \
	$(SHARED_SRC) \
	pipeline_packet_mockup.c \
	test_portmockup_multiport.cc
test_portmockup_matchesmockup_multiport_LDADD = \
	$(SHARED_LIBS)
	
#Multiple portmockup with non-mockup matches	
test_portmockup_multiport_SOURCES = \
	$(SHARED_SRC) \
	$(top_srcdir)/src/pipeline-imp/packet.cc \
	test_portmockup_multiport.cc
test_portmockup_multiport_LDADD = \
	$(SHARED_LIBS)
	
#MMAP
test_portmmap_SOURCES = \
	$(SHARED_SRC) \
	$(top_srcdir)/src/pipeline-imp/packet.cc \
	test_mmapport.cc
test_portmmap_LDADD = \
	$(SHARED_LIBS)

if DEBUG
#test for the expiration of buffers in the datapacket_storage
test_storage_packets_expiration_SOURCES = \
	$(SHARED_SRC) \
	$(top_srcdir)/src/pipeline-imp/packet.cc \
	test_storage_packets_expiration.cc
test_storage_packets_expiration_LDADD = \
	$(SHARED_LIBS)
endif
	
#test for the hcl notifications for port events (add, delete & status change)
test_port_status_SOURCES = \
	$(SHARED_SRC) \
	$(top_srcdir)/src/pipeline-imp/packet.cc \
	test_port_status.cc
test_port_status_LDADD = \
	$(SHARED_LIBS)
	
	
check_SCRIPTS = \
	test_launcher.sh

TESTS = \
	$(check_SCRIPTS)

check_PROGRAMS = \
	test_portmockup_matchesmockup\
	test_portmockup\
	test_portmockup_matchesmockup_multiport\
	test_portmockup_multiport\
	test_portmmap \
	test_port_status
	
if DEBUG
check_PROGRAMS +=test_storage_packets_expiration
endif


