#include "api/flim_api.hh"
#include "api/parameters.hh"
#include "api/render/mesh_utils.hh"
#include "api/scene.hh"

using namespace Flim;

int main() {
  Flim::FlimAPI api = FlimAPI::init();
  Scene &scene = api.getScene();
  Renderer renderer = {
      Shader("shaders/default.vert.spv"),
      Shader("shaders/default.frag.spv"),
  };
  scene.defaultRenderer(&renderer);
  Mesh teddy = MeshUtils(scene).loadFromFile("resources/single_file/teddy.obj");
  auto& obj = scene.instantiate(teddy);
  obj.transform.position = vec3(6, 0, 0);
  obj.transform.scale = vec3(0.1f);
  return api.run();
}
