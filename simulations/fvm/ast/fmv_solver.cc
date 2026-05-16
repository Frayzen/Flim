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
  case NodeType::DIV:
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
  case NodeType::DIV:
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

using view_type = Kokkos::View<float *>;
struct Operand {
  Kokkos::Subview<view_type, std::pair<int, int>> data;
  size_t width;
  size_t height;
  MatrixXf unknownMultiplier;
  MatrixXf knownMultiplier;
};

void FVMSolver::assemble() {
  auto row_map = _A.graph.row_map;
  auto entries = _A.graph.entries;

  const auto &mesh = fvmMesh.mesh;
  const auto &node_list = nodes;

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
      "Face tmp values", maxMemoryInAST * 4 * mesh.triangles.size());
  auto indices = fvmMesh.getAdjacency().indices.d_view;
  Kokkos::parallel_for(
      "Assemble", mesh.triangles.size(), KOKKOS_LAMBDA(uint32_t id) {
        FieldLocation curLoc;
        auto resultMem = Kokkos::subview(
            scratchMemory, Kokkos::make_pair(maxMemoryInAST * 4 * id,
                                             (maxMemoryInAST + 1) * 4 * id));
        auto getOperand = [&](bool left, size_t width, size_t height) {
          auto memOffset = left ? 0 : 1;
          auto subview = Kokkos::subview(
              resultMem,
              Kokkos::make_pair(maxMemoryInAST * 2 * memOffset,
                                maxMemoryInAST * 2 * (memOffset + 1)));
          Operand cur = {
              .data = subview,
              .width = width,
              .height = height,
              .unknownMultiplier = Eigen::Map<Eigen::MatrixXf>(
                  subview.data() + maxMemoryInAST, height, width),
              .knownMultiplier =
                  Eigen::Map<Eigen::MatrixXf>(subview.data(), height, width),
          };
        };

        struct OperandStatus {
          bool leftUnknownSet;
          bool leftKnownSet;
          bool rightUnknownSet;
          bool rightKnownSet;
        } status = {};

        auto leftNode = true;
        for (size_t i = 0; i < node_list.size(); i++) {
          auto curNode = node_list(i);
          if (leftNode == 0 && !(static_cast<int>(curNode.type) &
                                 static_cast<int>(NodeType::UNARY_OP))) {
            leftNode = !leftNode;
          }
        }
      });
}
