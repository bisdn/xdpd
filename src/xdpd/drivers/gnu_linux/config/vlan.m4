#
# checking for support for tp_vlan_tpid in struct tpacketv2_hdr
# available since kernel 3.14
#


UNAME=`uname -r`

#Check for if_packet (MMAP)
AC_CHECK_HEADER([linux/if_packet.h], [], [AC_ERROR("Missing Linux kernel headers for kernel '$UNAME'")])

#Check whether we have a kernel that supports tp_vlan_tpid (>= 3.14.X)
AC_MSG_CHECKING(whether kernel supports S-TAG/I-TAG)
AC_COMPILE_IFELSE(
	[AC_LANG_PROGRAM(
		[#include <linux/if_packet.h>],
		[
			struct tpacket2_hdr hdr;
			hdr.tp_vlan_tpid = 2;
			(void)hdr;
		]
	)],
	[old_kernel="no"],
	[old_kernel="yes"]
)

if test "$old_kernel" = "yes"; then
    AC_MSG_RESULT(no)
    AC_MSG_WARN([kernel does not support vlan S-TAG/I-TAG headers. Consider upgrading to at least kernel version 3.14.x])
    sleep 5
else
    AC_MSG_RESULT(yes)
    AC_DEFINE([KERNEL_STAG_SUPPORT])
fi
