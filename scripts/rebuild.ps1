rm -r build
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local/libuo -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS="-O2" -G "MinGW Makefiles"
cmake --build . --target install
