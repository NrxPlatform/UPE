#pragma once
#include "upe/lexical.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <vector>

namespace upe {
// P1.2 comparison prototypes. Events are immutable input descriptions, NOT a
// transactional parser builder. Phase P1.3 will define the rollback protocol.
using SyntaxKind = std::uint32_t;
enum class CstEventType { start_node, leaf, finish_node };
struct CstEvent {
  CstEventType type{};
  SyntaxKind kind{};        // start_node only
  std::size_t leaf_index{}; // leaf only, index into input physical elements
  static CstEvent start(SyntaxKind kind) { return {CstEventType::start_node, kind, 0}; }
  static CstEvent leaf(std::size_t index) { return {CstEventType::leaf, 0, index}; }
  static CstEvent finish() { return {CstEventType::finish_node, 0, 0}; }
};
struct GreenMetrics {
  std::size_t nodes{};
  std::size_t leaves{};
  std::size_t edges{};
  std::size_t estimated_storage_bytes{}; // capacity-based approximation, not RSS
};

// Index-addressed immutable-after-construction arena prototype.
class FlatGreenTree final {
  friend class RedNode;
public:
  static FlatGreenTree build(std::shared_ptr<const SourceSnapshot> source,
                             std::span<const LexicalElement> leaves,
                             std::span<const CstEvent> events);
  std::string reconstruct() const;
  ByteOffset width() const noexcept;
  GreenMetrics metrics() const noexcept;
  // Byte range of a leaf in physical traversal order (not source identity).
  ByteRange leaf_range(std::size_t ordinal) const;
private:
  struct Entry {
    SyntaxKind kind{};
    std::size_t leaf_index{};
    ByteOffset width{};
    bool is_leaf{};
    std::vector<std::uint32_t> children;
  };
  std::shared_ptr<const SourceSnapshot> source_;
  std::vector<LexicalElement> leaves_;
  std::vector<Entry> entries_;
  std::uint32_t root_{};
  std::vector<std::size_t> traversal_; // indices into leaves_, in source order
};

// Shared immutable node prototype, intentionally distinct allocation pattern.
class SharedGreenTree final {
public:
  static SharedGreenTree build(std::shared_ptr<const SourceSnapshot> source,
                               std::span<const LexicalElement> leaves,
                               std::span<const CstEvent> events);
  std::string reconstruct() const;
  ByteOffset width() const noexcept;
  GreenMetrics metrics() const noexcept;
  ByteRange leaf_range(std::size_t ordinal) const;
private:
  struct Node {
    const SyntaxKind kind;
    const std::size_t leaf_index;
    const ByteOffset width;
    const bool is_leaf;
    const std::vector<std::shared_ptr<const Node>> children;
    Node(SyntaxKind k, std::size_t i, ByteOffset w, bool leaf,
         std::vector<std::shared_ptr<const Node>> kids)
      : kind(k), leaf_index(i), width(w), is_leaf(leaf), children(std::move(kids)) {}
  };
  std::shared_ptr<const SourceSnapshot> source_;
  std::vector<LexicalElement> leaves_;
  std::shared_ptr<const Node> root_;
  std::vector<std::size_t> traversal_;
  GreenMetrics metrics_;
};
} // namespace upe
