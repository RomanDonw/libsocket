macro(test name sources)
    add_executable(${name} ${sources} "tests/util.c")
    target_link_libraries(${name} PRIVATE socket)
endmacro()

test(test0 "tests/0.c")
test(test1 "tests/1.c")