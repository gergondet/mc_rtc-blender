#include "Robot.h"

#include <boost/filesystem.hpp>
namespace bfs = boost::filesystem;

#ifdef MC_RTC_HAS_ROS_SUPPORT
#  include <ros/package.h>
#endif

#include <mc_rbdyn/RobotLoader.h>
#include <mc_rbdyn/Robots.h>

#include <mc_rtc/config.h>
#include <mc_rtc/version.h>

namespace mc_rtc::blender
{

namespace details
{

static constexpr int MC_RTC_VERSION_MAJOR = mc_rtc::MC_RTC_VERSION[0] - '0';

template<typename T>
void setConfiguration(T & robot, const std::vector<std::vector<double>> & q)
{
  static_assert(std::is_same_v<T, mc_rbdyn::Robot>);
  if constexpr(MC_RTC_VERSION_MAJOR > 1)
  {
    robot.q()->set(rbd::paramToVector(robot.mb(), q));
  }
  else
  {
    robot.mbc().q = q;
  }
}

inline bfs::path convertURI(const mc_rbdyn::RobotModule & rm, const std::string & uri)
{
  const std::string package = "package://";
  if(uri.size() >= package.size() && uri.find(package) == 0)
  {
    size_t split = uri.find('/', package.size());
    std::string pkg = uri.substr(package.size(), split - package.size());
    auto leaf = bfs::path(uri.substr(split + 1));
    bfs::path MC_ENV_DESCRIPTION_PATH(mc_rtc::MC_ENV_DESCRIPTION_PATH);
#ifndef __EMSCRIPTEN__
#  ifndef MC_RTC_HAS_ROS_SUPPORT
    // FIXME Prompt the user for unknown packages
    if(pkg == "jvrc_description")
    {
      pkg = (MC_ENV_DESCRIPTION_PATH / ".." / "jvrc_description").string();
    }
    else if(pkg == "mc_env_description")
    {
      pkg = MC_ENV_DESCRIPTION_PATH.string();
    }
    else if(pkg == "mc_int_obj_description")
    {
      pkg = (MC_ENV_DESCRIPTION_PATH / ".." / "mc_int_obj_description").string();
    }
    else
    {
      mc_rtc::log::warning("Cannot resolve package: {}, assuming it's {}", pkg, rm.path);
      pkg = rm.path;
    }
#  else
    pkg = ros::package::getPath(pkg);
#  endif
#else
    pkg = "/assets/" + pkg;
#endif
    return pkg / leaf;
  }
  const std::string file = "file://";
  if(uri.size() >= file.size() && uri.find(file) == 0)
  {
    return bfs::path(uri.substr(file.size()));
  }
  return uri;
}

mc_rbdyn::RobotModulePtr fromParams(const std::vector<std::string> & p)
{
  mc_rbdyn::RobotModulePtr rm{nullptr};
  if(p.size() == 1)
  {
    rm = mc_rbdyn::RobotLoader::get_robot_module(p[0]);
  }
  if(p.size() == 2)
  {
    rm = mc_rbdyn::RobotLoader::get_robot_module(p[0], p[1]);
  }
  if(p.size() == 3)
  {
    rm = mc_rbdyn::RobotLoader::get_robot_module(p[0], p[1], p[2]);
  }
  if(p.size() > 3)
  {
    mc_rtc::log::warning("Too many parameters provided to load the robot, complain to the developpers of this package");
  }
  return rm;
}

inline std::array<double, 4> color(const rbd::parsers::Material & m)
{
  if(m.type == rbd::parsers::Material::Type::COLOR)
  {
    const auto & c = boost::get<rbd::parsers::Material::Color>(m.data);
    return {c.r, c.g, c.b, c.a};
  }
  return {0.8, 0.8, 0.8, 1.0};
}

struct RobotImpl
{
  RobotImpl(Robot & robot)
  : self_(robot), collectionVisual_(gui(), id().category, id().name + "/visual"),
    collectionCollision_(gui(), id().category, id().name + "/collision")
  {
    collectionCollision_.hide(true);
    if(id().category.size() > 1)
    {
      drawVisualModel_ = false;
      collectionVisual_.hide(true);
    }
  }

  ~RobotImpl() {}

  inline mc_rbdyn::Robot & robot()
  {
    return robots_->robot();
  }

  inline const mc_control::ElementId & id()
  {
    return self_.id;
  }

  inline Interface3D & gui()
  {
    return self_.client.gui();
  }

