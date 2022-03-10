#pragma once

#include "cafField.h"
#include "cafObject.h"

class TestObj : public caffa::Object
{
    CAFFA_HEADER_INIT;

public:
    TestObj();

    ~TestObj();

    caffa::Field<double> m_position;
};
