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
  }

  ~TransformBase() override = default;

  void data(bool ro, const sva::PTransformd & pos)
  {
    /** FIXME */
  }

  void draw3D() override
  {
    /** FIXME */
  }

protected:
  ElementId requestId_;
};

} // namespace mc_rtc::blender