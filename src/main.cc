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
  float pointSize = 1.0f;
  bool applyDiffuse = true;
} pointDesc;

int main() {
  Flim::FlimAPI api = FlimAPI::init();

  Mesh sphere = MeshUtils::createNodalMesh();
  Mesh cube = MeshUtils::createCube();

  RenderParams sphereParams = {
      Shader("shaders/default.vert.spv"),
      Shader("shaders/default.frag.spv"),
  };
  sphereParams.mode = RenderMode::RENDERER_MODE_POINTS,
  sphereParams.addGeneralDescriptor(0)->attach<LocationUniform>();
  sphereParams.addGeneralDescriptor(1)->attach<MaterialUniform>();
  sphereParams.addGeneralDescriptor(2)->attach<PointUniform>(pointDesc);

  RenderParams cubeParams = sphereParams;
  cubeParams.mode = RenderMode::RENDERER_MODE_LINE;
  cubeParams.useBackfaceCulling = false;

  Scene &scene = api.getScene();
  scene.registerMesh(sphere, sphereParams);
  scene.registerMesh(cube, cubeParams);

  constexpr long amount = 100;
  vec3 *velocities = new vec3[amount * amount * amount]{};
  const float offset = 5;
  float bounds = offset * (float)amount;

  for (int i = 0; i < amount; i++)
    for (int j = 0; j < amount; j++)
      for (int k = 0; k < amount; k++) {
        Instance &istc = scene.instantiate(sphere);
        istc.transform.scale = vec3(0.2f);
        istc.transform.position = vec3(i, j, k) * offset;
        istc.transform.scale = vec3(0.2f, 0.2f, 0.2f);
        velocities[i * amount * amount + j * amount + k] = glm::normalize(
            vec3(std::rand() - RAND_MAX / 2, std::rand() - RAND_MAX / 2,
                 std::rand() - RAND_MAX / 2));
      }

  Instance &cubeIstc = scene.instantiate(cube);

  /* scene.camera.is2D = true; */
  scene.camera.speed = 30;
  scene.camera.transform.position = vec3(0, 0, 0);
  scene.camera.sensivity = 8;

  float timeSpeed = 0.0f;
  int ret = api.run([&](float deltaTime) {
    ImGui::Text("%f ms (%f FPS)", deltaTime, 1.0f / deltaTime);
    /* const char *items[] = {"Triangles", "Bars", "Dots"}; */
    /* if (ImGui::Combo("Rendering type", ((int *)&(sphereParams.mode)), items,
     */
    /*                  IM_ARRAYSIZE(items))) { */
    /*   sphereParams.invalidate(); */
    /* } */
    ImGui::SliderFloat3("Ambient color", (float *)&sphere.getMaterial().ambient,
                        0.0f, 1.0f);

    ImGui::SliderFloat3("Diffuse color", (float *)&sphere.getMaterial().diffuse,
                        0.0f, 1.0f);

    if (sphereParams.mode == RenderMode::RENDERER_MODE_POINTS) {
      ImGui::SliderFloat("Point size", &pointDesc.pointSize, 0.5f, 2.0f);
      ImGui::Checkbox("Point diffuse color", &pointDesc.applyDiffuse);
    }
    ImGui::SliderFloat("Time speed", &timeSpeed, 0.0f, 100.0f);
    ImGui::SliderFloat("Bounds", &bounds, 0.0f, 20.0f);

    for (int i = 0; i < amount * amount * amount; i++) {
      vec3 &vel = velocities[i];
      vec3 &pos = sphere.instances[i].transform.position;
      pos += velocities[i] * deltaTime * timeSpeed;
      if (pos.x < 0 || pos.x > bounds) {
        vel.x *= -1.0f;
      }
      if (pos.y < 0 || pos.y > bounds) {
        vel.y *= -1.0f;
      }
      if (pos.z < 0 || pos.z > bounds) {
        vel.z *= -1.0f;
      }
      pos = glm::clamp(pos, 0.0f, bounds);
    }
    cubeIstc.transform.scale = vec3(bounds);
    cubeIstc.transform.position = vec3(bounds) / 2.0f;
    cube.updateModelViews();
    sphere.updateModelViews();
  });
  api.cleanup();
  delete[] velocities;
  return ret;
}
