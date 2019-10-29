# Melvix
<p align="center">
    <i>„A lightweight unix-like operating system“</i>
    <br><br>
    <a href="https://travis-ci.com/marvinborner/Melvix" target="_blank">
        <img src="https://img.shields.io/travis/marvinborner/Melvix?style=for-the-badge" />
    </a>
    <a href="https://app.codacy.com/manual/marvin-borner/Melvix/dashboard" target="_blank">
        <img src="https://img.shields.io/codacy/grade/4ae29e218d7c439eaa549ea828ffcaac?style=for-the-badge" />
    </a>
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

* Build a cross compiler using `make cross`

* Run `make build`

* Test Melvix in QEMU (opens after `make test`)

* Optional: Flash the built ISO to a USB/Floppy/CD drive using `sudo dd if=./build/melvix.iso of=/dev/sdX bs=4M oflag=sync`

## Licenses
Melvix is released under the MIT License and uses the following 3rd party applications (as stated in the regarding files): 
* [Spleen font](https://github.com/fcambus/spleen) - [MIT License](https://github.com/fcambus/spleen/blob/5759e9abb130b89ba192edc5324b12ef07b7dad3/LICENSE)
* [bdf2c converter](https://github.com/pixelmatix/bdf2c) - [AGPL 3.0 License](https://github.com/pixelmatix/bdf2c/blob/b07deb7a484751b3e3fb6c952f6bc54b1b2950fd/AGPL-3.0.txt)
