#pragma once

#include "cafField.h"
#include "cafObjectHandle.h"
#include "cafObservingPointer.h"

class TestObj : public caffa::ObjectHandle
{
public:
    TestObj();
    ~TestObj();

    caffa::Field<double> m_position;
};
