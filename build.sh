#!/bin/sh

if [ $# -gt 0 ]; then
    echo "PRODUCT_TYPE = '$1'" > build-setup.conf
    cat ./build-setup.conf

    shift
fi

scons -Q $*
