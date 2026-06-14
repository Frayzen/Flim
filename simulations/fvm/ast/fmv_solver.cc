#include "fmv_solver.hh"
#include "ast/fvm_ast.hh"
#include <Eigen/Core>
#include <HIP/Kokkos_HIP_Team.hpp>
#include <Kokkos_Core.hpp>
#include <decl/Kokkos_Declare_OPENMP.hpp>
#include <impl/Kokkos_Error.hpp>

#define STACK_SIZE 32
enum FieldLocation { Faces, Cell, Same };

static FieldLocation getOperatorOutput(NodeType type) {
  switch (type) {
  case NodeType::DIVERGENCE:
  case NodeType::UNKNOWN_IMPLICIT:
  case NodeType::UNKNOWN_EXPLICIT:
    return Cell;
  case NodeType::GRADIENT:
    return Faces;
  case NodeType::NEGATE:
  case NodeType::POW:
  case NodeType::ADD:
  case NodeType::SUB:
  case NodeType::MUL:
  case NodeType::CONST:
  case NodeType::TRANSPOSE:
    return Same;
  default:
    break;
  }
  KOKKOS_ASSERT(false);
}

static FieldLocation getOperatorInput(NodeType type) {
  switch (type) {
  case NodeType::DIVERGENCE:
    return Faces;
  case NodeType::GRADIENT:
    return Cell;
  case NodeType::NEGATE:
  case NodeType::POW:
  case NodeType::ADD:
  case NodeType::SUB:
  case NodeType::MUL:
    return Same;
  default:
    break;
  }
  KOKKOS_ASSERT(false);
}

void FVMSolver::setup(const FVMAst &ast) {
  KOKKOS_ASSERT(ast.isLinearImplicit());
  Kokkos::View<const FVMAst::Node *, Kokkos::HostSpace,
               Kokkos::MemoryTraits<Kokkos::Unmanaged>>
      host_nodes(rpn.data(), rpn.size());
  nodes = Kokkos::create_mirror_view_and_copy(DeviceSpace{}, host_nodes);
  auto values = ast.getNodeValues();
  auto offsets = ast.getNodeValuesOffset();
  nodeValues =
      CSRList<DeviceSpace, float>("Node Values", values.size(), offsets.size());
  for (size_t i = 0; i < values.size(); i++)
    nodeValues.values.h_view(i) = values[i];
  for (size_t i = 0; i < offsets.size(); i++)
    nodeValues.indices.h_view(i) = offsets[i];
  nodeValues.Sync();
}

using view_type = Kokkos::View<float *, DeviceSpace::memory_space>;
struct Operand {
  Kokkos::Subview<view_type, std::pair<int, int>> view;
  size_t width;
  size_t height;
  MatrixXf value;
  FieldLocation location;
};

KOKKOS_INLINE_FUNCTION MatrixXf
getConstVal(FVMAst::Node &node, const CSRList<DeviceSpace, float> &nodeValues) {
  float *ptr = &node.data.value;
  if (node.width > 1 || node.height > 1) {
    auto subview = nodeValues.entryAt_device(node.data.id);
    ptr = subview.data();
  }
  return Eigen::Map<Eigen::MatrixXf>(ptr, node.height, node.width);
};

KOKKOS_INLINE_FUNCTION void applyMul(Operand &leftOp, Operand &rightOp) {
  auto res = leftOp.value * rightOp.value;
  leftOp.value =
      Eigen::Map<Eigen::MatrixXf>(leftOp.view.data(), res.rows(), res.cols());
  leftOp.value = res;
  leftOp.width = rightOp.width;
}

