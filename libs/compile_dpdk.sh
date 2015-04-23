
##
## Little script to call DPDK compilation
##
## This script wouldn't be necessary if I would know how (or I would have the time to learn how)
## to make it in the Makefile target
##

#Recover parameters
LIB=dpdk
LIBS_DIR=$1/libs/
DPDK_TARGET=$2

#Define the commit file
COMMIT_FILE=$LIBS_DIR/.$LIB-commit

PREV_COMMIT=
if test -f $COMMIT_FILE ;then
	PREV_COMMIT=`cat $COMMIT_FILE`
fi

CURR_COMMIT=`cd $LIBS_DIR/$LIB && git log -1 --pretty=%H || echo`

if test "$PREV_COMMIT" = "$CURR_COMMIT" ;then
	#Print a nice trace
	echo Package \'$LIB\' is up-to-date
else
	#Print a nice trace
	echo Compiling \'$LIB\' \(T=$DPDK_TARGET\)...
	OLD_PWD=$PWD
	cd $LIBS_DIR/$LIB/ || exit -1
	make config T=$DPDK_TARGET || exit -1
	make || exit -1
	cd $OLD_PWD
	touch $COMMIT_FILE || exit -1
	echo $CURR_COMMIT > $COMMIT_FILE || exit -1
fi
