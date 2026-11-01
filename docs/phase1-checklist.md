# Phase 1 acceptance checklist

- [x] Prototype immutable byte-owning source snapshots and bounded slices.
- [x] Prototype byte-based line mapping for CRLF, CR, LF.
- [x] Independently buildable source test executables.
- [x] Implement and test BOM/invalid UTF-8 policy and Unicode coordinate adapters (P1.1).
- [x] Implement token/trivia, synthetic and error element contracts and diagnostic records (P1.1).
- [x] Implement two green-node storage prototypes; run initial reconstruction, lifetime, validation and layout measurements (P1.2).
- [x] Select the flat index-arena as Phase 1 default after multi-shape synthetic benchmarking (P1.2 decision gate); revisit for structural sharing and real-world workloads.
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

## P1.2 storage-layout decision (2026-11-01)

**Decision:** choose `FlatGreenTree` (index-based arena) as the Phase 1 default representation.
Keep `SharedGreenTree` as a comparison prototype, not a required execution-path dependency.
This is a reversible decision; a packed arena, immutable-node interning, and incremental
structural sharing remain future optimization work. Do not remove either prototype yet.

The upgraded `bench_syntax_layout` tests four _synthetic_ tree shapes (9 repetitions, median,
Release, this development container). All cases reconstruct identical bytes:

| Shape                                    | Leaves | Flat build (µs) | Shared build (µs) | Flat reconstruct (µs) | Shared reconstruct (µs) | Flat estimate (bytes) | Shared estimate (bytes) |
| ---------------------------------------- | -----: | --------------: | ----------------: | --------------------: | ----------------------: | --------------------: | ----------------------: |
| Wide                                     | 12,000 |             519 |               791 |                    46 |                      67 |             1,655,136 |               1,728,184 |
| Grouped (12 per node)                    | 12,000 |             576 |               583 |                    45 |                      75 |             1,657,696 |               1,816,184 |
| Deep (64 nested groups + per-leaf nodes) |  2,000 |             100 |               187 |                    11 |                      17 |               357,920 |                 469,816 |
| Mixed (48 per node, 8 nested groups)     | 12,000 |             251 |               487 |                    43 |                      68 |             1,654,656 |               1,750,888 |

**Limits:** Results are machine/compiler-specific, not statistical confidence intervals;
input is synthetic, not sampled from real programming languages; estimated storage is not
RSS and excludes shared source bytes and allocator overhead. Repeat on production-sized
repositories and measure allocation count, cache effects, real memory, and incremental
sharing before adopting a permanent packed representation.

**Acceptance boundary:** Layout comparison and decision completed for the Phase 1 prototype.
The actual rollback-capable CST builder, red views, and grammar/parser execution remain
subsequent tasks.

## P1.3 grammar and parser contracts (2026-11-01)

- [x] Native C++ grammar builder: token/literal, sequence, choice, optional, repetition,
      predicates, references, EOF, cut, syntax node, and native descriptor contracts.
- [x] Rule ownership, stable declared rule IDs, forward declarations, immutable snapshot
      freeze, deterministic versioned fingerprint (non-cryptographic FNV-1a).
- [x] Validation reports: pinned grammar identity, missing definitions/references,
      invalid native registrations, left-recursive rule cycles, nullable repetitions,
      cut scope, unreachable-rule warnings. Detection is conservative; token/syntax
      registries and contextual validation require later extension.
- [x] Session contract prototype: explicit outcome status, monotonic resource budgets,
      cursor/event/diagnostic/context checkpoint rollback, choice-scoped cut state.
- [x] Separate independent CMake tests: `grammar.api`, `grammar.validation`, `parse.session`.
- [ ] **Not part of P1.3:** actual PEG execution, runtime grammar interpretation,
      contextual lexer checkpoint adapter, production native hook execution, runtime error
      recovery, memoization, and full grammar/static analysis.

The P1.3 API is an early, deliberately limited executable contract, not a claim of
full conformance with `04-grammar-api.md`. Remaining gaps are tracked for the PEG
runtime and Phase 1 integration review (P1.4).

## P1.4 integration and audit (2026-11-01)

- [x] Cross-component lossless source → lexical → speculative session → CST integration test.
- [x] Compile-only full combinator API example with frozen-fingerprint reproducibility test.
- [x] Review available implementation against Phase 1 exit criteria; record missing contracts.
- [ ] **Phase 1 sign-off blocked**: see `docs/phase1-acceptance-audit.md` and P1.4a–g.

P1.4 test additions are acceptance _evidence_, not proof that all Phase 1 contracts
are complete.
