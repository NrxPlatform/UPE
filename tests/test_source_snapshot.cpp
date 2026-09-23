#include "upe/source.hpp"
#include <cassert>
#include <string>

int main() {
  std::string raw("\xef\xbb\xbf" "A\0B\xff", 7);
  auto source = upe::SourceSnapshot::create(raw, "fixture", 3);
  assert(source->bytes() == raw);
  assert(source->size() == 7);
  assert(source->identity() == "fixture" && source->revision() == 3);
  assert(source->view({0, 7}) == raw);
  assert(source->view({7, 7}).empty());
  bool failed = false;
  try { (void)source->view({0, 8}); } catch (const std::out_of_range&) { failed = true; }
  assert(failed);
}
