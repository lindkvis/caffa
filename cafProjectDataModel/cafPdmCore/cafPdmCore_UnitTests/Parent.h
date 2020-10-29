#pragma once

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafObjectHandle.h"

class Child;

class Parent : public caf::ObjectHandle
{
public:
    Parent();
    ~Parent();

    void doSome();

    caf::PdmChildArrayField<Child*> m_simpleObjectsField;
    caf::PdmChildField<Child*>      m_simpleObjectF;
};
