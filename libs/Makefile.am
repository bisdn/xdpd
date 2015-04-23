MAINTAINERCLEANFILES = Makefile.in

.PHONY: rofl_common rofl_datapath
.NOTPARALLEL:

#SUBDIRS = rofl-common rofl-datapath


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
ROFL_DP_AC_FLAGS+= --with-pipeline-lockless --with-pipeline-platform-funcs-inlined
endif

#All targets
all: rofl_common rofl_datapath

#ROFL related targets
rofl_common:
	@$(SHELL) $(top_srcdir)/libs/compile_autotools_pkg.sh $(top_srcdir) "rofl-common" "$(ROFL_CM_AC_FLAGS)"

rofl_datapath:
	@$(SHELL) $(top_srcdir)/libs/compile_autotools_pkg.sh $(top_srcdir) "rofl-datapath" "$(ROFL_DP_AC_FLAGS)"


#Force recompiling
maintainer-clean-local:
	rm -f $(top_srcdir)/libs/.*-commit

if WITH_DPDK
#SUBDIRS += dpdk
endif
