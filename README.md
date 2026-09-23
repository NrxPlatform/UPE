# Universal Parsing Engine (UPE)

A native C++ project to build an independently implemented, multi-language syntax parsing engine. The long-term goal is **complete, lossless syntax parsing** for explicitly versioned languages, with a shared parsing runtime and language-specific frontends. Semantic analysis is a separate layer.

> **Status: early Phase 1 starter.** The current code is a source-management prototype, **not yet a working language parser**. It implements immutable source snapshots, byte-oriented ranges and slices, line mapping, and three focused tests. PEG execution, grammars, tokens, diagnostics, CSTs, and language frontends are not implemented yet.

## Architecture direction

- C++20 implementation and C++-authored grammar specifications.
- An owned PEG-based runtime, with specialized native parsing algorithms where required.
- A lossless concrete syntax tree (CST) and language-specific typed AST views.
- Separate syntax parsing and semantic analysis.
- One pinned language version per frontend initially; external parsers may be used for development-time conformance comparisons, not as production parser dependencies.
- Incremental parsing is a future optimization, not a current feature.

The planned language coverage includes Python, C, C++, JavaScript, TypeScript, Java, Go, Rust, HTML5, CSS, XML, Markdown, SQL (beginning with a PostgreSQL dialect), Cypher, JSON, YAML, TOML, Bash, CMake, and Jinja2. **These are roadmap targets, not currently supported languages.**

## Requirements

- A C++20-capable compiler.
- CMake 3.20 or newer.
- CTest (included with CMake).

No third-party parser runtime is required by the current starter.

## Build and test

From the repository root:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug

# Build only one focused test and its required library dependencies.
cmake --build build --target test_source_snapshot -j 2

# Run only the selected test.
ctest --test-dir build -R '^source\.snapshot$' --output-on-failure
```

Other independently buildable targets:

| CMake target | CTest name | Focus |
| --- | --- | --- |
| `test_source_snapshot` | `source.snapshot` | Immutable source snapshots |
| `test_source_slice` | `source.slice` | Bounded byte slices |
| `test_source_lines` | `source.lines` | Byte-oriented line mapping |

For example:

```sh
cmake --build build --target test_source_slice -j 2
ctest --test-dir build -R '^source\.slice$' --output-on-failure
```

**CTest runs tests; it does not compile them.** Build a selected target before running it. Focused test executables are excluded from the default build, so compiling one test does not compile unrelated test executables or language frontends. Shared library dependencies are still built when required.

To build and run all three current tests explicitly:

```sh
cmake --build build --target test_source_snapshot test_source_slice test_source_lines -j 2
ctest --test-dir build --output-on-failure
```

## Repository layout

```text
include/upe/           Public C++ headers
src/                   Current source-management implementation
tests/                 Independently compiled focused tests
docs/phase1-checklist.md  Current Phase 1 progress and remaining work
CMakeLists.txt         Build and test target definitions
```

The full architecture, implementation plan, acceptance policy, language matrix, grammar API, and syntax-tree contracts are maintained as separate project documentation. Add those documents under `docs/` when publishing them alongside this starter.

## Development approach

Implement a small contract, add focused positive and negative tests, build its target independently, and only then advance to the next component. Keep the core runtime independent of language frontends and repository/database integration. See `docs/phase1-checklist.md` for the current milestone.

## Contributing and license

The project is in its initial architecture and implementation stage. Please open an issue before undertaking substantial changes to core contracts or grammar APIs.

**License not yet selected.** Before inviting contributions or accepting pull requests, add a `LICENSE` file and document the contribution terms. Publishing a repository publicly does not by itself grant others permission to reuse or redistribute its code.
