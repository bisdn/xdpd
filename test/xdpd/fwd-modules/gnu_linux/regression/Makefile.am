MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = io processing 

#Shared stuff
SHARED_SRC=\
	of12_cmm_mockup.c \
	$(top_srcdir)/src/xdpd/fwd-modules/gnu_linux/afa-imp/forwarding_module.c\
	$(top_srcdir)/src/xdpd/fwd-modules/gnu_linux/io/iface_utils.cc \
	$(top_srcdir)/src/xdpd/fwd-modules/gnu_linux/pipeline-imp/memory.c \
	$(top_srcdir)/src/xdpd/fwd-modules/gnu_linux/pipeline-imp/pthread_lock.c \
	$(top_srcdir)/src/xdpd/fwd-modules/gnu_linux/pipeline-imp/pthread_atomic_operations.c \
	$(top_srcdir)/src/xdpd/fwd-modules/gnu_linux/util/ringbuffer.cc \
	$(top_srcdir)/src/xdpd/fwd-modules/gnu_linux/util/ringbuffer_c_wrapper.cc \
	$(top_srcdir)/src/xdpd/fwd-modules/gnu_linux/io/iomanager.cc \
	$(top_srcdir)/src/xdpd/fwd-modules/gnu_linux/io/iomanager_c_wrapper.cc \
	$(top_srcdir)/src/xdpd/fwd-modules/gnu_linux/io/bufferpool.cc \
	$(top_srcdir)/src/xdpd/fwd-modules/gnu_linux/io/bufferpool_c_wrapper.cc \
	$(top_srcdir)/src/xdpd/fwd-modules/gnu_linux/io/datapacketx86_c_wrapper.cc \
	$(top_srcdir)/src/xdpd/fwd-modules/gnu_linux/io/datapacketx86.cc \
	$(top_srcdir)/src/xdpd/fwd-modules/gnu_linux/io/datapacket_storage.cc \
	$(top_srcdir)/src/xdpd/fwd-modules/gnu_linux/io/datapacket_storage_c_wrapper.cc \
	$(top_srcdir)/src/xdpd/fwd-modules/gnu_linux/io/packet_classifiers/packetclassifier.cc \
	$(top_srcdir)/src/xdpd/fwd-modules/gnu_linux/io/packet_classifiers/rofl_pktclassifier.cc \
	$(top_srcdir)/src/xdpd/fwd-modules/gnu_linux/io/ports/ioport.cc \
	$(top_srcdir)/src/xdpd/fwd-modules/gnu_linux/io/ports/mockup/ioport_mockup.cc \
	$(top_srcdir)/src/xdpd/fwd-modules/gnu_linux/io/ports/mmap/ioport_mmap.cc \
	$(top_srcdir)/src/xdpd/fwd-modules/gnu_linux/io/ports/mmap/mmap_int.cc \
	$(top_srcdir)/src/xdpd/fwd-modules/gnu_linux/io/scheduler/epoll_ioscheduler.cc \
	$(top_srcdir)/src/xdpd/fwd-modules/gnu_linux/processing/processingmanager.cc \
	$(top_srcdir)/src/xdpd/fwd-modules/gnu_linux/bg_taskmanager.c \
	$(top_srcdir)/src/xdpd/fwd-modules/gnu_linux/bg_taskmanager.h \
	platform_hooks_of12_mockup.cc 
		
SHARED_LIBS=\
	-lrofl \
	-lrofl_pipeline \
	-lcppunit \
	-lpthread
	

#Simple portmockup with mockup matches	
test_portmockup_matchesmockup_SOURCES = \
	$(SHARED_SRC) \
	pipeline_packet_mockup.c \
	test_portmockup.cc
	
# 	$(top_srcdir)/src/xdpd/fwd-modules/gnu_linux/background_tasks_manager.h

test_portmockup_matchesmockup_LDADD = \
	$(SHARED_LIBS)

#Simple portmockup with non-mockup matches	
test_portmockup_SOURCES = \
	$(SHARED_SRC) \
	$(top_srcdir)/src/xdpd/fwd-modules/gnu_linux/pipeline-imp/packet.c \
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
	$(top_srcdir)/src/xdpd/fwd-modules/gnu_linux/pipeline-imp/packet.c \
	test_portmockup_multiport.cc
test_portmockup_multiport_LDADD = \
	$(SHARED_LIBS)
	
#MMAP
test_portmmap_SOURCES = \
	$(SHARED_SRC) \
	$(top_srcdir)/src/xdpd/fwd-modules/gnu_linux/pipeline-imp/packet.c \
	test_mmapport.cc
test_portmmap_LDADD = \
	$(SHARED_LIBS)

if DEBUG
#test for the expiration of buffers in the datapacket_storage
test_storage_packets_expiration_SOURCES = \
	$(SHARED_SRC) \
	$(top_srcdir)/src/xdpd/fwd-modules/gnu_linux/pipeline-imp/packet.c \
	test_storage_packets_expiration.cc
test_storage_packets_expiration_LDADD = \
	$(SHARED_LIBS)
endif
	
#test for the hcl notifications for port events (add, delete & status change)
test_port_status_SOURCES = \
	$(SHARED_SRC) \
	$(top_srcdir)/src/xdpd/fwd-modules/gnu_linux/pipeline-imp/packet.c \
	test_port_status.cc
test_port_status_LDADD = \
	$(SHARED_LIBS)
	
	
TESTS = \
	test_launcher.sh

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


