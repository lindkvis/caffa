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
#include "cafUiWidgetObjectEditorHandle.h"

#include <QAbstractItemModel>
#include <QPointer>
#include <QWidget>

class QTableView;

namespace caffa
{
class UiFieldEditorHandle;
class UiItem;
class ObjectCollection;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class UiListViewEditorAttribute : public UiEditorAttribute
{
public:
    UiListViewEditorAttribute() {}

public:
    std::vector<std::string> fieldNames;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class UiListViewModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit UiListViewModel( QObject* parent );

    void setData( ObjectCollection* objectGroup );

    // Qt overrides
    int      rowCount( const QModelIndex& parent = QModelIndex() ) const override;
    int      columnCount( const QModelIndex& parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;

private:
    void computeColumnCount();

private:
    ObjectCollection*         m_objectGroup;
    UiListViewEditorAttribute m_editorAttribute;
    int                       m_columnCount;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class UiListViewEditor : public UiWidgetObjectEditorHandle
{
public:
    UiListViewEditor();
    ~UiListViewEditor() override;

protected:
    QWidget* createWidget( QWidget* parent ) override;
    void     configureAndUpdateUi() override;

private:
    QPointer<QTableView> m_tableView;
    UiListViewModel*     m_tableModel;
};

} // end namespace caffa
