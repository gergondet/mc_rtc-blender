cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 17)

project(mc_rtc-blender LANGUAGES CXX)

find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
add_subdirectory(ext/pybind11 EXCLUDE_FROM_ALL)

find_package(mc_rtc REQUIRED)

find_package(Boost REQUIRED COMPONENTS filesystem)

add_subdirectory(src/mc_rtc-imgui)

set(client_SRC
  src/BlenderClient.h
  src/BlenderClient.cpp
  src/widgets/Arrow.h
  src/widgets/Force.h
  src/widgets/Point3D.cpp
  src/widgets/Point3D.h
  src/widgets/Polygon.h
  src/widgets/Robot.cpp
  src/widgets/Robot.h
  src/widgets/Rotation.h
  src/widgets/Trajectory.h
  src/widgets/Transform.h
  src/widgets/utils.h
  src/widgets/Widget.h
  src/widgets/XYTheta.h
  src/widgets/details/ControlAxis.h
  src/widgets/details/InteractiveMarker.h
  src/widgets/details/TransformBase.h
  ${mc_rtc-imgui-SRC}
  ${mc_rtc-imgui-HDR}
)

set(imgui_SRC
  src/imgui_config.h
  ext/imgui/imgui.cpp
  ext/imgui/imgui_demo.cpp
  ext/imgui/imgui_draw.cpp
  ext/imgui/imgui_tables.cpp
  ext/imgui/imgui_widgets.cpp
)

pybind11_add_module(mc_rtc_blender src/mc_rtc_blender.cpp ${imgui_SRC} ${client_SRC})
target_link_libraries(mc_rtc_blender PUBLIC mc_rtc::mc_control_client Boost::filesystem)
target_include_directories(mc_rtc_blender PUBLIC ext/imgui src)
target_compile_definitions(mc_rtc_blender PUBLIC -DIMGUI_USER_CONFIG="imgui_config.h")
set_target_properties(mc_rtc_blender PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})
foreach(CFG ${CMAKE_CONFIGURATION_TYPES})
  string(TOUPPER "${CFG}" CFG)
  set_target_properties(mc_rtc_blender PROPERTIES LIBRARY_OUTPUT_DIRECTORY_${CFG} ${PROJECT_SOURCE_DIR})
endforeach()
if(TARGET mc_rtc::mc_rtc_ros)
  target_compile_definitions(mc_rtc_blender PUBLIC MC_RTC_HAS_ROS_SUPPORT)
  target_link_libraries(mc_rtc_blender PUBLIC mc_rtc::mc_rtc_ros)
endif()
