#include "SheetTester.h"

using namespace std;

void SheetTester::TestAll() {
  TestPositionFromString();
  TestGetInsertPosition();
  TestCreateEmptyCell();
  TestSqueezePrintableArea();
  TestCreateNewCell();
  TestFormulaHasCircularRefs();
  TestDepGraphChange();
  TestRefGraphChange();
  TestInvalidationAtChange();
  TestNoInvalidation();

//  TestExpandPrintableArea();
//  TestFirstNonzeroElement();
//  TestLastNonzeroElement();
//  TestFindTopOffset();
//  TestFindBottomOffset();

//  TestDeleteCell();
//  TestClearCell();

//  TestCreateCell();
//  TestInvalidateDependencyGraph();
//  TestUpdateDependencyGraph();
 // throw;
}

void SheetTester::TestPositionFromString() {
  auto TestPositionFromString_1 = []() {
    ASSERT(!Position::FromString("X0").IsValid());
    ASSERT(!Position::FromString("ABCD1").IsValid());
    ASSERT(!Position::FromString("A123456").IsValid());
    ASSERT(!Position::FromString("ABCDEFGHIJKLMNOPQRS1234567890").IsValid());
    ASSERT(!Position::FromString("XFD16385").IsValid());
    ASSERT(!Position::FromString("XFE16384").IsValid());
    ASSERT(!Position::FromString("R2D2").IsValid());
  };

  TestRunner tr;

  RUN_TEST(tr, TestPositionFromString_1);
  //throw;
  /*
   *     try_formula("=X0");
    try_formula("=ABCD1");
    try_formula("=A123456");
    try_formula("=ABCDEFGHIJKLMNOPQRS1234567890");
    try_formula("=XFD16385");
    try_formula("=XFE16384");
    try_formula("=R2D2");
   */

}

void SheetTester::TestGetInsertPosition() {
  TestRunner tr;

  auto TestGetInsertPosition_Empty = []() {
    auto sheet = CreateImpSheet();
    auto [ins_row, ins_col] = sheet->GetInsertPosition("B5"_ppos);
    auto ins_pos = Position{ins_row, ins_col};
    auto expected = Position{0,0};
    ASSERT_EQUAL(ins_pos.ToString(), expected.ToString());
  };

  auto TestGetInsertPosition_A1IsSet = []() {
    auto sheet = CreateImpSheet();
    sheet->SetCell("A1"_ppos, "t");
    auto [ins_row, ins_col] = sheet->GetInsertPosition("A1"_ppos);
    auto ins_pos = Position{ins_row, ins_col};
    auto expected = Position{0,0};
    ASSERT_EQUAL(ins_pos.ToString(), expected.ToString());
  };

  auto TestGetInsertPosition_ChekAfter = []() {
    auto sheet = CreateImpSheet();
    sheet->SetCell("C3"_ppos, "t");
    {
      auto [ins_row, ins_col] = sheet->GetInsertPosition("C3"_ppos);
      auto ins_pos = Position{ins_row, ins_col};
      auto expected = Position{0,0};
      ASSERT_EQUAL(ins_pos.ToString(), expected.ToString());
    }
    {
      auto ins_pos = sheet->GetInsertPosition("E3"_ppos);
      ASSERT_EQUAL(ins_pos, (make_pair<int, int>(0,2)));
      ins_pos = sheet->GetInsertPosition("C4"_ppos);
      ASSERT_EQUAL(ins_pos, (make_pair<int,int>(1,0)));
      ins_pos = sheet->GetInsertPosition("D4"_ppos);
      ASSERT_EQUAL(ins_pos, (make_pair<int,int>(1,1)));
      ins_pos = sheet->GetInsertPosition("E4"_ppos);
      ASSERT_EQUAL(ins_pos, (make_pair<int,int>(1,2)));
    }

  };

  auto TestGetInsertPosition_ChekBefore = []() {
    auto sheet = CreateImpSheet();
    sheet->SetCell("F5"_ppos, "t");
    auto ins_pos = sheet->GetInsertPosition("F3"_ppos);
    ASSERT_EQUAL(ins_pos, (make_pair<int,int>(-2,0)));
    ASSERT_EQUAL(sheet->GetInsertPosition("C3"_ppos), (make_pair<int,int>(-2,-3)));
  };

  RUN_TEST(tr, TestGetInsertPosition_Empty);
  RUN_TEST(tr, TestGetInsertPosition_A1IsSet);
  RUN_TEST(tr, TestGetInsertPosition_ChekAfter);
  RUN_TEST(tr, TestGetInsertPosition_ChekBefore);
}


