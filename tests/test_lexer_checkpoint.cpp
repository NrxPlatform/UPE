#include "upe/parse_contract.hpp"
#include "check.hpp"
using namespace upe::parse;
int main() {
  SessionContracts s;
  s.set_lexer_state({4, {1,2}, {0,4}});
  auto initial = s.checkpoint();
  s.advance(2);
  s.set_lexer_state({9,{3},{0,8}});
  s.push_context(123);
  auto inner = s.checkpoint();
  s.set_lexer_state({11,{7},{0,8,12}});
  s.restore(inner);
  CHECK((s.lexer_state() == LexerState{9,{3},{0,8}}));
  s.restore(initial);
  CHECK((s.lexer_state() == LexerState{4,{1,2},{0,4}}));
  CHECK(s.cursor() == 0);
  SessionContracts foreign;
  bool rejected = false;
  try { foreign.restore(initial); } catch (const std::invalid_argument&) { rejected=true; }
  CHECK(rejected);
}
