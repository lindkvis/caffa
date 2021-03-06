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
#include "cafUiTreeItem.h"

#include <QAbstractItemModel>
#include <QStringList>

#include <memory>

namespace caffa
{
class ObjectHandle;
class UiItem;
class UiTreeViewEditor;
class UiTreeOrdering;
class UiDragDropInterface;

//==================================================================================================
//
// This class is intended to replace UiTreeModel (cafUiTreeModel)
//
//==================================================================================================
class UiTreeViewQModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit UiTreeViewQModel( UiTreeViewEditor* treeViewEditor );

    void setItemRoot( UiItem* rootItem );
    void updateSubTree( UiItem* subTreeRoot );

    void setColumnHeaders( const QStringList& columnHeaders );

    // These are supposed to be used from the Editor only, and to implement selection support.

    UiItem*     uiItemFromModelIndex( const QModelIndex& index ) const;
    QModelIndex findModelIndex( const UiItem* object ) const;

    void                 setDragDropInterface( UiDragDropInterface* dragDropInterface );
    UiDragDropInterface* dragDropInterface();

    std::list<QModelIndex> allIndicesRecursive( const QModelIndex& current = QModelIndex() ) const;

private:
    void updateSubTreeRecursive( const QModelIndex& uiSubTreeRootModelIdx,
                                 UiTreeOrdering*    uiModelSubTreeRoot,
                                 UiTreeOrdering*    updatedSubTreeRoot );

    UiTreeOrdering* treeItemFromIndex( const QModelIndex& index ) const;
    QModelIndex     findModelIndexRecursive( const QModelIndex& currentIndex, const UiItem* object ) const;

    void resetTree( UiTreeOrdering* root );
    void emitDataChanged( const QModelIndex& index );
    void updateEditorsForSubTree( UiTreeOrdering* root );

    std::unique_ptr<UiTreeOrdering> m_treeOrderingRoot;
    QStringList                     m_columnHeaders;

    UiTreeViewEditor* m_treeViewEditor;

    UiDragDropInterface* m_dragDropInterface;

private:
    // Overrides from QAbstractItemModel

    QModelIndex index( int row, int column, const QModelIndex& parentIndex = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex& index ) const override;

    int rowCount( const QModelIndex& parentIndex = QModelIndex() ) const override;
    int columnCount( const QModelIndex& parentIndex = QModelIndex() ) const override;

    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const override;
    bool     setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole ) override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;

    Qt::ItemFlags flags( const QModelIndex& index ) const override;

    QStringList mimeTypes() const override;
    QMimeData*  mimeData( const QModelIndexList& indexes ) const override;
    bool dropMimeData( const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent ) override;
    Qt::DropActions supportedDropActions() const override;
};

} // End of namespace caffa
