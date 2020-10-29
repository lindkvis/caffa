#pragma once

#include "cafPdmDataValueField.h"
#include "cafObjectHandle.h"
#include "cafPdmPointer.h"

class TestObj : public caf::ObjectHandle
{
public:
    TestObj();
    ~TestObj();

    caf::PdmDataValueField<double> m_position;
};
