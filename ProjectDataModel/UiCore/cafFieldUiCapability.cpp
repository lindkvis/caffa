#include "cafFieldUiCapability.h"

#include "cafAssert.h"
#include "cafFieldHandle.h"
#include "cafObjectUiCapability.h"
#include "cafUiEditorHandle.h"

namespace caffa
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
FieldUiCapability::FieldUiCapability( FieldHandle* owner, bool giveOwnership )
{
    m_owner = owner;
    owner->addCapability( this, giveOwnership );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
FieldUiCapability::~FieldUiCapability()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caffa::FieldHandle* FieldUiCapability::fieldHandle()
{
    return m_owner;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Variant FieldUiCapability::uiValue() const
{
    return Variant();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::deque<OptionItemInfo> FieldUiCapability::valueOptions( bool* useOptionsOnly ) const
{
    return std::deque<OptionItemInfo>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FieldUiCapability::notifyFieldChanged( const FieldCapability* changedByCapability,
                                            const Variant&         oldFieldValue,
                                            const Variant&         newFieldValue )
{
    if ( !this->isVariantDataEqual( oldFieldValue, newFieldValue ) )
    {
        FieldHandle* fieldHandle = this->fieldHandle();
        CAFFA_ASSERT( fieldHandle && fieldHandle->ownerObject() );

        ObjectHandle* ownerObjectHandle = fieldHandle->ownerObject();

        {
            bool noOwnerObject = true;

            // Object editors

            ObjectUiCapability* uiObjHandle = uiObj( ownerObjectHandle );
            if ( uiObjHandle )
            {
                uiObjHandle->fieldChangedByUi( fieldHandle, oldFieldValue, newFieldValue );
                uiObjHandle->updateConnectedEditors();

                noOwnerObject = false;
            }

            // Field editors

            for ( const auto& editorForThisField : m_editors )
            {
                UiEditorHandle* editorContainingThisField = editorForThisField->topMostContainingEditor();

                bool editorContainingThisFieldIsNotUpdated = !uiObjHandle->hasEditor( editorContainingThisField );

                if ( noOwnerObject || editorContainingThisFieldIsNotUpdated )
                {
                    editorContainingThisField->updateUi();
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FieldUiCapability::setValueFromUiEditor( const Variant& uiValue )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool FieldUiCapability::isVariantDataEqual( const Variant& oldUiBasedVariant, const Variant& newUiBasedVariant ) const
{
    CAFFA_ASSERT( false );
    return false;
}

} // End of namespace caffa
