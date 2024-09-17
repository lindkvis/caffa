#pragma once

#include "cafFieldCapability.h"

#include "cafJsonDefinitions.h"

#include <string>

namespace caffa
{
class FieldHandle;
class JsonSerializer;
//==================================================================================================
//
//
//
//==================================================================================================
class FieldIoCapability : public FieldCapability
{
public:
    FieldIoCapability();

    virtual void readFromJson( const json::value& value, const JsonSerializer& serializer ) = 0;
    virtual void writeToJson( json::value& value, const JsonSerializer& serializer ) const  = 0;

    [[nodiscard]] virtual json::object jsonType() const = 0;

protected:
    void assertValid() const;
};
} // End of namespace caffa
