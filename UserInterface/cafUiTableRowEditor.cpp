//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2014 Ceetron Solutions AS
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

#include "cafUiTableRowEditor.h"

#include "cafField.h"
#include "cafObject.h"
#include "cafUiEditorHandle.h"
#include "cafUiTableViewQModel.h"

namespace caffa
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiTableRowEditor::UiTableRowEditor( UiTableViewQModel* model, caffa::ObjectHandle* object, int row )
{
    m_model = model;
    m_row   = row;

    caffa::ObjectUiCapability* uiObject = uiObj( object );
    this->bindToItem( uiObject );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiTableRowEditor::~UiTableRowEditor()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTableRowEditor::configureAndUpdateUi()
{
    caffa::ObjectUiCapability* uiObject = dynamic_cast<caffa::ObjectUiCapability*>( this->item() );
    if ( uiObject )
    {
        // Call uiOrdering method, as this method is responsible for control of
        // object states like hidden/readOnly, etc...

        caffa::UiOrdering dummy;
        uiObject->uiOrdering( dummy );
    }

    if ( m_model )
    {
        QModelIndex miStart = m_model->index( m_row, 0 );
        QModelIndex miEnd   = m_model->index( m_row, m_model->columnCount() );

        m_model->notifyDataChanged( miStart, miEnd );
    }
}

} // end namespace caffa
