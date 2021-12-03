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


pair<int, int> GetInsertPosition(Position pos) {
	if (cells.empty())
		return {0,0};
	return {pos.row - LTC.row, pos.col - LTC.col};
}

ImpCell* CreateEmptyCell(Position pos) {
	auto [ins_row, ins_col] = GetInsertPosition(pos);

	if (ins_row < 0) { // insert before existing rows
		vector<Row> new_rows(abs(ins_row));
		cells.insert(cells.begin(), make_move_iterator(new_rows.begin()), make_move_iterator(new_rows.end()));
		LTC.row += ins_row;
		ins_row = 0; // in this case alway insert to zero new row
	} else {
		if (ins_row >= cells.size()) {
			cells.resize(ins_row + 1);
			RBC.row = LTC.row + cells.size() - 1;
		}	
	}

	auto& row = cells[ins_row];

	if (ins_col < 0) {
		// resize and move contents of all rows
		for (auto& row: cells) {
			vector<unique_ptr<ImpCell> new_cols(abs(ins_col));
			row.insert(row.begin(), make_move_iterator(new_cols.begin()), make_move_iterator(new_cols.end()));
		}
		LTC.col += ins_col;
		ins_col = 0; // in this case alway insert to zero new row
	} else {
		if (ins_col >= row.size()) { // resize only current row
			row.resize(ins_col + 1);
			RBC.col = LTC.col + row.size() - 1;
		}
	}

	cells[ins_row][ins_col] = make_unique<ImpCell*>();
	return cells[ins_row][ins_col].get();
}

void ExpandPrintableArea(Position pos) {
	printable_LTC.row = min(printable_LTC.row, pos.row);
	printable_LTC.col = min(printable_LTC.col, pos.col);
	printable_RBC.row = max(printable_RBC.row, pos.row);
	printable_RBC.col = max(printable_RBC.col, pos.col);
}

optional<int> FirstNonzeroElement(int idx) {
	if (idx < 0 || idx >= cells.size())
		return nullopt;
	for (int i = 0; i < static_cast<int>(cells[idx].size()); i++) {
		if (cells[idx][i] && !cells[idx][i]->GetText().empty())
			return i;
	}
	return nullopt;
}

optional<int> LastNonzeroElement(int idx) {
	if (idx < 0 || idx >= cells.size())
		return nullopt;
	for (int i = static_cast<int>(cells[idx].size()) - 1; i >= 0; --i) {
		if (cells[idx][i] && !cells[idx][i]->GetText().empty())
			return i;
	}
	return nullopt;
}

pair<int,int> FindTopOffset() {
	int row_offset = 0;
	int col_offset = RBC.col;
	auto row_nnz = FirstNonzeroElement(row_offset);
	while(row_offset < cells.size() && !row_nnz)
		row_nnz = FirstNonzeroElement(++row_offset);
	for (int i = row_offset; i < static_cast<int>(cells.size()); ++i) {
		row_nnz = FirstNonzeroElement(i);
		if (row_nnz)
			if (*row_nnz < col_offset)
				col_offset = *row_nnz;
	}
	return {row_offset, col_offset};
}

pair<int, int> FindBottomOffset() {
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

void SqueezePrintableArea(Position deleted_position) {
	if (cells.empty()) {
		printable_LTC = AbsMaxPosition;
		printable_RBC = AbsMinPosition;
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
		printable_LTC = AbsMaxPosition;
		printable_RBC = AbsMinPosition;
	}
}

void DeleteCell(Position pos) {
	auto [del_row, del_col] = GetInsertPosition(pos);
	if (del_row >= cells.size())
		return;
	auto& row = cells[del_row];
	if (row.size() <= del_col)
		return;
	row[del_col].reset(nullptr)
}

void ClearCell(Position pos) {
	if (GetImpCell(pos)) {
		InvalidateDependencyGraph(pos);
		DeleteCell(pos);
		SqueezePrintableArea(pos);	
	}
}

void CreateCell(Position pos, string text, unique_ptr<ImpFormula>&& formula) {
	ImpCell* ptr = GetImpCell(pos);
	if (ptr) {
		if (formula) {
			if (formula->GetExpression() == ptr->GetExpression())
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

ImpSheet::SetCell(Position pos, string text) {
	//если тект - формула
		// парсим формулу
		// если формула некорректна
			// бросаем исключение
		// иначе
			// CreateCell(Position pos, string text, formula)
			// UpdateDependencyGraph(pos)
	// иначе
		CreateCell(Position pos, string text, nullptr)
}

