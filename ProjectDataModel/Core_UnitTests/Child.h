#pragma once

#include "TestObj.h"
#include "cafChildField.h"
#include "cafObjectHandle.h"
#include "cafObservingPointer.h"

class Child : public caffa::ObjectHandle
{
public:
    Child();
    ~Child();

    std::string classKeywordDynamic() const override { return "Child"; }

    caffa::ChildField<TestObj*> m_testObj;
};
