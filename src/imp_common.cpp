#include "common.h"
#include "imp_formula.h"
#include "imp_cell.h"
#include "imp_sheet.h"

using namespace std;

// Position

ostream& operator<<(std::ostream& output, Position pos) {
  return output << "(" << pos.row << ", " << pos.col << ")";
}

bool Position::operator==(const Position& rhs) const {
    return row == rhs.row && col == rhs.col;
}

bool Position::operator<(const Position& rhs) const {
  if (row == rhs.row)
    return col < rhs.col;
  return row < rhs.row;
    //return row < rhs.row || col < rhs.col;
}

bool Position::IsValid() const {
    return row >= 0 && row < kMaxRows && col >= 0 && col < kMaxCols;
}

string Position::ToString() const {
  if (!IsValid())
    return {};
  string s;
  char dum;
  int c = col;
  while (c >= 0) {
    dum = 'A' + c % 26;
    s = dum + s;
    c /= 26;
    c -= 1;
  }
  return s+to_string(row+1);
}

int ParseNum(string_view str) {
  // shift co C++ - index: 1 -> 0
  int num = 0;
  int mult = 1;
  while(!str.empty()) {
    int val = str[str.size() - 1] - '0';
    num += val*mult;
    mult *=10;
    str.remove_suffix(1);
  }
  return num-1;
}

int ParseLetters(string_view str) {
  int num = 0;
  int mult = 1;
  while(!str.empty()) {
    int val = str[str.size()-1] - 'A' + 1;
    num += val*mult;
    mult *= 26;
    str.remove_suffix(1);
  }
  return num - 1;
}

Position Position::FromString(std::string_view str) {
  int cntr = 0;
  bool is_valid_input = true;
  while (isalpha(str[cntr++])) {
    if (!isupper(str[cntr-1]))
      is_valid_input = false;
  }
  auto letters = str.substr(0, --cntr);
  if (letters.empty())
    is_valid_input = false;
  str.remove_prefix(cntr);
  if (str.empty())
    is_valid_input = false;
  for (size_t i = 0; i < str.size(); ++i)
    if (!isdigit(str[i]))
      is_valid_input = false;
  if (!is_valid_input)
    return {-1,-1};
  return {ParseNum(str), ParseLetters(letters)};
}

// ~Position

// Size

bool Size::operator==(const Size& rhs) const {
    return cols == rhs.cols && rows == rhs.rows;
}

// ~Size


// FormulaError

FormulaError::FormulaError(Category category): category_(category) {}

FormulaError::Category FormulaError::GetCategory() const {
    return category_;
}

bool FormulaError::operator==(FormulaError rhs) const {
    return category_ == rhs.category_;
}

string_view FormulaError::ToString() const {
  switch(category_) {
    case Category::Div0:
      return "#DIV/0!";
    case Category::Ref:
      return "#REF!";
    default:
      return "#VALUE!";
  };
}

ostream& operator<<(ostream& output, FormulaError fe) {
  output << fe.ToString();
  return output;
}

// ~FormulaError
