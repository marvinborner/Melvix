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

## Disclaimer

This project is somewhat of a coding playground for me. It doesn't have any useful functionality. Be aware that the installation on real hardware is not recommended and may break your computer.

## Features

-   From scratch (no POSIX/UNIX compatibility at all)
-   No external libraries
-   Efficient Multitasking
-   Fast boot time (< 1s)
-   Small size (~10KiB)
-   Compiles with `-Wall -Wextra -pedantic-errors -std=c99 -Os`

## Build

-   Use any system running GNU/Linux

-   Install build dependencies (package names may vary depending on your operating system)

    -   Ubuntu/Debian _"instructions"_ can be found here: [GitHub Workflow](https://raw.githubusercontent.com/marvinborner/Melvix/master/.github/workflows/build.yml)
    -   binutils
    -   ccache
    -   gcc
    -   make
    -   bison
    -   flex
    -   gmp
    -   mpc
    -   mpfr
    -   texinfo
    -   curl
    -   nasm
    -   qemu

-   Build a cross compiler using `./run cross`

-   Run `./run build` or `./run test`

## Licenses

Melvix is released under the MIT License and uses parts of the following 3rd party projects:

Knowledge:

-   [OSDev wiki](https://wiki.osdev.org) - Very helpful!
-   [James Molloy's tutorials](http://jamesmolloy.co.uk/tutorial_html/)
-   [virtix - tasking inspiration](https://github.com/16Bitt/virtix/) - [MIT License](https://github.com/16Bitt/virtix/blob/85a3c58f3d3b8932354e85a996a79c377139c201/LICENSE)
-   [studix - FS inspiration](https://github.com/orodley/studix) - [MIT License](https://github.com/orodley/studix/blob/d1b1d006010120551df58ff3faaf97484dfa9806/LICENSE)

Resources:

-   [Spleen font](https://github.com/fcambus/spleen) - [MIT License](https://github.com/fcambus/spleen/blob/5759e9abb130b89ba192edc5324b12ef07b7dad3/LICENSE)
-   [ext2util](https://github.com/lazear/ext2util) - [MIT License](https://github.com/lazear/ext2util/blob/9bbcc81be46416491f8d46f25f7f775cfb7f2261/LICENSE)
