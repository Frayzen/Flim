#pragma once

#include "api/fwd.hh"
#include "api/tree/tree_object.hh"
namespace Flim {

class RootObject : public TreeObject {
public:
  RootObject(Scene& scene) : TreeObject(nullptr, scene) {};
};

} // namespace Flim
