#include "upe/cst.hpp"
#include "check.hpp"
#include <stdexcept>
#include <vector>
using namespace upe;
static void reject(std::shared_ptr<const SourceSnapshot> source,
                   const std::vector<LexicalElement>& leaves,
                   const std::vector<CstEvent>& events) {
  bool a=false,b=false;
  try { (void)FlatGreenTree::build(source,leaves,events); } catch(const std::invalid_argument&) { a=true; }
  try { (void)SharedGreenTree::build(source,leaves,events); } catch(const std::invalid_argument&) { b=true; }
  CHECK(a && b);
}
int main() {
  auto source=SourceSnapshot::create("ab");
  const TokenIdentity id{1,1,1};
  std::vector<LexicalElement> leaves{
    {source,{0,1},id,ElementClass::token},{source,{1,2},id,ElementClass::token}};
  reject(source,leaves,{CstEvent::start(1),CstEvent::leaf(0),CstEvent::finish()}); // gap
  reject(source,leaves,{CstEvent::start(1),CstEvent::leaf(1),CstEvent::leaf(0),CstEvent::finish()});
  reject(source,leaves,{CstEvent::start(1),CstEvent::leaf(0),CstEvent::leaf(0),CstEvent::finish()});
  reject(source,leaves,{CstEvent::start(1),CstEvent::leaf(0),CstEvent::leaf(2),CstEvent::finish()});
  reject(source,leaves,{CstEvent::start(1),CstEvent::leaf(0),CstEvent::leaf(1)}); // unbalanced
  reject(source,leaves,{CstEvent::start(1),CstEvent::finish(),CstEvent::start(2)});
  reject(source,leaves,{CstEvent::leaf(0),CstEvent::start(1),CstEvent::finish()});
  reject(source,leaves,{CstEvent::start(0),CstEvent::leaf(0),CstEvent::leaf(1),CstEvent::finish()});
  auto foreign=SourceSnapshot::create("ab");
  std::vector<LexicalElement> wrong{{foreign,{0,2},id,ElementClass::token}};
  reject(source,wrong,{CstEvent::start(1),CstEvent::leaf(0),CstEvent::finish()});
}
