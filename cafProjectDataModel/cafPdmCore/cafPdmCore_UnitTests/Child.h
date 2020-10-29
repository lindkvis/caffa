#pragma once

#include "cafPdmChildField.h"
#include "cafObjectHandle.h"
#include "cafPdmPointer.h"

class TestObj;

class Child : public caf::ObjectHandle
{
public:
    Child();
    ~Child();

    caf::PdmChildField<TestObj*> m_testObj;
};
