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

#include "cafUiDoubleValueEditor.h"

#include "cafField.h"
#include "cafObject.h"
#include "cafUiDefaultObjectEditor.h"
#include "cafUiFieldEditorHandle.h"

#include "cafFactory.h"

#include <QDoubleValidator>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

namespace caf
{
CAF_UI_FIELD_EDITOR_SOURCE_INIT( UiDoubleValueEditor );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiDoubleValueEditor::UiDoubleValueEditor()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiDoubleValueEditor::~UiDoubleValueEditor()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiDoubleValueEditor::configureAndUpdateUi()
{
    CAF_ASSERT( !m_lineEdit.isNull() );

    UiFieldEditorHandle::updateLabelFromField( m_label );

    m_lineEdit->setEnabled( !uiField()->isUiReadOnly() );

    caf::ObjectUiCapability* uiObject = uiObj( uiField()->fieldHandle()->ownerObject() );
    if ( uiObject )
    {
        uiObject->editorAttribute( uiField()->fieldHandle(), &m_attributes );
        if ( m_attributes.m_validator )
        {
            m_lineEdit->setValidator( m_attributes.m_validator );
        }
    }

    bool   valueOk = uiField()->uiValue().canConvert<double>();
    double value   = valueOk ? uiField()->uiValue().value<double>() : 0.0;

    QString textValue;
    if ( valueOk )
    {
        if ( m_attributes.m_numberFormat == UiDoubleValueEditorAttribute::NumberFormat::FIXED )
            textValue = QString::number( value, 'f', m_attributes.m_decimals );
        else if ( m_attributes.m_numberFormat == UiDoubleValueEditorAttribute::NumberFormat::SCIENTIFIC )
            textValue = QString::number( value, 'e', m_attributes.m_decimals );
        else
            textValue = QString::number( value, 'g', m_attributes.m_decimals );
        m_lineEdit->setText( textValue );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* UiDoubleValueEditor::createEditorWidget( QWidget* parent )
{
    QWidget* containerWidget = new QWidget( parent );

    QHBoxLayout* layout = new QHBoxLayout();
    layout->setMargin( 0 );
    containerWidget->setLayout( layout );

    m_lineEdit = new QLineEdit( containerWidget );
    connect( m_lineEdit, SIGNAL( editingFinished() ), this, SLOT( slotEditingFinished() ) );

    layout->addWidget( m_lineEdit );

    return containerWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* UiDoubleValueEditor::createLabelWidget( QWidget* parent )
{
    m_label = new QLabel( parent );
    return m_label;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiDoubleValueEditor::slotEditingFinished()
{
    writeValueToField();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiDoubleValueEditor::writeValueToField()
{
    QString textValue = m_lineEdit->text();
    Variant v( textValue.toStdString() );
    this->setValueToField( v );
}

} // end namespace caf
