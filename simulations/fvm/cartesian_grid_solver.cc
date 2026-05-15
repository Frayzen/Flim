#include "cartesian_grid_solver.hh"
#include <KokkosBlas1_axpby.hpp>
#include <KokkosBlas1_dot.hpp>
#include <KokkosSparse_CrsMatrix.hpp>
#include <KokkosSparse_spmv.hpp>
#include <Kokkos_Core.hpp>

CartesianGridSolver::CartesianGridSolver(int nb_x, int nb_y, float L_x,
                                         float L_y)
    : _nx(nb_x), _ny(nb_y), _total(nb_x * nb_y) {

  _dx = L_x / (float)(nb_x - 1);
  _dy = L_y / (float)(nb_y - 1);
  _Vj = _dx * _dy;

  _u_n = Kokkos::View<float *>("u_n", _total);
  _f_ext = Kokkos::View<float *>("f_ext", _total);
  _b = Kokkos::View<float *>("rhs", _total);

  // Pre-allocate solver buffers
  _r = Kokkos::View<float *>("r", _total);
  _p = Kokkos::View<float *>("p", _total);
  _Ap = Kokkos::View<float *>("Ap", _total);

  setupTopology();
}

// Accessors
Kokkos::View<float *> CartesianGridSolver::getSolution() const { return _u_n; }
int CartesianGridSolver::getWidth() const { return _nx; }
int CartesianGridSolver::getHeight() const { return _ny; }
Kokkos::View<float *> CartesianGridSolver::getSourceTerm() const {
  return _f_ext;
}

void CartesianGridSolver::setupTopology() {
  typename MatrixType::StaticCrsGraphType::row_map_type::non_const_type row_map(
      "row_map", _total + 1);
  auto row_map_host = Kokkos::create_mirror_view(row_map);

  int nnz = 0;
  for (int idx = 0; idx < _total; idx++) {
    row_map_host(idx) = nnz;
    int i = idx % _nx;
    int j = idx / _nx;
    nnz++; // Diagonal
    if (i > 0)
      nnz++;
    if (i < _nx - 1)
      nnz++;
    if (j > 0)
      nnz++;
    if (j < _ny - 1)
      nnz++;
  }
  row_map_host(_total) = nnz;
  Kokkos::deep_copy(row_map, row_map_host);

  typename MatrixType::index_type::non_const_type entries("entries", nnz);
  typename MatrixType::values_type::non_const_type values("values", nnz);
  _A = MatrixType("FVM_A", _total, _total, nnz, values, row_map, entries);
}

void CartesianGridSolver::assemble(const Params &p, float time) {
  auto row_map = _A.graph.row_map;
  auto entries = _A.graph.entries;
  auto values = _A.values;
  auto u_n = _u_n;
  auto b = _b;
  auto f_ext = _f_ext;

  int nx = _nx;
  int ny = _ny;
  float dx = _dx;
  float dy = _dy;
  float Vj = _Vj;

  Kokkos::parallel_for(
      "AssembleSystem", _total, KOKKOS_LAMBDA(const int idx) {
        int i = idx % nx;
        int j = idx / nx;
        int row_ptr = row_map(idx);
        int current_entry = row_ptr;

        // Use p.T as provided in your updated Params struct
        float diag = Vj * (p.T / p.dt) + Vj * p.K;
        float rhs = Vj * f_ext(idx) + Vj * (p.T / p.dt) * u_n(idx);

        // Diagonal entry first
        entries(current_entry) = idx;
        current_entry++;

        // lambda now uses directional A and B coefficients
        auto add_face = [&](int ni, int nj, float n_d, float face_len,
                            float dist, float advection_coeff,
                            float diffusion_coeff) {
          int k_idx = nj * nx + ni;

          // ck (neighbor coefficient) and cj (contribution to center diagonal)
          float ck = face_len * ((advection_coeff * n_d * 0.5f) +
                                 (diffusion_coeff / dist));
          float cj = face_len * ((advection_coeff * n_d * 0.5f) -
                                 (diffusion_coeff / dist));

          entries(current_entry) = k_idx;
          values(current_entry) = ck;
          diag += cj;
          current_entry++;
        };

        // Dimension X faces (Left/Right)
        if (i > 0)
          add_face(i - 1, j, -1.0f, dy, dx, p.A_0, p.B_0);
        if (i < nx - 1)
          add_face(i + 1, j, 1.0f, dy, dx, p.A_0, p.B_0);

        // Dimension Y faces (Top/Bottom)
        if (j > 0)
          add_face(i, j - 1, -1.0f, dx, dy, p.A_1, p.B_1);
        if (j < ny - 1)
          add_face(i, j + 1, 1.0f, dx, dy, p.A_1, p.B_1);

        values(row_ptr) = diag;
        b(idx) = rhs;
      });
}

void CartesianGridSolver::solve() {
  Kokkos::deep_copy(_u_n, 0.0f); // Or use previous guess
  Kokkos::deep_copy(_r, _b);
  Kokkos::deep_copy(_p, _r);

  float rr_old = KokkosBlas::dot(_r, _r);
  if (rr_old < 1e-10f)
    return;

  for (int i = 0; i < 500; ++i) {
    KokkosSparse::spmv("N", 1.0f, _A, _p, 0.0f, _Ap);
    float pAp = KokkosBlas::dot(_p, _Ap);
    if (std::abs(pAp) < 1e-12f)
      break;

    float alpha = rr_old / pAp;
    KokkosBlas::axpy(alpha, _p, _u_n);
    KokkosBlas::axpy(-alpha, _Ap, _r);

    float rr_new = KokkosBlas::dot(_r, _r);
    if (std::sqrt(rr_new) < 1e-5f)
      break;

    KokkosBlas::axpby(1.0f, _r, rr_new / rr_old, _p);
    rr_old = rr_new;
  }
}
