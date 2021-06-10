#include "cafFieldIoCapability.h"

#include "cafAssert.h"
#include "cafFieldHandle.h"
#include "cafObjectHandle.h"
#include "cafObjectIoCapability.h"

#include <iostream>

using namespace caffa;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
FieldIoCapability::FieldIoCapability( FieldHandle* owner, bool giveOwnership )
    : m_isIOReadable( true )
    , m_isIOWritable( true )
    , m_isCopyable( true )
{
    m_owner = owner;
    owner->addCapability( this, giveOwnership );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FieldIoCapability::disableIO()
{
    setIOReadable( false );
    setIOWritable( false );
    setCopyable( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool FieldIoCapability::assertValid() const
{
    if ( m_owner->keyword().empty() )
    {
        std::cout << "Field: Detected use of non-initialized field. Did you forget to do initField() on "
                     "this field ?\n";
        return false;
    }

    if ( !ObjectIoCapability::isValidElementName( m_owner->keyword() ) )
    {
        std::cout << "Field: The supplied keyword: \"" << m_owner->keyword()
                  << "\" is an invalid XML element name, and will break your file format!\n";
        return false;
    }

    return true;
}
