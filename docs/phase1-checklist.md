# Phase 1 acceptance checklist

- [x] Prototype immutable byte-owning source snapshots and bounded slices.
- [x] Prototype byte-based line mapping for CRLF, CR, LF.
- [x] Independently buildable source test executables.
- [ ] Specify and test BOM/invalid encoding policy and Unicode coordinate adapters.
- [ ] Define token/trivia, synthetic and error token ownership and diagnostics.
- [ ] Compare two green-node layouts with measured memory/traversal/lifetime tests.
- [ ] Specify event-based CST builder with rollback checkpoints.
- [ ] Compile-only examples for all C++ grammar combinators and native hooks.
- [ ] Grammar graph validation and freeze API contracts.
- [ ] Explicit parse status, resource budget, speculative diagnostics and cut contracts.
- [ ] Phase 0 baseline and Phase 1 contract review/signoff.

Phase 1 is not accepted until all checklist items and the exit criteria in `01-implementation-plan.md` are satisfied.
