cmake_minimum_required(VERSION 3.23)

# ========== SETTINGS ==========
set(PROJ_VERSION 1.0.0)
set(VERSION_CHECK_METHOD SameMajorVersion)
# ========== ======== ==========

project(socket VERSION ${PROJ_VERSION})

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.h" "src/*.c")

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

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/configure.cmake")

install(EXPORT libsocketTargets
    FILE libsocketTargets.cmake
    NAMESPACE libsocket::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libsocket
)

# ===== TESTS =====

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/tests.cmake")