void SheetTester::TestCreateEmptyCell() {
  TestRunner tr;

  auto TestCreateEmptyCell_SingleCell = []() {
    auto sheet = CreateImpSheet();
    auto ptr = sheet->CreateEmptyCell("C3"_ppos);
    ASSERT(ptr);
    ASSERT_EQUAL(sheet->LTC.ToString(), "C3");
    ASSERT_EQUAL(sheet->RBC.ToString(), "C3");
    ASSERT_EQUAL(sheet->cells.size(), 1u);
    ASSERT_EQUAL(sheet->cells[0].size(), 1u);
  };

  auto TestCreateEmptyCell_CellsAfter = []() {
    auto sheet = CreateImpSheet();
    auto ptr = sheet->CreateEmptyCell("C3"_ppos);
    ASSERT(ptr);
    ptr = sheet->CreateEmptyCell("E4"_ppos);
    ASSERT(ptr);
    ASSERT_EQUAL(sheet->LTC.ToString(), "C3");
    ASSERT_EQUAL(sheet->RBC.ToString(), "E4");
    ASSERT_EQUAL(sheet->cells.size(), 2u);
    ASSERT_EQUAL(sheet->cells[1].size(), 3u);
  };

  auto TestCreateEmptyCell_CellsBefore = []() {
    auto sheet = CreateImpSheet();
    auto ptr1 = sheet->CreateEmptyCell("C3"_ppos);
    ASSERT(ptr1);
    auto ptr2 = sheet->CreateEmptyCell("C1"_ppos);
    ASSERT(ptr2);
    ASSERT_EQUAL(sheet->LTC.ToString(), "C1");
    ASSERT_EQUAL(sheet->RBC.ToString(), "C3");
    ASSERT_EQUAL(sheet->cells.size(), 3u);
    ASSERT_EQUAL(sheet->GetImpCell("C3"_ppos), ptr1);
    ASSERT_EQUAL(sheet->GetImpCell("C1"_ppos), ptr2);
    ASSERT_EQUAL(sheet->cells[0].size(), 1u);
    ASSERT_EQUAL(sheet->cells[1].size(), 0u);
    ASSERT_EQUAL(sheet->cells[2].size(), 1u);
    auto ptr3 = sheet->CreateEmptyCell("A1"_ppos);
    ASSERT_EQUAL(ptr3, sheet->GetImpCell("A1"_ppos));
    ASSERT_EQUAL(sheet->LTC.ToString(), "A1");
    ASSERT_EQUAL(sheet->RBC.ToString(), "C3");
    ASSERT_EQUAL(sheet->cells[0].size(), 3u);
    ASSERT_EQUAL(sheet->cells[1].size(), 0u);
    ASSERT_EQUAL(sheet->cells[2].size(), 3u);
    ASSERT(!sheet->GetImpCell("A2"_ppos));
    ASSERT(!sheet->GetImpCell("B2"_ppos));
    ASSERT(!sheet->GetImpCell("C2"_ppos));
    ASSERT(!sheet->GetImpCell("A3"_ppos));
    ASSERT(!sheet->GetImpCell("B3"_ppos));
  };

  RUN_TEST(tr, TestCreateEmptyCell_SingleCell);
  RUN_TEST(tr, TestCreateEmptyCell_CellsAfter);
  RUN_TEST(tr, TestCreateEmptyCell_CellsBefore);
}


void SheetTester::TestSqueezePrintableArea() {
  TestRunner tr;

  auto TestSqueezePrintableArea_ResetOne = []() {
    auto sheet = CreateImpSheet();
    sheet->SetCell("A1"_ppos, "");
    auto ptr_A1 = sheet->GetCell("A1"_ppos);
    ASSERT(ptr_A1);
    ASSERT(ptr_A1->GetText().empty());
    sheet->SetCell("B2"_ppos, "t");
    sheet->SetCell("C3"_ppos, "t");
    ASSERT_EQUAL(sheet->LTC.ToString(), "A1");
    ASSERT_EQUAL(sheet->RBC.ToString(), "C3");
    ASSERT_EQUAL(sheet->printable_LTC.ToString(), "B2");
    ASSERT_EQUAL(sheet->printable_RBC.ToString(), "C3");
    sheet->SetCell("B2"_ppos, "");
    ASSERT_EQUAL(sheet->printable_LTC.ToString(), "C3");
    ASSERT_EQUAL(sheet->printable_RBC.ToString(), "C3");
    ASSERT_EQUAL(sheet->GetPrintableSize().rows, 1);
    ASSERT_EQUAL(sheet->GetPrintableSize().cols, 1);
  };

  RUN_TEST(tr, TestSqueezePrintableArea_ResetOne);
}

