#include "upe/encoding.hpp"
#include <stdexcept>
namespace upe {
BomKind detect_bom(std::string_view s) noexcept {
  const auto b = [&](std::size_t i) { return static_cast<unsigned char>(s[i]); };
  if(s.size()>=4 && b(0)==0xff && b(1)==0xfe && b(2)==0 && b(3)==0) return BomKind::utf32_le;
  if(s.size()>=4 && b(0)==0 && b(1)==0 && b(2)==0xfe && b(3)==0xff) return BomKind::utf32_be;
  if(s.size()>=3 && b(0)==0xef && b(1)==0xbb && b(2)==0xbf) return BomKind::utf8;
  if(s.size()>=2 && b(0)==0xff && b(1)==0xfe) return BomKind::utf16_le;
  if(s.size()>=2 && b(0)==0xfe && b(1)==0xff) return BomKind::utf16_be;
  return BomKind::none;
}
namespace {
struct Decoded { char32_t value; std::size_t width; bool valid; };
Decoded decode(std::string_view s, std::size_t pos) noexcept {
  const auto a = static_cast<unsigned char>(s[pos]);
  if(a<0x80) return {a,1,true};
  std::size_t width=0; char32_t value=0; char32_t min=0;
  if(a>=0xc2 && a<=0xdf) {width=2;value=a&0x1f;min=0x80;}
  else if(a>=0xe0 && a<=0xef) {width=3;value=a&0x0f;min=0x800;}
  else if(a>=0xf0 && a<=0xf4) {width=4;value=a&0x07;min=0x10000;}
  else return {0xfffd,1,false};
  if(pos+width>s.size()) return {0xfffd,1,false};
  for(std::size_t i=1;i<width;++i) {
    const auto c=static_cast<unsigned char>(s[pos+i]);
    if((c&0xc0)!=0x80) return {0xfffd,1,false};
    value=(value<<6)|(c&0x3f);
  }
  if(value<min || value>0x10ffff || (value>=0xd800 && value<=0xdfff)) return {0xfffd,1,false};
  return {value,width,true};
}
}

Utf8Validation validate_utf8(std::string_view s) noexcept {
  Utf8Validation r;
  for(std::size_t i=0;i<s.size();) {
    auto d=decode(s,i);
    if(!d.valid) {if(r.valid) r.first_invalid=i; r.valid=false; ++r.invalid_sequences;}
    i+=d.width;
  }
  return r;
}
UnicodeLocation locate_unicode(const SourceSnapshot& source, ByteOffset offset) {
  const auto loc=source.locate(offset); // performs bounds checking
  const auto start=offset-loc.byte_column;
  const auto view=source.bytes();
  UnicodeLocation r{loc.line,0,0};
  for(auto i=static_cast<std::size_t>(start);i<offset;) {
    auto d=decode(view,i);
    // Offsets in the middle of a code point don't count that code point.
    if(i+d.width>offset) break;
    ++r.scalar_column;
    r.utf16_column+=(d.value>0xffff?2:1);
    i+=d.width;
  }
  return r;
}
} // namespace upe
