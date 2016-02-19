```
_   _            ___  ____  
| | | | _____  __/ _ \/ ___|
| |_| |/ _ \ \/ / | | \___ \
|  _  |  __/>  <| |_| |___) |
|_| |_|\___/_/\_\\___/|____/
```

HexOS is an operating system kernel that, currently, does not do much.

## System requirements
See [this file](REQUIREMENTS.md).

## Things it does do
* Turn on
* Set up interrupts and GDT
* Memory and paging
* Keyboard input
* Probably can multitask (there's code, but nothing really uses it)
* Browse a virtual filesystem
* Mount its own ISO9660 filesystem into that virtual filesystem
* Read ISO9660 files

## Things it will do soon
* Run a.out programs
* Have a userspace
* Syscalls

## Things I hope it does at some point
* Run `Xorg` and `twm`
* Have a networking stack
* Read ext2 filesystems
* Write to ext2 filesystems
* Be self-hosting (you can make changes to HexOS using HexOS)
