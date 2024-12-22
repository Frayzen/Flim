#include "api/flim_api.hh"
#include "api/parameters.hh"
#include "api/render/mesh_utils.hh"
#include "api/scene.hh"
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>

using namespace Flim;

int main() {
  Flim::FlimAPI api = FlimAPI::init();
  Scene &scene = api.getScene();
  Renderer renderer = {
      Shader("shaders/default.vert.spv"),
      Shader("shaders/default.frag.spv"),
      /* RendererMode::RENDERER_MODE_LINE, */
  };
  scene.defaultRenderer(&renderer);

  Mesh teddy =
      MeshUtils(scene).loadFromFile("resources/single_file/teddy.obj");
  auto &obj = scene.instantiate(teddy);
  obj.transform.position = vec3(0, 0, 30);
  obj.transform.scale = vec3(0.2f, 0.2f, 0.2f);

  auto &obj2 = scene.instantiate(teddy);
  obj2.transform.position = vec3(30, 0, 30);
  obj2.transform.scale = vec3(0.2f, 0.2f, 0.2f);

  scene.mainCamera->speed = 30;
  scene.mainCamera->transform.position = vec3(0, 0, -30);
  scene.mainCamera->sensivity = 8;
  /* obj.transform.rotation = glm::identity<quat>(); */
  return api.run();
}
