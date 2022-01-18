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

#include "cafUiCheckBoxEditor.h"

#include "cafUiDefaultObjectEditor.h"

#include "cafField.h"
#include "cafObject.h"
#include "cafUiFieldEditorHandle.h"
#include "cafUiOrdering.h"

#include "cafFactory.h"

#include <QLabel>

namespace caffa
{
CAFFA_UI_FIELD_EDITOR_SOURCE_INIT( UiCheckBoxEditor );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiCheckBoxEditor::configureAndUpdateUi()
{
    CAFFA_ASSERT( !m_checkBox.isNull() );
    CAFFA_ASSERT( !m_label.isNull() );

    UiCheckBoxEditorAttribute  attributes;
    caffa::ObjectUiCapability* uiObject = uiObj( uiField()->fieldHandle()->ownerObject() );
    if ( uiObject )
    {
        uiObject->editorAttribute( uiField()->fieldHandle(), &attributes );
    }

    if ( attributes.m_useNativeCheckBoxLabel )
    {
        m_checkBox->setText( QString::fromStdString( uiField()->uiName() ) );

        m_label->setEnabled( uiField()->isUiWritable() );
        m_label->setToolTip( QString::fromStdString( uiField()->uiToolTip() ) );
    }
    else
    {
        UiFieldEditorHandle::updateLabelFromField( m_label );
    }

    m_checkBox->setEnabled( uiField()->isUiWritable() );
    m_checkBox->setToolTip( QString::fromStdString( uiField()->uiToolTip() ) );

    if ( uiField()->isUiReadable() ) m_checkBox->setChecked( uiField()->uiValue().value<bool>() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* UiCheckBoxEditor::createEditorWidget( QWidget* parent )
{
    m_checkBox = new QCheckBox( parent );
    connect( m_checkBox, SIGNAL( clicked( bool ) ), this, SLOT( slotClicked( bool ) ) );
    return m_checkBox;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* UiCheckBoxEditor::createLabelWidget( QWidget* parent )
{
    m_label = new QLabel( parent );
    return m_label;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiCheckBoxEditor::slotClicked( bool checked )
{
    Variant v( checked );
    this->setValueToField( v );
}

} // end namespace caffa
