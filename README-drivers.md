
Drivers
=======

xDPd supports several platform backends or "drivers".

Available platforms
-------------------

Shipped with all xDPd instances:
- GNU/Linux amd64/x86 user-space with MMAP suppport (`gnu-linux`). open-source
- GNU/Linux Intel's DPDK accelerated driver (`gnu-linux-dpdk`). open-source
- NetFGPA-10G (`netfpga10g`). open-source. 

Also available:

- Broadcom Triumph2 (`bcm`).
- Octeon network processors (`octeon`). 
- Other: refer to the wiki page for more details

The default is user-space `gnu-linux`

Read more
=========

Please have a look to the specific details of each driver: 

src/xdpd/drivers/{driver-code-name}/README

Note that drivers may, or may not, make use of xdpd's `-e` option to receive additional parameter.

Build driver support
====================

To compile support for a driver:

	sh# ../configure --with-hw-support="<driver-code-name>" 
	sh# make  
	sh# make install

Default is:

	sh# ../configure --with-hw-support="gnu-linux" 
