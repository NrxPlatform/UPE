# Phase 1 acceptance checklist

- [x] Prototype immutable byte-owning source snapshots and bounded slices.
- [x] Prototype byte-based line mapping for CRLF, CR, LF.
- [x] Independently buildable source test executables.
- [x] Implement and test BOM/invalid UTF-8 policy and Unicode coordinate adapters (P1.1).
- [x] Implement token/trivia, synthetic and error element contracts and diagnostic records (P1.1).
- [x] Implement two green-node storage prototypes; run initial reconstruction, lifetime, validation and layout measurements (P1.2).
- [x] Select the flat index-arena as Phase 1 default after multi-shape synthetic benchmarking (P1.2 decision gate); revisit for structural sharing and real-world workloads.
- [x] Specify event-based CST builder with rollback checkpoints.
- [x] Compile-only examples for all C++ grammar combinators and native hooks.
- [x] Grammar graph validation and freeze API contracts.
- [x] Explicit parse status, resource budget, speculative diagnostics and cut contracts.
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

## P1.4 Batch 1 — contract hardening

- [x] P1.4a terminal result/failure metadata; independently tested (`parse.result`).
- [x] P1.4b snapshot/restore lexer mode, indent, and byte offset (`lexer.checkpoint`).
- [x] P1.4c farthest speculative expectations versus rolled-back and committed diagnostics (`parse.failures`).
- [ ] P1.4d–g and original Phase 1 acceptance/signoff remain open.

## P1.4 Batch 2 — grammar/native contract hardening (2026-11-01)

- [x] Add explicit native descriptors for side effects, contextual state, cancellation, rollback, and memoization eligibility.
- [x] Reject statically unsafe native configurations (`GRAM-11`, `GRAM-12`); pure hooks may opt into memoization.
- [x] Preserve rule definitions and native descriptors in immutable frozen snapshots.
- [x] Ensure native safety metadata affects content fingerprints.
- [x] Compile native-hook and every grammar combinator API with an isolated test executable.
- [ ] Phase 2 executor must enforce these declarations at runtime, including cancellation and backtracking; no hooks are run in P1.4.
- [ ] Formal Phase 1 sign-off remains gated by P1.4e–g and full grammar conformance audit.

## P1.4 Batch 3 — CST event builder and red views (2026-11-02)

- [x] Implement a dedicated event-based CST builder producing the Phase 1 default `FlatGreenTree`.
- [x] Checkpoint handles are per-instance, serial-numbered, and cannot be restored after commit/rollback; rollback invalidates younger nested checkpoints.
- [x] Commit requires the most recently opened checkpoint; finalization rejects open checkpoints, unfinished roots, or invalid lexical coverage.
- [x] Red views pin `shared_ptr<const FlatGreenTree>` and calculate absolute ranges from immutable green widths; nested views outlive the original tree owner without parent reference cycles.
- [x] Independently compiled `syntax.builder` and `syntax.red_view` tests.
- [x] Debug 24/24, Release 24/24, ASan+UBSan new tests 2/2 (leak detection disabled).
- [ ] Parser-session/CST-builder checkpoint coordination is a Phase 2 executor integration concern; Phase 1 builder and session remain independent contracts.
- [ ] Phase 0 baseline and exact-revision signoff remain open (P1.4f/g).

## P1.4 Batch 4 — Phase 0 foundation and acceptance verification (2026-11-02)

- [x] Add CMake Debug/Release/sanitizer presets and a GitHub Actions GCC/Clang test matrix.
- [x] Activate `-Wall -Wextra -Wpedantic -Werror` for GNU/Clang targets; verify compilation with warnings-as-errors enabled.
- [x] Add an independently buildable standalone contract smoke consumer, not a claim of PEG runtime completeness.
- [x] Replace disabled-in-Release `assert` checks in the three starter source tests with always-active `CHECK` checks.
- [x] Run reconstructed local test matrix: GCC Debug 25/25, GCC Release 25/25, GCC ASan+UBSan 25/25 (`detect_leaks=0`), Clang Debug 25/25.
- [x] Verify fresh focused `test_token_trivia` build compiles only its own test source (plus needed libraries).
- [x] Add provenance/redistribution notes and an acceptance evidence schema in `docs/phase0-baseline.md`.
- [ ] P1.4f final: verify CI run results and approve all Phase 0 portability/provenance policies on the actual repository revision.
- [ ] P1.4g final: run and archive exact-GitHub-commit matrix and signed Phase 1 acceptance review.

**Final original Phase 1 sign-off item remains unchecked.** See `docs/phase1-batch4-verification.md`; local reconstructed results cannot certify an unverified remote Git commit.
