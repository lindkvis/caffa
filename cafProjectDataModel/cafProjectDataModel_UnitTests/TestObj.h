#pragma once

#include "cafField.h"
#include "cafObject.h"
#include "cafPdmPointer.h"

class TestObj : public caf::Object
{
    CAF_PDM_HEADER_INIT;

public:
    TestObj();

    ~TestObj();

    caf::Field<double> m_position;
};
