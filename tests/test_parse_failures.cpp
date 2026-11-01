#include "upe/parse_contract.hpp"
#include "check.hpp"
using namespace upe::parse;
int main() {
  SessionContracts s({100,100,2,10});
  auto cp = s.checkpoint();
  s.record_failure(5, "identifier");
  s.record_failure(3, "number");
  s.record_failure(5, "identifier");
  s.record_failure(5, "string");
  s.diagnose({});
  s.restore(cp);
  CHECK(s.diagnostics().empty());
  CHECK(s.farthest_failure()->byte_offset == 5);
  CHECK(s.farthest_failure()->expected.size() == 2);
  s.commit_failure({});
  s.restore(cp);
  CHECK(s.committed_diagnostics().size() == 1);
  CHECK(s.budget().diagnostics == 2);
  bool exhausted = false;
  try { s.commit_failure({}); } catch (const std::length_error&) { exhausted = true; }
  CHECK(exhausted);
  s.clear_failure_tracking();
  CHECK(!s.farthest_failure() && s.committed_diagnostics().empty());
  CHECK(s.budget().diagnostics == 2);
}
