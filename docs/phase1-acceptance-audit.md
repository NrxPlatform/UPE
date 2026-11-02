# Phase 1 — P1.4 acceptance audit (2026-11-01)

**Status: conditional / NOT signed off.** This report applies to the supplied starter plus
P1.1, P1.2, layout-decision, P1.3 and test-runner patches, **not** a live checkout of GitHub.

## Evidence collected

- Independent CMake test executables; `upe_tests` builds tests rather than every optional benchmark.
- Verified in the reconstructed patch baseline: Debug **16/16**, Release **16/16**, ASan+UBSan **16/16** (leak detection disabled for the sanitizer run).
- `contract.integration` exercises source snapshot, physical lexical ownership,
  speculative parse event/diagnostic rollback, flat CST construction and byte-for-byte reconstruction.
- `grammar.contract_examples` compiles all P1.3 grammar constructor forms and checks
  reproducibility of frozen grammar fingerprints across equivalent builders.
- Flat arena remains the provisional Phase 1 CST storage decision based on synthetic benchmarks.

## Specification-to-implementation trace

| Contract                                              | Assessment                       | Follow-up                                                                                   |
| ----------------------------------------------------- | -------------------------------- | ------------------------------------------------------------------------------------------- |
| Immutable byte source and byte-coordinate API         | Prototype implemented and tested | Source identity/version enforcement across adapters should be strengthened                  |
| UTF-8, BOM, malformed bytes, UTF-16/scalar conversion | Prototype implemented and tested | Add corpus of boundary/line-break cases                                                     |
| Physical token/trivia/missing/error contracts         | Implemented and tested           | Language-specific lexer interface deferred                                                  |
| Two CST prototypes, byte-exact reconstruction         | Implemented and tested           | Red views, embedded roots, production event builder deferred                                |
| Event checkpoint/restore                              | Partial session prototype        | Production builder integration and rollback generations required                            |
| Native C++ grammar combinators                        | Constructors compiled/tested     | Runtime rule execution deferred to Phase 2                                                  |
| Validation, freeze, stable grammar identity           | Initial implementation           | Need stronger nullability/contextual and native-hook verification                           |
| Parse status contract                                 | **Gap**                          | Current status enum lacks explicit unsupported-configuration mapping and rich error details |
| Lexical/context checkpoints                           | **Gap**                          | No lexer checkpoint adapter yet; session context stores placeholders                        |
| Farther-failure vs committed diagnostics              | **Gap**                          | No dedicated farthest-failure accumulator yet                                               |
| Native-hook purity/memoization contract               | **Gap**                          | Descriptor insufficient to enforce purity, cancellation, rollback and context dependencies  |
| Phase 0 baseline                                      | **Gap**                          | No committed CI presets/matrix, CLI/provenance/test-report baseline verified                |

## Gated follow-up checklist (ad hoc)

- [x] P1.4a: finalize explicit parse-result/status mapping and failure metadata.
- [x] P1.4b: design lexer checkpoint integration and context ownership (no PEG executor).
- [x] P1.4c: specify farthest-failure diagnostic accumulation and committed-error distinction.
- [x] P1.4d: specify native-hook purity, input bounds, rollback and memoization eligibility.
- [x] P1.4e: specify production CST builder checkpoint generation and red-view ownership.
- [ ] P1.4f: verify Phase 0 compiler/CI/sanitizer baseline and license/provenance policy.
- [ ] P1.4g: run and archive complete test matrix against the exact GitHub revision.

**Gate:** Phase 1 must remain open until the documented Phase 1 exit criteria are
satisfied or explicitly re-scoped in the authoritative master implementation plan.
Do not promote tentative prototypes to stable public APIs merely because their tests pass.

## Batch 1 verification note (P1.4a–c)

- Added structured `FailureReason`, terminal outcome validation, and explicit limit/cut failure metadata. This is a contract-level mapping, **not** PEG runtime status production.
- Added session-owned lexer mode/indent/byte-offset state captured by parser checkpoints; external language-specific lexer integration remains deferred.
- Added a farthest-failure expectation accumulator (survives branch rollback) distinct from tentative diagnostics (rolled back) and committed diagnostic records (survive rollback). The latter charge the monotonic diagnostic budget.
- New isolated tests: `parse.result`, `lexer.checkpoint`, `parse.failures`.
- **Batch 1 is contract-level only.** Other P1.4 items, Phase 0 baseline, and final Phase 1 acceptance remain open.

## Batch 2 verification note (P1.4d)

- Native descriptor contracts now include effect, context, rollback, cancellation, lookahead declaration, and memoization eligibility. Unsafe static descriptors fail validation.
- Frozen grammar now exposes immutable rule definitions and native metadata; native-contract flags participate in the fingerprint.
- Isolated tests: `grammar.native_contract`, `grammar.freeze_contract`, `grammar.compile_contract`.
- These are **declarative contracts**, not runtime enforcement of native hooks or a formal proof of purity. Runtime enforcement remains Phase 2 work.
- All original Phase 1 acceptance items stay subject to end-of-phase review; P1.4e–g remain open.

## Batch 3 verification note (P1.4e, 2026-11-02)

- Implemented a standalone, transaction-aware `CstEventBuilder` on top of the default flat green-tree implementation. Nested rollback invalidates younger handles; stale/foreign handles are rejected. Building an unfinished tree or while a checkpoint remains open is rejected.
- `RedNode` provides immutable parent-relative navigation and absolute byte ranges while retaining ownership of the immutable green tree. Red views are safe after original tree/source handles are released.
- Focused tests `syntax.builder`, `syntax.red_view` build independently, and the complete reconstructed test suite passes Debug and Release (24/24); new sanitizer tests pass (2/2, leak checking disabled).
- **P1.4e closed at contract level.** No PEG executor exists yet; coordination with `SessionContracts` checkpoints and incremental red-view caches remain deferred to runtime phases.
- P1.4f and P1.4g remain open, so Phase 1 is **not** signed off.

## Batch 4 — baseline and final audit (2026-11-02)

- Added build/test presets, GCC/Clang GitHub Actions workflow, warnings-as-errors baseline, and standalone contract smoke executable.
- Converted the three original `assert()`-based tests to Release-safe `CHECK()` assertions after compiler warnings exposed unused variables under `NDEBUG`.
- Reconstructed-source verification: GCC Debug/Release/ASan+UBSan and Clang Debug all pass **25/25**; ASan leak detection disabled. Fresh focused target build compiled no unrelated test `.cpp` sources.
- Added `docs/phase0-baseline.md` and `docs/phase1-batch4-verification.md` for provenance, test evidence, and unresolved baseline decisions.
- **P1.4f/g NOT SIGNED OFF**: exact GitHub SHA and CI results are unavailable from this environment; portability/ABI and public licensing decisions also remain explicit. Leave original sign-off checklist item unchecked.
