//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2019- Ceetron Solutions AS
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

#include "cafToggleItemsOnOthersOffFeature.h"

#include "cafToggleItemsFeatureImpl.h"

#include "cafSelectionManager.h"

#include "cafObject.h"
#include "cafObjectHandle.h"
#include "cafPdmUiItem.h"
#include <QAction>

namespace caf
{
CAF_CMD_SOURCE_INIT( ToggleItemsOnOthersOffFeature, "cafToggleItemsOnOthersOffFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool ToggleItemsOnOthersOffFeature::isCommandEnabled()
{
    std::vector<caf::Object*> selectedObjects;
    caf::SelectionManager::instance()->objectsByType( &selectedObjects );

    caf::FieldHandle*               commonParent = verifySameParentForSelection( selectedObjects );
    std::vector<caf::ObjectHandle*> children     = childObjects( commonParent );

    return commonParent != nullptr && children.size() > 0 && objectToggleField( children.front() ) &&
           children.size() > selectedObjects.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ToggleItemsOnOthersOffFeature::onActionTriggered( bool isChecked )
{
    std::vector<caf::Object*> selectedObjects;
    caf::SelectionManager::instance()->objectsByType( &selectedObjects );

    // First toggle off all siblings

    caf::FieldHandle* commonParent = verifySameParentForSelection( selectedObjects );

    for ( caf::ObjectHandle* child : childObjects( commonParent ) )
    {
        caf::Field<bool>* field = objectToggleField( child );

        if ( field )
        {
            field->setValueWithFieldChanged( false );
        }
    }

    // Then toggle on the selected item(s)
    for ( caf::Object* selectedObject : selectedObjects )
    {
        caf::Field<bool>* field = dynamic_cast<caf::Field<bool>*>( selectedObject->objectToggleField() );

        field->setValueWithFieldChanged( true );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ToggleItemsOnOthersOffFeature::setupActionLook( ActionWrapper* actionToSetup )
{
    actionToSetup->setText( "On - Others Off" );

    actionToSetup->setIcon( IconProvider( ":/cafCommandFeatures/ToggleOnOthersOffL16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::FieldHandle*
    ToggleItemsOnOthersOffFeature::verifySameParentForSelection( const std::vector<caf::Object*>& selection )
{
    caf::FieldHandle* sameParent = nullptr;

    for ( caf::Object* obj : selection )
    {
        caf::FieldHandle* parent = obj->parentField();
        if ( parent )
        {
            if ( !sameParent )
            {
                sameParent = parent;
            }
            else if ( parent != sameParent )
            {
                // Different parents detected

                return nullptr;
            }
        }
    }
    return sameParent;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::ObjectHandle*> ToggleItemsOnOthersOffFeature::childObjects( caf::FieldHandle* parent )
{
    std::vector<caf::ObjectHandle*> children;
    if ( parent )
    {
        parent->childObjects( &children );
    }
    return children;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::Field<bool>* ToggleItemsOnOthersOffFeature::objectToggleField( caf::ObjectHandle* objectHandle )
{
    caf::ObjectUiCapability* childUiObject = uiObj( objectHandle );
    if ( childUiObject && childUiObject->objectToggleField() )
    {
        return dynamic_cast<caf::Field<bool>*>( childUiObject->objectToggleField() );
    }
    return nullptr;
}

} // namespace caf
