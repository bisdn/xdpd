MAINTAINERCLEANFILES = Makefile.in

.PHONY: rofl-common rofl-datapath dpdk
.NOTPARALLEL:

#
# So here we are, battling with autotools again.
#
# Autotools subpackages would be the natural solution for this, but unfortunately
# they forgot that in most cases, and  dependencies is a good example,
# the configure options for the inner subpackages will in general be different
# than the ones in the main package, and likely be dependent on them.
#
# Note that CFLAGS, CXXFLAGS... are deliverately not passed to the inner subpackages,
# Use LIBS_CFLAGS, LIBS_CXXFLAGS etc... for that
#

#General variables
LIBS_DIR=$(abs_top_srcdir)/libs
LIBS_BUILD_DIR=$(abs_top_builddir)/libs

##
## ROFL common
##

#Defaults
ROFL_CM_AC_FLAGS=$(AC_CONFIGURE_ARGS)

#Makefile specific variables (at automake time)
RC_COMMIT_FILE=$(LIBS_BUILD_DIR)/.rofl-common-commit

#ROFL-common target
rofl-common:
	@LIB="rofl-common";									\
	PREV_COMMIT= ;										\
	if test -f $(RC_COMMIT_FILE) ; then							\
		PREV_COMMIT=`cat $(RC_COMMIT_FILE)` ;						\
	fi ;											\
	CURR_COMMIT=`cd $(LIBS_DIR)/$$LIB && git log -1 --pretty=%H  || echo ` ;		\
	if test "$$PREV_COMMIT" = "$$CURR_COMMIT" ; then					\
		echo Package \'$$LIB\' is up-to-date; 						\
	else											\
		echo Compiling \'$$LIB\'... ;							\
		OLD_PWD=$$PWD ;									\
		mkdir -p $(LIBS_BUILD_DIR)/$$LIB/build || exit -1 ;				\
		cd $(LIBS_DIR)/$$LIB/ || exit -1 ;						\
		sh autogen.sh || exit -1 ;							\
		cd $(LIBS_BUILD_DIR)/$$LIB/build || exit -1 ;					\
		CFLAGS="$(LIBS_CFLAGS)" CXXFLAGS="$(LIBS_CXXFLAGS)" LDFLAGS="$(LIBS_LDFLAGS)" 	\
		CPPFLAGS="$(LIBS_CPPFLAGS)" CC="$(CC)" CXX="$(CXX)" 				\
		$(LIBS_DIR)/$$LIB/configure $(ROFL_CM_AC_FLAGS) || exit -1 ; 			\
		$(MAKE) || exit -1 ; 								\
		cd $$OLD_PWD || exit -1 ;							\
		touch $(RC_COMMIT_FILE) || exit -1 ; 						\
		echo $$CURR_COMMIT > $(RC_COMMIT_FILE) || exit -1 ; 				\
	fi

clean-rofl-common:
	@cd $(builddir)/rofl-common/build && $(MAKE) clean
	@rm -f $(builddir)/.rofl-common-commit

maintainer-clean-rofl-common: clean-rofl-common
	@cd $(builddir)/rofl-common/build && $(MAKE) maintainer-clean


##
## ROFL datapath
##

#Defaults
ROFL_DP_AC_FLAGS=""
if DRIVER_HAS_INLINE_SUPPORT
if !USER_OVERRIDES_PIPELINE_FLAGS
ROFL_DP_AC_FLAGS+= --with-pipeline-lockless --with-pipeline-platform-funcs-inlined
endif
endif
ROFL_DP_AC_FLAGS+=$(AC_CONFIGURE_ARGS)

#Makefile specific variables (at automake time)
RD_COMMIT_FILE=$(LIBS_BUILD_DIR)/.rofl-datapath-commit

