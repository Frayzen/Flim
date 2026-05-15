#include "cartesian_grid_solver.hh"
struct FluidParams {
  float viscosity = 0.01f;
  float density = 1.0f;
  float dt = 0.01f;
};

class NavierStokesSolver {
public:
  NavierStokesSolver(int nx, int ny, float Lx, float Ly)
      : solverU(nx, ny, Lx, Ly), solverV(nx, ny, Lx, Ly),
        solverP(nx, ny, Lx, Ly), _nx(nx), _ny(ny) {}

  void step(const FluidParams &fp, float time) {
    const int nx = _nx;
    const int ny = _ny;
    float dx = 1.0f / (nx - 1);
    float dy = 1.0f / (ny - 1);

    // --- 1. Predictor Step (Momentum) ---
    // Note: For true NS, solverU/V needs to use the velocity field
    // to compute the advection direction at each cell face.
    CartesianGridSolver::Params momentumParams;
    momentumParams.dt = fp.dt;
    momentumParams.T = fp.density;
    momentumParams.B_d = fp.viscosity;
    momentumParams.A_d = 1.0f;

    solverU.step(momentumParams, time);
    solverV.step(momentumParams, time);

    // --- 2. Pressure Poisson Equation ---
    CartesianGridSolver::Params poissonParams;
    poissonParams.T = 0.0f;   // Steady state solve
    poissonParams.B_d = 1.0f; // Laplacian operator
    poissonParams.A_d = 0.0f; // No advection for pressure
    poissonParams.dt = fp.dt;

    auto u_star = solverU.getSolution();
    auto v_star = solverV.getSolution();
    auto p_rhs = solverP.getSourceTerm(); // Expose f_ext

    // Source term for Pressure: f = (rho/dt) * div(U*)
    Kokkos::parallel_for(
        "CalcDivergence", nx * ny, KOKKOS_LAMBDA(int idx) {
          int i = idx % nx;
          int j = idx / nx;

          // Central difference divergence (interior nodes)
          if (i > 0 && i < nx - 1 && j > 0 && j < ny - 1) {
            float du_dx = (u_star(idx + 1) - u_star(idx - 1)) / (2.0f * dx);
            float dv_dy = (v_star(idx + nx) - v_star(idx - nx)) / (2.0f * dy);
            p_rhs(idx) = (fp.density / fp.dt) * (du_dx + dv_dy);
          } else {
            p_rhs(idx) = 0.0f; // Boundary logic
          }
        });

    solverP.step(poissonParams, time);

    // --- 3. Correction Step ---
    auto p_corr = solverP.getSolution();
    Kokkos::parallel_for(
        "CorrectVelocity", nx * ny, KOKKOS_LAMBDA(int idx) {
          int i = idx % nx;
          int j = idx / nx;

          if (i > 0 && i < nx - 1 && j > 0 && j < ny - 1) {
            float dp_dx = (p_corr(idx + 1) - p_corr(idx - 1)) / (2.0f * dx);
            float dp_dy = (p_corr(idx + nx) - p_corr(idx - nx)) / (2.0f * dy);

            // u_n+1 = u* - (dt/rho) * grad(p)
            u_star(idx) -= (fp.dt / fp.density) * dp_dx;
            v_star(idx) -= (fp.dt / fp.density) * dp_dy;
          }
        });
  }

  // Helpers for your visualization code
  Kokkos::View<float *> getU() { return solverU.getSolution(); }
  Kokkos::View<float *> getV() { return solverV.getSolution(); }
  Kokkos::View<float *> getP() { return solverP.getSolution(); }

private:
  CartesianGridSolver solverU, solverV, solverP;
  int _nx, _ny;
};
