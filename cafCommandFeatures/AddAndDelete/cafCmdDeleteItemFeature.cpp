//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron AS
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

#include "cafCmdDeleteItemFeature.h"

#include "cafCmdDeleteItemExec.h"
#include "cafCmdDeleteItemExecData.h"
#include "cafCmdExecCommandManager.h"
#include "cafCmdSelectionHelper.h"
#include "cafPdmReferenceHelper.h"
#include "cafSelectionManager.h"

#include "cafChildArrayField.h"
#include "cafField.h"

#include <QAction>

namespace caf
{
CAF_CMD_SOURCE_INIT( CmdDeleteItemFeature, "PdmListField_DeleteItem" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CmdExecuteCommand* CmdDeleteItemFeature::createExecuteCommand()
{
    std::vector<UiItem*> items;
    SelectionManager::instance()->selectedItems( items, SelectionManager::FIRST_LEVEL );

    caf::ChildArrayFieldHandle* childArrayFieldHandle =
        caf::SelectionManager::instance()->activeChildArrayFieldHandle();
    if ( !childArrayFieldHandle ) return nullptr;

    caf::ObjectHandle* currentObject = nullptr;

    for ( size_t i = 0; i < items.size(); i++ )
    {
        if ( dynamic_cast<caf::ObjectUiCapability*>( items[i] ) )
        {
            currentObject = dynamic_cast<caf::ObjectUiCapability*>( items[i] )->objectHandle();
        }
    }

    if ( !currentObject ) return nullptr;

    int indexAfter = -1;

    std::vector<ObjectHandle*> childObjects;
    childArrayFieldHandle->childObjects( &childObjects );

    for ( size_t i = 0; i < childObjects.size(); i++ )
    {
        if ( childObjects[i] == currentObject )
        {
            indexAfter = static_cast<int>( i );
        }
    }

    // Did not find currently selected pdm object in the current list field
    CAF_ASSERT( indexAfter != -1 );

    CmdDeleteItemExec* executeCmd = new CmdDeleteItemExec();

    CmdDeleteItemExecData* data = executeCmd->commandData();
    data->m_rootObject          = PdmReferenceHelper::findRoot( childArrayFieldHandle );
    data->m_pathToField   = PdmReferenceHelper::referenceFromRootToField( data->m_rootObject, childArrayFieldHandle );
    data->m_indexToObject = indexAfter;

    return executeCmd;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool CmdDeleteItemFeature::isCommandEnabled()
{
    caf::Object* currentObject = dynamic_cast<caf::Object*>(
        caf::SelectionManager::instance()->selectedItem( caf::SelectionManager::FIRST_LEVEL ) );
    if ( !currentObject ) return false;

    caf::ChildArrayFieldHandle* childArrayFieldHandle =
        caf::SelectionManager::instance()->activeChildArrayFieldHandle();
    if ( !childArrayFieldHandle ) return false;

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CmdDeleteItemFeature::onActionTriggered( bool isChecked )
{
    if ( isCommandEnabled() )
    {
        CmdExecuteCommand* exeCmd = createExecuteCommand();
        if ( exeCmd )
        {
            CmdExecCommandManager::instance()->processExecuteCommand( exeCmd );
        }
        else
        {
            CAF_ASSERT( 0 );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CmdDeleteItemFeature::setupActionLook( ActionWrapper* actionToSetup )
{
    actionToSetup->setText( "Delete object" );
    applyShortcutWithHintToAction( actionToSetup, QKeySequence::Delete );
}

} // end namespace caf
