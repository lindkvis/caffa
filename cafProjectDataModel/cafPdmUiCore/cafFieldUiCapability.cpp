#include "cafFieldUiCapability.h"

#include "cafAssert.h"
#include "cafFieldHandle.h"
#include "cafObjectUiCapability.h"
#include "cafUiEditorHandle.h"
#include "cafUiModelChangeDetector.h"

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
FieldUiCapability::FieldUiCapability( FieldHandle* owner, bool giveOwnership )
    : m_isAutoAddingOptionFromValue( true )
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
caf::FieldHandle* FieldUiCapability::fieldHandle()
{
    return m_owner;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QVariant FieldUiCapability::uiValue() const
{
    return QVariant();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> FieldUiCapability::valueOptions( bool* useOptionsOnly ) const
{
    return QList<PdmOptionItemInfo>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FieldUiCapability::notifyFieldChanged( const QVariant& oldFieldValue, const QVariant& newFieldValue )
{
    if ( !this->isQVariantDataEqual( oldFieldValue, newFieldValue ) )
    {
        FieldHandle* fieldHandle = this->fieldHandle();
        CAF_ASSERT( fieldHandle && fieldHandle->ownerObject() );

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

        if ( ownerObjectHandle->parentField() && ownerObjectHandle->parentField()->ownerObject() )
        {
            ObjectUiCapability* uiObjHandle = uiObj( ownerObjectHandle->parentField()->ownerObject() );
            if ( uiObjHandle )
            {
                uiObjHandle->childFieldChangedByUi( ownerObjectHandle->parentField() );

                // If updateConnectedEditors() is required, this has to be called in childFieldChangedByUi()
            }
        }

        PdmUiModelChangeDetector::instance()->setModelChanged();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool FieldUiCapability::isAutoAddingOptionFromValue() const
{
    return m_isAutoAddingOptionFromValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FieldUiCapability::setAutoAddingOptionFromValue( bool isAddingValue )
{
    m_isAutoAddingOptionFromValue = isAddingValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FieldUiCapability::setValueFromUiEditor( const QVariant& uiValue )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool FieldUiCapability::isQVariantDataEqual( const QVariant& oldUiBasedQVariant, const QVariant& newUiBasedQVariant ) const
{
    CAF_ASSERT( false );
    return false;
}

} // End of namespace caf
