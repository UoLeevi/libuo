#!/bin/bash

rm -r build
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local/libuo -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS="-O2 -flto"
cmake --build . --target install
