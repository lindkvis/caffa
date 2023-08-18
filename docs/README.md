# About Caffa
Caffa is an Application Framework for Embedded server applications written in C++. It features Runtime Introspection, serialisation and a REST-interface 

Caffa is intended to help write applications with strong separation between application logic and the data model and allow for unforseen new methods of accessing objects using introspection. The main target for Caffa is to create simple control applications for embedded Linux systems or client/server c++-applications with a shared client and server code base.

As an example, you would write Data Model Objects with Fields holding data instead of simple variables. This gives you runtime introspection of the fields without using a pre-compiler and all objects can easily be written out to JSON. Caffa is set up for allowing scripting access by utilising the introspection capabilites to optionally expose fields and objects to scripting languages with little additional work from the application developer.

The REST-interface is designed to be as transparent as possible, and fields and methods can be accessed remotely from the client as if they are local fields through the use of remote REST-accessors being applied to them when the object is instantiated on the client.

~~~cpp
#pragma once
#include "cafObject.h"

using namespace caffa;

class ChildObject : public Object
{
    // Repeat the class name and parent Caffa Object
    // This registers methods for inspecting the class hierarchy
    CAFFA_HEADER_INIT(DemoObject, Object)

public:
    // Caffa classes must be default instantiable, since they are created by a factory for serialization
    // But as long as they have default values for all parameters, we are good!
    ChildObject(const std::string& childName = "");

public:
    Field<std::string> name;
};

class TinyDemoObject : public Object
{
    CAFFA_HEADER_INIT_WITH_DOC("A tiny demo object with some documentation", TinyDemoObject, Object);

public:
    enum TestEnumType
    {
        T1,
        T2,
        T3
    };

    TinyDemoObject();
    ~TinyDemoObject() noexcept override;

public:

    Field<bool>                     toggleField;
    Field<double>                   doubleField;
    Field<int>                      intField;
    Field<std::vector<int>>         intVectorField;
    Field<AppEnum<TestEnumType>>    enumField;
    ChildArrayField<ChildObject>    children; // ChildArrayFields hold a vector of Caffa Objects
    ChildField<ChildObject>         specialChild; // Child fields hold a single Caffa Object

public:
    Method<void(double)>            scaleDoubleField; // A registered method taking a double parameter
};
~~~

In the cpp file you then register the object and fields.

~~~cpp
CAFFA_SOURCE_INIT(ChildObject)

ChildObject::ChildObject(const std::string& childName)
{
    initField(name, "name").withDefault(childName).withScripting();
}

CAFFA_SOURCE_INIT(TinyDemoObject)

// Must be in caffa namespace
namespace caffa
{
template <>
void AppEnum<TinyDemoObject::TestEnumType>::setUp()
{
    // Register enum values with a corresponding text string
    addItem( TinyDemoObject::T1, "T1" );
    addItem( TinyDemoObject::T2, "T2" );
    addItem( TinyDemoObject::T3, "T3" );
    setDefault( TinyDemoObject::T1 );
}

} // namespace caffa


TinyDemoObject::TinyDemoObject()
{
    initField(toggleField, "Toggle").withDefault(true).withScripting();    
    initField(doubleField, "Number").withDefault(11.0).withScripting();
    initField(intField, "Integer").withDefault(42).withScripting();
    initField(enumField, "Enum").withScripting();
    initField(intVectorField, "Integers").withScripting();
    initField(children, "Children").withScripting();
    initFIeld(specialChild, "SpecialChild"); // Omitted withScripting. This field will not be remote accessible.
    
    initMethod(scaleDoubleField, "scaleDouble", {"scalingFactor"}, [this](double scalingFactor)
    {
        this->doubleField.setValue(this->doubleField.value() * scalingFactor);
    });


    // Add a couple of children to the child array field
    children.push_back(std::make_shared<ChildObject>("Alice"));
    children.push_back(std::make_shared<ChildObject>("Bob"));

    // Set the single child object
    specialChild = std::make_shared<ChildObject>("Balthazar");
}
~~~

# Requirements
Caffa uses modern C++ and requires a C++20 compatible compiler, Boost 1.71.0+ and CMake 3.16+.

# Building
Caffa uses git submodules so it is important to initialise submodules recursively first

```bash
git submodule update --init --recursive
```

# Licensing
Caffa is licensed under the LGPL 2.1 or newer.
