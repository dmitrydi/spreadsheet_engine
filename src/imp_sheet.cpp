#include "imp_sheet.h"

using namespace std;

unique_ptr<ImpSheet> CreateImpSheet() {
  return make_unique<ImpSheet>();
}

unique_ptr<ISheet> CreateSheet() {
  return CreateImpSheet();
}

void ImpSheet::SetCell(Position pos, string text) {
  if (!pos.IsValid())
    throw InvalidPositionException{pos.ToString()};
  if ((text.size() > 1) && (text[0] == kFormulaSign)) {
    auto f = ParseImpFormula({next(text.begin()), text.end()});
    if (FormulaHasCircularRefs(pos, f))
      throw CircularDependencyException{text};
    else {
      PopulateFormulaPtrs(f, pos);
      CreateCell(pos, move(text), move(f));
    }
  } else {
    CreateCell(pos, move(text), nullptr);
  }
}

const ICell* ImpSheet::GetCell(Position pos) const  {
  return GetImpCell(pos);
}
ICell* ImpSheet::GetCell(Position pos) {
  return GetImpCell(pos);
}

void ImpSheet::ClearCell(Position pos) {
    if (GetImpCell(pos)) {
        InvalidateDependencyGraph(pos);
        RemoveDependencies(pos);
        UpdateReferenceGraph(pos);
        DeleteCell(pos);
        //SqueezePrintableArea(pos);
    }
}

void ImpSheet::InsertRows(int before, int count) {
  CheckCellsSize(count, 0);
  CheckFormulas(count, 0);

  UpdateFormulasByRowInsertion(before, count);
  UpdateGraphsByRowInsertion(before, count);
  CreateRows(before, count);
}

void ImpSheet::InsertCols(int before, int count) {
  CheckCellsSize(0, count);
  CheckFormulas(0, count);

  UpdateFormulasByColInsertion(before, count);
  UpdateGraphsByColInsertion(before, count);
  CreateCols(before, count);
}

void ImpSheet::DeleteRows(int first, int count) {
  UpdateFormulasByRowDeletion(first, count);
  UpdateGraphsBuRowDeletion(first, count);
  EraseRows(first, count);
}

void ImpSheet::DeleteCols(int first, int count) {
  UpdateFoumulasByColDeletion(first, count);
  UpdateGraphsByColDeletion(first, count);
  EraseCols(first, count);
}

Size ImpSheet::GetPrintableSize() const {
  if (cells.empty())
    return {0,0};

  auto bottom_offset = FindBottomOffset();

  int vsize = RBC.row + 1 - bottom_offset.first;
  if (vsize <= 0)
    return {0,0};
  int hsize = RBC.col + 1 - bottom_offset.second;
  if (hsize <=0)
    return {0,0};
  return {vsize, hsize};
}

void ImpSheet::PrintValues(std::ostream& output) const {
  auto psz = GetPrintableSize();
  for (int ridx = 0; ridx < psz.rows; ++ridx) {
    if(ridx < LTC.row) {
      output << "\t";
    } else {
      if (!RowHasPrintableCells(cells[ridx-LTC.row]))
        output << "\t";
      else {
        bool first = true;
        for (int cidx = 0; cidx < psz.cols; ++cidx) {
          if (!first)
            output << "\t";
          first = false;
          auto ptr = GetCell({ridx, cidx});
          if (ptr)
            PrintValue(output, ptr->GetValue());
        }
      }
    }
    output << "\n";
  }
}

void ImpSheet::PrintTexts(std::ostream& output) const {
  auto psz = GetPrintableSize();
  for (int ridx = 0; ridx < psz.rows; ++ridx) {
    if(ridx < LTC.row) {
      output << "\t";
    } else {
      if (!RowHasPrintableCells(cells[ridx-LTC.row]))
        output << "\t";
      else {
        bool first = true;
        for (int cidx = 0; cidx < psz.cols; ++cidx) {
          if (!first)
            output << "\t";
          first = false;
          auto ptr = GetCell({ridx, cidx});
          if (ptr)
            output << ptr->GetText();
        }
      }
    }
    output << "\n";
  }
}

// --------------PRIVATE--------------
//
//
//
//------------------------------------

void ImpSheet::CheckCellsSize(int row_count, int col_count) const {
  if (RBC.row + row_count >= Position::kMaxRows)
    throw TableTooBigException{to_string(RBC.row + row_count)};
  if (RBC.col + col_count >= Position::kMaxCols)
    throw TableTooBigException{to_string(RBC.col + col_count)};
}

