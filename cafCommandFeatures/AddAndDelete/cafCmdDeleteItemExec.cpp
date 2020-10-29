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

#include "cafCmdDeleteItemExec.h"
#include "cafCmdDeleteItemExecData.h"

#include "cafChildArrayField.h"
#include "cafFieldUiCapability.h"
#include "cafPdmReferenceHelper.h"

#include "cafSelectionManager.h"

#include <memory>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString CmdDeleteItemExec::name()
{
    return m_commandData->classKeyword();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CmdDeleteItemExec::redo()
{
    FieldHandle* field =
        PdmReferenceHelper::fieldFromReference( m_commandData->m_rootObject, m_commandData->m_pathToField );

    ChildArrayFieldHandle* listField = dynamic_cast<ChildArrayFieldHandle*>( field );
    if ( listField )
    {
        std::vector<ObjectHandle*> children;
        listField->childObjects( &children );

        std::unique_ptr<ObjectHandle> obj( children[m_commandData->m_indexToObject] );
        caf::SelectionManager::instance()->removeObjectFromAllSelections( obj.get() );

        if ( m_commandData->m_deletedObjectAsXml().isEmpty() )
        {
            QString encodedXml;
            {
                m_commandData->m_deletedObjectAsXml = obj->capability<ObjectIoCapability>()->writeObjectToString();
            }
        }

        listField->erase( m_commandData->m_indexToObject );

        // TODO: The notification here could possibly be changed to
        // FieldUiCapability::notifyDataChange() similar to void CmdFieldChangeExec::redo()

        caf::ObjectUiCapability* ownerUiObject = uiObj( listField->ownerObject() );
        if ( ownerUiObject )
        {
            ownerUiObject->fieldChangedByUi( field, QVariant(), QVariant() );
        }

        listField->capability<FieldUiCapability>()->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CmdDeleteItemExec::undo()
{
    FieldHandle* field =
        PdmReferenceHelper::fieldFromReference( m_commandData->m_rootObject, m_commandData->m_pathToField );

    ChildArrayFieldHandle* listField = dynamic_cast<ChildArrayFieldHandle*>( field );
    if ( listField )
    {
        ObjectHandle* obj = ObjectIoCapability::readUnknownObjectFromString( m_commandData->m_deletedObjectAsXml(),
                                                                                   PdmDefaultObjectFactory::instance(),
                                                                                   false );

        listField->insertAt( m_commandData->m_indexToObject, obj );

        // TODO: The notification here could possibly be changed to
        // FieldUiCapability::notifyDataChange() similar to void CmdFieldChangeExec::redo()

        caf::ObjectUiCapability* ownerUiObject = uiObj( listField->ownerObject() );
        if ( ownerUiObject )
        {
            ownerUiObject->fieldChangedByUi( field, QVariant(), QVariant() );
        }

        listField->capability<FieldUiCapability>()->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CmdDeleteItemExec::CmdDeleteItemExec()
{
    m_commandData = new CmdDeleteItemExecData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CmdDeleteItemExecData* CmdDeleteItemExec::commandData()
{
    return m_commandData;
}

} // end namespace caf
