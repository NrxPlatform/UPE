#include "upe/cst_builder.hpp"
#include <algorithm>
#include <atomic>
#include <limits>
#include <stdexcept>
#include <utility>

namespace upe {
namespace { std::atomic<std::uint64_t> next_owner{1}; }
CstEventBuilder::CstEventBuilder() : owner_(next_owner.fetch_add(1)) {}
BuilderCheckpoint CstEventBuilder::checkpoint() {
  if (next_serial_ == std::numeric_limits<std::uint64_t>::max())
    throw std::overflow_error("CST checkpoint serial overflow");
  auto serial = next_serial_++;
  marks_.push_back({serial, events_.size(), depth_, completed_});
  return {owner_, serial};
}
void CstEventBuilder::rollback(BuilderCheckpoint mark) {
  if (mark.owner != owner_) throw std::invalid_argument("foreign CST checkpoint");
  auto it = std::find_if(marks_.begin(), marks_.end(),
      [&](const Mark& m) { return m.serial == mark.serial; });
  if (it == marks_.end()) throw std::invalid_argument("stale CST checkpoint");
  events_.resize(it->event_size);
  depth_ = it->depth;
  completed_ = it->completed;
  marks_.erase(it, marks_.end());
}
void CstEventBuilder::commit(BuilderCheckpoint mark) {
  if (mark.owner != owner_ || marks_.empty() || marks_.back().serial != mark.serial)
    throw std::invalid_argument("CST checkpoint commit must be LIFO");
  marks_.pop_back();
}
void CstEventBuilder::start(SyntaxKind kind) {
  if (!kind || completed_) throw std::invalid_argument("invalid CST start");
  events_.push_back(CstEvent::start(kind));
  ++depth_;
}
void CstEventBuilder::leaf(std::size_t index) {
  if (!depth_ || completed_) throw std::invalid_argument("leaf outside CST node");
  events_.push_back(CstEvent::leaf(index));
}
void CstEventBuilder::finish() {
  if (!depth_ || completed_) throw std::invalid_argument("unbalanced CST finish");
  events_.push_back(CstEvent::finish());
  if (--depth_ == 0) completed_ = true;
}
FlatGreenTree CstEventBuilder::build(std::shared_ptr<const SourceSnapshot> source,
    std::span<const LexicalElement> leaves) const {
  if (!complete() || !marks_.empty())
    throw std::logic_error("CST builder has incomplete tree or open checkpoints");
  return FlatGreenTree::build(std::move(source), leaves, events_);
}

RedNode::RedNode(std::shared_ptr<const FlatGreenTree> tree)
  : tree_(std::move(tree)) {
  if (!tree_) throw std::invalid_argument("null red tree");
  index_ = tree_->root_;
}
RedNode::RedNode(std::shared_ptr<const FlatGreenTree> tree, std::uint32_t index,
                 ByteOffset offset) : tree_(std::move(tree)), index_(index), offset_(offset) {}
SyntaxKind RedNode::kind() const { return tree_->entries_.at(index_).kind; }
bool RedNode::is_leaf() const { return tree_->entries_.at(index_).is_leaf; }
ByteRange RedNode::range() const {
  const auto& entry = tree_->entries_.at(index_);
  return {offset_, offset_ + entry.width};
}
std::size_t RedNode::child_count() const { return tree_->entries_.at(index_).children.size(); }
RedNode RedNode::child(std::size_t ordinal) const {
  const auto& kids = tree_->entries_.at(index_).children;
  if (ordinal >= kids.size()) throw std::out_of_range("red child ordinal");
  ByteOffset offset = offset_;
  for (std::size_t i=0; i<ordinal; ++i) offset += tree_->entries_.at(kids[i]).width;
  return RedNode(tree_,kids[ordinal],offset);
}
} // namespace upe
