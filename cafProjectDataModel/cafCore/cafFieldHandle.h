#pragma once

#include "cafBase.h"
#include "cafSignal.h"
#include "cafVariant.h"

#include <any>
#include <string>
#include <utility>
#include <vector>

namespace caf
{
class ObjectHandle;
class FieldUiCapability;
class FieldIoCapability;

//==================================================================================================
/// Base class for all fields, making it possible to handle them generically
//==================================================================================================
class FieldCapability;

class FieldHandle : public SignalEmitter
{
    Signal<std::tuple<const FieldCapability*, Variant, Variant>> changed;

public:
    FieldHandle();
    virtual ~FieldHandle();

    std::string   keyword() const { return m_keyword; }
    bool          matchesKeyword( const std::string& keyword ) const;
    ObjectHandle* ownerObject();
    std::string   ownerClass() const;

    void                     registerKeywordAlias( const std::string& alias );
    bool                     matchesKeywordAlias( const std::string& keyword ) const;
    std::vector<std::string> keywordAliases() const;

    // Child objects
    bool         hasChildObjects();
    virtual void childObjects( std::vector<ObjectHandle*>* ) {}
    virtual void removeChildObject( ObjectHandle* ) {}
    void         setOwnerClass( const std::string& ownerClass );

    // Ptr referenced objects
    bool         hasPtrReferencedObjects();
    virtual void ptrReferencedObjects( std::vector<ObjectHandle*>* ) {}

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

private:
    CAF_DISABLE_COPY_AND_ASSIGN( FieldHandle );

    friend class ObjectHandle; // Give access to m_ownerObject and set Keyword
    void          setKeyword( const std::string& keyword );
    ObjectHandle* m_ownerObject;
    std::string   m_ownerClass;

    std::string              m_keyword;
    std::vector<std::string> m_keywordAliases;

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
    return NULL;
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
    return NULL;
}

} // End of namespace caf
