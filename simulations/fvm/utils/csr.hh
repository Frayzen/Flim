//
// This file is subject to the terms and conditions defined in
// file 'LICENSE.txt', which is part of this source code package.
//

#pragma once

#include <Kokkos_DualView.hpp>

template <typename ExecSpace, typename ValueType>
/**
 * A representation of non constant size vector list.
 * The vector list values is contiguous in memory in the shape of a kokkos view.
 * A second view is allocated to specifies the starting and ending points of
 * each vector. Example: to store (1,2,3), (4,5) in memory, values would contain
 * (1,2,3,4,5) and indices (0,3,5)
 */
class CSRList {
  using MemorySpace = typename ExecSpace::memory_space;

public:
  Kokkos::DualView<ValueType *, MemorySpace> values;
  Kokkos::DualView<uint *, MemorySpace> indices;

  CSRList() = default;

  CSRList(std::string name, int nbValues, int nbIndices)
      : indices(Kokkos::view_alloc(ExecSpace{}, Kokkos::WithoutInitializing,
                                   name + " indices " + ExecSpace{}.name()),
                nbIndices),
        values(Kokkos::view_alloc(ExecSpace{}, Kokkos::WithoutInitializing,
                                  name + " values " + ExecSpace{}.name()),
               nbValues) {}

  /**
   * @brief Retrieve the subview of a specific entry. In the example where this
   * object holds ((1,2,3), (4,5)), calling this function whith the position 0
   * returns (1,2,3) and with the position 1 returns (4,5).
   * @param[in] pos the position of the entry.
   * @return A subview of the value view holding the values of the vector at
   * position "pos".
   */
  KOKKOS_INLINE_FUNCTION Kokkos::View<ValueType *, MemorySpace>
  entryAt_device(int pos) const {
    return Kokkos::subview(values.view_device(),
                           Kokkos::make_pair(indices.view_device()(pos),
                                             indices.view_device()(pos + 1)));
  }

  Kokkos::View<ValueType *, Kokkos::HostSpace> entryAt_host(int pos) {
    Kokkos::View<ValueType *, Kokkos::HostSpace> data = values.view_host();
    auto span = Kokkos::make_pair(indices.view_host()(pos),
                                  indices.view_host()(pos + 1));
    return Kokkos::subview(data, span);
  }

  /**
   * @brief Retrieve the number of vector contained in this object.
   * @return The number of vector contained.
   */
  KOKKOS_INLINE_FUNCTION int size() const { return indices.extent(0) - 1; }

  CSRList(Kokkos::View<uint *, MemorySpace> fromIndices,
          Kokkos::View<ValueType *, MemorySpace> fromValues)
      : indices(fromIndices), values(fromValues) {};

  void Sync() {
    indices.modify_host();
    indices.sync_device();
    values.modify_host();
    values.sync_device();
  }
};
