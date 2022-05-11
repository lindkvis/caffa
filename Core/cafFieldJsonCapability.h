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
    FieldJsonCapability( FieldHandle* owner, bool giveOwnership );

    ~FieldJsonCapability() override = default;

    FieldHandle*       fieldHandle() { return m_owner; }
    const FieldHandle* fieldHandle() const { return m_owner; }

    virtual void readFromJson( const nlohmann::json& value, const Serializer& serializer ) = 0;
    virtual void writeToJson( nlohmann::json& value, const Serializer& serializer ) const  = 0;

protected:
    bool assertValid() const;

    FieldHandle* m_owner;
};
} // End of namespace caffa
