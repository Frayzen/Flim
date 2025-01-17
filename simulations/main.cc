
#include <Eigen/Eigen>
#include <Eigen/StdVector>

#include "api/flim_api.hh"
#include "api/render/mesh.hh"
#include "api/render/mesh_utils.hh"
#include "api/scene.hh"
#include "api/tree/camera.hh"
#include "api/tree/instance.hh"
#include "vulkan/buffers/attribute_descriptors.hh"
#include "vulkan/buffers/uniform_descriptors.hh"
#include <Eigen/src/Core/Matrix.h>
#include <cstdlib>
#include <cstring>
#include <imgui.h>
#include <imgui_internal.h>
#include <iostream>
#include <vulkan/vulkan_core.h>

using namespace Flim;

const float offset = 20;
constexpr long xyzLayout = 8;
const long amount = 2;
const long perAxis = amount * xyzLayout;
const Vector3i nbPerAxis(perAxis, perAxis, perAxis);

const float originalBounds = 2 * offset * amount;

struct LocationUniform {
  Matrix4f model;
  Matrix4f view;
  Matrix4f proj;
};

struct MaterialUniform {
  alignas(16) Vector3f ambient;
  alignas(16) Vector3f diffuse;
  alignas(16) Vector3f specular;
};

struct PointUniform {
  float pointSize = 10.0f;
  float edgeSize = 0.3f;
  bool applyDiffuse = true;
} pointDesc;

struct ParticleComputeParam {
  float delatTime;
  float bounds = originalBounds;
} cmpParam;

