#include "upe/grammar.hpp"
#include <algorithm>
#include <atomic>
#include <functional>
#include <iomanip>
#include <set>
#include <sstream>
#include <stdexcept>
#include <unordered_set>

namespace upe::grammar {
namespace {
std::atomic<std::uint64_t> next_builder{1};
std::string hash_string(const std::string& input) {
  // Reproducible non-cryptographic content fingerprint; versioned by caller.
  std::uint64_t value = 14695981039346656037ULL;
  for (unsigned char c : input) { value ^= c; value *= 1099511628211ULL; }
  std::ostringstream out; out << "fnv1a64-v1:" << std::hex << std::setw(16) << std::setfill('0') << value;
  return out.str();
}
}
bool ValidationReport::valid() const noexcept {
  return std::none_of(issues.begin(), issues.end(), [](const auto& issue) { return issue.level == Issue::Level::error; });
}
bool ValidationReport::contains(std::string_view code) const noexcept {
  return std::any_of(issues.begin(), issues.end(), [&](const auto& issue) { return issue.code == code; });
}
GrammarBuilder::GrammarBuilder(LanguageKey key) : language_(std::move(key)), owner_(next_builder.fetch_add(1)) {}
void GrammarBuilder::own(RuleHandle rule) const {
  if (rule.owner != owner_) throw std::invalid_argument("rule handle belongs to another builder");
}
RuleHandle GrammarBuilder::declare_rule(std::string name, RuleId id) {
  if (name.empty() || id.value == 0) throw std::invalid_argument("rule requires name and nonzero stable id");
  for (const auto& rule : rules_) if (rule.name == name || rule.id.value == id.value) throw std::invalid_argument("duplicate rule name or id");
  rules_.push_back({id, std::move(name), false, {}});
  return {id, owner_};
}
void GrammarBuilder::define(RuleHandle handle, ExprId expression) {
  own(handle);
  if (expression.value >= expressions_.size()) throw std::out_of_range("unknown expression");
  for (auto& rule : rules_) if (rule.id.value == handle.id.value) {
    if (rule.defined) throw std::invalid_argument("rule defined twice");
    rule.defined = true; rule.definition = expression; return;
  }
  throw std::invalid_argument("unknown rule handle");
}
void GrammarBuilder::register_native(NativeRuleDescriptor descriptor) {
  if (descriptor.id.value == 0 || descriptor.stable_name.empty()) throw std::invalid_argument("invalid native descriptor");
  for (const auto& n : natives_) if (n.id.value == descriptor.id.value || n.stable_name == descriptor.stable_name)
    throw std::invalid_argument("duplicate native descriptor");
  natives_.push_back(std::move(descriptor));
}
void GrammarBuilder::set_entrypoint(std::string name, RuleHandle handle) {
  own(handle);
  if (name.empty()) throw std::invalid_argument("empty entrypoint name");
  const auto it = std::find_if(rules_.begin(), rules_.end(), [&](const Rule& r) { return r.id.value == handle.id.value; });
  if (it == rules_.end() || !entrypoints_.emplace(std::move(name), handle.id).second) throw std::invalid_argument("invalid or duplicate entrypoint");
}
ExprId GrammarBuilder::add(Expression expr) {
  if (expressions_.size() >= UINT32_MAX) throw std::length_error("expression id exhausted");
  ExprId id{static_cast<std::uint32_t>(expressions_.size())}; expressions_.push_back(std::move(expr)); return id;
}
ExprId GrammarBuilder::token(std::uint32_t kind) { Expression e; e.kind = ExpressionKind::token; e.kind_id = kind; return add(std::move(e)); }
ExprId GrammarBuilder::literal(std::string bytes) { Expression e; e.kind = ExpressionKind::literal; e.bytes = std::move(bytes); return add(std::move(e)); }
ExprId GrammarBuilder::seq(std::vector<ExprId> operands) { Expression e; e.kind = ExpressionKind::sequence; e.operands = std::move(operands); return add(std::move(e)); }
ExprId GrammarBuilder::choice(std::vector<ExprId> operands) { Expression e; e.kind = ExpressionKind::ordered_choice; e.operands = std::move(operands); return add(std::move(e)); }
#define UNARY(name, k) ExprId GrammarBuilder::name(ExprId operand) { Expression e; e.kind = ExpressionKind::k; e.operands = {operand}; return add(std::move(e)); }
UNARY(optional, optional)
UNARY(repeat, repeat)
UNARY(one_or_more, one_or_more)
UNARY(and_predicate, and_predicate)
UNARY(not_predicate, not_predicate)
#undef UNARY
ExprId GrammarBuilder::ref(RuleHandle rule) { own(rule); Expression e; e.kind = ExpressionKind::reference; e.reference = rule; return add(std::move(e)); }
ExprId GrammarBuilder::eof() { Expression e; e.kind = ExpressionKind::eof; return add(std::move(e)); }
ExprId GrammarBuilder::cut() { Expression e; e.kind = ExpressionKind::cut; return add(std::move(e)); }
ExprId GrammarBuilder::node(std::uint32_t kind, ExprId operand) { Expression e; e.kind = ExpressionKind::node; e.kind_id = kind; e.operands = {operand}; return add(std::move(e)); }
ExprId GrammarBuilder::native(NativeRuleId id) { Expression e; e.kind = ExpressionKind::native_rule; e.kind_id = id.value; return add(std::move(e)); }
ValidationReport GrammarBuilder::validate() const {
  ValidationReport report;
  auto error = [&](std::string code, std::string msg) { report.issues.push_back({Issue::Level::error, std::move(code), std::move(msg)}); };
  auto warning = [&](std::string code, std::string msg) { report.issues.push_back({Issue::Level::warning, std::move(code), std::move(msg)}); };
  if (language_.language.empty() || language_.version.empty() || language_.profile.empty() || language_.revision.empty()) error("GRAM-09", "language/version/profile/revision must be pinned");
  if (entrypoints_.empty()) error("GRAM-02", "no entrypoints");
  std::unordered_map<std::uint32_t, ExprId> definitions;
  for (const auto& rule : rules_) {
    if (!rule.defined) error("GRAM-02", "undefined rule: " + rule.name);
    else definitions.emplace(rule.id.value, rule.definition);
  }
  // A native descriptor is an explicit promise about possible parser state.
  // This is validation only: the Phase 2 executor must enforce these promises.
  for (const auto& n : natives_) {
    if (n.side_effects == NativeSideEffects::external)
      error("GRAM-11", "external native effects cannot be backtracked safely: " + n.stable_name);
    if (!n.cancellation_safe)
      error("GRAM-11", "native must support cancellation: " + n.stable_name);
    if ((n.side_effects == NativeSideEffects::transactional ||
         n.context == NativeContextPolicy::checkpointed) && !n.supports_rollback)
      error("GRAM-11", "stateful native must support rollback: " + n.stable_name);
    if (n.context == NativeContextPolicy::external)
      error("GRAM-11", "external native context cannot be checkpointed: " + n.stable_name);
    if (n.memoizable && (n.side_effects != NativeSideEffects::none ||
                         n.context != NativeContextPolicy::none || !n.supports_rollback))
      error("GRAM-12", "memoization requires pure, context-free, rollback-safe native: " + n.stable_name);
  }
  for (std::size_t i = 0; i < expressions_.size(); ++i) {
    const auto& expr = expressions_[i];
    for (const auto operand : expr.operands) if (operand.value >= expressions_.size()) error("GRAM-02", "invalid expression operand at " + std::to_string(i));
    if (expr.kind == ExpressionKind::reference && (!definitions.contains(expr.reference.id.value) || expr.reference.owner != owner_)) error("GRAM-02", "undefined/foreign rule reference");
    if ((expr.kind == ExpressionKind::token || expr.kind == ExpressionKind::node) && expr.kind_id == 0) error("GRAM-07", "zero kind id");
    if (expr.kind == ExpressionKind::native_rule && std::none_of(natives_.begin(), natives_.end(), [&](const auto& n) { return n.id.value == expr.kind_id; })) error("GRAM-07", "unregistered native rule");
    if (expr.kind == ExpressionKind::literal && expr.bytes.empty()) error("GRAM-07", "empty literal");
    if ((expr.kind == ExpressionKind::ordered_choice || expr.kind == ExpressionKind::sequence) && expr.operands.empty()) error("GRAM-02", "empty sequence or choice");
    if (expr.kind == ExpressionKind::native_rule) for (const auto& n : natives_) if (n.id.value == expr.kind_id && !n.consumes_input_contract) error("GRAM-08", "native rule lacks source contract");
  }
  // Conservative least-fixed-point nullability. Unknown native hooks treated nullable when declared so.
  std::vector<bool> nullable(expressions_.size(), false);
  auto may_empty = [&](const Expression& e) {
    const auto child = [&](std::size_t i) { return i < e.operands.size() && e.operands[i].value < nullable.size() && nullable[e.operands[i].value]; };
    switch (e.kind) {
      case ExpressionKind::eof: case ExpressionKind::cut: case ExpressionKind::optional: case ExpressionKind::repeat:
      case ExpressionKind::and_predicate: case ExpressionKind::not_predicate: return true;
      case ExpressionKind::sequence: return std::all_of(e.operands.begin(), e.operands.end(), [&](ExprId id) { return id.value < nullable.size() && nullable[id.value]; });
      case ExpressionKind::ordered_choice: return std::any_of(e.operands.begin(), e.operands.end(), [&](ExprId id) { return id.value < nullable.size() && nullable[id.value]; });
      case ExpressionKind::node: case ExpressionKind::one_or_more: return child(0);
      case ExpressionKind::reference: { const auto it = definitions.find(e.reference.id.value); return it != definitions.end() && it->second.value < nullable.size() && nullable[it->second.value]; }
      case ExpressionKind::native_rule: { const auto it = std::find_if(natives_.begin(), natives_.end(), [&](const auto& n) { return n.id.value == e.kind_id; }); return it != natives_.end() && it->may_match_empty; }
      default: return false;
    }
  };
  for (std::size_t pass = 0; pass <= expressions_.size(); ++pass) {
    bool changed = false;
    for (std::size_t i = 0; i < expressions_.size(); ++i) if (!nullable[i] && may_empty(expressions_[i])) { nullable[i] = true; changed = true; }
    if (!changed) break;
  }
  for (const auto& expr : expressions_) if ((expr.kind == ExpressionKind::repeat || expr.kind == ExpressionKind::one_or_more) && !expr.operands.empty() && expr.operands.front().value < nullable.size() && nullable[expr.operands.front().value]) error("GRAM-05", "nullable repeated operand");
  // First-call graph detects recursion through nullable prefixes and predicates.
  std::unordered_map<std::uint32_t, std::set<std::uint32_t>> edges;
  std::function<void(ExprId, std::set<std::uint32_t>&, std::set<std::uint32_t>&)> first;
  first = [&](ExprId id, std::set<std::uint32_t>& targets, std::set<std::uint32_t>& visited) {
    if (id.value >= expressions_.size() || !visited.insert(id.value).second) return;
    const auto& e = expressions_[id.value];
    if (e.kind == ExpressionKind::reference) { targets.insert(e.reference.id.value); return; }
    if (e.kind == ExpressionKind::sequence) {
      for (const auto child : e.operands) { first(child, targets, visited); if (child.value >= nullable.size() || !nullable[child.value]) break; }
    } else for (const auto child : e.operands) first(child, targets, visited);
  };
  for (const auto& rule : rules_) if (rule.defined) { std::set<std::uint32_t> visited; first(rule.definition, edges[rule.id.value], visited); }
  std::unordered_map<std::uint32_t, int> state;
  std::function<void(std::uint32_t)> dfs = [&](std::uint32_t id) {
    state[id] = 1;
    for (auto to : edges[id]) if (definitions.contains(to)) {
      if (state[to] == 1) error("GRAM-04", "left recursion detected at rule " + std::to_string(to));
      else if (state[to] == 0) dfs(to);
    }
    state[id] = 2;
  };
  for (const auto& rule : rules_) if (!state[rule.id.value]) dfs(rule.id.value);
  // Cut scope is lexical in the expression graph; references are validated separately at runtime.
  std::function<void(ExprId, bool, std::set<std::uint64_t>&)> cuts;
  cuts = [&](ExprId id, bool in_choice, std::set<std::uint64_t>& visited) {
    if (id.value >= expressions_.size()) return;
    const auto key = (static_cast<std::uint64_t>(id.value) << 1) | static_cast<unsigned>(in_choice);
    if (!visited.insert(key).second) return;
    const auto& e = expressions_[id.value];
    if (e.kind == ExpressionKind::cut && !in_choice) error("GRAM-06", "cut without choice scope");
    for (auto child : e.operands) cuts(child, in_choice || e.kind == ExpressionKind::ordered_choice, visited);
    if (e.kind == ExpressionKind::reference) { const auto it = definitions.find(e.reference.id.value); if (it != definitions.end()) cuts(it->second, in_choice, visited); }
  };
  for (const auto& entry : entrypoints_) { const auto it = definitions.find(entry.second.value); if (it != definitions.end()) { std::set<std::uint64_t> visited; cuts(it->second, false, visited); } }
  std::set<std::uint32_t> reached;
  std::function<void(std::uint32_t)> visit = [&](std::uint32_t id) {
    if (!reached.insert(id).second) return;
    for (auto target : edges[id]) if (definitions.contains(target)) visit(target);
  };
  for (const auto& e : entrypoints_) visit(e.second.value);
  for (const auto& rule : rules_) if (!reached.contains(rule.id.value)) warning("GRAM-10", "unreachable rule: " + rule.name);
  return report;
}
std::shared_ptr<const FrozenGrammar> GrammarBuilder::freeze(const ValidationReport&) const {
  const auto report = validate();
  if (!report.valid()) throw std::logic_error("cannot freeze invalid grammar");
  auto frozen = std::make_shared<FrozenGrammar>();
  frozen->language_ = language_; frozen->expressions_ = expressions_;
  frozen->entrypoints_ = entrypoints_; frozen->native_rules_ = natives_;
  for (const auto& rule : rules_) frozen->rule_definitions_.emplace(rule.id.value, rule.definition);
  std::ostringstream signature;
  signature << language_.language << '\0' << language_.version << '\0' << language_.profile << '\0' << language_.revision << '\0';
  for (const auto& r : rules_) signature << "R" << r.id.value << ':' << r.name << ':' << r.definition.value << ';';
  for (const auto& n : natives_) signature << "H" << n.id.value << ':' << n.stable_name << ':' << n.may_match_empty << ':' << n.consumes_input_contract
    << ':' << static_cast<int>(n.side_effects) << ':' << static_cast<int>(n.context)
    << ':' << n.cancellation_safe << ':' << n.supports_rollback << ':'
    << n.memoizable << ':' << n.declared_lookahead_bytes << ';';
  for (const auto& e : expressions_) {
    signature << "E" << static_cast<int>(e.kind) << ':' << e.kind_id << ':' << e.reference.id.value << ':' << e.bytes.size() << ':' << e.bytes << ':';
    for (auto operand : e.operands) signature << operand.value << ',';
    signature << ';';
  }
  std::vector<std::pair<std::string, RuleId>> sorted(entrypoints_.begin(), entrypoints_.end());
  std::sort(sorted.begin(), sorted.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
  for (const auto& entry : sorted) signature << "P" << entry.first << ':' << entry.second.value << ';';
  frozen->fingerprint_ = hash_string(signature.str());
  return frozen;
}
} // namespace upe::grammar
