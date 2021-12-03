#include "SheetTester.h"

using namespace std;

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
    ASSERT_EQUAL(sheet->GetPrintableSize().rows, 1);
    ASSERT_EQUAL(sheet->GetPrintableSize().cols, 1);
    ASSERT_EQUAL(sheet->LTC.ToString(), "A1");
    ASSERT_EQUAL(sheet->RBC.ToString(), "C3");
    ASSERT_EQUAL(sheet->printable_LTC.ToString(), "C3");
    ASSERT_EQUAL(sheet->printable_RBC.ToString(), "C3");

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
