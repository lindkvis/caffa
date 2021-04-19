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

#include "cafUiTimeEditor.h"

#include "cafFactory.h"
#include "cafField.h"
#include "cafObject.h"
#include "cafSelectionManager.h"
#include "cafUiDefaultObjectEditor.h"
#include "cafUiFieldEditorHandle.h"
#include "cafUiOrdering.h"

#include <QApplication>
#include <QGridLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMessageBox>
#include <QPalette>
#include <QStatusBar>
#include <QString>

namespace caffa
{
CAF_UI_FIELD_EDITOR_SOURCE_INIT( UiTimeEditor );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTimeEditor::configureAndUpdateUi()
{
    CAF_ASSERT( !m_timeEdit.isNull() );

    UiFieldEditorHandle::updateLabelFromField( m_label );

    m_timeEdit->setEnabled( !uiField()->isUiReadOnly() );

    caffa::ObjectUiCapability* uiObject = uiObj( uiField()->fieldHandle()->ownerObject() );
    if ( uiObject )
    {
        uiObject->editorAttribute( uiField()->fieldHandle(), &m_attributes );
    }

    if ( !m_attributes.timeFormat.isEmpty() )
    {
        m_timeEdit->setDisplayFormat( m_attributes.timeFormat );
    }

    std::time_t secsSinceEpoch = uiField()->uiValue().value<std::time_t>();
    QTime       time           = QTime::fromMSecsSinceStartOfDay( secsSinceEpoch * 1000 );
    m_timeEdit->setTime( time );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* UiTimeEditor::createEditorWidget( QWidget* parent )
{
    m_timeEdit = new QTimeEdit( parent );
    connect( m_timeEdit, SIGNAL( editingFinished() ), this, SLOT( slotEditingFinished() ) );
    connect( m_timeEdit, SIGNAL( timeChanged( QTime ) ), this, SLOT( slotTimeChanged( QTime ) ) );
    return m_timeEdit;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* UiTimeEditor::createLabelWidget( QWidget* parent )
{
    m_label = new QLabel( parent );
    return m_label;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTimeEditor::slotEditingFinished()
{
    std::time_t time = m_timeEdit->time().msecsSinceStartOfDay() / 1000;
    Variant     v( time );
    this->setValueToField( v );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTimeEditor::slotTimeChanged( const QTime& time )
{
    std::time_t timet = m_timeEdit->time().msecsSinceStartOfDay() / 1000;
    Variant     v( timet );
    this->setValueToField( v );
}

} // end namespace caffa
