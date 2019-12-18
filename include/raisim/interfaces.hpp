//
// Created by jhwangbo on 21.01.19.
//

#ifndef RAISIM_OGRE_INTERFACES_HPP
#define RAISIM_OGRE_INTERFACES_HPP

// OGRE
#include <Ogre.h>

// RaiSim
#include "raisim/World.hpp"

namespace raisim {

/** graphic object represents the underlying object **/
class GraphicObject {
 public:
  GraphicObject() {
    scale.setConstant(1.0);
    offset.setZero();
    rotationOffset.setIdentity();
  }
  
public:
  std::string name;
  std::string meshName;
  raisim::Vec<3> scale;
  raisim::Vec<3> offset;
  raisim::Mat<3, 3> rotationOffset;
  Ogre::SceneNode* graphics = nullptr;
  unsigned long int group{1ul};
  size_t localId{0};
  bool selectable_{true};
};


/** Visual object is for visualization only**/
class VisualObject {
 public:
  VisualObject(Ogre::SceneNode* grp) : graphics(grp) {
    scale.setConstant(1.0);
    offset.setZero();
    rotationOffset.setIdentity();
  }

  VisualObject() {
    scale.setConstant(1.0);
    offset.setZero();
    rotationOffset.setIdentity();
  }
  
  void setScale(double x, double y, double z) {
    scale = {x, y, z};
  }
  
  void setScale(const Eigen::Vector3d &s) {
    scale.e() = s;
  }
  
  void setScale(const raisim::Vec<3> &s) {
    scale = s;
  }
  
  void setPosition(double x, double y, double z) {
    offset = {x, y, z};
  }
  
  void setPosition(const Eigen::Vector3d &r) {
    offset.e() = r;
  }
  
  void setPosition(const raisim::Vec<3> &r) {
    offset = r;
  }
  
  void setOrientation(const Eigen::Matrix3d &R) {
    rotationOffset.e() = R;
  }
  
  void setOrientation(const raisim::Mat<3, 3> &R) {
    rotationOffset = R;
  }
  
public:
  std::string name;
  raisim::Vec<3> scale;
  raisim::Vec<3> offset;
  raisim::Mat<3, 3> rotationOffset;
  Ogre::SceneNode* graphics{nullptr};
  unsigned long int group{1ul};
};

class SimAndGraphicsObjectPool {
 public:
  void setWorld(World* world) {
    world_ = world;
  }

  void insert(const std::string& name, Object* object, std::vector<GraphicObject>&& graphics) {
    set[object] = graphics;
    ref[name] = object;
  }

  void erase(const std::string& name) {
    world_->removeObject(ref[name]);
    set.erase(ref[name]);
    ref.erase(name);
  }

  void erase(Object* obj) {
    set.erase(obj);
    auto it = std::find_if(ref.begin(), ref.end(),
                           [obj](const std::pair<std::string, Object*>& ref){ return ref.second == obj; });
    ref.erase(it->first);
  }

  /** update pose of graphical objects **/
  void sync() {
    Vec<3> pos, offsetInWorld;
    Vec<4> quat;
    Mat<3, 3> bodyRotation, rot;

    for (auto &pair : set) {
      for (auto &grp : pair.second) {
        auto raisimObject = pair.first;
        raisimObject->getPosition(grp.localId, pos);
        raisimObject->getOrientation(grp.localId, bodyRotation);
        matvecmul(bodyRotation, grp.offset, offsetInWorld);
        matmul(bodyRotation, grp.rotationOffset, rot);
        grp.graphics->setPosition(float(pos[0] + offsetInWorld[0]),
                                  float(pos[1] + offsetInWorld[1]),
                                  float(pos[2] + offsetInWorld[2]));
        raisim::rotMatToQuat(rot, quat);
        grp.graphics->setOrientation(float(quat[0]), float(quat[1]), float(quat[2]), float(quat[3]));
//        grp.graphics->setScale({float(grp.scale[0]), float(grp.scale[1]), float(grp.scale[2])});
      }
    }
  }

  /** return graphical objects and its existence **/
  std::pair<std::vector<raisim::GraphicObject>*, bool> operator [](Object* ob) { return {&set[ob], set.find(ob) != set.end()}; }

  /** return raisim objects and its existence **/
  std::pair<Object*, bool> operator [](const std::string& name) { return {ref[name], ref.find(name) != ref.end()}; }

  /** return raisim objects and its local index. The first element is nullptr if not found **/
  std::pair<Object*, size_t> operator [](Ogre::SceneNode* node) {
    for(auto& ele : set) {
      auto found = std::find_if(ele.second.begin(), ele.second.end(),
                   [node](const raisim::GraphicObject& ref) { return ref.graphics == node; });
      if(found != ele.second.end())
        return {ele.first, found->localId};
    }
    return {nullptr, 0};
  }

  /** return graphic object and its existence**/
  raisim::GraphicObject* getGraphicObject(Ogre::SceneNode* node) {
    for(auto& ele : set) {
      auto found = std::find_if(ele.second.begin(), ele.second.end(),
                                [node](const raisim::GraphicObject& ref) { return ref.graphics == node; });
      if(found != ele.second.end())
        return &(*found);
    }
    return nullptr;
  }

  std::map<Object*, std::vector<raisim::GraphicObject>> set;
  std::map<std::string, Object*> ref;

 private:
  World* world_;
};

} // namespace raisim

#endif // RAISIM_OGRE_INTERFACES_HPP
