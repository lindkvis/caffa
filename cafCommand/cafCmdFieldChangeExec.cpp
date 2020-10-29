//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
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

#include "cafCmdFieldChangeExec.h"

#include "cafPdmReferenceHelper.h"

namespace caf
{
CAF_PDM_SOURCE_INIT( CmdFieldChangeExecData, "CmdFieldChangeExecData" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString CmdFieldChangeExec::name()
{
    PdmFieldHandle* field =
        PdmReferenceHelper::fieldFromReference( m_commandData->m_rootObject, m_commandData->m_pathToField );
    if ( field )
    {
        QString fieldText;

        PdmUiFieldHandle* uiFieldHandle = field->capability<PdmUiFieldHandle>();
        if ( uiFieldHandle )
        {
            fieldText = QString( "Change field '%1'" ).arg( uiFieldHandle->uiName() );
        }

        if ( field->ownerObject() )
        {
            PdmUiObjectHandle* uiObjHandle = uiObj( field->ownerObject() );
            if ( uiObjHandle )
            {
                fieldText += QString( " in '%1'" ).arg( uiObjHandle->uiName() );
            }
        }
        return fieldText;
    }
    else
    {
        return m_commandData->classKeyword();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CmdFieldChangeExec::redo()
{
    PdmFieldHandle* field =
        PdmReferenceHelper::fieldFromReference( m_commandData->m_rootObject, m_commandData->m_pathToField );
    if ( !field )
    {
        CAF_ASSERT( false );
        return;
    }

    PdmUiFieldHandle*     uiFieldHandle = field->capability<PdmUiFieldHandle>();
    PdmFieldIoCapability* ioCapability  = field->capability<PdmFieldIoCapability>();
    if ( uiFieldHandle && ioCapability )
    {
        if ( m_commandData->m_redoFieldValueSerialized.isEmpty() )
        {
            // We end up here only when the user actually has done something in the actual living Gui editor.
            {
                QXmlStreamWriter xmlStream( &m_commandData->m_undoFieldValueSerialized );
                writeFieldDataToValidXmlDocument( xmlStream, ioCapability );
            }

            // This function will notify field change, no need to explicitly call notification
            // The ui value might be an index into the option entry cache, so we need to set the value
            // and be aware of the option entries, and then serialize the actual field value we ended up with.

            uiFieldHandle->setValueFromUiEditor( m_commandData->m_newUiValue );

            {
                QXmlStreamWriter xmlStream( &m_commandData->m_redoFieldValueSerialized );
                writeFieldDataToValidXmlDocument( xmlStream, ioCapability );
            }
        }
        else
        {
            QVariant oldFieldData = uiFieldHandle->toUiBasedQVariant();

            QXmlStreamReader xmlStream( m_commandData->m_redoFieldValueSerialized );

            readFieldValueFromValidXmlDocument( xmlStream, ioCapability );

            QVariant newFieldData = uiFieldHandle->toUiBasedQVariant();

            // New data is present in field, notify data changed
            uiFieldHandle->notifyFieldChanged( oldFieldData, newFieldData );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CmdFieldChangeExec::undo()
{
    PdmFieldHandle* field =
        PdmReferenceHelper::fieldFromReference( m_commandData->m_rootObject, m_commandData->m_pathToField );
    if ( !field )
    {
        CAF_ASSERT( false );
        return;
    }

    PdmUiFieldHandle*     uiFieldHandle = field->capability<PdmUiFieldHandle>();
    PdmFieldIoCapability* ioCapability  = field->capability<PdmFieldIoCapability>();
    if ( uiFieldHandle && ioCapability )
    {
        QXmlStreamReader xmlStream( m_commandData->m_undoFieldValueSerialized );
        QVariant         oldFieldData = uiFieldHandle->toUiBasedQVariant();

        readFieldValueFromValidXmlDocument( xmlStream, ioCapability );

        QVariant newFieldData = uiFieldHandle->toUiBasedQVariant();

        // New data is present in field, notify data changed
        uiFieldHandle->notifyFieldChanged( oldFieldData, newFieldData );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CmdFieldChangeExec::CmdFieldChangeExec()
    : CmdExecuteCommand()
{
    m_commandData = new CmdFieldChangeExecData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CmdFieldChangeExec::~CmdFieldChangeExec()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CmdFieldChangeExecData* CmdFieldChangeExec::commandData()
{
    return m_commandData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CmdFieldChangeExec::writeFieldDataToValidXmlDocument( QXmlStreamWriter& xmlStream, PdmFieldIoCapability* ioCapability )
{
    xmlStream.setAutoFormatting( true );
    xmlStream.writeStartDocument();
    xmlStream.writeStartElement( "", "d" );
    ioCapability->writeFieldData( xmlStream );
    xmlStream.writeEndElement();
    xmlStream.writeEndDocument();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CmdFieldChangeExec::readFieldValueFromValidXmlDocument( QXmlStreamReader& xmlStream, PdmFieldIoCapability* ioCapability )
{
    // See PdmObject::readFields and friends to match token count for reading field values
    // The stream is supposed to be pointing at the first token of field content when calling readFieldData()
    QXmlStreamReader::TokenType tt;
    int                         tokenCount = 3;
    for ( int i = 0; i < tokenCount; i++ )
    {
        tt = xmlStream.readNext();
    }
    ioCapability->readFieldData( xmlStream, PdmDefaultObjectFactory::instance() );
}

} // end namespace caf
