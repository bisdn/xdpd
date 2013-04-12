MAINTAINERCLEANFILES = Makefile.in

SHARED_SRC=\
	../of12_cmm_mockup.c \
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
	$(top_srcdir)/src/xdpd/fwd-modules/gnu_linux/pipeline-imp/packet.c 
	

if DEBUG
processingmanagertest_SOURCES= $(SHARED_SRC)\
			../platform_hooks_of12_mockup.cc\
			processingmanagertest.cc 
			
processingmanagertest_LDADD= -lrofl -lcppunit -lpthread -lrofl_pipeline


check_PROGRAMS = processingmanagertest
TESTS = processingmanagertest 

endif
