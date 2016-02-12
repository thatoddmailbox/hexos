#!/bin/sh
set -e
. ./build.sh

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp sysroot/boot/hexos.kernel isodir/boot/hexos.kernel
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "HexOS (serial debug on COM1)" {
	multiboot /boot/hexos.kernel
	play 60 440 1
}
menuentry "HexOS" {
	multiboot /boot/hexos.kernel
}
EOF
grub-mkrescue -o hexos.iso isodir
