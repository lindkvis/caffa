#pragma once

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafObjectHandle.h"
#include "cafObjectMacros.h"

class Child;

class Parent : public caffa::ObjectHandle
{
    CAFFA_HEADER_INIT( Parent, ObjectHandle );

public:
    Parent();

    void doSome();

    caffa::ChildArrayField<Child*> m_simpleObjectsField;
    caffa::ChildField<Child*>      m_simpleObjectF;
};
