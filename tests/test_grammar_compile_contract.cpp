#include "upe/grammar.hpp"
#include "check.hpp"
#include <type_traits>
using namespace upe::grammar;
static_assert(std::is_same_v<decltype(std::declval<GrammarBuilder&>().freeze()),
                             std::shared_ptr<const FrozenGrammar>>);
static_assert(std::is_same_v<decltype(std::declval<GrammarBuilder&>().native(NativeRuleId{1})), ExprId>);
int main() {
  GrammarBuilder g({"compile", "2026", "default", "r1"});
  auto root = g.declare_rule("root", {1});
  g.register_native({{5}, "hook", false, true, NativeSideEffects::none,
                     NativeContextPolicy::none, true, true, true});
  auto atom = g.choice({g.token(1), g.literal("x"), g.native({5})});
  auto guarded = g.seq({g.and_predicate(atom), g.not_predicate(g.literal("!")), atom});
  auto alternative = g.seq({g.cut(), g.optional(guarded)});
  g.define(root, g.node(10, g.seq({g.choice({alternative, g.literal("fallback")}),
         g.repeat(g.token(2)), g.one_or_more(g.token(3)), g.eof()})));
  g.set_entrypoint("main", root);
  CHECK(g.validate().valid());
  auto graph = g.freeze();
  CHECK(!graph->expressions().empty());
  CHECK(graph->native_rules().front().memoizable);
}
