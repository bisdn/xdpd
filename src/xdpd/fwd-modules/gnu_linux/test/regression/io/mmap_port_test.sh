#!/bin/bash

# check for veth0
ip link show veth0 > /dev/null 2>&1
if [ 0 -ne $? ]; then
	# no veth0 and veth1
	sudo ip link add type veth
fi

# enable devices 
sudo ip link set dev veth0 up
sudo ip link set dev veth1 up

# run test
sudo ./mmap_port_test
