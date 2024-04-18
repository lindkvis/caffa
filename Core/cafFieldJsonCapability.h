#pragma once

#include "cafFieldCapability.h"

#include <nlohmann/json.hpp>

#include <chrono>
#include <memory>
#include <string>
#include <vector>

namespace caffa
{
class FieldHandle;
class ObjectFactory;
class ObjectHandle;
class Serializer;
//==================================================================================================
//
//
//
//==================================================================================================
class FieldJsonCapability : public FieldCapability
{
public:
    FieldJsonCapability();

    virtual void readFromJson( const nlohmann::ordered_json& value, const Serializer& serializer ) = 0;
    virtual void writeToJson( nlohmann::ordered_json& value, const Serializer& serializer ) const  = 0;

    virtual nlohmann::ordered_json jsonType() const = 0;

    bool isSerializable() const;
    void setSerializable( bool serializable );

protected:
    bool assertValid() const;

    bool m_serializable;
};
} // End of namespace caffa
