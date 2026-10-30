#include "upe/encoding.hpp"
#include "check.hpp"
using namespace upe;
int main() {
  auto s=SourceSnapshot::create("a\xf0\x9f\x98\x80\xc3\xa9\r\n" "b\xff");
  auto x=locate_unicode(*s,7);
  CHECK(x.line==0 && x.scalar_column==3 && x.utf16_column==4);
  CHECK(locate_unicode(*s,3).scalar_column==1); // middle of emoji
  CHECK(locate_unicode(*s,9).line==1);
  CHECK(locate_unicode(*s,11).scalar_column==2);
}
