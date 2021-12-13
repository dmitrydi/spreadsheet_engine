#pragma once

#include <memory>
#include <unordered_set>

static const std::unordered_set<char> UnaryOps = {'+', '-'};

class AstUnaryParseError {
public:
  AstUnaryParseError() = default;
};

struct UnaryOP {
  virtual double operator()(double val) const = 0;
  virtual char symbol() const = 0;
  virtual ~UnaryOP() = default;
};

struct UnaryPlus: UnaryOP {
  double operator()(double val) const override { return val; }
  char symbol() const override { return '+';  }
};

struct UnaryMinus: UnaryOP {
  double operator()(double val) const override { return -val; }
  char symbol() const override { return '-'; }
};

std::unique_ptr<UnaryOP> make_unary_op(const char op);
