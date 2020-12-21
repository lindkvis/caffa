#pragma once

#include "cafDataValueField.h"
#include "cafObjectHandle.h"
#include "cafPdmPointer.h"

class TestObj : public caf::ObjectHandle
{
public:
    TestObj();
    ~TestObj();

    caf::DataValueField<double> m_position;
};
