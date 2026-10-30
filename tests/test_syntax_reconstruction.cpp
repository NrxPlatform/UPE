#include "upe/cst.hpp"
#include "check.hpp"
#include <string>
#include <vector>
using namespace upe;
int main() {
  // BOM, invalid UTF-8, NUL and CRLF preserved byte-for-byte.
  const std::string input("\xEF\xBB\xBF" "a\xFF\0\r\n", 8), expected=input;
  auto source=SourceSnapshot::create(input);
  const TokenIdentity id{1,1,1};
  std::vector<LexicalElement> leaves;
  leaves.emplace_back(source,ByteRange{0,3},id,ElementClass::trivia);
  leaves.emplace_back(source,ByteRange{3,4},id,ElementClass::token);
  leaves.emplace_back(source,ByteRange{4,6},id,ElementClass::error);
  leaves.emplace_back(source,ByteRange{6,6},id,ElementClass::missing);
  leaves.emplace_back(source,ByteRange{6,8},id,ElementClass::trivia);
  const std::vector<CstEvent> events{
    CstEvent::start(10),CstEvent::leaf(0),CstEvent::start(11),
    CstEvent::leaf(1),CstEvent::leaf(2),CstEvent::leaf(3),CstEvent::finish(),
    CstEvent::leaf(4),CstEvent::finish()};
  auto flat=FlatGreenTree::build(source,leaves,events);
  auto shared=SharedGreenTree::build(source,leaves,events);
  CHECK(flat.reconstruct()==expected);
  CHECK(shared.reconstruct()==expected);
  CHECK(flat.width()==source->size());
  CHECK(shared.width()==source->size());
  CHECK(flat.metrics().nodes==2);
  CHECK(flat.metrics().leaves==5);
  CHECK(shared.metrics().nodes==2);
  CHECK(flat.leaf_range(3).begin==6 && flat.leaf_range(3).end==6);
  CHECK(shared.leaf_range(4).end==8);
  // Source must remain alive after the original owner is released.
  source.reset(); leaves.clear();
  CHECK(flat.reconstruct()==expected && shared.reconstruct()==expected);
  // Empty file is a valid zero-width root.
  auto empty=SourceSnapshot::create("");
  const std::vector<CstEvent> events2{CstEvent::start(1),CstEvent::finish()};
  CHECK(FlatGreenTree::build(empty,{},events2).reconstruct().empty());
  CHECK(SharedGreenTree::build(empty,{},events2).reconstruct().empty());
}
