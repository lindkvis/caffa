#pragma once

#include "cafChildField.h"
#include "cafObject.h"
#include "cafPointer.h"

class TestObj;

class Child : public caf::Object
{
    CAF_HEADER_INIT;

public:
    Child();

    ~Child();

    caf::ChildField<TestObj*> m_testObj;
};
