#pragma once

#include "ast/fvm_mesh.hh"
#include "ast/fvm_stack.hh"
#include "fvm_ast.hh"

// #include <KokkosBlas1_axpby.hpp>
// #include <KokkosBlas1_dot.hpp>
// #include <KokkosSparse_spmv.hpp>
#include <KokkosSparse_CrsMatrix.hpp>
#include <Kokkos_Core.hpp>
#include <vector>

using MatrixType = KokkosSparse::CrsMatrix<float, int, DeviceSpace, void, int>;

class FVMSolver {
public:
  FVMSolver(const FVMAst &ast, const FVMMesh &mesh)
      : fvmMesh(mesh), rpn(ast.getRPN()), stack(rpn, ast.getDepth()) {
    setup(ast);
  }

  void setup(const FVMAst &ast);
  void assemble();

private:
  Kokkos::View<const FVMAst::Node *, DeviceSpace> nodes;
  CSRList<DeviceSpace, float> nodeValues;
  MatrixType _A;
  const FVMMesh &fvmMesh;
  const std::vector<FVMAst::Node> rpn;
  const FVMStack stack;
};
