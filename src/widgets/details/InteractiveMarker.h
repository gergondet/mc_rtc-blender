#pragma once

#include "../Widget.h"

#include "ControlAxis.h"

namespace mc_rtc::blender
{

template<ControlAxis ctl>
struct InteractiveMarker
{
  template<typename Callback>
  InteractiveMarker(Client & client, const ElementId & id, Interface3D & gui, Callback && cb) : client_(client), gui_(gui)
  {
    marker_ = gui_.add_interactive_marker(id.category, id.name, ctl, cb);
  }

  ~InteractiveMarker()
  {
    gui_.remove_interactive_marker(marker_);
  }

  void update(bool ro, const sva::PTransformd & pos)
  {
    gui_.update_interactive_marker(marker_, ro, pos);
  }

  void hidden(bool hidden)
  {
    gui_.set_marker_hidden(marker_, hidden);
  }

private:
  Client & client_;
  Interface3D & gui_;
  std::string marker_;
};

} // namespace mc_rtc::blender
