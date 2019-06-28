#!/usr/bin/env bash

export source=$1

set +h
umask 022
export MELVIX=/home/melvix/os
cd ${MELVIX}

export LC_ALL=POSIX
export PATH=${MELVIX}/cross-tools/bin:/bin:/usr/bin

mkdir -p ${MELVIX}/sources
cd ${MELVIX}/sources
echo "Downloading resources"
curl -sSL "http://ftp.gnu.org/gnu/binutils/binutils-2.32.tar.xz" | tar xJ
curl -sSL "https://busybox.net/downloads/busybox-1.31.0.tar.bz2" | tar xj
curl -sSL "https://github.com/cross-lfs/bootscripts-embedded/archive/master.tar.gz" | tar xz
curl -sSL "https://gcc.gnu.org/pub/gcc/releases/gcc-9.1.0/gcc-9.1.0.tar.xz" | tar xJ
curl -sSL "http://ftp.gnu.org/gnu/gmp/gmp-6.1.2.tar.xz" | tar xJ
curl -sSL "https://mirrors.edge.kernel.org/pub/linux/kernel/v5.x/linux-5.1.tar.xz" | tar xJ
curl -sSL "http://ftp.gnu.org/gnu/mpc/mpc-1.1.0.tar.gz" | tar xz
curl -sSL "http://ftp.gnu.org/gnu/glibc/glibc-2.29.tar.xz" | tar xJ
curl -sSL "http://ftp.gnu.org/gnu/mpfr/mpfr-4.0.2.tar.xz" | tar xJ
curl -sSL "https://www.zlib.net/zlib-1.2.11.tar.xz" | tar xJ
cd ${MELVIX}

echo "Initializing filesystem structure"
mkdir -p ${MELVIX}/{bin,boot{,grub},dev,{etc/,}opt,home,lib/{firmware,modules},lib64,mnt}
mkdir -p ${MELVIX}/{proc,media/{floppy,cdrom},sbin,srv,sys}
mkdir -p ${MELVIX}/var/{lock,log,mail,run,spool}
mkdir -p ${MELVIX}/var/{opt,cache,lib/{misc,locate},local}
install -d -m 0750 ${MELVIX}/root &> /dev/null
install -d -m 1777 ${MELVIX}{/var,}/tmp &> /dev/null
install -d ${MELVIX}/etc/init.d &> /dev/null
mkdir -p ${MELVIX}/usr/{,local/}{bin,include,lib{,64},sbin,src}
mkdir -p ${MELVIX}/usr/{,local/}share/{doc,info,locale,man}
mkdir -p ${MELVIX}/usr/{,local/}share/{misc,terminfo,zoneinfo}
mkdir -p ${MELVIX}/usr/{,local/}share/man/man{1,2,3,4,5,6,7,8}
for dir in ${MELVIX}/usr{,/local}; do
    ln -s share/{man,doc,info} ${dir} &> /dev/null
    done

install -d ${MELVIX}/cross-tools{,/bin} &> /dev/null
ln -sf /proc/mounts ${MELVIX}/etc/mtab

echo "melvix" > ${MELVIX}/etc/HOSTNAME

touch ${MELVIX}/var/run/utmp ${MELVIX}/var/log/{btmp,lastlog,wtmp}
chmod 664 ${MELVIX}/var/run/utmp ${MELVIX}/var/log/lastlog

unset CFLAGS
unset CXXFLAGS
export MELVIX_HOST=$(echo ${MACHTYPE} | sed "s/-[^-]*/-cross/")
export MELVIX_TARGET=x86_64-unknown-linux-gnu
export MELVIX_CPU=k8
export MELVIX_ARCH=$(echo ${MELVIX_TARGET} | sed -e 's/-.*//' -e 's/i.86/i386/')
export MELVIX_ENDIAN=little

