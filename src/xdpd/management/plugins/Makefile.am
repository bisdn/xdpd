MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = $(PLUGIN_DIRS)
EXTRA_SUBDIRS = $(PLUGIN_DIRS)

noinst_LTLIBRARIES = libxdpd_mgmt_plugins.la
		
libxdpd_mgmt_plugins_la_SOURCES = plugin_manager_loader.cc 

#Never remove this line
libxdpd_mgmt_plugins_la_LIBADD = 

#
# Inclusion of the portion of Makefile for plugin
# which does add its plugin library to LIBADD.
#
# Note that it must always add to _LIBADD the library
# conditionally, if 
#

#Config
include config/Plugin.ma

#QMF
include qmf/Plugin.ma

#XMP
include xmp/Plugin.ma

#[+] Add yours here...

#Example (leave it always at the very end)
include example/Plugin.ma
