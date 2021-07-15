#pragma once

/** Helper header to have all the stuff available in widgets */

#include "../BlenderClient.h"
#include "../mc_rtc-imgui/Widget.h"

#include "utils.h"

#include "imgui.h"

namespace mc_rtc::blender
{

struct Widget : public mc_rtc::imgui::Widget
{
  Widget(Client & client, const ElementId & id, Interface3D & gui) : mc_rtc::imgui::Widget(client, id), gui_(gui) {}

  inline Interface3D & gui() noexcept
  {
    return gui_;
  }

protected:
  Interface3D & gui_;
};

} // namespace mc_rtc::blender
