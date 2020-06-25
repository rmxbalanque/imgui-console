
cmake_minimum_required(VERSION 3.1...3.16)
project(imgui_console VERSION 1.0 LANGUAGES CXX)

add_library(imgui_console STATIC "./include/imgui_console/imgui_console.h" "./src/imgui_console.cpp")