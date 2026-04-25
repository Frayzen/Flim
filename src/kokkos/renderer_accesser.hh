#pragma once
#include "vulkan/buffers/buffer_utils.hh"
#include "vulkan/rendering/renderer.hh"
#define CUR_FRAME -1

#include <Kokkos_Core.hpp>

namespace Flim {

template <typename Type>
Kokkos::View<Type *, Kokkos::DefaultExecutionSpace>
getBufferView(const Buffer &buffer) {
  // Check if the DefaultExecutionSpace can access a device memory space
  using HostMemSpace = typename Kokkos::HostSpace::memory_space;
  static_assert(!Kokkos::SpaceAccessibility<Kokkos::DefaultExecutionSpace,
                                            HostMemSpace>::accessible);
  assert(buffer.getSize() % sizeof(Type) == 0);
  return Kokkos::View<Type *, Kokkos::DefaultExecutionSpace>(
      (Type *)buffer.getExternalPtr(), buffer.getSize() / sizeof(Type));
}

template <typename Type>
Kokkos::View<Type *, Kokkos::DefaultExecutionSpace>
getAttributeBufferView(const Renderer &r, int binding,
                       ssize_t frame = CUR_FRAME) {
  for (auto &attr : r.params.getAttributeDescriptors()) {
    if (attr.second->getBinding() == binding)
      return getBufferView<Type>(*(attr.second->getBuffer(frame)));
  }
  throw std::runtime_error("Invalid binding");
};

Kokkos::View<Vector3<uint32_t> *, Kokkos::DefaultExecutionSpace>
getIndexBufferView(const Renderer &r) {
  return getBufferView<Vector3<uint32_t>>(r.indexBuffer);
}

}; // namespace Flim
