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
      return 0.; // throw here is better
  }
  if (state == CellState::Invalid) {
    rendered_text = RenderText();
    auto double_val = MaybeGetDouble(raw_text);
    if (double_val)
      cached_value = *double_val;
    else
      cached_value = rendered_text;
    state = CellState::Valid;
  }
  return cached_value;
}

optional<double> ImpCell::MaybeGetDouble(const string& str) {
  if (str.empty())
    return 0.; // due to convention
  double result;
  std::istringstream i(str);
  i >> result;
  if (!i.fail() && i.eof())
    return result;
  return nullopt;
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
  // dependencies are invalidated in sheet
 Invalidate();
 formula.reset(nullptr);
 raw_text.clear();
 rendered_text.clear();
 dep_ptrs.clear();
}
