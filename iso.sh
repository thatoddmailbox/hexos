#!/bin/sh
set -e
. ./build.sh

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp sysroot/boot/hexos.kernel isodir/boot/hexos.kernel
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "hexos" {
	multiboot /boot/hexos.kernel
}
EOF
grub-mkrescue -o hexos.iso isodir