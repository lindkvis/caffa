#pragma once

#include "cafChildField.h"
#include "cafObject.h"
#include "cafPointer.h"

class TestObj;

class Child : public caffa::Object
{
    CAF_HEADER_INIT;

public:
    Child();

    ~Child();

    caffa::ChildField<TestObj*> m_testObj;
};
