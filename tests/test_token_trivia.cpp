#include "upe/lexical.hpp"
#include "check.hpp"
#include <vector>
using namespace upe;
int main() {
  auto s=SourceSnapshot::create("a //x\n");
  std::vector<LexicalElement> elements;
  elements.emplace_back(s,ByteRange{0,1},TokenIdentity{},ElementClass::token);
  elements.emplace_back(s,ByteRange{1,2},TokenIdentity{},ElementClass::trivia);
  elements.emplace_back(s,ByteRange{2,5},TokenIdentity{},ElementClass::trivia);
  elements.emplace_back(s,ByteRange{5,6},TokenIdentity{},ElementClass::trivia);
  CHECK(covers_source(elements,*s));
  elements.pop_back();
  CHECK(!covers_source(elements,*s));
}
