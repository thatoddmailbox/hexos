# Getting Started (Mac OS X)
This guide will walk you through the steps to set up a HexOS development environment under Mac OS X. If you get stuck, feel free to make an issue!

## Cross-compile toolchain
The first step is to build a *cross-compile toolchain*. This builds a copy of `gcc`, `ld`, `as`, and other binutils programs that can be used to compile operating systems for x86 processors (basically all computers).

### Before you begin
This guide assumes you have XCode, Homebrew and QEMU installed. If you don't have Homebrew, you can [download it here](http://brew.sh). QEMU is the VM software used for HexOS. While you could technically use VirtualBox or something else, QEMU is the easiest to do development with. You should be able to install QEMU by running `brew install qemu`. (you can also compile QEMU from source if you feel like it, but Homebrew is easier)

This guide also assumes you have access to an Administrator account. Your account does not have to be an administrator, but you must have access to one so you can run Homebrew command and to install some libraries.

You also have to create a folder to develop HexOS and put your toolchain into. **The path to this folder may not contain spaces.** (well, it could, but stuff will probably break). A common name for this directory is `src`, and a good place to put it is in your home folder.

Once you've decided where to make this folder, clone the code of HexOS into it. **It is highly recommended to fork HexOS and clone your fork into the folder you just made.** When you want to make a change, you commit it to your fork and submit a pull request to the main HexOS repository.

### Programs to install
These can be all installed with a `brew install <program name>`. You must have them all installed, or you'll get weird errors.

* autoconf
* automake
* binutils
* bison
* flex
* gettext
* glib
* libffi
* xorriso
* wget

### Compiling
There are two programs you will compile to create your toolchain. They are `binutils` and `gcc`. Binutils is a collection of small, useful programs used to assemble and modify executable files, and gcc compiles C code.

#### Downloading the programs
First, we'll download `binutils`. Go to [the binutils website](https://gnu.org/software/binutils/) and look what the latest version is (as of the time of this writing, it's 2.26). Once you've determined the latest version, go to the [download mirror](http://ftp.gnu.org/gnu/binutils/) and download the file for the latest version. (for example, if the latest version was 2.26, you would download `binutils-2.26.tar.gz`)

Once it's finished downloading, extract the binutils .tar.gz archive and **move the `binutils-x.xx` (the x's will be the version you downloaded) folder into the folder you created in the "Before you begin" step.**

`binutils` requires a library called `isl`, which can be downloaded [from this site](http://isl.gforge.inria.fr). Download **version 0.14** of isl and extract it. Then, place the `isl-0.14` *inside* the `binutils-x.xx` folder. `binutils` will automagically detect `isl` and include it.

Next, it's time to download `gcc`. You can find it at [this website](http://ftp.gnu.org/gnu/gcc/). As of the time of this writing, the latest version was 5.3.0. Select the folder with the latest version (it might be at the bottom of the page) and download the .tar.gz file (**not the .diff.gz!**). Extract it, and **move the `gcc-x.x.x` (x's will be the version you downloaded) folder into the folder you created in the "Before you begin step".**

Finally, there's one more library that must be installed. It's called `libiconv`. You can download it [at this site](http://ftp.gnu.org/gnu/libiconv/). The latest version at the time of this writing is 1.14. Download the .tar.gz and extract it. Rename the `libiconv-x.xx` folder to just `libiconv` and copy it *inside* the `gcc-x.x.x` folder.

At this point, you should have one folder, and inside of that folder: the HexOS source code, a `binutils-x.xx` folder, and a `gcc-x.x.x` folder. If so, continue to the next step.

#### Compiling
First, we'll set some parameters for the compilation. These parameters tell `binutils` and `gcc` that they should not be outputting Mac OS X programs, but instead be outputting a generic x86 program.

Open a Terminal window and run these commands:
```
export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

mkdir $HOME/opt/cross
```

GCC requires binutils, so we will compile binutils first. `cd` to the folder with the `binutils-x.xx` folder in it (*don't go into the binutils folder!*). Then, run the following (replace the x.xx with your version of binutils):

```
mkdir build-binutils
cd build-binutils
../binutils-x.xx/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make
make install
```

The compilation might take a couple of minutes&mdash;that's OK. Once it's done, try running `i686-elf-as --version`. The output should look like:

```
GNU assembler (GNU Binutils) 2.26.20160125
Copyright (C) 2015 Free Software Foundation, Inc.
This program is free software; you may redistribute it under the terms of
the GNU General Public License version 3 or later.
This program has absolutely no warranty.
This assembler was configured for a target of `i686-elf'.
```

(it's fine if the version is different)

If so, congratulations! You've compiled and installed a custom version of `binutils`. Now, you have to compile `gcc`!

First, `gcc` needs to download some files. To do so, run the following commands from the same Terminal window: (remember, replace `x.x.x` with your `gcc` version)
```
cd ..
cd gcc-x.x.x
contrib/download_prerequisites
```

To compile `gcc`, run the following commands from the same Terminal window:

**note: the compilation may take 15 to 25 minutes, and is battery-intensive. It is recommended to connect your computer to power before starting.**
```
cd ..
mkdir build-gcc
cd build-gcc
../gcc-x.x.x/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc
make all-target-libgcc
make install-gcc
make install-target-libgcc
```

Once all of those commands have completed, try running `i686-elf-gcc --version`. You should see something like this:
```
i686-elf-gcc (GCC) 5.3.0
Copyright (C) 2015 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```

If so, great! You've successfully set up a custom version of `binutils` and `gcc` that can be used to compile operating systems! But you're not done yet...

#### Adding it to your PATH
You might have noticed something strange. The `i686-elf-whatever` commands only work in the Terminal window you compiled everything from. Open a new Terminal window, and it won't work! To fix this, run the following command from any Terminal window:

```
nano ~/.bash_profile
```

This will open the `nano` text editor. The file that opens might already have some text, or it could be blank. Type in the line `export PATH=$HOME/opt/cross/bin:$PATH` at the bottom of the file. Then, press control-O (not command) and then enter. Finally, press control-X to exit `nano`.

## Bootloader
TODO: this section
