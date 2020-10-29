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


#include "cafPdmWebLineEditor.h"

#include "cafFactory.h"
#include "cafField.h"
#include "cafObject.h"
#include "cafPdmWebDefaultObjectEditor.h"
#include "cafPdmWebFieldEditorHandle.h"
#include "cafPdmUiOrdering.h"
#include "cafSelectionManager.h"

#include <Wt/WApplication.h>
#include <Wt/WLabel.h>
#include <Wt/WLineEdit.h>
#include <Wt/WSignal.h>
#include <Wt/WText.h>

//#include <QDebug>
//#include <QString>


using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Wt::WWidget* PdmWebLineEditor::createEditorWidget()
{
    m_lineEdit = new Wt::WLineEdit;
    m_lineEdit->changed().connect(std::bind(&PdmWebLineEditor::slotEditingFinished, this));

    return m_lineEdit.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Wt::WLabel* caf::PdmWebLineEditor::createLabelWidget()
{
    m_label = new Wt::WLabel;
    return m_label.get();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmWebLineEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    applyTextToLabel(m_label.get(), uiConfigName);

    if (m_lineEdit)
    {
        bool isReadOnly = uiField()->isUiReadOnly(uiConfigName);
        if (isReadOnly)
        {
            m_lineEdit->setReadOnly(true);
        }
        else
        {
            m_lineEdit->setReadOnly(false);
        }

        m_lineEdit->setToolTip(uiField()->uiToolTip(uiConfigName).toStdString());

        QString displayString = uiField()->uiValue().toString();
        m_lineEdit->setText(displayString.toStdString());
    }
}

// Define at this location to avoid duplicate symbol definitions in 'cafPdmUiDefaultObjectEditor.cpp' in a cotire build. The
// variables defined by the macro are prefixed by line numbers causing a crash if the macro is defined at the same line number.
CAF_PDM_WEB_FIELD_EDITOR_SOURCE_INIT(PdmWebLineEditor);

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::PdmWebLineEditor::slotEditingFinished()
{
    QString textValue = QString::fromStdString(m_lineEdit->text().narrow());
    QVariant v = textValue;
    this->setValueToField(v);
}
