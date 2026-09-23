#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace upe {
using ByteOffset = std::uint64_t;
struct ByteRange { ByteOffset begin{}, end{}; };
struct SourceLocation { ByteOffset line{}, byte_column{}; }; // zero-based
class SourceSnapshot final {
public:
  static std::shared_ptr<const SourceSnapshot> create(std::string bytes,
      std::string identity = {}, std::uint64_t revision = 0);
  std::string_view bytes() const noexcept { return bytes_; }
  ByteOffset size() const noexcept { return bytes_.size(); }
  const std::string& identity() const noexcept { return identity_; }
  std::uint64_t revision() const noexcept { return revision_; }
  std::string_view view(ByteRange range) const;
  SourceLocation locate(ByteOffset offset) const;
private:
  SourceSnapshot(std::string bytes, std::string identity, std::uint64_t revision);
  std::string bytes_, identity_;
  std::uint64_t revision_{};
  std::vector<ByteOffset> line_starts_;
};
class SourceSlice final {
public:
  SourceSlice(std::shared_ptr<const SourceSnapshot> source, ByteRange range);
  const SourceSnapshot& source() const noexcept { return *source_; }
  const std::shared_ptr<const SourceSnapshot>& snapshot() const noexcept { return source_; }
  ByteRange range() const noexcept { return range_; }
  std::string_view bytes() const { return source_->view(range_); }
  SourceSlice sub_slice(ByteRange relative) const;
private:
  std::shared_ptr<const SourceSnapshot> source_;
  ByteRange range_;
};
} // namespace upe
