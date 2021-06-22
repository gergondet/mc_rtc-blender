#include <pybind11/pybind11.h>

#include <pybind11/eigen.h>
#include <pybind11/functional.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include <imgui.h>
#include <imgui_internal.h>

#include "Client.h"

namespace py = pybind11;

struct ImDrawListProxy
{
  int Size;
  ImDrawList ** Data;

  ImDrawListProxy(ImDrawData & source) : Size(source.CmdListsCount), Data(source.CmdLists) {}

  ImDrawList ** begin()
  {
    return Data;
  }

  ImDrawList ** end()
  {
    return Data + Size;
  }
};

struct BlenderInterface : public Interface3D
{
  ~BlenderInterface() override = default;

  std::string add_collection(const std::vector<std::string> & category, const std::string & name) override
  {
    PYBIND11_OVERRIDE_PURE(std::string, Interface3D, add_collection, category, name);
  }

  void hide_collection(const std::string & name, bool hide) override
  {
    PYBIND11_OVERRIDE_PURE(void, Interface3D, hide_collection, name, hide);
  }

  void remove_collection(const std::string & name) override
  {
    PYBIND11_OVERRIDE_PURE(void, Interface3D, remove_collection, name);
  }

  std::string load_mesh(const std::string & collection,
                        const std::string & meshPath,
                        const std::string & meshName,
                        const std::array<double, 4> & defaultColor) override
  {
    PYBIND11_OVERRIDE_PURE(std::string, Interface3D, load_mesh, collection, meshPath, meshName, defaultColor);
  }

  void set_mesh_position(const std::string & meshName, const sva::PTransformd & pose) override
  {
    PYBIND11_OVERRIDE_PURE(void, Interface3D, set_mesh_position, meshName, pose);
  }

  void remove_mesh(const std::string & meshName) override
  {
    PYBIND11_OVERRIDE_PURE(void, Interface3D, remove_mesh, meshName);
  }

  std::string add_interactive_marker(const std::vector<std::string> & category,
                                     const std::string & name,
                                     const mc_rtc::blender::ControlAxis & axis,
                                     const std::function<void(const sva::PTransformd &)> & callback) override
  {
    PYBIND11_OVERRIDE_PURE(std::string, Interface3D, add_interactive_marker, category, name, axis, callback);
  }

  void update_interactive_marker(const std::string & name, bool ro, const sva::PTransformd & pos) override
  {
    PYBIND11_OVERRIDE_PURE(void, Interface3D, update_interactive_marker, name, ro, pos);
  }

  void set_marker_hidden(const std::string & name, bool hidden) override
  {
    PYBIND11_OVERRIDE_PURE(void, Interface3D, set_marker_hidden, name, hidden);
  }

  void remove_interactive_marker(const std::string & name) override
  {
    PYBIND11_OVERRIDE_PURE(void, Interface3D, remove_interactive_marker, name);
  }
};

