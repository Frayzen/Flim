#pragma once

#include <Kokkos_Core.hpp>
#include <Kokkos_Core_fwd.hpp>
#include <cstdint>
#include <decl/Kokkos_Declare_OPENMP.hpp>
#include <functional>
#include <setup/Kokkos_Setup_HIP.hpp>

using IndexType = uint32_t;

template <typename ValueType> class DualCSRList;

template <typename ValueType, typename ExecSpace> class CSRList {
  using MemorySpace = typename ExecSpace::memory_space;

public:
  Kokkos::View<ValueType *, MemorySpace> values;
  Kokkos::View<IndexType *, MemorySpace> indices;

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
  entryAt(IndexType pos) const {
    return Kokkos::subview(values,
                           Kokkos::make_pair(indices(pos), indices(pos + 1)));
  }

  KOKKOS_INLINE_FUNCTION Kokkos::View<ValueType *, MemorySpace>
  amount(IndexType pos) const {
    return indices(pos + 1) - indices(pos);
  }
  friend class DualCSRList<ValueType>;
};

template <typename ValueType, typename ExecSpace> class CSRListBuilder {
  CSRList<ValueType, ExecSpace> buildCSR(
      std::string name, IndexType elementNumber,
      std::function<size_t(IndexType index)> amountLambda,
      std::function<ValueType(IndexType index, size_t subIndex)> valueLambda) {
    using MemorySpace = typename ExecSpace::memory_space;
    CSRList<ValueType, Kokkos::HostSpace> hostCSR("host builder " + name, 0,
                                                  elementNumber);
    CSRList<ValueType, ExecSpace> deviceCSR;
    auto numRows = hostCSR.indices.size() + 1;
    Kokkos::parallel_scan(
        "BuildRowPtr",
        Kokkos::RangePolicy<Kokkos::DefaultHostExecutionSpace>(0, numRows),
        KOKKOS_LAMBDA(const IndexType i, int &partial_sum,
                      const bool is_final) {
          int val = (i < numRows) ? hostCSR.indices(i) : 0;
          partial_sum += amountLambda(i);
          if (is_final) {
            hostCSR.indices(i) = partial_sum;
          }
        });
    Kokkos::fence();
    hostCSR.values = Kokkos::View<ValueType *, Kokkos::HostSpace>(
        hostCSR.indices(hostCSR.indices.size() - 1));
    Kokkos::parallel_for(
        hostCSR.indices.size(), KOKKOS_LAMBDA(IndexType index) {
          for (size_t subIndex = 0; subIndex < hostCSR.amount(index);
               subIndex++) {
            hostCSR.entryAt(index)(subIndex) = valueLambda(index);
          }
        });
    deviceCSR.indices =
        Kokkos::View<IndexType *, MemorySpace>(hostCSR.indices.size());
    deviceCSR.values =
        Kokkos::View<IndexType *, MemorySpace>(hostCSR.values.size());
    Kokkos::fence();
    Kokkos::deep_copy(deviceCSR.values, hostCSR.values);
    Kokkos::deep_copy(deviceCSR.indices, hostCSR.indices);
    return deviceCSR;
  }
};
