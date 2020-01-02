#!/usr/bin/env sh
# Sets up a cross compiler

if [ ! -d "./cross/" ]; then
  # Create directory
  mkdir -p cross
  cd cross || exit
  DIR=$(pwd)

  # Get sources
  mkdir "${DIR}/src" && cd "${DIR}/src" || exit
  echo "Downloading..."
  curl -sSL "https://ftp.gnu.org/gnu/binutils/binutils-2.32.tar.xz" | tar xJ
  curl -sSL "https://ftp.gnu.org/gnu/gcc/gcc-9.2.0/gcc-9.2.0.tar.xz" | tar xJ

  # Prepare compiling
  mkdir -p "${DIR}/opt/bin"
  export PREFIX="${DIR}/opt"
  export TARGET=i686-elf
  export PATH="$PREFIX/bin:$PATH"

  # Compile binutilsq
  mkdir "${DIR}/src/build-binutils" && cd "${DIR}/src/build-binutils" || exit
  ../binutils-2.32/configure --target="$TARGET" --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
  make
  make install

  # Compile GCC
  mkdir "${DIR}/src/build-gcc" && cd "${DIR}/src/build-gcc" || exit
  ../gcc-9.2.0/configure --target="$TARGET" --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
  make all-gcc
  make all-target-libgcc
  make install-gcc
  make install-target-libgcc

  # Source exported variables
  cd "${DIR}/.." || exit
  . cross.sh
else
  # Should be sourced to take effect
  cd cross || exit
  DIR=$(pwd)
  export PREFIX="${DIR}/opt"
  export TARGET=i686-elf
  export PATH="$PREFIX/bin:$PATH"
  cd ..
fi