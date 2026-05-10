function(setuptarget_autoinit name source_dir libsocket_autoinit_target)
    file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${source_dir}/*.h" "${source_dir}/*.c")

    add_library(${name} ${SOURCES})
    target_link_libraries(${name} PUBLIC ${libsocket_autoinit_target})
endfunction()