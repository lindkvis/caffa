# CAFFA Application Framework

CAFFA is a C++20 application framework for introspected object graphs, JSON serialization, schema
generation, RPC-style client/server access, and REST exposure. Application data is modeled with
`caffa::Object`, `caffa::Document`, fields, child fields, methods, capabilities, and factories so the
same model can be used locally, serialized to JSON, exposed over REST, and accessed through remote
clients.

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
| RpcBase | `RpcBase` | Transport-independent client/server abstractions and remote accessors |
| RestInterface | `RestInterface` | Boost.Beast REST server/client implementation, routing, and authentication |
| Bindings | `RestInterface/Bindings` | Python and Java clients built by their own toolchains |

## Build and test

```sh
git submodule update --init --recursive
cmake -S . -B build
cmake --build build -j10
ctest --test-dir build -V
```

Useful CMake options:

```sh
cmake -S . -B build -DCAFFA_REST_INTERFACE=ON -DCAFFA_BUILD_UNIT_TESTS=ON -DCAFFA_BUILD_EXAMPLES=ON
cmake -S . -B build -DCAFFA_BUILD_DOCS=ON
cmake -S . -B build -DCAFFA_BUILD_SHARED=OFF
```

The root CMake build includes `DataModel` and `DataModel/Base`. The Java and Python binding
submodules are checked out under `RestInterface/Bindings/`, but they are built with their own Gradle
or Python packaging workflows.

## Contributing

Follow `.clang-format` and `.clang-tidy`. Keep changes close to the existing layer boundaries and
prefer established CAFFA abstractions over ad hoc field, serialization, REST, or RPC plumbing. Add
GoogleTest coverage near the affected layer: `DataModel/DataModel_UnitTests`,
`DataModel/Base/Base_UnitTests`, `Core/IoCore_UnitTests`, `Core/ProjectDataModel_UnitTests`, or
`RestInterface/RestInterface_UnitTests`.
