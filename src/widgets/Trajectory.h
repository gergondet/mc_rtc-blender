#pragma once

#include "Widget.h"

namespace mc_rtc::blender
{

template<typename T>
struct Trajectory : public Widget
{
  Trajectory(Client & client, const ElementId & id, Interface3D & gui) : Widget(client, id, gui) {}

  void data(const T & point, const mc_rtc::gui::LineConfig & config)
  {
    points_.push_back(point);
    config_ = config;
  }

  void data(const std::vector<T> & points, const mc_rtc::gui::LineConfig & config)
  {
    points_ = points;
    config_ = config;
  }

  void draw3D() override
  {
    if(points_.size() < 2)
    {
      return;
    }
    /** FIXME Implement */
    // auto c = convert(config_.color);
    // for(size_t i = 0; i < points_.size() - 1; ++i)
    //{
    //  const auto & p0 = points_[i];
    //  const auto & p1 = points_[i + 1];
    //  client.gui().drawLine(translation(p0), translation(p1), c);
    //}
    // if constexpr(std::is_same_v<T, sva::PTransformd>)
    //{
    //  if(points_.size() < 10) // For "small" trajectories, display all points
    //  {
    //    for(const auto & p : points_)
    //    {
    //      client.gui().drawFrame(convert(p));
    //    }
    //  }
    //  else // Otherwise draw the start and end points
    //  {
    //    client.gui().drawFrame(convert(points_[0]));
    //    client.gui().drawFrame(convert(points_.back()));
    //  }
    //}
    // else
    //{
    //  client.gui().drawCube(translation(points_[0]), {}, {0.04, 0.04, 0.04}, c);
    //  client.gui().drawSphere(translation(points_.back()), 0.04, c);
    //}
  }

private:
  std::vector<T> points_;
  mc_rtc::gui::LineConfig config_;
};

} // namespace mc_rtc::blender
