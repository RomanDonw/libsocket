cmake_minimum_required(VERSION 3.23)
project(socket)

#set(BUILD_SHARED_LIBS OFF)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.h" "src/*.c")
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "include/*.h")

add_library(socket STATIC ${SOURCES})

set_target_properties(socket PROPERTIES PUBLIC_HEADER ${HEADERS})
target_include_directories(socket PUBLIC "include")

install(TARGETS socket
    EXPORT libsocketTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT libsocketTargets
    FILE libsocketTargets.cmake
    NAMESPACE libsocket::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libsocket
)

# ===== TESTS =====

add_executable(test0 "tests/0.c")
target_link_libraries(test0 PRIVATE socket -lws2_32)