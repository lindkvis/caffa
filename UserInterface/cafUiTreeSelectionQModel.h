//##################################################################################################
//
//   Custom Visualization Core library
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
#include "cafUiTreeItem.h"

#include <QAbstractItemModel>

#include <map>
#include <vector>

namespace caffa
{
class OptionItemInfo;
class FieldUiCapability;

//==================================================================================================
///
//==================================================================================================
class UiTreeSelectionQModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit UiTreeSelectionQModel( QObject* parent = nullptr );
    ~UiTreeSelectionQModel() override;

    static int headingRole();
    static int optionItemValueRole();

    void setCheckedStateForItems( const QModelIndexList& indices, bool checked );
    void enableSingleSelectionMode( bool enable );

    size_t optionItemCount() const;
    void   setOptions( caffa::UiFieldEditorHandle* field, const std::deque<OptionItemInfo>& options );
    void   setUiValueCache( const Variant& uiValuesCache );
    void   resetUiValueCache();
    bool   isReadOnly( const QModelIndex& index ) const;
    bool   isChecked( const QModelIndex& index ) const;

    bool hasGrandChildren() const;

    Qt::ItemFlags flags( const QModelIndex& index ) const override;
    QModelIndex   index( int row, int column, const QModelIndex& parent = QModelIndex() ) const override;
    int           columnCount( const QModelIndex& parent = QModelIndex() ) const override;
    QModelIndex   parent( const QModelIndex& child ) const override;
    int           rowCount( const QModelIndex& parent = QModelIndex() ) const override;
    QVariant      data( const QModelIndex& index, int role = Qt::DisplayRole ) const override;
    bool          setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole ) override;

    QModelIndex indexForLastUncheckedItem() const;
    void        clearIndexForLastUncheckedItem();

private:
    typedef caffa::UiTreeItem<int> TreeItemType;

    const caffa::OptionItemInfo* optionItem( const QModelIndex& index ) const;
    int                        optionIndex( const QModelIndex& index ) const;
    void                       buildOptionItemTree( int optionIndex, TreeItemType* parentNode );

    void notifyChangedForAllModelIndices();
    void recursiveNotifyChildren( const QModelIndex& index );

private:
    std::deque<OptionItemInfo>    m_options;
    QPointer<UiFieldEditorHandle> m_uiFieldHandle;

    Variant m_uiValueCache;

    TreeItemType* m_tree;

    bool        m_singleSelectionMode;
    QModelIndex m_indexForLastUncheckedItem;
};

} // end namespace caffa
