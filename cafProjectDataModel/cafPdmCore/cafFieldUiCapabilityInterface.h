#pragma once

#include "cafVariant.h"

namespace caf
{
class FieldUiCapabilityInterface
{
public:
    FieldUiCapabilityInterface() {}
    virtual ~FieldUiCapabilityInterface() {}

    virtual Variant toUiBasedVariant() const { return Variant(); }
    virtual void    notifyFieldChanged( const Variant& oldUiBasedAny, const Variant& newUiBasedAny ){};
};

} // End of namespace caf
