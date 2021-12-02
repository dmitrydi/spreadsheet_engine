#pragma once

#include "common.h"
#include "imp_cell.h"
#include "virtual_cells.h"
#include <unordered_map>
#include <unordered_set>
#include "utility.h"

class ImpSheet: public ISheet {
public:
  ImpSheet() = default;
  void SetCell(Position pos, std::string text) override;
  const ICell* GetCell(Position pos) const override;
  ICell* GetCell(Position pos) override;
  void ClearCell(Position pos) override;
  void InsertRows(int before, int count = 1) override;
  void InsertCols(int before, int count = 1) override;
  void DeleteRows(int first, int count = 1) = 0;
  void DeleteCols(int first, int count = 1) = 0;
  Size GetPrintableSize() const override;
  void PrintValues(std::ostream& output) const override;
  void PrintTexts(std::ostream& output) const override;
private:
  using Row = std::vector<std::unique_ptr<ImpCell>>;
  std::vector<Row> cells;
  Size printable_size = {0,0};
  VirtualCells virtual_cells;

  constexpr static const Position AbsMaxPos{Position::kMaxRows, Position::kMaxCols};
  constexpr static const Position AbsMinPos{-1, -1};

  Position LTC = AbsMaxPos; // Left Top Corner
  Position RBC = AbsMinPos; // Right Bottom Corner

  struct BadInnerPosition {
    std::string text;
  };

  Position GetInnerPosition(const Position& pos) const {
    if (!pos.IsValid() || pos < LTC || RBC < pos)
      throw BadInnerPosition{pos.ToString()};
    return {pos.row - LTC.row, pos.col - LTC.col};
  }

  bool FormulaHasCircularRefs(Position current_pos, const std::unique_ptr<ImpFormula>& f) const;

  ImpCell* GetImpCell(Position pos);
  const ImpCell* GetImpCell(Position pos) const;

  ImpCell* GetVirtualCell(Position pos) {
    return virtual_cells.GetCell(pos);
  }

  ImpCell* CreateEmptyCell(Position pos) {
    virtual_cells.SetCell(pos);
    return virtual_cells.GetCell(pos);
  }

  ImpCell* CreateNewCell(Position pos);

  std::unique_ptr<ImpCell>& GetUptr(Position pos);
  const std::unique_ptr<ImpCell>& GetConstUptr(Position pos) const;

  using Graph = std::unordered_map<Position, std::unordered_set<Position, PosHasher>, PosHasher>;

  mutable Graph dependency_graph;

  void PopulateFormulaCells(const ImpFormula::UNode& root);
  void PopulateNode(ImpFormula::UNode& root);
  void PopulateFormulaPtrs(std::unique_ptr<ImpFormula>& formula);
  void PopulateCellDependencies(Position pos);
  void PopulateCellPtrs(ImpCell* cell_ptr);

  void AddDependentCell(const Position& to_cell, const Position& pos);
  void RemoveDependentCell(const Position& from_cell, const Position& pos);
  void InvalidateCell(Position pos);
};
