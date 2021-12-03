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
  if ((text.size() > 1) && (text[0] == '=')) {
    auto f = ParseImpFormula(text);
    if (f && FormulaHasCircularRefs(pos, f))
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

void ImpSheet::InsertRows(int before, int count) {}
void ImpSheet::InsertCols(int before, int count) {}
void ImpSheet::DeleteRows(int first, int count) {}
void ImpSheet::DeleteCols(int first, int count) {}

Size ImpSheet::GetPrintableSize() const {
  Size sz = {printable_RBC.row - printable_LTC.row + 1, printable_RBC.col - printable_LTC.col + 1};
  if (sz.rows < 0 || sz.cols < 0)
    return {0,0};
  return sz;
}

void ImpSheet::PrintValues(std::ostream& output) const {

}

void ImpSheet::PrintTexts(std::ostream& output) const {

}

// ----------PRIVATE--------------


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
        CreateEmptyCell(*pos); // create empty cell
      node->populate(*this);
      formula.get()->ref_ptrs.insert(GetImpCell(*pos));
    }
    for (auto& ch: node->get_children()) {
      st.push(ch.get());
    }
  }
}

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

// new functions
pair<int, int> ImpSheet::GetInsertPosition(Position pos) {
    if (cells.empty())
        return {0,0};
    return {pos.row - LTC.row, pos.col - LTC.col};
}

ImpCell* ImpSheet::CreateEmptyCell(Position pos) {
    auto [ins_row, ins_col] = GetInsertPosition(pos);

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
        // resize and move contents of all rows
        for (auto& row: cells) {
            vector<unique_ptr<ImpCell>> new_cols(abs(ins_col));
            row.insert(row.begin(), make_move_iterator(new_cols.begin()), make_move_iterator(new_cols.end()));
        }
        LTC.col += ins_col;
        ins_col = 0; // in this case alway insert to zero new row
    } else {
        if (ins_col >= (int)row.size()) { // resize only current row
            row.resize(ins_col + 1);
            RBC.col = LTC.col + row.size() - 1;
        }
    }

    cells[ins_row][ins_col] = make_unique<ImpCell>();
    return cells[ins_row][ins_col].get();
}

void ImpSheet::ExpandPrintableArea(Position pos) {
    printable_LTC.row = min(printable_LTC.row, pos.row);
    printable_LTC.col = min(printable_LTC.col, pos.col);
    printable_RBC.row = max(printable_RBC.row, pos.row);
    printable_RBC.col = max(printable_RBC.col, pos.col);
}

optional<int> ImpSheet::FirstNonzeroElement(int idx) {
    if (idx < 0 || idx >= (int)cells.size())
        return nullopt;
    for (int i = 0; i < static_cast<int>(cells[idx].size()); i++) {
        if (cells[idx][i] && !cells[idx][i]->GetText().empty())
            return i;
    }
    return nullopt;
}

optional<int> ImpSheet::LastNonzeroElement(int idx) {
    if (idx < 0 || idx >= (int)cells.size())
        return nullopt;
    for (int i = static_cast<int>(cells[idx].size()) - 1; i >= 0; --i) {
        if (cells[idx][i] && !cells[idx][i]->GetText().empty())
            return i;
    }
    return nullopt;
}

pair<int,int> ImpSheet::FindTopOffset() {
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

pair<int, int> ImpSheet::FindBottomOffset() {
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
        printable_LTC.row += row_offset;
        printable_LTC.col += col_offset;
    }
    if (deleted_position.row == printable_RBC.row || deleted_position.col == printable_RBC.col) {
        auto [row_offset, col_offset] = FindBottomOffset();
        printable_RBC.row -= row_offset;
        printable_RBC.col -= col_offset;
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

void ImpSheet::ClearCell(Position pos) {
    if (GetImpCell(pos)) {
        InvalidateDependencyGraph(pos);
        DeleteCell(pos);
        SqueezePrintableArea(pos);
    }
}

void ImpSheet::CreateCell(Position pos, string text, unique_ptr<ImpFormula>&& formula) {
    ImpCell* ptr = GetImpCell(pos);
    if (ptr) {
        if (formula) {
            if (formula->GetExpression() == ptr->GetText())
                return;
        }  else {
            if (ptr->GetText() == text)
                return;
        }
    }
    ClearCell(pos);
    ptr  = CreateEmptyCell(pos);
    if (!text.empty()) {
        ptr->SetText(move(text));
        ptr->SetFormula(move(formula));
        ExpandPrintableArea(pos);
        UpdateDependencyGraph(pos); //?
    }
}

void ImpSheet::InvalidateDependencyGraph(Position pos) {}
void ImpSheet::UpdateDependencyGraph(Position pos) {}
