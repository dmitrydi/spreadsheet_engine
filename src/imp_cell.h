#pragma once

#include "common.h"
#include "imp_formula.h"

class ImpSheet;

enum class CellState {
  Valid,
  Invalid
};

class ImpCell: public ICell {
public:
  ImpCell() = default;
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
  mutable CellState state = CellState::Invalid;
  mutable Value cached_value;
  const ISheet *parent = nullptr;
  std::unique_ptr<ImpFormula> formula = nullptr;
  std::string raw_text;
  std::string rendered_text;
  std::unordered_set<ImpCell*> dep_ptrs; // dependent cells

  friend class ImpSheet;

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
