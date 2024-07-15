#pragma once

#include "cafField.h"
#include "cafObject.h"

class TestObj final : public caffa::Object
{
public:
    TestObj();
    ~TestObj() override;

    caffa::Field<double> m_position;
};
