#pragma once

#include "cafFieldIoCapability.h"

#include <nlohmann/json.hpp>

#include <string>

namespace caffa
{
class Serializer;
//==================================================================================================
//
//
//
//==================================================================================================
class FieldJsonCapability : public FieldIoCapability
{
public:
    FieldJsonCapability();

    virtual nlohmann::json jsonType() const = 0;

protected:
    bool assertValid() const;
};
} // End of namespace caffa
