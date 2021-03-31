# Melvix

<p align="center">
    <i>„A lightweight unix-unlike operating system“</i>
    <br><br>
    <a href="https://github.com/marvinborner/Melvix/actions?query=workflow%3A%22Project+build+and+test%22" target="_blank">
        <img src="https://img.shields.io/github/workflow/status/marvinborner/Melvix/Project%20build?style=for-the-badge" />
    </a>
    <a href="https://app.codacy.com/manual/marvin-borner/Melvix/dashboard" target="_blank">
        <img src="https://img.shields.io/codacy/grade/4ae29e218d7c439eaa549ea828ffcaac?style=for-the-badge" />
    </a>
    <a href="https://www.buymeacoffee.com/marvinborner" target="_blank">
        <img src="https://img.shields.io/static/v1?label=Support&message=buymeacoffee&color=brightgreen&style=for-the-badge" />
    </a>
</p>

## Disclaimer

This project is somewhat of a coding playground for me. It doesn't have any useful functionality (yet?). Be aware that the installation on real hardware is not recommended and may break your computer.

## Features

-   From scratch (no POSIX/UNIX compatibility at all)
-   Multitasking
-   VFS with EXT2, DEVFS and PROCFS
-   Minimal GUI
-   Fast boot time (< 1s)
-   TCP/IP stack and rtl8139 driver
-   Small size (< 100KiB)
-   Sandboxed processes (paging, ring 3, iomap)
-   Full UBSan support
-   Compiles with `-Wall -Wextra -Werror -pedantic-errors -std=c99 -Ofast`

## Screenshot

![Melvix screenshot](screenshot.png?raw=true "Screenshot")

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

## Contributions

If you decide to contribute to this project, these are some ideas that could get you started:

-   Fix some `TODO`s in the code. While these are often not very well described, most of them should be self-explanatory
-   Fix issues posted on GitHubs issues tab
-   Improve the size/speed of the system or the overall performance of libraries and functions
-   Find/fix security issues and potentially write an exploit
-   Add new features (whatever you like, really)
-   Huge parts of the code are kind of a mess to be honest. Feel free to make the code more beautiful :)
-   I'm not very good in design and frontend, so feel free to improve these kind of things too
-   Write more tests for the test suite (which is extremely incomplete at the moment)
-   Write/improve documentation - either in code or in the README, wiki or manuals.

Just remember to pass the test suite and follow the code formatting guidelines (`.clang-format`).

## Contributors :heart:

-   [Marvin Borner](https://github.com/marvinborner/) - Project initiator and main contributor
-   [LarsVomMars](https://github.com/LarsVomMars/) - General help and fixes

## Licenses

Melvix is released under the MIT License and uses parts of the following 3rd party projects:

Inspiration/usage (documented in the respective files):

-   [OSDev wiki](https://wiki.osdev.org) - Very helpful!
-   [James Molloy's tutorials](http://jamesmolloy.co.uk/tutorial_html/)
-   [virtix - tasking inspiration](https://github.com/16Bitt/virtix/) - [MIT License](https://github.com/16Bitt/virtix/blob/85a3c58f3d3b8932354e85a996a79c377139c201/LICENSE)
-   [studix - FS inspiration](https://github.com/orodley/studix) - [MIT License](https://github.com/orodley/studix/blob/d1b1d006010120551df58ff3faaf97484dfa9806/LICENSE)
-   [skiftOS - Memory management inspiration](https://github.com/skiftOS/skift) - [MIT License](https://github.com/skiftOS/skift/blob/ea0e1cf0d7b07302370fc1519be2e072a4cad70c/license.md)
-   [ToAruOS - PCI and network driver inspiration](https://github.com/klange/toaruos) - [NCSA License](https://github.com/klange/toaruos/blob/351d5d38f22b570459931475d36468bf4e37f45a/LICENSE)

Resources:

-   [Spleen font](https://github.com/fcambus/spleen) - [MIT License](https://github.com/fcambus/spleen/blob/5759e9abb130b89ba192edc5324b12ef07b7dad3/LICENSE)

Libraries:

-   [lodepng (modified)](https://github.com/lvandeve/lodepng) - [ZLIB License](https://github.com/lvandeve/lodepng/blob/7fdcc96a5e5864eee72911c3ca79b1d9f0d12292/LICENSE)
-   [sxml (modified)](https://github.com/capmar/sxml) - [Unlicense License](https://github.com/capmar/sxml/blob/91176b4c62ef7c6342804e02fc440b2e82326469/UNLICENSE)
