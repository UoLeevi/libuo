# libuo

Simple C library providing functionally for asynchronous I/O and networking.

## Supported architectures and compilers

### Linux
 - x86-64 gcc 7.3.0 (Ubuntu 18.04)
 - ARM64 gcc 6.3.0 (Raspbian GNU/Linux 9)

### Windows 10
 - x86-64 gcc 8.1.0 (x86_64-posix-seh-rev0, MinGW-W64)

## Installation

### Prerequisites
 - CMake 3.12

### Windows 10

mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -G "MinGW Makefiles" -DCMAKE_INSTALL_PREFIX=/usr/local/libuo
cmake --build . --target install

### Linux

mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr/local/libuo
cmake --build . --target install
