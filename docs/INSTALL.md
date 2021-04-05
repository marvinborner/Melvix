# Installation guide

This document explains the testing/building/booting process and requirements. Everything is done using the virtual machine QEMU. I will not explain how you can install Melvix on real hardware as this is not recommended right now.

## Test

-   Install the qemu i386 emulator
-   Download the `disk-img` artifact from the newest stable [GitHub Workflow build](https://github.com/marvinborner/Melvix/actions?query=branch%3Amain)
-   Unzip `disk-img.zip`
-   Run `qemu-system-i386 -m 256M -vga std -drive file=path/to/disk.img,format=raw,index=1,media=disk -netdev user,id=net0 -device rtl8139,netdev=net0`
-   Try entering `browser`, `files`, `mandelbrot` (or any other program in `apps/`) into the input field and press enter
-   Move windows using `ALT`+`Left click`
-   Enjoy, or try building it yourself!

## Build it yourself

-   Use any system running GNU/Linux or OpenBSD

-   Install build dependencies

    -   General template: `[pkg_manager] [install] git binutils ccache gcc make bison flex gmp mpc mpfr texinfo curl nasm qemu inkscape` (package names may vary depending on your operating system)
    -   Ubuntu/Debian: `apt-get install -y build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo ccache curl nasm grub-common qemu qemu-kvm mtools ctags inkscape`
    -   OpenBSD: `pkg_add git ccache gcc g++ gmake bison gmp libmpc mpfr texinfo curl nasm qemu e2fsprogs inkscape`

-   Clone this repository using `git clone https://github.com/marvinborner/Melvix.git`

-   Switch to the stable `main` branch using `git checkout main`

-   Run `./run` (pure magic!)

-   If you need help: `./run to help`
