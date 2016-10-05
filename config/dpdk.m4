#Load DPDK libs in the include_ and library_path

if test "$WITH_DPDK" = "yes"; then

AC_MSG_CHECKING(and adding support for the DPDK library...)
#Adding the includes
DPDK_INCLUDES="-I$XDPD_BUILDDIR/libs/dpdk/include -I$XDPD_BUILDDIR/libs/dpdk/include/dpdk"

#Library path
DPDK_LDFLAGS="-L$XDPD_BUILDDIR/libs/dpdk/lib"

#Add them
CPPFLAGS="$DPDK_INCLUDES $CPPFLAGS -msse4.2"
LDFLAGS="$DPDK_LDFLAGS $LDFLAGS "

#
# Determine the OS
#
AC_CANONICAL_HOST

OS=
case $host_os in
	linux*)
	OS=linux
	;;
	*BSD*)
	OS=bsd
	;;
	*)
	#Default Case
	AC_MSG_ERROR([Your platform is not currently supported])
	;;
esac

TARGET_CC=
case $CC in
	*gcc*)
	TARGET_CC=gcc
	;;
	*clang*)
	TARGET_CC=clang
	;;
	*icc*)
	TARGET_CC=icc
	;;
	*)
	#Default Case
	AC_MSG_ERROR([Could not deduce DPDK target for compiler '$CC'. DPDK supported compiler families: gcc, clang and icc])
	;;
esac

#Compose DPDK target string
DPDK_TARGET="x86_64-native-${OS}app-${TARGET_CC}"

AC_SUBST(DPDK_TARGET)
AC_MSG_RESULT([added ($DPDK_TARGET)])

fi