void ImpSheet::CheckFormulas(int row_count, int col_count) const {
  Position max_pos{-1,-1};
  for (const auto& [cur_pos, ref_cells]: reference_graph) {
    if (max_pos < cur_pos)
      max_pos = cur_pos;
    for (const auto& ref: ref_cells)
      if (max_pos < ref)
        max_pos = ref;
  }
  if (max_pos.row + row_count >= Position::kMaxRows)
    throw TableTooBigException{to_string(max_pos.row + row_count)};
  if (max_pos.col + col_count >= Position::kMaxCols)
    throw TableTooBigException{to_string(max_pos.col + col_count)};
}

void ImpSheet::UpdateFormulasByRowInsertion(int before, int count) {
  // update all ast in formulas
  for (auto& row: cells)
    for (auto& cell: row)
      if (cell && cell->formula)
        cell->formula->HandleInsertedRows(before, count);
}

void ImpSheet::UpdateListByRow(unordered_set<Position, PosHasher>& dep_cells, int before, int count) {
  vector<Position> to_update;
  for (const auto& dep: dep_cells)
    if (dep.row >= before)
      to_update.push_back(dep);
  for (const auto& del_pos: to_update)
    dep_cells.erase(del_pos);
  for (const auto& pos: to_update)
    dep_cells.insert({pos.row + count, pos.col});
}

void ImpSheet::UpdateKeysByRow(Graph& gr, int before, int count) {
  vector<Position> update_keys;
  Graph tmp;
  for (const auto& [k, _]: gr)
    if (k.row >= before)
      update_keys.push_back(k);
  for (const auto key: update_keys) {
    tmp[key] = move(gr[key]);
    gr.erase(key);
  }
  for (const auto key: update_keys) {
    gr[{key.row + count, key.col}] = move(tmp[key]);
  }
}

void ImpSheet::UpdateGraphsByRowInsertion(int before, int count) {
  for (auto& [_, dep_cells]: dependency_graph) {
    UpdateListByRow(dep_cells, before, count);
  }
  UpdateKeysByRow(dependency_graph, before, count);

  for (auto& [_, ref_cells]: reference_graph) {
    UpdateListByRow(ref_cells, before, count);
  }
  UpdateKeysByRow(reference_graph, before, count);
}

void ImpSheet::CreateRows(int before, int count) {
  if (cells.empty()) {
    return;
  }
  int insert_position = before - LTC.row;
  if (insert_position < 0) {
    LTC.row += count;
    RBC.row += count;
  } else if (insert_position >= 0 && insert_position < (int)cells.size()) {
    vector<Row> to_insert(count);
    auto it = cells.begin();
    it += insert_position;
    cells.insert(it, make_move_iterator(to_insert.begin()), make_move_iterator(to_insert.end()));
    RBC.row += count;
  }
}

void ImpSheet::UpdateFormulasByColInsertion(int before, int count) {
  for (auto& row: cells)
    for (auto& cell: row)
      if (cell && cell->formula)
        cell->formula->HandleInsertedCols(before, count);
}

void ImpSheet::UpdateListByCol(std::unordered_set<Position, PosHasher>& dep_cells, int before, int count) {
  vector<Position> to_update;
  for (const auto& dep: dep_cells)
    if (dep.col >= before)
      to_update.push_back(dep);
  for (const auto& del_pos: to_update)
    dep_cells.erase(del_pos);
  for (const auto& pos: to_update)
    dep_cells.insert({pos.row, pos.col + count});
}

void ImpSheet::UpdateKeysByCol(Graph& gr, int before, int count) {
  vector<Position> update_keys;
  Graph tmp;
  for (const auto& [k, _]: gr)
    if (k.col >= before)
      update_keys.push_back(k);
  for (const auto key: update_keys) {
    tmp[key] = move(gr[key]);
    gr.erase(key);
  }
  for (const auto key: update_keys) {
    gr[{key.row, key.col + count}] = move(tmp[key]);
  }
}

void ImpSheet::UpdateGraphsByColInsertion(int before, int count) {
  for (auto& [_, dep_cells]: dependency_graph) {
    UpdateListByCol(dep_cells, before, count);
  }
  UpdateKeysByCol(dependency_graph, before, count);

  for (auto& [_, ref_cells]: reference_graph) {
    UpdateListByCol(ref_cells, before, count);
  }
  UpdateKeysByCol(reference_graph, before, count);
}

