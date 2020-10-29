#pragma once

#include "cafFieldHandle.h"

namespace caf
{
//==================================================================================================
///
///
///
//==================================================================================================
class PtrArrayFieldHandle : public FieldHandle
{
public:
    PtrArrayFieldHandle() {}
    ~PtrArrayFieldHandle() override {}

    virtual size_t size() const                                     = 0;
    virtual bool   empty() const                                    = 0;
    virtual void   clear()                                          = 0;
    virtual void   insertAt( int indexAfter, ObjectHandle* obj ) = 0;
    virtual void   erase( size_t index )                            = 0;

    virtual ObjectHandle* at( size_t index ) = 0;
};

} // namespace caf