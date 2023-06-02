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
std::vector<FieldCapability*> FieldHandle::capabilities()
{
    std::vector<FieldCapability*> allCapabilities;
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
    m_capabilities.push_back( std::move( capability ) );
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
