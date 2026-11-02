#pragma once
#include "upe/cst.hpp"
#include <cstdint>
#include <memory>
#include <vector>

namespace upe {
// Builder checkpoints are LIFO transactional handles. A restore invalidates
// the restored handle and all younger handles. Handles from another builder
// or a previous generation cannot be reused.
struct BuilderCheckpoint {
  std::uint64_t owner{}, serial{};
};
class CstEventBuilder final {
public:
  CstEventBuilder();
  CstEventBuilder(const CstEventBuilder&) = delete;
  CstEventBuilder& operator=(const CstEventBuilder&) = delete;
  BuilderCheckpoint checkpoint();
  void rollback(BuilderCheckpoint checkpoint);
  void commit(BuilderCheckpoint checkpoint); // topmost checkpoint only
  void start(SyntaxKind kind);
  void leaf(std::size_t index);
  void finish();
  const std::vector<CstEvent>& events() const noexcept { return events_; }
  bool complete() const noexcept { return completed_ && depth_ == 0; }
  std::size_t depth() const noexcept { return depth_; }
  FlatGreenTree build(std::shared_ptr<const SourceSnapshot> source,
                      std::span<const LexicalElement> leaves) const;
private:
  struct Mark {
    std::uint64_t serial;
    std::size_t event_size, depth;
    bool completed;
  };
  std::uint64_t owner_, next_serial_{1};
  std::vector<CstEvent> events_;
  std::vector<Mark> marks_;
  std::size_t depth_{};
  bool completed_{};
};

// Red views pin a finished immutable tree, even when the original tree owner
// is released. Offsets are calculated from green child widths; no parent cycles.
class RedNode final {
public:
  explicit RedNode(std::shared_ptr<const FlatGreenTree> tree);
  SyntaxKind kind() const;
  bool is_leaf() const;
  ByteRange range() const;
  std::size_t child_count() const;
  RedNode child(std::size_t ordinal) const;
private:
  RedNode(std::shared_ptr<const FlatGreenTree> tree, std::uint32_t index, ByteOffset offset);
  std::shared_ptr<const FlatGreenTree> tree_;
  std::uint32_t index_{};
  ByteOffset offset_{};
};
} // namespace upe
