#pragma once

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafObjectHandle.h"

class Child;

class Parent : public caffa::ObjectHandle
{
public:
    Parent();

    void                           doSome();
    std::string                    classKeyword() const override { return "Parent"; }
    caffa::ChildArrayField<Child*> m_simpleObjectsField;
    caffa::ChildField<Child*>      m_simpleObjectF;
};
