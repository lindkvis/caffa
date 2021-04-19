#include "cafObjectUiCapability.h"

#include "cafAssert.h"
#include "cafField.h"
#include "cafFieldHandle.h"
#include "cafFieldUiCapability.h"
#include "cafObjectHandle.h"
#include "cafUiOrdering.h"
#include "cafUiTreeOrdering.h"

namespace caffa
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectUiCapability::ObjectUiCapability( ObjectHandle* owner, bool giveOwnership )
{
    m_owner = owner;
    m_owner->addCapability( this, giveOwnership );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectUiCapability* uiObj( const ObjectHandle* obj )
{
    if ( !obj ) return nullptr;
    ObjectUiCapability* uiObject = obj->capability<ObjectUiCapability>();
    CAFFA_ASSERT( uiObject );
    return uiObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectUiCapability::uiOrdering( UiOrdering& uiOrdering )
{
    // Restore state for includeRemainingFields, as this flag
    // can be changed in defineUiOrdering()
    bool includeRemaining_originalState = uiOrdering.isIncludingRemainingFields();

    this->defineUiOrdering( uiOrdering );
    if ( uiOrdering.isIncludingRemainingFields() )
    {
        // Add the remaining Fields To UiConfig
        std::vector<FieldHandle*> fields = m_owner->fields();
        for ( size_t i = 0; i < fields.size(); ++i )
        {
            FieldUiCapability* field = fields[i]->capability<FieldUiCapability>();
            if ( !uiOrdering.contains( field ) )
            {
                uiOrdering.add( field->fieldHandle() );
            }
        }
    }

    // Restore incoming value
    uiOrdering.skipRemainingFields( !includeRemaining_originalState );

    CAFFA_ASSERT( includeRemaining_originalState == uiOrdering.isIncludingRemainingFields() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectUiCapability::editorAttribute( const FieldHandle* field, UiEditorAttribute* attribute )
{
    this->defineEditorAttribute( field, attribute );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectUiCapability::objectEditorAttribute( UiEditorAttribute* attribute )
{
    this->defineObjectEditorAttribute( attribute );
}

//--------------------------------------------------------------------------------------------------
/// This method creates a tree-representation of the object hierarchy starting at this
/// object to be used for a tree view.
/// This method calls the optional virtual user defined method "defineUiTreeOrdering" to customize the
/// order and content of the children directly below each object. If this method does nothing,
/// the default behavior applies: Add all fields that contains objects, and their objects.
///
/// The caller is responsible to delete the returned UiTreeOrdering
//--------------------------------------------------------------------------------------------------
UiTreeOrdering* ObjectUiCapability::uiTreeOrdering() const
{
    CAFFA_ASSERT( this ); // This method actually is possible to call on a nullptr without getting a crash, so we assert
                        // instead.

    UiTreeOrdering* uiTreeOrdering = new UiTreeOrdering( nullptr, m_owner );

    expandUiTree( uiTreeOrdering );

    return uiTreeOrdering;
}

//--------------------------------------------------------------------------------------------------
/// Adds the direct children of this Object to the UiTree according to
/// the default rules. Add all fields that contains objects, and their objects.
/// Takes into account the control variables regarding this:
/// Field::isUiHidden
/// Field::isUiChildrenHidden
/// And whether the fields and objects are already added by the user.
//--------------------------------------------------------------------------------------------------
void ObjectUiCapability::addDefaultUiTreeChildren( UiTreeOrdering* uiTreeOrdering )
{
#if 1
    if ( uiTreeOrdering->isIncludingRemainingChildren() )
    {
        for ( auto field : m_owner->fields() )
        {
            if ( field->hasChildObjects() && !uiTreeOrdering->containsField( field ) )
            {
                if ( field->capability<FieldUiCapability>()->isUiHidden() &&
                     !field->capability<FieldUiCapability>()->isUiTreeChildrenHidden() )
                {
                    std::vector<ObjectHandle*> children;
                    field->childObjects( &children );

                    std::set<ObjectHandle*> objectsAddedByApplication;
                    for ( int i = 0; i < uiTreeOrdering->childCount(); i++ )
                    {
                        if ( uiTreeOrdering->child( i )->isRepresentingObject() )
                        {
                            objectsAddedByApplication.insert( uiTreeOrdering->child( i )->object() );
                        }
                    }

                    for ( size_t cIdx = 0; cIdx < children.size(); cIdx++ )
                    {
                        if ( children[cIdx] )
                        {
                            bool                              isAlreadyAdded = false;
                            std::set<ObjectHandle*>::iterator it = objectsAddedByApplication.find( children[cIdx] );
                            if ( it != objectsAddedByApplication.end() )
                            {
                                isAlreadyAdded = true;
                                break;
                            }

                            if ( !isAlreadyAdded )
                            {
                                uiTreeOrdering->add( children[cIdx] );
                            }
                        }
                    }
                }
                else if ( !field->capability<FieldUiCapability>()->isUiHidden() )
                {
                    uiTreeOrdering->add( field );
                }
            }
        }
    }
#endif
}

//--------------------------------------------------------------------------------------------------
/// Builds the sUiTree for all the children of @param root recursively, and stores the result
/// in root
//--------------------------------------------------------------------------------------------------
void ObjectUiCapability::expandUiTree( UiTreeOrdering* root )
{
#if 1
    if ( !root || !root->isValid() ) return;

    if ( root->childCount() > 0 )
    {
        for ( int cIdx = 0; cIdx < root->childCount(); ++cIdx )
        {
            UiTreeOrdering* child = root->child( cIdx );
            if ( child->isValid() && !child->ignoreSubTree() )
            {
                expandUiTree( child );
            }
        }
    }
    else //( root->childCount() == 0) // This means that no one has tried to expand it.
    {
        if ( !root->ignoreSubTree() )
        {
            if ( root->isRepresentingField() && !root->field()->capability<FieldUiCapability>()->isUiTreeChildrenHidden() )
            {
                std::vector<ObjectHandle*> fieldsChildObjects;
                root->field()->childObjects( &fieldsChildObjects );
                for ( size_t cIdx = 0; cIdx < fieldsChildObjects.size(); ++cIdx )
                {
                    ObjectHandle* childObject = fieldsChildObjects[cIdx];
                    if ( childObject )
                    {
                        root->appendChild( uiObj( childObject )->uiTreeOrdering() );
                    }
                }
            }
            else if ( root->isRepresentingObject() &&
                      !root->object()->capability<ObjectUiCapability>()->isUiTreeChildrenHidden() )
            {
                uiObj( root->object() )->defineUiTreeOrdering( *root );
                uiObj( root->object() )->addDefaultUiTreeChildren( root );
                if ( root->childCount() )
                {
                    expandUiTree( root );
                }
            }
        }
    }
#endif
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectUiCapability::updateUiIconFromToggleField()
{
    if ( m_owner->objectToggleField() )
    {
        Field<bool>* toggleField = dynamic_cast<Field<bool>*>( m_owner->objectToggleField() );
        if ( toggleField )
        {
            bool active = toggleField->value();
            updateUiIconFromState( active );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectUiCapability::fieldChangedByUi( const FieldHandle* changedField, const Variant& oldValue, const Variant& newValue )
{
    m_owner->fieldChangedByCapability( changedField, changedField->capability<FieldUiCapability>(), oldValue, newValue );
}

} // End namespace caffa
