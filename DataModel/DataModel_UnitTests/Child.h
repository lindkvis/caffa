#pragma once

#include "TestObj.h"
#include "cafChildField.h"
#include "cafObjectHandle.h"
#include "cafObjectMacros.h"

class Child : public caffa::ObjectHandle
{
    CAFFA_HEADER_INIT( Child, ObjectHandle )

public:
    Child();
    ~Child();

    caffa::ChildField<TestObj*> m_testObj;
};