#ROFL-datapath target
rofl-datapath:
	@LIB="rofl-datapath";									\
	PREV_COMMIT= ;										\
	if test -f $(RD_COMMIT_FILE) ; then							\
		PREV_COMMIT=`cat $(RD_COMMIT_FILE)` ;						\
	fi ;											\
	CURR_COMMIT=`cd $(LIBS_DIR)/$$LIB && git log -1 --pretty=%H  || echo ` ;		\
	if test "$$PREV_COMMIT" = "$$CURR_COMMIT" ; then					\
		echo Package \'$$LIB\' is up-to-date; 						\
	else											\
		echo Compiling \'$$LIB\'... ;							\
		OLD_PWD=$$PWD ;									\
		mkdir -p $(LIBS_BUILD_DIR)/$$LIB/build || exit -1 ;				\
		cd $(LIBS_DIR)/$$LIB/ || exit -1 ;						\
		sh autogen.sh || exit -1 ;							\
		cd $(LIBS_BUILD_DIR)/$$LIB/build || exit -1 ;					\
		CFLAGS="$(LIBS_CFLAGS)" CXXFLAGS="$(LIBS_CXXFLAGS)" LDFLAGS="$(LIBS_LDFLAGS)" 	\
		CPPFLAGS="$(LIBS_CPPFLAGS)" CC="$(CC)" CXX="$(CXX)" 				\
		$(LIBS_DIR)/$$LIB/configure $(ROFL_DP_AC_FLAGS) || exit -1 ; 			\
		$(MAKE) || exit -1 ; 								\
		cd $$OLD_PWD || exit -1 ;							\
		touch $(RD_COMMIT_FILE) || exit -1 ; 						\
		echo $$CURR_COMMIT > $(RD_COMMIT_FILE) || exit -1 ; 				\
	fi


clean-rofl-common-datapath:
	@cd $(builddir)/rofl-datapath/build && $(MAKE) clean
	@rm -f $(builddir)/.rofl-datapath-commit


maintainer-clean-rofl-datapath: clean-rofl-common-datapath
	@cd $(builddir)/rofl-datapath/build && $(MAKE) maintainer-clean


##
## DPDK
##

if WITH_DPDK

#Makefile specific variables (at automake time)
DPDK_COMMIT_FILE=$(LIBS_BUILD_DIR)/.dpdk-commit

dpdk:
	@LIB="dpdk";									\
	PREV_COMMIT= ;									\
	if test -f $(DPDK_COMMIT_FILE) ; then						\
		PREV_COMMIT=`cat $(DPDK_COMMIT_FILE)` ;					\
	fi ;										\
	CURR_COMMIT=`cd $(LIBS_DIR)/$$LIB && git log -1 --pretty=%H  || echo ` ;	\
	if test "$$PREV_COMMIT" = "$$CURR_COMMIT" ; then				\
		echo Package \'$$LIB\' is up-to-date; 					\
	else										\
		echo Compiling \'$$LIB\' \(T=$(DPDK_TARGET)\)... ;			\
		OLD_PWD=$$PWD ;								\
		mkdir -p $(LIBS_BUILD_DIR)/$$LIB || exit -1 ;				\
		cd $(LIBS_DIR)/$$LIB/ || exit -1 ;					\
		$(MAKE) config T=$(DPDK_TARGET) ; 					\
		if test "$(LIBS_BUILD_DIR)/$$LIB" == "$(abs_srcdir)/dpdk" ; then	\
			$(MAKE) || exit -1 ;						\
		else									\
			DESTDIR=$(LIBS_BUILD_DIR)/$$LIB $(MAKE) || exit -1 ;		\
		fi ;									\
		touch $(DPDK_COMMIT_FILE) || exit -1 ; 					\
		echo $$CURR_COMMIT > $(DPDK_COMMIT_FILE) || exit -1 ; 			\
	fi

clean-dpdk:
	@rm -rf $(builddir)/dpdk/build
	@rm -f $(builddir)/.dpdk-commit

maintainer-clean-dpdk: clean-dpdk

endif


##
## Main targets
##

#All targets
if WITH_DPDK
all: rofl-common rofl-datapath dpdk
clean-libs: clean-rofl-common clean-rofl-common-datapath clean-dpdk
maintainer-clean-local: maintainer-clean-rofl-common maintainer-clean-rofl-datapath maintainer-clean-dpdk
else
all: rofl-common rofl-datapath
clean-libs: clean-rofl-common clean-rofl-common-datapath
maintainer-clean-local: maintainer-clean-rofl-common maintainer-clean-rofl-datapath
endif

check: all
