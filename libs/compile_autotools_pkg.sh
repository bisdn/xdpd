
##
## Little script to call autotools
##
## This script wouldn't be necessary if a) I would know how (or I would have the time to learn how)
## to make it in the Makefile target b) autotools subpackages could define different configure options
## than the main package
##

#Recover parameters
LIB=$2
AC__FLAGS=$3
MK__FLAGS=$4
LIBS_DIR=$1/libs/

#Define the commit file
COMMIT_FILE=$LIBS_DIR/.$LIB-commit
#echo Commit file: $COMMIT_FILE

PREV_COMMIT=
if test -f $COMMIT_FILE ;then
	PREV_COMMIT=`cat $COMMIT_FILE`
fi

CURR_COMMIT=`cd $LIBS_DIR/$LIB && git log -1 --pretty=%H || echo`
#echo Current commit: $CURR_COMMIT, prev: $PREV_COMMIT

if test "$PREV_COMMIT" = "$CURR_COMMIT" ;then
	#Print a nice trace
	echo Package \'$LIB\' is up-to-date

else
	#Print a nice trace
	echo Compiling \'$LIB\'...

	OLD_PWD=$PWD

	cd $LIBS_DIR/$LIB/ || exit -1
	sh autogen.sh || exit -1
	cd build || exit -1
	../configure $AC__FLAGS || exit -1
	make $MK__FLAGS || exit -1
	cd $OLD_PWD || exit -1
	touch $COMMIT_FILE || exit -1
	echo $CURR_COMMIT > $COMMIT_FILE || exit -1
fi
