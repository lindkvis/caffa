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

#include "cafUiToolButtonEditor.h"

#include "cafFieldHandle.h"
#include "cafFieldUiCapability.h"
#include "cafObjectHandle.h"
#include "cafObjectUiCapability.h"

namespace caffa
{
CAFFA_UI_FIELD_EDITOR_SOURCE_INIT( UiToolButtonEditor );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiToolButtonEditor::configureAndUpdateUi()
{
    CAFFA_ASSERT( !m_toolButton.isNull() );

    auto ic = uiField()->uiIconProvider();
    if ( ic )
    {
        m_toolButton->setIcon( QIcon( QString::fromStdString( ic->iconResourceString() ) ) );
    }

    QString buttonText = QString::fromStdString( uiField()->uiName() );
    m_toolButton->setText( buttonText );

    m_toolButton->setEnabled( !uiField()->isUiReadOnly() );
    m_toolButton->setToolTip( QString::fromStdString( uiField()->uiToolTip() ) );

    UiToolButtonEditorAttribute attributes;

    ObjectUiCapability* uiOjectHandle = uiObj( uiField()->fieldHandle()->ownerObject() );
    if ( uiOjectHandle )
    {
        uiOjectHandle->editorAttribute( uiField()->fieldHandle(), &attributes );
    }
    bool isCheckable = attributes.m_checkable;
    m_toolButton->setCheckable( isCheckable );
    m_toolButton->setSizePolicy( attributes.m_sizePolicy );

    Variant variantFieldValue = uiField()->uiValue();
    if ( isCheckable )
    {
        m_toolButton->setChecked( uiField()->uiValue().value<bool>() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* UiToolButtonEditor::createEditorWidget( QWidget* parent )
{
    m_toolButton = new QToolButton( parent );
    connect( m_toolButton, SIGNAL( clicked( bool ) ), this, SLOT( slotClicked( bool ) ) );
    return m_toolButton;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* UiToolButtonEditor::createLabelWidget( QWidget* parent )
{
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiToolButtonEditor::slotClicked( bool checked )
{
    Variant v( checked );
    this->setValueToField( v );
}

} // end namespace caffa
