#
# checking for support for tp_vlan_tpid in struct tpacketv2_hdr
# available since kernel 3.14
#

AC_MSG_CHECKING([for vlan S-TAG/I-TAG header support in kernel])
UNAME=`uname -r`
kernel_version=`sed -n 's/^VERSION = //p' "/lib/modules/$(uname -r)/build/Makefile" `
kernel_patchlevel=`sed -n 's/^PATCHLEVEL = //p' "/lib/modules/$(uname -r)/build/Makefile" `
kernel_sublevel=`sed -n 's/^SUBLEVEL = //p' "/lib/modules/$(uname -r)/build/Makefile" `

if ( test X"$kernel_version" = X || test X"$kernel_patchlevel" = X ); then
    AC_MSG_WARN([unable to detect kernel version, continuing anyway, vlan S-TAG/I-TAG headers may not work])
elif ( test "$kernel_version" -ge 3 && test "$kernel_patchlevel -ge 14" ); then
    AC_MSG_RESULT([found])
    AC_DEFINE(KERNEL_STAG_SUPPORT)
else
    AC_MSG_WARN([kernel does not support vlan S-TAG/I-TAG headers, consider upgrading to at least kernel version 3.14.x])
fi

