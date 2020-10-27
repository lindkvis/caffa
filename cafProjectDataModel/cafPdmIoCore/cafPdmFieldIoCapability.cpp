#include "cafPdmFieldIoCapability.h"

#include "cafAssert.h"
#include "cafPdmFieldHandle.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmObjectIoCapability.h"

#include <iostream>

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmFieldIoCapability::PdmFieldIoCapability( PdmFieldHandle* owner, bool giveOwnership )
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
void PdmFieldIoCapability::disableIO()
{
    setIOReadable( false );
    setIOWritable( false );
    setCopyable( false );
}

//--------------------------------------------------------------------------------------------------
/// Returns the classKeyword of the child class type, if this field is supposed to contain pointers
/// to PdmObjectHandle derived objects.
/// Returns typeid(DataType).name() if the field is not containing some PdmObjectHandle type.
/// Warning: typeid(DataType).name() is compiler implementation specific and thus you should not
/// Compare this with a predefined literal, like "double" or "float". Instead compare with typeid(double).name().
//--------------------------------------------------------------------------------------------------
QString PdmFieldIoCapability::dataTypeName() const
{
    return m_dataTypeName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmFieldIoCapability::assertValid() const
{
    if ( m_owner->keyword().isEmpty() )
    {
        std::cout << "PdmField: Detected use of non-initialized field. Did you forget to do CAF_PDM_InitField() on "
                     "this field ?\n";
        return false;
    }

    if ( !PdmObjectIoCapability::isValidElementName( m_owner->keyword() ) )
    {
        std::cout << "PdmField: The supplied keyword: \"" << m_owner->keyword().toStdString()
                  << "\" is an invalid XML element name, and will break your file format!\n";
        return false;
    }

    return true;
}
