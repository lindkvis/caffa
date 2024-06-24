#pragma once

#include "TestObj.h"
#include "cafObject.h"

class Child : public caffa::Object
{
    CAFFA_HEADER_INIT( Child, Object )

public:
    Child();

    ~Child();

    caffa::Field<std::shared_ptr<TestObj>> m_testObj;
};
