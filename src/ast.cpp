#include "ast.h"

using namespace std;

void AstNode::set_child(size_t pos, Unode&& ptr) {
  if (children.size() <= pos)
    children.resize(pos+1);
  children[pos] = std::move(ptr);
}

using Value = ICell::Value;

Value AstUnary::evaluate (const ISheet& sh) {
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

void AstUnary::print(ostream& os) const  {
  os << op->symbol();
  children[0]->print_as_son(os, op->symbol());
}

Value AstBinary::evaluate(const ISheet& sh) {
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

void AstBinary::print(std::ostream& os) const {
  children[0]->print_as_left_son(os, op->symbol());
  os << op->symbol();
  children[1]->print_as_right_son(os, op->symbol());
}

void AstBinary::print_as_son(std::ostream& os, char parent_op) const  {
  if (op->symbol() == '+' || op->symbol() == '-') {
    print_in_brackets(os);
  } else
    print(os);
}

void AstBinary::print_as_left_son(std::ostream& os, char parent_op) const  {
  if (op->symbol() == '+' || op->symbol() == '-') {
    if (parent_op == '*' || parent_op == '/') {
      print_in_brackets(os);;
    } else {
      print(os);
    }
  } else {
    print(os);
  }
}

void AstBinary::print_as_right_son(std::ostream& os, char parent_op) const  {
  if (op->symbol() == '+' || op->symbol() == '-') {
    if (parent_op == '+')
      print(os);
    else {
      print_in_brackets(os);
    }
  } else {
    if (parent_op == '/') {
      print_in_brackets(os);
    } else
      print(os);
  }
}

void AstBinary::print_in_brackets(std::ostream& os) const {
  os << '(';
  print(os);
  os << ')';
}

AstCell::AstCell(std::string_view p): cell_ptr() {
  auto mp = Position::FromString(p);
  if (!mp.IsValid()) {
    throw FormulaException{mp.ToString()};
  }
  pos = mp;
}

optional<Position> AstCell::get_position() const {
  return pos;
}

Position* AstCell::get_mutable_position() {
  return &pos;
}

ICell* AstCell::get_ptr() const  {
  return const_cast<ICell*>(cell_ptr);
}

void AstCell::populate(const ISheet& sh) {
  cell_ptr = sh.GetCell(pos);
}

Value AstCell::evaluate(const ISheet& sh) {
  if (cell_ptr)
    return cell_ptr->GetValue();
  // for separate formulas
  auto ptr = sh.GetCell(pos);
  if (!ptr)
    return {0.};
  else {
    return ptr->GetValue();
  }
}

Position AstCell::get_pos() const  {
  return pos;
}

void AstCell::maybe_insert_pos(std::vector<Position>& positions) const {
  positions.push_back(pos);
}

void AstCell::print(std::ostream& os) const {
  if (pos.IsValid()) {
    os << pos.ToString();
  } else {
    os << "#REF!";
  }
}

void AstCell::print_as_son(std::ostream& os, char parent_op) const{
  print(os);
}

void AstCell::print_as_left_son(std::ostream& os, char parent_op) const  {
  print(os);
}

void AstCell::print_as_right_son(std::ostream& os, char parent_op) const {
  print(os);
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
