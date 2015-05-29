#############################
# CHECKING Hardware support #
#############################
AC_MSG_CHECKING(for hardware support to be compiled)

#With
AC_ARG_WITH(hw-support, AS_HELP_STRING([--with-hw-support="driver-name"],[Compile with hardware support for platform 'driver-name' [default=gnu-linux]])
			  [Available platforms:]
				[gnu-linux: compile GNU/Linux hardware support]
				[gnu-linux-dpdk: compile GNU/Linux Intel DPDK support]
				[bcm: compile Broadcom hardware support]
				[octeon: compile OCTEON hardware support]
				[netfpga10g: compile NetFPGA 10G hardware code (HW code not included)]
				[example: compile the example forwarding module code]
,
 [ 
	#They provide a platform 
	HW=$withval
 ],[
	#Use default
	HW="gnu-linux"
])

#General settings
DRIVER_HAS_INLINE_SUPPORT="yes"

#Add subpackages conditionally
if ( test "$HW" = "gnu-linux");then
	msg="$msg GNU/Linux"
	AC_DEFINE(HW_GNU_LINUX)
	PLATFORM=gnu_linux
	AC_CONFIG_SUBDIRS([src/xdpd/drivers/gnu_linux])
fi
if ( test "$HW" = "gnu-linux-dpdk");then
	msg="$msg GNU/Linux Intel DPDK"
	AC_DEFINE(HW_GNU_LINUX_DPDK)
	PLATFORM=gnu_linux_dpdk
	AC_CONFIG_SUBDIRS([src/xdpd/drivers/gnu_linux_dpdk])
#In DPDK-1.7, Poll Mode Drivers (PMD) register with DPDK from static constructors. If linker decides
#not to include the corresponding object file (which is usually the case since PMDs mostly contain
#static functions, only accessed indirectly), then the PMD is not registered.
#To fix this, PMDs have to be linked in using the --whole-archive linker flag.
#Unfortunately, autoconf+libtool don't seem to provide a way to include these extra linker flags in
#the .la file. Hence, they have to be provided here at the top-level...
	xdpd_HW_LDFLAGS=" -Wl,--whole-archive \
		-Wl,-lrte_pmd_e1000 \
		-Wl,-lrte_pmd_ixgbe \
		-Wl,-lrte_pmd_i40e \
		-Wl,-lrte_pmd_vmxnet3_uio \
		-Wl,-lrte_pmd_ring \
		-Wl,-lrte_pmd_virtio_uio \
		-Wl,--no-whole-archive"

	#Onboard DPDK compilation
	WITH_DPDK="yes"
fi
if( test "$HW" = "bcm");then
	msg="$msg Broadcom"
	AC_DEFINE(HW_BCM)
	PLATFORM=bcm
	AC_CONFIG_SUBDIRS([src/xdpd/drivers/bcm])
	DRIVER_HAS_INLINE_SUPPORT="no"
fi	
if( test "$HW" = "octeon");then
	msg="$msg OCTEON"
	AC_DEFINE(HW_OCTEON)
	PLATFORM=octeon
	AC_CONFIG_SUBDIRS([src/xdpd/drivers/octeon])
fi
if( test "$HW" = "netfpga10g");then
	msg="$msg NetFPGA-10G"
	AC_DEFINE(HW_NETFPGA10G)
	PLATFORM=netfpga10g
	AC_CONFIG_SUBDIRS([src/xdpd/drivers/netfpga10g])
	DRIVER_HAS_INLINE_SUPPORT="no"
fi
if( test "$HW" = "example");then
	msg="$msg Example platform"
	AC_DEFINE(HW_EXAMPLE)
	PLATFORM=example
	AC_CONFIG_SUBDIRS([src/xdpd/drivers/example])
	DRIVER_HAS_INLINE_SUPPORT="no"
fi
#[+]Add your platform here...

if test -z $PLATFORM; then
	AC_MSG_RESULT(error)
  	AC_ERROR(Unknown platform: $HW)
fi

#Print fancy message
AC_MSG_RESULT($msg)

#Print a nice message
AC_MSG_CHECKING(whether driver supports rofl-pipeline inlining...)
AC_MSG_RESULT($DRIVER_HAS_INLINE_SUPPORT)

AM_CONDITIONAL([DRIVER_HAS_INLINE_SUPPORT], [test "$DRIVER_HAS_INLINE_SUPPORT" = "yes"])
AM_CONDITIONAL([WITH_DPDK], [test "$WITH_DPDK" = "yes"])
AC_SUBST(PLATFORM)
AC_SUBST(xdpd_HW_LDFLAGS)

