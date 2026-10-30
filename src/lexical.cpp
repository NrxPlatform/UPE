#include "upe/lexical.hpp"
#include <stdexcept>
#include <utility>
namespace upe {
LexicalElement::LexicalElement(std::shared_ptr<const SourceSnapshot> source, ByteRange range,
                               TokenIdentity kind, ElementClass type)
  : source_(std::move(source)), range_(range), kind_(kind), type_(type) {
  if(!source_) throw std::invalid_argument("null lexical source");
  (void)source_->view(range_);
  if(type_==ElementClass::missing ? range.begin!=range.end : range.begin==range.end)
    throw std::invalid_argument("invalid lexical element width");
}
bool covers_source(std::span<const LexicalElement> elements, const SourceSnapshot& source) noexcept {
  ByteOffset cursor=0;
  for(const auto& element : elements) {
    if(element.snapshot().get()!=&source) return false;
    const auto range=element.range();
    if(element.synthetic()) {if(range.begin!=range.end || range.begin!=cursor) return false;continue;}
    if(range.begin!=cursor || range.end<=range.begin || range.end>source.size()) return false;
    cursor=range.end;
  }
  return cursor==source.size();
}
}
