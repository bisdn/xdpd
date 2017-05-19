#!/bin/bash

HUGEMNT=/mnt/huge
HUGEPAGES=16
HUGEPGSZ=1048576kB
#HUGEPGSZ=cat /proc/meminfo  | grep Hugepagesize | cut -d : -f 2 | tr -d ' '
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


#
# Bind dpdk devices
#
bind_dpdk_devices()
{
	DPDK_BIND_INTERFACES=$(dpdk-devbind -s | grep "Virtual Function" | python -c "import sys; print ''.join([x.split()[0]+' ' for x in sys.stdin.readlines()]);")
	for IFACE in $DPDK_BIND_INTERFACES;
	do
		echo "$DPDK_DEVBIND -b igb_uio $IFACE"
		$DPDK_DEVBIND -b igb_uio $IFACE
	done
}


#
# Unbind dpdk devices
#
unbind_dpdk_devices()
{
	DPDK_BIND_INTERFACES=$(dpdk-devbind -s | grep "Virtual Function" | python -c "import sys; print ''.join([x.split()[0]+' ' for x in sys.stdin.readlines()]);")
	for IFACE in $DPDK_BIND_INTERFACES;
	do
		echo "$DPDK_DEVBIND -u $IFACE"
		$DPDK_DEVBIND -u $IFACE
	done
}


#
# create SR-IOV interfaces
#
create_sr_iov()
{
	# create SR-IOV interfaces
	j=0; 
	for HFACE in $SR_IOV_INTERFACES;
	do
		# get the interface name
		IFACE=$(echo $HFACE | python -c "import sys; print sys.stdin.read().split(':')[0];")
		# get the VLAN id
		VLAN=$(echo $HFACE | python -c "import sys; print sys.stdin.read().split(':')[1];")
		# get the target interface name (eno1 => em0) wtf ????
		JFACE=$(echo $IFACE | python -c "import sys; str=sys.stdin.read().split('eno'); print 'em'+str[1] if len(str) > 1 else str[0];")

		echo "echo $NUM_SRIOV > /sys/class/net/$IFACE/device/sriov_numvfs"
		echo $NUM_SRIOV > /sys/class/net/$IFACE/device/sriov_numvfs 
		sleep 3
                for i in $(seq 0 $((NUM_SRIOV-1))); 
		do
			HWADDR=$(ip link show ${JFACE}_${i} | grep "link/ether" | python -c "import sys; print sys.stdin.read().split()[1];")
			echo "ip link set $IFACE vf $i mac $HWADDR"
			ip link set $IFACE vf $i mac $HWADDR
			echo "ip link set $IFACE vf $i vlan $(($VLAN+i))"
			ip link set $IFACE vf $i vlan $(($VLAN+i))
		done
		((j++));
	done
}


#
# remove SR-IOV interfaces
#
remove_sr_iov()
{
	for IFACE in $SR_IOV_INTERFACES;
        do
                echo "echo 0 > /sys/class/net/$IFACE/device/sriov_numvfs"
                echo 0 > /sys/class/net/$IFACE/device/sriov_numvfs
        done
}


if [ ! -e $DPDK_DEVBIND ];
then
	echo "dpdk-devbind.py not found, aborting."
	exit -1	
fi
	

case "$1" in 
sriov)
	create_sr_iov
	;;
nosriov)
	remove_sr_iov
	;;
bind)
	bind_dpdk_devices
	;;
unbind)
	unbind_dpdk_devices
	;;
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

	create_sr_iov

	bind_dpdk_devices
	;;
stop)
	unbind_dpdk_devices

	remove_sr_iov

	clear_huge_pages
	;;
*)
	echo "usage: $0 [start|stop|bind|unbind|sriov|nosriov]"
;;
esac


