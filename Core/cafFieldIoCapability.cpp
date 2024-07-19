#include "cafFieldIoCapability.h"

#include "cafAssert.h"
#include "cafFieldHandle.h"
#include "cafLogger.h"
#include "cafObjectHandle.h"

#include <iostream>

using namespace caffa;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
FieldIoCapability::FieldIoCapability()
    : FieldCapability()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FieldIoCapability::assertValid() const
{
    if ( owner()->keyword().empty() )
    {
        CAFFA_CRITICAL( "Field: Detected use of non-initialized field. Did you forget to do initField() on "
                        "this field ?" );
    }

    if ( !ObjectHandle::isValidKeyword( owner()->keyword() ) )
    {
        CAFFA_CRITICAL( "Field: The supplied keyword: \""
                        << owner()->keyword() << "\" is an invalid element name, and will break your file format!" );
    }
}
