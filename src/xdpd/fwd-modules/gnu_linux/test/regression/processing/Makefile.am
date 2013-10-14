MAINTAINERCLEANFILES = Makefile.in

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
	$(top_srcdir)/src/io/packet_classifiers/packetclassifier.cc \
	$(top_srcdir)/src/io/packet_classifiers/rofl_pktclassifier.cc \
	$(top_srcdir)/src/io/packet_classifiers/static_pktclassifier.cc \
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
	$(top_srcdir)/src/pipeline-imp/packet.cc 
	

if DEBUG
processingmanagertest_SOURCES= $(SHARED_SRC)\
			../platform_hooks_of1x_mockup.cc\
			processingmanagertest.cc 
			
processingmanagertest_LDADD= -lrofl -lcppunit -lpthread -lrofl_pipeline


check_PROGRAMS = processingmanagertest
TESTS = processingmanagertest 

endif
