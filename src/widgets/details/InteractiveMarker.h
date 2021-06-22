#pragma once

#include "../Widget.h"

#include "ControlAxis.h"

namespace mc_rtc::blender
{

template<ControlAxis ctl>
struct InteractiveMarker
{
  template<typename Callback>
  InteractiveMarker(Client & client, const ElementId & id, Callback && cb) : client_(client)
  {
    marker_ = client.gui().add_interactive_marker(id.category, id.name, ctl, cb);
  }

  ~InteractiveMarker()
  {
    client_.gui().remove_interactive_marker(marker_);
  }

  void update(bool ro, const sva::PTransformd & pos)
  {
    client_.gui().update_interactive_marker(marker_, ro, pos);
  }

  void hidden(bool hidden)
  {
    client_.gui().set_marker_hidden(marker_, hidden);
  }

private:
  Client & client_;
  std::string marker_;
};

} // namespace mc_rtc::blender
