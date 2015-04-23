MAINTAINERCLEANFILES = Makefile.in

.PHONY: rofl_common rofl_datapath dpdk
.NOTPARALLEL:

#
# So here we are, battling with autotools again.
#
# Autotools subpackages would be the natural solution for this, but unfortunately
# they forgot that in most cases, and  dependencies is a good example,
# the configure options for the inner subpackages will in general be different
# than the ones in the main package, and likely be dependent on them.
#
# So we define them as regular Makefile targets. The `compile_autotools_pkg.sh` is I
# guess not needed, but I don't have the time to figure out how to embed it
# into the Makefile target without automake screwing it.
#
# Note that CFLAGS, CXXFLAGS... are deliverately not passed to the inner subpackages
#

#ROFL defaults
ROFL_CM_AC_FLAGS=
ROFL_DP_AC_FLAGS=

if DEBUG
ROFL_CM_AC_FLAGS+= --enable-debug
ROFL_DP_AC_FLAGS+= --enable-debug
else

if DRIVER_HAS_INLINE_SUPPORT
ROFL_DP_AC_FLAGS+= --with-pipeline-lockless --with-pipeline-platform-funcs-inlined
endif

endif

#All targets
if WITH_DPDK
all: rofl_common rofl_datapath dpdk
else
all: rofl_common rofl_datapath
endif

#ROFL related targets
rofl_common:
	@$(SHELL) $(top_srcdir)/libs/compile_autotools_pkg.sh $(top_srcdir) "rofl-common" "$(ROFL_CM_AC_FLAGS)"

rofl_datapath:
	@$(SHELL) $(top_srcdir)/libs/compile_autotools_pkg.sh $(top_srcdir) "rofl-datapath" "$(ROFL_DP_AC_FLAGS)"


#Force recompiling
clean-libs:
	@cd $(top_srcdir)/libs/rofl-common/build && $(MAKE) clean
	@cd $(top_srcdir)/libs/rofl-datapath/build && $(MAKE) clean
	@rm -f $(top_srcdir)/libs/.*-commit
	@rm -rf $(top_srcdir)/libs/dpdk/build/

maintainer-clean-local:
	@cd $(top_srcdir)/libs/rofl-common/build && $(MAKE) maintainer-clean
	@cd $(top_srcdir)/libs/rofl-datapath/build && $(MAKE) maintainer-clean
	@rm -f $(top_srcdir)/libs/.*-commit
	@rm -rf $(top_srcdir)/libs/dpdk/build/

if WITH_DPDK
dpdk:
	@$(SHELL) $(top_srcdir)/libs/compile_dpdk.sh $(top_srcdir) $(DPDK_TARGET) 
endif
