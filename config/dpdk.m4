#Load DPDK libs in the include_ and library_path

if test "$WITH_DPDK" = "yes"; then

AC_MSG_CHECKING(and adding support for the DPDK library...)
#Adding the includes
DPDK_INCLUDES="-I$XDPD_SRCDIR/libs/dpdk/build/include"

#Library path
DPDK_LDFLAGS="-L$XDPD_SRCDIR/libs/dpdk/build/lib"

#Add them
CPPFLAGS="$DPDK_INCLUDES $CPPFLAGS"
LDFLAGS="$DPDK_LDFLAGS $LDFLAGS "

#Set target
#Todo add other architectures and  OSs
DPDK_TARGET=x86_64-native-linuxapp-$CC

AC_SUBST(DPDK_TARGET)
AC_MSG_RESULT([added ($DPDK_TARGET)])

fi
