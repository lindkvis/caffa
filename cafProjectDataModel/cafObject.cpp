#include "cafObject.h"

using namespace caf;

CAF_PDM_ABSTRACT_SOURCE_INIT( Object, "ObjectBase" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::Object::Object()
    : ObjectHandle()
    , ObjectIoCapability( this, false )
    , ObjectUiCapability( this, false )
{
    CAF_PDM_InitObject( "Base PDM Object", "", "", "The Abstract Base Class for the Project Data Model" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Object::firstAncestorOrThisFromClassKeyword( const QString& classKeyword, Object*& ancestor ) const
{
    ancestor = nullptr;

    // Check if this matches the type
    if ( this->inheritsClassWithKeyword( classKeyword ) )
    {
        ancestor = const_cast<Object*>( this );
        return;
    }

    // Search parents for first type match

    Object*      parent      = nullptr;
    FieldHandle* parentField = this->parentField();
    if ( parentField ) parent = dynamic_cast<Object*>( parentField->ownerObject() );

    while ( parent != nullptr )
    {
        if ( parent->inheritsClassWithKeyword( classKeyword ) )
        {
            ancestor = parent;
            return;
        }
        // Get next level parent

        FieldHandle* nextParentField = parent->parentField();
        if ( nextParentField )
        {
            parent = dynamic_cast<Object*>( nextParentField->ownerObject() );
        }
        else
        {
            parent = nullptr;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Object::descendantsIncludingThisFromClassKeyword( const QString&           classKeyword,
                                                          std::vector<Object*>& descendants ) const
{
    if ( this->inheritsClassWithKeyword( classKeyword ) )
    {
        descendants.push_back( const_cast<Object*>( this ) );
    }

    std::vector<FieldHandle*> fields;
    this->fields( fields );
    for ( auto f : fields )
    {
        std::vector<ObjectHandle*> childObjects;
        f->childObjects( &childObjects );
        for ( auto childObject : childObjects )
        {
            Object* pdmObjectChild = dynamic_cast<Object*>( childObject );
            if ( pdmObjectChild )
            {
                pdmObjectChild->descendantsIncludingThisFromClassKeyword( classKeyword, descendants );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Object::childrenFromClassKeyword( const QString& classKeyword, std::vector<Object*>& children ) const
{
    std::vector<FieldHandle*> fields;
    this->fields( fields );
    for ( auto f : fields )
    {
        std::vector<ObjectHandle*> childObjects;
        f->childObjects( &childObjects );
        for ( auto childObject : childObjects )
        {
            Object* pdmObjectChild = dynamic_cast<Object*>( childObject );
            if ( pdmObjectChild && pdmObjectChild->matchesClassKeyword( classKeyword ) )
            {
                children.push_back( pdmObjectChild );
            }
        }
    }
}
