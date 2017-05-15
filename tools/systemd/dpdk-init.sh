#!/bin/bash

HUGEMNT=/mnt/huge
HUGEPAGES=4
HUGEPGSZ=`cat /proc/meminfo  | grep Hugepagesize | cut -d : -f 2 | tr -d ' '`
DPDK_DEVBIND=/usr/local/sbin/dpdk-devbind

test -f /etc/sysconfig/dpdk && . /etc/sysconfig/dpdk

# these functions shamelessly taken from dpdk-setup.py script ;)
#

#
# Creates hugepage filesystem.
#
create_mnt_huge()
{
        #echo "Creating /mnt/huge and mounting as hugetlbfs"
        test -d $HUGEMNT || mkdir -p $HUGEMNT

        grep -s "$HUGEMNT" /proc/mounts > /dev/null
        if [ $? -ne 0 ] ; then
                mount -t hugetlbfs nodev $HUGEMNT
        fi
}

#
# Removes hugepage filesystem.
#
remove_mnt_huge()
{
        #echo "Unmounting /mnt/huge and removing directory"
        grep -s "$HUGEMNT" /proc/mounts > /dev/null
        if [ $? -eq 0 ] ; then
                umount $HUGEMNT
        fi

        if [ -d $HUGEMNT ] ; then
                rm -R $HUGEMNT
        fi
}

#
# Removes all reserved hugepages.
#
clear_huge_pages()
{
        #echo "Removing currently reserved hugepages"
        for d in /sys/devices/system/node/node? ; do
                echo 0 > $d/hugepages/hugepages-${HUGEPGSZ}/nr_hugepages
        done

        remove_mnt_huge
}

#
# Creates hugepages.
#
set_non_numa_pages()
{
        clear_huge_pages

        #echo "Reserving hugepages for non-NUMA system"
        echo $HUGEPAGES > /sys/kernel/mm/hugepages/hugepages-${HUGEPGSZ}/nr_hugepages

        create_mnt_huge
}

#
# Creates hugepages on specific NUMA nodes.
#
set_numa_pages()
{
        clear_huge_pages

        #echo "Reserving hugepages for NUMA system"
        for d in /sys/devices/system/node/node? ; do
                echo $HUGEPAGES > $d/hugepages/hugepages-${HUGEPGSZ}/nr_hugepages
        done

        create_mnt_huge
}


if [ ! -e $DPDK_DEVBIND ];
then
	echo "dpdk-devbind.py not found, aborting."
	exit -1	
fi
	

case "$1" in 
start)
	NUM_NUMA_NODES=$(lscpu | grep "NUMA node(s)" | wc -l)
	if [ $NUM_NUMA_NODES == "0" ];
	then
		#echo "set_non_numa_pages"
		set_non_numa_pages
	else
		#echo "set_numa_pages"
		set_numa_pages
	fi

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
		$DPDK_DEVBIND -u $IFACE
	done

	clear_huge_pages
	;;

*)
	echo "usage: $0 [start|stop]"
;;
esac


