#pragma once

#include "details/TransformBase.h"

namespace mc_rtc::blender
{

struct Rotation : public TransformBase<ControlAxis::ROTATION>
{
  Rotation(Client & client, const ElementId & id, const ElementId & reqId) : TransformBase(client, id, reqId) {}

  void draw3D() override
  {
    TransformBase::draw3D();
    /** FIXME Implement */
  }
};

} // namespace mc_rtc::blender