#pragma once

#include "cafBase.h"
#include "cafSignal.h"

#include <any>
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
    bool                                  hasChildObjects();
    virtual std::vector<ObjectHandle*>    childObjects() const { return {}; }
    virtual std::unique_ptr<ObjectHandle> removeChildObject( ObjectHandle* );

    virtual std::string dataType() const = 0;

    // Capabilities
    void addCapability( FieldCapability* capability, bool takeOwnership )
    {
        m_capabilities.push_back( std::make_pair( capability, takeOwnership ) );
    }

    template <typename CapabilityType>
    CapabilityType* capability();
    template <typename CapabilityType>
    const CapabilityType* capability() const;

protected:
    bool isInitializedByInitFieldMacro() const { return m_ownerObject != nullptr; }

    std::vector<FieldCapability*> capabilities();

private:
    CAFFA_DISABLE_COPY_AND_ASSIGN( FieldHandle );

    friend class ObjectHandle; // Give access to m_ownerObject and set Keyword
    void          setKeyword( const std::string& keyword );
    ObjectHandle* m_ownerObject;

    std::string m_keyword;

    std::vector<std::pair<FieldCapability*, bool>> m_capabilities;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename CapabilityType>
CapabilityType* FieldHandle::capability()
{
    for ( size_t i = 0; i < m_capabilities.size(); ++i )
    {
        CapabilityType* capability = dynamic_cast<CapabilityType*>( m_capabilities[i].first );
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
    for ( size_t i = 0; i < m_capabilities.size(); ++i )
    {
        const CapabilityType* capability = dynamic_cast<CapabilityType*>( m_capabilities[i].first );
        if ( capability ) return capability;
    }
    return nullptr;
}

} // End of namespace caffa
