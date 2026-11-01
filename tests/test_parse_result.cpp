#include "upe/parse_contract.hpp"
#include "check.hpp"
#include <stdexcept>
using namespace upe::parse;
template<class F> bool invalid(F fn) { try { fn(); return false; } catch (const std::invalid_argument&) { return true; } }
int main() {
  auto ok = make_outcome(Status::success, 7, true);
  CHECK(ok.complete && ok.token_cursor == 7 && !ok.failure);
  auto limited = make_outcome(Status::limit_exceeded, 3, false, FailureDetail{FailureReason::budget_steps, 12, "step cap"});
  CHECK(limited.failure->offset == 12);
  CHECK(invalid([]{ (void)make_outcome(Status::limit_exceeded, 0, false); }));
  CHECK(invalid([]{ (void)make_outcome(Status::success, 0, false); }));
  CHECK(invalid([]{ (void)make_outcome(Status::committed_failure, 0, false, FailureDetail{FailureReason::unexpected_input, 0, ""}); }));
  auto unsupported = make_outcome(Status::internal_error, 0, false, FailureDetail{FailureReason::unsupported_configuration, 0, "unsupported"});
  CHECK(unsupported.failure->reason == FailureReason::unsupported_configuration);
}
