cmake_minimum_required(VERSION 3.23)

# ========== SETTINGS ==========
set(PROJ_VERSION 1.0)
set(VERSION_CHECK_METHOD SameMajorVersion)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
# ========== ======== ==========

project(socket VERSION ${PROJ_VERSION})

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# add target for build

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.h" "src/*.c")

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

add_library(socket ${SOURCES})

target_compile_definitions(socket 
    PRIVATE $<$<BOOL:${BUILD_SHARED_LIBS}>:LIBSOCKET_EXPORTS>
    INTERFACE $<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:LIBSOCKET_STATIC>
)

message("============================================")
if(BUILD_SHARED_LIBS)
    message("Building dynamic version of libsocket.")
else()
    target_compile_definitions(socket PUBLIC LIBSOCKET_STATIC)
    message("Building static version of libsocket.")
endif()
message("============================================")

# setup includes & required libs

target_include_directories(socket PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(WIN32)
    target_link_libraries(socket PUBLIC "ws2_32")
endif()

# setup correct installation process

install(TARGETS socket
    EXPORT libsocketTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(DIRECTORY "include/" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/configure.cmake")

install(EXPORT libsocketTargets
    FILE libsocketTargets.cmake
    NAMESPACE libsocket::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libsocket
)

# ===== TESTS =====

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/tests.cmake")