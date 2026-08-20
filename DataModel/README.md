# Caffa Data Model
C++ Application Framework for Introspection Data Model Library

The reflection layer of caffa: `ObjectHandle`, fields, child fields, capabilities, factories,
visitors and method handles.

This was split into its own repository so that lower level libraries could depend on it without
caffa, and then brought back in when it turned out nothing did - `caffaCore` was its only consumer.
It now lives in the caffa repository, imported with `git subtree`. See the root `README.md` and
`docs/caffa_architecture.md` for the current layering.

`Base` below this directory is still a separate repository, because that one genuinely is consumed
on its own.
