#!/bin/bash
set -e

if [[ $UID != 0  ]]; then
	echo "Please run this script with sudo:"
	echo "sudo $0 $*"
	exit 1
fi

mkdir -p cmake-build-debug
cd cmake-build-debug
cmake ..
sudo make install
cd ../../..
