#include "upe/lexical.hpp"
#include "check.hpp"
using namespace upe;
int main() {
  auto s=SourceSnapshot::create("abc");
  LexicalElement t(s,{0,3},{1,7,42},ElementClass::token);
  CHECK(t.bytes()=="abc" && t.kind().kind==42 && !t.synthetic());
  bool rejected=false;
  try { LexicalElement bad(s,{0,4},{},ElementClass::token); } catch(const std::out_of_range&) {rejected=true;}
  CHECK(rejected);
}
