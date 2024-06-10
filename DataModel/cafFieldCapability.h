#pragma once

namespace caffa
{
class FieldHandle;

class FieldCapability
{
public:
    FieldCapability()          = default;
    virtual ~FieldCapability() = default;

    virtual const FieldHandle* owner() const                  = 0;
    virtual void               setOwner( FieldHandle* field ) = 0;
};

} // End of namespace caffa