  void data(const std::vector<std::string> & params,
            const std::vector<std::vector<double>> & q,
            const sva::PTransformd & posW)
  {
    if(!robots_ || robot().module().parameters() != params)
    {
      auto rm = fromParams(params);
      if(!rm)
      {
        return;
      }
      robots_ = mc_rbdyn::loadRobot(*rm);
      auto loadMeshCallback = [&](std::vector<std::function<void()>> & draws, Collection & collection, size_t bIdx,
                                  const rbd::parsers::Visual & visual) {
        const auto & meshInfo = boost::get<rbd::parsers::Geometry::Mesh>(visual.geometry.data);
        auto path = convertURI(*rm, meshInfo.filename);
        auto mesh = std::make_shared<Mesh>(collection, path.string(), robot().mb().body(bIdx).name(), color(visual.material));
        draws.push_back([this, bIdx, visual, mesh]() {
          const auto & X_0_b = visual.origin * robot().mbc().bodyPosW[bIdx];
          mesh->set_position(X_0_b);
        });
      };
      auto loadBoxCallback = [&](std::vector<std::function<void()>> & draws, Collection & collection, size_t bIdx,
                                 const rbd::parsers::Visual & visual) {
        draws.push_back([this, bIdx, visual]() {
          const auto & box = boost::get<rbd::parsers::Geometry::Box>(visual.geometry.data);
          const auto & X_0_b = visual.origin * robot().mbc().bodyPosW[bIdx];
          // gui().drawCube(translation(X_0_b), convert(X_0_b.rotation()), translation(box.size),
          // color(visual.material));
        });
      };
      auto loadCylinderCallback = [&](std::vector<std::function<void()>> & draws, Collection & collection, size_t bIdx,
                                      const rbd::parsers::Visual & visual) {
        draws.push_back([this, bIdx, visual]() {
          const auto & cylinder = boost::get<rbd::parsers::Geometry::Cylinder>(visual.geometry.data);
          const auto & start = sva::PTransformd(Eigen::Vector3d{0.0, 0.0, -cylinder.length / 2}) * visual.origin
                               * robot().mbc().bodyPosW[bIdx];
          const auto & end = sva::PTransformd(Eigen::Vector3d{0.0, 0.0, cylinder.length}) * start;
          // gui().drawArrow(translation(start), translation(end), 2 * cylinder.radius, 0.0f, 0.0f,
          //                color(visual.material));
        });
      };
      auto loadSphereCallback = [&](std::vector<std::function<void()>> & draws, Collection & collection, size_t bIdx,
                                    const rbd::parsers::Visual & visual) {
        draws.push_back([this, bIdx, visual]() {
          const auto & sphere = boost::get<rbd::parsers::Geometry::Sphere>(visual.geometry.data);
          const auto & X_0_b = visual.origin * robot().mbc().bodyPosW[bIdx];
          // gui().drawSphere(translation(X_0_b), static_cast<float>(sphere.radius), color(visual.material));
        });
      };
      auto loadBodyCallbacks = [&](std::vector<std::function<void()>> & draws, Collection & collection, size_t bIdx,
                                   const std::vector<rbd::parsers::Visual> & visuals) {
        for(const auto & visual : visuals)
        {
          using Geometry = rbd::parsers::Geometry;
          switch(visual.geometry.type)
          {
            case Geometry::MESH:
              loadMeshCallback(draws, collection, bIdx, visual);
              break;
            case Geometry::BOX:
              loadBoxCallback(draws, collection, bIdx, visual);
              break;
            case Geometry::CYLINDER:
              loadCylinderCallback(draws, collection, bIdx, visual);
              break;
            case Geometry::SPHERE:
              loadSphereCallback(draws, collection, bIdx, visual);
              break;
            default:
              break;
          };
        }
      };
      auto loadCallbacks = [&](std::vector<std::function<void()>> & draws, Collection & collection,
                               const auto & visuals) {
        draws.clear();
        const auto & bodies = robot().mb().bodies();
        for(size_t i = 0; i < bodies.size(); ++i)
        {
          const auto & b = bodies[i];
          if(!visuals.count(b.name()))
          {
            continue;
          }
          loadBodyCallbacks(draws, collection, i, visuals.at(b.name()));
        }
      };
      loadCallbacks(drawVisual_, collectionVisual_, robot().module()._visual);
      loadCallbacks(drawCollision_, collectionCollision_, robot().module()._collision);
    }
    robot().posW(posW);
    setConfiguration(robot(), q);
  }

  void draw2D()
  {
    if(!robots_)
    {
      return;
    }
    if(ImGui::Button(self_.label(fmt::format("Reload {}", self_.id.name)).c_str()))
    {
      robots_.reset();
    }
    if(ImGui::Checkbox(self_.label(fmt::format("Draw {} visual model", self_.id.name)).c_str(), &drawVisualModel_))
    {
      collectionVisual_.hide(!drawVisualModel_);
    }

    if(ImGui::Checkbox(self_.label(fmt::format("Draw {} collision model", self_.id.name)).c_str(),
                       &drawCollisionModel_))
    {
      collectionCollision_.hide(!drawCollisionModel_);
    }
  }

  void draw3D()
  {
    if(!robots_)
    {
      return;
    }
    if(drawVisualModel_)
    {
      for(const auto & d : drawVisual_)
      {
        d();
      }
    }
    if(drawCollisionModel_)
    {
      for(const auto & d : drawCollision_)
      {
        d();
      }
    }
  }

private:
  Robot & self_;
  std::shared_ptr<mc_rbdyn::Robots> robots_;
  bool drawVisualModel_ = true;
  bool drawCollisionModel_ = false;
  std::vector<std::function<void()>> drawVisual_;
  std::vector<std::function<void()>> drawCollision_;
  Collection collectionVisual_;
  Collection collectionCollision_;
};

} // namespace details

Robot::Robot(Client & client, const ElementId & id) : Widget(client, id), impl_(new details::RobotImpl{*this}) {}

Robot::~Robot() = default;

void Robot::data(const std::vector<std::string> & params,
                 const std::vector<std::vector<double>> & q,
                 const sva::PTransformd & posW)
{
  impl_->data(params, q, posW);
}

void Robot::draw2D()
{
  impl_->draw2D();
}

void Robot::draw3D()
{
  impl_->draw3D();
}

} // namespace mc_rtc::blender
