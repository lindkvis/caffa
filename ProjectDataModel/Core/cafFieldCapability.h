#pragma once

namespace caffa
{
class Variant;

class FieldCapability
{
public:
    FieldCapability() {}
    virtual ~FieldCapability() {}

    virtual void
        notifyFieldChanged( const FieldCapability* changedByCapability, const Variant& oldValue, const Variant& newValue )
    {
    }
};

} // End of namespace caffa
