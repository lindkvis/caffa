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

#pragma once
#include "cafUiFieldEditorHandle.h"

#include <QDoubleValidator>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPointer>
#include <QString>
#include <QWidget>

namespace caffa
{
//==================================================================================================
///
//==================================================================================================
class UiDoubleValueEditorAttribute : public UiEditorAttribute
{
public:
    enum class NumberFormat
    {
        FIXED,
        SCIENTIFIC,
        AUTOMATIC
    };
    UiDoubleValueEditorAttribute()
    {
        m_decimals     = 6;
        m_numberFormat = NumberFormat::AUTOMATIC;
    }

public:
    int                        m_decimals;
    NumberFormat               m_numberFormat;
    QPointer<QDoubleValidator> m_validator;
};

//==================================================================================================
///
//==================================================================================================
class UiDoubleValueEditor : public UiFieldEditorHandle
{
    Q_OBJECT
    CAFFA_UI_FIELD_EDITOR_HEADER_INIT;

public:
    UiDoubleValueEditor();
    ~UiDoubleValueEditor() override;

protected:
    void     configureAndUpdateUi() override;
    QWidget* createEditorWidget( QWidget* parent ) override;
    QWidget* createLabelWidget( QWidget* parent ) override;

protected slots:
    void slotEditingFinished();

private:
    void writeValueToField();

private:
    QPointer<QLineEdit> m_lineEdit;
    QPointer<QLabel>    m_label;

    UiDoubleValueEditorAttribute m_attributes;
};

} // end namespace caffa
