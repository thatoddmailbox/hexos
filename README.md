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

## How do I help?
If you're using a Windows computer, you should get a Linux VM (recommended) or use Cygwin/MSYS.

If you're using a Mac computer, you can [read the guide here](GETTING_STARTED_MAC.md).

If you're using a Linux computer, the instructions should be pretty much the same as for Mac.

Please note that all HexOS code is licensed under the GPLv2 license. For more information, see the [LICENSE](LICENSE) file.

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
* Have a userspace with syscalls
* Run a.out programs
* Have a C library ported to it

## Things I hope it does at some point
* Run `Xorg` and `twm`
* Have a networking stack
* Read ext2 filesystems
* Write to ext2 filesystems
* Be self-hosting (you can make changes to HexOS using HexOS)
