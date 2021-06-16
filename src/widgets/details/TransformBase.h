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
          client.send_request(this->id, pos);
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
