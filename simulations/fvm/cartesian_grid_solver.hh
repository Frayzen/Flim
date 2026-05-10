#include <KokkosBlas1_axpby.hpp>
#include <KokkosBlas1_dot.hpp>
#include <KokkosSparse_CrsMatrix.hpp>
#include <KokkosSparse_spmv.hpp>
#include <Kokkos_Core.hpp>

/*
 * Solve the equation
 * T_d du/dt + A_d du/dx + A_d du/dy + B_d d^2u/dx^2 + B_d d^2u/dy^2 + Ku = f
 */

class CartesianGridSolver {
public:
  using DeviceSpace = Kokkos::DefaultExecutionSpace;
  using MatrixType =
      KokkosSparse::CrsMatrix<float, int, DeviceSpace, void, int>;

  struct Params {
    float T_d = 1.0f; // Time coefficient
    float A_d = 0.0f; // Advection coefficient
    float B_d = 0.1f; // Diffusion coefficient
    float K = 0.0f;   // Reaction coefficient
    float dt = 0.01f; // Time step
  };

  CartesianGridSolver(int nb_x, int nb_y, float L_x, float L_y);

  void step(const Params &p, float time);

  // Accessors
  Kokkos::View<float *> getSolution() const;
  int getWidth() const;
  int getHeight() const;

private:
  int _nx, _ny, _total;
  float _dx, _dy, _Vj;
  MatrixType _A;
  Kokkos::View<float *> _u_n, _f_ext, _b;
  // Solver internal buffers
  Kokkos::View<float *> _r, _p, _Ap;
  void setupTopology();
  void assemble(const Params &p, float time);
  void solve();
};
