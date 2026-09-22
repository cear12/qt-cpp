cmake_minimum_required(VERSION 3.16)
project(qtcpp CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# NOTE: this repo's CMake setup has not been run locally -- the sandbox
# this was cleaned up in has no Qt installation and no root access to
# install one (see the repository README). CI (.github/workflows/ci.yml)
# is the first place this actually gets compiled; please check its result
# before relying on any of this.
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    Gui
    Network
    Concurrent
    OpenGL
    OpenGLWidgets
    StateMachine
    Qml
    UiPlugin
)

option(QTCPP_BUILD_EXAMPLES "Build examples/" ON)

add_library(qtcpp STATIC
    src/client_server.cpp
    src/custom-delegate.cpp
    src/custom-opengl-widget.cpp
    src/custom-plugin-widget.cpp
    src/qml-cpp-custom-type-integration.cpp
)
target_include_directories(qtcpp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(qtcpp PUBLIC
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Network
    Qt6::Concurrent
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    Qt6::StateMachine
    Qt6::Qml
    Qt6::UiPlugin
)
target_compile_options(qtcpp PRIVATE $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra>)

enable_testing()

if(QTCPP_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
