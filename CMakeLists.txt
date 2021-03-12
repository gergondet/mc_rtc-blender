cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 17)

project(mc_rtc-blender LANGUAGES CXX)

find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
add_subdirectory(ext/pybind11 EXCLUDE_FROM_ALL)

find_package(mc_rtc REQUIRED)

find_package(Boost REQUIRED COMPONENTS filesystem)

set(client_SRC
  src/Category.h
  src/Category.cpp
  src/Client.h
  src/Client.cpp
  src/Widget.h
  src/widgets/ArrayInput.h
  src/widgets/ArrayLabel.h
  src/widgets/Arrow.h
  src/widgets/Button.h
  src/widgets/Checkbox.h
  src/widgets/ComboInput.h
  src/widgets/DataComboInput.h
  src/widgets/Force.h
  src/widgets/Form.h
  src/widgets/IntegerInput.h
  src/widgets/Label.h
  src/widgets/NumberInput.h
  src/widgets/NumberSlider.h
  src/widgets/Point3D.cpp
  src/widgets/Point3D.h
  src/widgets/Polygon.h
  src/widgets/Robot.cpp
  src/widgets/Robot.h
  src/widgets/Rotation.h
  src/widgets/Schema.cpp
  src/widgets/Schema.h
  src/widgets/StringInput.h
  src/widgets/Table.h
  src/widgets/Trajectory.h
  src/widgets/Transform.h
  src/widgets/utils.h
  src/widgets/Widget.h
  src/widgets/XYTheta.h
  src/widgets/form/schema.h
  src/widgets/form/schema.cpp
  src/widgets/form/widgets.h
  src/widgets/form/widgets.cpp
  src/widgets/details/ControlAxis.h
  src/widgets/details/SingleInput.h
  src/widgets/details/TransformBase.h
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