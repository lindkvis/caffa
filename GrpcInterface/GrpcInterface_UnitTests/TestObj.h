#pragma once

#include "cafField.h"
#include "cafObject.h"
#include "cafObservingPointer.h"

class TestObj : public caffa::Object
{
public:
    TestObj();
    ~TestObj();

    caffa::Field<double> m_position;
};
