#include "upe/grammar.hpp"
#include "check.hpp"
#include <memory>
#include <stdexcept>
using namespace upe::grammar;
static bool fails(auto callback) { try { callback(); return false; } catch (const std::exception&) { return true; } }
int main() {
  GrammarBuilder g({"pinned", "1.0", "default", "rev-2"});
  auto start = g.declare_rule("start", {7});
  auto terminal = g.literal("ok");
  g.define(start, g.seq({terminal, g.eof()}));
  g.set_entrypoint("main", start);
  const auto report = g.validate();
  CHECK(report.valid());
  auto frozen = g.freeze(report);
  CHECK(frozen->entrypoints().at("main").value == 7);
  CHECK(frozen->rule_definitions().at(7).value < frozen->expressions().size());
  const auto fingerprint = std::string(frozen->fingerprint());
  auto unused = g.token(99);
  (void)unused;
  CHECK(frozen->expressions().size() == 3);
  CHECK(frozen->fingerprint() == fingerprint);
  CHECK(g.freeze()->fingerprint() != fingerprint);
  GrammarBuilder bad({"pinned", "1.0", "default", "rev-2"});
  auto unknown = bad.declare_rule("unknown", {1});
  bad.set_entrypoint("main", unknown);
  CHECK(bad.validate().contains("GRAM-02"));
  CHECK(fails([&] { (void)bad.freeze(); }));
  GrammarBuilder a({"pinned", "1.0", "default", "rev-2"});
  auto ar = a.declare_rule("root", {1});
  a.define(ar, a.token(1)); a.set_entrypoint("main", ar);
  GrammarBuilder b({"pinned", "1.0", "default", "rev-2"});
  auto br = b.declare_rule("root", {1});
  b.define(br, b.token(1)); b.set_entrypoint("main", br);
  CHECK(a.freeze()->fingerprint() == b.freeze()->fingerprint());
  b.register_native({{2}, "other", false, true, NativeSideEffects::none,
                     NativeContextPolicy::none, true, true, true});
  CHECK(b.freeze()->fingerprint() != a.freeze()->fingerprint());
}
