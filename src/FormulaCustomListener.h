#pragma once

#include "FormulaBaseListener.h"
#include "ast.h"
#include <stack>

using Unode = std::unique_ptr<AstNode>;

class FormulaCustomListener: public FormulaBaseListener {
public:
  virtual void exitEveryRule(antlr4::ParserRuleContext * ctx) override {

  }

  virtual void exitLiteral(FormulaParser::LiteralContext * ctx) override {
    st.push(std::make_unique<AstNum>(stod(ctx->getText())));
  }

  virtual void exitCell(FormulaParser::CellContext * ctx) override {
    st.push(std::make_unique<AstCell>(ctx->getText()));
  }

  virtual void exitUnaryOp(FormulaParser::UnaryOpContext * ctx) override {
    char op = ctx->getText()[0];
    auto un = std::make_unique<AstUnary>(op);
    auto operand = std::move(st.top());
    st.pop();
    un->push_child(std::move(operand));
    st.push(std::move(un));
  }

  virtual void exitBinaryOp(FormulaParser::BinaryOpContext * ctx) override {
    char op;
    auto ch = ctx->children;
    for (const auto c: ch) {
      if (antlrcpp::is<antlr4::tree::TerminalNode* >(c)) {
        op = c->getText()[0];
        break;
      }
    }

    auto bn = std::make_unique<AstBinary>(op);
    auto rhs = std::move(st.top());
    st.pop();
    auto lhs = std::move(st.top());
    st.pop();
    bn->set_child(0, std::move(lhs));
    bn->set_child(1, std::move(rhs));
    st.push(std::move(bn));
  }

  Unode GetAstRoot() {
    if (st.empty())
      return nullptr;
    auto ret = std::move(st.top());
    st.pop();
    return ret;
  }
private:
  std::stack<Unode> st;
  char op;
};
