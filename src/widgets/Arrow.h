#pragma once

#include "Widget.h"

namespace mc_rtc::blender
{

struct Arrow : public Widget
{
  Arrow(Client & client, const ElementId & id, const ElementId & reqId) : Widget(client, id), requestId_(reqId) {}

  void data(const Eigen::Vector3d & start,
            const Eigen::Vector3d & end,
            const mc_rtc::gui::ArrowConfig & config,
            bool ro)
  {
    config_ = config;
  }

  void draw3D() override
  {
    /** FIXME Implement */
  }

private:
  ElementId requestId_;
  mc_rtc::gui::ArrowConfig config_;
};

} // namespace mc_rtc::blender