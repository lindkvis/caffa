#pragma once

#include "cafChildField.h"
#include "cafObject.h"
#include "cafPdmPointer.h"

class TestObj;

class Child : public caf::Object
{
    CAF_PDM_HEADER_INIT;

public:
    Child();

    ~Child();

    caf::ChildField<TestObj*> m_testObj;
};
