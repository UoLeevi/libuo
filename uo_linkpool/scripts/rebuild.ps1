rm -r build
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local/libuo -DCMAKE_BUILD_TYPE=Debug -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
cmake --build . --target install
