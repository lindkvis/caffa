#pragma once

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafObject.h"

class Child;

class Parent : public caffa::Object
{
    CAFFA_HEADER_INIT;

public:
    Parent();

    void                           doSome();
    caffa::ChildArrayField<Child*> m_simpleObjectsField;
    caffa::ChildField<Child*>      m_simpleObjectF;
};
