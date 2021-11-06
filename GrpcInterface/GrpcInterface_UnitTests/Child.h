#pragma once

#include "TestObj.h"
#include "cafChildField.h"
#include "cafObjectHandle.h"
#include "cafObservingPointer.h"

class Child : public caffa::ObjectHandle
{
public:
    Child();
    ~Child();

    caffa::ChildField<TestObj*> m_testObj;
};
