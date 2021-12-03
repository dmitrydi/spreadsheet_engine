#pragma once

#include "common.h"

template<class... Ts> struct overload : Ts... { using Ts::operator()...; };
template<class... Ts> overload(Ts...) -> overload<Ts...>;

struct PosHasher {
  size_t operator()(const Position pos) const {
    size_t lhs = static_cast<size_t>(pos.row);
    size_t rhs = static_cast<size_t>(pos.col);
    lhs ^= rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2);
    return lhs;
  }
};

Position operator"" _ppos(const char* str, std::size_t);
