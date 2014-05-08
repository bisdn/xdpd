#!/bin/sh

PKG_NAME="xdpd"
PKG_NAME_DBG="xdpd-dbg"
VERSION=""
RELEASE=""
REQUIRES+=""
LICENSE="MPL"
GROUP="net"
MAINTAINER="dev@xdpd.org"

IS_DEBUG=""

dump_result(){
	echo "Detected configuration"
	echo "Package name: $1"
	echo "Version: $VERSION"
	echo "Release: $RELEASE"
	echo "Requires: $REQUIRES"
	echo "License: $LICENSE"
	echo "Package group: $GROUP"
	echo "Maintainer: $MAINTAINER"
	echo "Replaces and conflicts: $2"

	read -p "Proceed? " -n 1 -r
	echo    # (optional) move to a new line
	if [[ $REPLY =~ ^[Yy]$ ]]
	then
		echo	
	else
		exit 0
	fi	
}

is_debug(){
	IS_NOT_DEBUG=`cat config.log| grep NDEBUG`
}

get_version(){
	VERSION=`cat ../VERSION | sed s/dev//g | sed s/RC.*//g | tr -d v`
}

get_release(){
	RELEASE=`cat ../VERSION`
}

generate_pkg(){
	sudo checkinstall -D -y --pkgname=$1 --pkgversion=$VERSION --pkgrelease=$RELEASE --pkglicense=$LICENSE --pkggroup=$GROUP --maintainer=$MAINTAINER --provides=$1 --requires=$REQUIRES --conflicts=$2 --replaces=$2
}

#
# Main routine
#

#Detect stuff
is_debug
get_version
get_release

#Generate the package
if test -z $IS_NOT_DEBUG;
then
	dump_result $PKG_NAME_DBG $PKG_NAME
	generate_pkg $PKG_NAME_DBG $PKG_NAME
else
	dump_result  $PKG_NAME $PKG_NAME_DBG
	generate_pkg $PKG_NAME $PKG_NAME_DBG
fi
