#pragma once
#include "upe/source.hpp"
#include <optional>
namespace upe {
enum class BomKind { none, utf8, utf16_le, utf16_be, utf32_le, utf32_be };
struct Utf8Validation { bool valid{true}; ByteOffset first_invalid{}; std::uint64_t invalid_sequences{}; };
struct UnicodeLocation { ByteOffset line{}, scalar_column{}, utf16_column{}; };
BomKind detect_bom(std::string_view bytes) noexcept;
Utf8Validation validate_utf8(std::string_view bytes) noexcept;
// Offset inside a multibyte sequence counts only code points fully before offset.
// Invalid byte sequences decode as U+FFFD, consuming one invalid byte at a time.
UnicodeLocation locate_unicode(const SourceSnapshot& source, ByteOffset offset);
}
