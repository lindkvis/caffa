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
#include "cafPdmWebComboBoxEditor.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmWebFieldEditorHandle.h"

#include "cafFactory.h"

#include <Wt/WComboBox.h>
#include <Wt/WLabel.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WPushButton.h>

#include <memory>

using namespace std::placeholders;

namespace caf
{

CAF_PDM_WEB_FIELD_EDITOR_SOURCE_INIT(PdmWebComboBoxEditor);

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmWebComboBoxEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    applyTextToLabel(m_label.get(), uiConfigName);

    // Handle attributes
    caf::PdmUiObjectHandle* uiObject = uiObj(uiField()->fieldHandle()->ownerObject());
    if (uiObject)
    {
        uiObject->editorAttribute(uiField()->fieldHandle(), uiConfigName, &m_attributes);
    }

    if (m_comboBox)
    {
        m_comboBox->setEnabled(!uiField()->isUiReadOnly(uiConfigName));
        m_comboBox->setToolTip(uiField()->uiToolTip(uiConfigName).toStdString());


        bool fromMenuOnly = true;
        QList<PdmOptionItemInfo> options = uiField()->valueOptions(&fromMenuOnly);
        CAF_ASSERT(fromMenuOnly); // Not supported

        m_comboBox->clear();

        if (!options.isEmpty())
        {
            for (const auto& option : options)
            {
                m_comboBox->addItem(option.optionUiText().toStdString());
            }
            m_comboBox->setCurrentIndex(uiField()->uiValue().toInt());
        }
        else
        {
            m_comboBox->addItem(uiField()->uiValue().toString().toStdString());
            m_comboBox->setCurrentIndex(0);
        }        
    }

    if (m_container)
    {
        if (m_attributes.showPreviousAndNextButtons)
        {
            if (!m_previousItemButton)
            {
                m_previousItemButton = new Wt::WPushButton;
                m_previousItemButton->changed().connect(std::bind(&PdmWebComboBoxEditor::slotPreviousButtonPressed, this));
                m_previousItemButton->setToolTip("Previous");
            }

            if (!m_nextItemButton)
            {
                m_previousItemButton = new Wt::WPushButton;
                m_previousItemButton->clicked().connect(std::bind(&PdmWebComboBoxEditor::slotNextButtonPressed, this));
                m_nextItemButton->setToolTip("Next");
            }

            m_container->addWidget(std::unique_ptr<Wt::WPushButton>(m_previousItemButton.get()));
            m_container->addWidget(std::unique_ptr<Wt::WPushButton>(m_nextItemButton.get()));

            m_previousItemButton->setDisabled(m_comboBox->count() == 0 || m_comboBox->currentIndex() <= 0);
            m_nextItemButton->setDisabled(m_comboBox->count() == 0 || m_comboBox->currentIndex() >= m_comboBox->count() - 1);

            // Update button texts
            if (!m_attributes.nextButtonText.isEmpty())
            {
                m_nextItemButton->setToolTip(m_attributes.nextButtonText.toStdString());
            }

            if (!m_attributes.prevButtonText.isEmpty())
            {
                m_previousItemButton->setToolTip(m_attributes.prevButtonText.toStdString());
            }
        }
        else
        {
            if (m_previousItemButton)
            {
                m_container->removeWidget(m_previousItemButton.get());
            }

            if (m_nextItemButton)
            {
                m_container->removeWidget(m_nextItemButton.get());
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Wt::WWidget* PdmWebComboBoxEditor::createEditorWidget()
{
    m_comboBox = new Wt::WComboBox;
    m_comboBox->activated().connect(std::bind(&PdmWebComboBoxEditor::slotIndexActivated, this, _1));
    //m_comboBox->setMinimumSize(Wt::WLength(10, Wt::LengthUnit::FontEx), Wt::WLength::Auto);
    m_container = new Wt::WContainerWidget;
    m_container->addWidget(std::unique_ptr<Wt::WComboBox>(m_comboBox.get()));
    //m_container->setMinimumSize(Wt::WLength(10, Wt::LengthUnit::FontEx), Wt::WLength::Auto);
    return m_container.get();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Wt::WLabel* PdmWebComboBoxEditor::createLabelWidget()
{
    m_label = new Wt::WLabel;
    return m_label.get();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmWebComboBoxEditor::slotIndexActivated(int index)
{
    if (m_attributes.enableEditableContent)
    {
        // Use the text directly, as the item text could be entered directly by the user

        auto text = m_comboBox->itemText(index);
        this->setValueToField(QString::fromStdString(text.narrow()));
    }
    else
    {
        // Use index as data carrier to PDM field
        // The index will be used as a lookup in a list of option items

        QVariant v;
        v = index;

        QVariant uintValue(v.toUInt());
        this->setValueToField(uintValue);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmWebComboBoxEditor::slotNextButtonPressed()
{
    int indexCandidate = m_comboBox->currentIndex() + 1;

    if (indexCandidate < m_comboBox->count())
    {
        slotIndexActivated(indexCandidate);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmWebComboBoxEditor::slotPreviousButtonPressed()
{
    int indexCandidate = m_comboBox->currentIndex() - 1;

    if (indexCandidate > -1 && indexCandidate < m_comboBox->count())
    {
        slotIndexActivated(indexCandidate);
    }
}

} // end namespace caf
