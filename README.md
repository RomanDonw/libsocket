# Universal cross-platform C99/C++ sockets library.

This library provides universal cross-platform network sockets implementation. Supports Windows (recommended MinGW UCRT64 env., MSVC also supported), and any other OS that provides POSIX socket implementation. Also this library supports both static & dynamic linking and building (if set CMake `BUILD_SHARED_LIBS` to `ON`).

## Building the library

### Windows

Requirements: `MSYS2 UCRT64`, `MinGW UCRT64`, `make`, `CMake`.

1. Run **MSYS2 UCRT64 environment** (__**recommended**__). Install all requirements if you haven't it.
2. Clone this repo and go to repository root folder.
3. Create folder `build`. Go to this catalog.
4. Run `cmake -S .. -G "MinGW Makefiles" -DCMAKE_INSTALL_PATH=$MSYSTEM_PREFIX`. Wait for generating Makefile.
5. Run `mingw32-make` and wait for building library.

### Linux (UNIX)

Requirements: `GCC`, `make`, `CMake`

1. Install all requirements if you haven't it.
2. Clone this repo and go to repository root folder.
3. Create folder `build`. Go to this catalog.
4. Run `cmake -S ..`. Wait for generating Makefile.
5. Run `make` and wait for building library.

## Installation

### Windows

Just run `mingw32-make install`. Library will be installed to your MSYS2 system path (specified in -DCMAKE_INSTALL_PATH CMake option; `/ucrt64` in this example).

### Linux (UNIX)

Also just run `sudo make install`. Library will be installed to your system.

## Using in project

Include `<libsocket.h>` header where you need to use this library. That link your executable with library by adding flag to command line `-lsocket`. On Windows you already need to link you executable with WinSock2 library, so just add flag `-lws2_32` command line. If you use static version of this library, please specify definition `LIBSOCKET_STATIC` when you compile file, where included __libsocket__ header.

If you using CMake in your project, add `libsocket` by same method or with using `find_package`:

```cmake
find_package(libsocket REQUIRED)
target_link_libraries(<target> PRIVATE libsocket::socket)
```

This will fully automatily setup __libsocket__ for your project. By default CMake will use static version of __libsocket__, so if you want to use dynamic version of this library, set `BUILD_SHARED_LIBS` flag in your CMake config command prompt to `ON`.

## Tests

Tests are available in `/tests/` folder in the root of repo. You can also use they as examples.

### All tests short description:

- Test 0 (0.c): HTTP 1.0 GET request test.
- Test 1 (1.c): Unsafe API functions (for ex. `socket_gethandle`) test.
