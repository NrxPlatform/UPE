#pragma once
#include "upe/cst.hpp"
#include "upe/diagnostics.hpp"
#include <cstdint>
#include <stdexcept>
#include <vector>
namespace upe::parse {
enum class Status { success, recovered, no_match, committed_failure, limit_exceeded, cancelled, internal_error };
struct Outcome { Status status{Status::no_match}; std::uint64_t token_cursor{}; bool complete{}; };
struct BudgetLimits { std::uint64_t max_steps{1'000'000}, max_events{1'000'000}, max_diagnostics{10'000}, max_depth{1'024}; };
struct BudgetState {
  BudgetLimits limits;
  std::uint64_t steps{}, events{}, diagnostics{}, depth{}, peak_depth{};
  bool charge_step();
  bool charge_event();
  bool charge_diagnostic();
  bool enter_rule();
  void exit_rule();
};
struct Checkpoint {
  std::size_t cursor{}, event_mark{}, diagnostic_mark{}, context_mark{}, choice_depth{};
  std::uint64_t generation{};
  std::vector<bool> choice_cuts;
};
struct ChoiceFrame { bool cut{}; };
// Cursor and contextual state are deliberately local to a session; no executor is supplied yet.
class SessionContracts final {
public:
  explicit SessionContracts(BudgetLimits limits = {});
  Checkpoint checkpoint() const;
  void restore(Checkpoint mark); // rollback only speculative state; budget charges never refunded
  void advance(std::size_t cursor);
  std::size_t cursor() const noexcept { return cursor_; }
  void emit(upe::CstEvent event);
  void diagnose(upe::Diagnostic diagnostic);
  void push_context(std::uint64_t value);
  void begin_choice();
  void commit_choice(); // PEG cut: nearest dynamically enclosing choice
  bool current_choice_committed() const;
  void end_choice();
  const std::vector<upe::CstEvent>& events() const noexcept { return events_; }
  const std::vector<upe::Diagnostic>& diagnostics() const noexcept { return diagnostics_; }
  const BudgetState& budget() const noexcept { return budget_; }
  bool step();
  bool enter_rule();
  void exit_rule();
private:
  std::uint64_t generation_{};
  std::size_t cursor_{};
  std::vector<upe::CstEvent> events_;
  std::vector<upe::Diagnostic> diagnostics_;
  std::vector<std::uint64_t> context_;
  std::vector<ChoiceFrame> choices_;
  BudgetState budget_;
};
} // namespace upe::parse
