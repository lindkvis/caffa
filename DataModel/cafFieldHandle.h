#pragma once

#include "cafAssert.h"

#include <chrono>
#include <memory>
#include <string>
#include <vector>

namespace caffa
{
class FieldCapability;
class ObjectHandle;

class Editor;
class Inspector;

//==================================================================================================
/// Base class for all fields, making it possible to handle them generically
//==================================================================================================
class FieldHandle
{
public:
    FieldHandle();
    virtual ~FieldHandle();

    [[nodiscard]] std::string         keyword() const { return m_keyword; }
    ObjectHandle*                     ownerObject();
    [[nodiscard]] const ObjectHandle* ownerObject() const;

    [[nodiscard]] virtual std::string dataType() const = 0;

    // Capabilities
    template <typename CapabilityType>
    void addCapability( std::unique_ptr<CapabilityType> capability );

    template <typename CapabilityType>
    CapabilityType* capability();
    template <typename CapabilityType>
    const CapabilityType* capability() const;

    /**
     * Accept the visit by an inspecting visitor
     * @param visitor
     */
    virtual void accept( Inspector* visitor ) const = 0;

    /**
     * Accept the visit by an editing visitor
     * @param visitor
     */
    virtual void accept( Editor* visitor ) = 0;

    /**
     * Can the field be read. A non-scriptable field cannot be read within the client.
     */
    [[nodiscard]] virtual bool isReadable() const = 0;

    /**
     * Can the field be written to. A non-scriptable field cannot be written to within the client.
     */
    [[nodiscard]] virtual bool isWritable() const = 0;

    [[nodiscard]] bool isDeprecated() const;
    void               markDeprecated();

    void                             setDocumentation( const std::string& documentation );
    [[nodiscard]] const std::string& documentation() const;

    FieldHandle( const FieldHandle& ) = delete;

    const std::chrono::system_clock::time_point& lastModified() const;

    /**
     * Is the field volatile (can be changed external means)
     * @return true if volatile
     */
    bool isVolatile() const;

    /**
     * Mark the field as volatile (can be changed external means)
     */
    void markVolatile();

protected:
    [[nodiscard]] bool isInitialized() const { return m_ownerObject != nullptr; }
    void               updateLastModified();

private:
    friend class ObjectHandle; // Give access to m_ownerObject and set Keyword
    void          setKeyword( const std::string& keyword );
    ObjectHandle* m_ownerObject;

    std::string m_keyword;

    std::vector<std::unique_ptr<FieldCapability>> m_capabilities;

    bool m_isDeprecated;
    bool m_volatile;

    std::string m_documentation;

    std::chrono::system_clock::time_point m_lastModified;
};

template <typename CapabilityType>
void FieldHandle::addCapability( std::unique_ptr<CapabilityType> capability )
{
    CAFFA_ASSERT( this->capability<CapabilityType>() == nullptr && "Cannot add more than one of the same capability" );
    capability->setOwner( this );
    m_capabilities.push_back( std::move( capability ) );
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename CapabilityType>
CapabilityType* FieldHandle::capability()
{
    for ( auto& capabilityPtr : m_capabilities )
    {
        if ( auto* cap = dynamic_cast<CapabilityType*>( capabilityPtr.get() ); cap ) return cap;
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
        if ( const auto* cap = dynamic_cast<const CapabilityType*>( capabilityPtr.get() ); cap ) return cap;
    }
    return nullptr;
}

} // End of namespace caffa
