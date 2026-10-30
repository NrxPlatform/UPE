#include "upe/cst.hpp"
#include <functional>
#include <limits>
#include <stdexcept>
#include <utility>

namespace upe {
namespace {
struct Validated {
  std::vector<std::size_t> traversal;
  std::size_t nodes{}, edges{};
};
Validated validate(const SourceSnapshot& source, std::span<const LexicalElement> leaves,
                   std::span<const CstEvent> events) {
  if(events.empty()) throw std::invalid_argument("empty CST events");
  std::vector<std::size_t> stack;
  std::vector<bool> used(leaves.size());
  Validated out;
  ByteOffset cursor=0;
  bool complete=false;
  for (const auto& e : events) {
    if (complete) throw std::invalid_argument("events after root");
    switch(e.type) {
      case CstEventType::start_node:
        if(e.kind==0) throw std::invalid_argument("zero node kind");
        if(!stack.empty()) ++out.edges;
        stack.push_back(0);
        ++out.nodes;
        break;
      case CstEventType::leaf: {
        if(stack.empty() || e.leaf_index>=leaves.size() || used[e.leaf_index])
          throw std::invalid_argument("invalid or repeated leaf");
        const auto& leaf=leaves[e.leaf_index];
        if(leaf.snapshot().get()!=&source) throw std::invalid_argument("mixed snapshots");
        const auto r=leaf.range();
        if(r.begin!=cursor || r.end>source.size() ||
           (leaf.synthetic() ? r.end!=r.begin : r.end<=r.begin))
          throw std::invalid_argument("noncontiguous physical source");
        used[e.leaf_index]=true;
        out.traversal.push_back(e.leaf_index);
        ++out.edges;
        cursor=r.end;
        break;
      }
      case CstEventType::finish_node:
        if(stack.empty()) throw std::invalid_argument("unbalanced finish");
        stack.pop_back();
        if(stack.empty()) complete=true;
        break;
      default: throw std::invalid_argument("unknown CST event");
    }
  }
  if(!complete || cursor!=source.size() || out.traversal.size()!=leaves.size())
    throw std::invalid_argument("incomplete CST source coverage");
  return out;
}
ByteOffset checked_add(ByteOffset a, ByteOffset b) {
  if (b > std::numeric_limits<ByteOffset>::max()-a)
    throw std::overflow_error("CST byte width overflow");
  return a+b;
}
ByteRange range_from(const std::vector<LexicalElement>& leaves,
                     const std::vector<std::size_t>& traversal, std::size_t ordinal) {
  if(ordinal>=traversal.size()) throw std::out_of_range("leaf ordinal");
  return leaves[traversal[ordinal]].range();
}
} // namespace

FlatGreenTree FlatGreenTree::build(std::shared_ptr<const SourceSnapshot> source,
    std::span<const LexicalElement> leaves, std::span<const CstEvent> events) {
  if(!source) throw std::invalid_argument("null CST source");
  const auto v=validate(*source,leaves,events);
  FlatGreenTree t;
  t.source_=std::move(source);
  t.leaves_.assign(leaves.begin(), leaves.end());
  t.traversal_=v.traversal;
  std::vector<std::uint32_t> stack;
  for(const auto& e: events) {
    if(e.type==CstEventType::start_node) {
      if(t.entries_.size()>=std::numeric_limits<std::uint32_t>::max())
        throw std::overflow_error("arena index overflow");
      const auto idx=static_cast<std::uint32_t>(t.entries_.size());
      t.entries_.push_back({e.kind,0,0,false,{}});
      if(!stack.empty()) t.entries_[stack.back()].children.push_back(idx);
      else t.root_=idx;
      stack.push_back(idx);
    } else if(e.type==CstEventType::leaf) {
      if(t.entries_.size()>=std::numeric_limits<std::uint32_t>::max())
        throw std::overflow_error("arena index overflow");
      const auto idx=static_cast<std::uint32_t>(t.entries_.size());
      const auto r=t.leaves_[e.leaf_index].range();
      t.entries_.push_back({0,e.leaf_index,r.end-r.begin,true,{}});
      t.entries_[stack.back()].children.push_back(idx);
    } else {
      const auto idx=stack.back(); stack.pop_back();
      ByteOffset width=0;
      for (auto child: t.entries_[idx].children)
        width=checked_add(width,t.entries_[child].width);
      t.entries_[idx].width=width;
    }
  }
  if(t.entries_[t.root_].width!=t.source_->size())
    throw std::invalid_argument("root width mismatch");
  return t;
}
std::string FlatGreenTree::reconstruct() const {
  std::string bytes;
  std::vector<std::uint32_t> pending{root_};
  while(!pending.empty()) {
    const auto idx=pending.back(); pending.pop_back();
    const auto& e=entries_[idx];
    if(e.is_leaf) {
      if(!leaves_[e.leaf_index].synthetic()) bytes.append(leaves_[e.leaf_index].bytes());
    } else for(auto i=e.children.rbegin();i!=e.children.rend();++i) pending.push_back(*i);
  }
  return bytes;
}
ByteOffset FlatGreenTree::width() const noexcept { return entries_[root_].width; }
ByteRange FlatGreenTree::leaf_range(std::size_t i) const { return range_from(leaves_,traversal_,i); }
GreenMetrics FlatGreenTree::metrics() const noexcept {
  GreenMetrics m;
  m.leaves=leaves_.size();
  m.nodes=entries_.size()-m.leaves;
  m.estimated_storage_bytes=sizeof(*this)+entries_.capacity()*sizeof(Entry)
    +leaves_.capacity()*sizeof(LexicalElement)+traversal_.capacity()*sizeof(std::size_t);
  for(const auto& entry:entries_) {
    m.edges+=entry.children.size();
    m.estimated_storage_bytes+=entry.children.capacity()*sizeof(std::uint32_t);
  }
  return m;
}

SharedGreenTree SharedGreenTree::build(std::shared_ptr<const SourceSnapshot> source,
    std::span<const LexicalElement> leaves, std::span<const CstEvent> events) {
  if(!source) throw std::invalid_argument("null CST source");
  const auto v=validate(*source,leaves,events);
  SharedGreenTree t;
  t.source_=std::move(source);
  t.leaves_.assign(leaves.begin(), leaves.end());
  t.traversal_=v.traversal;
  struct Frame { SyntaxKind kind; std::vector<std::shared_ptr<const Node>> children; };
  std::vector<Frame> stack;
  for(const auto& e: events) {
    if(e.type==CstEventType::start_node) stack.push_back({e.kind,{}});
    else if(e.type==CstEventType::leaf) {
      const auto r=t.leaves_[e.leaf_index].range();
      stack.back().children.push_back(std::make_shared<const Node>(0,e.leaf_index,r.end-r.begin,
                                          true,std::vector<std::shared_ptr<const Node>>{}));
    } else {
      Frame frame=std::move(stack.back()); stack.pop_back();
      ByteOffset w=0;
      for(const auto& child:frame.children) w=checked_add(w,child->width);
      auto node=std::make_shared<const Node>(frame.kind,0,w,false,std::move(frame.children));
      if(stack.empty()) t.root_=std::move(node);
      else stack.back().children.push_back(std::move(node));
    }
  }
  if(t.root_->width!=t.source_->size()) throw std::invalid_argument("root width mismatch");
  t.metrics_.nodes=v.nodes;
  t.metrics_.leaves=t.leaves_.size();
  t.metrics_.edges=v.edges;
  // Approximation: make_shared control blocks and vector capacities are implementation-dependent.
  // sizeof(Node) excludes shared_ptr control-block metadata and allocator overhead.
  t.metrics_.estimated_storage_bytes=sizeof(t)+t.leaves_.capacity()*sizeof(LexicalElement)
    +t.traversal_.capacity()*sizeof(std::size_t)
    +(v.nodes+t.leaves_.size())*(sizeof(Node)+2*sizeof(void*))
    +v.edges*sizeof(std::shared_ptr<const Node>);
  return t;
}
std::string SharedGreenTree::reconstruct() const {
  std::string bytes;
  std::vector<std::shared_ptr<const Node>> pending{root_};
  while(!pending.empty()) {
    auto node=std::move(pending.back()); pending.pop_back();
    if(node->is_leaf) {
      if(!leaves_[node->leaf_index].synthetic()) bytes.append(leaves_[node->leaf_index].bytes());
    } else for(auto i=node->children.rbegin();i!=node->children.rend();++i) pending.push_back(*i);
  }
  return bytes;
}
ByteOffset SharedGreenTree::width() const noexcept { return root_->width; }
ByteRange SharedGreenTree::leaf_range(std::size_t i) const { return range_from(leaves_,traversal_,i); }
GreenMetrics SharedGreenTree::metrics() const noexcept { return metrics_; }
} // namespace upe
