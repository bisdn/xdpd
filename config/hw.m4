#############################
# CHECKING Hardware support #
#############################
AC_MSG_CHECKING(hardware support to be compiled)

AC_ARG_ENABLE(gnu-linux,
	AS_HELP_STRING([--enable-gnu-linux], [compile GNU/Linux hardware support [default=no]])
			, , enable_gnu_linux="no")
AC_ARG_ENABLE(gnu-linux-dpdk,
	AS_HELP_STRING([--enable-gnu-linux-dpdk], [compile GNU/Linux Intel DPDK support [default=no]])
			, , enable_gnu_linux_dpdk="no")
AC_ARG_ENABLE(bcm,
	AS_HELP_STRING([--enable-bcm], [compile Broadcom hardware support [default=no]])
			, , enable_bcm="no")
AC_ARG_ENABLE(octeon,
	AS_HELP_STRING([--enable-octeon], [compile OCTEON 5650 hardware support [default=no]])
			, , enable_octeon="no")
AC_ARG_ENABLE(netfpga10g,
	AS_HELP_STRING([--enable-netfpga10g], [compile NetFPGA 10G hardware code (HW code not included) [default=no]])
			, , enable_netfpga10g="no")
AC_ARG_ENABLE(example,
	AS_HELP_STRING([--enable-example], [compile the Forwarding Module example code [default=no]])
			, , enable_example="no")
#[+] Add your platform here


#Make default GNU/Linux if not defined. 
#[+] Makesure you add your platform here
if ( test "$enable_gnu_linux_dpdk" = "no" ) &&
   ( test "$enable_gnu_linux" = "no" ) &&
   ( test "$enable_bcm" = "no" ) &&
   ( test "$enable_octeon" = "no" ) &&
   ( test "$enable_netfpga10g" = "no" ) &&
   ( test "$enable_example" = "no" ); then
   
   enable_gnu_linux="yes"

fi

#Print messages
if ( test "$enable_gnu_linux" = "yes" );then
	msg="$msg GNU/Linux"
	AC_DEFINE(ENABLE_GNU_LINUX)
	PLATFORM=gnu_linux
	AC_CONFIG_SUBDIRS([src/xdpd/fwd-modules/gnu_linux])
fi
if ( test "$enable_gnu_linux_dpdk" = "yes" );then
	msg="$msg GNU/Linux Intel DPDK"
	AC_DEFINE(ENABLE_GNU_LINUX_DPDK)
	PLATFORM=gnu_linux_dpdk
	AC_CONFIG_SUBDIRS([src/xdpd/fwd-modules/gnu_linux_dpdk])
fi
if( test "$enable_bcm" = "yes" );then
	msg="$msg Broadcom"
	AC_DEFINE(ENABLE_BCM)
	PLATFORM=bcm
	AC_CONFIG_SUBDIRS([src/xdpd/fwd-modules/bcm])
fi	
if( test "$enable_octeon" = "yes" );then
	msg="$msg OCTEON 5650"
	AC_DEFINE(ENABLE_OCTEON5650)
	PLATFORM=octeon5650
	AC_CONFIG_SUBDIRS([src/xdpd/fwd-modules/octeon5650])
fi
if( test "$enable_netfpga10g" = "yes" );then
	msg="$msg NetFPGA-10G"
	AC_DEFINE(ENABLE_NETFPGA10G)
	PLATFORM=netfpga10g
	AC_CONFIG_SUBDIRS([src/xdpd/fwd-modules/netfpga10g])
fi
if( test "$enable_example" = "yes" );then
	msg="$msg Example platform"
	AC_DEFINE(ENABLE_EXAMPLE)
	PLATFORM=example
	AC_CONFIG_SUBDIRS([src/xdpd/fwd-modules/example])
fi
#[+]Add your platform here...


#Print fancy message
AC_MSG_RESULT($msg)
AC_SUBST(PLATFORM)


