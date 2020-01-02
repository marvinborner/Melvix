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
    <a href="https://marvinborner.de/melvix/" target="_blank">
        <img src="https://img.shields.io/badge/download-latest-brightgreen?style=for-the-badge" />
    </a>
</p>

## Build
* Use any system running GNU/Linux (Successfully tried building on Debian, Arch Linux and even OpenBSD)

* Install build dependencies (package names may vary depending on your operating system)
  * Ubuntu/Debian _"instructions"_ can be found here: [GitHub Workflow](https://raw.githubusercontent.com/marvinborner/Melvix/master/.github/workflows/build.yml)
  * binutils
  * gcc
  * make
  * cmake
  * bison
  * flex
  * gmp
  * mpc
  * mpfr
  * texinfo
  * curl
  * nasm
  * genisoimage
  * qemu

* Build a cross compiler using `sh cross.sh`

* Run
  * `mkdir build && cd build`
  * `cmake .. && make`
  * `cd .. && rm -rf build`
  * The relevant files are in iso/

* Optional: Flash the built ISO to a CD drive using `sudo dd if=./iso/melvix.iso of=/dev/sdX bs=4M oflag=sync`

## Licenses
Melvix is released under the MIT License and uses the following 3rd party applications (as stated in the regarding files): 
* [Spleen font](https://github.com/fcambus/spleen) - [MIT License](https://github.com/fcambus/spleen/blob/5759e9abb130b89ba192edc5324b12ef07b7dad3/LICENSE)
* [bdf2c converter](https://github.com/pixelmatix/bdf2c) - [AGPL 3.0 License](https://github.com/pixelmatix/bdf2c/blob/b07deb7a484751b3e3fb6c952f6bc54b1b2950fd/AGPL-3.0.txt)
* [onedark.vim colorscheme](https://github.com/joshdick/onedark.vim/) - [MIT License](https://github.com/joshdick/onedark.vim/blob/fe035976117ba5c2481df3b2cad3bb0a8b045b9f/LICENSE)
