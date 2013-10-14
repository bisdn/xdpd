#!/bin/sh
set -e

export AUTOMAKE="automake --foreign -a"
autoreconf -f -i

#Patching libtool.m4 for Debian/Ubuntu (link_all_deplibs => unknown)
FILES=`find . -name libtool.m4`
PATCHED=

for file in $FILES;
do
	mv -f $file $file.orig
	sed 's/link_all_deplibs *, *\$1 *) *= *no/link_all_deplibs, \$1)=unknown/' $file.orig > $file	

	if diff $file $file.orig >/dev/null ; then
		#Not in Ubuntu
		continue;	
	else
		PATCHED="yes"
	fi
done

#Nice message		
if [ -n "$PATCHED" ]; then
	echo "INFO: Debian-like libtool.m4 script(s) detected. The following scripts have been patched setting 'link_all_deplibs=no' to 'link_all_deplibs=unknown':"
	for file in $FILES;
	do
		echo "$file"
	done
fi
