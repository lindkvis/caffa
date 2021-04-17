

#include "cafObjectGroup.h"
#include "cafFieldIoCapabilitySpecializations.h"

namespace caf
{
CAF_SOURCE_INIT( ObjectGroup, "ObjectGroup" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectGroup::ObjectGroup()
{
    assignUiInfo( "Object Group", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectGroup::~ObjectGroup()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectGroup::deleteObjects()
{
    size_t it;
    for ( it = 0; it != objects.size(); ++it )
    {
        delete objects[it];
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectGroup::addObject( ObjectHandle* obj )
{
    objects.push_back( obj );
}

CAF_SOURCE_INIT( ObjectCollection, "ObjectCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectCollection::ObjectCollection()
{
    initField( objects, "Objects" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectCollection::~ObjectCollection()
{
}

} // End of namespace caf