int main() {
  Flim::FlimAPI api = FlimAPI::init();

  /* Mesh particle =
   * MeshUtils::loadFromFile("./resources/single_file/teddy.obj"); */
  Mesh particle = MeshUtils::createNodalMesh();
  Mesh cube = MeshUtils::createCube();

  Scene &scene = api.getScene();
  auto &cam = scene.camera;

  RenderParams particlesParams("Particles");
  particlesParams.vertexShader = Shader("shaders/default.vert.spv"),
  particlesParams.fragmentShader = Shader("shaders/default.frag.spv");

  particlesParams.mode = RenderMode::RENDERER_MODE_POINTS;
  particlesParams.setUniform(0, VERTEX_SHADER_STAGE)
      .attach<LocationUniform>([&](LocationUniform *uni) {
        uni->model = particle.transform.getViewMatrix();
        uni->view = cam.getViewMat();
        uni->proj =
            cam.getProjMat(context.swapChain.swapChainExtent.width /
                           (float)context.swapChain.swapChainExtent.height);
      });
  particlesParams.setUniform(1, FRAGMENT_SHADER_STAGE)
      .attach<MaterialUniform>([&](MaterialUniform *uni) {
        uni->ambient = particle.getMaterial().ambient;
        uni->diffuse = particle.getMaterial().diffuse;
        uni->specular = particle.getMaterial().specular;
      });
  particlesParams.setUniform(2).attachObj<PointUniform>(pointDesc);

  particlesParams.setAttribute(0)
      .attach<Flim::Vertex>([](const Mesh &m, Flim::Vertex *vertices) {
        memcpy(vertices, m.vertices.data(),
               m.vertices.size() * sizeof(Flim::Vertex));
      })
      .onlySetup(true)
      .singleBuffered(true)
      .add(offsetof(Flim::Vertex, pos), VK_FORMAT_R32G32B32_SFLOAT)
      .add(offsetof(Flim::Vertex, normal), VK_FORMAT_R32G32B32_SFLOAT)
      .add(offsetof(Flim::Vertex, uv), VK_FORMAT_R32G32_SFLOAT);

  auto &positions =
      particlesParams.setAttribute(3, AttributeRate::INSTANCE)
          .attach<Matrix4f>([](const Mesh &m, Matrix4f *mats) {
            for (size_t i = 0; i < m.instances.size(); i++) {
              mats[i] = m.instances[i].transform.getViewMatrix();
            }
          })
          .onlySetup(true)
          .computeFriendly(true)
          .add(0 * sizeof(Vector4f), VK_FORMAT_R32G32B32A32_SFLOAT)
          .add(1 * sizeof(Vector4f), VK_FORMAT_R32G32B32A32_SFLOAT)
          .add(2 * sizeof(Vector4f), VK_FORMAT_R32G32B32A32_SFLOAT)
          .add(3 * sizeof(Vector4f), VK_FORMAT_R32G32B32A32_SFLOAT);
  auto &velocities = particlesParams.setAttribute(8, AttributeRate::INSTANCE)
                         .attach<Vector4f>([](const Mesh &m, Vector4f *vels) {
                           for (size_t i = 0; i < m.instances.size(); i++)
                             vels[i] =
                                 Vector3f::Random().normalized().homogeneous();
                         })
                         .add(0, VK_FORMAT_R32G32B32A32_SFLOAT)
                         .computeFriendly(true)
                         .singleBuffered(true)
                         .onlySetup(true);

  particlesParams.setAttribute(7, AttributeRate::INSTANCE)
      .attach<Vector4f>([](const Mesh &m, Vector4f *ptrs) {
        for (size_t i = 0; i < m.instances.size(); i++) {
          if (i % 2 == 0)
            ptrs[i] = Vector4f(1, 0, 0, 1);
          else
            ptrs[i] = Vector4f(0, 0, 1, 1);
        }
      })
      .add(0, VK_FORMAT_R32G32B32A32_SFLOAT)
      .onlySetup(true)
      .singleBuffered(true);

  RenderParams cubeParams("Cube", particlesParams);
  cubeParams.updateAttribute(3).onlySetup(false).computeFriendly(false);

  cubeParams.setUniform(1, FRAGMENT_SHADER_STAGE)
      .attach<MaterialUniform>([&](MaterialUniform *uni) {
        uni->ambient = cube.getMaterial().ambient;
        uni->diffuse = cube.getMaterial().diffuse;
        uni->specular = cube.getMaterial().specular;
      });
  cubeParams.mode = RenderMode::RENDERER_MODE_LINE;
  cubeParams.useBackfaceCulling = false;

  ComputeParams particlesCompute("Particle");
  particlesCompute.shader = Shader("shaders/default.comp.spv");
  particlesCompute.setAttribute(positions, 2).previousFrame(true);
  particlesCompute.setAttribute(positions, 1);
  particlesCompute.setAttribute(velocities, 0);
  particlesCompute.setUniform(3, VK_SHADER_STAGE_COMPUTE_BIT)
      .attachObj(cmpParam);

  scene.registerMesh(particle, particlesParams);
  scene.registerComputer(particlesCompute, perAxis, perAxis, perAxis);
  scene.registerMesh(cube, cubeParams);

  for (int i = 0; i < nbPerAxis.x(); i++)
    for (int j = 0; j < nbPerAxis.y(); j++)
      for (int k = 0; k < nbPerAxis.z(); k++) {
        Instance &istc = scene.instantiate(particle);
        istc.transform.scale = Vector3f(0.2f, 0.2f, 0.2f);
        auto pos = Vector3f(i, j, k);
        istc.transform.position =
            pos * offset -
            Vector3f(cmpParam.bounds, cmpParam.bounds, cmpParam.bounds);
      }

  Instance &cubeIstc = scene.instantiate(cube);

  /* scene.camera.is2D = true; */
  scene.camera.speed = 100;
  scene.camera.transform.position = Vector3f(0, 0, 10);
  scene.camera.sensivity = 5;

  float timeSpeed = 0.0f;
  int ret = api.run([&](float deltaTime) {
    ImGui::Text("%f ms (%f FPS)", deltaTime, 1.0f / deltaTime);
    const char *items[] = {"Triangles", "Bars", "Dots"};
    if (ImGui::Combo("Rendering type", ((int *)&(cubeParams.mode)), items,
                     IM_ARRAYSIZE(items))) {
      cubeParams.invalidate();
    }

    if (ImGui::Checkbox("Faceculling", &cubeParams.useBackfaceCulling)) {
      cubeParams.invalidate();
    }

    ImGui::SliderFloat3("Ambient color",
                        (float *)&particle.getMaterial().ambient, 0.0f, 1.0f);

    ImGui::SliderFloat3("Diffuse color",
                        (float *)&particle.getMaterial().diffuse, 0.0f, 1.0f);

    if (particlesParams.mode == RenderMode::RENDERER_MODE_POINTS) {
      ImGui::SliderFloat("Point size", &pointDesc.pointSize, 0.5f, 20.0f);
      ImGui::SliderFloat("Edge size", &pointDesc.edgeSize, 0.0f, 0.5f);
      ImGui::Checkbox("Point diffuse color", &pointDesc.applyDiffuse);
    }

    ImGui::SliderFloat("Time speed", &timeSpeed, 0.0f, 50.0f);
    cmpParam.delatTime = deltaTime * timeSpeed;

    ImGui::SliderFloat("Bounds", &cmpParam.bounds, 0.1f * originalBounds,
                       2.0f * originalBounds);

    cubeIstc.transform.scale =
        2.0f * Vector3f(cmpParam.bounds, cmpParam.bounds, cmpParam.bounds);
  });
  api.cleanup();
  return ret;
}
