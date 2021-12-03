#pragma once

#include "common.h"
#include "imp_cell.h"
#include <unordered_map>
#include <unordered_set>
#include "utility.h"
#include <cassert>

class SheetTester;

class ImpSheet: public ISheet {
public:
  ImpSheet() = default;
  void SetCell(Position pos, std::string text) override;
  const ICell* GetCell(Position pos) const override;
  ICell* GetCell(Position pos) override;
  void ClearCell(Position pos) override;
  void InsertRows(int before, int count = 1) override;
  void InsertCols(int before, int count = 1) override;
  void DeleteRows(int first, int count = 1) override;
  void DeleteCols(int first, int count = 1) override;
  Size GetPrintableSize() const override;
  void PrintValues(std::ostream& output) const override;
  void PrintTexts(std::ostream& output) const override;
private:
  friend class SheetTester;
  using Row = std::vector<std::unique_ptr<ImpCell>>;
  std::vector<Row> cells;
  Size printable_size = {0,0};

  constexpr static const Position AbsMaxPos{Position::kMaxRows, Position::kMaxCols};
  constexpr static const Position AbsMinPos{-1, -1};

  Position LTC = AbsMaxPos; // Left Top Corner
  Position RBC = AbsMinPos; // Right Bottom Corner
  Position printable_LTC = AbsMaxPos;
  Position printable_RBC = AbsMinPos;

  Position GetInnerPosition(Position pos) const {
    if (!pos.IsValid())
      throw InvalidPositionException{pos.ToString()};
    return {pos.row - LTC.row, pos.col - LTC.col};
  }
  ImpCell* GetImpCell(Position pos);
  const ImpCell* GetImpCell(Position pos) const;

  void ResetCell(Position pos);

  void MaybeResizePrintableArea(Position deleted_pos);
  void MaybeExpandPrintableArea(Position pos) {
    if (pos.row < printable_LTC.row)
      printable_LTC.row = pos.row;
    if (pos.col < printable_LTC.col)
      printable_LTC.col = pos.col;
    if (printable_RBC.row < pos.row)
      printable_RBC.row = pos.row;
    if (printable_RBC.col < pos.col)
      printable_RBC.col = pos.col;
  }
  ImpCell* CreateNewCell(Position pos, bool resize_printable_area);

  std::unique_ptr<ImpCell>& GetUptr(Position pos);
  const std::unique_ptr<ImpCell>& GetConstUptr(Position pos) const;

  using Graph = std::unordered_map<Position, std::unordered_set<Position, PosHasher>, PosHasher>;

  mutable Graph dependency_graph;
  bool FormulaHasCircularRefs(Position current_pos, const std::unique_ptr<ImpFormula>& f) const;
  static bool GraphIsCircular(const Graph& graph, const Position start_vertex);

  enum class Color {
    white,
    gray,
    black
  };

  void PopulateFormulaPtrs(std::unique_ptr<ImpFormula>& formula, Position formula_position);

  void PopulateFormulaCells(const ImpFormula::UNode& root);
  void PopulateNode(ImpFormula::UNode& root);

  void PopulateCellDependencies(Position pos);
  void PopulateCellPtrs(ImpCell* cell_ptr);

  void AddDependentCell(const Position& to_cell, const Position& pos);
  void RemoveDependentCell(const Position& from_cell, const Position& pos);
  void InvalidateCell(Position pos);
};

std::unique_ptr<ImpSheet> CreateImpSheet();
