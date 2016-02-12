#!/bin/sh
set -e
. ./build.sh

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp sysroot/boot/hexos.kernel isodir/boot/hexos.kernel
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "HexOS (serial debug on COM1)" {
	multiboot /boot/hexos.kernel -serdbg
}
menuentry "HexOS" {
	multiboot /boot/hexos.kernel
}
insmod gfxmenu
set theme=\$prefix/themes/hexos/theme.txt
set menu_color_normal=dark-gray/blue
set menu_color_highlight=white/blue
export theme
EOF

mkdir -p isodir/boot/grub/themes/hexos/

cat > isodir/boot/grub/themes/hexos/theme.txt << EOF
title-text: "HexOS"
+ label { text="hex" font="aqui 11" color="#8FF" }
EOF

grub-mkrescue -o hexos.iso isodir
