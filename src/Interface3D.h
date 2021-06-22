#pragma once

#include <functional>
#include <string>
#include <vector>

#include <SpaceVecAlg/SpaceVecAlg>

#include <mc_rtc/gui/types.h>

#include "widgets/details/ControlAxis.h"

/** Virtual interface that deals with Blender */
struct Interface3D
{
  virtual ~Interface3D() = default;

  virtual std::string add_collection(const std::vector<std::string> &, const std::string &) = 0;

  virtual void hide_collection(const std::string &, bool) = 0;

  virtual void remove_collection(const std::string &) = 0;

  virtual std::string load_mesh(const std::string & collection,
                                const std::string & meshPath,
                                const std::string & meshName,
                                const std::array<double, 4> & defaultColor) = 0;

  virtual void set_mesh_position(const std::string & meshName, const sva::PTransformd & pose) = 0;

  virtual void remove_mesh(const std::string & meshName) = 0;

  virtual std::string add_interactive_marker(const std::vector<std::string> & category,
                                             const std::string & name,
                                             const mc_rtc::blender::ControlAxis & axis,
                                             const std::function<void(const sva::PTransformd &)> & callback) = 0;

  virtual void update_interactive_marker(const std::string & name, bool ro, const sva::PTransformd & pos) = 0;

  virtual void set_marker_hidden(const std::string & name, bool hidden) = 0;

  virtual void remove_interactive_marker(const std::string & name) = 0;

  virtual std::string add_arrow(const std::vector<std::string> & category, const std::string & name) = 0;

  virtual void update_arrow(const std::string & name, const Eigen::Vector3d & start, const Eigen::Vector3d & end, double shaft_diam, double head_diam, double head_len, const std::array<double, 4> & color) = 0;

  virtual void remove_arrow(const std::string & name) = 0;

  struct Arrow
  {
    Arrow(Interface3D & parent,
          const std::vector<std::string> & category,
          const std::string & name)
    : parent_(parent), name_(parent_.add_arrow(category, name))
    {
    }

    ~Arrow()
    {
      parent_.remove_arrow(name_);
    }

    void update(const Eigen::Vector3d & start, const Eigen::Vector3d & end, const mc_rtc::gui::ArrowConfig & config)
    {
      const auto & c = config.color;
      parent_.update_arrow(name_, start, end, config.shaft_diam, config.head_diam, config.head_diam, {c.r, c.g, c.b, c.a});
    }
  private:
    Interface3D & parent_;
    std::string name_;
  };
};

struct Collection
{
  Collection(Interface3D & parent, const std::vector<std::string> & category, const std::string & name)
  : parent_(parent), collection_(gui().add_collection(category, name))
  {
  }

  ~Collection()
  {
    gui().remove_collection(collection_);
  }

  void hide(bool h)
  {
    gui().hide_collection(collection_, h);
  }

  Interface3D & gui()
  {
    return parent_.get();
  }

  const std::string & collection()
  {
    return collection_;
  }

private:
  std::reference_wrapper<Interface3D> parent_;
  std::string collection_;
};

struct Mesh
{
  Mesh(Collection & collection,
       const std::string & meshPath,
       const std::string & meshName,
       const std::array<double, 4> & defaultColor)
  : collection_(collection), name_(gui().load_mesh(this->collection(), meshPath, meshName, defaultColor))
  {
  }

  ~Mesh()
  {
    gui().remove_mesh(name_);
  }

  void set_position(const sva::PTransformd & pos)
  {
    gui().set_mesh_position(name_, pos);
  }

  const std::string & collection()
  {
    return collection_.get().collection();
  }

  Interface3D & gui()
  {
    return collection_.get().gui();
  }

private:
  std::reference_wrapper<Collection> collection_;
  std::string name_;
};
