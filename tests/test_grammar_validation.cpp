#include "upe/grammar.hpp"
#include "check.hpp"
#include <stdexcept>
using namespace upe::grammar;
template <typename F> bool throws(F f) { try { f(); return false; } catch (const std::exception&) { return true; } }
int main() {
  GrammarBuilder g({"test", "1", "default", "r1"});
  auto r = g.declare_rule("r", {1});
  CHECK(throws([&]{ g.declare_rule("again", {1}); }));
  CHECK(g.validate().contains("GRAM-02"));
  GrammarBuilder foreign({"test", "1", "default", "r1"});
  CHECK(throws([&]{ foreign.ref(r); }));
  g.define(r, g.repeat(g.optional(g.token(2))));
  g.set_entrypoint("main", r);
  CHECK(g.validate().contains("GRAM-05"));
  CHECK(throws([&]{ (void)g.freeze(); }));
  GrammarBuilder recursion({"test", "1", "default", "r1"});
  auto x = recursion.declare_rule("x", {1});
  auto y = recursion.declare_rule("y", {2});
  recursion.define(x, recursion.ref(y));
  recursion.define(y, recursion.ref(x));
  recursion.set_entrypoint("main", x);
  CHECK(recursion.validate().contains("GRAM-04"));
  GrammarBuilder cut({"test", "1", "default", "r1"});
  auto c = cut.declare_rule("c", {1});
  cut.define(c, cut.seq({cut.cut(), cut.token(1)}));
  cut.set_entrypoint("main", c);
  CHECK(cut.validate().contains("GRAM-06"));
  GrammarBuilder fine({"test", "1", "default", "r1"});
  auto f = fine.declare_rule("f", {1});
  fine.define(f, fine.choice({fine.seq({fine.token(1), fine.cut(), fine.token(2)}), fine.token(3)}));
  fine.set_entrypoint("main", f);
  CHECK(fine.validate().valid());
}
