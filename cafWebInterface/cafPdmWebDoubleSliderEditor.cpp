//##################################################################################################
//
//   Caffa Web Interface
//   Copyright (C) Ceetron AS
//   Copyright (C) Gaute Lindkvist
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
#include "cafPdmWebDoubleSliderEditor.h"

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 4267 4275 4564 )
#endif

#include "cafAssert.h"
#include "cafField.h"
#include "cafFieldUiCapability.h"
#include "cafObjectUiCapability.h"

#include <Wt/WContainerWidget.h>
#include <Wt/WHBoxLayout.h>

#include <functional>

namespace caf
{
CAF_PDM_WEB_FIELD_EDITOR_SOURCE_INIT( PdmWebDoubleSliderEditor );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebDoubleSliderEditor::configureAndUpdateUi()
{
    CAF_ASSERT( m_spinBox );

    applyTextToLabel( m_label.get() );

    m_spinBox->setEnabled( !uiField()->isUiReadOnly() );
    m_slider->setEnabled( !uiField()->isUiReadOnly() );

    caf::ObjectUiCapability* uiObject = uiObj( uiField()->fieldHandle()->ownerObject() );
    if ( uiObject )
    {
        uiObject->editorAttribute( uiField()->fieldHandle(), &m_attributes );
    }

    double doubleValue = uiField()->uiValue().value<double>();

    m_slider->setMaximum( m_attributes.m_sliderTickCount );
    m_spinBox->setRange( m_attributes.m_minimum, m_attributes.m_maximum );
    m_spinBox->setSingleStep( stepSize() );

    m_spinBox->setValue( doubleValue );
    m_sliderValue = doubleValue;
    updateSliderPosition( doubleValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Wt::WWidget* PdmWebDoubleSliderEditor::createEditorWidget()
{
    Wt::WContainerWidget* containerWidget = new Wt::WContainerWidget;

    auto layout = std::make_unique<Wt::WHBoxLayout>();
    layout->setContentsMargins( 0, 0, 0, 0 );

    auto spinBox = std::make_unique<Wt::WDoubleSpinBox>();
    m_spinBox    = spinBox.get();
    m_spinBox->setMaximumSize( 50, 50 );
    m_spinBox->valueChanged().connect(
        std::bind( &PdmWebDoubleSliderEditor::slotEditingFinished, this, std::placeholders::_1 ) );

    auto slider = std::make_unique<Wt::WSlider>();
    m_slider    = slider.get();
    slider->valueChanged().connect(
        std::bind( &PdmWebDoubleSliderEditor::slotSliderReleased, this, std::placeholders::_1 ) );
    slider->sliderMoved().connect( std::bind( &PdmWebDoubleSliderEditor::slotSliderMoved, this, std::placeholders::_1 ) );

    layout->addWidget( std::move( spinBox ) );
    layout->addWidget( std::move( slider ) );
    containerWidget->setLayout( std::move( layout ) );

    return containerWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Wt::WLabel* PdmWebDoubleSliderEditor::createLabelWidget()
{
    m_label = new Wt::WLabel;
    return m_label.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebDoubleSliderEditor::slotEditingFinished( double value )
{
    double doubleVal = m_spinBox->value();
    doubleVal        = std::clamp( doubleVal, m_attributes.m_minimum, m_attributes.m_maximum );
    m_sliderValue    = doubleVal;

    writeValueToField( doubleVal );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebDoubleSliderEditor::slotSliderMoved( int value )
{
    double newDoubleValue = convertFromSliderValue( value );
    m_sliderValue         = newDoubleValue;

    m_spinBox->setValue( m_sliderValue );

    if ( !m_attributes.m_delaySliderUpdateUntilRelease )
    {
        writeValueToField( m_sliderValue );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebDoubleSliderEditor::slotSliderReleased( int value )
{
    double newDoubleValue = convertFromSliderValue( value );
    m_sliderValue         = newDoubleValue;

    m_spinBox->setValue( m_sliderValue );

    writeValueToField( value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebDoubleSliderEditor::updateSliderPosition( double value )
{
    int newSliderPosition = convertToSliderValue( value );
    if ( m_slider->value() != newSliderPosition )
    {
        m_slider->setValue( newSliderPosition );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebDoubleSliderEditor::writeValueToField( double value )
{
    Variant v( value );
    this->setValueToField( v );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double PdmWebDoubleSliderEditor::stepSize() const
{
    return ( m_attributes.m_maximum - m_attributes.m_minimum ) / m_slider->maximum();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int PdmWebDoubleSliderEditor::convertToSliderValue( double value ) const
{
    double exactSliderValue =
        m_slider->maximum() * ( value - m_attributes.m_minimum ) / ( m_attributes.m_maximum - m_attributes.m_minimum );

    int sliderValue = static_cast<int>( exactSliderValue + 0.5 );
    sliderValue     = std::clamp( sliderValue, m_slider->minimum(), m_slider->maximum() );

    return sliderValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double PdmWebDoubleSliderEditor::convertFromSliderValue( int sliderValue ) const
{
    double newDoubleValue = m_attributes.m_minimum + sliderValue * stepSize();
    newDoubleValue        = std::clamp( newDoubleValue, m_attributes.m_minimum, m_attributes.m_maximum );

    return newDoubleValue;
}

} // end namespace caf

#ifdef _MSC_VER
#pragma warning( pop )
#endif
