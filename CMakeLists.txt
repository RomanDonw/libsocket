cmake_minimum_required(VERSION 3.23)
project(socket)

#set(BUILD_SHARED_LIBS OFF)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.h" "src/*.c")

add_library(socket STATIC ${SOURCES})

target_include_directories(socket PUBLIC "include")

# ===== TESTS =====

add_executable(test0 "tests/0.c")
target_link_libraries(test0 PRIVATE socket -lws2_32)