// Phase 0 smoke: a standalone consumer of UPE libraries, not a PEG parser.
#include "upe/source.hpp"
#include "upe/grammar.hpp"
#include "upe/cst.hpp"
#include "upe/lexical.hpp"
#include <iostream>
#include <string>
#include <vector>

int main() {
  const std::string bytes = "smoke";
  auto source = upe::SourceSnapshot::create(bytes, "smoke", 1);
  const upe::TokenIdentity identity{1, 1, 1};
  const std::vector<upe::LexicalElement> elements{
      {source, {0, bytes.size()}, identity, upe::ElementClass::token}};
  const std::vector<upe::CstEvent> events{
      upe::CstEvent::start(1), upe::CstEvent::leaf(0), upe::CstEvent::finish()};
  const auto tree = upe::FlatGreenTree::build(source, elements, events);
  upe::grammar::GrammarBuilder builder({"smoke", "0", "contract", "1"});
  const auto root = builder.declare_rule("root", {1});
  builder.define(root, builder.literal(bytes));
  builder.set_entrypoint("root", root);
  const auto grammar = builder.freeze();
  if (!grammar || tree.reconstruct() != bytes) return 1;
  std::cout << "UPE contract smoke OK\n";
  return 0;
}
