//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2019- Ceetron Solutions AS
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

#include <QLabel>
#include <QPointer>
#include <QString>
#include <QTimeEdit>
#include <QWidget>

namespace caf
{
//==================================================================================================
///
//==================================================================================================
class UiTimeEditorAttribute : public UiEditorAttribute
{
public:
    QString timeFormat;

public:
    UiTimeEditorAttribute() {}
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class UiTimeEditor : public UiFieldEditorHandle
{
    Q_OBJECT
    CAF_UI_FIELD_EDITOR_HEADER_INIT;

public:
    UiTimeEditor() {}
    ~UiTimeEditor() override {}

protected:
    QWidget* createEditorWidget( QWidget* parent ) override;
    QWidget* createLabelWidget( QWidget* parent ) override;
    void     configureAndUpdateUi() override;

protected slots:
    void slotEditingFinished();
    void slotTimeChanged( const QTime& time );

private:
    QPointer<QTimeEdit> m_timeEdit;
    QPointer<QLabel>    m_label;

    UiTimeEditorAttribute m_attributes;
};

} // end namespace caf
