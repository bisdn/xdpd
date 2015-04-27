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
# Note that CFLAGS, CXXFLAGS... are deliverately not passed to the inner subpackages
#

##
## ROFL common
##

#Defaults
ROFL_CM_AC_FLAGS=

if DEBUG
ROFL_CM_AC_FLAGS+= --enable-debug
endif

#ROFL-common target
rofl-common:
	@$(SHELL) $(top_srcdir)/libs/compile_autotools_pkg.sh $(top_srcdir) "rofl-common" "$(ROFL_CM_AC_FLAGS)"

clean-rofl-common:
	@cd $(top_srcdir)/libs/rofl-common/build && $(MAKE) clean
	@rm -f $(top_srcdir)/libs/.rofl-common-commit

maintainer-clean-rofl-common: clean-rofl-common
	@cd $(top_srcdir)/libs/rofl-common/build && $(MAKE) maintainer-clean


##
## ROFL datapath
##

#Defaults
ROFL_DP_AC_FLAGS=

if DEBUG
ROFL_DP_AC_FLAGS+= --enable-debug
else

if DRIVER_HAS_INLINE_SUPPORT
ROFL_DP_AC_FLAGS+= --with-pipeline-lockless --with-pipeline-platform-funcs-inlined
endif

endif

#ROFL-datapath target
rofl-datapath:
	@$(SHELL) $(top_srcdir)/libs/compile_autotools_pkg.sh $(top_srcdir) "rofl-datapath" "$(ROFL_DP_AC_FLAGS)"

clean-rofl-common-datapath:
	@cd $(top_srcdir)/libs/rofl-datapath/build && $(MAKE) clean
	@rm -f $(top_srcdir)/libs/.rofl-datapath-commit

maintainer-clean-rofl-datapath: clean-rofl-common-datapath
	@cd $(top_srcdir)/libs/rofl-datapath/build && $(MAKE) maintainer-clean


##
## DPDK
##

if WITH_DPDK
dpdk:
	@$(SHELL) $(top_srcdir)/libs/compile_dpdk.sh $(top_srcdir) $(DPDK_TARGET) 

clean-dpdk:
	@rm -rf $(top_srcdir)/libs/dpdk/build/
	@rm -f $(top_srcdir)/libs/.dpdk-commit

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
