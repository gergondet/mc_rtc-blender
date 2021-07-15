#pragma once

#include "details/TransformBase.h"

namespace mc_rtc::blender
{

struct XYTheta : public TransformBase<ControlAxis::XYZTHETA>
{
  XYTheta(Client & client, const ElementId & id, Interface3D & gui, const ElementId & reqId) : TransformBase(client, id, gui, reqId) {}

  void data(bool ro, const Eigen::Vector3d & xytheta, double altitude)
  {
    TransformBase::data(ro, {sva::RotZ(xytheta.z()), {xytheta.x(), xytheta.y(), altitude}});
  }

  void draw3D() override
  {
    TransformBase::draw3D();
    /** FIXME Implement */
    // client.gui().drawFrame(convert(marker_.pose()));
  }
};

} // namespace mc_rtc::blender
