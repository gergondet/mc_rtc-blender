#pragma once

#include "details/TransformBase.h"

namespace mc_rtc::blender
{

struct Rotation : public TransformBase<ControlAxis::ROTATION>
{
  Rotation(Client & client, const ElementId & id, Interface3D & gui, const ElementId & reqId) : TransformBase(client, id, gui, reqId) {}

  void draw3D() override
  {
    TransformBase::draw3D();
    /** FIXME Implement */
  }
};

} // namespace mc_rtc::blender
