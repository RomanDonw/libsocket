cmake_minimum_required(VERSION 3.23)
project(socket)

include(GNUInstallDirs)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.h" "src/*.c")
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "include/*.h")

add_library(socket STATIC ${SOURCES})

target_include_directories(socket PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(WIN32)
    target_link_libraries(socket PUBLIC "ws2_32")
endif()

install(TARGETS socket
    EXPORT libsocketTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(DIRECTORY "include/" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT libsocketTargets
    FILE libsocketTargets.cmake
    NAMESPACE libsocket::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libsocket
)

# ===== TESTS =====

add_executable(test0 "tests/0.c")
target_link_libraries(test0 PRIVATE socket)

add_executable(test1 "tests/1.c")
target_link_libraries(test1 PRIVATE socket)