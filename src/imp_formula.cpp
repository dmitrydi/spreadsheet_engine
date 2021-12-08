#include "imp_formula.h"

using namespace std;

using Value = ImpFormula::Value;

Value ImpFormula::Evaluate(const ISheet& sheet) const {
  visit(overload {
    [this](double val) { this->cached_val = val;},
    [this](const string& val) { this->cached_val = FormulaError(FormulaError::Category::Value); },
    [this](const FormulaError& err) { this->cached_val = err;}
  }, ast.get()->evaluate(sheet)
      );
  return cached_val;
}

std::string ImpFormula::GetExpression() const {
  return {};
}

std::vector<Position> ImpFormula::GetReferencedCells() const  {
  return ref_cells;
}

ImpFormula::HandlingResult ImpFormula::HandleInsertedRows(int before, int count) {
  if (before > ref_cells.back().row)
    return HandlingResult::NothingChanged;

  auto it = lower_bound(ref_cells.begin(), ref_cells.end(), before, [](Position pos, double value) { return pos.row < value;});
  for (;it != ref_cells.end(); ++it) {
    it->row += count;
  }

  MoveAstRowsByInsertion(before, count);
  return HandlingResult::ReferencesRenamedOnly;
}

ImpFormula::HandlingResult ImpFormula::HandleInsertedCols(int before, int count) {
  if (before > ref_cells.back().col)
    return HandlingResult::NothingChanged;

  auto it = lower_bound(ref_cells.begin(), ref_cells.end(), before, [](Position pos, double value) { return pos.col < value;});
  for (;it != ref_cells.end(); ++it) {
    it->col += count;
  }

  MoveAstColsByInsertion(before, count);
  return HandlingResult::ReferencesRenamedOnly;
}

ImpFormula::HandlingResult ImpFormula::HandleDeletedRows(int first, int count) {
  if (first > ref_cells.back().row)
    return HandlingResult::NothingChanged;
  if (first + count - 1 < ref_cells[0].row) {
    for (auto& ref: ref_cells)
      ref.row -= count;
    return HandlingResult::ReferencesRenamedOnly;
  }
  bool cell_deleted = false;
  auto it = lower_bound(ref_cells.begin(), ref_cells.end(), first, [](Position pos, double value) { return pos.row < value;});
  for ( ; it != ref_cells.end(); ++it) {
    if (it->row >= first && it->row <= first + count - 1) { // to optimize
      cell_deleted = true;
      //*it = {-1,-1}; // invalid position
    } else {
      it->row -=count;
    }
  }
  return cell_deleted ? HandlingResult::ReferencesChanged : HandlingResult::ReferencesRenamedOnly;
}

ImpFormula::HandlingResult ImpFormula::HandleDeletedCols(int first, int count) {
  if (first > ref_cells.back().col)
    return HandlingResult::NothingChanged;
  if (first + count - 1 < ref_cells[0].col) {
    for (auto& ref: ref_cells)
      ref.col -= count;
    return HandlingResult::ReferencesRenamedOnly;
  }
  bool cell_deleted = false;
   auto it = lower_bound(ref_cells.begin(), ref_cells.end(), first, [](Position pos, double value) { return pos.col < value;});
   for ( ; it != ref_cells.end(); ++it) {
     if (it->col >= first && it->col <= first + count - 1) { // to optimize
       cell_deleted = true;
       //*it = {-1,-1}; // invalid position
     } else {
       it->col -=count;
     }
   }
   return cell_deleted ? HandlingResult::ReferencesChanged : HandlingResult::ReferencesRenamedOnly;
}

class BailErrorListener : public antlr4::BaseErrorListener {
public:
    void syntaxError(antlr4::Recognizer* /* recognizer */,
                     antlr4::Token* /* offendingSymbol */, size_t /* line */,
                     size_t /* charPositionInLine */, const std::string& msg,
                     std::exception_ptr /* e */
    ) override {
        throw std::runtime_error("Error when lexing: " + msg);
    }
};

void ImpFormula::VisitCells(const Unode& root, std::vector<Position>& positions) {
  if (!root)
    return;
  root->maybe_insert_pos(positions);
  auto& children = root->get_children();
  if (children.empty())
    return;
  for (const auto& ch: children)
    VisitCells(ch, positions);
}


std::unique_ptr<ImpFormula> ParseImpFormula(std::string expression) {
try {
    auto ret = make_unique<ImpFormula>();

    antlr4::ANTLRInputStream input(expression);
    FormulaLexer lexer(&input);
    BailErrorListener error_listener;
    lexer.removeErrorListeners();
    lexer.addErrorListener(&error_listener);

    antlr4::CommonTokenStream tokens(&lexer);
    FormulaParser parser(&tokens);
    auto error_handler = std::make_shared<antlr4::BailErrorStrategy>();
    parser.setErrorHandler(error_handler);
    parser.removeErrorListeners();


    antlr4::tree::ParseTree* tree = parser.main();
    FormulaCustomListener listener;
    antlr4::tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);

    ret->ast = std::move(listener.GetAstRoot());

    std::vector<Position> positions;
    ImpFormula::VisitCells(ret->ast, positions);
    std::sort(positions.begin(), positions.end());
    auto last = std::unique(positions.begin(), positions.end());
    positions.erase(last, positions.end());
    ret->ref_cells = std::move(positions);

    return ret;
} catch (...) {
  throw FormulaException{expression};
}
}

std::unique_ptr<IFormula> ParseFormula(string expression) {
  return ParseImpFormula(move(expression));
}

void ImpFormula::MoveAstRowsByInsertion(int from, int count) {
  stack<AstNode*> st;
  st.push(ast.get());
  while(!st.empty()) {
    auto cptr = st.top();
    st.pop();
    auto mb_pos = cptr->get_mutable_position();
    if (mb_pos) {
      auto& pos_ref = *mb_pos;
      if (pos_ref.row >= from)
        pos_ref.row += count;
    }
    for (auto& ch: cptr->get_children())
      st.push(ch.get());
  }
}
void ImpFormula::MoveAstColsByInsertion(int from, int count) {
  stack<AstNode*> st;
  st.push(ast.get());
  while(!st.empty()) {
    auto cptr = st.top();
    st.pop();
    auto mb_pos = cptr->get_mutable_position();
    if (mb_pos) {
      auto& pos_ref = *mb_pos;
      if (pos_ref.col >= from)
        pos_ref.col += count;
    }
    for (auto& ch: cptr->get_children())
      st.push(ch.get());
  }
}
void ImpFormula::MoveAstRowsByDeletion(int from, int count) {

}
void ImpFormula::MoveAstColsByDeletion(int from, int count) {

}
