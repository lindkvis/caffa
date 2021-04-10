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

#include "cafUiSliderEditor.h"

#include "cafField.h"
#include "cafObject.h"
#include "cafUiDefaultObjectEditor.h"
#include "cafUiFieldEditorHandle.h"

#include "cafFactory.h"

#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>

namespace caf
{
CAF_UI_FIELD_EDITOR_SOURCE_INIT( UiSliderEditor );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiSliderEditor::configureAndUpdateUi()
{
    CAF_ASSERT( !m_spinBox.isNull() );

    UiFieldEditorHandle::updateLabelFromField( m_label );

    m_spinBox->setEnabled( !uiField()->isUiReadOnly() );
    m_spinBox->setToolTip( QString::fromStdString( uiField()->uiToolTip() ) );

    m_slider->setEnabled( !uiField()->isUiReadOnly() );
    m_slider->setToolTip( QString::fromStdString( uiField()->uiToolTip() ) );

    caf::ObjectUiCapability* uiObject = uiObj( uiField()->fieldHandle()->ownerObject() );
    if ( uiObject )
    {
        uiObject->editorAttribute( uiField()->fieldHandle(), &m_attributes );
    }

    {
        m_spinBox->blockSignals( true );
        m_spinBox->setMinimum( m_attributes.m_minimum );
        m_spinBox->setMaximum( m_attributes.m_maximum );

        QString textValue = QString::fromStdString( uiField()->uiValue().value<std::string>() );
        m_spinBox->setValue( textValue.toInt() );
        m_spinBox->blockSignals( false );
    }

    {
        m_slider->blockSignals( true );
        m_slider->setRange( m_attributes.m_minimum, m_attributes.m_maximum );
        updateSliderPosition();
        m_slider->blockSignals( false );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* UiSliderEditor::createEditorWidget( QWidget* parent )
{
    QWidget* containerWidget = new QWidget( parent );

    QHBoxLayout* layout = new QHBoxLayout();
    layout->setMargin( 0 );
    containerWidget->setLayout( layout );

    m_spinBox = new QSpinBox( containerWidget );
    m_spinBox->setMaximumWidth( 60 );
    m_spinBox->setKeyboardTracking( false );
    connect( m_spinBox, SIGNAL( valueChanged( int ) ), this, SLOT( slotSpinBoxValueChanged( int ) ) );

    m_slider = new QSlider( Qt::Horizontal, containerWidget );
    layout->addWidget( m_spinBox );
    layout->addWidget( m_slider );

    connect( m_slider, SIGNAL( valueChanged( int ) ), this, SLOT( slotSliderValueChanged( int ) ) );

    return containerWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* UiSliderEditor::createLabelWidget( QWidget* parent )
{
    m_label = new QLabel( parent );
    return m_label;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiSliderEditor::slotSliderValueChanged( int position )
{
    m_spinBox->setValue( position );

    writeValueToField();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiSliderEditor::slotSpinBoxValueChanged( int spinBoxValue )
{
    updateSliderPosition();

    writeValueToField();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiSliderEditor::updateSliderPosition()
{
    QString textValue = m_spinBox->text();

    bool convertOk      = false;
    int  newSliderValue = textValue.toInt( &convertOk );
    if ( convertOk )
    {
        newSliderValue = qBound( m_attributes.m_minimum, newSliderValue, m_attributes.m_maximum );
        m_slider->setValue( newSliderValue );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiSliderEditor::writeValueToField()
{
    QString textValue = m_spinBox->text();
    Variant v( textValue.toStdString() );
    this->setValueToField( v );
}

} // end namespace caf