void ImpSheet::CreateCols(int before, int count) {
  if (cells.empty()) {
    return;
  }
  int insert_position = before - LTC.col;
  if (insert_position < 0) {
    LTC.col += count;
    RBC.col += count;
  } else if (insert_position >= 0 && insert_position <= RBC.col - LTC.col) {
    for (auto& row: cells) {
      if (before < (int)row.size()) {
        vector<unique_ptr<ImpCell>> new_cells(count);
        auto it = row.begin() + before;
        row.insert(it, make_move_iterator(new_cells.begin()), make_move_iterator(new_cells.end()));
      }
    }
    RBC.col += count;
  }
}

void ImpSheet::UpdateFormulasByRowDeletion(int first,int count) {
  for (auto& row: cells)
    for (auto& cell: row) {
      if (cell && cell->formula)
        cell->formula->HandleDeletedRows(first, count);
    }
}

void ImpSheet::DeleteRowsFromGraph(Graph& gr, int first, int count) {
  vector<Position> to_del_keys;

  for (auto& [k, dep_cells]: gr) {
    if (k.row >= first && k.row <= first + count - 1)
      to_del_keys.push_back(k);

    vector<Position> to_del_vals;
    for (const auto& dep_cell: dep_cells) {
      if (dep_cell.row >= first && dep_cell.row <= first + count - 1)
        to_del_vals.push_back(dep_cell);
    }
    for (const auto& v: to_del_vals)
      dep_cells.erase(v);
  }
  for (const auto& k: to_del_keys)
    gr.erase(k);

  Graph tmp;
  for (const auto& [k, dep_cells]: gr) {
    Position new_key = k;
    if (k.row >= first)
      new_key.row -= count;
    for (const auto& dep_cell: dep_cells) {
      Position new_dep = dep_cell;
      if (new_dep.row >= first)
        new_dep.row -= count;
      tmp[new_key].insert(new_dep);
    }
  }
  swap(tmp, gr);
}

void ImpSheet::UpdateGraphsBuRowDeletion(int first, int count) {
  // invalidate dependency graphs
  int start_row = max(first, LTC.row);
  int end_row = first + count - 1;
  for (int i = start_row; i <= min(end_row, RBC.row); ++i) {
    for (int j = 0; j != (int)cells[i].size(); ++j) {
      Position pos = {i, LTC.col + j};
      InvalidateDependencyGraph(pos);
    }
  }
  DeleteRowsFromGraph(dependency_graph, first, count);
  DeleteRowsFromGraph(reference_graph, first, count);

}
void ImpSheet::EraseRows(int first, int count) {
  if (first + count - 1 < LTC.row) {
    LTC.row -= count;
    RBC.row -= count;
  } else if (first <= RBC.row ){
    int start_row = max(0,first - LTC.row);
    int end_row = min(first+count-1 - LTC.row, RBC.row);
    int delta = end_row - start_row + 1;
    auto it_first = cells.begin() + start_row;
    auto it_last = cells.begin() + end_row + 1;
    cells.erase(it_first, it_last);
    RBC.row -= delta;
  }
}




void ImpSheet::UpdateFoumulasByColDeletion(int first, int count) {
  for (auto& row: cells)
    for (auto& cell: row) {
      if (cell && cell->formula)
        cell->formula->HandleDeletedCols(first, count);
    }
}

void ImpSheet::DeleteColsFromGraph(Graph& gr, int first, int count) {
  vector<Position> to_del_keys;

  for (auto& [k, dep_cells]: gr) {
    if (k.col >= first && k.col <= first + count - 1)
      to_del_keys.push_back(k);

    vector<Position> to_del_vals;
    for (const auto& dep_cell: dep_cells) {
      if (dep_cell.col >= first && dep_cell.col <= first + count - 1)
        to_del_vals.push_back(dep_cell);
    }
    for (const auto& v: to_del_vals)
      dep_cells.erase(v);
  }
  for (const auto& k: to_del_keys)
    gr.erase(k);

  Graph tmp;
  for (const auto& [k, dep_cells]: gr) {
    Position new_key = k;
    if (k.col >= first)
      new_key.col -= count;
    for (const auto& dep_cell: dep_cells) {
      Position new_dep = dep_cell;
      if (new_dep.col >= first)
        new_dep.col -= count;
      tmp[new_key].insert(new_dep);
    }
  }
}

