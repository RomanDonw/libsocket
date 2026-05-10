function(setuptarget_socket name source_dir)
    # add target for build

    file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${source_dir}/*.h" "${source_dir}/*.c")
    
    add_library(${name} ${SOURCES})

    target_compile_definitions(${name}
        PRIVATE $<$<BOOL:${BUILD_SHARED_LIBS}>:LIBSOCKET_EXPORT>
        PUBLIC $<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:LIBSOCKET_STATIC>
    )

    # setup includes & required libs

    target_include_directories(${name} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/libsocket>
    )

    if(WIN32)
        target_link_libraries(${name} PUBLIC "ws2_32")
    endif()

    # setup correct installation process

    install(TARGETS ${name}
        EXPORT libsocketTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
    install(DIRECTORY "include" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/libsocket")

    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/configure.cmake")

    install(EXPORT libsocketTargets
        FILE libsocketTargets.cmake
        NAMESPACE libsocket::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libsocket
    )
endfunction()