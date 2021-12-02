#include "ast.h"

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

void PrintCellValue(std::ostream& os, const ICell::Value& val) {
  std::visit(
      overload {
        [&os](const std::string& arg) { os << "Cell has string value: " << arg << std::endl; },
        [&os](double arg) { os << "Cell has double value: " << arg << std::endl; },
        [&os](const FormulaError& arg) {os << "Cell has FormulaError " << arg.ToString() << std::endl; }
      }, val
      );
}
