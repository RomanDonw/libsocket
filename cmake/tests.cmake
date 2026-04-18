add_executable(test0 "tests/0.c")
target_link_libraries(test0 PRIVATE socket)

add_executable(test1 "tests/1.c")
target_link_libraries(test1 PRIVATE socket)