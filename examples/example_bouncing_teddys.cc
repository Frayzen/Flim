#include "api/flim_api.hh"
#include "api/parameters.hh"
#include "api/render/mesh.hh"
#include "api/render/mesh_utils.hh"
#include "api/scene.hh"
#include "api/tree/camera.hh"
#include "api/tree/instance.hh"
#include <cstdlib>
#include <glm/common.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/gtx/string_cast.hpp>
#include <imgui.h>
#include <imgui_internal.h>

using namespace Flim;

struct LocationUniform {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;

  static void update(const Mesh &mesh, const Camera &cam,
                     LocationUniform *uni) {
    uni->model = mesh.transform.getViewMatrix();
    uni->view = cam.getViewMat();
    uni->proj = cam.getProjMat(context.swapChain.swapChainExtent.width /
                               (float)context.swapChain.swapChainExtent.height);
  }
};

struct MaterialUniform {
  alignas(16) glm::vec3 ambient;
  alignas(16) glm::vec3 diffuse;
  alignas(16) glm::vec3 specular;

  static void update(const Mesh &mesh, const Camera &, MaterialUniform *uni) {
    uni->ambient = mesh.getMaterial().ambient;
    uni->diffuse = mesh.getMaterial().diffuse;
    uni->specular = mesh.getMaterial().specular;
  }
};

struct PointUniform {
  float pointSize = 5.0f;
  bool applyDiffuse = true;
} pointDesc;

int main() {
  Flim::FlimAPI api = FlimAPI::init();

  Mesh teddy = MeshUtils::loadFromFile("resources/single_file/teddy.obj");
  /* Mesh room = MeshUtils::loadFromFile("resources/viking_room/viking_room.obj"); */

  RenderParams renderParams = {
      Shader("shaders/default.vert.spv"),
      Shader("shaders/default.frag.spv"),
  };
  renderParams.addGeneralDescriptor(0)->attach<LocationUniform>();
  renderParams.addGeneralDescriptor(1)->attach<MaterialUniform>();
  renderParams.addGeneralDescriptor(2)->attach<PointUniform>(pointDesc);
  renderParams.addImageDescriptor(3, teddy.getMaterial().texturePath);

  Scene &scene = api.getScene();
  scene.registerMesh(teddy, renderParams);
  /* scene.registerMesh(room, renderParams); */

  constexpr int amount = 5;
  vec3 velocities[amount * amount * amount] = {};
  const float offset = 10;
  float bounds = offset * (float)amount;

  for (int i = 0; i < amount; i++)
    for (int j = 0; j < amount; j++)
      for (int k = 0; k < amount; k++) {
        Instance &teddy_obj = scene.instantiate(teddy);
        teddy_obj.transform.scale = vec3(0.2f);
        teddy_obj.transform.position = vec3(i, j, k) * offset;
        teddy_obj.transform.scale = vec3(0.2f, 0.2f, 0.2f);
        velocities[i * amount * amount + j * amount + k] = glm::normalize(
            vec3(std::rand() - RAND_MAX / 2, std::rand() - RAND_MAX / 2,
                 std::rand() - RAND_MAX / 2));
      }

  scene.camera.speed = 30;
  scene.camera.transform.position = vec3(0, 0, 0);
  scene.camera.sensivity = 8;

  float timeSpeed = 0.0f;
  int ret = api.run([&](float deltaTime) {
    ImGui::Text("%i ms (%f FPS)", (int)deltaTime * 1000, 1.0f / deltaTime);
    const char *items[] = {"Triangles", "Bars", "Dots"};
    if (ImGui::Combo("Rendering type", ((int *)&(renderParams.mode)), items,
                     IM_ARRAYSIZE(items))) {
      renderParams.invalidate();
    }
    ImGui::SliderFloat3("Ambient color", (float *)&teddy.getMaterial().ambient,
                        0.0f, 1.0f);

    ImGui::SliderFloat3("Diffuse color", (float *)&teddy.getMaterial().diffuse,
                        0.0f, 1.0f);

    if (renderParams.mode == RendererMode::RENDERER_MODE_POINTS) {
      ImGui::SliderFloat("Point size", &pointDesc.pointSize, 0.0f, 20.0f);
      ImGui::Checkbox("Point diffuse color", &pointDesc.applyDiffuse);
    }
    ImGui::SliderFloat("Time speed", &timeSpeed, 0.0f, 100.0f);

    for (int i = 0; i < amount * amount * amount; i++) {
      vec3 &vel = velocities[i];
      vec3 &pos = teddy.instances[i].transform.position;
      pos += velocities[i] * deltaTime * timeSpeed;
      if (pos.x < 0 || pos.x > bounds) {
        vel.x *= -1;
      }
      if (pos.y < 0 || pos.y > bounds) {
        vel.y *= -1;
      }
      if (pos.z < 0 || pos.z > bounds) {
        vel.z *= -1;
      }
    }

    teddy.updateModelViews();
  });
  api.cleanup();
  return ret;
}
