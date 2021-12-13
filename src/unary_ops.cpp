#include "unary_ops.h"

std::unique_ptr<UnaryOP> make_unary_op(const char op) {
  switch (op) {
  case '+':
    return std::make_unique<UnaryPlus>();
  case '-':
    return std::make_unique<UnaryMinus>();
  default:
    throw AstUnaryParseError{};
  }
}
