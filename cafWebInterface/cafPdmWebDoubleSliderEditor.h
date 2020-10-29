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

#pragma once

#include "cafUiSliderEditorAttribute.h"
#include "cafPdmWebFieldEditorHandle.h"

#include <Wt/Core/observing_ptr.hpp>
#include <Wt/WDoubleSpinBox.h>
#include <Wt/WLabel.h>
#include <Wt/WSlider.h>

namespace caf
{
//==================================================================================================
///
//==================================================================================================
class PdmWebDoubleSliderEditor : public PdmWebFieldEditorHandle
{
    CAF_PDM_WEB_FIELD_EDITOR_HEADER_INIT;

public:
    PdmWebDoubleSliderEditor() {}
    ~PdmWebDoubleSliderEditor() override {}

protected:
    void         configureAndUpdateUi( const QString& uiConfigName ) override;
    Wt::WWidget* createEditorWidget() override;
    Wt::WLabel*  createLabelWidget() override;

    // protected slots:
    void slotEditingFinished( int value );
    void slotSliderMoved( int value );
    void slotSliderReleased( int value );

private:
    void updateSliderPosition( double value );
    void writeValueToField( double value );

    double stepSize() const;
    int    convertToSliderValue( double value ) const;
    double convertFromSliderValue( int sliderValue ) const;

private:
    Wt::Core::observing_ptr<Wt::WLabel>         m_label;
    Wt::Core::observing_ptr<Wt::WDoubleSpinBox> m_spinBox;
    Wt::Core::observing_ptr<Wt::WSlider>        m_slider;

    double m_sliderValue;

    PdmUiDoubleSliderEditorAttribute m_attributes;
};

} // end namespace caf
