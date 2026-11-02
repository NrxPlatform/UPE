#include "upe/source.hpp"
#include "check.hpp"
int main() {
  auto source = upe::SourceSnapshot::create("0123456789");
  upe::SourceSlice slice(source, {2, 8});
  CHECK(slice.bytes() == "234567");
  CHECK(slice.sub_slice({1, 3}).bytes() == "34");
  CHECK(slice.sub_slice({6, 6}).bytes().empty());
  bool failed = false;
  try { (void)slice.sub_slice({0, 7}); } catch (const std::out_of_range&) { failed = true; }
  CHECK(failed);
}
