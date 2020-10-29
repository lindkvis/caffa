#pragma once

#include "cafPdmBase.h"
#include <QString>
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

class FieldHandle
{
public:
    FieldHandle() { m_ownerObject = nullptr; }
    virtual ~FieldHandle();

    QString          keyword() const { return m_keyword; }
    bool             matchesKeyword( const QString& keyword ) const;
    ObjectHandle* ownerObject();
    QString          ownerClass() const;

    void                 registerKeywordAlias( const QString& alias );
    bool                 matchesKeywordAlias( const QString& keyword ) const;
    std::vector<QString> keywordAliases() const;

    // Child objects
    bool         hasChildObjects();
    virtual void childObjects( std::vector<ObjectHandle*>* ) {}
    virtual void removeChildObject( ObjectHandle* ) {}
    void         setOwnerClass( const QString& ownerClass );

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
    PDM_DISABLE_COPY_AND_ASSIGN( FieldHandle );

    friend class ObjectHandle; // Give access to m_ownerObject and set Keyword
    void             setKeyword( const QString& keyword );
    ObjectHandle* m_ownerObject;
    QString          m_ownerClass;

    QString              m_keyword;
    std::vector<QString> m_keywordAliases;

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
