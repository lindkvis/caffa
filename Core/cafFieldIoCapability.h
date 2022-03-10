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
class FieldIoCapability : public FieldCapability
{
public:
    FieldIoCapability( FieldHandle* owner, bool giveOwnership );

    ~FieldIoCapability() override = default;

    FieldHandle*       fieldHandle() { return m_owner; }
    const FieldHandle* fieldHandle() const { return m_owner; }

    bool isIOReadable() const { return m_isIOReadable; }
    bool isIOWritable() const { return m_isIOWritable; }

    void disableIO();
    void setIOWritable( bool isWritable ) { m_isIOWritable = isWritable; }
    void setIOReadable( bool isReadable ) { m_isIOReadable = isReadable; }

    virtual void readFromJson( const nlohmann::json& value, const Serializer& serializer ) = 0;
    virtual void writeToJson( nlohmann::json& value, const Serializer& serializer ) const  = 0;

protected:
    bool assertValid() const;

protected:
    bool m_isIOReadable;
    bool m_isIOWritable;

    FieldHandle* m_owner;
};
} // End of namespace caffa
