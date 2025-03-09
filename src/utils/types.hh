#pragma once

#include <Eigen/Core>
#include <Eigen/Eigen>
#include <Eigen/Sparse>
#include <eigency_cpp.h>
#include <memory>

typedef Eigen::Vector2f Vector2f;
typedef Eigen::Vector3f Vector3f;
typedef Eigen::Vector4f Vector4f;

constexpr auto D = Eigen::Dynamic;

template <typename T>
std::shared_ptr<>

#define MATRIX_TYPE(Type, ShortType, Rows, Cols)                               \
  typedef Eigen::Matrix<Type, Rows, Cols> Matrix##ShortType##Rows##Cols;       \
  typedef Eigen::Map<Eigen::Matrix<Type, Rows, Cols>>                          \
      MapMatrix##ShortType##Rows##Cols; \
  typename <>

#define SET_MATRIX(Rows, Cols)                                                 \
  MATRIX_TYPE(float, f, Cols, Rows)                                            \
  MATRIX_TYPE(double, d, Cols, Rows)                                           \
  MATRIX_TYPE(float, f, Rows, Cols)                                            \
  MATRIX_TYPE(double, d, Rows, Cols)

#define SET_MATRIX_SYM(Val)                                                    \
  MATRIX_TYPE(float, f, Val, Val)                                              \
  MATRIX_TYPE(double, d, Val, Val)

SET_MATRIX_SYM(1)
SET_MATRIX_SYM(2)
SET_MATRIX_SYM(3)
SET_MATRIX_SYM(4)
SET_MATRIX_SYM(D)

SET_MATRIX(1, D)
SET_MATRIX(1, 2)
SET_MATRIX(1, 3)
SET_MATRIX(1, 4)
SET_MATRIX(2, D)
SET_MATRIX(2, 3)
SET_MATRIX(2, 4)
SET_MATRIX(3, D)
SET_MATRIX(3, 4)
SET_MATRIX(4, D)
