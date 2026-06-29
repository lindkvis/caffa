# CAFFA Introspection and Serialization Library

CAFFA is a C++20 library for introspected object graphs, JSON serialization, schema generation, and
local method metadata. Application data is modeled with `caffa::Object`, `caffa::Document`, fields,
child fields, methods, capabilities, and factories so the same model can be traversed locally and
serialized to JSON.

CAFFA 2.0 intentionally does not provide REST/RPC transport or language bindings. Server APIs and
client transports are owned by applications that consume CAFFA's introspection and serialization
surface.

## Development guidance and architecture docs

`AGENTS.md` and `CLAUDE.md` are intentionally identical and contain the working instructions for
agents in this repository. Use `docs/caffa_architecture.md` as the architecture map before making
cross-layer assumptions, then verify details against the current code.

The main C++ layers are:

| Layer | Path | Role |
|---|---|---|
| Base | `DataModel/Base` | Shared assertions, logging, string, UUID, and utility code |
| DataModel | `DataModel` | Reflection primitives, fields, capabilities, factories, visitors, and object handles |
| Core | `Core` | `Object`, `Document`, JSON IO, sessions, methods, scripting, and validators |

## Build and test

```sh
git submodule update --init --recursive
cmake -S . -B build
cmake --build build -j10
ctest --test-dir build -V
```

Useful CMake options:

```sh
cmake -S . -B build -DCAFFA_BUILD_UNIT_TESTS=ON
cmake -S . -B build -DCAFFA_BUILD_DOCS=ON
cmake -S . -B build -DCAFFA_BUILD_SHARED=OFF
```

The root CMake build includes `DataModel`, `DataModel/Base`, and `Core`.

## Contributing

Follow `.clang-format` and `.clang-tidy`. Keep changes close to the existing layer boundaries and
prefer established CAFFA abstractions over ad hoc field or serialization plumbing. Add
GoogleTest coverage near the affected layer: `DataModel/DataModel_UnitTests`,
`DataModel/Base/Base_UnitTests`, `Core/IoCore_UnitTests`, `Core/ProjectDataModel_UnitTests`, or
another focused Core/DataModel test target.
