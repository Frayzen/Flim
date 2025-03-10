#include "api/flim_api.hh"
#include "api/parameters/render_params.hh"
#include "api/render/mesh.hh"
#include "api/render/mesh_utils.hh"
#include "api/transform.hh"
#include "api/tree/instance.hh"
#include <Eigen/src/Core/Matrix.h>
#include <Kokkos_Core.hpp>
#include <cstdio>
#include <imgui.h>
#include <iostream>
#include <unistd.h>

using namespace Flim;

void check(Vector3f v) {
  Eigen::Quaternionf q = Quaternionf::FromTwoVectors(world_front, v);
  static int x = 0;
  std::cout << "POS " << x++ << " IS " << v << std::endl;
  std::cout << "W:" << q.w() << " X:" << q.x() << " Y:" << q.y()
            << " Z:" << q.z() << '\n';
  std::cout << "AFTER APPLYING : " << q * world_front << '\n';
}

int main() {
  Kokkos::initialize();

  check(Vector3f(0, 0, 1));
  check(Vector3f(0, 1, 0));
  check(Vector3f(1, 0, 0));
  check(Vector3f(0, 0, -1));
  check(Vector3f(0, -1, 0));
  check(Vector3f(-1, 0, 0));

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
    RenderParams params2("Second", params);
    scene.registerMesh(mesh, params);
    scene.registerMesh(cube, params2);

    Instance &instance = scene.instantiate(mesh);
    instance.transform.scale = Vector3f(0.05, 0.05, 0.05);

    Instance &c = scene.instantiate(cube);

    scene.camera.controls = true;
    scene.camera.speed = 30;
    // main loop
    static float speed = 0.5;
    static float radius = 5;
    static Vector3f pointing(0, 0, 0);
    api.run([&](float deltaTime) {
      ImGui::SliderFloat("Speed", &speed, 0, 1);
      ImGui::SliderFloat("Radius", &radius, 0.5, 10);
      ImGui::InputFloat3("Coord pointing", &pointing.x());
      static float time = 0;
      time += deltaTime * speed;
      instance.transform.position = radius * Vector3f(cos(time), 0, sin(time));
      instance.transform.lookAt(pointing);
      auto p = instance.transform.position;
      ImGui::Text("COORD IS %f %f %f", p.x(), p.y(), p.z());
    });
  }
  Kokkos::finalize();
  return 0;
}
