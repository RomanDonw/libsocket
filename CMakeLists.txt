cmake_minimum_required(VERSION 3.23)

# ========== SETTINGS ==========
project(socket VERSION 0.0 LANGUAGES C)

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

# add target for build

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.h" "src/*.c")
add_library(${PROJECT_NAME} ${SOURCES})
unset(SOURCES)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE $<$<BOOL:${DEBUG}>:LIBSOCKET_DEBUG>
    PRIVATE $<$<BOOL:${BUILD_SHARED_LIBS}>:LIBSOCKET_EXPORT>
    PUBLIC $<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:LIBSOCKET_STATIC>
)

# setup includes & required libs

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

find_package(libmutex 1.0 REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE libmutex::mutex)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PUBLIC "ws2_32")
endif()

# setup correct installation process

install(TARGETS ${PROJECT_NAME}
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