//##################################################################################################
//
//   Caffa Web Interface
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

#include "cafPdmWebCheckBoxTristateEditor.h"

#include "cafPdmWebDefaultObjectEditor.h"

#include "cafPdmObject.h"
#include "cafPdmWebFieldEditorHandle.h"
#include "cafPdmUiOrdering.h"
#include "cafPdmField.h"

#include "cafFactory.h"
#include "cafTristate.h"



namespace caf
{

CAF_PDM_WEB_FIELD_EDITOR_SOURCE_INIT(PdmWebCheckBoxTristateEditor);


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmWebCheckBoxTristateEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    CAF_ASSERT(m_checkBox);
    CAF_ASSERT(m_label);

    applyTextToLabel(m_label.get(), uiConfigName);

    m_checkBox->setEnabled(!uiField()->isUiReadOnly(uiConfigName));
    m_checkBox->setToolTip(uiField()->uiToolTip(uiConfigName).toStdString());

    Tristate state = uiField()->uiValue().value<Tristate>();
    
    if (state == Tristate::State::True)
    {
        m_checkBox->setCheckState(Wt::CheckState::Checked);
    }
    else if (state == Tristate::State::PartiallyTrue)
    {
        m_checkBox->setCheckState(Wt::CheckState::PartiallyChecked);
    }
    else if (state == Tristate::State::False)
    {
        m_checkBox->setCheckState(Wt::CheckState::Unchecked);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Wt::WWidget* PdmWebCheckBoxTristateEditor::createEditorWidget()
{
    m_checkBox = new Wt::WCheckBox;
    m_checkBox->setTristate(true);
    m_checkBox->changed().connect(std::bind(&PdmWebCheckBoxTristateEditor::slotClicked, this));

    return m_checkBox.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Wt::WLabel* PdmWebCheckBoxTristateEditor::createLabelWidget()
{
    m_label = new Wt::WLabel;
    return m_label.get();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmWebCheckBoxTristateEditor::slotClicked()
{
    Tristate state;

    if (m_checkBox->checkState() == Wt::CheckState::Checked)
    {
        state = Tristate::State::True;
    }
    else if (m_checkBox->checkState() == Wt::CheckState::PartiallyChecked)
    {
        state = Tristate::State::PartiallyTrue;
    }
    else if (m_checkBox->checkState() == Wt::CheckState::Unchecked)
    {
        state = Tristate::State::False;
    }

    QVariant v = QVariant::fromValue(state);

    this->setValueToField(v);
}


} // end namespace caf
