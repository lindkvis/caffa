#pragma once

#include "cafSignal.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace caffa
{
class ObjectHandle;

//==================================================================================================
/// Base class for all fields, making it possible to handle them generically
//==================================================================================================
class FieldCapability;

class FieldHandle : public SignalEmitter
{
public:
    FieldHandle();
    virtual ~FieldHandle();

    std::string   keyword() const { return m_keyword; }
    bool          matchesKeyword( const std::string& keyword ) const;
    ObjectHandle* ownerObject();

    // Child objects
    bool                                     hasChildObjects();
    virtual std::vector<ObjectHandle*>       childObjects() { return {}; }
    virtual std::vector<const ObjectHandle*> childObjects() const { return {}; }
    virtual std::unique_ptr<ObjectHandle>    removeChildObject( ObjectHandle* );

    virtual std::string dataType() const = 0;

    // Capabilities
    void addCapability( std::unique_ptr<FieldCapability> capability );

    template <typename CapabilityType>
    CapabilityType* capability();
    template <typename CapabilityType>
    const CapabilityType* capability() const;

    virtual void resetToDefault() {}

protected:
    bool isInitialized() const { return m_ownerObject != nullptr; }

    std::vector<FieldCapability*> capabilities();

private:
    FieldHandle( const FieldHandle& )            = delete;
    FieldHandle& operator=( const FieldHandle& ) = delete;

    friend class ObjectHandle; // Give access to m_ownerObject and set Keyword
    void          setKeyword( const std::string& keyword );
    ObjectHandle* m_ownerObject;

    std::string m_keyword;

    std::vector<std::unique_ptr<FieldCapability>> m_capabilities;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename CapabilityType>
CapabilityType* FieldHandle::capability()
{
    for ( auto& capabilityPtr : m_capabilities )
    {
        CapabilityType* capability = dynamic_cast<CapabilityType*>( capabilityPtr.get() );
        if ( capability ) return capability;
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename CapabilityType>
const CapabilityType* FieldHandle::capability() const
{
    for ( const auto& capabilityPtr : m_capabilities )
    {
        const CapabilityType* capability = dynamic_cast<const CapabilityType*>( capabilityPtr.get() );
        if ( capability ) return capability;
    }
    return nullptr;
}

} // End of namespace caffa
