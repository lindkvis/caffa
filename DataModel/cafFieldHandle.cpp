#include "cafFieldHandle.h"

#include "cafFieldCapability.h"
#include "cafObjectHandle.h"

#include <typeinfo>

namespace caffa
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
FieldHandle::FieldHandle()
    : m_ownerObject( nullptr )
    , m_isDeprecated( false )
    , m_volatile( false )
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
ObjectHandle* FieldHandle::ownerObject()
{
    return m_ownerObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const ObjectHandle* FieldHandle::ownerObject() const
{
    return m_ownerObject;
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

bool FieldHandle::isVolatile() const
{
    return m_volatile;
}
void FieldHandle::markVolatile()
{
    m_volatile = true;
}

} // End of namespace caffa
