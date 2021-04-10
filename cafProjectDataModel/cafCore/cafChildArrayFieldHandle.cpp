#include "cafChildArrayField.h"

#include "cafFieldHandle.h"
#include "cafObjectHandle.h"

namespace caf
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

    size_t fieldCount = 0;
    for ( size_t i = 0; i < listObjects.size(); i++ )
    {
        std::vector<FieldHandle*> fields;
        listObjects[i]->fields( fields );

        if ( i == 0 )
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

} // End of namespace caf
