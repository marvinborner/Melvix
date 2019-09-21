# Melvix
<p align="center">
    <i>„A lightweight unix-like operating system“</i>
</p>

## Build
* Use any system running GNU/Linux (Successfully tried building on Debian, Arch Linux and even OpenBSD)
* Install build dependencies (package names may vary depending on your operating system)
    * binutils
    * gcc
    * make
    * bison
    * flex
    * gmp
    * mpc
    * mpfr
    * texinfo
    * curl
    * nasm
    * grub
    * qemu
* Run `make` 
* Test Melvix in QEMU (should open automatically after successful `make`)
* Optional: Flash the built ISO to a USB/Floppy/CD drive using `sudo dd if=./build/melvix.iso of=/dev/sdX bs=4M oflag=sync`
