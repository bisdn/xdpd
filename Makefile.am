AUTOMAKE_OPTIONS = subdir-objects 1.5 foreign

MAINTAINERCLEANFILES = \
	$(top_srcdir)/aclocal.m4 \
	$(top_srcdir)/configure \
	$(top_srcdir)/Makefile.in

SUBDIRS = libs src doc test

#Could be improved.. 
.PHONY: doc
doc:
	@cd doc/ && make doc		

install-exec-hook:
	@echo "Install"
uninstall-hook:
	@echo "Uninstall"

clean-libs:
	cd libs && $(MAKE) clean-libs

maintainer-clean-local:
	rm -rf $(top_srcdir)/m4
	rm -rf $(top_srcdir)/build-aux
