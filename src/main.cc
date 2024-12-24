#include "api/flim_api.hh"
#include "api/parameters.hh"
#include "api/render/mesh_utils.hh"
#include "api/scene.hh"
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <imgui.h>
#include <imgui_internal.h>

using namespace Flim;

int main() {
  Flim::FlimAPI api = FlimAPI::init();
  Scene &scene = api.getScene();
  Renderer renderer = {
      Shader("shaders/default.vert.spv"), Shader("shaders/default.frag.spv"),
  };
  scene.defaultRenderer(&renderer);

  Mesh teddy = MeshUtils(scene).loadFromFile("resources/single_file/teddy.obj");
  auto &obj = scene.instantiate(teddy);
  obj.transform.position = vec3(0, 0, 30);
  obj.transform.scale = vec3(0.2f, 0.2f, 0.2f);

  scene.mainCamera->speed = 30;
  scene.mainCamera->transform.position = vec3(0, 0, -30);
  scene.mainCamera->sensivity = 8;
  /* obj.transform.rotation = glm::identity<quat>(); */
  float a;
  return api.run([&] {
    const char *items[] = {"Triangles", "Bars", "Dots"};
    if (ImGui::Combo("Rendering type", ((int *)&renderer.mode), items,
                     IM_ARRAYSIZE(items))) {
      scene.invalidateRenderer();
    }
    ImGui::SliderFloat3("Ambient color", (float*) &obj.mesh.getMaterial().ambient, 0.0f, 1.0f);
    ImGui::SliderFloat3("Diffuse color", (float*) &obj.mesh.getMaterial().diffuse, 0.0f, 1.0f);
  });
}
