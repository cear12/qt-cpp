add_executable(console_demo console_demo.cpp)
target_link_libraries(console_demo PRIVATE qtcpp)

add_executable(widgets_demo widgets_demo.cpp)
target_link_libraries(widgets_demo PRIVATE qtcpp)

add_executable(client_server_demo client_server_demo.cpp)
target_link_libraries(client_server_demo PRIVATE qtcpp)

# The demos double as smoke tests, so plain `ctest` exercises them. None of
# them needs a real display: Qt's offscreen platform plugin stands in for one.
add_test(NAME console_demo COMMAND console_demo)
add_test(NAME widgets_demo COMMAND widgets_demo)
add_test(NAME client_server_demo COMMAND client_server_demo)
set_tests_properties(console_demo widgets_demo client_server_demo PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    TIMEOUT 120)
