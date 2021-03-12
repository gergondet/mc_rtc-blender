#pragma once

#include "Widget.h"

struct Polygon : public Widget
{
  Polygon(Client & client, const ElementId & id) : Widget(client, id) {}

  void data(const std::vector<std::vector<Eigen::Vector3d>> & points, const mc_rtc::gui::LineConfig & config)
  {
    if(points_ != points)
    {
      points_ = points;
    }
    config_ = config;
  }

  void draw3D() override
  {
    /** FIXME Implement */
  }

private:
  std::vector<std::vector<Eigen::Vector3d>> points_;
  mc_rtc::gui::LineConfig config_;
};
