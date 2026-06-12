# Repository Guidelines

## Document Duplication

The following document is exactly duplicated in two places AGENTS.md and CLAUDE.md. This is due
to difficulties maintaining symbolic links on different platforms. The files should always
remain exactly the same and projects should have CI rules that enforce this.

## Project Structure & Module Organization

This is the CAFFA C++20 application framework. It provides introspected object graphs, JSON
serialization, schema generation, RPC-style client/server access, and REST exposure. Core C++ code is
split across `DataModel/Base` for shared utilities, `DataModel` for reflection primitives, `Core` for
`Object`/`Document`, JSON IO, sessions, methods, scripting, and validators, `RpcBase` for
transport-independent client/server abstractions and remote accessors, and `RestInterface` for the
Boost.Beast REST transport and C++ REST client. Example and test support lives under
`RestInterface/RestInterface_Example`, `RestInterface/RestInterface_TestData`, and the `*_UnitTests`
directories. Python and Java bindings are checked out under `RestInterface/Bindings/` and use their
own toolchains.

## Architecture Documentation

Use `docs/caffa_architecture.md` as the map before making cross-layer assumptions, then verify
implementation details against the current code. The architecture spans Base -> DataModel -> Core ->
RpcBase -> RestInterface -> language bindings.

Task-scope guidance:

* For reflection, field, capability, factory, visitor, or object-graph work, read
  `docs/caffa_architecture.md` first and verify details in `DataModel/`, `DataModel/Base/`, and
  `Core/`.
* For serialization, schema, document, method, session, validator, or scripting work, read
  `docs/caffa_architecture.md` first and verify details in `Core/`.
* For RPC, REST server/client, authentication, routing, sessions, or remote accessor work, read
  `docs/caffa_architecture.md` first and verify details in `RpcBase/` and `RestInterface/`.
* For Python or Java binding work, read `docs/caffa_architecture.md` first, then use
  `RestInterface/Bindings/Python/caffa/`, `RestInterface/Bindings/Java/`, and nearby binding
  README/build files.
* If documentation and code disagree, trust the code for implementation details and mention the
  discrepancy in your response.

## Build, Test, and Development Commands

```sh
git submodule update --init --recursive
cmake -S . -B build
cmake --build build -j10
ctest --test-dir build -V
```

Use these CMake options to narrow builds when useful:

```sh
cmake -S . -B build -DCAFFA_REST_INTERFACE=ON -DCAFFA_BUILD_UNIT_TESTS=ON -DCAFFA_BUILD_EXAMPLES=ON
cmake -S . -B build -DCAFFA_BUILD_DOCS=ON
cmake -S . -B build -DCAFFA_BUILD_SHARED=OFF
```

The root CMake build includes `DataModel` and `DataModel/Base`. The Java and Python binding
submodules are present in the tree but are built by their own Gradle/Python packaging workflows, not
by the root CMake build.

## Coding Style & Naming Conventions

Follow `.clang-format`: 4-space indentation, Allman braces, 120-column limit, sorted includes, no
tabs, and left pointer alignment. Follow `.clang-tidy`, currently focused on selected modernize
checks such as `modernize-use-nullptr`, `modernize-use-override`, and
`modernize-deprecated-headers`. C++ types use PascalCase where established by the framework; fields,
methods, keywords, and accessors should follow nearby code and the public CAFFA naming conventions.
Prefer existing CAFFA abstractions such as `Object`, `Document`, `Field<T>`, `ChildField`,
`ChildArrayField`, field capabilities, visitors, factories, serializers, and remote accessors over
new ad hoc plumbing.

## Testing Guidelines

Tests use GoogleTest through CMake/CTest. Add focused tests near the affected layer:
`DataModel/DataModel_UnitTests` for reflection/data-model behavior, `DataModel/Base/Base_UnitTests`
for low-level utilities, `Core/IoCore_UnitTests` and `Core/ProjectDataModel_UnitTests` for core
serialization/document behavior, and `RestInterface/RestInterface_UnitTests` for REST/RPC behavior.
Run `ctest --test-dir build -V`; individual test binaries are emitted under `build/bin`.

## Commit & Pull Request Guidelines

Keep subjects concise, imperative, and specific. Pull requests should describe behavior changes,
affected libraries/bindings, test results, and any required submodule, schema, REST API, or binding
compatibility updates. Call out public API, serialized JSON, schema, or wire-format changes
explicitly.

When an LLM makes a commit, the commit message must end with a `Co-Authored-By` trailer identifying the harness, model, and effort level. For example, the final line should be something like `Co-Authored-By: Codex GPT-5.5 (High) <codex@openai.com>` for Codex and GPT.

## Configuration & Security Notes

Do not commit real credentials, local tokens, generated build directories, local logs, cache
directories, or generated documentation output unless it is intentionally versioned. Be careful with
REST authentication/session changes and avoid weakening default security behavior.

## Architecture Documentation Tasks

When asked to explain or reason about the architecture, always write the following documents:

1. `tmp/{task}_{name_of_engine}/result.md`
2. `tmp/{task}_{name_of_engine}/notes.md`
3. `tmp/{task}_{name_of_engine}/raw_output.md`

Where `task` is a concise 1-3 word slug summarizing what you were asked to do, and `name_of_engine`
is the lowercase engine name, such as `claude` or `codex`. The first file should contain your output
document. The second file should contain metadata about the task. The notes should use this template,
replacing placeholder values with metadata for the current run. If token accounting is unavailable,
write `Unknown`; add or remove token rows, such as `Input (cached)` or `Output (reasoning)`, based on
what is actually available.

Inside the metadata the MODEL_NAME should be a fully resolved model name, i.e. GPT-5.5 High (for
High thinking level).

```md
# Run Metadata

| Field | Value |
|---|---|
| **Model** | {MODEL_NAME} |
| **Repo** | `caffa` |
| **Tests/build run** | {YES_OR_NO_COMMANDS_IF_ANY} |
| **Timing** | {ROUGH_DURATION_OF_THINKING_BEFORE_RESULT} |
| **Total Token Usage** | {TOTAL_TOKENS_USED_OR_UNKNOWN} |

For Claude you may have to run /usage to fill in the total token usage.

The third file (raw output) should be a transcript of the session, starting from when the task was
initiated.

## Task

{Brief one paragraph description of the task, including the exact prompt when useful}

## Approach

{Brief one paragraph description of your approach}

## Tools Used

{List the external tools and commands used, such as `rg`, `sed`, or `apply_patch`}

## Caveats

{List anything you think needs to be mentioned}
```
