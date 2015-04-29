
Node orchestrator plugin
========================

The `node_orchestrator` is used to interact with the Universal Node orchestrator (https://github.com/bisdn/un-orchestrator).

Supported platforms (drivers)
=============================

All

Requirements
============

- json spirit (https://github.com/sirikata/json-spirit)


Explicit build
==============

	sh# ../configure --with-hw-support=gnu-linux-dpdk --with-plugins="node_orchestrator"
	sh# make  
	sh# make install

`node_orchestrator` plugin can be combined with other plugins, e.g. `rest` or `xmp`.

