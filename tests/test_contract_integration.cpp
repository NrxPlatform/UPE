#include "upe/grammar.hpp"
#include "upe/parse_contract.hpp"
#include "upe/cst.hpp"
#include "check.hpp"
#include <string>
#include <vector>

int main() {
  // Integration: source bytes -> lexical elements -> speculative session events
  // -> immutable CST reconstruction, including invalid bytes and zero-width missing tokens.
  const std::string bytes("\xEF\xBB\xBF" "ab\xFF\r\n", 8);
  auto source = upe::SourceSnapshot::create(bytes, "integration", 7);
  const upe::TokenIdentity kind{10, 1, 2};
  const std::vector<upe::LexicalElement> leaves{
    {source, {0, 3}, kind, upe::ElementClass::trivia},
    {source, {3, 5}, kind, upe::ElementClass::token},
    {source, {5, 6}, kind, upe::ElementClass::error},
    {source, {6, 6}, kind, upe::ElementClass::missing},
    {source, {6, 8}, kind, upe::ElementClass::trivia}
  };
  CHECK(upe::covers_source(leaves, *source));
  upe::parse::SessionContracts session;
  session.emit(upe::CstEvent::start(20));
  session.emit(upe::CstEvent::leaf(0));
  session.begin_choice();
  const auto attempt = session.checkpoint();
  session.advance(4);
  session.emit(upe::CstEvent::leaf(2));
  session.diagnose({});
  session.commit_choice();
  CHECK(session.current_choice_committed());
  session.restore(attempt);
  CHECK(!session.current_choice_committed());
  CHECK(session.cursor() == 0 && session.diagnostics().empty());
  CHECK(session.events().size() == 2);
  session.emit(upe::CstEvent::leaf(1));
  session.emit(upe::CstEvent::leaf(2));
  session.emit(upe::CstEvent::leaf(3));
  session.emit(upe::CstEvent::leaf(4));
  session.emit(upe::CstEvent::finish());
  session.end_choice();
  auto tree = upe::FlatGreenTree::build(source, leaves, session.events());
  CHECK(tree.reconstruct() == bytes);
  CHECK(tree.width() == source->size());
  CHECK(tree.leaf_range(3).begin == tree.leaf_range(3).end);
  CHECK(session.budget().events == 8);  // rejected speculative event stays charged
  CHECK(session.budget().diagnostics == 1);
}
