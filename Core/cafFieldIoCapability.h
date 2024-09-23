#pragma once

#include "cafFieldCapability.h"

#include <nlohmann/json.hpp>

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

    virtual void readFromJson( const nlohmann::json& value, const JsonSerializer& serializer ) = 0;
    virtual void writeToJson( nlohmann::json& value, const JsonSerializer& serializer ) const  = 0;

    [[nodiscard]] virtual nlohmann::json jsonType() const = 0;

protected:
    void assertValid() const;
};
} // End of namespace caffa
