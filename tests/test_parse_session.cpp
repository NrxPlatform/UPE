#include "upe/parse_contract.hpp"
#include "check.hpp"
#include <stdexcept>
using namespace upe::parse;
template <typename F> bool throws(F f) { try { f(); return false; } catch (const std::exception&) { return true; } }
int main() {
  SessionContracts s({3, 2, 2, 1});
  auto mark = s.checkpoint();
  CHECK(s.step()); CHECK(s.enter_rule()); CHECK(!s.enter_rule()); s.exit_rule();
  s.advance(5);
  s.emit(upe::CstEvent::start(1));
  s.begin_choice(); s.commit_choice(); CHECK(s.current_choice_committed());
  s.diagnose({});
  s.restore(mark);
  CHECK(s.cursor() == 0 && s.events().empty() && s.diagnostics().empty());
  CHECK(s.budget().events == 1 && s.budget().diagnostics == 1 && s.budget().steps == 1);
  CHECK(!s.current_choice_committed());
  CHECK(throws([&]{ s.commit_choice(); }));
  s.begin_choice();
  const auto inside = s.checkpoint();
  s.commit_choice(); CHECK(s.current_choice_committed());
  s.restore(inside); CHECK(!s.current_choice_committed());
  s.end_choice();
  SessionContracts foreign;
  CHECK(throws([&]{ foreign.restore(mark); }));
  s.emit(upe::CstEvent::finish());
  CHECK(throws([&]{ s.emit(upe::CstEvent::finish()); }));
  CHECK(s.step()); CHECK(s.step()); CHECK(!s.step());
}
