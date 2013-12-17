AUTOMAKE_OPTIONS = subdir-objects 1.5 foreign

MAINTAINERCLEANFILES = $(top_srcdir)/aclocal.m4 \
	$(top_srcdir)/ar-lib \
	$(top_srcdir)/config.guess \
	$(top_srcdir)/config.sub \
	$(top_srcdir)/configure \
	$(top_srcdir)/depcomp \
	$(top_srcdir)/install-sh \
	$(top_srcdir)/ltmain.sh \
	$(top_srcdir)/Makefile.in \
	$(top_srcdir)/missing

SUBDIRS = src doc test

install-exec-hook:
	@echo "Install"
uninstall-hook:
	@echo "Uninstall"

maintainer-clean-local:
	rm -rf $(top_srcdir)/m4

#AM_CPPFLAGS=-I/home/andreas/local/rofl
#AM_CFLAGS=-I/home/andreas/local/rofl
#AM_LDFLAGS=-L/home/andreas/local/lib
