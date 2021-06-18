#pragma once

#include "cafFieldHandle.h"

namespace caffa
{
//==================================================================================================
///
///
///
//==================================================================================================
class ChildArrayFieldHandle : public FieldHandle
{
public:
    ChildArrayFieldHandle() {}
    ~ChildArrayFieldHandle() override {}

    virtual size_t size() const          = 0;
    virtual bool   empty() const         = 0;
    virtual void   clear()               = 0;
    virtual void   erase( size_t index ) = 0;

    virtual ObjectHandle* at( size_t index ) = 0;

    virtual Pointer<ObjectHandle> insertAt( size_t index, std::unique_ptr<ObjectHandle> obj ) = 0;

    bool hasSameFieldCountForAllObjects();
};

} // namespace caffa
