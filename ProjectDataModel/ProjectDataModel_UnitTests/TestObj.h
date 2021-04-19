#pragma once

#include "cafField.h"
#include "cafObject.h"
#include "cafPointer.h"

class TestObj : public caffa::Object
{
    CAF_HEADER_INIT;

public:
    TestObj();

    ~TestObj();

    caffa::Field<double> m_position;
};
