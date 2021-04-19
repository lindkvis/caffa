#include "cafFieldHandle.h"

#include "cafFieldCapability.h"
#include "cafObjectHandle.h"

namespace caffa
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
FieldHandle::FieldHandle()
    : changed( this )
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
    std::vector<ObjectHandle*> children;
    this->childObjects( &children );
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
    if ( m_keyword == keyword ) return true;

    return matchesKeywordAlias( keyword );
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
/// Get the class in the class hierarchy the field actually belongs to.
/// This can be different to ownerObject's class, which may be a sub-class.
//--------------------------------------------------------------------------------------------------
std::string FieldHandle::ownerClass() const
{
    return m_ownerClass;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<ObjectHandle> FieldHandle::removeChildObject( ObjectHandle* )
{
    return std::unique_ptr<ObjectHandle>();
}

//--------------------------------------------------------------------------------------------------
/// Set the class in the class hierarchy the field actually belongs to.
/// This can be different to ownerObject's class, which may be a sub-class.
//--------------------------------------------------------------------------------------------------
void FieldHandle::setOwnerClass( const std::string& ownerClass )
{
    m_ownerClass = ownerClass;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool FieldHandle::hasPtrReferencedObjects()
{
    std::vector<ObjectHandle*> ptrReffedObjs;
    this->ptrReferencedObjects( &ptrReffedObjs );
    return ( ptrReffedObjs.size() > 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FieldHandle::registerKeywordAlias( const std::string& alias )
{
    m_keywordAliases.push_back( alias );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool FieldHandle::matchesKeywordAlias( const std::string& keyword ) const
{
    for ( const std::string& alias : m_keywordAliases )
    {
        if ( alias == keyword ) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> FieldHandle::keywordAliases() const
{
    return m_keywordAliases;
}

} // End of namespace caffa
