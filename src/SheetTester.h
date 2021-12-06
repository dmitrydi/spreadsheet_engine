#pragma once

#include "imp_sheet.h"
#include "test_runner.h"
#include "utility.h"

struct SheetTester {
  void TestAll();

  /*
  pair<int, int> GetInsertPosition(Position pos);
void ExpandPrintableArea(Position pos);
optional<int> FirstNonzeroElement(int idx);
optional<int> LastNonzeroElement(int idx);
pair<int,int> FindTopOffset();
pair<int, int> FindBottomOffset();
void SqueezePrintableArea(Position deleted_position);
void DeleteCell(Position pos);
void ClearCell(Position pos);
ImpCell* CreateEmptyCell(Position pos);
void CreateCell(Position pos, string text, unique_ptr<ImpFormula>&& formula);
void InvalidateDependencyGraph(Position pos);
void UpdateDependencyGraph(Position pos);
   */
  void TestGetInsertPosition();
  void TestExpandPrintableArea();
  void TestFirstNonzeroElement();
  void TestLastNonzeroElement();
  void TestFindTopOffset();
  void TestFindBottomOffset();
  void TestSqueezePrintableArea();
  void TestDeleteCell();
  void TestClearCell();
  void TestCreateEmptyCell();
  void TestCreateCell();
  void TestInvalidateDependencyGraph();
  void TestUpdateDependencyGraph();

  void TestCreateNewCell();

};
