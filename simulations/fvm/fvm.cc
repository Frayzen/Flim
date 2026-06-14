#include "api/flim_api.hh"
#include "api/parameters/base_params.hh"
#include "api/parameters/render_params.hh"
#include "api/mesh/mesh.hh"
#include "api/mesh/mesh_utils.hh"
#include "api/tree/instance.hh"
#include "ast/fmv_solver.hh"
#include "ast/fvm_ast.hh"
#include "ast/fvm_mesh.hh"
#include "vulkan/buffers/params_utils.hh"
#include "vulkan/rendering/renderer.hh"

using namespace Flim;

// #include <KokkosBlas1_axpby.hpp>
// #include <KokkosBlas1_dot.hpp>
// #include <KokkosSparse_CrsMatrix.hpp>
// #include <KokkosSparse_spmv.hpp>

int main(int argc, char *argv[]) {
  Kokkos::initialize(argc, argv);
  FlimAPI api = FlimAPI::init();
  {

    const int nb_x = 50;
    const int nb_y = 50;
    const float L_x = 1.0f;
    const float L_y = 1.0f;

    Mesh mesh = MeshUtils::createGrid(L_x, nb_x, nb_y);

    FVMAst ast(2, 1);
    auto u = ast.getImplicitUnknown();
    auto ue = ast.getExplicitUnknown();

    auto dt = 0.0001f;
    auto heat_equation = (u - ue) / dt + div(grad(u));
    heat_equation.print();

    FVMMesh fvmMesh(mesh);
    FVMSolver solver(ast, fvmMesh);
    std::cout << "HERE " << std::endl;
    solver.assemble();

    auto &scene = api.getScene();
    RenderParams params("Default");

    ParamsUtils::createViewMatrixUniform(params, BINDING_DEFAULT_VIEWS_UNIFORM,
                                         mesh, scene.camera);
    ParamsUtils::createInstanceMatrixAttribute(params, 2);
    ParamsUtils::createVerticesAttribute(params, 0, true, false, false);

    params.vertexShader = Shader("shaders/fvm.vert.spv");
    params.fragmentShader = Shader("shaders/fvm.frag.spv");
    params.setAttribute(1, AttributeRate::VERTEX)
        .attach<Vector4f>([](const Mesh &m, Vector4f *colors) {
          for (uint i = 0; i < m.getVertices().size(); i++)
            colors[i] = Vector4f(1, 1, 1, 1);
        })
        .add(0, VK_FORMAT_R32G32B32A32_SFLOAT)
        .computeFriendly(true)
        .singleBuffered(true)
        .onlySetup(true);

    params.mode = RenderMode::RENDERER_MODE_TRIS;
    params.useBackfaceCulling = false;
    const Renderer &rd = scene.registerMesh(mesh, params);
    scene.instantiate(mesh);

    api.setupGraphics();

    scene.camera.controls = true;
    scene.camera.speed = 5;
    scene.camera.sensivity = 5;
    scene.camera.transform.position = Vector3f(0, nb_y, (nb_x + nb_y) / 2.0f);

    // auto colors = getAttributeBufferView<Vector4f>(rd, 1);

    bool running = false;
    float cold[3] = {0.1f, 0.1f, 1.0f};
    float hot[3] = {1.0f, 0.1f, 0.1f};
    static float max = 1.0f;
    static float min = 1.0f;

    api.run([&](float deltaTime) {
      static float sumtime = 0.0f;
      sumtime += deltaTime;

      if (ImGui::Button("Step") || running) {
        // --- ASSEMBLE MATRIX AND RHS ---
        float curtime = sumtime;
        typedef Kokkos::MinMax<float>::value_type MinMax;
        MinMax minmax;
        union test {
          int a;
          float b;
        };
        Kokkos::View<test *> a("example", 10);
        Kokkos::parallel_for(
            "MinMaxReduce", 10, KOKKOS_LAMBDA(uint32_t i) { a(i).a = 3; });
        Kokkos::fence();
        auto t = Kokkos::create_mirror_view_and_copy(Kokkos::HostSpace{}, a);
        std::cout << t(0).a << std::endl;
        Kokkos::parallel_for(
            "MinMaxReduce", 10, KOKKOS_LAMBDA(uint32_t i) { a(i).b = 3.8f; });
        Kokkos::fence();
        t = Kokkos::create_mirror_view_and_copy(Kokkos::HostSpace{}, a);
        std::cout << t(0).b << std::endl;
      }

      // --- VISUALIZATION ---
      ImGui::Checkbox("Running", &running);
      ImGui::ColorEdit3("Cold", cold);
      ImGui::ColorEdit3("Hot", hot);

      ImGui::Text("Delta time : %f", dt);
      ImGui::Text("MAX VAL: %f", max);
      ImGui::Text("MIN VAL: %f", min);
    });
  }
  Kokkos::finalize();
  return 0;
}
