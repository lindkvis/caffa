#pragma once

#include "TestObj.h"
#include "cafChildField.h"
#include "cafObject.h"
#include "cafObservingPointer.h"

class Child : public caffa::Object
{
    CAFFA_HEADER_INIT;

public:
    Child();
    ~Child();

    caffa::ChildField<TestObj*> m_testObj;
};
