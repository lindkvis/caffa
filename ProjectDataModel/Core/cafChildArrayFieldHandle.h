#pragma once

#include "cafFieldHandle.h"

#include <memory>

namespace caffa
{
class ChildArrayFieldAccessor;
class ObjectHandle;
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

    virtual size_t                                     size() const = 0;
    bool                                               empty() const { return this->size() == 0u; }
    virtual std::vector<std::unique_ptr<ObjectHandle>> clear()               = 0;
    virtual void                                       erase( size_t index ) = 0;

    virtual ObjectHandle* at( size_t index ) = 0;

    virtual void insertAt( size_t index, std::unique_ptr<ObjectHandle> obj )               = 0;
    virtual void push_back_obj( std::unique_ptr<ObjectHandle> obj )                        = 0;
    virtual void setFieldDataAccessor( std::unique_ptr<ChildArrayFieldAccessor> accessor ) = 0;

    bool hasSameFieldCountForAllObjects();
};

} // namespace caffa