void ImpSheet::UpdateGraphsByColDeletion(int first, int count) {
  // invalidate dependency graphs
  for (int i = LTC.row; i <= RBC.row; ++i) {
    for (int j = max(LTC.col, first); j <= min(RBC.col, first + count - 1); ++j) {
      InvalidateDependencyGraph({i,j});
    }
  }
  DeleteColsFromGraph(dependency_graph, first, count);
  DeleteColsFromGraph(reference_graph, first, count);
}
void ImpSheet::EraseCols(int first, int count) {
  if (first + count -1 < LTC.col) {
    LTC.col -= count;
    RBC.col -= count;
  } else if (first <= RBC.col) {
    int inner_start = max(LTC.col, first) - LTC.col;
    int inner_end = min(RBC.col, first+count-1) - LTC.col;
    int delta = inner_end - inner_start + 1;
    for (auto& row: cells) {
      auto it_first = row.begin() + inner_start;
      auto it_last = row.begin() + inner_end + 1;
      row.erase(it_first, it_last);
    }
    RBC.col -= delta;
  }
}

void ImpSheet::PrintValue(std::ostream& os, ICell::Value val) {
  visit(
      overload {
    [&](FormulaError&& err) {
      switch(err.GetCategory()) {
        case FormulaError::Category::Div0:
          os << "#DIV/0!";
          break;
        case FormulaError::Category::Ref:
          os << "#REF!";
          break;
        case FormulaError::Category::Value:
          os << "#VALUE!";
          break;
      }
    },
    [&](auto&& v) {
      os << v;
    }
  },
      val);
}

bool ImpSheet::RowHasPrintableCells(const Row& row) {
  for (const auto& cell: row)
    if(!cell->GetText().empty())
      return true;
  return false;
}

ImpCell* ImpSheet::GetImpCell(Position pos) {
  if (cells.empty())
    return nullptr;
  auto in_pos = GetInnerPosition(pos);
  if (!in_pos.IsValid())
    return nullptr;
  if (in_pos.row >= (int)cells.size())
    return nullptr;
  auto& row = cells[in_pos.row];
  if (static_cast<size_t>(in_pos.col) >= row.size())
    return nullptr;
  return row[in_pos.col].get();
}

const ImpCell* ImpSheet::GetImpCell(Position pos) const {
  if (cells.empty())
    return nullptr;
  auto in_pos = GetInnerPosition(pos);
  if (!in_pos.IsValid())
    return nullptr;
  if (in_pos.row >= (int)cells.size())
    return nullptr;
  auto& row = cells[in_pos.row];
  if (static_cast<size_t>(in_pos.col) >= row.size())
    return nullptr;
  return row[in_pos.col].get();
}

void ImpSheet::PopulateFormulaPtrs(unique_ptr<ImpFormula>& formula, Position formula_pos) {

  if (!formula)
    return;
  ImpFormula::UNode& ast = formula.get()->ast;
  stack<AstNode*> st;
  st.push(ast.get());
  while(!st.empty()) {
    AstNode* node = st.top();
    st.pop();
    auto pos = node->get_position();
    if (pos) {
      if (!GetCell(*pos))
        SetCell(*pos, "");
      node->populate(*this);
    }
    for (auto& ch: node->get_children()) {
      st.push(ch.get());
    }
  }

}

pair<int, int> ImpSheet::GetInsertPosition(Position pos) const {
    if (cells.empty())
        return {0,0};
    return {pos.row - LTC.row, pos.col - LTC.col};
}

ImpCell* ImpSheet::CreateEmptyCell(Position pos) {
    auto [ins_row, ins_col] = GetInsertPosition(pos);

    if (cells.empty())
      LTC = pos;

    if (ins_row < 0) { // insert before existing rows
        vector<Row> new_rows(abs(ins_row));
        cells.insert(cells.begin(), make_move_iterator(new_rows.begin()), make_move_iterator(new_rows.end()));
        LTC.row += ins_row;
        ins_row = 0; // in this case alway insert to zero new row
    } else {
        if (ins_row >= (int)cells.size()) {
            cells.resize(ins_row + 1);
            RBC.row = LTC.row + cells.size() - 1;
        }
    }

    auto& row = cells[ins_row];

    if (ins_col < 0) {
        // resize and move contents of all non-empty rows
        for (auto& row_: cells) {
          if(!row_.empty()) {
            vector<unique_ptr<ImpCell>> new_cols(abs(ins_col));
            row_.insert(row_.begin(), make_move_iterator(new_cols.begin()), make_move_iterator(new_cols.end()));
          }
        }
        LTC.col += ins_col;
        ins_col = 0; // in this case alway insert to zero new row
    }

    if (ins_col >= (int)row.size()) { // resize only current row
        row.resize(ins_col + 1);
        RBC.col = max(RBC.col, LTC.col + (int)row.size() - 1);
    }


    cells[ins_row][ins_col] = make_unique<ImpCell>(this);
    return cells[ins_row][ins_col].get();
}

