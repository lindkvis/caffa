#pragma once

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafObject.h"
#include "cafPointer.h"

class Child;

class Parent : public caf::Object
{
    CAF_HEADER_INIT;

public:
    Parent();
    ~Parent();

    void doSome();

    caf::ChildArrayField<Child*> m_simpleObjectsField;
    caf::ChildField<Child*>      m_simpleObjectF;
};
