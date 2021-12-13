#pragma once

#include "common.h"
#include "imp_formula.h"
#include <optional>

class ImpSheet;
class SheetTester;

enum class CellState {
  Valid,
  Invalid
};

class ImpCell: public ICell {
public:
  ImpCell(const ISheet *parent): parent(parent) {};
  Value GetValue() const override;
  std::string GetText() const override;
  std::vector<Position> GetReferencedCells() const override;

  void SetText(const std::string text) {
    raw_text = text;
  }
  void SetFormula(std::unique_ptr<ImpFormula>&& f) {
    formula = std::move(f);
  }

  void Clear();
private:
  friend class ImpSheet;
  mutable CellState state = CellState::Invalid;
  mutable Value cached_value;
  const ISheet *parent = nullptr;
  std::unique_ptr<ImpFormula> formula = nullptr;
  std::string raw_text;
  mutable std::string rendered_text;
  std::unordered_set<ImpCell*> dep_ptrs; // dependent cells
  bool IsValid() const {
    return state == CellState::Valid;
  }

  static std::optional<double> MaybeGetDouble(const std::string& str);


  friend class SheetTester;

  std::string RenderText() const {
    if (!raw_text.empty()) {
      if (raw_text[0] == kEscapeSign)
        return {next(raw_text.begin()), raw_text.end()};
    }
    return raw_text;
  }

  void AddDepPtr(ImpCell* ptr) {
    dep_ptrs.insert(ptr);
  }
  void RemoveDepPtr(ImpCell* ptr) {
    dep_ptrs.erase(ptr);
  }

  void Invalidate() {
    state = CellState::Invalid;
  }
};
