#include "upe/diagnostics.hpp"
#include "check.hpp"
using namespace upe;
int main() {
  auto s=SourceSnapshot::create("x");
  DiagnosticBag bag;
  bag.add({"UPE_LEX_001",Severity::warning,{s,{0,1}},"message",{}});
  CHECK(!bag.has_errors() && bag.entries().size()==1);
  bag.add({"UPE_LEX_002",Severity::error,{s,{1,1}},"",{{{s,{0,1}},"related"}}});
  CHECK(bag.has_errors() && bag.entries()[1].related.size()==1);
  bool rejected=false;
  try {bag.add({"",Severity::error,{s,{0,0}},"",{}});}catch(const std::invalid_argument&){rejected=true;}
  CHECK(rejected);
}
