# CMake project settings
cmake_minimum_required(VERSION 3.1)
PROJECT(imgui_console_example LANGUAGES C CXX)

# Example executable
add_executable(example "./src/example_main.cpp"
                       "./thirdparty/imgui/imgui_impl_glfw.cpp"
                       "./thirdparty/imgui/imgui_impl_opengl3.cpp")

target_compile_definitions(example PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD)   # Using GLAD
target_compile_features(example PUBLIC cxx_std_17)                          # Use C++17

# IMGUI Library
add_library(imgui STATIC "./thirdparty/imgui/imgui.cpp"
                         "./thirdparty/imgui/imgui_widgets.cpp"
                         "./thirdparty/imgui/imgui_draw.cpp"
                         "./thirdparty/imgui/imgui_demo.cpp")

target_include_directories(imgui PUBLIC "./thirdparty/imgui")
target_link_libraries(example PRIVATE imgui)

# Glad
if (WIN32)
    set(GLAD_ACCESS STATIC)
else()
    set(GLAD_ACCESS SHARED)
endif()
add_library(glad ${GLAD_ACCESS} "./thirdparty/glad/src/glad.c")
target_include_directories(glad PUBLIC "./thirdparty/glad/include")
target_link_libraries(example PRIVATE glad)

# Install glfw. (For windows the example includes a bundled glfw)
# Linux: apt-get install libglfw-dev
#GLFW
if (WIN32)
    target_include_directories(example PRIVATE "${CMAKE_SOURCE_DIR}/example/thirdparty/glfw/include")
    target_link_libraries(example PRIVATE "${CMAKE_SOURCE_DIR}/example/thirdparty/glfw/lib-vc2015/glfw3.lib")
else()
    find_package(PkgConfig REQUIRED)
    pkg_search_module(GLFW REQUIRED glfw3)
    target_link_libraries(example PRIVATE ${GLFW_LIBRARIES})
    target_include_directories(example PRIVATE "${GLFW_INCLUDE_DIRS}")
endif()

#CSYS
add_library(csys INTERFACE)
target_include_directories(csys INTERFACE "${CMAKE_SOURCE_DIR}/include")
target_compile_features(csys INTERFACE cxx_std_17)

# IMGUI Console
add_library(imgui_console STATIC "../src/imgui_console.cpp" "../include/imgui_console/imgui_console.h")
target_include_directories(imgui_console PUBLIC "./thirdparty/imgui" "../include/imgui_console")
target_compile_features(imgui_console PUBLIC cxx_std_17)
target_link_libraries(imgui_console PUBLIC csys)
target_link_libraries(example PUBLIC imgui_console)

# Misc
# Copy script to binary directory for testing.
configure_file("scripts/console.script" "${CMAKE_CURRENT_BINARY_DIR}/console.script" COPYONLY)



