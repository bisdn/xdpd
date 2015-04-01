
Config plugin
=============

The `config` plugin is the default management plugin. During boostrap, `config` plugin reads a configuration file (specified in the command line via `-c`) and creates the necessary LSIs, attches ports and configures other LSI parameters.

Supported platforms (drivers)
=============================

All, as for any management plugin in xDPd.

Requirements
============

- libconfig C++ lib (libconfig++-dev in Debian/Ubuntu systems)


Explicit build
==============

Defaut `configure` already includes `config` plugin, and it is equivalent to:

	sh# ../configure --with-plugins="config" 
	sh# make  
	sh# make install

`config` plugin can be combined with other plugins, e.g. `rest` or `xmp`. It is **not** mandatory that `config` plugins is compiled in, provided that other plugins can perform the initialization tasks (e.g. `xmp`).

Example config files
====================

Two examples are provided, but not distributed during `make install`:

* `src/xdpd/management/plugins/config/example.cfg` : 1 LSI, simple configuration
* `src/xdpd/management/plugins/config/example_complex.cfg` : 2 LSIs running different versions, connected through a vlink, and the majority of optional parameters
