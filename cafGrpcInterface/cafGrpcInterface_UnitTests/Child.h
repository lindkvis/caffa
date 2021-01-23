#pragma once

#include "cafChildField.h"
#include "cafObjectHandle.h"
#include "cafPdmPointer.h"

class TestObj;

class Child : public caf::ObjectHandle
{
public:
    Child();
    ~Child();

    caf::ChildField<TestObj*> m_testObj;
};
