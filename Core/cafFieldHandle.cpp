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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FieldHandle::accept( Inspector* visitor ) const
{
    visitor->visitField( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FieldHandle::accept( Editor* visitor )
{
    visitor->visitField( this );
}

} // End of namespace caffa
