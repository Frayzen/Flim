#pragma once

#include "api/parameters/render_params.hh"
#include "api/tree/camera.hh"
#include "fwd.hh"
#include "vulkan/buffers/descriptor_holder.hh"
#include "vulkan/rendering/pipeline.hh"
#include <vector>
namespace Flim {
class RenderParams;
class Instance;
class Mesh;
}; // namespace Flim

class Renderer : public DescriptorHolder {
public:
  Renderer(Renderer &) = delete;
  Renderer() = delete;
  void setup();
  void cleanup();
  void update(const Flim::Camera &cam);

  void setupUniforms();
  void updateUniforms(const Flim::Instance &obj, const Flim::Camera &cam);

  const std::vector<Flim::Instance> &getInstances();

  Renderer(Flim::Mesh &mesh, Flim::RenderParams &params)
      : DescriptorHolder(mesh, params, false), params(params), version(0),
        pipeline(*this) {};
  Flim::RenderParams &params;

  Buffer indexBuffer;
  Pipeline pipeline;

private:
  int version;
};
