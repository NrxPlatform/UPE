# Phase 1 acceptance checklist

- [x] Prototype immutable byte-owning source snapshots and bounded slices.
- [x] Prototype byte-based line mapping for CRLF, CR, LF.
- [x] Independently buildable source test executables.
- [x] Implement and test BOM/invalid UTF-8 policy and Unicode coordinate adapters (P1.1).
- [x] Implement token/trivia, synthetic and error element contracts and diagnostic records (P1.1).
- [x] Implement two green-node storage prototypes; run initial reconstruction, lifetime, validation and layout measurements (P1.2).
- [ ] Finalize storage layout after representative multi-shape benchmarking (P1.2 decision gate).
- [ ] Specify event-based CST builder with rollback checkpoints.
- [ ] Compile-only examples for all C++ grammar combinators and native hooks.
- [ ] Grammar graph validation and freeze API contracts.
- [ ] Explicit parse status, resource budget, speculative diagnostics and cut contracts.
- [ ] Phase 0 baseline and Phase 1 contract review/signoff.

Phase 1 is not accepted until all checklist items and the exit criteria in `01-implementation-plan.md` are satisfied.

## P1.1 implementation notes

- Physical elements use single-owner, gapless source coverage; missing elements have zero width.
- UTF-8 decoding preserves bytes; invalid sequences consume one invalid byte per replacement.
- Unicode columns count fully decoded scalars / UTF-16 units before requested byte offset.
- Nine focused test executables (three starter, six P1.1) build independently.
- Release/sanitizer verification is required before acceptance.

## P1.2 prototype results (2026-10-30)

- Added `upe_cst` with **flat index-arena** and **shared immutable-node** prototypes.
- Both validate balanced events, exactly one root, unique contiguous source coverage,
  source identity, missing-token zero width and node widths.
- Reconstruction traverses actual stored trees and preserves original bytes, including malformed input.
- `test_syntax_reconstruction` and `test_syntax_validation` are independent CMake test targets.
- Optional `bench_syntax_layout` target (excluded from normal `upe_tests`); run with
  `cmake --build build/debug --target bench_syntax_layout && ./build/debug/bench_syntax_layout`.
- First Debug comparison (12,000 single-byte leaves, shallow tree; single sample):
  flat build 10,055 µs / reconstruct 1,634 µs / estimated storage 1,655,136 bytes;
  shared build 8,279 µs / reconstruct 3,702 µs / estimated storage 1,728,184 bytes.
  **Not statistically significant**: measurements exclude source buffer and allocator overhead;
  shared estimate does not precisely capture per-node vector capacities.
- Debug: 11/11; Release: 11/11; ASan+UBSan: 2/2 new syntax tests.
- **Not yet locked:** canonical layout, packed arena design, red views, embedded roots,
  concurrent reads, representative real-world tree shapes and statistically sound benchmarks.
- Event input is a prototype format, **not** the rollback-capable builder specified for P1.3.
