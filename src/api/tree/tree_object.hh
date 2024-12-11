#pragma once

#include "api/fwd.hh"
#include "api/transform.hh"
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <vector>
#include <type_traits>
#include <utility>

namespace Flim {
class TreeObject {
public:
  Scene &scene;
  Transform transform;

  template <typename T> T *findAny() const {
    for (auto &c : children) {
      if (auto cast = std::dynamic_pointer_cast<T>(c))
        return cast.get();
    }
    return nullptr;
  }

  template <typename T, typename... Args>
    requires std::is_base_of_v<TreeObject, T>
  T &append(Args &&...args) {
    auto shared = std::make_shared<T>(this, std::forward(args)...);
    children.push_back(shared);
    return *shared.get();
  }

  template <typename T>
    requires std::is_base_of_v<TreeObject, T>
  T &append(T *obj) {
    children.push_back(std::make_shared(obj));
    return *obj;
  }

  template <typename T>
    requires std::is_base_of_v<TreeObject, T>
  T &append(T &obj) {
    children.push_back(std::make_shared(obj));
    return obj;
  }

  size_t nbChildren() const { return children.size(); }

  virtual ~TreeObject() = default;

protected:
  TreeObject(TreeObject *parent) : parent(parent), scene(parent->scene) {};

  TreeObject(Scene &scene) : parent(nullptr), scene(scene) {};
  TreeObject *parent;
  std::vector<std::shared_ptr<TreeObject>> children;

  friend class InstanceObject;
};


} // namespace Flim
