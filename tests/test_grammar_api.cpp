#include "upe/grammar.hpp"
#include "check.hpp"
using namespace upe::grammar;
int main() {
  GrammarBuilder g({"json", "RFC8259", "document", "grammar-v1"});
  auto document = g.declare_rule("document", {1});
  auto value = g.declare_rule("value", {2});
  g.register_native({{1}, "parse_value", false, true});
  const auto native_expr = g.native({1});
  auto a = g.token(1);
  auto b = g.literal("x");
  auto alternatives = g.choice({a,b});
  auto group = g.node(5, g.seq({g.and_predicate(alternatives), g.not_predicate(g.token(3)), g.optional(a), g.repeat(a), g.one_or_more(a), g.ref(value)}));
  // All supported API constructors compile; unreachable expressions are permitted at this stage.
  (void)group;
  g.define(value, native_expr);
  g.define(document, g.seq({g.ref(value), g.eof()}));
  g.set_entrypoint("document", document);
  CHECK(g.validate().valid());
  const auto frozen = g.freeze();
  CHECK(!frozen->fingerprint().empty());
  CHECK(frozen->entrypoints().size() == 1);
  GrammarBuilder copy({"json", "RFC8259", "document", "grammar-v1"});
  auto v2 = copy.declare_rule("document", {1});
  auto vv = copy.declare_rule("value", {2});
  copy.register_native({{1}, "parse_value", false, true});
  copy.define(vv, copy.native({1}));
  copy.define(v2, copy.seq({copy.ref(vv), copy.eof()}));
  copy.set_entrypoint("document", v2);
  CHECK(g.freeze()->fingerprint() != copy.freeze()->fingerprint()); // graph differs; identical builds are tested separately
}
