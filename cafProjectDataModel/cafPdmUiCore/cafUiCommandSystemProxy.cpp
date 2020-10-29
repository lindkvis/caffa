//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################

#include "cafUiCommandSystemProxy.h"

#include "cafAssert.h"
#include "cafInternalUiCommandSystemInterface.h"

#include "cafFieldHandle.h"
#include "cafFieldUiCapability.h"
#include "cafObjectHandle.h"
#include "cafObjectUiCapability.h"
#include "cafSelectionManager.h"

#include <cstddef>
#include <typeinfo>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiCommandSystemProxy* UiCommandSystemProxy::instance()
{
    static UiCommandSystemProxy staticInstance;

    return &staticInstance;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiCommandSystemProxy::UiCommandSystemProxy()
{
    m_commandInterface = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiCommandSystemProxy::setCommandInterface( UiCommandSystemInterface* commandInterface )
{
    m_commandInterface = commandInterface;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiCommandSystemProxy::setUiValueToField( FieldUiCapability* uiFieldHandle, const QVariant& newUiValue )
{
    if ( uiFieldHandle )
    {
        // Handle editing multiple objects when several objects are selected
        FieldHandle* editorField = uiFieldHandle->fieldHandle();
        auto            ownerObject = editorField->ownerObject();

        CAF_ASSERT( ownerObject );
        auto&                 ownerRef         = *ownerObject;
        const std::type_info& fieldOwnerTypeId = typeid( ownerRef );

        std::vector<FieldHandle*> fieldsToUpdate;
        fieldsToUpdate.push_back( editorField );

        // For level 1 selection, find all fields with same keyword
        // Todo: Should traverse the ui ordering and find all fields with same keyword and same ownerobject type.
        //       Until we do, fields embedded into the property panel from a different object will not work with
        //       multiselection edit For now we only makes sure we have same ownerobject type
        {
            std::vector<UiItem*> items;

            int selectionLevel = 1; // = 0;
            SelectionManager::instance()->selectedItems( items, selectionLevel );

            for ( size_t i = 0; i < items.size(); i++ )
            {
                ObjectHandle* objectHandle = dynamic_cast<ObjectHandle*>( items[i] );
                if ( objectHandle && typeid( *objectHandle ) == fieldOwnerTypeId )
                {
                    // An object is selected, find field with same keyword as the current field being edited
                    FieldHandle* fieldHandle = objectHandle->findField( editorField->keyword() );
                    if ( fieldHandle && fieldHandle != editorField )
                    {
                        fieldsToUpdate.push_back( fieldHandle );
                    }
                }
                else
                {
                    // Todo Remove when dust has settled. Selection manager is not supposed to select single fields
                    // A field is selected, check if keywords are identical
                    FieldUiCapability* itemFieldHandle = dynamic_cast<FieldUiCapability*>( items[i] );
                    if ( itemFieldHandle )
                    {
                        FieldHandle* field = itemFieldHandle->fieldHandle();
                        if ( field && field != editorField && field->keyword() == editorField->keyword() )
                        {
                            fieldsToUpdate.push_back( field );
                        }
                    }
                }
            }
        }

        if ( m_commandInterface )
        {
            m_commandInterface->fieldChangedCommand( fieldsToUpdate, newUiValue );
        }
        else
        {
            for ( auto fieldHandle : fieldsToUpdate )
            {
                fieldHandle->capability<FieldUiCapability>()->setValueFromUiEditor( newUiValue );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiCommandSystemProxy::setCurrentContextMenuTargetWidget( QWidget* targetWidget )
{
    if ( m_commandInterface )
    {
        m_commandInterface->setCurrentContextMenuTargetWidget( targetWidget );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiCommandSystemProxy::populateMenuWithDefaultCommands( const QString& uiConfigName, MenuInterface* menu )
{
    if ( m_commandInterface )
    {
        m_commandInterface->populateMenuWithDefaultCommands( uiConfigName, menu );
    }
}

} // end namespace caf
