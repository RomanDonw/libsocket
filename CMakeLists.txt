cmake_minimum_required(VERSION 3.23)

# ========== SETTINGS ==========
project(socket VERSION 1.0)

set(VERSION_CHECK_METHOD SameMajorVersion)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
# ========== ======== ==========

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

# ========== TARGETS ===========
set(TARGETS_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/cmake/targets/")

set(TARGET_LIBSOCKET_NAME socket)
set(TARGET_LIBSOCKET_AUTOINIT_NAME socket_autoinit)

# socket
include("${TARGETS_FOLDER}/socket.cmake")
setuptarget_socket(${TARGET_LIBSOCKET_NAME} "src/socket")

# socket_autoinit
include("${TARGETS_FOLDER}/autoinit.cmake")
setuptarget_autoinit(${TARGET_LIBSOCKET_AUTOINIT_NAME} "src/autoinit/" ${TARGET_LIBSOCKET_NAME})

# (tests)
include("${TARGETS_FOLDER}/tests.cmake")
setuptargets_tests("tests/" ${TARGET_LIBSOCKET_AUTOINIT_NAME})