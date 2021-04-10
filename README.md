# About Caffa
Caffa is an Application Framework for Embedded and Desktop Applications written in C++. Caffa is based on the Qt5-based Ceetron Application Framework used in ResInsight (http://www.resinsight.org) but has a simplified API and uses JSON for serialisation.

It allows you to write applications both for desktop console deployment with one code base. The main target for Caffa is to create simple control applications for embedded Linux systems.

As an example, you would write Data Model Objects with Fields holding data instead of simple variables. This gives you runtime introspection of the fields without using a pre-compiler and all objects can easily be written out to JSON. Caffa is set up for allowing scripting access by utilising the introspection capabilites to optionally expose fields and objects to scripting languages with little additional work from the application developer.

```C++
class TinyDemoObject : public caf::Object
{
    CAF_HEADER_INIT;

public:
    TinyDemoObject();

private:
    caf::Field<bool>   m_toggleField;
    caf::Field<double> m_doubleField;
    caf::Field<int>    m_scriptableIntField;
};
```

In the cpp file you then register the object and fields.
```C++
CAF_SOURCE_INIT(TinyDemoObject, "TinyDemoObject");

TinyDemoObject::TinyDemoObject()
{
    InitObject();

    InitField(m_toggleField, "Toggle", false);
    InitField(m_doubleField, "Number", 0.0).withUi("Number", "", "Enter a number here", "Double precision number");
    InitField(m_scriptableIntField, "Integer", 42).withScripting("AnInteger");
}
```
# Requirements
Caffa uses modern C++ and requires a C++17 compatible compiler and CMake 3.12+. It requires Qt5 for desktop deployment but not for console or headless gRPC-server deployment.

# Licensing
Caffa is licensed under the LGPL 2.1. 
