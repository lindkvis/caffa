//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2017 Ceetron Solutions AS
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

#include "cafUiDateEditor.h"

#include "cafFactory.h"
#include "cafField.h"
#include "cafObject.h"
#include "cafSelectionManager.h"
#include "cafUiDefaultObjectEditor.h"
#include "cafUiFieldEditorHandle.h"
#include "cafUiOrdering.h"

#include <QApplication>
#include <QDate>
#include <QGridLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMessageBox>
#include <QPalette>
#include <QStatusBar>
#include <QString>
#include <QTimeZone>

namespace caf
{
CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT( PdmUiDateEditor );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiDateEditor::configureAndUpdateUi()
{
    CAF_ASSERT( !m_dateEdit.isNull() );

    UiFieldEditorHandle::updateLabelFromField( m_label );

    m_dateEdit->setEnabled( !uiField()->isUiReadOnly() );

    caf::ObjectUiCapability* uiObject = uiObj( uiField()->fieldHandle()->ownerObject() );
    if ( uiObject )
    {
        uiObject->editorAttribute( uiField()->fieldHandle(), &m_attributes );
    }

    if ( !m_attributes.dateFormat.empty() )
    {
        m_dateEdit->setDisplayFormat( QString::fromStdString( m_attributes.dateFormat ) );
    }

    Variant     v = uiField()->uiValue();
    std::time_t t = v.value<std::time_t>();

    QDateTime dateTime = QDateTime::fromSecsSinceEpoch( t );

    m_dateEdit->setDateTime( dateTime );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiDateEditor::createEditorWidget( QWidget* parent )
{
    m_dateEdit = new QDateTimeEdit( parent );
    m_dateEdit->setCalendarPopup( true );
    connect( m_dateEdit, SIGNAL( editingFinished() ), this, SLOT( slotEditingFinished() ) );
    return m_dateEdit;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiDateEditor::createLabelWidget( QWidget* parent )
{
    m_label = new QLabel( parent );
    return m_label;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiDateEditor::slotEditingFinished()
{
    std::time_t t = (std::time_t)m_dateEdit->dateTime().toSecsSinceEpoch();
    Variant     v( t );
    this->setValueToField( v );
}

} // end namespace caf
