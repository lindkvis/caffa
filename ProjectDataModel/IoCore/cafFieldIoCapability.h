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
// class FieldXmlCapability;
class ObjectFactory;
class ObjectHandle;

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

    virtual bool resolveReferences() = 0;

    virtual std::string referenceString() const { return std::string(); }

    virtual void writeToField( const nlohmann::json& value, ObjectFactory* objectFactory, bool copyDataValues ) = 0;
    virtual void readFromField( nlohmann::json& value, bool copyServerAddress, bool copyDataValues ) const      = 0;

protected:
    bool assertValid() const;

protected:
    bool m_isIOReadable;
    bool m_isIOWritable;

    FieldHandle* m_owner;
};
} // End of namespace caffa
