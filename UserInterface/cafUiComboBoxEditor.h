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
#include "cafUiComboBoxEditorAttribute.h"
#include "cafUiFieldEditorHandle.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPointer>
#include <QString>
#include <QToolButton>
#include <QWidget>

namespace caffa
{
//==================================================================================================
///
//==================================================================================================
class UiComboBoxEditor : public UiFieldEditorHandle
{
    Q_OBJECT
    CAF_UI_FIELD_EDITOR_HEADER_INIT;

public:
    UiComboBoxEditor() {}
    ~UiComboBoxEditor() override {}

protected:
    QWidget* createEditorWidget( QWidget* parent ) override;
    QWidget* createLabelWidget( QWidget* parent ) override;
    void     configureAndUpdateUi() override;
    QMargins calculateLabelContentMargins() const override;

protected slots:
    void slotIndexActivated( int index );

    void slotNextButtonPressed();
    void slotPreviousButtonPressed();

private:
    QPointer<QComboBox> m_comboBox;
    QPointer<QLabel>    m_label;

    QPointer<QToolButton> m_previousItemButton;
    QPointer<QToolButton> m_nextItemButton;
    QPointer<QHBoxLayout> m_layout;
    QPointer<QWidget>     m_placeholder;

    UiComboBoxEditorAttribute m_attributes;
};

} // end namespace caffa
