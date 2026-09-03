add_executable(console_demo console_demo.cpp)
target_link_libraries(console_demo PRIVATE qtcpp)

add_executable(widgets_demo widgets_demo.cpp)
target_link_libraries(widgets_demo PRIVATE qtcpp)

add_executable(client_server_demo client_server_demo.cpp)
target_link_libraries(client_server_demo PRIVATE qtcpp)
