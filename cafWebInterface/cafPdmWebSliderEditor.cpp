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
#include "cafPdmWebSliderEditor.h"

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 4267 4275 4564 )
#endif

#include "cafAssert.h"
#include "cafField.h"
#include "cafObject.h"
#include "cafPdmWebDefaultObjectEditor.h"
#include "cafPdmWebFieldEditorHandle.h"

#include "cafFactory.h"

#include <Wt/WContainerWidget.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WIntValidator.h>

#include <functional>
#include <memory>

namespace caf
{
CAF_PDM_WEB_FIELD_EDITOR_SOURCE_INIT( PdmWebSliderEditor );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebSliderEditor::configureAndUpdateUi()
{
    applyTextToLabel( m_label.get() );

    if ( m_spinBox )
    {
        m_spinBox->setEnabled( !uiField()->isUiReadOnly() );
        m_spinBox->setToolTip( uiField()->uiToolTip() );
    }
    if ( m_slider )
    {
        m_slider->setEnabled( !uiField()->isUiReadOnly() );
        m_slider->setToolTip( uiField()->uiToolTip() );
    }

    caf::ObjectUiCapability* uiObject = uiObj( uiField()->fieldHandle()->ownerObject() );
    if ( uiObject )
    {
        uiObject->editorAttribute( uiField()->fieldHandle(), &m_attributes );
    }

    {
        if ( m_spinBox ) m_spinBox->setRange( m_attributes.m_minimum, m_attributes.m_maximum );
        if ( m_slider ) m_slider->setRange( m_attributes.m_minimum, m_attributes.m_maximum );

        int value = uiField()->uiValue().value<int>();
        if ( m_spinBox ) m_spinBox->setValue( value );
        if ( m_slider ) m_slider->setValue( value );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Wt::WWidget* PdmWebSliderEditor::createEditorWidget()
{
    auto containerWidget = new Wt::WContainerWidget;

    auto layout = std::make_unique<Wt::WHBoxLayout>();
    layout->setContentsMargins( 0, 0, 0, 0 );

    auto spinBox = std::make_unique<Wt::WSpinBox>();
    m_spinBox    = spinBox.get();
    m_spinBox->setMaximumSize( 50, 50 );
    m_spinBox->valueChanged().connect(
        std::bind( &PdmWebSliderEditor::slotSpinBoxValueChanged, this, std::placeholders::_1 ) );

    auto slider = std::make_unique<Wt::WSlider>();
    m_slider    = slider.get();
    m_slider->setTickPosition( Wt::WSlider::TicksBothSides );
    m_slider->valueChanged().connect(
        std::bind( &PdmWebSliderEditor::slotSliderValueChanged, this, std::placeholders::_1 ) );
    slider->sliderMoved().connect( std::bind( &PdmWebSliderEditor::slotSliderMoved, this, std::placeholders::_1 ) );

    layout->addWidget( std::move( spinBox ), 0, Wt::AlignmentFlag::Middle );
    layout->addWidget( std::move( slider ), 1, Wt::AlignmentFlag::Middle );

    containerWidget->setLayout( std::move( layout ) );
    return containerWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Wt::WLabel* PdmWebSliderEditor::createLabelWidget()
{
    m_label = new Wt::WLabel;
    return m_label.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebSliderEditor::slotSliderValueChanged( int value )
{
    m_spinBox->setValue( value );
    writeValueToField();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebSliderEditor::slotSliderMoved( int value )
{
    m_spinBox->setValue( value );
    if ( !m_attributes.m_delaySliderUpdateUntilRelease )
    {
        writeValueToField();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebSliderEditor::slotSpinBoxValueChanged( int value )
{
    m_slider->setValue( value );
    writeValueToField();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebSliderEditor::writeValueToField()
{
    int     value = m_slider->value();
    Variant v( value );
    this->setValueToField( v );
}

} // end namespace caf

#ifdef _MSC_VER
#pragma warning( pop )
#endif
