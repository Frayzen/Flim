#include <Kokkos_Core.hpp>

#include <Eigen/Eigen>
#include <Eigen/StdVector>
#include <iostream>

#include "api/flim_api.hh"
#include "api/parameters.hh"
#include "api/render/mesh.hh"
#include "api/render/mesh_utils.hh"
#include "api/scene.hh"
#include "api/tree/camera.hh"
#include "api/tree/instance.hh"
#include <Kokkos_Core_fwd.hpp>
#include <cstdlib>
#include <decl/Kokkos_Declare_OPENMP.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <impl/Kokkos_Profiling.hpp>
#include <setup/Kokkos_Setup_HIP.hpp>

using namespace Flim;

struct LocationUniform {
  Matrix4f model;
  Matrix4f view;
  Matrix4f proj;

  static void update(const Mesh &mesh, const Camera &cam,
                     LocationUniform *uni) {
    uni->model = mesh.transform.getViewMatrix();
    uni->view = cam.getViewMat();
    uni->proj = cam.getProjMat(context.swapChain.swapChainExtent.width /
                               (float)context.swapChain.swapChainExtent.height);
  }
};

struct MaterialUniform {
  alignas(16) Vector3f ambient;
  alignas(16) Vector3f diffuse;
  alignas(16) Vector3f specular;

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
  Kokkos::initialize();
  Kokkos::print_configuration(std::cout);
  Flim::FlimAPI api = FlimAPI::init();

  Mesh sphere = MeshUtils::createNodalMesh();
  Mesh cube = MeshUtils::createCube();

  RenderParams sphereParams = {
      Shader("shaders/default.vert.spv"),
      Shader("shaders/default.frag.spv"),
  };
  sphereParams.mode = RenderMode::RENDERER_MODE_POINTS;
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

  std::vector<Vector3f, Eigen::aligned_allocator<Eigen::Vector3f>> velocities(
      amount * amount * amount);
  const float offset = 5;
  float bounds = offset * (float)amount / 2.0f;
  const float originalBounds = bounds;

  for (int i = 0; i < amount; i++)
    for (int j = 0; j < amount; j++)
      for (int k = 0; k < amount; k++) {
        Instance &istc = scene.instantiate(sphere);
        istc.transform.scale = Vector3f(0.2f, 0.2f, 0.2f);
        auto pos = Vector3f(i, j, k);
        istc.transform.position = pos * offset - Vector3f(bounds, bounds, bounds);
        auto vel =
            Vector3f(std::rand() - RAND_MAX / 2, std::rand() - RAND_MAX / 2,
                     std::rand() - RAND_MAX / 2);
        velocities[i * amount * amount + j * amount + k] = vel.normalized();
      }

  Instance &cubeIstc = scene.instantiate(cube);

  /* scene.camera.is2D = true; */
  scene.camera.speed = 30;
  scene.camera.transform.position = Vector3f(0, 0, 10);
  scene.camera.sensivity = 8;

  float timeSpeed = 0.0f;
  int ret = api.run([&](float deltaTime) {
    ImGui::Text("%f ms (%f FPS)", deltaTime, 1.0f / deltaTime);
    const char *items[] = {"Triangles", "Bars", "Dots"};
    if (ImGui::Combo("Rendering type", ((int *)&(sphereParams.mode)), items,

                     IM_ARRAYSIZE(items))) {
      sphereParams.invalidate();
    }
    ImGui::SliderFloat3("Ambient color", (float *)&sphere.getMaterial().ambient,
                        0.0f, 1.0f);

    ImGui::SliderFloat3("Diffuse color", (float *)&sphere.getMaterial().diffuse,
                        0.0f, 1.0f);

    if (sphereParams.mode == RenderMode::RENDERER_MODE_POINTS) {
      ImGui::SliderFloat("Point size", &pointDesc.pointSize, 0.5f, 5.0f);
      ImGui::Checkbox("Point diffuse color", &pointDesc.applyDiffuse);
    }
    ImGui::SliderFloat("Time speed", &timeSpeed, 0.0f, 100.0f);
    ImGui::SliderFloat("Bounds", &bounds, 0.1f, 2.0f * originalBounds);

    // Kokkos

    Kokkos::DefaultHostExecutionSpace hostExecSpace;
    Kokkos::DefaultExecutionSpace execSpace;

    Kokkos::View<Matrix4f *, Kokkos::DefaultHostExecutionSpace,
                 Kokkos::MemoryTraits<Kokkos::Unmanaged>>
        matView(sphere.modelViews.data(), sphere.modelViews.size());

    auto matViewDevice =
        Kokkos::create_mirror_view_and_copy(execSpace, matView);

    Kokkos::View<Vector3f *, Kokkos::DefaultHostExecutionSpace,
                 Kokkos::MemoryTraits<Kokkos::Unmanaged>>
        velView(velocities.data(), velocities.size());
    auto velViewDevice =
        Kokkos::create_mirror_view_and_copy(execSpace, velView);

    Kokkos::parallel_for(
        "update pos",
        Kokkos::RangePolicy<Kokkos::DefaultExecutionSpace>(
            0, sphere.modelViews.size()),
        KOKKOS_LAMBDA(const int i) {
          Vector3f pos = matViewDevice(i).rightCols<1>().head<3>();
          Vector3f vel = velViewDevice(i);
          pos += vel * deltaTime * timeSpeed;
          if (pos.x() >= bounds || pos.x() <= -bounds) {
            vel.x() *= -1.0f;
          }
          if (pos.y() >= bounds || pos.y() <= -bounds) {
            vel.y() *= -1.0f;
          }
          if (pos.z() >= bounds || pos.z() <= -bounds) {
            vel.z() *= -1.0f;
          }
          pos.x() = Kokkos::clamp(pos.x(), -bounds, bounds);
          pos.y() = Kokkos::clamp(pos.y(), -bounds, bounds);
          pos.z() = Kokkos::clamp(pos.z(), -bounds, bounds);

          velViewDevice(i) = vel;
          matViewDevice(i).rightCols<1>().head<3>() = pos;

          // Set the bottom-left corner (row 3, column 0) to 1.0f
          matViewDevice(i)(3, 0) = 1.0f;
        });
    Kokkos::fence();
    Kokkos::deep_copy(matView, matViewDevice);
    Kokkos::deep_copy(velView, velViewDevice);
    Kokkos::fence();

    cubeIstc.transform.scale = 2.0f * Vector3f(bounds, bounds, bounds);
    cube.updateModelViews();
  });
  api.cleanup();
  Kokkos::finalize();
  return ret;
}
