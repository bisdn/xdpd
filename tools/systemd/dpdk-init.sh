#!/bin/bash

HUGEMNT=/mnt/huge
HUGEPAGES=4
HUGEPGSZ=`cat /proc/meminfo  | grep Hugepagesize | cut -d : -f 2 | tr -d ' '`
DPDK_DEVBIND=/usr/local/sbin/dpdk-devbind

test -f /etc/sysconfig/dpdk && . /etc/sysconfig/dpdk

init_huge_pages()
{
	echo "reserving huge pages ..."
	echo > .echo_tmp
        	for d in /sys/devices/system/node/node? ; do
                	node=$(basename $d)
                	echo "$PAGES pages for $node "
            		echo "echo $PAGES > $d/hugepages/hugepages-${HUGEPGSZ}/nr_hugepages" >> .echo_tmp
        	done
	sudo sh .echo_tmp
	rm -f .echo_tmp
	
        echo "mounting hugetlbfs ..."
        test -d $HUGEMNT || mkdir -p $HUGEMNT

        grep -s $HUGEMNT /proc/mounts > /dev/null
        if [ $? -ne 0 ] ; then
                sudo mount -t hugetlbfs nodev $HUGEMNT
        fi
}

if [ ! -e $DPDK_DEVBIND ];
then
	echo "dpdk-devbind.py not found, aborting."
	exit -1	
fi
	

case "$1" in 
start)
	init_huge_pages

	# bind interfaces to DPDK driver
	for IFACE in $DPDK_BIND_INTERFACES;
	do
		$DPDK_DEVBIND -b igb_uio $IFACE
	done
	;;
stop)
	# unbind interfaces from DPDK driver
	for IFACE in $DPDK_BIND_INTERFACES;
	do
		$DPDK_DEVBIND -u igb_uio $IFACE
	done
	;;

*)
	echo "usage: $0 [start|stop]"
;;
esac


