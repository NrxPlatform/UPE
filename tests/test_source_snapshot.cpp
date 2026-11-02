#include "upe/source.hpp"
#include "check.hpp"
#include <string>
int main() {
  std::string raw("\xef\xbb\xbf" "A\0B\xff", 7);
  auto source = upe::SourceSnapshot::create(raw, "fixture", 3);
  CHECK(source->bytes() == raw);
  CHECK(source->size() == 7);
  CHECK(source->identity() == "fixture" && source->revision() == 3);
  CHECK(source->view({0, 7}) == raw);
  CHECK(source->view({7, 7}).empty());
  bool failed = false;
  try { (void)source->view({0, 8}); } catch (const std::out_of_range&) { failed = true; }
  CHECK(failed);
}
