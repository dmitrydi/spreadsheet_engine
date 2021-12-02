#include "imp_sheet.h"

using namespace std;

unique_ptr<ImpSheet> CreateImpSheet() {
  return nullptr;
}

unique_ptr<ISheet> CreateSheet() {
  return CreateImpSheet();
}

void ImpSheet::SetCell(Position pos, string text) {
  if (!pos.IsValid())
    throw InvalidPositionException{pos.ToString()};
  if (GetCell(pos) && GetCell(pos)->GetText() == text)
    return;

  if (text.empty()) {
    if (GetCell(pos)) {
      ClearCell(pos); // this invalidates dependent cells

      if (dependency_graph.count(pos)) // remove dependencies from the graph as empty cell do not depend on any cells
        dependency_graph.erase(pos);
      MaybeResizePrintableArea(pos);
    } else {
      CreateNewCell(pos, false); // no need to add dependencies and no resize of printable area
    }
  } else {
    auto f = ParseImpFormula(text); // may parse non-formula text returning nullptr, throws exceptions if formula is incorrect
    if (f && FormulaHasCircularRefs(pos, f))
      throw CircularDependencyException{text};
    ImpCell* ptr = GetImpCell(pos);
    if (!ptr)
      ptr = CreateNewCell(pos, true);
    else
      ptr->Clear();
    ptr->SetText(move(text));
    PopulateFormulaPtrs(f, pos); // fill formula->ast and formula->ref_ptrs with pointers to cells, set dependent cells to other cells
    ptr->SetFormula(move(f));
    //PopulateCellPtrs(ptr); // fill dependency cells
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

void ImpSheet::InsertRows(int before, int count) {}
void ImpSheet::InsertCols(int before, int count) {}
void ImpSheet::DeleteRows(int first, int count) {}
void ImpSheet::DeleteCols(int first, int count) {}

Size ImpSheet::GetPrintableSize() const {
  return {printable_RBC.row - printable_LTC.row + 1, printable_RBC.col - printable_LTC.col + 1};
}

void ImpSheet::PrintValues(std::ostream& output) const {

}

void ImpSheet::PrintTexts(std::ostream& output) const {

}

// ----------PRIVATE--------------
void ImpSheet::MaybeResizePrintableArea(Position deleted_pos) {
  if (deleted_pos.row == LTC.row) {
    bool found = false;
    for (size_t i = 0;  i != cells.size(); ++i) {
      for (const auto& cptr: cells[i]) {
        if (!cptr->GetText().empty()) {
          found = true;
          break;
        }
      }
      if (found) break;
      ++printable_LTC.row;
    }
  } else if (deleted_pos.row == RBC.row) {
    bool found = false;
    for (int i = cells.size()-1; i >=0 ; --i) {
      for (const auto& cptr: cells[i]) {
        if (!cptr->GetText().empty()) {
          found = true;
          break;
        }
      }
      if (found) break;
      --printable_RBC.row;
    }
  }
  if (deleted_pos.col == LTC.col) {
    size_t new_min = Position::kMaxCols;
    for (const auto& row : cells) {
      for (size_t i = 0; i != row.size(); ++i)
        if (!row[i]->GetText().empty()) {
          if (i == 0)
            return; // no need to change
          if(i < new_min) {
            new_min = i;
            break;
          }
        }
    }
    printable_LTC.col += new_min;
  } else if (deleted_pos.col == RBC.col) {
    int new_max = -1;
    for (const auto& row: cells) {
      for (int i = row.size() - 1; i >= 0; --i) {
        if (!row[i]->GetText().empty()) {
          if (i == RBC.col - LTC.col)
            return;
          if (i > new_max) {
            new_max = i;
            break;
          }
        }
      }
    }
    printable_RBC.col = printable_LTC.col + new_max;
  }
}

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

ImpCell* ImpSheet::CreateNewCell(Position pos, bool resize_printable_area) {
  if (!pos.IsValid())
    throw InvalidPositionException{pos.ToString()};
  if (cells.empty()) {
    cells.resize(1);
    cells[0].resize(1);
    cells[0][0] = make_unique<ImpCell>();
    LTC = RBC = pos;
    if (resize_printable_area)
      SetPrintableAreaToInner();
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
  if (resize_printable_area)
    SetPrintableAreaToInner();
  return cells[in_row][in_col].get();

}

void ImpSheet::PopulateFormulaPtrs(unique_ptr<ImpFormula>& formula, Position formula_pos) {
  if (!formula)
    return;
//  PopulateFormulaCells(formula.get()->ast);
//  PopulateNode(formula.get()->ast);
//  formula.get()->PopulatePtrs();
  ImpFormula::UNode& ast = formula.get()->ast;
  stack<AstNode*> st;
  st.push(ast.get());
  while(!st.empty()) {
    AstNode* node = st.top();
    st.pop();
    auto pos = node->get_position();
    if (pos) {
      if (!GetCell(*pos))
        CreateNewCell(*pos, false); // create empty cell
      node->populate(*this);
      AddDependentCell(*pos, formula_pos);
      formula.get()->ref_ptrs.insert(GetImpCell(*pos));
    }
    for (auto& ch: node->get_children()) {
      st.push(ch.get());
    }
  }
}

void ImpSheet::AddDependentCell(const Position& to_cell, const Position& pos) {
  ImpCell* to_ptr = GetImpCell(to_cell);
  ImpCell* ptr = GetImpCell(pos);
  assert(to_ptr); // to del
  assert(ptr); // to del
  to_ptr->AddDepPtr(ptr);
}

// ????????
void ImpSheet::PopulateFormulaCells(const ImpFormula::UNode& root) { // change to non-recursive
  optional<Position> mbpos = root.get()->get_position();
  if (mbpos)
    if (!GetImpCell(*mbpos))
      SetCell(*mbpos, "");
  for (const auto& ch: root.get()->get_children())
    PopulateFormulaCells(ch);
}

void ImpSheet::PopulateNode(ImpFormula::UNode& root) { // change to non-recursive
  root->populate(*this);
  for (auto& ch: root.get()->get_children())
    PopulateNode(ch);
}



void ImpSheet::PopulateCellPtrs(ImpCell* cell_ptr) {
  auto ref_positions = cell_ptr->GetReferencedCells();
  for (const auto& pos: ref_positions)
    GetImpCell(pos)->AddDepPtr(cell_ptr);
}

// ??????????

bool ImpSheet::GraphIsCircular(const Graph& graph, const Position start_vertex) {
  unordered_map<Position, Color, PosHasher> color;
  for (const auto& [k, refs]: graph) {
    color[k] = Color::white;
    for (const auto v: refs)
      color[v] = Color::white;
  }
  stack<Position> st;
  st.push(start_vertex);
  while(!st.empty()) {
    auto v = st.top();
    st.pop();
    if (color[v] == Color::white) {
      color[v] = Color::gray;
      st.push(v);
      for (const auto& w: graph.at(v)) {
        if (color[w] == Color::gray)
          return true;
        st.push(w);
      }
    } else if (color[v] == Color::gray)
      color[v] = Color::black;
  }
  return false;
}

bool ImpSheet::FormulaHasCircularRefs(Position current_pos, const std::unique_ptr<ImpFormula>& f) const {
  unordered_set<Position, PosHasher> temp;
  if (dependency_graph.count(current_pos)) {
    temp = move(dependency_graph[current_pos]);
    dependency_graph.erase(current_pos);
  }
  for (const auto& pos: f->GetReferencedCells())
    dependency_graph[current_pos].insert(pos);
  if (GraphIsCircular(dependency_graph, current_pos)) {
    if (temp.empty())
      dependency_graph.erase(current_pos);
    else
      dependency_graph[current_pos] = move(temp);
    return true;
  }
  for (auto& pos: temp)
    dependency_graph[current_pos].insert(move(pos));
  return false;
}