void ImpSheet::ExpandPrintableArea(Position pos) {
    printable_LTC.row = min(printable_LTC.row, pos.row);
    printable_LTC.col = min(printable_LTC.col, pos.col);
    printable_RBC.row = max(printable_RBC.row, pos.row);
    printable_RBC.col = max(printable_RBC.col, pos.col);
}

optional<int> ImpSheet::FirstNonzeroElement(int idx) const {
    if (idx < 0 || idx >= (int)cells.size())
        return nullopt;
    for (int i = 0; i < static_cast<int>(cells[idx].size()); i++) {
        if (cells[idx][i] && !cells[idx][i]->GetText().empty())
            return i;
    }
    return nullopt;
}

optional<int> ImpSheet::LastNonzeroElement(int idx) const {
    if (idx < 0 || idx >= (int)cells.size())
        return nullopt;
    for (int i = static_cast<int>(cells[idx].size()) - 1; i >= 0; --i) {
        if (cells[idx][i] && !cells[idx][i]->GetText().empty())
            return i;
    }
    return nullopt;
}

pair<int,int> ImpSheet::FindTopOffset() const {
    int row_offset = 0;
    int col_offset = RBC.col;
    auto row_nnz = FirstNonzeroElement(row_offset);
    while(row_offset < (int)cells.size() && !row_nnz)
        row_nnz = FirstNonzeroElement(++row_offset);
    for (int i = row_offset; i < static_cast<int>(cells.size()); ++i) {
        row_nnz = FirstNonzeroElement(i);
        if (row_nnz)
            if (*row_nnz < col_offset)
                col_offset = *row_nnz;
    }
    return {row_offset, col_offset};
}

pair<int, int> ImpSheet::FindBottomOffset() const {
    int row_offset = 0;
    int col_offset = 0;
    int i = static_cast<int>(cells.size()) - 1;
    for (; i >=0; --i) {
        auto row_nnz = FirstNonzeroElement(i);
        if (!row_nnz)
            ++row_offset;
        else
            break;
    }
    optional<int> max_ind = col_offset;
    for (; i >= 0; --i) {
        auto last_ind = LastNonzeroElement(i);
        if (last_ind && *last_ind > *max_ind)
            max_ind = last_ind;
    }
    col_offset = RBC.col-LTC.col-*max_ind;
    return {row_offset, col_offset};
}

void ImpSheet::SqueezePrintableArea(Position deleted_position) {
    if (cells.empty()) {
        printable_LTC = AbsMaxPos;
        printable_RBC = AbsMinPos;
        return;
    }
    if (deleted_position.row == printable_LTC.row || deleted_position.col == printable_LTC.col) {
        auto [row_offset, col_offset] = FindTopOffset();
        printable_LTC.row = LTC.row + row_offset;
        printable_LTC.col = LTC.col + col_offset;
    }
    if (deleted_position.row == printable_RBC.row || deleted_position.col == printable_RBC.col) {
        auto [row_offset, col_offset] = FindBottomOffset();
        printable_RBC.row = RBC.row - row_offset;
        printable_RBC.col = RBC.col - col_offset;
    }
    if ((printable_LTC.row > printable_RBC.row) || (printable_LTC.col > printable_RBC.col)) {
        printable_LTC = AbsMaxPos;
        printable_RBC = AbsMinPos;
    }
}

void ImpSheet::DeleteCell(Position pos) {
    auto [del_row, del_col] = GetInsertPosition(pos);
    if (del_row >= static_cast<int>(cells.size()))
        return;
    auto& row = cells[del_row];
    if (static_cast<int>(row.size()) <= del_col)
        return;
    row[del_col].reset(nullptr);
}

