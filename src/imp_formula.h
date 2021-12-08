#pragma once

#include "formula.h"
#include "ast.h"
#include "utility.h"
#include <variant>
#include <algorithm>
#include <stack>
#include "antlr4-runtime.h"
#include "FormulaLexer.h"
#include "FormulaCustomListener.h"
#include "FormulaParser.h"

class ImpCell;
class ImpSheet;
class SheetTester;

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
  static void VisitCells(const Unode& root, std::vector<Position>& positions);
private:
  friend class ImpCell;
  friend class ImpSheet;
  friend class SheetTester;
  friend std::unique_ptr<ImpFormula> ParseImpFormula(std::string expression);
  using UNode = std::unique_ptr<AstNode>;
  UNode ast = nullptr;
  std::vector<Position> ref_cells; //always sorted
  mutable Value cached_val;
  mutable bool valid = false;
  void MoveAstRowsByInsertion(int from, int count);
  void MoveAstColsByInsertion(int from, int count);
  void MoveAstRowsByDeletion(int from, int count);
  void MoveAstColsByDeletion(int from, int count);

};

std::unique_ptr<ImpFormula> ParseImpFormula(std::string expression);
