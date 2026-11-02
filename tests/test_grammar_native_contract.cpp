#include "upe/grammar.hpp"
#include "check.hpp"
#include <stdexcept>
using namespace upe::grammar;
static GrammarBuilder make(NativeRuleDescriptor descriptor) {
  GrammarBuilder g({"native", "1", "default", "r1"});
  auto r = g.declare_rule("root", {1});
  g.register_native(descriptor);
  g.define(r, g.native(descriptor.id));
  g.set_entrypoint("root", r);
  return g;
}
int main() {
  auto pure = make({{1}, "pure", false, true, NativeSideEffects::none,
                    NativeContextPolicy::none, true, true, true, 4});
  CHECK(pure.validate().valid());
  CHECK(pure.freeze()->native_rules().size() == 1);
  auto transactional = make({{1}, "stateful", false, true, NativeSideEffects::transactional,
                             NativeContextPolicy::checkpointed, true, true, false});
  CHECK(transactional.validate().valid());
  auto unsafe = make({{1}, "external", false, true, NativeSideEffects::external});
  CHECK(unsafe.validate().contains("GRAM-11"));
  auto unsafe_context = make({{1}, "external_context", false, true, NativeSideEffects::none,
                              NativeContextPolicy::external});
  CHECK(unsafe_context.validate().contains("GRAM-11"));
  auto unsafe_cancel = make({{1}, "not_cancel_safe", false, true, NativeSideEffects::none,
                             NativeContextPolicy::none, false});
  CHECK(unsafe_cancel.validate().contains("GRAM-11"));
  auto unsafe_rollback = make({{1}, "no_rollback", false, true, NativeSideEffects::transactional,
                               NativeContextPolicy::checkpointed, true, false});
  CHECK(unsafe_rollback.validate().contains("GRAM-11"));
  auto bad_memo = make({{1}, "unsafe_memo", false, true, NativeSideEffects::transactional,
                        NativeContextPolicy::checkpointed, true, true, true});
  CHECK(bad_memo.validate().contains("GRAM-12"));
  auto no_source = make({{1}, "no_source", false, false});
  CHECK(no_source.validate().contains("GRAM-08"));
  auto empty = make({{1}, "nullable", true});
  CHECK(empty.validate().valid());
  auto repeated = GrammarBuilder({"native", "1", "default", "r1"});
  auto r = repeated.declare_rule("root", {1});
  repeated.register_native({{1}, "nullable", true});
  repeated.define(r, repeated.repeat(repeated.native({1})));
  repeated.set_entrypoint("root", r);
  CHECK(repeated.validate().contains("GRAM-05"));
}