void SheetTester::TestCreateNewCell() {
  TestRunner tr;


  auto TestInit = [](){
    auto sheet = CreateImpSheet();
    ASSERT(sheet.get());
  };

  auto TestPrintableSizeInit = []() {
    auto sheet = CreateImpSheet();
    auto ps = sheet->GetPrintableSize();
    ASSERT_EQUAL(ps.rows, 0);
    ASSERT_EQUAL(ps.cols, 0);
  };

  auto TestInsertCells = []() {
    auto sheet = CreateImpSheet();
    sheet->SetCell("A1"_ppos, "");
    auto ps = sheet->GetPrintableSize();
    ASSERT_EQUAL(ps.rows, 0);
    ASSERT_EQUAL(ps.cols, 0);

    sheet->SetCell("B2"_ppos, "text");
    ps = sheet->GetPrintableSize();
    ASSERT_EQUAL(ps.rows, 1);
    ASSERT_EQUAL(ps.cols, 1);
    ASSERT_EQUAL(sheet->LTC.ToString(), "A1");
    ASSERT_EQUAL(sheet->RBC.ToString(), "B2");
    ASSERT_EQUAL(sheet->printable_LTC.ToString(), "B2");
    ASSERT_EQUAL(sheet->printable_RBC.ToString(), "B2");

    sheet->SetCell("C3"_ppos, "text");
    ASSERT_EQUAL(sheet->GetPrintableSize().rows, 2);
    ASSERT_EQUAL(sheet->GetPrintableSize().cols, 2);
    ASSERT_EQUAL(sheet->LTC.ToString(), "A1");
    ASSERT_EQUAL(sheet->RBC.ToString(), "C3");
    ASSERT_EQUAL(sheet->printable_LTC.ToString(), "B2");
    ASSERT_EQUAL(sheet->printable_RBC.ToString(), "C3");

    sheet->SetCell("B2"_ppos, "");
    ASSERT_EQUAL(sheet->printable_LTC.ToString(), "C3");
    ASSERT_EQUAL(sheet->printable_RBC.ToString(), "C3");
    ASSERT_EQUAL(sheet->GetPrintableSize().rows, 1);
    ASSERT_EQUAL(sheet->GetPrintableSize().cols, 1);
    ASSERT_EQUAL(sheet->LTC.ToString(), "A1");
    ASSERT_EQUAL(sheet->RBC.ToString(), "C3");


    sheet->SetCell("C3"_ppos, "");
    ASSERT_EQUAL(sheet->GetPrintableSize().rows, 0);
    ASSERT_EQUAL(sheet->GetPrintableSize().cols, 0);
    ASSERT_EQUAL(sheet->LTC.ToString(), "A1");
    ASSERT_EQUAL(sheet->RBC.ToString(), "C3");
    ASSERT_EQUAL(sheet->printable_LTC.ToString(), ImpSheet::AbsMaxPos.ToString());
    ASSERT_EQUAL(sheet->printable_RBC.ToString(), ImpSheet::AbsMinPos.ToString());
  };

  auto ResetLTC = []() {
    auto sheet = CreateImpSheet();
    sheet->SetCell("A1"_ppos, "t");
    sheet->SetCell("A2"_ppos, "t");
    sheet->SetCell("B1"_ppos, "t");
    sheet->SetCell("B3"_ppos, "t");
    ASSERT_EQUAL(sheet->GetPrintableSize().rows, 3);
    ASSERT_EQUAL(sheet->GetPrintableSize().cols, 2);
    sheet->SetCell("A1"_ppos, "");
    ASSERT_EQUAL(sheet->GetPrintableSize().rows, 3);
    ASSERT_EQUAL(sheet->GetPrintableSize().cols, 2);
    ASSERT_EQUAL(sheet->printable_LTC.ToString(), "A1");
    ASSERT_EQUAL(sheet->printable_RBC.ToString(), "B3");
    sheet->SetCell("B1"_ppos, "");
    ASSERT_EQUAL(sheet->GetPrintableSize().rows, 2);
    ASSERT_EQUAL(sheet->GetPrintableSize().cols, 2);
    ASSERT_EQUAL(sheet->printable_LTC.ToString(), "A2");
    ASSERT_EQUAL(sheet->printable_RBC.ToString(), "B3");
    sheet->SetCell("A2"_ppos, "");
    ASSERT_EQUAL(sheet->GetPrintableSize().rows, 1);
    ASSERT_EQUAL(sheet->GetPrintableSize().cols, 1);
    ASSERT_EQUAL(sheet->printable_LTC.ToString(), "B3");
    ASSERT_EQUAL(sheet->printable_RBC.ToString(), "B3");
    sheet->SetCell("B3"_ppos, "");
    ASSERT_EQUAL(sheet->GetPrintableSize().rows, 0);
    ASSERT_EQUAL(sheet->GetPrintableSize().cols, 0);
    ASSERT_EQUAL(sheet->printable_LTC.ToString(), ImpSheet::AbsMaxPos.ToString());
    ASSERT_EQUAL(sheet->printable_RBC.ToString(), ImpSheet::AbsMinPos.ToString());
    ASSERT_EQUAL(sheet->LTC.ToString(), "A1");
    ASSERT_EQUAL(sheet->RBC.ToString(), "B3");
    // setting back
    sheet->SetCell("B3"_ppos, "t");
    ASSERT_EQUAL(sheet->GetPrintableSize().rows, 1);
    ASSERT_EQUAL(sheet->GetPrintableSize().cols, 1);
    ASSERT_EQUAL(sheet->printable_LTC.ToString(), "B3");
    ASSERT_EQUAL(sheet->printable_RBC.ToString(), "B3");
    sheet->SetCell("B1"_ppos, "t");
    ASSERT_EQUAL(sheet->GetPrintableSize().rows, 3);
    ASSERT_EQUAL(sheet->GetPrintableSize().cols, 1);
    ASSERT_EQUAL(sheet->printable_LTC.ToString(), "B1");
    ASSERT_EQUAL(sheet->printable_RBC.ToString(), "B3");
    sheet->SetCell("A1"_ppos, "text");
    ASSERT_EQUAL(sheet->GetPrintableSize().rows, 3);
    ASSERT_EQUAL(sheet->GetPrintableSize().cols, 2);
    ASSERT_EQUAL(sheet->printable_LTC.ToString(), "A1");
    ASSERT_EQUAL(sheet->printable_RBC.ToString(), "B3");
  };

  auto ResetRBC = []() {
    auto sheet = CreateImpSheet();
    sheet->SetCell("A1"_ppos, "t");
    sheet->SetCell("A3"_ppos, "t");
    sheet->SetCell("B3"_ppos, "t");
    sheet->SetCell("B2"_ppos, "t");
    ASSERT_EQUAL(sheet->GetPrintableSize().rows, 3);
    ASSERT_EQUAL(sheet->GetPrintableSize().cols, 2);
    ASSERT_EQUAL(sheet->printable_LTC.ToString(), "A1");
    ASSERT_EQUAL(sheet->printable_RBC.ToString(), "B3");
    sheet->SetCell("B3"_ppos, "");
    ASSERT_EQUAL(sheet->GetPrintableSize().rows, 3);
    ASSERT_EQUAL(sheet->GetPrintableSize().cols, 2);
    ASSERT_EQUAL(sheet->printable_LTC.ToString(), "A1");
    ASSERT_EQUAL(sheet->printable_RBC.ToString(), "B3");
    sheet->SetCell("A3"_ppos, "");
    ASSERT_EQUAL(sheet->GetPrintableSize().rows, 2);
    ASSERT_EQUAL(sheet->GetPrintableSize().cols, 2);
    ASSERT_EQUAL(sheet->printable_LTC.ToString(), "A1");
    ASSERT_EQUAL(sheet->printable_RBC.ToString(), "B2");
    sheet->SetCell("B2"_ppos, "");
    ASSERT_EQUAL(sheet->GetPrintableSize().rows, 1);
    ASSERT_EQUAL(sheet->GetPrintableSize().cols, 1);
    ASSERT_EQUAL(sheet->printable_LTC.ToString(), "A1");
    ASSERT_EQUAL(sheet->printable_RBC.ToString(), "A1");
    sheet->SetCell("A1"_ppos, "");
    ASSERT_EQUAL(sheet->GetPrintableSize().rows, 0);
    ASSERT_EQUAL(sheet->GetPrintableSize().cols, 0);
    ASSERT_EQUAL(sheet->printable_LTC.ToString(), ImpSheet::AbsMaxPos.ToString());
    ASSERT_EQUAL(sheet->printable_RBC.ToString(), ImpSheet::AbsMinPos.ToString());
    // setting back
    sheet->SetCell("B3"_ppos, "t");
    ASSERT_EQUAL(sheet->GetPrintableSize().rows, 1);
    ASSERT_EQUAL(sheet->GetPrintableSize().cols, 1);
    ASSERT_EQUAL(sheet->printable_LTC.ToString(), "B3");
    ASSERT_EQUAL(sheet->printable_RBC.ToString(), "B3");
    sheet->SetCell("A1"_ppos, "t");
    ASSERT_EQUAL(sheet->GetPrintableSize().rows, 3);
    ASSERT_EQUAL(sheet->GetPrintableSize().cols, 2);
    ASSERT_EQUAL(sheet->printable_LTC.ToString(), "A1");
    ASSERT_EQUAL(sheet->printable_RBC.ToString(), "B3");
  };

  auto TestDeleteLTCCol = []() {
    auto sheet = CreateImpSheet();
    sheet->SetCell("B3"_ppos, "t");
    sheet->SetCell("C3"_ppos, "t");
    sheet->SetCell("D2"_ppos, "t");
    ASSERT_EQUAL(sheet->LTC.ToString(), "B2");
    ASSERT_EQUAL(sheet->RBC.ToString(), "D3");
    ASSERT_EQUAL(sheet->printable_LTC.ToString(), "B2");
    ASSERT_EQUAL(sheet->printable_RBC.ToString(), "D3");
    sheet->SetCell("D4"_ppos, "t");
    ASSERT_EQUAL(sheet->GetPrintableSize().rows, 3);
    ASSERT_EQUAL(sheet->GetPrintableSize().cols, 3);
    ASSERT_EQUAL(sheet->LTC.ToString(), "B2");
    ASSERT_EQUAL(sheet->RBC.ToString(), "D4");
    ASSERT_EQUAL(sheet->printable_LTC.ToString(), "B2");
    ASSERT_EQUAL(sheet->printable_RBC.ToString(), "D4");
    sheet->SetCell("E3"_ppos, "t");
    ASSERT_EQUAL(sheet->GetPrintableSize().rows, 3);
    ASSERT_EQUAL(sheet->GetPrintableSize().cols, 4);
    ASSERT_EQUAL(sheet->LTC.ToString(), "B2");
    ASSERT_EQUAL(sheet->RBC.ToString(), "E4");
    ASSERT_EQUAL(sheet->printable_LTC.ToString(), "B2");
    ASSERT_EQUAL(sheet->printable_RBC.ToString(), "E4");
    sheet->SetCell("B3"_ppos, "");
    ASSERT_EQUAL(sheet->printable_LTC.ToString(), "C2");
    ASSERT_EQUAL(sheet->printable_RBC.ToString(), "E4");
    sheet->SetCell("C3"_ppos, "");
    ASSERT_EQUAL(sheet->printable_LTC.ToString(), "D2");
    ASSERT_EQUAL(sheet->printable_RBC.ToString(), "E4");
    sheet->SetCell("D4"_ppos, "");
    ASSERT_EQUAL(sheet->printable_LTC.ToString(), "D2");
    ASSERT_EQUAL(sheet->printable_RBC.ToString(), "E3");
    sheet->SetCell("D2"_ppos, "");
    ASSERT_EQUAL(sheet->printable_LTC.ToString(), "E3");
    ASSERT_EQUAL(sheet->printable_RBC.ToString(), "E3");
    sheet->SetCell("E3"_ppos, "");
    ASSERT_EQUAL(sheet->printable_LTC.ToString(), ImpSheet::AbsMaxPos.ToString());
    ASSERT_EQUAL(sheet->printable_RBC.ToString(), ImpSheet::AbsMinPos.ToString());
    ASSERT_EQUAL(sheet->GetPrintableSize().rows, 0);
    ASSERT_EQUAL(sheet->GetPrintableSize().cols, 0);
    sheet->SetCell("F3"_ppos, "");
    ASSERT_EQUAL(sheet->printable_LTC.ToString(), ImpSheet::AbsMaxPos.ToString());
    ASSERT_EQUAL(sheet->printable_RBC.ToString(), ImpSheet::AbsMinPos.ToString());
    ASSERT_EQUAL(sheet->GetPrintableSize().rows, 0);
    ASSERT_EQUAL(sheet->GetPrintableSize().cols, 0);
    ASSERT_EQUAL(sheet->LTC.ToString(), "B2");
    ASSERT_EQUAL(sheet->RBC.ToString(), "F4");
  };

  auto TestDeleteRBCCol = []() {
    auto sheet = CreateImpSheet();
    sheet->SetCell("E3"_ppos, "t");
    sheet->SetCell("D3"_ppos, "t");
    sheet->SetCell("C2"_ppos, "t");
    sheet->SetCell("C4"_ppos, "t");
    sheet->SetCell("B3"_ppos, "t");
    ASSERT_EQUAL(sheet->LTC.ToString(), "B2");
    ASSERT_EQUAL(sheet->RBC.ToString(), "E4");
    ASSERT_EQUAL(sheet->printable_LTC.ToString(), "B2");
    ASSERT_EQUAL(sheet->printable_RBC.ToString(), "E4");
    sheet->SetCell("E3"_ppos, "");
    ASSERT_EQUAL(sheet->printable_RBC.ToString(), "D4");
    sheet->SetCell("D3"_ppos, "");
    ASSERT_EQUAL(sheet->printable_RBC.ToString(), "C4");
    sheet->SetCell("C4"_ppos, "");
    ASSERT_EQUAL(sheet->printable_RBC.ToString(), "C3");
    sheet->SetCell("C2"_ppos, "");
    ASSERT_EQUAL(sheet->printable_RBC.ToString(), "B3");
    ASSERT_EQUAL(sheet->printable_LTC.ToString(), "B3");
    sheet->SetCell("B3"_ppos, "");
    ASSERT_EQUAL(sheet->printable_LTC.ToString(), ImpSheet::AbsMaxPos.ToString());
    ASSERT_EQUAL(sheet->printable_RBC.ToString(), ImpSheet::AbsMinPos.ToString());
  };


  RUN_TEST(tr, TestInit);
  RUN_TEST(tr, TestPrintableSizeInit);
  RUN_TEST(tr, TestInsertCells);
  RUN_TEST(tr, ResetLTC);
  RUN_TEST(tr, ResetRBC);
  RUN_TEST(tr, TestDeleteLTCCol);
  RUN_TEST(tr, TestDeleteRBCCol);

}

