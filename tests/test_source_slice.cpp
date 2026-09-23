#include "upe/source.hpp"
#include <cassert>

int main() {
  auto source = upe::SourceSnapshot::create("0123456789");
  upe::SourceSlice slice(source, {2, 8});
  assert(slice.bytes() == "234567");
  assert(slice.sub_slice({1, 3}).bytes() == "34");
  assert(slice.sub_slice({6, 6}).bytes().empty());
  bool failed = false;
  try { (void)slice.sub_slice({0, 7}); } catch (const std::out_of_range&) { failed = true; }
  assert(failed);
}
