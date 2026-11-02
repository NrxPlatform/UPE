# Phase 0 baseline — UPE C++ foundation

Status: **provisional, awaiting exact GitHub revision CI evidence**.

## Supported/development baseline

- C++ language mode: C++20 (`upe_source` propagates the requirement).
- CMake: >=3.20, CTest enabled; GCC and Clang on Linux are the initially tested toolchains.
- CMake presets: `debug`, `release`, `sanitizers`. Use `cmake --preset debug && cmake --build --preset debug-tests && ctest --preset debug` (equivalent for other presets).
- Existing `scripts/test.sh` is the primary local workflow; focused test executables are `EXCLUDE_FROM_ALL` and aggregate under `upe_tests` only when requested.
- Compiler warnings: `-Wall -Wextra -Wpedantic -Werror` (GCC/Clang), `/W4 /WX` (MSVC). MSVC is **not yet validated**.
- CI: GitHub Actions builds GCC Debug/Release/ASan+UBSan and Clang Debug. Release and sanitizer suites must pass against the exact commit.
- Optional smoke executable: `cmake --build build/debug --target upe_contract_smoke`; validates source/CST/grammar contract library linkage, **not** end-to-end PEG execution.
- Runtime, CLI, language frontends, and platform ABI are not implemented in Phase 0/1; full `parser_runtime`, `parser_frontend_api` and `parser_cli` targets are deferred until the executable contracts exist.
- No external parser is linked in production libraries; development references must be provenance-tracked.

## Test report record

Capture for every acceptance run: UTC timestamp; exact Git commit SHA and dirty-tree status; OS/architecture; compiler name/version; CMake version; build preset and flags; focused executable names; CTest results/duration; sanitizer configuration; fixture/corpus revision (or `none`); frontend profile (or `contract-only`); reference parser/version (or `none`). Record machine-local results separately from CI.

## Provenance and redistribution policy

- The engine implementation is owned C++ code. Reference parsers and corpora are **development-time tools only**, never bundled in the runtime without review.
- Record origin URL, revision, license identifier, attribution and redistribution restrictions for every imported corpus, sample or third-party source.
- Original generated synthetic tests are distinguished from external fixtures; do not copy external reference suites into Git without checking permissions.
- The public repository is not automatically open-source without an explicit `LICENSE`; do not claim otherwise. Choose a license before external contribution/reuse policy is established. Document future contributors' licensing terms before accepting patches.

## Open verification gates

- [ ] Confirm the exact GitHub commit SHA and clean/dirty tree state.
- [ ] Verify GitHub Actions matrix on that commit and archive run URLs/artifacts.
- [ ] Validate focused build excludes unrelated tests via verbose build output.
- [ ] Review compiler portability beyond the Linux GCC/Clang baseline and record ABI decision.
- [ ] Complete review of Phase 1 acceptance against architecture/specifications before marking final checklist item.
