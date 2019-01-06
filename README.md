# libuo

C library for concurrency, asynchronous I/O, networking, etc.

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

``` 
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_INSTALL_PREFIX=/usr/local/libuo #-DCMAKE_BUILD_TYPE=Debug
cmake --build . --target install
```

### Linux

```
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local/libuo #-DCMAKE_BUILD_TYPE=Debug
cmake --build . --target install
```

## Usage example with CMake

#### `CMakeLists.txt` file

```cmake
set(CMAKE_PREFIX_PATH "/usr/local/libuo")

find_package(uo_conf CONFIG REQUIRED)
find_package(uo_http CONFIG REQUIRED)

add_executable(my-web-app
    main.c)

target_link_libraries(my-web-app
    PRIVATE
        uo::uo_conf
        uo::uo_http)
```

#### `main.c` file

```c
#include "uo_conf.h"
#include "uo_http.h"
#include "uo_http_server.h"

#include <stdio.h>

void before_send(
    uo_http_conn *http_conn,
    uo_cb *cb)
{
    // override response header
    uo_http_response_set_header(http_conn->http_response, "server", "libuo http");
    
    // do some other work...
    
    // proceed to the next step
    uo_cb_invoke_async(cb, NULL);
}

int main(
    int argc, 
    char **argv)
{
    // initialize http library
    uo_http_init();

    // read and parse configuration file
    uo_conf *conf = uo_conf_create("my-web-app.conf");

    // get string values from configuration
    const char *port = uo_conf_get(conf, "http_server.port");
    const char *root_dir = uo_conf_get(conf, "http_server.root_dir");

    // create HTTP server
    uo_http_server *http_server = uo_http_server_create(port);

    // set up additional event handler
    http_server->evt.before_send_handler = before_send;
    
    // set root directory
    uo_http_server_set_opt_serve_static_files(http_server, root_dir);

    // start the HTTP server
    uo_http_server_start(http_server);

    printf("Press 'q' to quit...\n");
    while(getchar() != 'q');

    // do cleanup before exit
    uo_http_server_destroy(http_server);
    uo_conf_destroy(conf);

    return 0;
}
```

