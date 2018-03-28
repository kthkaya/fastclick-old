#!/bin/bash

if [ "$(id -u)" != "0" ]; then
        echo "You are not root."
        exit 1
fi

if [ "$CLICK_DIR" == "" ]; then
	echo "CLICK_DIR has not been defined."
	exit 1
fi

cd "$CLICK_DIR"

./configure \
--enable-multithread --disable-linuxmodule --enable-intel-cpu --enable-user-multithread \
--verbose CFLAGS="-g -O3" CXXFLAGS="-g -std=gnu++11 -O3" --disable-dynamic-linking --enable-poll \
--enable-ip6 --enable-local --enable-bound-port-transfer --enable-dpdk --enable-batch --with-netmap=no \
--enable-zerocopy --enable-dpdk-pool --disable-dpdk-packet

echo "Making fastclick in $CLICK_DIR"
make -j4 && make install

