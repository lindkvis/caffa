#pragma once

#include "cafDataValueField.h"
#include "cafObjectHandle.h"
#include "cafPointer.h"

class TestObj : public caf::ObjectHandle
{
public:
    TestObj();
    ~TestObj();

    caf::DataValueField<double> m_position;
};
