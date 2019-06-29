#!/usr/bin/env bash

export SOURCE=$(pwd)

sudo useradd -m melvix
sudo mkdir -p /home/melvix/os
sudo cp -r boot etc kernel.conf /home/melvix/os
sudo chown -R melvix /home/melvix
sudo cp bootstrap.sh /home/melvix

tput civis
while :;do for s in / - \\ \|; do printf "\r$s";sleep .1;done;done &
trap "kill $!" EXIT
sudo -i -u melvix bash bootstrap.sh ${SOURCE}
kill $! && trap " " EXIT
tput cnorm

export MELVIX=/home/melvix/os
sudo -i bash << EOF
find ${MELVIX}-copy/{,usr/}{bin,lib,sbin} -type f -exec sudo strip --strip-debug '{}' ';' &> /dev/null
find ${MELVIX}-copy/{,usr/}lib64 -type f -exec sudo strip --strip-debug '{}' ';' &> /dev/null
chown -R root:root ${MELVIX}-copy
chgrp 13 ${MELVIX}-copy/var/run/utmp ${MELVIX}-copy/var/log/lastlog &> /dev/null
mknod -m 0666 ${MELVIX}-copy/dev/null c 1 3 &> /dev/null
mknod -m 0600 ${MELVIX}-copy/dev/console c 5 1 &> /dev/null
chmod 4755 ${MELVIX}-copy/bin/busybox
cd ${MELVIX}-copy
tar cfJ ${SOURCE}/melvix-build.tar.xz * &> /dev/null
EOF
