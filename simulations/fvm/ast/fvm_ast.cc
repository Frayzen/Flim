#include "fvm_ast.hh"
#include <iostream>

// --- FVMAst::Expr Implementations ---

size_t FVMAst::Expr::width() const { return tree.nodes[id].width; }

size_t FVMAst::Expr::height() const { return tree.nodes[id].height; }

// --- FVMAst Node Creation Implementations ---

FVMAst::Expr FVMAst::getImplicitUnknown() {
  return {*this, addNode({NodeType::UNKNOWN_IMPLICIT, unknown_size, 1})};
}

FVMAst::Expr FVMAst::getExplicitUnknown() {
  return {*this, addNode({NodeType::UNKNOWN_EXPLICIT, unknown_size, 1})};
}

FVMAst::Expr FVMAst::constant(float v) {
  Node n{NodeType::CONST, 1, 1};
  n.data.value = v;
  return {*this, addNode(n)};
}

int FVMAst::addNode(Node n) {
  nodes.push_back(n);
  return static_cast<int>(nodes.size()) - 1;
}

// --- Global Operator Overloads ---

#define VAL(sign, type)                                                        \
  FVMAst::Expr operator sign(float s, FVMAst::Expr e) {                        \
    int s_idx = e.tree.addNode({NodeType::CONST, 1, 1, -1, -1, s});            \
    return {e.tree, e.tree.addNode({NodeType::type, e.height(), e.width(),     \
                                    s_idx, e.id})};                            \
  }                                                                            \
  FVMAst::Expr operator sign(FVMAst::Expr e, float s) {                        \
    int s_idx = e.tree.addNode({NodeType::CONST, 1, 1, -1, -1, s});            \
    return {e.tree, e.tree.addNode({NodeType::type, e.height(), e.width(),     \
                                    e.id, s_idx})};                            \
  }                                                                            \
  FVMAst::Expr operator sign(FVMAst::Expr lhs, FVMAst::Expr rhs) {             \
    assert(lhs.width() == rhs.width());                                        \
    assert(lhs.height() == rhs.height());                                      \
    return {lhs.tree, lhs.tree.addNode({NodeType::type, lhs.height(),          \
                                        lhs.width(), lhs.id, rhs.id})};        \
  }
AST_OPERATORS
#undef VAL

FVMAst::Expr transpose(FVMAst::Expr e) {
  // ONLY WORKS FOR VECTOR FOR NOW
  assert(e.width() == 1 || e.height() == 1);
  return {e.tree,
          e.tree.addNode({NodeType::TRANSPOSE, e.width(), e.height(), e.id})};
}

// --- PDE Differential Operators ---

FVMAst::Expr div(FVMAst::Expr e) {
  assert(e.width() == e.tree.space_dimension);
  return {e.tree, e.tree.addNode({NodeType::DIVERGENCE, e.height(), 1, e.id})};
}

FVMAst::Expr grad(FVMAst::Expr e) {
  assert(e.width() == 1);
  return {e.tree, e.tree.addNode({NodeType::GRADIENT, e.height(),
                                  e.tree.space_dimension, e.id})};
}

FVMAst::Expr FVMAst::vector(std::initializer_list<float> elements) {
  size_t h = elements.size();

  // Register standard CSR offsets for a dense column vector
  int mat_id = static_cast<int>(offsets.size());
  offsets.push_back(static_cast<int>(values.size()));

  for (float val : elements) {
    values.push_back(val);
    offsets.push_back(
        static_cast<int>(values.size())); // Every element gets a row offset
  }

  Node n{NodeType::CONST, h, 1};
  n.data.id = mat_id;
  return {*this, addNode(n)};
}

FVMAst::Expr
FVMAst::matrix(std::initializer_list<std::initializer_list<float>> rows) {
  size_t h = rows.size();
  size_t w = (h > 0) ? rows.begin()->size() : 0;

  int mat_id = static_cast<int>(offsets.size());
  offsets.push_back(static_cast<int>(values.size()));

  for (const auto &row : rows) {
    assert(row.size() == w && "All rows in a matrix must have the same width!");
    for (float val : row) {
      values.push_back(val);
    }
    offsets.push_back(static_cast<int>(values.size()));
  }

  Node n{NodeType::CONST, h, w};
  n.data.id = mat_id;
  return {*this, addNode(n)};
}

static std::string nodeTypeToString(NodeType type) {
  switch (type) {
  case NodeType::ADD:
    return " + ";
  case NodeType::SUB:
    return " - ";
  case NodeType::MUL:
    return " * ";
  case NodeType::DIV:
    return " / ";
  case NodeType::DIVERGENCE:
    return "div";
  case NodeType::GRADIENT:
    return "grad";
  case NodeType::TRANSPOSE:
    return "transpose";
  case NodeType::NEGATE:
    return "-";
  default:
    return "";
  }
}

std::string FVMAst::nodeToString(int node_id) const {
  if (node_id == -1)
    return "";

  const Node &node = nodes[node_id];
  int type_val = static_cast<int>(node.type);

  // 1. Handle leaf/value nodes (Constants and Unknowns)
  if ((type_val & 0x100) == 0x100) {
    if (node.type == NodeType::CONST) {
      // Note: If you stored matrix_id in CSR, you might print a shape tag like
      // [2x2 Matrix]
      if (node.width == 1 && node.height == 1) {
        return std::to_string(node.data.value);
      }
      return "[" + std::to_string(node.height) + "x" +
             std::to_string(node.width) + " Const]";
    }
    if (node.type == NodeType::UNKNOWN_IMPLICIT) {
      return "u_implicit(" + std::to_string(node.height) + "x" +
             std::to_string(node.width) + ")";
    }
    if (node.type == NodeType::UNKNOWN_EXPLICIT) {
      return "u_explicit(" + std::to_string(node.height) + "x" +
             std::to_string(node.width) + ")";
    }
  }

  // 2. Handle binary operations (In-order traversal: Left Op Right)
  if (type_val & static_cast<int>(NodeType::BINARY_OP)) {
    std::string left_str = nodeToString(node.left);
    std::string right_str = nodeToString(node.right);
    std::string op_str = nodeTypeToString(node.type);

    // Wrapping in parentheses ensures clear evaluation order layout
    return "(" + left_str + " " + op_str + " " + right_str + ")";
  }

  // 3. Handle unitary/prefix operations (Prefix traversal: Op(Left))
  if (type_val & static_cast<int>(NodeType::UNARY_OP)) {
    std::string child_str = nodeToString(node.left);
    std::string op_str = nodeTypeToString(node.type);

    if (node.type == NodeType::NEGATE) {
      return op_str + "(" + child_str + ")";
    }
    // Differential operators like div() or grad()
    return op_str + "(" + child_str + ")";
  }

  return "INVALID_NODE";
}

void FVMAst::Expr::print() const { std::cout << toString() << std::endl; }

std::string FVMAst::Expr::toString() const { return tree.nodeToString(id); }
