#!/bin/sh

TOOL_PACKAGES="build-essential scons pandoc pkg-config python-requests"
LIB_PACKAGES="libmariadb-client-lgpl-dev libpcap-dev libboost-all-dev libcppnetlib-dev libyaml-dev libjsoncpp-dev"
TILERA_PACKAGES="libc6-i386"

apt-get install $TOOL_PACKAGES $LIB_PACKAGES $TILERA_PACKAGES


