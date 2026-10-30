#pragma once
#include "upe/source.hpp"
#include <vector>

namespace upe {
enum class Severity { note, warning, error, fatal };
struct DiagnosticLocation { std::shared_ptr<const SourceSnapshot> source; ByteRange range; };
struct RelatedLocation { DiagnosticLocation location; std::string message; };
struct Diagnostic { 
  std::string code; Severity severity{}; 
  DiagnosticLocation primary; 
  std::string message; std::vector<RelatedLocation> related; 
};
class DiagnosticBag final {
public:
  void add(Diagnostic diagnostic);
  const std::vector<Diagnostic>& entries() const noexcept { return entries_; }
  bool has_errors() const noexcept;
private:
  std::vector<Diagnostic> entries_;
};
}
