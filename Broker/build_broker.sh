#!/bin/bash
mkdir build/
cd build
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
make
mv broker ..
cd ..
