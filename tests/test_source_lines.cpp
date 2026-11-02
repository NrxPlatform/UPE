#include "upe/source.hpp"
#include "check.hpp"
int main() {
  auto source = upe::SourceSnapshot::create("a\r\nb\rc\nd");
  auto a = source->locate(0); CHECK(a.line == 0 && a.byte_column == 0);
  auto b = source->locate(3); CHECK(b.line == 1 && b.byte_column == 0);
  auto c = source->locate(5); CHECK(c.line == 2 && c.byte_column == 0);
  auto d = source->locate(7); CHECK(d.line == 3 && d.byte_column == 0);
  auto end = source->locate(8); CHECK(end.line == 3 && end.byte_column == 1);
}
