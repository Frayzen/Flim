#include "api/flim_api.hh"
#include "api/parameters.hh"
#include "api/render/mesh_utils.hh"
#include "api/scene.hh"
#include "api/tree/camera_object.hh"
#include "api/tree/instance_object.hh"
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

  static void update(const InstanceObject &istc, const CameraObject &cam,
                     LocationUniform *uni) {
    uni->model =
        istc.transform.getViewMatrix() * istc.mesh.transform.getViewMatrix();
    uni->view = cam.getViewMat();
    uni->proj = cam.getProjMat(context.swapChain.swapChainExtent.width /
                               (float)context.swapChain.swapChainExtent.height);
  }
};

struct MaterialUniform {
  alignas(16) glm::vec3 ambient;
  alignas(16) glm::vec3 diffuse;
  alignas(16) glm::vec3 specular;

  static void update(const InstanceObject &istc, const CameraObject &,
                     MaterialUniform *uni) {
    auto &mesh = istc.mesh;
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
  Scene &scene = api.getScene();
  Renderer renderer = {
      Shader("shaders/default.vert.spv"),
      Shader("shaders/default.frag.spv"),
  };

  renderer.addGeneralDescriptor(0)->attach<LocationUniform>();
  renderer.addGeneralDescriptor(1)->attach<MaterialUniform>();
  renderer.addGeneralDescriptor(2)->attach<PointUniform>(pointDesc);

  Mesh teddy = MeshUtils(scene).loadFromFile("resources/single_file/teddy.obj");
  Mesh room = MeshUtils(scene).loadFromFile("resources/viking_room/viking_room.obj");
  renderer.addImageDescriptor(3, teddy.getMaterial().texturePath);

  scene.defaultRenderer(renderer);

  auto &teddy_obj = scene.instantiate(teddy);
  teddy_obj.transform.position = vec3(0, 0, -11);
  teddy_obj.transform.scale = vec3(0.2f, 0.2f, 0.2f);

  auto &room_obj = scene.instantiate(room);
  room_obj.transform.scale = vec3(5);


  scene.mainCamera->speed = 30;
  scene.mainCamera->transform.position = vec3(0, 0, 0);
  scene.mainCamera->sensivity = 8;

  float a;
  return api.run([&] {
    const char *items[] = {"Triangles", "Bars", "Dots"};
    if (ImGui::Combo("Rendering type", ((int *)&(renderer.mode)), items,
                     IM_ARRAYSIZE(items))) {
      scene.invalidateRenderer();
    }
    ImGui::SliderFloat3("Ambient color",
                        (float *)&teddy_obj.mesh.getMaterial().ambient, 0.0f, 1.0f);
    ImGui::SliderFloat3("Diffuse color",
                        (float *)&teddy_obj.mesh.getMaterial().diffuse, 0.0f, 1.0f);
    if (renderer.mode == RendererMode::RENDERER_MODE_POINTS) {
      ImGui::SliderFloat("Point size", &pointDesc.pointSize, 0.0f, 20.0f);
      ImGui::Checkbox("Point diffuse color", &pointDesc.applyDiffuse);
    }
  });
}
