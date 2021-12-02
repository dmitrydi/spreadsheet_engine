#pragma once

#include "common.h"
#include "imp_cell.h"
#include <unordered_map>

struct VirtualCells {
  using VirtualRow = std::unordered_map<int, std::unique_ptr<ImpCell>>;
  std::unordered_map<int, VirtualRow> virtual_cells;
  Position LTC{Position::kMaxRows, Position::kMaxCols};
  Position RBC{-1, -1};
  size_t sz = 0;

  void SetCell(Position pos);
  const ImpCell* GetCell(Position pos) const;
  ImpCell* GetCell(Position pos);
  void ShiftIndices(int row_shift, int col_shift);
  std::unique_ptr<ImpCell> EjectVirtualCell(Position pos);
  void DeleteCell(Position pos);
  void InsertRows(int first, int count = 1);
  void InsertCols(int first, int count = 1);
  void DeleteRows(int first, int count = 1);
  void DeleteCols(int first, int count = 1);
  void MoveRow(int from, int to);
  void MoveCol(int from, int to);
  bool VcHasPos(Position pos) const;
  bool IsEmpty() const { return !sz; }
  bool IsShiftCorrect(int row_shift, int col_shift) const;
  std::vector<Position> GetCrossedCells(Position new_main_LTC, Position new_main_RBC) const;
};
