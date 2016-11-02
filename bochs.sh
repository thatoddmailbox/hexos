#!/bin/sh
set -e
. ./iso.sh

#qemu-system-$(./target-triplet-to-arch.sh $HOST) -cdrom hexos.iso -soundhw pcspk -serial stdio -net nic,model=rtl8139,netdev=mynet0 -m 128M -drive file=hdd.img,format=raw -netdev user,id=mynet0,net=192.168.76.0/24,dhcpstart=192.168.76.9
bochs
