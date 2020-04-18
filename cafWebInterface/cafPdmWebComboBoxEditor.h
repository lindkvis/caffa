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

#pragma once

#include "cafPdmUiComboBoxEditorAttribute.h"
#include "cafPdmWebFieldEditorHandle.h"

#include <Wt/Core/observing_ptr.hpp>
#include <Wt/WComboBox.h>
#include <Wt/WLabel.h>
#include <Wt/WPushButton.h>

namespace caf
{
//==================================================================================================
///
//==================================================================================================
class PdmWebComboBoxEditor : public PdmWebFieldEditorHandle
{
    CAF_PDM_WEB_FIELD_EDITOR_HEADER_INIT;

public:
    PdmWebComboBoxEditor() {}
    ~PdmWebComboBoxEditor() override {}

protected:
    Wt::WWidget* createEditorWidget() override;
    Wt::WLabel*  createLabelWidget() override;
    void         configureAndUpdateUi( const QString& uiConfigName ) override;

protected slots:
    void slotIndexActivated( int index );

    void slotNextButtonPressed();
    void slotPreviousButtonPressed();

private:
    Wt::Core::observing_ptr<Wt::WComboBox> m_comboBox;
    Wt::Core::observing_ptr<Wt::WLabel>    m_label;

    Wt::Core::observing_ptr<Wt::WPushButton>      m_previousItemButton;
    Wt::Core::observing_ptr<Wt::WPushButton>      m_nextItemButton;
    Wt::Core::observing_ptr<Wt::WContainerWidget> m_container;

    PdmUiComboBoxEditorAttribute m_attributes;
};

} // end namespace caf
