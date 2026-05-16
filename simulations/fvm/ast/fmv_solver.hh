#pragma once

#include "fvm_ast.hh"

#include <KokkosBlas1_axpby.hpp>
#include <KokkosBlas1_dot.hpp>
#include <KokkosSparse_CrsMatrix.hpp>
#include <KokkosSparse_spmv.hpp>
#include <Kokkos_Core.hpp>

using DeviceSpace = Kokkos::DefaultExecutionSpace;
using MatrixType = KokkosSparse::CrsMatrix<float, int, DeviceSpace, void, int>;

class FVMSolver {
public:
  FVMSolver(FVMAst ast) : ast(ast) {};

  void assemble();

private:
  MatrixType _A;
  FVMAst ast;
};
