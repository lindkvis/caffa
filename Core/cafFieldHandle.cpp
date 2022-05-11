#include "cafFieldHandle.h"

#include "cafFieldCapability.h"
#include "cafObjectHandle.h"

namespace caffa
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
FieldHandle::FieldHandle()
    : m_isReadable( true )
    , m_isWritable( true )
{
    m_ownerObject = nullptr;
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
bool FieldHandle::hasChildObjects()
{
    std::vector<ObjectHandle*> children = this->childObjects();
    return ( children.size() > 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
FieldHandle::~FieldHandle()
{
    for ( size_t i = 0; i < m_capabilities.size(); ++i )
    {
        if ( m_capabilities[i].second ) delete m_capabilities[i].first;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool FieldHandle::matchesKeyword( const std::string& keyword ) const
{
    return m_keyword == keyword;
}

//--------------------------------------------------------------------------------------------------
/// The class of the ownerObject() can be different to ownerClass().
/// This is because the ownerClass() may be a super-class to the instantiated owner object.
//--------------------------------------------------------------------------------------------------
caffa::ObjectHandle* FieldHandle::ownerObject()
{
    return m_ownerObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<ObjectHandle> FieldHandle::removeChildObject( ObjectHandle* )
{
    return std::unique_ptr<ObjectHandle>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<FieldCapability*> FieldHandle::capabilities()
{
    std::vector<FieldCapability*> allCapabilities;
    for ( auto capability : m_capabilities )
    {
        allCapabilities.push_back( capability.first );
    }
    return allCapabilities;
}

} // End of namespace caffa
