# About Caffa
Caffa is an Application Framework for Embedded server applications written in C++. It features Runtime Introspection, serialisation and a gRPC-interface 

Caffa has been forked from the Ceetron Application Framework used as part of ResInsight (https://github.com/OPM/ResInsight) but has diverged considerably with a simplified API, removal of User Interface code and uses JSON rather than XML for serialisation.

Caffa is intended to help write applications with strong separation between application logic and the data model and allow for unforseen new methods of accessing objects using introspection. The main target for Caffa is to create simple control applications for embedded Linux systems or client/server c++-applications with a shared client and server code base.

As an example, you would write Data Model Objects with Fields holding data instead of simple variables. This gives you runtime introspection of the fields without using a pre-compiler and all objects can easily be written out to JSON. Caffa is set up for allowing scripting access by utilising the introspection capabilites to optionally expose fields and objects to scripting languages with little additional work from the application developer.

~~~{.cpp}
using namespace caffa;

class ChildObject : public caffa::Object
{
    CAFFA_HEADER_INIT;
public:
    ChildObject();

  ...

};

class TinyDemoObject : public caffa::Object
{
    CAFFA_HEADER_INIT;

public:
    TinyDemoObject();

private:
    Field<bool>                     m_toggleField;
    Field<double>                   m_doubleField;
    Field<int>                      m_scriptableIntField;
    ChildArrayField<ChildObject>    m_children; // ChildArrayFields hold caffa::Objects
};
~~~

In the cpp file you then register the object and fields.
~~~{.cpp}
CAFFA_SOURCE_INIT(TinyDemoObject, "TinyDemoObject", "Object");

TinyDemoObject::TinyDemoObject()
{
    initField(m_doubleField, "Number", 0.0);
    initField(m_scriptableIntField, "Integer", 42).withScripting("AnInteger");
    initField(m_children, "Children");
    
    initField(m_toggleField, "Toggle", false).withScripting();   
    
    m_children.push_back(std::make_unique<ChildObject>)());
}
~~~
# Requirements
Caffa uses modern C++ and requires a C++17 compatible compiler, Boost 1.71.0+ and CMake 3.12+ in addition to gRPC 1.16.1+ for gRPC-server deployment.

# Licensing
Caffa is licensed under the LGPL 2.1 or newer.
