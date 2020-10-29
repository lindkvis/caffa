#include "cafObjectUiCapability.h"

#include "cafAssert.h"
#include "cafFieldHandle.h"
#include "cafFieldUiCapability.h"
#include "cafObjectHandle.h"
#include "cafUiOrdering.h"
#include "cafUiTreeOrdering.h"

namespace caf
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
    CAF_ASSERT( uiObject );
    return uiObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectUiCapability::uiOrdering( const QString& uiConfigName, PdmUiOrdering& uiOrdering )
{
    // Restore state for includeRemainingFields, as this flag
    // can be changed in defineUiOrdering()
    bool includeRemaining_originalState = uiOrdering.isIncludingRemainingFields();

    this->defineUiOrdering( uiConfigName, uiOrdering );
    if ( uiOrdering.isIncludingRemainingFields() )
    {
        // Add the remaining Fields To UiConfig
        std::vector<FieldHandle*> fields;
        m_owner->fields( fields );
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

    CAF_ASSERT( includeRemaining_originalState == uiOrdering.isIncludingRemainingFields() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectUiCapability::editorAttribute( const FieldHandle* field,
                                             const QString&        uiConfigName,
                                             UiEditorAttribute* attribute )
{
    this->defineEditorAttribute( field, uiConfigName, attribute );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ObjectUiCapability::objectEditorAttribute( const QString& uiConfigName, UiEditorAttribute* attribute )
{
    this->defineObjectEditorAttribute( uiConfigName, attribute );
}

//--------------------------------------------------------------------------------------------------
/// This method creates a tree-representation of the object hierarchy starting at this
/// object to be used for a tree view.
/// This method calls the optional virtual user defined method "defineUiTreeOrdering" to customize the
/// order and content of the children directly below each object. If this method does nothing,
/// the default behavior applies: Add all fields that contains objects, and their objects.
///
/// The caller is responsible to delete the returned PdmUiTreeOrdering
//--------------------------------------------------------------------------------------------------
PdmUiTreeOrdering* ObjectUiCapability::uiTreeOrdering( const QString& uiConfigName /*= ""*/ ) const
{
    CAF_ASSERT( this ); // This method actually is possible to call on a NULL ptr without getting a crash, so we assert
                        // instead.

    PdmUiTreeOrdering* uiTreeOrdering = new PdmUiTreeOrdering( nullptr, m_owner );

    expandUiTree( uiTreeOrdering, uiConfigName );

    return uiTreeOrdering;
}

//--------------------------------------------------------------------------------------------------
/// Adds the direct children of this Object to the PdmUiTree according to
/// the default rules. Add all fields that contains objects, and their objects.
/// Takes into account the control variables regarding this:
/// Field::isUiHidden
/// Field::isUiChildrenHidden
/// And whether the fields and objects are already added by the user.
//--------------------------------------------------------------------------------------------------
void ObjectUiCapability::addDefaultUiTreeChildren( PdmUiTreeOrdering* uiTreeOrdering )
{
#if 1
    if ( uiTreeOrdering->isIncludingRemainingChildren() )
    {
        // Add the remaining Fields To UiConfig
        std::vector<FieldHandle*> fields;
        m_owner->fields( fields );

        for ( size_t fIdx = 0; fIdx < fields.size(); ++fIdx )
        {
            if ( fields[fIdx]->hasChildObjects() && !uiTreeOrdering->containsField( fields[fIdx] ) )
            {
                if ( fields[fIdx]->capability<FieldUiCapability>()->isUiHidden() &&
                     !fields[fIdx]->capability<FieldUiCapability>()->isUiTreeChildrenHidden() )
                {
                    std::vector<ObjectHandle*> children;
                    fields[fIdx]->childObjects( &children );

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
                            bool                                 isAlreadyAdded = false;
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
                else if ( !fields[fIdx]->capability<FieldUiCapability>()->isUiHidden() )
                {
                    uiTreeOrdering->add( fields[fIdx] );
                }
            }
        }
    }
#endif
}

//--------------------------------------------------------------------------------------------------
/// Builds the sPdmUiTree for all the children of @param root recursively, and stores the result
/// in root
//--------------------------------------------------------------------------------------------------
void ObjectUiCapability::expandUiTree( PdmUiTreeOrdering* root, const QString& uiConfigName /*= "" */ )
{
#if 1
    if ( !root || !root->isValid() ) return;

    if ( root->childCount() > 0 )
    {
        for ( int cIdx = 0; cIdx < root->childCount(); ++cIdx )
        {
            PdmUiTreeOrdering* child = root->child( cIdx );
            if ( child->isValid() && !child->ignoreSubTree() )
            {
                expandUiTree( child, uiConfigName );
            }
        }
    }
    else //( root->childCount() == 0) // This means that no one has tried to expand it.
    {
        if ( !root->ignoreSubTree() )
        {
            if ( root->isRepresentingField() &&
                 !root->field()->capability<FieldUiCapability>()->isUiTreeChildrenHidden( uiConfigName ) )
            {
                std::vector<ObjectHandle*> fieldsChildObjects;
                root->field()->childObjects( &fieldsChildObjects );
                for ( size_t cIdx = 0; cIdx < fieldsChildObjects.size(); ++cIdx )
                {
                    ObjectHandle* childObject = fieldsChildObjects[cIdx];
                    if ( childObject )
                    {
                        root->appendChild( uiObj( childObject )->uiTreeOrdering( uiConfigName ) );
                    }
                }
            }
            else if ( root->isRepresentingObject() &&
                      !root->object()->capability<ObjectUiCapability>()->isUiTreeChildrenHidden( uiConfigName ) )
            {
                uiObj( root->object() )->defineUiTreeOrdering( *root, uiConfigName );
                uiObj( root->object() )->addDefaultUiTreeChildren( root );
                if ( root->childCount() )
                {
                    expandUiTree( root, uiConfigName );
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
    if ( objectToggleField() )
    {
        FieldUiCapability* uiFieldHandle = objectToggleField()->capability<FieldUiCapability>();
        if ( uiFieldHandle )
        {
            bool active = uiFieldHandle->uiValue().toBool();
            updateUiIconFromState( active );
        }
    }
}

} // End namespace caf