#pragma once

#include "common.h"
#include "utility.h"
#include <memory>
#include <unordered_set>
#include <variant>
#include <iostream>
#include <set>
#include <optional>

class AstNode;

static const std::unordered_set<char> UnaryOps = {'+', '-'};
static const std::unordered_set<char> BinaryOps = {'+', '-', '*', '/'};



class AstUnaryParseError {
public:
  AstUnaryParseError() = default;
};

class AstBinaryParseError {
public:
  AstBinaryParseError() = default;
};

using Unode = std::unique_ptr<AstNode>;

class AstNode {
public:
  using Value = ICell::Value;
  virtual Value evaluate(const ISheet&) = 0;
  virtual std::optional<Position> get_position() const { return std::nullopt; }
  virtual ICell* get_ptr() const { return nullptr; }
  virtual void populate(const ISheet& sh) {};
  std::vector<Unode>& get_children() {
    return children;
  }
  void push_child(Unode&& ptr) {
    children.push_back(std::move(ptr));
  }
  void set_child(size_t pos, Unode&& ptr) {
    if (children.size() <= pos)
      children.resize(pos+1);
    children[pos] = std::move(ptr);
  }
  virtual void maybe_insert_pos(std::vector<Position>& positions) const {};
  virtual ~AstNode() = default;
protected:
  std::vector<Unode> children;
};

class AstNodeNotCompletedError {
public:
  AstNodeNotCompletedError() = default;
};


class AstNum: public AstNode {
public:
  AstNum(double num): num(num) {};
  Value evaluate(const ISheet&) override {
    return {num};
  }
private:
  double num = 0.;
};


struct UnaryOP {
  virtual double operator()(double val) const = 0;
  virtual ~UnaryOP() = default;
};

struct UnaryPlus: UnaryOP {
  double operator()(double val) const override {
    return val;
  }
};

struct UnaryMinus: UnaryOP {
  double operator()(double val) const override {
    return -val;
  }
};

std::unique_ptr<UnaryOP> make_unary_op(const char op);

class AstUnary: public AstNode {
public:
  AstUnary(char op): op(make_unary_op(op)) {
  };
  Value evaluate (const ISheet& sh) override {
    if (children.size() != 1)
      throw AstNodeNotCompletedError{};
    return std::visit(
          overload{
             [this](double arg) -> Value{return (*(this->op))(arg); },
             [](const std::string&) -> Value{ return FormulaError(FormulaError::Category::Value); },
             [](const FormulaError& err) -> Value{ return err;}
          }, children[0]->evaluate(sh)
        );
  }
private:
  std::unique_ptr<UnaryOP> op;
};

struct BinaryOP {
  virtual double operator()(double lhs, double rhs) const = 0;
  virtual ~BinaryOP() = default;
};

struct AddOp: BinaryOP {
  double operator()(double lhs, double rhs) const override {
    return lhs + rhs;
  }
};

struct SubOp: BinaryOP {
  double operator()(double lhs, double rhs) const override {
    return lhs - rhs;
  }
};

struct MulOp: BinaryOP {
  double operator()(double lhs, double rhs) const override {
    return lhs * rhs;
  }
};

struct DivOp: BinaryOP {
  double operator()(double lhs, double rhs) const override {
    return lhs / rhs;
  }
};

std::unique_ptr<BinaryOP> make_binary_op(const char op);

class AstBinary: public AstNode {
public:
  AstBinary(char op): op(make_binary_op(op)) {
  };
  Value evaluate(const ISheet& sh) override {
    if (children.size() != 2)
      throw AstNodeNotCompletedError{};
    return std::visit(
        overload{
          [this](double a, double b)-> Value{ return (*(this->op))(a,b); },
          [](double a, const std::string& b) -> Value { return FormulaError(FormulaError::Category::Value); },
          [](const std::string& a, double b) -> Value { return FormulaError(FormulaError::Category::Value); },
          [](const std::string& a, const std::string& b) -> Value { return FormulaError(FormulaError::Category::Value);},
          [](const std::string& a, const FormulaError& b) -> Value { return b; },
          [](const FormulaError& a, const std::string& b) -> Value { return a; },
          [](const FormulaError& a, const FormulaError& b) -> Value { return a; },
          [](const FormulaError& a, double b) -> Value { return a; },
          [](double a, const FormulaError& b) -> Value { return b; }
        }, children[0]->evaluate(sh), children[1]->evaluate(sh)
    );
  }
private:
  std::unique_ptr<BinaryOP> op;

};

void PrintCellValue(std::ostream& os, const ICell::Value& val);

class AstCell: public AstNode {
public:
  AstCell(std::string_view p): cell_ptr() {
    auto mp = Position::FromString(p);
    if (!mp.IsValid())
      throw FormulaException{mp.ToString()};
    pos = mp;
  };

  std::optional<Position> get_position() const override {
    return pos;
  }

  ICell* get_ptr() const override {
    return const_cast<ICell*>(cell_ptr);
  }

  void populate(const ISheet& sh) override {
    cell_ptr = sh.GetCell(pos);
  };

  Value evaluate(const ISheet& sh) override {
    if (cell_ptr)
      return cell_ptr->GetValue();
    // for separate formulas
    auto ptr = sh.GetCell(pos);
    if (!ptr)
      return {0.};
    return ptr->GetValue();
  }
  Position get_pos() const {
    return pos;
  }
  void maybe_insert_pos(std::vector<Position>& positions) const override {
    positions.push_back(pos);
  }
private:
  const ICell *cell_ptr;
  Position pos;

};
