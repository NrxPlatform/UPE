#include "upe/grammar.hpp"
#include "check.hpp"
#include <memory>
#include <string>

using namespace upe::grammar;

static std::shared_ptr<const FrozenGrammar> build_example() {
  GrammarBuilder grammar({"example", "1", "module", "contract-1"});
  auto document = grammar.declare_rule("document", {1});
  auto atom = grammar.declare_rule("atom", {2});
  grammar.register_native({{1}, "contextual_atom", false, true});
  const auto digit = grammar.token(1);
  const auto word = grammar.literal("word");
  const auto native = grammar.native({1});
  grammar.define(atom, grammar.choice({digit, word, native}));
  // All P1.3 constructors compile together, including predicates, cut,
  // repetitions, syntax nodes, references, and EOF.
  const auto guarded = grammar.seq({grammar.and_predicate(grammar.ref(atom)),
                                     grammar.not_predicate(grammar.literal("!")),
                                     grammar.ref(atom)});
  const auto alternative = grammar.seq({grammar.cut(), grammar.optional(guarded)});
  const auto body = grammar.node(100, grammar.seq({
    grammar.choice({alternative, grammar.literal("fallback")}),
    grammar.repeat(grammar.token(2)),
    grammar.one_or_more(grammar.token(3)),
    grammar.eof()}));
  grammar.define(document, body);
  grammar.set_entrypoint("module", document);
  CHECK(grammar.validate().valid());
  return grammar.freeze();
}
int main() {
  const auto a = build_example();
  const auto b = build_example();
  CHECK(a->fingerprint() == b->fingerprint());
  CHECK(a->entrypoints().size() == 1);
  CHECK(!a->expressions().empty());
}
