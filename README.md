# Universal cross-platform C99/C++ sockets library.

<!-- ========================================================================================================================================================== -->

## Description

### Basic
This library provides universal cross-platform network sockets implementation. Supports Windows (recommended MinGW UCRT64 env., MSVC also supported), and any other OS that provides POSIX socket implementation. Also this library supports both static & dynamic linking and building (if set CMake `BUILD_SHARED_LIBS` to `ON`).

### Thread-safe
> [!WARNING]
> This library`s API is mostly thread-safe, instead of initialization & cleanup functions.

### Supported platforms
> [!CAUTION]
> Note, that <ins>**now supported only little-endian CPU arch.-es**</ins>, because host to network byte order convertion macroses hardcoded in header(-s) only for little-endian CPU arch.-es.

- **Windows** (fully x86-64, theoretically x86-32 (IA-32), "ARM64", "ARM32") with WinSock 2.2 support <ins>(but you can change WinSock version through initialization options struct when you initializating the library)</ins>.
- any **POSIX-compatible OS**, such as Linux (inclues Android; library tested in Termux), BSD-s and other **(arch.-es same as for Windows)**.

<!-- ========================================================================================================================================================== -->

## Dependencies
- [libmutex](https://github.com/RomanDonw/libmutex).

## Building the library

### Windows

#### MSVC
Requirements: `Visual Studio`, `CMake`

1. Clone this repo and open it **folder** in Visual Studio.
2. Visual Studio must generate MSBuild script through CMake automaticly. Just wait for it.
3. Build library and tests through Viusal Studio GUI, using hotkey (Ctrl + Shift + B by default) or terminal.

#### MinGW UCRT64
Requirements: `MSYS2 UCRT64`, `MinGW UCRT64`, `make`, `CMake`.

1. Run **MSYS2 UCRT64 environment** (<ins>**recommended**</ins>). Install all requirements if you haven't it.
2. Clone this repo and go to repository root folder.
3. Create folder `build`. Go to this catalog.
4. Run `cmake -S .. -G "MinGW Makefiles" -DCMAKE_INSTALL_PREFIX=$MSYSTEM_PREFIX`. Wait for generating Makefile.
5. Run `mingw32-make` and wait for building library.

### Linux (UNIX)
Requirements: `GCC`, `make`, `CMake`

1. Install all requirements if you haven't it.
2. Clone this repo and go to repository root folder.
3. Create folder `build`. Go to this catalog.
4. Run `cmake -S ..`. Wait for generating Makefile.
5. Run `make` and wait for building library.

<!-- ========================================================================================================================================================== -->

## Installation

### Windows

#### MinGW UCRT64
Just run `mingw32-make install`. Library will be installed to your MSYS2 system path (specified in -DCMAKE_INSTALL_PATH CMake option; `/ucrt64` in this example).

### Linux (UNIX)
Also just run `sudo make install`. Library will be installed to your system.

<!-- ========================================================================================================================================================== -->

## Usage in project

### Manual linking
> [!WARNING]
> Manual linking is not recommended library usage way. Please use CMake for correct linking with library.

Include `<libsocket.h>` header where you need to use this library. That link your executable with library by adding flag to command line `-lsocket`. On Windows you already need to link you executable with WinSock2 library, so just add flag `-lws2_32` command line. If you use static version of this library, please specify definition `LIBSOCKET_STATIC` when you compile file, where included <ins>libsocket</ins> header.

### CMake
If you using CMake in your project, add `libsocket` by same method or with using `find_package`:

```cmake
find_package(libsocket REQUIRED)
target_link_libraries(<target> PRIVATE libsocket::socket)
```

This will fully automatily setup <ins>libsocket</ins> for your project. By default CMake will use static version of <ins>libsocket</ins>, so if you want to use dynamic version of this library, set `BUILD_SHARED_LIBS` flag in your CMake config command prompt to `ON`.

<!-- ========================================================================================================================================================== -->

## Tests
Tests are available in `/tests/` folder in the root of repo. You can also use they as examples. Also in `/tests/` folder available README.md file with (short) description of every available test.

<!-- ========================================================================================================================================================== -->

## Debugging & Troubleshooting

### Got SocketError_InternalUnknownError
Rebuild library with CMake `-DDEBUG=ON` flag, rebuild & rerun you program to again got this error. You will see message in <ins>stderr</ins> like *Got unhandled system error...*. Copy this message & open an issue on this repo with this error message.