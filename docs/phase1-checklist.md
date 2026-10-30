# Phase 1 acceptance checklist

- [x] Prototype immutable byte-owning source snapshots and bounded slices.
- [x] Prototype byte-based line mapping for CRLF, CR, LF.
- [x] Independently buildable source test executables.
- [x] Implement and test BOM/invalid UTF-8 policy and Unicode coordinate adapters (P1.1).
- [x] Implement token/trivia, synthetic and error element contracts and diagnostic records (P1.1).
- [ ] Compare two green-node layouts with measured memory/traversal/lifetime tests.
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
