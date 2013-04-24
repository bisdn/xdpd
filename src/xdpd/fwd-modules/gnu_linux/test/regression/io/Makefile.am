MAINTAINERCLEANFILES = Makefile.in

#Shared stuff
SHARED_SRC=\
	../of12_cmm_mockup.c \
	$(top_srcdir)/src/afa-imp/forwarding_module.c\
	$(top_srcdir)/src/io/iface_utils.cc \
	$(top_srcdir)/src/pipeline-imp/platform_hooks_of12.cc \
	$(top_srcdir)/src/pipeline-imp/pthread_lock.c \
	$(top_srcdir)/src/pipeline-imp/pthread_atomic_operations.c \
	$(top_srcdir)/src/util/ringbuffer.cc \
	$(top_srcdir)/src/util/ringbuffer_c_wrapper.cc \
	$(top_srcdir)/src/io/iomanager.cc \
	$(top_srcdir)/src/io/iomanager_c_wrapper.cc \
	$(top_srcdir)/src/io/bufferpool.cc \
	$(top_srcdir)/src/io/bufferpool_c_wrapper.cc \
	$(top_srcdir)/src/io/datapacketx86_c_wrapper.cc \
	$(top_srcdir)/src/io/datapacketx86.cc \
	$(top_srcdir)/src/io/datapacket_storage.cc \
	$(top_srcdir)/src/io/datapacket_storage_c_wrapper.cc \
	$(top_srcdir)/src/io/packet_classifiers/packetclassifier.cc \
	$(top_srcdir)/src/io/packet_classifiers/rofl_pktclassifier.cc \
	$(top_srcdir)/src/io/ports/ioport.cc \
	$(top_srcdir)/src/io/ports/mockup/ioport_mockup.cc \
	$(top_srcdir)/src/io/ports/mmap/ioport_mmap.cc \
	$(top_srcdir)/src/io/ports/mmap/mmap_int.cc \
	$(top_srcdir)/src/io/scheduler/epoll_ioscheduler.cc \
	$(top_srcdir)/src/processing/processingmanager.cc \
	$(top_srcdir)/src/bg_taskmanager.c \
	$(top_srcdir)/src/bg_taskmanager.h \
	$(top_srcdir)/src/pipeline-imp/memory.c \
	$(top_srcdir)/src/pipeline-imp/packet.c 
	
datapacketx86test_SOURCES= $(top_srcdir)/src/io/datapacketx86.cc\
			$(top_srcdir)/src/io/bufferpool.cc\
			$(top_srcdir)/src/pipeline-imp/memory.c \
			$(top_srcdir)/src/io/packet_classifiers/packetclassifier.cc \
			$(top_srcdir)/src/io/packet_classifiers/rofl_pktclassifier.cc \
			datapacketx86test.cc 
			
datapacketx86test_LDADD= -lrofl -lcppunit -lpthread

mmap_port_test_SOURCES = $(SHARED_SRC)\
			mmap_port_test.cc
	

			
mmap_port_test_LDADD = -lrofl -lcppunit -lpthread -lrofl_pipeline 

#check_SCRIPTS = mmap_port_test.sh

#if DEBUG
#DEBUG_ONLY_TESTS= portmockuptest 
#endif

check_PROGRAMS = mmap_port_test datapacketx86test $(DEBUG_ONLY_TESTS)

TESTS = mmap_port_test.sh datapacketx86test $(DEBUG_ONLY_TESTS) 
