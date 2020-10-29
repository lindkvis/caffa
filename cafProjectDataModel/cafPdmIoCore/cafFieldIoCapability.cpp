#include "cafFieldIoCapability.h"

#include "cafAssert.h"
#include "cafFieldHandle.h"
#include "cafObjectHandle.h"
#include "cafObjectIoCapability.h"

#include <iostream>

using namespace caf;

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
/// Returns the classKeyword of the child class type, if this field is supposed to contain pointers
/// to ObjectHandle derived objects.
/// Returns typeid(DataType).name() if the field is not containing some ObjectHandle type.
/// Warning: typeid(DataType).name() is compiler implementation specific and thus you should not
/// Compare this with a predefined literal, like "double" or "float". Instead compare with typeid(double).name().
//--------------------------------------------------------------------------------------------------
QString FieldIoCapability::dataTypeName() const
{
    return m_dataTypeName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool FieldIoCapability::assertValid() const
{
    if ( m_owner->keyword().isEmpty() )
    {
        std::cout << "Field: Detected use of non-initialized field. Did you forget to do CAF_PDM_InitField() on "
                     "this field ?\n";
        return false;
    }

    if ( !ObjectIoCapability::isValidElementName( m_owner->keyword() ) )
    {
        std::cout << "Field: The supplied keyword: \"" << m_owner->keyword().toStdString()
                  << "\" is an invalid XML element name, and will break your file format!\n";
        return false;
    }

    return true;
}
