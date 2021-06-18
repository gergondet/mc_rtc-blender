#pragma once

#include "../Widget.h"

#include "ControlAxis.h"

namespace mc_rtc::blender
{

template<ControlAxis ctl>
struct TransformBase : public Widget
{
  TransformBase(Client & client, const ElementId & id, const ElementId & requestId)
  : Widget(client, id), requestId_(requestId)
  {
    markerName_ =
        client.gui().add_interactive_marker(id.category, id.name, ctl, [&client, this](const sva::PTransformd & pos) {
          if constexpr(ctl == ControlAxis::TRANSLATION)
          {
            client.send_request(requestId_, pos.translation());
          }
          else if constexpr(ctl == ControlAxis::ROTATION)
          {
            client.send_request(requestId_, pos.rotation());
          }
          else if constexpr(ctl == ControlAxis::ALL)
          {
            client.send_request(requestId_, pos);
          }
          else if constexpr(ctl == ControlAxis::XYTHETA || ctl == ControlAxis::XYZTHETA)
          {
            Eigen::VectorXd data = Eigen::VectorXd::Zero(4);
            const auto & t = pos.translation();
            auto yaw = mc_rbdyn::rpyFromMat(pos.rotation()).z();
            data(0) = t.x();
            data(1) = t.y();
            data(2) = yaw;
            data(3) = t.z();
            client.send_request(requestId_, data);
          }
        });
  }

  ~TransformBase() override
  {
    client.gui().remove_interactive_marker(markerName_);
  }

  void data(bool ro, const sva::PTransformd & pos)
  {
    client.gui().update_interactive_marker(markerName_, ro, pos);
  }

  void draw3D() override {}

protected:
  ElementId requestId_;
  std::string markerName_;
};

} // namespace mc_rtc::blender
