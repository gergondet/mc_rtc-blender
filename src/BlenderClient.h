#pragma once

#include "mc_rtc-imgui/Client.h"

#include "Interface3D.h"

namespace mc_rtc::blender
{

using Client = mc_rtc::imgui::Client;
using ElementId = mc_rtc::imgui::ElementId;

struct BlenderClient : public mc_rtc::imgui::Client
{
  BlenderClient(Interface3D & gui) : mc_rtc::imgui::Client{}, gui_(gui) {}

private:
  Interface3D & gui_;

  void point3d(const ElementId & id,
               const ElementId & requestId,
               bool ro,
               const Eigen::Vector3d & pos,
               const mc_rtc::gui::PointConfig & config) override;

  void trajectory(const ElementId & id,
                  const std::vector<Eigen::Vector3d> & points,
                  const mc_rtc::gui::LineConfig & config) override;

  void trajectory(const ElementId & id,
                  const std::vector<sva::PTransformd> & points,
                  const mc_rtc::gui::LineConfig & config) override;

  void trajectory(const ElementId & id, const Eigen::Vector3d & point, const mc_rtc::gui::LineConfig & config) override;

  void trajectory(const ElementId & id,
                  const sva::PTransformd & point,
                  const mc_rtc::gui::LineConfig & config) override;

  void polygon(const ElementId & id,
               const std::vector<std::vector<Eigen::Vector3d>> & points,
               const mc_rtc::gui::LineConfig & config) override;

  inline void polygon(const ElementId & id,
                      const std::vector<std::vector<Eigen::Vector3d>> & points,
                      const mc_rtc::gui::Color & color) override
  {
    polygon(id, points, mc_rtc::gui::LineConfig(color));
  }

  void force(const ElementId & id,
             const ElementId & requestId,
             const sva::ForceVecd & force,
             const sva::PTransformd & pos,
             const mc_rtc::gui::ForceConfig & forceConfig,
             bool /* ro */) override;

  void arrow(const ElementId & id,
             const ElementId & requestId,
             const Eigen::Vector3d & start,
             const Eigen::Vector3d & end,
             const mc_rtc::gui::ArrowConfig & config,
             bool ro) override;

  void rotation(const ElementId & id, const ElementId & requestId, bool ro, const sva::PTransformd & pos) override;

  void transform(const ElementId & id, const ElementId & requestId, bool ro, const sva::PTransformd & pos) override;

  void xytheta(const ElementId & id,
               const ElementId & requestId,
               bool ro,
               const Eigen::Vector3d & xytheta,
               double altitude) override;

  void robot(const ElementId & id,
             const std::vector<std::string> & params,
             const std::vector<std::vector<double>> & q,
             const sva::PTransformd & posW) override;
};

} // namespace mc_rtc::blender