echo "Building kernel headers"
cd ${MELVIX}/sources/linux-5.1/
make mrproper &> /dev/null
make ARCH=${MELVIX_ARCH} headers_check &> /dev/null
make ARCH=${MELVIX_ARCH} INSTALL_HDR_PATH=dest headers_install &> /dev/null
cp -r dest/include/* ${MELVIX}/usr/include

echo "Building binutils"
mkdir ${MELVIX}/sources/binutils-build/
cd ${MELVIX}/sources/binutils-build/
../binutils-2.32/configure --prefix=${MELVIX}/cross-tools --target=${MELVIX_TARGET} --with-sysroot=${MELVIX} --disable-nls --enable-shared --disable-multilib &> /dev/null
make configure-host &> /dev/null
make &> /dev/null
ln -s lib ${MELVIX}/cross-tools/lib64
make install &> /dev/null
cp ../binutils-2.32/include/libiberty.h ${MELVIX}/usr/include

echo "Building static gcc"
cd ${MELVIX}/sources/
mv gmp-6.1.2 gcc-9.1.0/gmp/
mv mpc-1.1.0 gcc-9.1.0/mpc/
mv mpfr-4.0.2 gcc-9.1.0/mpfr/
mkdir ${MELVIX}/sources/gcc-static
cd ${MELVIX}/sources/gcc-static
AR=ar LDFLAGS="-Wl,-rpath,${MELVIX}/cross-tools/lib" \
../gcc-9.1.0/configure --prefix=${MELVIX}/cross-tools \
--build=${MELVIX_HOST} --host=${MELVIX_HOST} \
--target=${MELVIX_TARGET} \
--with-sysroot=${MELVIX}/target --disable-nls \
--disable-shared \
--with-mpfr-include=$(pwd)/../gcc-9.1.0/mpfr/src \
--with-mpfr-lib=$(pwd)/mpfr/src/.libs \
--without-headers --with-newlib --disable-decimal-float \
--disable-libgomp --disable-libmudflap --disable-libssp \
--disable-threads --enable-languages=c,c++ \
--disable-multilib --with-arch=${MELVIX_CPU} &> /dev/null
make all-gcc all-target-libgcc &> /dev/null
make install-gcc install-target-libgcc &> /dev/null
ln -s libgcc.a `${MELVIX_TARGET}-gcc -print-libgcc-file-name | sed 's/libgcc/&_eh/'`

echo "Building glibc"
mkdir ${MELVIX}/sources/glibc-build
cd ${MELVIX}/sources/glibc-build
echo "libc_cv_forced_unwind=yes" > config.cache
echo "libc_cv_c_cleanup=yes" >> config.cache
echo "libc_cv_ssp=no" >> config.cache
echo "libc_cv_ssp_strong=no" >> config.cache
BUILD_CC="gcc" CC="${MELVIX_TARGET}-gcc" \
AR="${MELVIX_TARGET}-ar" \
RANLIB="${MELVIX_TARGET}-ranlib" CFLAGS="-O2" \
../glibc-2.29/configure --prefix=/usr \
--host=${MELVIX_TARGET} --build=${MELVIX_HOST} \
--disable-profile --enable-add-ons --with-tls \
--enable-kernel=2.6.32 --with-__thread \
--with-binutils=${MELVIX}/cross-tools/bin \
--with-headers=${MELVIX}/usr/include \
--cache-file=config.cache &> /dev/null
make &> /dev/null
make install_root=${MELVIX}/ install &> /dev/null

echo "Building gcc"
mkdir ${MELVIX}/sources/gcc-build
cd ${MELVIX}/sources/gcc-build
AR=ar LDFLAGS="-Wl,-rpath,${MELVIX}/cross-tools/lib" \
../gcc-9.1.0/configure --prefix=${MELVIX}/cross-tools \
--build=${MELVIX_HOST} --target=${MELVIX_TARGET} \
--host=${MELVIX_HOST} --with-sysroot=${MELVIX} \
--disable-nls --enable-shared \
--enable-languages=c,c++ --enable-c99 \
--enable-long-long \
--with-mpfr-include=$(pwd)/../gcc-9.1.0/mpfr/src \
--with-mpfr-lib=$(pwd)/mpfr/src/.libs \
--disable-multilib --with-arch=${MELVIX_CPU} &> /dev/null
make &> /dev/null
make install &> /dev/null
cp ${MELVIX}/cross-tools/${MELVIX_TARGET}/lib64/libgcc_s.so.1 ${MELVIX}/lib64
export CC="${MELVIX_TARGET}-gcc"
export CXX="${MELVIX_TARGET}-g++"
export CPP="${MELVIX_TARGET}-gcc -E"
export AR="${MELVIX_TARGET}-ar"
export AS="${MELVIX_TARGET}-as"
export LD="${MELVIX_TARGET}-ld"
export RANLIB="${MELVIX_TARGET}-ranlib"
export READELF="${MELVIX_TARGET}-readelf"
export STRIP="${MELVIX_TARGET}-strip"

echo "Building busybox"
cd ${MELVIX}/sources/busybox-1.31.0
make CROSS_COMPILE="${MELVIX_TARGET}-" defconfig &> /dev/null
# make CROSS_COMPILE="${MELVIX_TARGET}-" menuconfig
make CROSS_COMPILE="${MELVIX_TARGET}-" &> /dev/null
make CROSS_COMPILE="${MELVIX_TARGET}-" CONFIG_PREFIX="${MELVIX}" install &> /dev/null
cp examples/depmod.pl ${MELVIX}/cross-tools/bin
chmod 755 ${MELVIX}/cross-tools/bin/depmod.pl

echo "Building kernel"
cd ${MELVIX}/sources/linux-5.1
make ARCH=${MELVIX_ARCH} CROSS_COMPILE=${MELVIX_TARGET}- x86_64_defconfig &> /dev/null
mv ${MELVIX}/kernel.conf ${MELVIX}/sources/linux-5.1/.config
# make ARCH=${MELVIX_ARCH} CROSS_COMPILE=${MELVIX_TARGET}- menuconfig
make ARCH=${MELVIX_ARCH} CROSS_COMPILE=${MELVIX_TARGET}- &> /dev/null
make ARCH=${MELVIX_ARCH} CROSS_COMPILE=${MELVIX_TARGET}- INSTALL_MOD_PATH=${MELVIX} modules_install &> /dev/null
cp arch/x86/boot/bzImage ${MELVIX}/boot/vmlinuz
cp System.map ${MELVIX}/boot/System.map
cp .config ${MELVIX}/boot/config

${MELVIX}/cross-tools/bin/depmod.pl -F ${MELVIX}/boot/System.map -b ${MELVIX}/lib/modules/5.1.0 &> /dev/null

echo "Installing boot scripts"
cd ${MELVIX}/sources/bootscripts-embedded-master
make DESTDIR=${MELVIX}/ install-bootscripts &> /dev/null
ln -s ../rc.d/startup ${MELVIX}/etc/init.d/rcS

echo "Building zlib"
cd ${MELVIX}/sources/zlib-1.2.11
sed -i 's/-O3/-Os/g' configure
./configure --prefix=/usr --shared &> /dev/null
make &> /dev/null
make DESTDIR=${MELVIX}/ install &> /dev/null
mv ${MELVIX}/usr/lib/libz.so.* ${MELVIX}/lib
ln -sf ../../lib/libz.so.1 ${MELVIX}/usr/lib/libz.so
ln -sf ../../lib/libz.so.1 ${MELVIX}/usr/lib/libz.so.1
ln -sf ../lib/libz.so.1 ${MELVIX}/lib64/libz.so.1

# Cleanup
echo "Cleaning up and backing up current state"
cp -rf ${MELVIX}/ ${MELVIX}-copy
rm -rf ${MELVIX}-copy/cross-tools
rm -rf ${MELVIX}-copy/usr/src/*
rm -rf ${MELVIX}-copy/sources
FILES="$(ls ${MELVIX}-copy/usr/lib64/*.a)"
for file in ${FILES};
do rm -f ${file}
done
