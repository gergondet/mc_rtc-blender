#pragma once

#include "details/InteractiveMarker.h"

namespace mc_rtc::blender
{

struct Arrow : public Widget
{
  Arrow(Client & client, const ElementId & id, Interface3D & gui, const ElementId & reqId)
  : Widget(client, id, gui), requestId_(reqId), arrow_(gui, id.category, id.name),
    startMarker_(client,
                 id,
                 gui,
                 [&client, this](const sva::PTransformd & pos) {
                   Eigen::Vector6d data;
                   data << pos.translation(), end_;
                   client.send_request(requestId_, data);
                 }),
    endMarker_(client, id, gui, [&client, this](const sva::PTransformd & pos) {
      Eigen::Vector6d data;
      data << start_, pos.translation();
      client.send_request(requestId_, data);
    })
  {
  }

  void data(const Eigen::Vector3d & start,
            const Eigen::Vector3d & end,
            const mc_rtc::gui::ArrowConfig & config,
            bool ro)
  {
    if(ro != ro_)
    {
      ro_ = ro;
      startMarker_.hidden(ro_);
      endMarker_.hidden(ro_);
    }
    start_ = start;
    startMarker_.update(ro, {start});
    end_ = end;
    endMarker_.update(ro, {end});
    arrow_.update(start, end, config);
  }

  void draw3D() override {}

private:
  ElementId requestId_;
  bool ro_ = false;
  Eigen::Vector3d start_ = Eigen::Vector3d::Zero();
  InteractiveMarker<ControlAxis::TRANSLATION> startMarker_;
  Eigen::Vector3d end_ = Eigen::Vector3d::Zero();
  InteractiveMarker<ControlAxis::TRANSLATION> endMarker_;
  Interface3D::Arrow arrow_;
};

} // namespace mc_rtc::blender
