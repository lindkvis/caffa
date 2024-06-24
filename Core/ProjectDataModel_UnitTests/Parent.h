#pragma once

#include "Child.h"

#include "cafField.h"
#include "cafObject.h"

class Parent : public caffa::Object
{
    CAFFA_HEADER_INIT( Parent, Object )

public:
    Parent();
    ~Parent();

    void doSome();

    caffa::Field<std::vector<std::shared_ptr<Child>>> m_simpleObjectsField;
    caffa::Field<std::shared_ptr<Child>>              m_simpleObjectF;
};
