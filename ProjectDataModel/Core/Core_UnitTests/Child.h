#pragma once

#include "TestObj.h"
#include "cafChildField.h"
#include "cafObjectHandle.h"
#include "cafPointer.h"

class Child : public caffa::ObjectHandle
{
public:
    Child();
    ~Child();

    caffa::ChildField<TestObj*> m_testObj;
};
