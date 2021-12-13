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

  constexpr static const Position AbsMaxPos{Position::kMaxRows, Position::kMaxCols};
  constexpr static const Position AbsMinPos{-1, -1};

  Position LTC = AbsMaxPos; // Left Top Corner
  Position RBC = AbsMinPos; // Right Bottom Corner
  Position printable_LTC = AbsMaxPos;
  Position printable_RBC = AbsMinPos;

  std::pair<int, int> GetInsertPosition(Position pos) const;
  void ExpandPrintableArea(Position pos);
  std::optional<int> FirstNonzeroElement(int idx) const;
  std::optional<int> LastNonzeroElement(int idx) const;
  std::pair<int,int> FindTopOffset() const;
  std::pair<int, int> FindBottomOffset() const;
  void SqueezePrintableArea(Position deleted_position);
  void DeleteCell(Position pos);
  void ResetCell(Position pos);
  ImpCell* CreateEmptyCell(Position pos);
  void CreateCell(Position pos, std::string text, std::unique_ptr<ImpFormula>&& formula);


  ImpCell* GetImpCell(Position pos);
  const ImpCell* GetImpCell(Position pos) const;

  Position GetInnerPosition(Position pos) const;

  using Graph = std::unordered_map<Position, std::unordered_set<Position, PosHasher>, PosHasher>;

  static void PrintValue(std::ostream& os, ICell::Value val);
  static bool RowHasPrintableCells(const Row& row);

  mutable Graph reference_graph;
  mutable Graph dependency_graph;
  bool FormulaHasCircularRefs(Position current_pos, const std::unique_ptr<ImpFormula>& f) const;
  static bool GraphIsCircular(const Graph& graph, const Position start_vertex);
  void InvalidateDependencyGraph(Position pos);
  void UpdateReferenceGraph(Position pos);
  void UpdateDependencyGraph(Position pos);
  void RemoveDependencies(Position pos);

  // insertion
  void CheckCellsSize(int row_count, int col_count) const;
  void CheckFormulas(int row_count, int col_count) const;
  void CreateRows(int before, int count);
  void UpdateFormulasByRowInsertion(int before, int count);
  void UpdateGraphsByRowInsertion(int before, int count);
  void UpdateFormulasByColInsertion(int before, int count);
  void UpdateGraphsByColInsertion(int before, int count);
  void CreateCols(int before, int count);

  static void UpdateListByRow(std::unordered_set<Position, PosHasher>& dep_cells, int before, int count);
  static void UpdateKeysByRow(Graph& gr, int before, int count);
  static void UpdateListByCol(std::unordered_set<Position, PosHasher>& dep_cells, int before, int count);
  static void UpdateKeysByCol(Graph& gr, int before, int count);

  // deletion
  void UpdateFormulasByRowDeletion(int first,int count);
  void UpdateGraphsBuRowDeletion(int first, int count);
  void EraseRows(int first, int count);
  void UpdateFoumulasByColDeletion(int first, int count);
  void UpdateGraphsByColDeletion(int first, int count);
  void EraseCols(int first, int count);

  static void DeleteRowsFromGraph(Graph& gr, int first, int count);
  static void DeleteColsFromGraph(Graph& gr, int first, int count);

  enum class Color {
    white,
    gray,
    black
  };

  void PopulateFormulaPtrs(std::unique_ptr<ImpFormula>& formula, Position formula_position);

};

std::unique_ptr<ImpSheet> CreateImpSheet();
