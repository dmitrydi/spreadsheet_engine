#pragma once

#include <unordered_set>
#include <memory>
#include <cmath>
#include "common.h"

static const std::unordered_set<char> BinaryOps = {'+', '-', '*', '/'};

class AstBinaryParseError {
public:
  AstBinaryParseError() = default;
};

struct BinaryOP {
  virtual ICell::Value operator()(double lhs, double rhs) const = 0;
  virtual char symbol() const = 0;
  virtual ~BinaryOP() = default;
};

struct AddOp: BinaryOP {
  ICell::Value operator()(double lhs, double rhs) const override {
    if (std::isinf(lhs+rhs))
      return FormulaError(FormulaError::Category::Div0);
    return lhs + rhs;
  }
  char symbol() const override {
    return '+';
  }
};

struct SubOp: BinaryOP {
  ICell::Value operator()(double lhs, double rhs) const override {
    if (std::isinf(lhs-rhs))
      return FormulaError(FormulaError::Category::Div0);
    return lhs - rhs;
  }
  char symbol() const override {
    return '-';
  }
};

struct MulOp: BinaryOP {
  ICell::Value operator()(double lhs, double rhs) const override {
    if (std::isinf(lhs*rhs))
      return FormulaError(FormulaError::Category::Div0);
    return lhs * rhs;
  }
  char symbol() const override {
    return '*';
  }
};

struct DivOp: BinaryOP {
  ICell::Value operator()(double lhs, double rhs) const override {
    if (std::isinf(lhs/rhs) || std::isnan(lhs/rhs))
      return FormulaError(FormulaError::Category::Div0);
    return lhs / rhs;
  }
  char symbol() const override {
    return '/';
  }
};

std::unique_ptr<BinaryOP> make_binary_op(const char op);
