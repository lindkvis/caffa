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
#include "cafUiFieldEditorHandle.h"

class QItemSelection;
class QLabel;
class QListViewHeightHint;
class QModelIndex;
class QStringList;
class QStringListModel;

namespace caffa
{
//==================================================================================================
///
//==================================================================================================
class UiListEditorAttribute : public UiEditorAttribute
{
public:
    UiListEditorAttribute()
        : m_heightHint( 2000 )
        , m_allowHorizontalScrollBar( true )
    {
        QPalette myPalette;

        m_baseColor = myPalette.color( QPalette::Active, QPalette::Base );
    }

public:
    QColor m_baseColor;
    int    m_heightHint;
    bool   m_allowHorizontalScrollBar;
};

//==================================================================================================
///
//==================================================================================================
class UiListEditor : public UiFieldEditorHandle
{
    Q_OBJECT
    CAFFA_UI_FIELD_EDITOR_HEADER_INIT;

public:
    UiListEditor();
    ~UiListEditor() override;

protected:
    QWidget* createEditorWidget( QWidget* parent ) override;
    QWidget* createLabelWidget( QWidget* parent ) override;
    void     configureAndUpdateUi() override;
    bool     eventFilter( QObject* listView, QEvent* event ) override; // To catch delete key press in list view.
    bool     isMultiRowEditor() const override;

protected slots:
    void slotSelectionChanged( const QItemSelection& selected, const QItemSelection& deselected );
    void slotListItemEdited( const QModelIndex&, const QModelIndex& );
    void slotScrollToSelectedItem() const;

private:
    QString contentAsString() const;
    void    pasteFromString( const QString& content );

    void trimAndSetValuesToField( const QStringList& stringList );

private:
    QPointer<QListViewHeightHint> m_listView;
    QPointer<QLabel>              m_label;
    QPointer<QStringListModel>    m_model;

    bool   m_isEditOperationsAvailable;
    size_t m_optionItemCount;
    bool   m_isScrollToItemAllowed;
};

} // end namespace caffa
