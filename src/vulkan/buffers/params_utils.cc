#include "params_utils.hh"

#include "api/parameters/render_params.hh"
#include "api/tree/camera.hh"
#include "vulkan/buffers/attribute_descriptors.hh"
#include "vulkan/buffers/uniform_descriptors.hh"
#include "vulkan/context.hh"

namespace Flim {

AttributeDescriptor &
ParamsUtils::createInstanceMatrixAttribute(RenderParams &params, int binding) {
  auto &attr = params.setAttribute(binding, AttributeRate::INSTANCE);
  attr.onlySetup(true)
      .attach<Matrix4f>([](const Mesh &m, Matrix4f *mats) {
        for (size_t i = 0; i < m.instances.size(); i++) {
          mats[i] = m.instances[i].transform.getViewMatrix();
        }
      })
      .add(0 * sizeof(Vector4f), VK_FORMAT_R32G32B32A32_SFLOAT)
      .add(1 * sizeof(Vector4f), VK_FORMAT_R32G32B32A32_SFLOAT)
      .add(2 * sizeof(Vector4f), VK_FORMAT_R32G32B32A32_SFLOAT)
      .add(3 * sizeof(Vector4f), VK_FORMAT_R32G32B32A32_SFLOAT);
  return attr;
}

AttributeDescriptor &ParamsUtils::createVerticesAttribute(RenderParams &params,
                                                          int binding,
                                                          bool usesPos,
                                                          bool usesNormal,
                                                          bool usesUv) {
  AttributeDescriptor &attr =
      params.setAttribute(binding)
          .attach<Flim::Vertex>([](const Mesh &m, Flim::Vertex *vertices) {
            memcpy(vertices, m.vertices.data(),
                   m.vertices.size() * sizeof(Flim::Vertex));
          })
          .onlySetup(true)
          .singleBuffered(true);

  if (usesPos)
    attr.add(offsetof(Flim::Vertex, pos), VK_FORMAT_R32G32B32_SFLOAT);
  if (usesNormal)
    attr.add(offsetof(Flim::Vertex, normal), VK_FORMAT_R32G32B32_SFLOAT);
  if (usesUv)
    attr.add(offsetof(Flim::Vertex, uv), VK_FORMAT_R32G32_SFLOAT);

  return attr;
}

UniformDescriptor &ParamsUtils::createViewMatrixUniform(RenderParams &params,
                                                        int binding,
                                                        const Mesh &m,
                                                        const Camera &cam) {
  struct LocationUniform {
    Matrix4f model;
    Matrix4f view;
    Matrix4f proj;
  };
  return params.setUniform(binding, VERTEX_SHADER_STAGE)
      .attach<LocationUniform>([&](LocationUniform *uni) {
        uni->model = m.transform.getViewMatrix();
        uni->view = cam.getViewMat();
        uni->proj =
            cam.getProjMat(context.swapChain.swapChainExtent.width /
                           (float)context.swapChain.swapChainExtent.height);
      });
}

UniformDescriptor &ParamsUtils::createMaterialUniform(RenderParams &params,
                                                      int binding,
                                                      const Mesh &m) {
  struct MaterialUniform {
    Vector3f ambient;
    Vector3f diffuse;
    Vector3f specular;
    int mask;
  };
  return params.setUniform(binding, FRAGMENT_SHADER_STAGE)
      .attach<MaterialUniform>([&](MaterialUniform *uni) {
        uni->ambient = m.getMaterial().ambient;
        uni->diffuse = m.getMaterial().diffuse;
        uni->specular = m.getMaterial().specular;
        uni->mask = m.getMaterial().mask;
      });
}
} // namespace Flim
