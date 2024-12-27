#include "api/flim_api.hh"
#include "api/parameters.hh"
#include "api/render/mesh.hh"
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

  static void update(const Mesh &mesh, const CameraObject &cam,
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

  static void update(const Mesh &mesh, const CameraObject &,
                     MaterialUniform *uni) {
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
  Mesh room = MeshUtils::loadFromFile("resources/viking_room/viking_room.obj");

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

  InstanceObject &teddy_obj = scene.instantiate(teddy);
  teddy_obj.transform.scale = vec3(0.2f);
  InstanceObject &teddy_obj2 = scene.instantiate(teddy);
  teddy_obj2.transform.scale = vec3(0.2f);
  teddy_obj2.transform.position = vec3(0, 0, -11);
  teddy_obj2.transform.scale = vec3(0.2f, 0.2f, 0.2f);

  /* InstanceObject &room_obj = scene.instantiate(room); */
  /* room_obj.transform.scale = vec3(10); */

  scene.mainCamera->speed = 30;
  scene.mainCamera->transform.position = vec3(0, 0, 0);
  scene.mainCamera->sensivity = 8;

  float a;
  int ret = api.run([&] {
    const char *items[] = {"Triangles", "Bars", "Dots"};
    if (ImGui::Combo("Rendering type", ((int *)&(renderParams.mode)), items,
                     IM_ARRAYSIZE(items))) {
      renderParams.invalidate();
    }
    ImGui::SliderFloat3("Ambient color",
                        (float *)&teddy_obj.mesh.getMaterial().ambient, 0.0f,
                        1.0f);

    ImGui::SliderFloat3("Diffuse color",
                        (float *)&teddy_obj.mesh.getMaterial().diffuse, 0.0f,
                        1.0f);

    float size;
    /* if (ImGui::SliderFloat("Taille du nounours", (float *)&size, 0.1f, 10.0f)) { */
    /*   teddy.transform.scale = vec3(size); */
    /* } */
    /* if (ImGui::SliderFloat("Taille de la room", (float *)&size, 0.1f, 100.0f)) { */
    /*   room.transform.scale = vec3(size); */
    /* } */
    if (renderParams.mode == RendererMode::RENDERER_MODE_POINTS) {
      ImGui::SliderFloat("Point size", &pointDesc.pointSize, 0.0f, 20.0f);
      ImGui::Checkbox("Point diffuse color", &pointDesc.applyDiffuse);
    }
  });
  api.cleanup();
  return ret;
}
