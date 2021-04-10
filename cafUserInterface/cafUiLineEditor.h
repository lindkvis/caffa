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
#include "cafField.h"
#include "cafUiFieldEditorHandle.h"

#include <QLabel>
#include <QLineEdit>
#include <QPointer>
#include <QString>
#include <QValidator>
#include <QWidget>

class QGridLayout;
class QCompleter;
class QStringListModel;

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class UiLineEditorAttribute : public UiEditorAttribute
{
public:
    UiLineEditorAttribute()
    {
        avoidSendingEnterEventToParentWidget = false;
        completerCaseSensitivity             = Qt::CaseInsensitive;
        completerFilterMode                  = Qt::MatchContains;
        maximumWidth                         = -1;
        selectAllOnFocusEvent                = false;
        placeholderText                      = "";
    }

public:
    bool                 avoidSendingEnterEventToParentWidget;
    QPointer<QValidator> validator;

    // Completer setup
    Qt::CaseSensitivity completerCaseSensitivity;
    Qt::MatchFlags      completerFilterMode;
    int                 maximumWidth;
    bool                selectAllOnFocusEvent;
    std::string         placeholderText;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class UiLineEditorAttributeUiDisplayString : public UiEditorAttribute
{
public:
    UiLineEditorAttributeUiDisplayString() {}

public:
    std::string m_displayString;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class UiLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    UiLineEdit( QWidget* parent );
    void setAvoidSendingEnterEventToParentWidget( bool avoidSendingEnter );

protected:
    void keyPressEvent( QKeyEvent* event ) override;

private:
    bool m_avoidSendingEnterEvent;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class UiLineEditor : public UiFieldEditorHandle
{
    Q_OBJECT
    CAF_UI_FIELD_EDITOR_HEADER_INIT;

public:
    UiLineEditor()
        : m_ignoreCompleterActivated( false )
    {
    }
    ~UiLineEditor() override {}

protected:
    QWidget* createEditorWidget( QWidget* parent ) override;
    QWidget* createLabelWidget( QWidget* parent ) override;
    void     configureAndUpdateUi() override;
    QMargins calculateLabelContentMargins() const override;

    virtual bool eventFilter( QObject* watched, QEvent* event ) override;

protected slots:
    void slotEditingFinished();
    void slotCompleterActivated( const QModelIndex& index );

private:
    bool isMultipleFieldsWithSameKeywordSelected( FieldHandle* editorField ) const;

protected:
    QPointer<UiLineEdit> m_lineEdit;
    QPointer<QLabel>     m_label;

    QPointer<QCompleter>       m_completer;
    QPointer<QStringListModel> m_completerTextList;
    std::deque<OptionItemInfo> m_optionCache;
    bool                       m_ignoreCompleterActivated;

    const OptionItemInfo* findOption( const QString& uiText );
};

} // end namespace caf
