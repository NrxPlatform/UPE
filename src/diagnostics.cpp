#include "upe/diagnostics.hpp"
#include <stdexcept>
#include <utility>
namespace upe {
static void check(const DiagnosticLocation& loc) {
  if(!loc.source) throw std::invalid_argument("diagnostic requires source");
  (void)loc.source->view(loc.range);
}
void DiagnosticBag::add(Diagnostic d) {
  if(d.code.empty()) throw std::invalid_argument("diagnostic requires stable code");
  check(d.primary);
  for(const auto& related : d.related) check(related.location);
  entries_.push_back(std::move(d));
}
bool DiagnosticBag::has_errors() const noexcept {
  for(const auto& d:entries_) if(d.severity==Severity::error || d.severity==Severity::fatal) return true;
  return false;
}
}
