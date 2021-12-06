#include "SheetTester.h"

using namespace std;

void SheetTester::TestAll() {
  TestGetInsertPosition();
  TestCreateEmptyCell();
  TestSqueezePrintableArea();
  TestCreateNewCell();
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
