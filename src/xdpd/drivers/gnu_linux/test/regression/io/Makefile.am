MAINTAINERCLEANFILES = Makefile.in

AUTOMAKE_OPTIONS = no-dependencies

#Shared stuff
SHARED_SRC=\
	../of1x_cmm_mockup.c \
	$(top_srcdir)/src/hal-imp/driver.cc\
	$(top_srcdir)/src/io/iface_utils.cc \
	$(top_srcdir)/src/pipeline-imp/platform_hooks_of1x.cc \
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
	
CLASSIFIER_SRC=$(top_srcdir)/src/io/packet_classifiers/c_types_pktclassifier/c_types_pktclassifier.c \
		$(top_srcdir)/src/io/packet_classifiers/c_types_pktclassifier/autogen_pkt_types.c \
		$(top_srcdir)/src/io/packet_classifiers/packet_operations.cc
	
#datapacketx86test_SOURCES= $(top_srcdir)/src/io/datapacketx86.cc\
#			$(top_srcdir)/src/io/bufferpool.cc\
#			$(top_srcdir)/src/pipeline-imp/memory.c \
#			$(CLASSIFIER_SRC) \
#			datapacketx86test.cc 
#			
#datapacketx86test_LDADD= -lrofl -lcppunit -lpthread

#mmap_port_test_SOURCES = $(SHARED_SRC)\
#			$(CLASSIFIER_SRC) \
#			mmap_port_test.cc
	

			
#mmap_port_test_LDADD = -lrofl -lcppunit -lpthread -lrofl_pipeline 

#check_SCRIPTS = mmap_port_test.sh

#if DEBUG
#DEBUG_ONLY_TESTS= portmockuptest 
#endif

check_PROGRAMS = $(DEBUG_ONLY_TESTS)

TESTS = $(DEBUG_ONLY_TESTS) 
