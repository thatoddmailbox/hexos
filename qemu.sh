#!/bin/sh
set -e
. ./iso.sh

qemu-system-$(./target-triplet-to-arch.sh $HOST) -cdrom hexos.iso -soundhw pcspk -serial stdio -net nic,model=rtl8139 -m 128M
