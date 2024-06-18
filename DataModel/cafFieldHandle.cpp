#include "cafFieldHandle.h"

#include "cafFieldCapability.h"
#include "cafObjectHandle.h"
#include "cafVisitor.h"

namespace caffa
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
FieldHandle::FieldHandle()
    : m_ownerObject( nullptr )
    , m_isDeprecated( false )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
FieldHandle::~FieldHandle()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FieldHandle::setKeyword( const std::string& keyword )
{
    m_keyword = keyword;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caffa::ObjectHandle* FieldHandle::ownerObject()
{
    return m_ownerObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const caffa::ObjectHandle* FieldHandle::ownerObject() const
{
    return m_ownerObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::list<FieldCapability*> FieldHandle::capabilities()
{
    std::list<FieldCapability*> allCapabilities;
    for ( auto& capability : m_capabilities )
    {
        allCapabilities.push_back( capability.get() );
    }
    return allCapabilities;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FieldHandle::addCapability( std::unique_ptr<FieldCapability> capability )
{
    capability->setOwner( this );

    // Push to the front, so that any new capability takes precedence over old ones.
    m_capabilities.push_front( std::move( capability ) );
}

bool FieldHandle::isDeprecated() const
{
    return m_isDeprecated;
}

void FieldHandle::markDeprecated()
{
    m_isDeprecated = true;
}

void FieldHandle::setDocumentation( const std::string& documentation )
{
    m_documentation = documentation;
}

const std::string& FieldHandle::documentation() const
{
    return m_documentation;
}

} // End of namespace caffa
