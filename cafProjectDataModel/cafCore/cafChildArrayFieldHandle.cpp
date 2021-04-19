#include "cafChildArrayField.h"

#include "cafFieldHandle.h"
#include "cafObjectHandle.h"

#include <limits>

namespace caffa
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool ChildArrayFieldHandle::hasSameFieldCountForAllObjects()
{
    std::vector<ObjectHandle*> listObjects;
    this->childObjects( &listObjects );

    if ( listObjects.size() == 0 )
    {
        return true;
    }

    size_t fieldCount = std::numeric_limits<size_t>::infinity();
    for ( auto object : listObjects )
    {
        std::vector<FieldHandle*> fields = object->fields();

        if ( fieldCount == std::numeric_limits<size_t>::infinity() )
        {
            fieldCount = fields.size();
        }
        else if ( fieldCount != fields.size() )
        {
            return false;
        }
    }

    return true;
}

} // End of namespace caffa
