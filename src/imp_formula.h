#pragma once

#include "formula.h"
#include "ast.h"
#include "utility.h"
#include <variant>
#include <algorithm>
#include <stack>

class ImpCell;
class ImpSheet;

class ImpFormula: public IFormula {
public:
  using Value = IFormula::Value;
  ImpFormula() = default;
  Value Evaluate(const ISheet& sheet) const override;
  std::string GetExpression() const override;
  std::vector<Position> GetReferencedCells() const override;
  HandlingResult HandleInsertedRows(int before, int count = 1) override;
  HandlingResult HandleInsertedCols(int before, int count = 1) override;
  HandlingResult HandleDeletedRows(int first, int count = 1) override;
  HandlingResult HandleDeletedCols(int first, int count = 1) override;
  void Invalidate() {valid = false;}

private:
  friend class ImpCell;
  friend class ImpSheet;
  using UNode = std::unique_ptr<AstNode>;
  UNode ast = nullptr;
  // maybe move pointers from ast to a separate graph
  std::vector<Position> ref_cells; //always sorted
  std::unordered_set<ImpCell*> ref_ptrs;
  std::unordered_set<ImpCell*>& GetRefPtrs() {
    return ref_ptrs;
  }
  void PopulatePtrs();
  mutable Value cached_val;
  mutable bool valid = false;

};

std::unique_ptr<ImpFormula> ParseImpFormula(std::string expression);
