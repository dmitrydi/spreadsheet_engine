#pragma once

#include "imp_sheet.h"
#include "test_runner.h"
#include "utility.h"

struct SheetTester {
  void TestAll();
  void TestPositionFromString();
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

  static void PrintGraph(const ImpSheet::Graph& gr);
  void TestFormulaHasCircularRefs();
  // changing cells tests
  static bool GraphsEqual(const ImpSheet::Graph& lhs, const ImpSheet::Graph& rhs);
  void TestDepGraphChange();
  void TestRefGraphChange();
  void TestInvalidationAtChange();
  void TestNoInvalidation();

  // insertion tests
  static std::vector<Position> GetNodeRefs(AstNode* root);
  static bool VectorAndSetEqual(const std::vector<Position>& vpos, const std::unordered_set<Position, PosHasher>& spos);
  void TestInsertRows();
  void TestInsertCols();

  // expression tests
  void TestGetExpression();


};
