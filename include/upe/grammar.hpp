#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace upe::grammar {
struct RuleId { std::uint32_t value{}; };
struct ExprId { std::uint32_t value{}; };
struct NativeRuleId { std::uint32_t value{}; };
struct RuleHandle { RuleId id; std::uint64_t owner{}; };
struct LanguageKey { std::string language, version, profile, revision; };
enum class ExpressionKind { token, literal, sequence, ordered_choice, optional, repeat, one_or_more, and_predicate, not_predicate, reference, eof, cut, node, native_rule };
struct Expression {
  ExpressionKind kind{};
  std::vector<ExprId> operands;
  std::uint32_t kind_id{}; // token, syntax or native id depending on expression
  RuleHandle reference{};
  std::string bytes;
};
// Native functions are not executed by this API. These are static promises that
// the Phase 2 dispatcher must enforce at the session boundary.
enum class NativeSideEffects { none, transactional, external };
enum class NativeContextPolicy { none, checkpointed, external };
struct NativeRuleDescriptor {
  NativeRuleId id;
  std::string stable_name;
  bool may_match_empty{};
  bool consumes_input_contract{true};
  NativeSideEffects side_effects{NativeSideEffects::none};
  NativeContextPolicy context{NativeContextPolicy::none};
  bool cancellation_safe{true};
  bool supports_rollback{true};
  bool memoizable{false};
  std::uint32_t declared_lookahead_bytes{}; // 0 = unknown, not an enforced bound
};
struct Issue { enum class Level { warning, error }; Level level; std::string code, message; };
struct ValidationReport {
  std::vector<Issue> issues;
  bool valid() const noexcept;
  bool contains(std::string_view code) const noexcept;
};
class FrozenGrammar final {
public:
  const LanguageKey& language() const noexcept { return language_; }
  const std::vector<Expression>& expressions() const noexcept { return expressions_; }
  std::string_view fingerprint() const noexcept { return fingerprint_; }
  const std::unordered_map<std::string, RuleId>& entrypoints() const noexcept { return entrypoints_; }
  const std::unordered_map<std::uint32_t, ExprId>& rule_definitions() const noexcept { return rule_definitions_; }
  const std::vector<NativeRuleDescriptor>& native_rules() const noexcept { return native_rules_; }
private:
  friend class GrammarBuilder;
  LanguageKey language_;
  std::vector<Expression> expressions_;
  std::unordered_map<std::string, RuleId> entrypoints_;
  std::unordered_map<std::uint32_t, ExprId> rule_definitions_;
  std::vector<NativeRuleDescriptor> native_rules_;
  std::string fingerprint_;
};
class GrammarBuilder final {
public:
  explicit GrammarBuilder(LanguageKey key);
  RuleHandle declare_rule(std::string name, RuleId id);
  void define(RuleHandle rule, ExprId expression);
  void register_native(NativeRuleDescriptor descriptor);
  void set_entrypoint(std::string name, RuleHandle rule);
  ExprId token(std::uint32_t kind);
  ExprId literal(std::string bytes);
  ExprId seq(std::vector<ExprId> operands);
  ExprId choice(std::vector<ExprId> operands);
  ExprId optional(ExprId operand);
  ExprId repeat(ExprId operand);
  ExprId one_or_more(ExprId operand);
  ExprId and_predicate(ExprId operand);
  ExprId not_predicate(ExprId operand);
  ExprId ref(RuleHandle rule);
  ExprId eof();
  ExprId cut();
  ExprId node(std::uint32_t syntax_kind, ExprId operand);
  ExprId native(NativeRuleId id);
  ValidationReport validate() const;
  std::shared_ptr<const FrozenGrammar> freeze(const ValidationReport& ignored = {}) const;
private:
  struct Rule { RuleId id; std::string name; bool defined{}; ExprId definition{}; };
  ExprId add(Expression expr);
  void own(RuleHandle rule) const;
  LanguageKey language_;
  std::uint64_t owner_;
  std::vector<Expression> expressions_;
  std::vector<Rule> rules_;
  std::vector<NativeRuleDescriptor> natives_;
  std::unordered_map<std::string, RuleId> entrypoints_;
};
} // namespace upe::grammar
