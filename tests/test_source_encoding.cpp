#include "upe/encoding.hpp"
#include "check.hpp"
using namespace upe;
using namespace std::literals;
int main() {
  auto s=SourceSnapshot::create(std::string("\xef\xbb\xbf" "A\0\xff",6));
  CHECK(s->size()==6);
  CHECK(detect_bom(s->bytes())==BomKind::utf8);
  CHECK(s->view({0,6})==s->bytes());
  auto r=validate_utf8(s->bytes());
  CHECK(!r.valid && r.first_invalid==5 && r.invalid_sequences==1);
  CHECK(validate_utf8("hello").valid);
  CHECK(!validate_utf8("\xed\xa0\x80").valid); // surrogate
  CHECK(!validate_utf8("\xf4\x90\x80\x80").valid); // > Unicode max
  CHECK(detect_bom("\xff\xfe\0\0"sv)==BomKind::utf32_le);
}
