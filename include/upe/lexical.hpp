#pragma once
#include "upe/source.hpp"
#include <optional>
#include <span>
namespace upe {
using TokenKind = std::uint32_t;
using LanguageId = std::uint32_t;
using GrammarRevision = std::uint32_t;
struct TokenIdentity { LanguageId language{}; GrammarRevision revision{}; TokenKind kind{}; };
enum class ElementClass { token, trivia, error, missing };
class LexicalElement final {
public:
  LexicalElement(std::shared_ptr<const SourceSnapshot> source, ByteRange range,
                 TokenIdentity kind, ElementClass type);
  const std::shared_ptr<const SourceSnapshot>& snapshot() const noexcept { return source_; }
  ByteRange range() const noexcept { return range_; }
  TokenIdentity kind() const noexcept { return kind_; }
  ElementClass type() const noexcept { return type_; }
  std::string_view bytes() const { return source_->view(range_); }
  bool synthetic() const noexcept { return type_==ElementClass::missing; }
private:
  std::shared_ptr<const SourceSnapshot> source_;
  ByteRange range_;
  TokenIdentity kind_;
  ElementClass type_;
};
// True iff physical elements cover source exactly once and missing elements are zero-width.
bool covers_source(std::span<const LexicalElement> elements, const SourceSnapshot& source) noexcept;
}
