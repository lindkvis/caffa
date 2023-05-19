#pragma once

#include "TestObj.h"
#include "cafChildField.h"
#include "cafObject.h"

class Child : public caffa::Object
{
    CAFFA_HEADER_INIT( Child, Object )

public:
    Child();
    ~Child();

    caffa::ChildField<TestObj*> m_testObj;
};