void SheetTester::PrintGraph(const ImpSheet::Graph& gr) {
  for (const auto& [curr_v, children]: gr) {
    cerr << curr_v.ToString() << ": ";
    for (const auto& ch: children)
      cerr << ch.ToString() << " ";
    cerr << endl;
  }
}

void SheetTester::TestFormulaHasCircularRefs() {
  auto TestFormulaHasCircularRefs_1 = []() {
    auto sheet = CreateImpSheet();

    sheet->SetCell("D7"_ppos, "=F15");
    sheet->SetCell("F15"_ppos, "=I11");
    sheet->SetCell("H7"_ppos, "=F10+D7+F5");
    sheet->SetCell("F5"_ppos, "1");
    sheet->SetCell("F10"_ppos, "1");
    try {
      sheet->SetCell("I11"_ppos, "1");
    } catch (CircularDependencyException& ex) {
      cerr << ex.what() << endl;
      sheet->SetCell("I11"_ppos, "1");
    }
    ASSERT_EQUAL(get<double>(sheet->GetCell("F5"_ppos)->GetValue()), 1.0);
    ASSERT_EQUAL(get<double>(sheet->GetCell("F10"_ppos)->GetValue()), 1.0);
    ASSERT_EQUAL(get<double>(sheet->GetCell("I11"_ppos)->GetValue()), 1.0);
    ASSERT_EQUAL(get<double>(sheet->GetCell("D7"_ppos)->GetValue()), 1.0);
    ASSERT_EQUAL(get<double>(sheet->GetCell("F15"_ppos)->GetValue()), 1.0);
    ASSERT_EQUAL(get<double>(sheet->GetCell("H7"_ppos)->GetValue()), 3.0);
  };

  auto TestFormulaHasCircularRefs_FromMain = []() {
    auto sheet = CreateImpSheet();
    sheet->SetCell("E2"_ppos, "=E4");
    sheet->SetCell("E4"_ppos, "=X9");
    sheet->SetCell("X9"_ppos, "=M6");
    sheet->SetCell("M6"_ppos, "Ready");
    bool caught = false;
    try {
      sheet->SetCell("M6"_ppos, "=E2");
    } catch(CircularDependencyException& ex) {
      caught = true;
    }
    ASSERT(caught);
    ASSERT_EQUAL(sheet->GetCell("M6"_ppos)->GetText(), "Ready");
    auto val = sheet->GetCell("E2"_ppos)->GetValue();
    ASSERT(holds_alternative<FormulaError>(val));
  };

  TestRunner tr;

  RUN_TEST(tr, TestFormulaHasCircularRefs_1);
  RUN_TEST(tr, TestFormulaHasCircularRefs_FromMain);
}

