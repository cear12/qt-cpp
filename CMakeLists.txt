cmake_minimum_required(VERSION 3.16)
project(qtcpp CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

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
    # Keep Qt headers in the target source list so AUTOMOC discovers classes
    # that contain Q_OBJECT but are included by the examples only.
    include/qtcpp/advanced-network-manager.h
    include/qtcpp/algorithm-strategy.h
    include/qtcpp/animated-graphic-scene.h
    include/qtcpp/async-processor.h
    include/qtcpp/client_server.h
    include/qtcpp/composite-for-ui-component.h
    include/qtcpp/custom-delegate.h
    include/qtcpp/custom-opengl-widget.h
    include/qtcpp/custom-plugin-widget.h
    include/qtcpp/model-view-delegate.h
    include/qtcpp/multi-threading-widget-creation.h
    include/qtcpp/proxy-for-service-layer.h
    include/qtcpp/qml-cpp-custom-type-integration.h
    include/qtcpp/signals-slots-observer.h
    include/qtcpp/state-machine-controller.h
    include/qtcpp/template-magic-factory.h
    include/qtcpp/undo-redo-command.h
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
