#include "cafFieldHandle.h"

#include "cafFieldCapability.h"

namespace caf
{
#if 0
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool FieldHandle::assertValid() const
{
    if (m_keyword == "UNDEFINED")
    {
        std::cout << "Field: Detected use of non-initialized field. Did you forget to do CAF_InitField() on this field ?\n";
        return false;
    }

    if (!PdmXmlSerializable::isValidElementName(m_keyword))
    {
        std::cout << "Field: The supplied keyword: \"" << m_keyword.toStdString() << "\" is an invalid XML element name, and will break your file format!\n";
        return false;
    }

    return true;
}
#endif

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FieldHandle::setKeyword( const QString& keyword )
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
bool FieldHandle::matchesKeyword( const QString& keyword ) const
{
    if ( m_keyword == keyword ) return true;

    return matchesKeywordAlias( keyword );
}

//--------------------------------------------------------------------------------------------------
/// The class of the ownerObject() can be different to ownerClass().
/// This is because the ownerClass() may be a super-class to the instantiated owner object.
//--------------------------------------------------------------------------------------------------
caf::ObjectHandle* FieldHandle::ownerObject()
{
    return m_ownerObject;
}

//--------------------------------------------------------------------------------------------------
/// Get the class in the class hierarchy the field actually belongs to.
/// This can be different to ownerObject's class, which may be a sub-class.
//--------------------------------------------------------------------------------------------------
QString FieldHandle::ownerClass() const
{
    return m_ownerClass;
}

//--------------------------------------------------------------------------------------------------
/// Set the class in the class hierarchy the field actually belongs to.
/// This can be different to ownerObject's class, which may be a sub-class.
//--------------------------------------------------------------------------------------------------
void FieldHandle::setOwnerClass( const QString& ownerClass )
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
void FieldHandle::registerKeywordAlias( const QString& alias )
{
    m_keywordAliases.push_back( alias );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool FieldHandle::matchesKeywordAlias( const QString& keyword ) const
{
    for ( const QString& alias : m_keywordAliases )
    {
        if ( alias == keyword ) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> FieldHandle::keywordAliases() const
{
    return m_keywordAliases;
}

// These two functions can be used when PdmCore is used standalone without PdmUi/PdmXml
/*
FieldUiCapability* FieldHandle::capability<FieldUiCapability>()
{
    return NULL;
}

PdmXmlFieldHandle* FieldHandle::capability<ObjectXmlCapability>()
{
    return NULL;
}
*/

} // End of namespace caf
