# Melvix
<p align="center">
    <i>„A lightweight unix-unlike operating system“</i>
    <br><br>
    <a href="https://github.com/marvinborner/Melvix/actions?query=workflow%3A%22Project+build%22" target="_blank">
        <img src="https://img.shields.io/github/workflow/status/marvinborner/Melvix/Project%20build?style=for-the-badge" />
    </a>
    <a href="https://app.codacy.com/manual/marvin-borner/Melvix/dashboard" target="_blank">
        <img src="https://img.shields.io/codacy/grade/4ae29e218d7c439eaa549ea828ffcaac?style=for-the-badge" />
    </a>
</p>

## Build
* Use any system running GNU/Linux

* Install build dependencies (package names may vary depending on your operating system)
  * Ubuntu/Debian _"instructions"_ can be found here: [GitHub Workflow](https://raw.githubusercontent.com/marvinborner/Melvix/master/.github/workflows/build.yml)
  * binutils
  * ccache
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
  * qemu
  * grub

* Build a cross compiler using `./run cross`

* Run `./run build` or `./run test`

* Optional: Flash the built ISO to a CD drive using `sudo dd if=./iso/melvix.iso of=/dev/sdX bs=4M oflag=sync`

## Licenses
Melvix is released under the MIT License and uses the following 3rd party applications (as stated in the regarding files): 
* [Spleen font](https://github.com/fcambus/spleen) - [MIT License](https://github.com/fcambus/spleen/blob/5759e9abb130b89ba192edc5324b12ef07b7dad3/LICENSE)
* [bdf2c converter](https://github.com/pixelmatix/bdf2c) - [AGPL 3.0 License](https://github.com/pixelmatix/bdf2c/blob/b07deb7a484751b3e3fb6c952f6bc54b1b2950fd/AGPL-3.0.txt)
* [onedark.vim colorscheme](https://github.com/joshdick/onedark.vim/) - [MIT License](https://github.com/joshdick/onedark.vim/blob/fe035976117ba5c2481df3b2cad3bb0a8b045b9f/LICENSE)
