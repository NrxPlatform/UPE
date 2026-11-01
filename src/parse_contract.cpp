#include "upe/parse_contract.hpp"
#include <atomic>
#include <limits>
namespace upe::parse {
namespace {
std::atomic<std::uint64_t> next_session{1};
bool charge(std::uint64_t& counter, std::uint64_t limit) { if (counter >= limit) return false; ++counter; return true; }
}
bool BudgetState::charge_step() { return charge(steps, limits.max_steps); }
bool BudgetState::charge_event() { return charge(events, limits.max_events); }
bool BudgetState::charge_diagnostic() { return charge(diagnostics, limits.max_diagnostics); }
bool BudgetState::enter_rule() {
  if (depth >= limits.max_depth) return false;
  ++depth; if (peak_depth < depth) peak_depth = depth; return true;
}
void BudgetState::exit_rule() { if (depth == 0) throw std::logic_error("rule depth underflow"); --depth; }
SessionContracts::SessionContracts(BudgetLimits limits) : generation_(next_session.fetch_add(1)) { budget_.limits = limits; }
Checkpoint SessionContracts::checkpoint() const {
  Checkpoint mark{cursor_, events_.size(), diagnostics_.size(), context_.size(), choices_.size(), generation_, {}};
  for (const auto& frame : choices_) mark.choice_cuts.push_back(frame.cut);
  return mark;
}
void SessionContracts::restore(Checkpoint mark) {
  if (mark.generation != generation_ || mark.event_mark > events_.size() || mark.diagnostic_mark > diagnostics_.size() || mark.context_mark > context_.size() || mark.choice_depth > choices_.size() || mark.choice_cuts.size() != mark.choice_depth)
    throw std::invalid_argument("invalid/foreign/stale checkpoint");
  cursor_ = mark.cursor;
  events_.resize(mark.event_mark); diagnostics_.resize(mark.diagnostic_mark);
  context_.resize(mark.context_mark); choices_.resize(mark.choice_depth);
  for (std::size_t i = 0; i < mark.choice_depth; ++i) choices_[i].cut = mark.choice_cuts[i];
}
void SessionContracts::advance(std::size_t cursor) { if (cursor < cursor_) throw std::invalid_argument("cursor moved backwards without restore"); cursor_ = cursor; }
void SessionContracts::emit(upe::CstEvent event) {
  if (!budget_.charge_event()) throw std::length_error("event budget exceeded");
  events_.push_back(event);
}
void SessionContracts::diagnose(upe::Diagnostic diagnostic) {
  if (!budget_.charge_diagnostic()) throw std::length_error("diagnostic budget exceeded");
  diagnostics_.push_back(std::move(diagnostic));
}
void SessionContracts::push_context(std::uint64_t value) { context_.push_back(value); }
void SessionContracts::begin_choice() { choices_.push_back({}); }
void SessionContracts::commit_choice() { if (choices_.empty()) throw std::logic_error("cut requires choice"); choices_.back().cut = true; }
bool SessionContracts::current_choice_committed() const { return !choices_.empty() && choices_.back().cut; }
void SessionContracts::end_choice() { if (choices_.empty()) throw std::logic_error("choice underflow"); choices_.pop_back(); }
bool SessionContracts::step() { return budget_.charge_step(); }
bool SessionContracts::enter_rule() { return budget_.enter_rule(); }
void SessionContracts::exit_rule() { budget_.exit_rule(); }
} // namespace upe::parse
