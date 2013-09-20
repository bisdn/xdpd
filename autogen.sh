#!/bin/sh

#Generate Makefile.in's even if they are not going to be compiled
LIST=config/plugins_list.m4
echo "if test 1 == 0;then "> $LIST 

for DIR in `find src/xdpd/management/plugins -mindepth 1 -type d `;do
	echo "AC_CONFIG_FILES([
		$DIR/Makefile
	])">> $LIST	
done
echo "echo hhhhhhhhhhhhhhhhhhhhhhhhhhhhh;fi;">> $LIST 

export AUTOMAKE="automake --foreign -a"
autoreconf -f -i 
