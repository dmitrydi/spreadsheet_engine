#include "imp_formula.h"

using namespace std;

using Value = ImpFormula::Value;

Value ImpFormula::Evaluate(const ISheet& sheet) const {
  if (valid)
    return cached_val;
  //cached_val = ast.get()->evaluate(sheet);
  visit(overload {
    [this](double val) { this->cached_val = val;},
    [this](const string& val) { this->cached_val = FormulaError(FormulaError::Category::Value); },
    [this](const FormulaError& err) { this->cached_val = err;}
  }, ast.get()->evaluate(sheet)
      );
  valid = true;
  return cached_val;
}

std::string ImpFormula::GetExpression() const {
  return {};
}

std::vector<Position> ImpFormula::GetReferencedCells() const  {
  return ref_cells;
}

ImpFormula::HandlingResult ImpFormula::HandleInsertedRows(int before, int count) {
  if (before > ref_cells.back().row)
    return HandlingResult::NothingChanged;
  if (before <= ref_cells[0].row) {
    for (auto& ref: ref_cells)
      ref.row += count;
  } else {
    auto it = lower_bound(ref_cells.begin(), ref_cells.end(), before, [](Position pos, double value) { return pos.row < value;});
    for (;it != ref_cells.end(); ++it) {
      it->row += count;
    }
  }
  return HandlingResult::ReferencesRenamedOnly;
}

ImpFormula::HandlingResult ImpFormula::HandleInsertedCols(int before, int count) {
  if (before > ref_cells.back().col)
    return HandlingResult::NothingChanged;
  if (before <= ref_cells[0].col) {
    for (auto& ref: ref_cells)
      ref.col += count;
  } else {
    auto it = lower_bound(ref_cells.begin(), ref_cells.end(), before, [](Position pos, double value) { return pos.col < value;});
    for (;it != ref_cells.end(); ++it) {
      it->col += count;
    }
  }
  return HandlingResult::ReferencesRenamedOnly;
}

ImpFormula::HandlingResult ImpFormula::HandleDeletedRows(int first, int count) {
  if (first > ref_cells.back().row)
    return HandlingResult::NothingChanged;
  if (first + count - 1 < ref_cells[0].row) {
    for (auto& ref: ref_cells)
      ref.row -= count;
    return HandlingResult::ReferencesRenamedOnly;
  }
  bool cell_deleted = false;
  auto it = lower_bound(ref_cells.begin(), ref_cells.end(), first, [](Position pos, double value) { return pos.row < value;});
  for ( ; it != ref_cells.end(); ++it) {
    if (it->row >= first && it->row <= first + count - 1) { // to optimize
      cell_deleted = true;
      //*it = {-1,-1}; // invalid position
    } else {
      it->row -=count;
    }
  }
  return cell_deleted ? HandlingResult::ReferencesChanged : HandlingResult::ReferencesRenamedOnly;
}

ImpFormula::HandlingResult ImpFormula::HandleDeletedCols(int first, int count) {
  if (first > ref_cells.back().col)
    return HandlingResult::NothingChanged;
  if (first + count - 1 < ref_cells[0].col) {
    for (auto& ref: ref_cells)
      ref.col -= count;
    return HandlingResult::ReferencesRenamedOnly;
  }
  bool cell_deleted = false;
   auto it = lower_bound(ref_cells.begin(), ref_cells.end(), first, [](Position pos, double value) { return pos.col < value;});
   for ( ; it != ref_cells.end(); ++it) {
     if (it->col >= first && it->col <= first + count - 1) { // to optimize
       cell_deleted = true;
       //*it = {-1,-1}; // invalid position
     } else {
       it->col -=count;
     }
   }
   return cell_deleted ? HandlingResult::ReferencesChanged : HandlingResult::ReferencesRenamedOnly;
}

void ImpFormula::PopulatePtrs() {
  stack<AstNode*> st;
  st.push(ast.get());
  while(!st.empty()) {
    auto top = st.top();
    st.pop();
    ImpCell* ptr = reinterpret_cast<ImpCell*>(top->get_ptr());
    ref_ptrs.insert(ptr);
    for (auto& ch: top->get_children())
      st.push(ch.get());
  }
}
