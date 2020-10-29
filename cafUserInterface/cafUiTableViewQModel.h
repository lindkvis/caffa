//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2014 Ceetron Solutions AS
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
#include "cafUiTreeOrdering.h"
#include "cafUserInterface_export.h"

#include "cafFieldUiCapability.h"
#include "cafUiTableViewEditor.h"

#include <QAbstractItemModel>
#include <QItemSelection>
#include <QPushButton>

namespace caf
{
class ChildArrayFieldHandle;
class ObjectHandle;
class UiFieldEditorHandle;
class UiItem;
class PdmUiTableRowEditor;
class PdmUiTreeOrdering;
class PdmUiTreeViewEditor;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class cafUserInterface_EXPORT TableViewPushButton : public QPushButton
{
    Q_OBJECT
public:
    explicit TableViewPushButton( caf::FieldUiCapability* field, const QString& text, QWidget* parent = nullptr );

private slots:
    void slotPressed();

private:
    caf::FieldUiCapability* m_fieldHandle;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class PdmUiTableViewQModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit PdmUiTableViewQModel( QWidget* parent );

    QItemSelection   modelIndexFromObject( ObjectHandle* pdmObject );
    FieldHandle*  getField( const QModelIndex& index ) const;
    void             setArrayFieldAndBuildEditors( ChildArrayFieldHandle* pdmObject, const QString& configName );
    ObjectHandle* pdmObjectForRow( int row ) const;

    // Qt overrides
    int      rowCount( const QModelIndex& parent = QModelIndex() ) const override;
    int      columnCount( const QModelIndex& parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;

    Qt::ItemFlags flags( const QModelIndex& index ) const override;
    bool          setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole ) override;

    void notifyDataChanged( const QModelIndex& topLeft, const QModelIndex& bottomRight );

    bool isRepresentingBoolean( const QModelIndex& index ) const;

    void createPersistentPushButtonWidgets( QTableView* tableView );

private:
    int                   getFieldIndex( FieldHandle* field ) const;
    void                  recreateTableItemEditors();
    FieldUiCapability* getUiFieldHandle( const QModelIndex& index ) const;

    friend class PdmUiTableViewDelegate;
    QWidget*                getEditorWidgetAndTransferOwnership( QWidget* parent, const QModelIndex& index );
    UiFieldEditorHandle* getEditor( const QModelIndex& index );

    ChildArrayFieldHandle* childArrayFieldHandle() const;

private:
    // Required to have a PdmPointer to the owner object. Used to guard access to a field inside this object
    PdmPointer<ObjectHandle> m_ownerObject;
    ChildArrayFieldHandle*   m_pdmList;
    QString                     m_currentConfigName;

    std::map<QString, UiFieldEditorHandle*> m_fieldEditors;
    std::vector<int>                           m_modelColumnIndexToFieldIndex;

    std::vector<PdmUiTableRowEditor*> m_tableRowEditors;

    PdmUiTableViewPushButtonEditorAttribute m_pushButtonEditorAttributes;
};

} // End of namespace caf
