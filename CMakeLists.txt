
cmake_minimum_required(VERSION 3.1...3.16)
project(imgui_console VERSION 1.0 LANGUAGES CXX)

#set(IMGUI_CONSOLE_SRC "./src/imgui_console.cpp" "./include/imgui_console.h")
#add_library(imgui_console STATIC ${IMGUI_CONSOLE_SRC})
#target_include_directories(imgui_console PRIVATE "./src/")
#target_compile_features(imgui_console PRIVATE cxx_std_17)

# Build example project
add_subdirectory(example)