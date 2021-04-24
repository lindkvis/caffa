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
    bool isCopyable() const { return m_isCopyable; }

    void disableIO();
    void setIOWritable( bool isWritable ) { m_isIOWritable = isWritable; }
    void setIOReadable( bool isReadable ) { m_isIOReadable = isReadable; }
    void setCopyable( bool isCopyable ) { m_isCopyable = isCopyable; }

    std::string dataTypeName() const;

    virtual bool resolveReferences() = 0;

    virtual std::string referenceString() const { return std::string(); }

    virtual void writeToField( const nlohmann::json& value, ObjectFactory* objectFactory )               = 0;
    virtual void readFromField( nlohmann::json& value, bool writeServerAddress, bool writeValues ) const = 0;

protected:
    bool assertValid() const;

    std::string m_dataTypeName; ///< Must be set in constructor of derived XmlFieldHandle

protected:
    bool m_isIOReadable;
    bool m_isIOWritable;
    bool m_isCopyable;

    FieldHandle* m_owner;
};
} // End of namespace caffa
