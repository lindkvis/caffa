#pragma once

#include "cafDataValueField.h"
#include "cafObjectHandle.h"
#include "cafPointer.h"

class TestObj : public caffa::ObjectHandle
{
public:
    TestObj();
    ~TestObj();

    caffa::DataValueField<double> m_position;
};
