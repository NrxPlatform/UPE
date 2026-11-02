#include "upe/cst_builder.hpp"
#include "check.hpp"
#include <stdexcept>
#include <vector>
using namespace upe;
int main() {
  auto src=SourceSnapshot::create(std::string("a\0b",3));
  const TokenIdentity id{1,1,1};
  std::vector<LexicalElement> leaves;
  for(std::size_t i=0;i<3;++i)
    leaves.emplace_back(src,ByteRange{i,i+1},id,ElementClass::token);
  CstEventBuilder builder;
  builder.start(10);builder.leaf(0);builder.start(20);builder.leaf(1);builder.leaf(2);
  builder.finish();builder.finish();
  auto tree=std::make_shared<const FlatGreenTree>(builder.build(src,leaves));
  RedNode root(tree);
  CHECK(root.kind()==10 && root.range().begin==0 && root.range().end==3);
  CHECK(root.child_count()==2);
  CHECK(root.child(0).is_leaf() && root.child(0).range().end==1);
  auto nested=root.child(1);
  CHECK(nested.kind()==20 && nested.range().begin==1 && nested.range().end==3);
  CHECK(nested.child(1).range().begin==2);
  tree.reset(); src.reset();leaves.clear();
  CHECK(nested.child(0).range().begin==1);
  CHECK(root.child(1).child(1).range().end==3);
  bool caught=false;try { nested.child(999); } catch(const std::out_of_range&) {caught=true;}
  CHECK(caught);
}
