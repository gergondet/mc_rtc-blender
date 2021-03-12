#pragma once

#include "details/TransformBase.h"

struct TransformWidget : public TransformBase<ControlAxis::ALL>
{
  TransformWidget(Client & client, const ElementId & id, const ElementId & reqId) : TransformBase(client, id, reqId) {}

  void draw3D() override
  {
    TransformBase::draw3D();
    /** FIXME Implement */
    //client.gui().drawFrame(convert(marker_.pose()));
  }
};
