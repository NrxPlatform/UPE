#include "upe/cst_builder.hpp"
#include "check.hpp"
#include <stdexcept>
#include <vector>
using namespace upe;
template<class F> bool throws(F f) { try { f(); } catch (const std::exception&) { return true; } return false; }
int main() {
  CstEventBuilder b, foreign;
  auto outer=b.checkpoint();
  b.start(10);
  auto middle=b.checkpoint();
  b.start(20);
  auto inner=b.checkpoint();
  b.leaf(99);
  b.finish();
  b.rollback(middle);
  CHECK(throws([&]{b.rollback(inner);}));
  CHECK(throws([&]{b.rollback(middle);}));
  CHECK(b.depth()==1 && b.events().size()==1);
  CHECK(throws([&]{foreign.rollback(outer);}));
  auto second=b.checkpoint();
  CHECK(throws([&]{b.commit(outer);}));
  b.commit(second);
  b.rollback(outer);
  CHECK(b.events().empty() && b.depth()==0);
  CHECK(throws([&]{b.rollback(outer);}));
  auto src=SourceSnapshot::create("ab");
  const TokenIdentity id{1,1,1};
  std::vector<LexicalElement> leaves;
  leaves.emplace_back(src,ByteRange{0,1},id,ElementClass::token);
  leaves.emplace_back(src,ByteRange{1,2},id,ElementClass::token);
  b.start(1);
  b.leaf(0);
  auto nested=b.checkpoint();
  b.start(2); b.leaf(1); b.finish();
  b.commit(nested);
  b.finish();
  CHECK(b.complete());
  CHECK(b.build(src,leaves).reconstruct()=="ab");
  CHECK(throws([&]{b.start(4);}));
  CHECK(throws([&]{b.finish();}));
  CstEventBuilder invalid;
  invalid.start(1); invalid.leaf(99); invalid.finish();
  CHECK(throws([&]{invalid.build(src,leaves);}));
}
