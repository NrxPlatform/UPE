#include "upe/lexical.hpp"
#include "check.hpp"
#include <vector>
using namespace upe;
int main() {
  auto s=SourceSnapshot::create("a?");
  std::vector<LexicalElement> e;
  e.emplace_back(s,ByteRange{0,1},TokenIdentity{},ElementClass::token);
  e.emplace_back(s,ByteRange{1,1},TokenIdentity{1,1,3},ElementClass::missing);
  e.emplace_back(s,ByteRange{1,2},TokenIdentity{},ElementClass::error);
  CHECK(e[1].synthetic() && e[1].bytes().empty() && covers_source(e,*s));
  bool rejected=false;
  try {LexicalElement bad(s,{0,1},{},ElementClass::missing);}catch(const std::invalid_argument&){rejected=true;}
  CHECK(rejected);
}
