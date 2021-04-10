#pragma once

#include "cafField.h"
#include "cafObject.h"
#include "cafPointer.h"

class TestObj : public caf::Object
{
public:
    TestObj();
    ~TestObj();

    caf::Field<double> m_position;
};
