# About Caffa
Caffa is an Application Framework for Embedded server applications written in C++. It features Runtime Introspection, serialisation and a REST-interface 

Caffa is intended to help write applications with strong separation between application logic and the data model and allow for unforseen new methods of accessing objects using introspection. The main target for Caffa is to create simple control applications for embedded Linux systems or client/server c++-applications with a shared client and server code base.

As an example, you would write Data Model Objects with Fields holding data instead of simple variables. This gives you runtime introspection of the fields without using a pre-compiler and all objects can easily be written out to JSON. Caffa is set up for allowing scripting access by utilising the introspection capabilites to optionally expose fields and objects to scripting languages with little additional work from the application developer.

The REST-interface is designed to be as transparent as possible, and fields and methods can be accessed remotely from the client as if they are local fields through the use of remote REST-accessors being applied to them when the object is instantiated on the client.

The REST-interface communicates via JSON adhering to the latest draft [JSON Schema](http://json-schema.org/) specification and the REST-server provides access to both schemas and data.

# Advantages
- Caffa enables easy access to the full power of C++ while the introspection and clear separation of network code and data model gives some of the advantages of higher level languages such as Python.
- Serialisation to JSON is as easy as calling a single method on a Caffa Object.
- You get a REST-interface to your classes virtually "for free" once you have set up your application as a Caffa application.

# Bindings
Caffa has currently implemented client bindings to the following langages:
- Python
- Java

This means you write your server application on your embedded device in C++ and you can write the client code in C++, Python, Java (or any other languages using REST and JSON directly).

# Examples

~~~cpp
#pragma once
#include "cafObject.h"

using namespace caffa;

class ChildObject : public Object
{
    // Repeat the class name and parent Caffa Object
    // This registers methods for inspecting the class hierarchy
    CAFFA_HEADER_INIT(ChildObject, Object)

public:
    // Caffa classes must be default instantiable, since they are created by a factory for serialization
    // But as long as they have default values for all parameters, we are good!
    ChildObject(const std::string& childName = "");

public:
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

public:

    Field<bool>                     toggleField;
    Field<double>                   doubleField;
    Field<int>                      intField;
    Field<std::vector<int>>         intVectorField;
    Field<AppEnum<TestEnumType>>    enumField;
    ChildArrayField<ChildObject*>   children; // ChildArrayFields hold a vector of Caffa Objects
    ChildField<ChildObject*>        specialChild; // Child fields hold a single Caffa Object

public:
    Method<void(double)>            scaleDoubleField; // A registered method
};
~~~

In the cpp file you then register the object and fields.

~~~cpp
CAFFA_SOURCE_INIT(ChildObject)

ChildObject::ChildObject(const std::string& childName)
{
    initField(name, "name").withDefault(childName).withScripting();
}

CAFFA_SOURCE_INIT(TinyDemoDocument)

// Must be in caffa namespace
namespace caffa
{
template <>
void AppEnum<TinyDemoDocument::TestEnumType>::setUp()
{
    // Register enum values with a corresponding text string
    addItem( TinyDemoDocument::T1, "T1" );
    addItem( TinyDemoDocument::T2, "T2" );
    addItem( TinyDemoDocument::T3, "T3" );
    setDefault( TinyDemoDocument::T1 );
}

} // namespace caffa


TinyDemoDocument::TinyDemoDocument()
{
    initField(toggleField, "Toggle").withDefault(true).withScripting();    
    initField(doubleField, "Number").withDefault(11.0).withScripting();
    initField(intField, "Integer").withDefault(42).withScripting();
    initField(enumField, "Enum").withScripting();
    initField(intVectorField, "Integers").withScripting();
    initField(children, "Children").withScripting();
    initField(specialChild, "SpecialChild"); // Omitted withScripting => not remote accessible.
    
    initMethod(scaleDoubleField, "scaleDouble", [this](double scalingFactor)
    {
        this->doubleField.setValue(this->doubleField.value() * scalingFactor).withArgumentNames({"scalingFactor"});
    });


    // Add a couple of children to the child array field
    children.push_back(std::make_shared<ChildObject>("Alice"));
    children.push_back(std::make_shared<ChildObject>("Bob"));

    // Set the single child object
    specialChild = std::make_shared<ChildObject>("Balthazar");
}
~~~

Fields and methods can be accessed locally in the following way:
~~~cpp

    auto doc = std::make_shared<TinyDemoDocument>();
    doc->toggleField = true;
    int currentIntValue = doc->intField;
    doc->scaleDoubleField(3.0);
~~~

If your application inherits the caffa::rpc::RestServerApplication and the document is provided by the server app through your implementation of the virtual document and documents() methods, you can access the same fields and methods remotely. The server object
will automatically be updated.

~~~cpp
    // The result of the document method is a generic document, so need casting.
    auto doc = std::dynamic_pointer_cast<TinyDemoDocument>(client->document("TinyDemoDocument"));
    doc->toggleField = true;
    int currentIntValue = doc->intField;
    doc->scaleDoubleField(3.0);
~~~

~~~cpp
    // The result of the document method is a generic document, so need casting.
    auto doc = std::dynamic_pointer_cast<TinyDemoDocument>(client->document("TinyDemoDocument"));
    doc->toggleField = true;
    int currentIntValue = doc->intField;
    doc->scaleDoubleField(3.0);
~~~

To serialize an object to string or file (both JSON) you can do the following:
~~~cpp
    // The result of the document method is a generic document, so need casting.
    auto doc =  std::dynamic_pointer_cast<TinyDemoDocument>(client->document("TinyDemoDocument"));
    doc->writeToFile(); // Will write it to the file set as "filename" in the document
    auto child = doc->children.objects().front();
    // To file
    child->writeObjectToFile("/tmp/child.json");
    // To string
    auto string = caffa::JsonSerializer().writeObjectToString(child.get());
~~~

The child object will yield the following JSON file:

~~~json
{
  "keyword": "ChildObject",
  "name": "Alice"
}
~~~

The TinyDemoDocument will yield the following JSON:
~~~json
{
  "Children": [
    {
      "keyword": "ChildObject",
      "name": "Alice",
    },
    {
      "keyword": "ChildObject",
      "name": "Bob",
    }
  ],
  "Enum": "T1",
  "Integer": 42,
  "Integers": [],
  "Number": 33,
  "SpecialChild": {
    "keyword": "ChildObject",
    "name": "Balthazar",
  },
  "Toggle": true,
  "fileName": "",
  "id": "Document",
  "keyword": "TinyDemoDocument",
}
~~~

See [ExampleServer.cpp](https://github.com/lindkvis/caffa/blob/master/RestInterface/RestInterface_Example/ExampleServer.cpp) and [ExampleClient.cpp](https://github.com/lindkvis/caffa/blob/master/RestInterface/RestInterface_Example/ExampleClient.cpp) for a
more complete example.

# Requirements
Caffa uses modern C++ and requires a C++20 compatible compiler, Boost 1.71.0+ Nlohmann JSON and CMake 3.16+. 

# Building
Caffa uses git submodules so it is important to initialise submodules recursively first

```bash
git submodule update --init --recursive
```

```bash
git submodule update --init --recursive
```

Dependencies can be build automatically with VCPKG using the vcpkg ninja multi build:
```bash
cmake --preset ninja-multi-vcpkg
cmake --build --preset ninja-vcpkg
```

If you don't have VCPKG the following dependencies are needed:
- Boost (System, Beast, Program Options and Serialization)
- Ninja
- GoogleTest (GTest)
- Nlohmann JSON

These can be installed on ubuntu using:
```bash
apt-get install libboost-all-dev ninja-build googletest libgtest-dev nlohmann-json3-dev
```

Caffa can then be built using the regular ninja multi build:
```bash
cmake --preset ninja-multi
cmake --build --preset ninja
```

# Licensing
Caffa is licensed under the LGPL 2.1 or newer.

# Credits
Caffa is originally adapted from by Ceetron and Ceetron Solutions for the [ResInsight](https://resinsight.org/) reservoir visualisation package. The code bases have, however, diverged over several years of active development.
