#include "imp_sheet.h"

using namespace std;

void ImpSheet::SetCell(Position pos, string text) {
  if (!pos.IsValid())
    throw InvalidPositionException{pos.ToString()};
  if (GetCell(pos) && GetCell(pos)->GetText() == text)
    return;

  if (text.empty()) {
    if (GetCell(pos)) {
      ClearCell(pos);
    } else {
      if (GetVirtualCell(pos)) // cells already exists in virtual
        return; // do nothing
      CreateEmptyCell(pos); // create empty cell in virtual
    }
  } else {
    auto f = ParseImpFormula(text); // may parse non-formula text returning nullptr, throws exceptions if formula is incorrect
    if (f && FormulaHasCircularRefs(pos, f))
      throw CircularDependencyException{text};
    ImpCell* ptr = GetImpCell(pos);
    if (!ptr)
      ptr = CreateNewCell(pos);
    else
      ptr->Clear();
    ptr->SetText(move(text));
    PopulateFormulaPtrs(f); // fill formula->ast and formula->ref_ptrs with pointers to cells
    ptr->SetFormula(move(f));
    PopulateCellPtrs(ptr); // fill dependency cells
  }
}

const ICell* ImpSheet::GetCell(Position pos) const  {
  return GetImpCell(pos);
}
ICell* ImpSheet::GetCell(Position pos) {
  return GetImpCell(pos);
}

void ImpSheet::ClearCell(Position pos) {
  ImpCell* ptr = GetImpCell(pos);
  if (!ptr)
    return;
  ptr->Clear();
}

// ----------PRIVATE--------------
ImpCell* ImpSheet::GetImpCell(Position pos) {
  auto in_pos = GetInnerPosition(pos);
  auto& row = cells[in_pos.row];
  if (static_cast<size_t>(in_pos.col) >= row.size())
    return nullptr;
  return row[in_pos.col].get();
}

const ImpCell* ImpSheet::GetImpCell(Position pos) const {
  auto in_pos = GetInnerPosition(pos);
  auto& row = cells[in_pos.row];
  if (static_cast<size_t>(in_pos.col) >= row.size())
    return nullptr;
  return row[in_pos.col].get();
}

std::unique_ptr<ImpCell>& ImpSheet::GetUptr(Position pos) {
  auto in_pos = GetInnerPosition(pos);
  return cells[in_pos.row][pos.col];
}

ImpCell* ImpSheet::CreateNewCell(Position pos) {
  if (!pos.IsValid())
    throw InvalidPositionException{pos.ToString()};
  if (cells.empty()) {
    cells.resize(1);
    cells[0].resize(1);
    cells[0][0] = make_unique<ImpCell>();
    LTC = RBC = pos;
    return cells[0][0].get();
  }
  if (pos.row > RBC.row) {
    cells.resize(pos.row-LTC.row + 1);
    RBC.row = pos.row;
  } else if (pos.row < LTC.row) {
    size_t delta = LTC.row - pos.row;
    vector<Row> new_cells(RBC.row - pos.row + 1);
    size_t old_sz = cells.size();
    for (size_t i = 0; i != old_sz; ++i) {
      new_cells[i+delta] = move(cells[i]);
    }
    swap(new_cells, cells);
    LTC.row = pos.row;
  }
  if (pos.col > RBC.col) { // resize later
    RBC.col = pos.col;
  } else if (pos.col < LTC.col) {
    size_t cs = cells.size();
    size_t delta = LTC.col - pos.col;
    for (size_t i = 0; i!=cs; ++i) {
      if (cells[i].empty())
        cells[i].resize(delta); //?
      else {
        size_t old_sz = cells[i].size();
        Row new_row(old_sz+delta);
        for (size_t j = 0; j != old_sz; ++j) {
          new_row[j+delta] = move(cells[i][j]);
        }
        swap(cells[i], new_row);
      }
    }
    LTC.col = pos.col;
  }
  size_t in_row = pos.row-LTC.row;
  size_t in_col = pos.col-LTC.col;
  if (cells[in_row].size() <= in_col) // resize only current row
    cells[in_row].resize(in_col + 1);
  cells[in_row][in_col] = make_unique<ImpCell>();
  return cells[in_row][in_col].get();

}

void ImpSheet::PopulateFormulaCells(const ImpFormula::UNode& root) { // change to non-recursive
  optional<Position> mbpos = root.get()->get_position();
  if (mbpos)
    if (!GetImpCell(*mbpos))
      CreateEmptyCell(*mbpos);
  for (const auto& ch: root.get()->get_children())
    PopulateFormulaCells(ch);
}

void ImpSheet::PopulateNode(ImpFormula::UNode& root) { // change to non-recursive
  root->populate(*this);
  for (auto& ch: root.get()->get_children())
    PopulateNode(ch);
}

void ImpSheet::PopulateFormulaPtrs(unique_ptr<ImpFormula>& formula) {
  if (!formula)
    return;
  PopulateFormulaCells(formula.get()->ast);
  PopulateNode(formula.get()->ast);
  formula.get()->PopulatePtrs();
}

void ImpSheet::PopulateCellDependencies(Position pos) {

}

void ImpSheet::PopulateCellPtrs(ImpCell* cell_ptr) {
  auto ref_positions = cell_ptr->GetReferencedCells();
  for (const auto& pos: ref_positions)
    GetImpCell(pos)->AddDepPtr(cell_ptr);
}


void InsertData(unordered_map<Position, unordered_set<Position, PosHasher>, PosHasher>& graph, const ImpSheet& sh, Position pos) {
  const auto ptr = sh.GetCell(pos);
  if (ptr) {
    for (const auto p: ptr->GetReferencedCells())
      graph[pos].insert(p);
    for (const auto p: ptr->GetReferencedCells())
      InsertData(graph, sh, p);
  }
}

bool GraphIsCircular(const unordered_map<Position, unordered_set<Position, PosHasher>, PosHasher>& graph) {
  return false;
}

bool ImpSheet::FormulaHasCircularRefs(Position current_pos, const std::unique_ptr<ImpFormula>& f) const {
  unordered_map<Position, unordered_set<Position, PosHasher>, PosHasher> graph;
  for (const auto p: f->GetReferencedCells())
    graph[current_pos].insert(p);
  for (const auto p: f->GetReferencedCells())
    InsertData(graph, *this, p);
  bool this_formula_circular = GraphIsCircular(graph);
  if (this_formula_circular)
    return true;
  Graph dum_graph = dependency_graph;
  for (auto& [k,v]: graph)
    dum_graph[k] = move(v);
//  for (const auto p: f->GetReferencedCells())
//    dum_graph[p].insert(current_pos);
  if (GraphIsCircular(dum_graph))
    return true;
  swap(dum_graph, dependency_graph);
  return false;
}
