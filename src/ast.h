#pragma once

#include "common.h"
#include "utility.h"
#include "unary_ops.h"
#include "binary_ops.h"
#include <memory>
#include <unordered_set>
#include <variant>
#include <iostream>
#include <set>
#include <optional>
#include <cmath>

class AstNode;
class SheetTester;

using Unode = std::unique_ptr<AstNode>;

class AstNodeNotCompletedError {
public:
  AstNodeNotCompletedError() = default;
};

class AstNode {
public:
  using Value = ICell::Value;
  virtual Value evaluate(const ISheet&) = 0;
  virtual std::optional<Position> get_position() const { return std::nullopt; }
  virtual Position* get_mutable_position() { return nullptr; }
  virtual ICell* get_ptr() const { return nullptr; }
  virtual void populate(const ISheet& sh) {};
  std::vector<Unode>& get_children() { return children;  }
  void push_child(Unode&& ptr) { children.push_back(std::move(ptr)); }
  void set_child(size_t pos, Unode&& ptr);
  virtual void maybe_insert_pos(std::vector<Position>& positions) const {};

  virtual void print(std::ostream& os) const = 0;
  virtual void print_as_son(std::ostream& os, char parent_op) const = 0;
  virtual void print_as_left_son(std::ostream& os, char parent_op) const = 0;
  virtual void print_as_right_son(std::ostream& os, char parent_op) const = 0;

  virtual ~AstNode() = default;
  friend class SheetTester;
protected:
  std::vector<Unode> children;
};

class AstNum: public AstNode {
public:
  AstNum(double num): num(num) {};
  Value evaluate(const ISheet&) override { return {num};  }
  void print(std::ostream& os) const override { os << num; };
  void print_as_son(std::ostream& os, char parent_op) const override { os << num; };
  void print_as_left_son(std::ostream& os, char parent_op) const override { os << num; };
  void print_as_right_son(std::ostream& os, char parent_op) const override { os << num; };
private:
  double num = 0.;
};

class AstUnary: public AstNode {
public:
  AstUnary(char op): op(make_unary_op(op)) { };
  Value evaluate (const ISheet& sh) override;
  void print(std::ostream& os) const override;
  void print_as_son(std::ostream& os, char parent_op) const override { print(os); };
  void print_as_left_son(std::ostream& os, char parent_op) const override { print(os); };
  void print_as_right_son(std::ostream& os, char parent_op) const override { print(os); };
protected:
  std::unique_ptr<UnaryOP> op;
};



class AstBinary: public AstNode {
public:
  AstBinary(char op): op(make_binary_op(op)) { };
  Value evaluate(const ISheet& sh) override;
  void print(std::ostream& os) const override;
  void print_as_son(std::ostream& os, char parent_op) const override;
  void print_as_left_son(std::ostream& os, char parent_op) const override;
  void print_as_right_son(std::ostream& os, char parent_op) const override;
protected:
  std::unique_ptr<BinaryOP> op;
private:
  void print_in_brackets(std::ostream& os) const;
};

void PrintCellValue(std::ostream& os, const ICell::Value& val);

class AstCell: public AstNode {
public:
  AstCell(std::string_view p);
  std::optional<Position> get_position() const override;
  Position* get_mutable_position() override;
  ICell* get_ptr() const override;
  void populate(const ISheet& sh) override;
  Value evaluate(const ISheet& sh) override;
  Position get_pos() const;
  void maybe_insert_pos(std::vector<Position>& positions) const override;
  void print(std::ostream& os) const override;
  void print_as_son(std::ostream& os, char parent_op) const override ;
  void print_as_left_son(std::ostream& os, char parent_op) const override;
  void print_as_right_son(std::ostream& os, char parent_op) const override;
protected:
  const ICell *cell_ptr;
  Position pos;

};
