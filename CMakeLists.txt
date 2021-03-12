cmake_minimum_required(VERSION 3.12)

project(mc_rtc-blender LANGUAGES CXX)

find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
add_subdirectory(ext/pybind11 EXCLUDE_FROM_ALL)


set(imgui_SRC
  ext/imgui/imgui.cpp
  ext/imgui/imgui_demo.cpp
  ext/imgui/imgui_draw.cpp
  ext/imgui/imgui_tables.cpp
  ext/imgui/imgui_widgets.cpp
)

pybind11_add_module(mc_rtc_blender src/mc_rtc_blender.cpp ${imgui_SRC})
target_include_directories(mc_rtc_blender PUBLIC ext/imgui src)
target_compile_definitions(mc_rtc_blender PUBLIC -DIMGUI_USER_CONFIG="imgui_config.h")
set_target_properties(mc_rtc_blender PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})
