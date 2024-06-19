#pragma once

#include <list>
#include <memory>
#include <string>
#include <utility>

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

    std::string         keyword() const { return m_keyword; }
    ObjectHandle*       ownerObject();
    const ObjectHandle* ownerObject() const;

    virtual std::string dataType() const = 0;

    // Capabilities
    void addCapability( std::unique_ptr<FieldCapability> capability );

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
    virtual bool isReadable() const = 0;

    /**
     * Can the field be written to. A non-scriptable field cannot be written to within the client.
     */
    virtual bool isWritable() const = 0;

    bool isDeprecated() const;
    void markDeprecated();

    void               setDocumentation( const std::string& documentation );
    const std::string& documentation() const;

protected:
    bool isInitialized() const { return m_ownerObject != nullptr; }

    std::list<FieldCapability*> capabilities();

private:
    FieldHandle( const FieldHandle& )            = delete;
    FieldHandle& operator=( const FieldHandle& ) = delete;

    friend class ObjectHandle; // Give access to m_ownerObject and set Keyword
    void          setKeyword( const std::string& keyword );
    ObjectHandle* m_ownerObject;

    std::string m_keyword;

    std::list<std::unique_ptr<FieldCapability>> m_capabilities;

    bool m_isDeprecated;

    std::string m_documentation;
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
