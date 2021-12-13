#include "binary_ops.h"

std::unique_ptr<BinaryOP> make_binary_op(const char op) {
  switch (op) {
  case '+':
    return std::make_unique<AddOp>();
  case '-':
    return std::make_unique<SubOp>();
  case '*':
    return std::make_unique<MulOp>();
  case '/':
    return std::make_unique<DivOp>();
  default:
    throw AstBinaryParseError{};
  }
}
