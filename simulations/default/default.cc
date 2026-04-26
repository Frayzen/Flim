#include "api/bvh/bvh.hh"
#include "api/flim_api.hh"
#include "api/parameters/render_params.hh"
#include "api/render/mesh.hh"
#include "api/render/mesh_utils.hh"
#include "api/transform.hh"
#include "api/tree/instance.hh"
#include "kokkos/renderer_accesser.hh"
#include "vulkan/buffers/uniform_descriptors.hh"
#include <Eigen/src/Core/Matrix.h>
// #include <Kokkos_Core.hpp>
// #include <Kokkos_Random.hpp>
#include <cstdint>
#include <cstdio>
// #include <decl/Kokkos_Declare_OPENMP.hpp>
#include <imgui.h>
// #include <impl/Kokkos_Profiling.hpp>
#include <iostream>
// #include <setup/Kokkos_Setup_HIP.hpp>
#include <unistd.h>

using namespace Flim;

int main() {
  // Kokkos::initialize();

  FlimAPI api = FlimAPI::init();
  {
    // this creates the buffer for the triangles
    Mesh mesh =
        MeshUtils::loadFromFile("resources/single_file/teddy.obj"); // or other
    Mesh cube = MeshUtils::createCube();
    // by default, mesh has a material with fixed color (ambient, diffuse,
    // specular) later on, material can have per vertex or per triangle color
    // later on, material can have textures instead of colors
    // this will be stored as flags in a single shader (with unbound values such
    // as textures) in any way, the basic color will be given as a uniform and
    // applied (set to 0 to discard it) the possibility to UPDATE THE MATERIAL
    // is here, no buffer are actually created until next line

    Scene &scene = api.getScene();

    // can specify render params (shaders) but default ones are using previous
    // explanations
    RenderParams params = RenderParams::DefaultParams(mesh, scene.camera);
    params.useBackfaceCulling = false;

    RenderParams params2("Second", params);
    params2.fragmentShader = Shader("shaders/outlined.frag.spv");
    uint32_t outlined_obj = 0;
    params2.setUniform(2, FRAGMENT_SHADER_STAGE).attachObj(outlined_obj);
    const Renderer &rd = scene.registerMesh(mesh, params);
    scene.registerMesh(cube, params2);

    Instance &instance = scene.instantiate(mesh);
    instance.transform.scale = Vector3f(0.05, 0.05, 0.05);

    Instance &c = scene.instantiate(cube);

    scene.camera.controls = true;
    scene.camera.speed = 20;
    scene.camera.sensivity = 5;

    api.setupGraphics();
    // Kokkos::View<Vertex *> vertices =
    //     getAttributeBufferView<Vertex>(rd,
    //     BINDING_DEFAULT_VERTICES_ATTRIBUTES);
    // Kokkos::View<Vector3f *> originals("Original vertices",
    // vertices.extent(0)); Kokkos::View<Vector3f *> dir("Directions",
    // vertices.extent(0)); Kokkos::Random_XorShift64_Pool<>
    // random_pool(42424242); Kokkos::parallel_for(
    //     "Init", vertices.extent(0), KOKKOS_LAMBDA(const int i) {
    //       originals(i) = vertices(i).pos;
    //       auto generator = random_pool.get_state();
    //       dir(i) =
    //           Vector3f(generator.drand(-1.0, 1.0),
    //           generator.drand(-1.0, 1.0),
    //                    generator.drand(-1.0, 1.0))
    //               .normalized();
    //       vertices(i).pos =
    //           vertices(i).pos + (dir(i) * generator.drand(-0.3, 0.3));
    //       random_pool.free_state(generator);
    //     });
    // Kokkos::fence("Wait for init");

    BVH<5> bvh(cube);

    // main loop
    static float speed = 0.5;
    static float radius = 5;
    static Vector3f pointing(0, 0, 0);
    static float maxDistMove = 0.5;
    static float dist = 100;
    api.run([&](float deltaTime) {
      float curMaxDist = maxDistMove;
      // Kokkos::parallel_for(
      //     "Move vertices", vertices.extent(0), KOKKOS_LAMBDA(const int i) {
      //       vertices(i).pos += dir(i) * deltaTime;
      //       if ((vertices(i).pos - originals(i)).norm() > curMaxDist) {
      //         vertices(i).pos = originals(i) + curMaxDist * dir(i);
      //         dir(i) *= -1;
      //       }
      //     });
      ImGui::SliderFloat("Speed", &speed, 0, 1);
      ImGui::SliderFloat("Radius", &radius, 0.5, 10);
      ImGui::InputFloat3("Coord pointing", &pointing.x());
      ImGui::SliderFloat("Max Dist Move", &maxDistMove, 0.3, 1);
      static float time = 0;
      time += deltaTime * speed;
      instance.transform.position = radius * Vector3f(cos(time), 0, sin(time));
      instance.transform.lookAt(pointing);
      auto p = instance.transform.position;
      ImGui::Text("COORD IS %f %f %f", p.x(), p.y(), p.z());

      Ray ray = {
          .origin = scene.camera.transform.position,
          .direction = scene.camera.getMouseDir(),
          .cull = false,
      };
      ImGui::SliderFloat("Dist", &dist, 0, 100);
      if (!bvh.castRay(ray, &outlined_obj))
        outlined_obj = UINT32_MAX;

      // Kokkos::fence("Wait for move");
    });
  }
  // Kokkos::finalize();
  return 0;
}
