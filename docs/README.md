# About CAFFA

CAFFA is a C++20 library for runtime introspection and JSON serialization of C++ object graphs. It
models application data as `caffa::Object` and `caffa::Document` instances with reflected fields,
child objects, child arrays, methods, capabilities, validators, and factories.

CAFFA 2.0 is intentionally transport-free. It does not provide REST/RPC servers, C++ remote clients,
or Python/Java bindings. Applications that need an HTTP API or another transport should build that
API outside CAFFA and use CAFFA only for local introspection, schema/data serialization, validation,
and object traversal.

# Advantages

- Runtime introspection without a pre-compiler.
- JSON serialization and deserialization through a single `JsonSerializer` entry point.
- Object factories keyed by class keyword for recreating object graphs from JSON.
- Field capabilities for IO, scripting metadata, documentation, and validation.
- Local methods that can expose typed argument metadata and JSON schemas.

# Example

```cpp
#pragma once

#include "cafObject.h"

using namespace caffa;

class ChildObject : public Object
{
    CAFFA_HEADER_INIT(ChildObject, Object)

public:
    explicit ChildObject(const std::string& childName = "");

    Field<std::string> name;
};

class TinyDemoDocument : public Document
{
    CAFFA_HEADER_INIT_WITH_DOC("A tiny object with documentation", TinyDemoDocument, Document)

public:
    enum TestEnumType
    {
        T1,
        T2,
        T3
    };

    TinyDemoDocument();
    ~TinyDemoDocument() noexcept override;

    Field<bool>                  toggleField;
    Field<double>                doubleField;
    Field<int>                   intField;
    Field<std::vector<int>>      intVectorField;
    Field<AppEnum<TestEnumType>> enumField;
    ChildArrayField<ChildObject*> children;
    ChildField<ChildObject*>      specialChild;

    Method<void(double)> scaleDoubleField;
};
```

Register the classes, enum values, fields, and methods in the implementation file:

```cpp
CAFFA_SOURCE_INIT(ChildObject)

ChildObject::ChildObject(const std::string& childName)
{
    initField(name, "name").withDefault(childName).withScripting();
}

CAFFA_SOURCE_INIT(TinyDemoDocument)

namespace caffa
{
template <>
void AppEnum<TinyDemoDocument::TestEnumType>::setUp()
{
    addItem(TinyDemoDocument::T1, "T1");
    addItem(TinyDemoDocument::T2, "T2");
    addItem(TinyDemoDocument::T3, "T3");
    setDefault(TinyDemoDocument::T1);
}
} // namespace caffa

TinyDemoDocument::TinyDemoDocument()
{
    initField(toggleField, "toggle").withDefault(true).withScripting();
    initField(doubleField, "number").withDefault(11.0).withScripting();
    initField(intField, "integer").withDefault(42).withScripting();
    initField(enumField, "enum").withScripting();
    initField(intVectorField, "integers").withScripting();
    initField(children, "children").withScripting();
    initField(specialChild, "special_child");

    initMethod(scaleDoubleField, "scale_double", [this](double scalingFactor)
    {
        doubleField.setValue(doubleField.value() * scalingFactor);
    }).withArgumentNames({"scaling_factor"});

    children.push_back(std::make_shared<ChildObject>("Alice"));
    children.push_back(std::make_shared<ChildObject>("Bob"));
    specialChild = std::make_shared<ChildObject>("Balthazar");
}
```

Fields and methods can be accessed locally:

```cpp
auto doc = std::make_shared<TinyDemoDocument>();
doc->toggleField = true;
int currentIntValue = doc->intField;
doc->scaleDoubleField(3.0);
```

Objects can be serialized to JSON:

```cpp
auto child = doc->children.objects().front();
auto json = caffa::JsonSerializer().writeObjectToString(child.get());
```

The child object serializes as:

```json
{
  "keyword": "ChildObject",
  "name": "Alice"
}
```

# Requirements

CAFFA requires a C++20-compatible compiler, CMake 3.22+, Boost JSON, Boost Regex, Boost UUID, and
GoogleTest when building unit tests.

# Building

Initialize submodules recursively first:

```bash
git submodule update --init --recursive
```

Configure, build, and test with CMake:

```bash
cmake -S . -B build
cmake --build build -j10
ctest --test-dir build -V
```

Useful options:

```bash
cmake -S . -B build -DCAFFA_BUILD_UNIT_TESTS=ON
cmake -S . -B build -DCAFFA_BUILD_DOCS=ON
cmake -S . -B build -DCAFFA_BUILD_SHARED=OFF
```

# Licensing

CAFFA is licensed under the LGPL 2.1 or newer.
