# libuo

**libuo** is the C library that provides the building blocks for many of my software projects. **libuo** aims to be a modular, cross-platform, high-performance C library that raises the level of abstraction of C programming just enough to enable the programmer to be productive and able to write code that is also readable and extensible.

**libuo** includes multiple libraries that can be linked with your C programs based on what you need. Here are highlighted some of the libraries that I use the most often.

### Networking
**`uo_http`** - for event-driven HTTP servers and clients  
**`uo_tcp`** - for event-driven TCP servers and clients

### Configuration
**`uo_conf`** - inteface for simple text file configuration

### Data encoding
**`uo_jwt`** - JWT encoding, decoding, signing and verification  
**`uo_json`** - JSON encoding and parsing  
**`uo_base64`** - base64 and base64url encoding and decoding

### Asynchronous I/O
**`uo_io`** - Asychronous socket I/O for Windows and Linux  

### Concurrency
**`uo_cb`** - Callbacks for asynchronous workloads  

### Data structures
**`uo_linklist`** - linked list without dynamic memory allocation 
**`uo_strhashtbl`** - hash table for storing values with string keys  
**`uo_queue`** - "blocking" queue for multithreaded synchronized data access 

### Utilities
**`uo_buf`** - buffer that can handle memory allocation automatically  
**`uo_prog`** - helpers for signal handling and program control  
**`uo_err`** - helper functions for error handling

## Supported architectures and compilers

### Linux
 - x86-64 gcc 7.3.0 (Ubuntu 18.04)
 - ARM64 gcc 6.3.0 (Raspbian GNU/Linux 9)

### Windows 10
 - x86-64 gcc 8.1.0 (x86_64-posix-seh-rev0, MinGW-W64)

## Installation

### Prerequisites
 - CMake 3.12

### Linux

```bash
scripts/rebuild.sh # for debug builds use: scripts/rebuild-debug.sh
scripts/test.sh
```

### Windows 10

```powershell
scripts/rebuild.ps1 # for debug builds use: scripts/rebuild-debug.ps1
scripts/test.ps1
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

## TO DO

- [ ] make separate `README.md` files for each sub-library
