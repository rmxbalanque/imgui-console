# CMake project settings
cmake_minimum_required(VERSION 3.1)
PROJECT(imgui_console_example LANGUAGES C CXX)

# Example executable
add_executable(example "./src/example_main.cpp"
                       "./thirdparty/imgui/imgui_impl_glfw.cpp"
                       "./thirdparty/imgui/imgui_impl_opengl3.cpp")

target_compile_definitions(example PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD)

# IMGUI Library
add_library(imgui STATIC "./thirdparty/imgui/imgui.cpp"
                         "./thirdparty/imgui/imgui_widgets.cpp"
                         "./thirdparty/imgui/imgui_draw.cpp"
                         "./thirdparty/imgui/imgui_demo.cpp")
target_include_directories(imgui PUBLIC "./thirdparty/imgui")
target_link_libraries(example PRIVATE imgui)

# Glad
add_library(glad SHARED "./thirdparty/glad/src/glad.c")
target_include_directories(glad PUBLIC "./thirdparty/glad/include")
target_link_libraries(example PRIVATE glad)

# NOTE: You will need to install glfw for the example to work. (http://www.glfw.org)
# Linux: apt-get install libglfw-dev
#GLFW
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
target_link_libraries(example PRIVATE ${GLFW_LIBRARIES})
target_include_directories(example PRIVATE "${GLFW_INCLUDE_DIRS}")




