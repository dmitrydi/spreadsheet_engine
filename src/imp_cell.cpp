#include "imp_cell.h"

using namespace std;

ICell::Value ImpCell::GetValue() const {
  if (formula) {
    if (parent) {
      if (state == CellState::Invalid) {
        visit([this](auto&& arg){ this->cached_value = arg; }, formula->Evaluate(*parent));
        state = CellState::Valid;
      }
      return cached_value;
    }
    else
      return 0.;
  }
  return rendered_text;
}

string ImpCell::GetText() const {
  if (formula)
    return formula->GetExpression();
  return raw_text;
}

std::vector<Position> ImpCell::GetReferencedCells() const {
  if (formula)
    return formula->GetReferencedCells();
  return {};
}

void ImpCell::Clear() {
 for (auto ptr: dep_ptrs)
   ptr->Invalidate();

 if (formula)
   for (auto ptr: formula->GetRefPtrs())
     ptr->RemoveDepPtr(this);

 Invalidate();
 formula.reset(nullptr);
 raw_text.clear();
 rendered_text.clear();
 dep_ptrs.clear();
}
