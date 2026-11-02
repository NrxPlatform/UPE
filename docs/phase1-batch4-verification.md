# P1.4 Batch 4 — local verification record

Date: 2026-11-02 (UTC date). Source basis: reconstructed files from the starter and supplied P1.1–P1.4 patch archives, **not** a verified checkout of the public GitHub repository. Git commit SHA: **unavailable**; tree is not a Git checkout. Corpus/reference parser: none (contract-only synthetic fixtures).

| Matrix entry     | Compiler / platform                           | Result     | Notes                                                                                                     |
| ---------------- | --------------------------------------------- | ---------- | --------------------------------------------------------------------------------------------------------- |
| Debug            | GCC 14.2.0, Linux x86_64                      | 25/25 pass | CMake 3.31.6, C++20, warnings-as-errors                                                                   |
| Release          | GCC 14.2.0, Linux x86_64                      | 25/25 pass | `NDEBUG`; starter tests converted to always-active `CHECK()`                                              |
| ASan + UBSan     | GCC 14.2.0, Linux x86_64                      | 25/25 pass | Debug, `-fsanitize=address,undefined`; leak detection disabled                                            |
| Debug            | Clang (local installed version), Linux x86_64 | 25/25 pass | Warnings-as-errors active                                                                                 |
| Target isolation | GCC 14.2.0, Linux x86_64                      | 1/1 pass   | Fresh focused build of `test_token_trivia` compiled only `tests/test_token_trivia.cpp` among test sources |

All results are local to the reconstructed source; no GitHub Actions run has been observed or certified.

## Phase 1 contract review

- Event-based builder, rollback/generation checks, red lifetime, grammar compile/validation/freeze, parse-session budgets/results/diagnostics/cut are backed by focused tests from prior batches.
- Native hooks are **declarative only**; actual execution and enforcement of declared side effects belong to Phase 2.
- Cross-component tree construction is tested, but production PEG execution is not implemented.
- Independent compilation remains required. `upe_tests` includes a separate Phase 0 contract smoke target without forcing benchmarks to build.
- C++20 is the baseline. Exact compiler minimum versions, cross-platform support, and stable ABI are not signed off.

## Pending formal sign-off

1. Apply these changed files to the actual public repository and record its exact commit SHA.
2. Run the GitHub Actions matrix on that SHA and archive result links/artifacts.
3. Inspect that run for compiler/version divergences, reproducibility, and complete test execution.
4. Review remaining Phase 0 items (real parser CLI/runtime/frontends are deferred, not represented as completed) and approve the Phase 1 contract boundary.
5. Only then tick the final `Phase 0 baseline and Phase 1 contract review/signoff` checklist item.