void ImpSheet::UpdateReferenceGraph(Position pos) {
  if (reference_graph.count(pos))
    reference_graph.erase(pos);
}

void ImpSheet::RemoveDependencies(Position pos) {
  for (auto& [_, dep_cells]: dependency_graph) {
    if (dep_cells.count(pos))
      dep_cells.erase(pos);
  }
}

void ImpSheet::ResetCell(Position pos) {
  ImpCell* ptr = GetImpCell(pos);
  if (!ptr)
    return;
  InvalidateDependencyGraph(pos);
  RemoveDependencies(pos);
  UpdateReferenceGraph(pos);
  ptr->Clear();
  SqueezePrintableArea(pos);
}



void ImpSheet::CreateCell(Position pos, string text, unique_ptr<ImpFormula>&& formula) {
    ImpCell* ptr = GetImpCell(pos);
    if (ptr && ptr->GetText() == text)
      return;
    if (ptr)
      ResetCell(pos);
    else
      ptr  = CreateEmptyCell(pos);
    if (!text.empty()) {
        ptr->SetText(move(text));
        if (formula) {
          for (const auto& ref_p: formula->GetReferencedCells())
            reference_graph[pos].insert(ref_p);
        }
        ptr->SetFormula(move(formula));
        ExpandPrintableArea(pos);
        UpdateDependencyGraph(pos); //?
    }
}

void ImpSheet::InvalidateDependencyGraph(Position pos) {
  if (!GetImpCell(pos))
    return;
  stack<Position> st;
  st.push(pos);
  while(!st.empty()) {
    auto cur_pos = st.top();
    st.pop();
    GetImpCell(cur_pos)->Invalidate();
    if (dependency_graph.count(cur_pos)) {
      for (const auto& dep_pos: dependency_graph.at(cur_pos)) {
        if (GetImpCell(dep_pos)->IsValid()) {
          st.push(dep_pos);
        }
      }
    }
  }
}

void ImpSheet::UpdateDependencyGraph(Position pos) {
  const auto ptr = GetImpCell(pos);
  for (const auto& p: ptr->GetReferencedCells()) {
    dependency_graph[p].insert(pos);
  }
}

//bool ImpSheet::GraphIsCircular(const Graph& graph, const Position start_vertex) {
//  unordered_map<Position, Color, PosHasher> color;
//  for (const auto& [k, refs]: graph) {
//    color[k] = Color::white;
//    for (const auto v: refs)
//      color[v] = Color::white;
//  }
//  stack<Position> st;
//  st.push(start_vertex);
//  while(!st.empty()) {
//    auto v = st.top();
//    st.pop();
//    if (color[v] == Color::white) {
//      color[v] = Color::gray;
//      st.push(v);
//      if (graph.count(v)) {
//        for (const auto& w: graph.at(v)) {
//          if (color[w] == Color::gray) {
//            return true;
//          }
//          st.push(w);
//        }
//      }
//    } else if (color[v] == Color::gray)
//      color[v] = Color::black;
//  }
//  return false;
//}

bool ImpSheet::GraphIsCircular(const Graph& graph, const Position start_vertex) {
  unordered_map<Position, Color, PosHasher> color;

  stack<Position> st;
  st.push(start_vertex);
  while(!st.empty()) {
    auto v = st.top();
    st.pop();
    if (!color.count(v)) {
      color[v] = Color::gray;
      st.push(v);
      if (graph.count(v)) {
        for (const auto& w: graph.at(v)) {
          if (color.count(w) && color[w] == Color::gray) {
            return true;
          }
          st.push(w);
        }
      }
    } else if (color[v] == Color::gray)
      color[v] = Color::black;
  }
  return false;
}

bool ImpSheet::FormulaHasCircularRefs(Position current_pos, const std::unique_ptr<ImpFormula>& f) const {
  unordered_set<Position, PosHasher> temp;
  if (reference_graph.count(current_pos)) {
    temp = move(reference_graph[current_pos]);
    reference_graph.erase(current_pos);
  }

  for (const auto& pos: f->GetReferencedCells()) {
    reference_graph[current_pos].insert(pos);
  }
  if (GraphIsCircular(reference_graph, current_pos)) {
    if (temp.empty())
      reference_graph.erase(current_pos);
    else
      reference_graph[current_pos] = move(temp);
    return true;
  }
  for (auto& pos: temp)
    reference_graph[current_pos].insert(move(pos));
  return false;
}