bool SheetTester::GraphsEqual(const ImpSheet::Graph& lhs, const ImpSheet::Graph& rhs) {
  if (lhs.size() != rhs.size())
    return false;
  for (const auto& [p, sp]: lhs) {
    if (!rhs.count(p))
      return false;
    if (lhs.at(p).size() != rhs.at(p).size())
      return false;
    for (const auto& pp: sp)
      if (!rhs.at(p).count(pp))
        return false;
  }
  return true;
}

void SheetTester::TestDepGraphChange() {
  TestRunner tr;
  auto TestDepGraphChange_SingleCell = []() {
    auto sheet = CreateImpSheet();
    sheet->SetCell("J4"_ppos, "1");
    ASSERT(sheet->dependency_graph.empty());
    sheet->SetCell("I4"_ppos, "=J4");
    ImpSheet::Graph exp_dep_gr;
    exp_dep_gr["J4"_ppos].insert("I4"_ppos);
    ASSERT(GraphsEqual(sheet->dependency_graph, exp_dep_gr));
  };

  auto TestDepGraphChange_GraphToEmpty = []() {
    auto sheet = CreateImpSheet();
    sheet->SetCell("J4"_ppos, "1");
    sheet->SetCell("I4"_ppos, "=J4");
    ImpSheet::Graph exp_dep_gr;
    exp_dep_gr["J4"_ppos].insert("I4"_ppos);
    ASSERT(GraphsEqual(sheet->dependency_graph, exp_dep_gr));
    sheet->SetCell("I4"_ppos, "2");
    ASSERT(sheet->dependency_graph.at("J4"_ppos).empty());
  };

  auto TestDepGraphChange_ChangeDependency = []() {
    auto sheet = CreateImpSheet();
    sheet->SetCell("J4"_ppos, "1");
    sheet->SetCell("I4"_ppos, "=J4");
    ImpSheet::Graph exp_dep_gr;
    exp_dep_gr["J4"_ppos].insert("I4"_ppos);
    ASSERT(GraphsEqual(sheet->dependency_graph, exp_dep_gr));
    sheet->SetCell("I4"_ppos, "=J5");
    exp_dep_gr["J4"_ppos].erase("I4"_ppos);
    exp_dep_gr["J5"_ppos].insert("I4"_ppos);
    ASSERT(GraphsEqual(sheet->dependency_graph, exp_dep_gr));
  };


  auto TestDepGraphChange_StillDependent = []() {
    auto sheet = CreateImpSheet();
    sheet->SetCell("I2"_ppos, "=J3");
    sheet->SetCell("I3"_ppos, "=J3");
    sheet->SetCell("I4"_ppos, "=J3");
    ImpSheet::Graph exp_dep_gr;
    exp_dep_gr["J3"_ppos].insert("I2"_ppos);
    exp_dep_gr["J3"_ppos].insert("I3"_ppos);
    exp_dep_gr["J3"_ppos].insert("I4"_ppos);
    ASSERT(GraphsEqual(sheet->dependency_graph, exp_dep_gr));
    sheet->SetCell("J3"_ppos, "2");
    ASSERT(GraphsEqual(sheet->dependency_graph, exp_dep_gr));
  };

  RUN_TEST(tr, TestDepGraphChange_SingleCell);
  RUN_TEST(tr, TestDepGraphChange_GraphToEmpty);
  RUN_TEST(tr, TestDepGraphChange_ChangeDependency);
  RUN_TEST(tr, TestDepGraphChange_StillDependent);
}
void SheetTester::TestRefGraphChange() {
  auto TestRefGraphChange_SingleCell = []() {
    auto sheet = CreateImpSheet();
    sheet->SetCell("A1"_ppos, "=B2");
    ImpSheet::Graph exp_ref_gr;
    exp_ref_gr["A1"_ppos].insert("B2"_ppos);
    ASSERT(GraphsEqual(sheet->reference_graph, exp_ref_gr));
  };

  auto TestRefGraphChange_RemovedDependency = []() {
    auto sheet = CreateImpSheet();
    sheet->SetCell("A1"_ppos, "=B2");
    ImpSheet::Graph exp_ref_gr;
    exp_ref_gr["A1"_ppos].insert("B2"_ppos);
    ASSERT(GraphsEqual(sheet->reference_graph, exp_ref_gr));
    sheet->SetCell("A1"_ppos, "text");
    exp_ref_gr.erase("A1"_ppos);
    ASSERT(GraphsEqual(sheet->reference_graph, exp_ref_gr));
  };

  TestRunner tr;

  RUN_TEST(tr, TestRefGraphChange_SingleCell);
  RUN_TEST(tr, TestRefGraphChange_RemovedDependency);
}
void SheetTester::TestInvalidationAtChange() {
  auto TestInvalidationAtChange_1 = []() {
    auto sheet = CreateImpSheet();
    sheet->SetCell("F5"_ppos, "=G4+G6");
    sheet->SetCell("G4"_ppos, "=H3+H5");
    sheet->SetCell("G6"_ppos, "=H5+H7");
    sheet->SetCell("H3"_ppos, "1");
    sheet->SetCell("H5"_ppos, "1");
    sheet->SetCell("H7"_ppos, "1");
    ASSERT(!sheet->GetImpCell("F5"_ppos)->IsValid());
    ASSERT(!sheet->GetImpCell("G4"_ppos)->IsValid());
    ASSERT(!sheet->GetImpCell("G6"_ppos)->IsValid());
    auto val = sheet->GetCell("F5"_ppos)->GetValue();
    ASSERT_EQUAL(get<double>(val), 4.0);
    ASSERT(sheet->GetImpCell("F5"_ppos)->IsValid());
    ASSERT(sheet->GetImpCell("G4"_ppos)->IsValid());
    ASSERT(sheet->GetImpCell("G6"_ppos)->IsValid());
    sheet->SetCell("H3"_ppos, "0");
    ASSERT(!sheet->GetImpCell("F5"_ppos)->IsValid());
    ASSERT(!sheet->GetImpCell("G4"_ppos)->IsValid());
    ASSERT(sheet->GetImpCell("G6"_ppos)->IsValid());
    val = sheet->GetCell("F5"_ppos)->GetValue();
    ASSERT_EQUAL(get<double>(val), 3.0);
    ASSERT(sheet->GetImpCell("F5"_ppos)->IsValid());
    ASSERT(sheet->GetImpCell("G4"_ppos)->IsValid());
    ASSERT(sheet->GetImpCell("G6"_ppos)->IsValid());
    sheet->SetCell("H7"_ppos, "");
    ASSERT(!sheet->GetImpCell("F5"_ppos)->IsValid());
    ASSERT(sheet->GetImpCell("G4"_ppos)->IsValid());
    ASSERT(!sheet->GetImpCell("G6"_ppos)->IsValid());
    val = sheet->GetCell("F5"_ppos)->GetValue();
    ASSERT_EQUAL(get<double>(val), 2.0);
    ASSERT(sheet->GetImpCell("F5"_ppos)->IsValid());
    ASSERT(sheet->GetImpCell("G4"_ppos)->IsValid());
    ASSERT(sheet->GetImpCell("G6"_ppos)->IsValid());
    sheet->SetCell("H5"_ppos, "");
    ASSERT(!sheet->GetImpCell("F5"_ppos)->IsValid());
    ASSERT(!sheet->GetImpCell("G4"_ppos)->IsValid());
    ASSERT(!sheet->GetImpCell("G6"_ppos)->IsValid());
    val = sheet->GetCell("F5"_ppos)->GetValue();
    ASSERT_EQUAL(get<double>(val), 0);
    sheet->SetCell("G4"_ppos, "1");
    val = sheet->GetCell("F5"_ppos)->GetValue();
    ASSERT_EQUAL(get<double>(val), 1.0);
    sheet->SetCell("H3"_ppos, "1");
    ASSERT(sheet->GetImpCell("F5"_ppos)->IsValid());
    val = sheet->GetCell("F5"_ppos)->GetValue();
    ASSERT_EQUAL(get<double>(val), 1.0);
  };

  TestRunner tr;

  RUN_TEST(tr, TestInvalidationAtChange_1);
}
void SheetTester::TestNoInvalidation() {
  auto TestNoInvalidation_1 = []() {
    auto sheet = CreateImpSheet();
    sheet->SetCell("F5"_ppos, "=G4+G6");
    sheet->SetCell("G4"_ppos, "=H3+H5");
    sheet->SetCell("G6"_ppos, "=H5+H7");
    sheet->SetCell("H3"_ppos, "1");
    sheet->SetCell("H5"_ppos, "1");
    sheet->SetCell("H7"_ppos, "1");
    auto val = sheet->GetCell("F5"_ppos)->GetValue();
    ASSERT_EQUAL(get<double>(val), 4.0);
    sheet->SetCell("G4"_ppos, "2");
    val = sheet->GetCell("F5"_ppos)->GetValue();
    ASSERT_EQUAL(get<double>(val), 4.0);
    sheet->SetCell("H3"_ppos, "0");
    ASSERT(sheet->GetImpCell("F5"_ppos)->IsValid());
    val = sheet->GetCell("F5"_ppos)->GetValue();
    ASSERT_EQUAL(get<double>(val), 4.0);
  };

  TestRunner tr;
  RUN_TEST(tr, TestNoInvalidation_1);
}