void FVMSolver::assemble() {
  auto row_map = _A.graph.row_map;
  auto entries = _A.graph.entries;

  const auto &mesh = fvmMesh.mesh;
  const auto &node_list = nodes;
  const CSRList<DeviceSpace, float> &node_values = nodeValues;

  size_t maxMemoryInAST = 0;
  for (auto node : rpn) {
    if (!(static_cast<int>(node.type) & static_cast<int>(NodeType::VALUE_OP))) {
      auto multiplier = 1;
      if (getOperatorOutput(node.type) == Faces)
        multiplier = 3;
      auto val = multiplier * node.height * node.width;
      if (val > maxMemoryInAST)
        maxMemoryInAST = val;
    }
  }
  KOKKOS_ASSERT(maxMemoryInAST > 0);
  Kokkos::View<float *, DeviceSpace> scratchMemory(
      "Face tmp values", maxMemoryInAST * 2 * mesh.triangles.size());
  auto indices = fvmMesh.getAdjacency().indices.d_view;
  // Kokkos::parallel_for(
  //     "Assemble", Kokkos::RangePolicy<DeviceSpace>(0, mesh.triangles.size()),
  //     KOKKOS_LAMBDA(uint32_t id) {
  //       auto resultMem = Kokkos::subview(
  //           scratchMemory, Kokkos::make_pair(maxMemoryInAST * 2 * id,
  //                                            (maxMemoryInAST + 1) * 2 * id));
  //       auto getOperand = [&](bool left, size_t width,
  //                             size_t height) -> Operand {
  //         auto memOffset = left ? 0 : 1;
  //         auto subview = Kokkos::subview(
  //             resultMem,
  //             Kokkos::make_pair(maxMemoryInAST * 2 * memOffset,
  //                               maxMemoryInAST * 2 * (memOffset + 1)));
  //         return {
  //             .view = subview,
  //             .width = width,
  //             .height = height,
  //             .value =
  //                 Eigen::Map<Eigen::MatrixXf>(subview.data(), height, width),
  //             .location = Cell,
  //         };
  //       };
  //       auto checkMatch = [](FieldLocation &loc1, FieldLocation &loc2) {
  //         if (loc1 == Same || loc2 == Same)
  //           return;
  //         KOKKOS_ASSERT(loc1 == loc2);
  //       };

  //       // LETS DO EVERYTHING EXPLICIT FIRST
  //       bool is_explicit = true;
  //       bool isLeftOpSet = false;
  //       Operand leftOp, rightOp;
  //       for (size_t i = 0; i < node_list.size(); i++) {
  //         auto curNode = node_list(i);

  //         Operand curOp =
  //             getOperand(!isLeftOpSet, curNode.width, curNode.height);

  //         bool is_value_op = static_cast<int>(curNode.type) &
  //                            static_cast<int>(NodeType::VALUE_OP);
  //         bool is_unary_op = static_cast<int>(curNode.type) &
  //                            static_cast<int>(NodeType::UNARY_OP);

  //         switch (curNode.type) {
  //         case NodeType::CONST:
  //           curOp.location = Same;
  //           if (is_explicit)
  //             curOp.value = getConstVal(curNode, node_values);
  //           else
  //             curOp.value.setZero();
  //           break;
  //         case NodeType::UNKNOWN_IMPLICIT:
  //           if (!is_explicit)
  //             curOp.value.setZero();
  //           else
  //             curOp.value.setIdentity();
  //           break;
  //         case NodeType::UNKNOWN_EXPLICIT:
  //           if (is_explicit)
  //             curOp.value.setZero();
  //           else
  //             curOp.value.setIdentity();
  //           break;
  //         case NodeType::MUL:
  //           checkMatch(leftOp.location, rightOp.location);
  //           applyMul(leftOp, rightOp);
  //           break;
  //         case NodeType::ADD:
  //           checkMatch(leftOp.location, rightOp.location);
  //           leftOp.value += rightOp.value;
  //           break;
  //         case NodeType::SUB:
  //           checkMatch(leftOp.location, rightOp.location);
  //           leftOp.value -= rightOp.value;
  //           break;
  //         case NodeType::DIVERGENCE:
  //           KOKKOS_ASSERT(leftOp.location == Faces);
  //           break;
  //         case NodeType::GRADIENT:
  //           KOKKOS_ASSERT(leftOp.location == Cell);
  //           break;
  //         default:
  //           KOKKOS_ASSERT(false);
  //           break;
  //         }
  //         if (is_value_op) {
  //           if (isLeftOpSet)
  //             rightOp = curOp;
  //           else
  //             leftOp = curOp;
  //         }
  //         isLeftOpSet = is_value_op || is_unary_op;
  //       }
  //     });
}
