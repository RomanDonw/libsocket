macro(test name sources)
    add_executable(${name} ${sources} "tests/base/base.c")
    target_include_directories(${name} PRIVATE "tests/base")
    target_link_libraries(${name} PRIVATE socket)
endmacro()

test(test0 "tests/0.c")
test(test1 "tests/1.c")