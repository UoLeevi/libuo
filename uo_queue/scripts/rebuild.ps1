rm -r build
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -G "MinGW Makefiles"
cd ..
cmake --build "./build"