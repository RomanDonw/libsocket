cmake_minimum_required(VERSION 3.23)
project(socket)

#set(BUILD_SHARED_LIBS OFF)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.h" "src/*.c")

add_library(socket STATIC ${SOURCES})

target_include_directories(socket PUBLIC "include")