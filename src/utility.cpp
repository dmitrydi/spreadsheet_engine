#include "utility.h"

Position operator"" _ppos(const char* str, std::size_t) {
  return Position::FromString(str);
}
