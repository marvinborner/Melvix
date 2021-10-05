#!/bin/sh
# Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
# SPDX-License-Identifier: MIT

set -e
trap '[ $? -eq 0 ] && exit 0 || { echo "Something went wrong, deleting $DIR"; rm -rf $DIR; exit 1; }' EXIT

# Configuration
MIRROR="https://ftp.gnu.org/gnu/"
FILE_TYPE=".tar.gz"
BINUTILS_VERSION="2.37"
GCC_VERSION="11.2.0"

# Programs
MAKE=make
NPROC=nproc
if [ "$(uname -s)" = "OpenBSD" ]; then
    NPROC="sysctl -n hw.ncpuonline"
    export MAKE=gmake
    export CC="egcc"
    export CXX="eg++"
    export LDFLAGS=-Wl,-z,notext
fi

# Show help
if [ "$1" = "-h" ]; then
    echo "Usage: $0 [arguments]\n\nArguments:"
    echo "\t-d: Delete cross directory"
    echo "\t-y: Accept all prompts"
    echo "\t-h: Show this message"
    exit 0
fi

# Delete cross directory
if [ "$1" = "-d" ]; then
    rm -rf ./cross/
    exit 0
fi

# Exit if exists
if [ -d "./cross/" ]; then
    echo "./cross/ already exists!"
    exit 0
fi

# Prompt to be sure
if [ "$1" != "-y" ]; then
    echo -n "Do you want to compile a cross compiler (this can take up to 20 minutes)? [yn] "
    read -r answer
    if ! [ "$answer" != "${answer#[Yy]}" ]; then
        echo "The compilation of Melvix requires a cross compiler!"
        exit 0
    fi
fi

# Create directory
mkdir -p ./cross/
cd ./cross/
DIR=$(pwd)

# Download compressed sources and signatures
mkdir "$DIR/src" && cd "$DIR/src"
echo "Downloading..."
curl "$MIRROR/binutils/binutils-$BINUTILS_VERSION$FILE_TYPE" >"binutils$FILE_TYPE"
curl "$MIRROR/binutils/binutils-$BINUTILS_VERSION$FILE_TYPE.sig" >"binutils$FILE_TYPE.sig"
curl "$MIRROR/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION$FILE_TYPE" >"gcc$FILE_TYPE"
curl "$MIRROR/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION$FILE_TYPE.sig" >"gcc$FILE_TYPE.sig"

# Verify sources
curl "$MIRROR/gnu-keyring.gpg" >gnu-keyring.gpg
gpg --quiet --verify --keyring ./gnu-keyring.gpg "binutils$FILE_TYPE.sig"
gpg --quiet --verify --keyring ./gnu-keyring.gpg "gcc$FILE_TYPE.sig"

# Extract sources
tar xzf "binutils$FILE_TYPE"
tar xzf "gcc$FILE_TYPE"

# Prepare compiling
mkdir -p "$DIR/bin"
export CFLAGS="-g0 -O2"
export PREFIX="$DIR"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

if [ "$(uname -s)" = "OpenBSD" ]; then
    export with_gmp=/usr/local
    sed -i 's/-no-pie/-nopie/g' "$DIR/src/gcc-$GCC_VERSION/gcc/configure"
fi

# Compile binutils
mkdir "$DIR/src/build-binutils" && cd "$DIR/src/build-binutils"
../binutils-$BINUTILS_VERSION/configure --target="$TARGET" --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
$MAKE -j $($NPROC)
$MAKE install

# Compile GCC
mkdir "$DIR/src/build-gcc" && cd "$DIR/src/build-gcc"
../gcc-$GCC_VERSION/configure --target="$TARGET" --prefix="$PREFIX" --disable-nls --enable-languages=c --without-headers
$MAKE -j $($NPROC) all-gcc all-target-libgcc
$MAKE install-gcc install-target-libgcc

# Fix things
if [ "$(uname -s)" = "OpenBSD" ]; then
    cd "$DIR/libexec/gcc/i686-elf/$GCC_VERSION/" && ln -sf liblto_plugin.so.0.0 liblto_plugin.so
fi

# Remove sources
rm -rf "$DIR/src/"
