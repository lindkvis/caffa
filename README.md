# About Caffa
Caffa is an Application Framework for Embedded and Desktop Applications written in C++. It features Runtime Introspection, serialization, a gRPC-interface and an optional Qt5 GUI.

Caffa is based on the Qt5-based Ceetron Application Framework used in ResInsight (http://www.resinsight.org) but has a simplified API and uses JSON for serialisation.

It allows you to write applications both for desktop and console deployment with one code base with built-in gRPC-based scripting. The main target for Caffa is to create simple control applications for embedded Linux systems.

As an example, you would write Data Model Objects with Fields holding data instead of simple variables. This gives you runtime introspection of the fields without using a pre-compiler and all objects can easily be written out to JSON. Caffa is set up for allowing scripting access by utilising the introspection capabilites to optionally expose fields and objects to scripting languages with little additional work from the application developer.

```C++
class ChildObject : public caffa::Object
{
    CAF_HEADER_INIT;
public:
    ChildObject();

  ...

};

class TinyDemoObject : public caffa::Object
{
    CAF_HEADER_INIT;

public:
    TinyDemoObject();

private:
    caffa::Field<bool>                     m_toggleField;
    caffa::Field<double>                   m_doubleField;
    caffa::Field<int>                      m_scriptableIntField;
    caffa::ChildArrayField<SomeOtherClass> m_children; // ChildArrayFields hold caffa::Objects
};
```

In the cpp file you then register the object and fields.
```C++
CAF_SOURCE_INIT(TinyDemoObject, "TinyDemoObject", "Object");

TinyDemoObject::TinyDemoObject()
{
    assignUiInfo("A tiny object", ":/anIcon.png", "A tooltip", "What's this?"); // Optional Ui data
    
    initField(m_toggleField, "Toggle", false);
    initField(m_doubleField, "Number", 0.0).withUi("Number", "", "Enter a number here", "Double precision number");
    initField(m_scriptableIntField, "Integer", 42).withScripting("AnInteger");
    initField(m_children, "Children");

    m_children.push_back(std::make_unique<ChildObject>)());
}
```
# Requirements
Caffa uses modern C++ and requires a C++17 compatible compiler and CMake 3.12+. It requires Qt5 for desktop deployment but not for console or headless gRPC-server deployment.

# Licensing
Caffa is licensed under the LGPL 2.1. 
