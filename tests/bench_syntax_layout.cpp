#include "upe/cst.hpp"
#include "check.hpp"
#include <chrono>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>
using namespace upe;
int main() {
  constexpr std::size_t n=12000;
  std::string bytes(n,'x');
  auto source=SourceSnapshot::create(bytes);
  std::vector<LexicalElement> leaves;
  std::vector<CstEvent> events;
  leaves.reserve(n);
  events.reserve(n+2);
  events.push_back(CstEvent::start(1));
  for(std::size_t i=0;i<n;++i) {
    leaves.emplace_back(source,ByteRange{i,i+1},TokenIdentity{1,1,1},ElementClass::token);
    events.push_back(CstEvent::leaf(i));
  }
  events.push_back(CstEvent::finish());
  using Clock=std::chrono::steady_clock;
  const auto t0=Clock::now();
  auto flat=FlatGreenTree::build(source,leaves,events);
  const auto t1=Clock::now();
  auto shared=SharedGreenTree::build(source,leaves,events);
  const auto t2=Clock::now();
  const auto f=flat.reconstruct();
  const auto t3=Clock::now();
  const auto s=shared.reconstruct();
  const auto t4=Clock::now();
  CHECK(f==bytes && s==bytes);
  const auto fm=flat.metrics(),sm=shared.metrics();
  auto us=[](auto a,auto b) { return std::chrono::duration_cast<std::chrono::microseconds>(b-a).count(); };
  std::cout << "layout,build_us,reconstruct_us,estimated_storage_bytes,nodes,leaves\n";
  std::cout << "flat," << us(t0,t1) << ',' << us(t2,t3) << ',' << fm.estimated_storage_bytes << ',' << fm.nodes << ',' << fm.leaves << '\n';
  std::cout << "shared," << us(t1,t2) << ',' << us(t3,t4) << ',' << sm.estimated_storage_bytes << ',' << sm.nodes << ',' << sm.leaves << '\n';
  std::cout << "Note: estimates exclude source buffer and allocator overhead; timings are single samples, not statistically reliable.\n";
}
