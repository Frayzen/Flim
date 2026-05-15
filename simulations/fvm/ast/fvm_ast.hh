#pragma once

#include <cassert>
#include <cstddef>
#include <string>
#include <vector>

enum class NodeType {
  VALUE_OP = 0x100,
  UNKNOWN_IMPLICIT,
  UNKNOWN_EXPLICIT,
  CONST,

  BINARY_OP = 0x200,
  ADD,
  SUB,
  MUL,
  DIV,
  POW,

  UNARY_OP = 0x400,
  TRANSPOSE,
  DIVERGENCE,
  GRADIENT,
  NEGATE
};

class FVMAst {
public:
  union NodeData {
    float value;
    int id; // corresponds to a csr id position in associated vector
  };

  struct Node {
    NodeType type;
    size_t height;
    size_t width;
    int left = -1;
    int right = -1;
    NodeData data = {0.0f};
  };

  // Lightweight handle for syntax sugar
  struct Expr {
    FVMAst &tree;
    int id;

    size_t width() const;
    size_t height() const;
    void print() const;
    std::string toString() const;
  };

  // --- Node Creation ---
  Expr getImplicitUnknown();
  Expr getExplicitUnknown();
  Expr constant(float v);

  int addNode(Node n);

  // --- Data Access ---
  const std::vector<Node> &getNodes() const { return nodes; }
  const Node &root(Expr e) const { return nodes[e.id]; }

  FVMAst(size_t space_dimension, size_t unknown_size)
      : unknown_size(unknown_size), space_dimension(space_dimension) {};

  Expr vector(std::initializer_list<float> elements);
  Expr matrix(std::initializer_list<std::initializer_list<float>> rows);
  const size_t unknown_size;
  const size_t space_dimension;

private:
  std::string nodeToString(int node_id) const;
  std::vector<Node> nodes;
  // CSR format for sparse matrices matrices data
  std::vector<float> values;
  std::vector<int> offsets;
};

// --- Global Operator Overloads ---

// Scalar Multiplication & Arithmetic via argument matching

#define AST_OPERATORS                                                          \
  VAL(*, MUL)                                                                  \
  VAL(+, ADD)                                                                  \
  VAL(-, SUB)                                                                  \
  VAL(^, POW)                                                                  \
  VAL(/, DIV)

#define VAL(sign, _)                                                           \
  FVMAst::Expr operator sign(float s, FVMAst::Expr e);                         \
  FVMAst::Expr operator sign(FVMAst::Expr e, float s);                         \
  FVMAst::Expr operator sign(FVMAst::Expr lhs, FVMAst::Expr rhs);
AST_OPERATORS
#undef VAL

// --- PDE Differential Operators ---
FVMAst::Expr div(FVMAst::Expr e);
FVMAst::Expr grad(FVMAst::Expr e);

FVMAst::Expr transpose(FVMAst::Expr e);
