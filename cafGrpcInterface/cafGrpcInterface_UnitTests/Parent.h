#pragma once

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafObjectHandle.h"

class Child;

class Parent : public caf::ObjectHandle
{
public:
    Parent();

    void doSome();

    caf::ChildArrayField<Child*> m_simpleObjectsField;
    caf::ChildField<Child*>      m_simpleObjectF;
};
