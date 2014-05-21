# Set application version based on the git version

#Default
XDPD_VERSION="$PACKAGE_VERSION" #Unknown (no GIT repository detected)"
FILE_VERSION=`cat $srcdir/VERSION`

#Since AC_INIT caches VERSION; force an autogen.sh
if test "$XDPD_VERSION" != "$FILE_VERSION"; 
then
	AC_MSG_ERROR("xdpd version file has been updated($XDPD_VERSION => $FILE_VERSION). Please regenerate Autoconf state by calling autogen.sh again.")
fi


AC_CHECK_PROG(ff_git,git,yes,no)

if test $ff_git = no
then
	AC_MSG_RESULT([git not found!])
else

	AC_MSG_CHECKING([the build number(version)])
	
	if test -d $srcdir/.git ; then
		#Try to retrieve the build number
		_XDPD_GIT_BUILD=`git log -1 --pretty=%H`
		_XDPD_GIT_BRANCH=`git rev-parse --abbrev-ref HEAD`
		#XDPD_VERSION=`git describe --abbrev=0`
		_XDPD_GIT_DESCRIBE=`git describe --abbrev=40`

		AC_DEFINE_UNQUOTED([XDPD_BUILD],["$_XDPD_GIT_BUILD"])
		AC_DEFINE_UNQUOTED([XDPD_BRANCH],["$_XDPD_GIT_BRANCH"])
		AC_DEFINE_UNQUOTED([XDPD_DESCRIBE],["$_XDPD_GIT_DESCRIBE"])

	fi

	AC_MSG_RESULT($XDPD_VERSION)
fi

AC_SUBST([XDPD_VERSION],["$XDPD_VERSION"])
AC_DEFINE_UNQUOTED([XDPD_VERSION],["$XDPD_VERSION"])
