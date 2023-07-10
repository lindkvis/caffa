#include "cafFieldJsonCapability.h"

#include "cafAssert.h"
#include "cafFieldHandle.h"
#include "cafObjectHandle.h"

#include <iostream>

using namespace caffa;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
FieldJsonCapability::FieldJsonCapability()
    : FieldCapability()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool FieldJsonCapability::assertValid() const
{
    if ( owner()->keyword().empty() )
    {
        std::cerr << "Field: Detected use of non-initialized field. Did you forget to do initField() on "
                     "this field ?\n";
        CAFFA_ASSERT( false );
        return false;
    }

    if ( !ObjectHandle::isValidKeyword( owner()->keyword() ) )
    {
        std::cerr << "Field: The supplied keyword: \"" << owner()->keyword()
                  << "\" is an invalid element name, and will break your file format!\n";
        CAFFA_ASSERT( false );
        return false;
    }

    return true;
}
