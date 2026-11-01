#include "upe/cst.hpp"
#include "check.hpp"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

using namespace upe;
namespace {
using Clock = std::chrono::steady_clock;
struct Case {
  const char* name;
  std::shared_ptr<const SourceSnapshot> source;
  std::vector<LexicalElement> leaves;
  std::vector<CstEvent> events;
};
Case make_case(const char* name, std::size_t count, std::size_t width, std::size_t nesting) {
  Case c{name, SourceSnapshot::create(std::string(count, 'x')), {}, {}};
  c.leaves.reserve(count);
  c.events.reserve(2 * count + nesting * 2 + 4);
  c.events.push_back(CstEvent::start(1));
  for (std::size_t d = 0; d < nesting; ++d) c.events.push_back(CstEvent::start(2));
  for (std::size_t i = 0; i < count; ++i) {
    if (width && i % width == 0) c.events.push_back(CstEvent::start(3));
    c.leaves.emplace_back(c.source, ByteRange{i, i + 1}, TokenIdentity{1,1,1}, ElementClass::token);
    c.events.push_back(CstEvent::leaf(i));
    if (width && (i % width == width - 1 || i + 1 == count)) c.events.push_back(CstEvent::finish());
  }
  for (std::size_t d = 0; d < nesting; ++d) c.events.push_back(CstEvent::finish());
  c.events.push_back(CstEvent::finish());
  return c;
}
template <typename Fn> std::int64_t median_us(Fn run, int repetitions) {
  std::vector<std::int64_t> measurements;
  measurements.reserve(static_cast<std::size_t>(repetitions));
  for (int i=0; i<repetitions; ++i) {
    const auto a = Clock::now();
    run();
    const auto b = Clock::now();
    measurements.push_back(std::chrono::duration_cast<std::chrono::microseconds>(b-a).count());
  }
  std::sort(measurements.begin(), measurements.end());
  return measurements[measurements.size()/2];
}
void benchmark(const Case& c, int repetitions) {
  const auto flat = FlatGreenTree::build(c.source, c.leaves, c.events);
  const auto shared = SharedGreenTree::build(c.source, c.leaves, c.events);
  const auto expected = c.source->bytes();
  CHECK(flat.reconstruct() == expected);
  CHECK(shared.reconstruct() == expected);
  CHECK(flat.width() == shared.width());
  CHECK(flat.metrics().nodes == shared.metrics().nodes);
  CHECK(flat.metrics().leaves == shared.metrics().leaves);
  // Repeated single-process measurements reduce, but do not eliminate, timing noise.
  const auto flat_build = median_us([&]{ const auto tree=FlatGreenTree::build(c.source,c.leaves,c.events); CHECK(tree.width()==c.source->size()); },repetitions);
  const auto shared_build = median_us([&]{ const auto tree=SharedGreenTree::build(c.source,c.leaves,c.events); CHECK(tree.width()==c.source->size()); },repetitions);
  const auto shared_read = median_us([&]{ CHECK(shared.reconstruct()==expected); },repetitions);
  const auto flat_read = median_us([&]{ CHECK(flat.reconstruct()==expected); },repetitions);
  const auto f=flat.metrics(),s=shared.metrics();
  std::cout << c.name << ",flat," << flat_build << ',' << flat_read << ',' << f.estimated_storage_bytes << ',' << f.nodes << ',' << f.leaves << '\n';
  std::cout << c.name << ",shared," << shared_build << ',' << shared_read << ',' << s.estimated_storage_bytes << ',' << s.nodes << ',' << s.leaves << '\n';
}
} // namespace
int main() {
  constexpr int repetitions=9;
  std::cout << "scenario,layout,median_build_us,median_reconstruct_us,estimated_storage_bytes,nodes,leaves\n";
  benchmark(make_case("wide",12000,0,0),repetitions);
  benchmark(make_case("grouped",12000,12,0),repetitions);
  benchmark(make_case("deep",2000,1,64),repetitions);
  benchmark(make_case("mixed",12000,48,8),repetitions);
  std::cerr << "Median of " << repetitions << " runs; estimates exclude source and allocator overhead.\n"
            << "Benchmark is exploratory, not a real-world workload or RSS measurement.\n";
}
