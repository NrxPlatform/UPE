#include "upe/source.hpp"
#include <algorithm>
#include <limits>
#include <utility>

namespace upe {
SourceSnapshot::SourceSnapshot(std::string bytes, std::string identity, std::uint64_t revision)
    : bytes_(std::move(bytes)), identity_(std::move(identity)), revision_(revision) {
  line_starts_.push_back(0);
  for (std::size_t i = 0; i < bytes_.size(); ++i) {
    if (bytes_[i] == '\r') {
      if (i + 1 < bytes_.size() && bytes_[i + 1] == '\n') ++i;
      line_starts_.push_back(i + 1);
    } else if (bytes_[i] == '\n') {
      line_starts_.push_back(i + 1);
    }
  }
}
std::shared_ptr<const SourceSnapshot> SourceSnapshot::create(
    std::string bytes, std::string identity, std::uint64_t revision) {
  return std::shared_ptr<const SourceSnapshot>(
      new SourceSnapshot(std::move(bytes), std::move(identity), revision));
}
std::string_view SourceSnapshot::view(ByteRange range) const {
  if (range.begin > range.end || range.end > size()) throw std::out_of_range("invalid byte range");
  return std::string_view(bytes_.data() + static_cast<std::size_t>(range.begin),
                          static_cast<std::size_t>(range.end - range.begin));
}
SourceLocation SourceSnapshot::locate(ByteOffset offset) const {
  if (offset > size()) throw std::out_of_range("invalid byte offset");
  auto it = std::upper_bound(line_starts_.begin(), line_starts_.end(), offset);
  auto index = static_cast<std::size_t>(it - line_starts_.begin() - 1);
  return {static_cast<ByteOffset>(index), offset - line_starts_[index]};
}
SourceSlice::SourceSlice(std::shared_ptr<const SourceSnapshot> source, ByteRange range)
    : source_(std::move(source)), range_(range) {
  if (!source_) throw std::invalid_argument("null source snapshot");
  (void)source_->view(range_);
}
SourceSlice SourceSlice::sub_slice(ByteRange relative) const {
  const auto length = range_.end - range_.begin;
  if (relative.begin > relative.end || relative.end > length)
    throw std::out_of_range("invalid relative slice");
  return SourceSlice(source_, {range_.begin + relative.begin, range_.begin + relative.end});
}
} // namespace upe
