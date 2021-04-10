#include "cafObject.h"

using namespace caf;

CAF_ABSTRACT_SOURCE_INIT( Object, "ObjectBase" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::Object::Object()
    : ObjectHandle()
    , ObjectIoCapability( this, false )
    , ObjectUiCapability( this, false )
{
    initObject();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Object& Object::initObject()
{
    this->isInheritedFromSerializable();
    this->registerClassKeyword( classKeyword() );
    return *this;
}

Object& Object::withUi( const std::string& uiName,
                        const std::string& iconResourceName,
                        const std::string& toolTip,
                        const std::string& whatsThis )
{
    std::string validUiName = uiName.empty() ? classKeyword() : uiName;
    this->isInheritedFromUiObject();
    caf::UiItemInfo objDescr( validUiName, iconResourceName, toolTip, whatsThis );
    this->setUiItemInfo( objDescr );
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Object::firstAncestorOrThisFromClassKeyword( const std::string& classKeyword, Object*& ancestor ) const
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
void Object::descendantsIncludingThisFromClassKeyword( const std::string&    classKeyword,
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
            Object* objectChild = dynamic_cast<Object*>( childObject );
            if ( objectChild )
            {
                objectChild->descendantsIncludingThisFromClassKeyword( classKeyword, descendants );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Object::childrenFromClassKeyword( const std::string& classKeyword, std::vector<Object*>& children ) const
{
    std::vector<FieldHandle*> fields;
    this->fields( fields );
    for ( auto f : fields )
    {
        std::vector<ObjectHandle*> childObjects;
        f->childObjects( &childObjects );
        for ( auto childObject : childObjects )
        {
            Object* objectChild = dynamic_cast<Object*>( childObject );
            if ( objectChild && objectChild->matchesClassKeyword( classKeyword ) )
            {
                children.push_back( objectChild );
            }
        }
    }
}