PYBIND11_MODULE(mc_rtc_blender, m)
{
  m.doc() = "mc_rtc helper for Blender plugin";

  py::class_<Interface3D, BlenderInterface>(m, "Interface3D").def(py::init<>());

  py::enum_<mc_rtc::blender::ControlAxis>(m, "ControlAxis", py::arithmetic())
      .value("NONE", mc_rtc::blender::ControlAxis::NONE)
      .value("TX", mc_rtc::blender::ControlAxis::TX)
      .value("TY", mc_rtc::blender::ControlAxis::TY)
      .value("TZ", mc_rtc::blender::ControlAxis::TZ)
      .value("RX", mc_rtc::blender::ControlAxis::RX)
      .value("RY", mc_rtc::blender::ControlAxis::RY)
      .value("RZ", mc_rtc::blender::ControlAxis::RZ)
      .value("TRANSLATION", mc_rtc::blender::ControlAxis::TRANSLATION)
      .value("ROTATION", mc_rtc::blender::ControlAxis::ROTATION)
      .value("XYTHETA", mc_rtc::blender::ControlAxis::XYTHETA)
      .value("XYZTHETA", mc_rtc::blender::ControlAxis::XYZTHETA)
      .value("ALL", mc_rtc::blender::ControlAxis::ALL)
      .def("__bool__", [](mc_rtc::blender::ControlAxis ax) { return ax != mc_rtc::blender::ControlAxis::NONE; })
      .def("__or__",
           py::overload_cast<mc_rtc::blender::ControlAxis, mc_rtc::blender::ControlAxis>(&mc_rtc::blender::operator|))
      .def("__and__",
           py::overload_cast<mc_rtc::blender::ControlAxis, mc_rtc::blender::ControlAxis>(&mc_rtc::blender::operator&));

  py::class_<Eigen::Quaterniond>(m, "Quaterniond")
      .def(py::init<double, double, double, double>())
      .def("inverse", [](const Eigen::Quaterniond & q) -> Eigen::Quaterniond { return q.inverse().normalized(); })
      .def("w", [](const Eigen::Quaterniond & q) { return q.w(); })
      .def("x", [](const Eigen::Quaterniond & q) { return q.x(); })
      .def("y", [](const Eigen::Quaterniond & q) { return q.y(); })
      .def("z", [](const Eigen::Quaterniond & q) { return q.z(); });

  py::class_<sva::PTransformd>(m, "PTransformd")
      .def_static("Identity", &sva::PTransformd::Identity)
      .def_property(
          "translation", [](const sva::PTransformd & pt) -> const Eigen::Vector3d & { return pt.translation(); },
          [](sva::PTransformd & pt, const Eigen::Vector3d & t) { pt.translation() = t; })
      .def_property(
          "rotation", [](const sva::PTransformd & pt) { return Eigen::Quaterniond(pt.rotation()); },
          [](sva::PTransformd & pt, const Eigen::Quaterniond & q) { pt.rotation() = q.toRotationMatrix(); });

  py::class_<mc_rtc::blender::Client>(m, "Client")
      .def(py::init<Interface3D &>())
      .def("connect", static_cast<void (mc_rtc::blender::Client::*)(const std::string &, const std::string &)>(
                          &mc_rtc::blender::Client::connect))
      .def("timeout", static_cast<void (mc_rtc::blender::Client::*)(double)>(&mc_rtc::blender::Client::timeout))
      .def("update", &mc_rtc::blender::Client::update)
      .def("draw2D", &mc_rtc::blender::Client::draw2D)
      .def("draw3D", &mc_rtc::blender::Client::draw3D);

  m.attr("INDEX_SIZE") = sizeof(ImDrawIdx);
  m.attr("VERTEX_SIZE") = sizeof(ImDrawVert);

  py::enum_<ImGuiKey_>(m, "ImGuiKey", py::arithmetic())
      .value("KEY_TAB", ImGuiKey_Tab)
      .value("KEY_LEFT_ARROW", ImGuiKey_LeftArrow)
      .value("KEY_RIGHT_ARROW", ImGuiKey_RightArrow)
      .value("KEY_UP_ARROW", ImGuiKey_UpArrow)
      .value("KEY_DOWN_ARROW", ImGuiKey_DownArrow)
      .value("KEY_PAGE_UP", ImGuiKey_PageUp)
      .value("KEY_PAGE_DOWN", ImGuiKey_PageDown)
      .value("KEY_HOME", ImGuiKey_Home)
      .value("KEY_END", ImGuiKey_End)
      .value("KEY_INSERT", ImGuiKey_Insert)
      .value("KEY_DELETE", ImGuiKey_Delete)
      .value("KEY_BACKSPACE", ImGuiKey_Backspace)
      .value("KEY_SPACE", ImGuiKey_Space)
      .value("KEY_ENTER", ImGuiKey_Enter)
      .value("KEY_ESCAPE", ImGuiKey_Escape)
      .value("KEY_A", ImGuiKey_A)
      .value("KEY_C", ImGuiKey_C)
      .value("KEY_V", ImGuiKey_V)
      .value("KEY_X", ImGuiKey_X)
      .value("KEY_Y", ImGuiKey_Y)
      .value("KEY_Z", ImGuiKey_Z)
      .export_values();

  py::class_<ImGuiContext>(m, "ImGuiContext");
  m.def(
      "create_context", []() { return ImGui::CreateContext(); }, py::return_value_policy::reference);
  m.def(
      "get_current_context", []() { return ImGui::GetCurrentContext(); }, py::return_value_policy::reference);
  m.def("destroy_context", &ImGui::DestroyContext);

  py::class_<ImFontAtlas>(m, "ImFontAtlas")
      .def("clear_tex_data", &ImFontAtlas::ClearTexData)
      .def("get_tex_data_as_rgba32",
           [](ImFontAtlas & self) -> std::tuple<int, int, py::bytes> {
             int width;
             int height;
             unsigned char * pixels;
             self.GetTexDataAsRGBA32(&pixels, &width, &height);
             return {width, height, py::bytes(reinterpret_cast<char *>(pixels), 4 * width * height)};
           })
      .def_property(
          "texture_id", [](ImFontAtlas & self) { return uint64_t(self.TexID); },
          [](ImFontAtlas & self, uint64_t value) { self.TexID = (void *)(value); });

  py::class_<ImGuiIO>(m, "ImGuiIO")
      .def_readwrite("delta_time", &ImGuiIO::DeltaTime)
      .def_property_readonly("key_map",
                             [](py::object & obj) {
                               ImGuiIO & io = obj.cast<ImGuiIO &>();
                               return py::array{ImGuiKey_COUNT, io.KeyMap, obj};
                             })
      .def_property_readonly("display_fb_scale",
                             [](ImGuiIO & io) {
                               return std::array<float, 2>{io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y};
                             })
      .def_property(
          "display_size",
          [](ImGuiIO & io) {
            return std::array<float, 2>{io.DisplaySize.x, io.DisplaySize.y};
          },
          [](ImGuiIO * self, const std::array<float, 2> & value) {
            self->DisplaySize.x = value[0];
            self->DisplaySize.y = value[1];
          })
      .def_property_readonly("keys_down",
                             [](py::object & obj) {
                               ImGuiIO & io = obj.cast<ImGuiIO &>();
                               return py::array{512, io.KeysDown, obj};
                             })
      .def_readwrite("key_ctrl", &ImGuiIO::KeyCtrl)
      .def_readwrite("key_shift", &ImGuiIO::KeyShift)
      .def_readwrite("key_alt", &ImGuiIO::KeyAlt)
      .def_readwrite("key_super", &ImGuiIO::KeySuper)
      .def_property_readonly("mouse_down",
                             [](py::object & obj) {
                               ImGuiIO & io = obj.cast<ImGuiIO &>();
                               return py::array{5, io.MouseDown, obj};
                             })
      .def_property(
          "mouse_pos",
          [](ImGuiIO & io) {
            return std::array<float, 2>{io.MousePos.x, io.MousePos.y};
          },
          [](ImGuiIO * self, const std::array<float, 2> & value) {
            self->MousePos.x = value[0];
            self->MousePos.y = value[1];
          })
      .def_readwrite("mouse_wheel", &ImGuiIO::MouseWheel)
      .def_readwrite("font_global_scale", &ImGuiIO::FontGlobalScale)
      .def_readwrite("fonts", &ImGuiIO::Fonts, py::return_value_policy::reference)
      .def_readonly("want_capture_mouse", &ImGuiIO::WantCaptureMouse)
      .def_readonly("want_capture_keyboard", &ImGuiIO::WantCaptureKeyboard)
      .def("add_input_character", &ImGuiIO::AddInputCharacter);

  m.def("get_io", &ImGui::GetIO, py::return_value_policy::reference);

  m.def("new_frame", &ImGui::NewFrame);
  m.def("end_frame", &ImGui::EndFrame);
  m.def("render", &ImGui::Render);

  py::class_<ImDrawCmd>(m, "ImDrawCmd")
      .def_property_readonly("clip_rect",
                             [](const ImDrawCmd & s) {
                               const auto & r = s.ClipRect;
                               return std::array<float, 4>{r.x, r.y, r.z, r.w};
                             })
      .def_property_readonly("texture_id", [](ImDrawCmd & self) { return uint64_t(self.TextureId); })
      .def_readonly("elem_count", &ImDrawCmd::ElemCount);

  py::class_<ImVector<ImDrawCmd>>(m, "_ImVectorOfImDrawCmd")
      .def("__len__", [](const ImVector<ImDrawCmd> & self) { return self.Size; })
      .def(
          "__iter__", [](ImVector<ImDrawCmd> & self) { return py::make_iterator(self.begin(), self.end()); },
          py::keep_alive<0, 1>());

  py::class_<ImDrawList>(m, "ImDrawList")
      .def_property_readonly("idx_buffer_size", [](ImDrawList & self) { return self.IdxBuffer.Size; })
      .def_property_readonly("idx_buffer_data", [](ImDrawList & self) { return std::uintptr_t(self.IdxBuffer.Data); })
      .def_property_readonly("vtx_buffer_size", [](ImDrawList & self) { return self.VtxBuffer.Size; })
      .def_property_readonly("vtx_buffer_data", [](ImDrawList & self) { return std::uintptr_t(self.VtxBuffer.Data); })
      .def_readonly("commands", &ImDrawList::CmdBuffer);

  py::class_<ImDrawListProxy>(m, "_ImDrawListProxy")
      .def("__len__", [](const ImDrawListProxy & self) { return self.Size; })
      .def(
          "__iter__", [](ImDrawListProxy & self) { return py::make_iterator(self.begin(), self.end()); },
          py::keep_alive<0, 1>());

  py::class_<ImDrawData>(m, "ImDrawData")
      .def("scale_clip_rects",
           [](ImDrawData & self, float x, float y) {
             self.ScaleClipRects({x, y});
           })
      .def_property_readonly("commands_lists", [](ImDrawData & self) { return ImDrawListProxy(self); });

  m.def("get_draw_data", &ImGui::GetDrawData, py::return_value_policy::reference);

  m.def("show_demo_window", []() { ImGui::ShowDemoWindow(); });
  m.def("begin", [](const char * name, bool open) { return ImGui::Begin(name, &open); });
  m.def("text", [](const char * label) { return ImGui::Text(label); });
  m.def("end", &ImGui::End);
}
