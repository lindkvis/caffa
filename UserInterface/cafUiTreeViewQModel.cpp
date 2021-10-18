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

#include "cafUiTreeViewQModel.h"

#include "cafField.h"
#include "cafObject.h"
#include "cafObjectUiCapability.h"
#include "cafQVariantConverter.h"
#include "cafUiCommandSystemProxy.h"
#include "cafUiDragDropInterface.h"
#include "cafUiTreeItemEditor.h"
#include "cafUiTreeOrdering.h"
#include "cafUiTreeViewEditor.h"

#include <QDragMoveEvent>
#include <QTreeView>

namespace caffa
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiTreeViewQModel::UiTreeViewQModel( UiTreeViewEditor* treeViewEditor )
{
    m_treeOrderingRoot  = nullptr;
    m_dragDropInterface = nullptr;

    m_treeViewEditor = treeViewEditor;
}

//--------------------------------------------------------------------------------------------------
/// Will populate the tree with the contents of the Pdm data structure rooted at rootItem.
/// Will not show the rootItem itself, only the children and downwards
//--------------------------------------------------------------------------------------------------
void UiTreeViewQModel::setItemRoot( UiItem* rootItem )
{
    // Check if we are already watching this root
    if ( rootItem && m_treeOrderingRoot && m_treeOrderingRoot->activeItem() == rootItem )
    {
        this->updateSubTree( rootItem );
        return;
    }

    UiTreeOrdering*    newRoot = nullptr;
    FieldUiCapability* field   = dynamic_cast<FieldUiCapability*>( rootItem );

    if ( field )
    {
        newRoot = new UiTreeOrdering( field->fieldHandle() );
        ObjectUiCapability::expandUiTree( newRoot );
    }
    else
    {
        ObjectUiCapability* obj = dynamic_cast<ObjectUiCapability*>( rootItem );
        if ( obj )
        {
            newRoot = obj->uiTreeOrdering();
        }
    }

    CAFFA_ASSERT( newRoot || rootItem == nullptr ); // Only fields, objects or nullptr is allowed.

    // if (newRoot) newRoot->debugDump(0);

    this->resetTree( newRoot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTreeViewQModel::resetTree( UiTreeOrdering* newRoot )
{
    beginResetModel();

    m_treeOrderingRoot = std::unique_ptr<UiTreeOrdering>( newRoot );

    updateEditorsForSubTree( m_treeOrderingRoot.get() );

    endResetModel();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTreeViewQModel::setColumnHeaders( const QStringList& columnHeaders )
{
    m_columnHeaders = columnHeaders;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTreeViewQModel::emitDataChanged( const QModelIndex& index )
{
    emit dataChanged( index, index );
}

//--------------------------------------------------------------------------------------------------
/// Refreshes the UI-tree below the supplied root UiItem
//--------------------------------------------------------------------------------------------------
void UiTreeViewQModel::updateSubTree( UiItem* root )
{
    // Build the new "Correct" Tree

    UiTreeOrdering*    newTreeRootTmp = nullptr;
    FieldUiCapability* field          = dynamic_cast<FieldUiCapability*>( root );
    if ( field )
    {
        newTreeRootTmp = new UiTreeOrdering( field->fieldHandle() );
    }
    else
    {
        ObjectUiCapability* obj = dynamic_cast<ObjectUiCapability*>( root );
        if ( obj )
        {
            newTreeRootTmp = new UiTreeOrdering( obj->objectHandle() );
        }
    }

    ObjectUiCapability::expandUiTree( newTreeRootTmp );

#if CAFFA_TREE_VIEW_DEBUG_PRINT
    std::cout << std::endl << "New Stuff: " << std::endl;
    newTreeRootTmp->debugDump( 0 );
#endif

    // Find the corresponding entry for "root" in the existing Ui tree

    QModelIndex existingSubTreeRootModIdx = findModelIndex( root );

    UiTreeOrdering* existingSubTreeRoot = nullptr;
    if ( existingSubTreeRootModIdx.isValid() )
    {
        existingSubTreeRoot = treeItemFromIndex( existingSubTreeRootModIdx );
    }
    else
    {
        existingSubTreeRoot = m_treeOrderingRoot.get();
    }

#if CAFFA_TREE_VIEW_DEBUG_PRINT
    std::cout << std::endl << "Old :" << std::endl;
    existingSubTreeRoot->debugDump( 0 );
#endif

    updateSubTreeRecursive( existingSubTreeRootModIdx, existingSubTreeRoot, newTreeRootTmp );

    delete newTreeRootTmp;

    updateEditorsForSubTree( existingSubTreeRoot );

#if CAFFA_TREE_VIEW_DEBUG_PRINT
    std::cout << std::endl << "Result :" << std::endl;
    existingSubTreeRoot->debugDump( 0 );
#endif
}

class RecursiveUpdateData
{
public:
    RecursiveUpdateData( int row, UiTreeOrdering* existingChild, UiTreeOrdering* sourceChild )
        : m_row( row )
        , m_existingChild( existingChild )
        , m_sourceChild( sourceChild ){};

    int             m_row;
    UiTreeOrdering* m_existingChild;
    UiTreeOrdering* m_sourceChild;
};

//--------------------------------------------------------------------------------------------------
/// Makes the existingSubTreeRoot tree become identical to the tree in sourceSubTreeRoot,
/// calling begin..() end..() to make the UI update accordingly.
/// This assumes that all the items have a pointer an unique Object
//--------------------------------------------------------------------------------------------------
void UiTreeViewQModel::updateSubTreeRecursive( const QModelIndex& existingSubTreeRootModIdx,
                                               UiTreeOrdering*    existingSubTreeRoot,
                                               UiTreeOrdering*    sourceSubTreeRoot )
{
    // Build map for source items
    std::map<caffa::UiItem*, int> sourceTreeMap;
    for ( int i = 0; i < sourceSubTreeRoot->childCount(); ++i )
    {
        UiTreeOrdering* child = sourceSubTreeRoot->child( i );

        if ( child && child->activeItem() )
        {
            sourceTreeMap[child->activeItem()] = i;
        }
    }

    // Detect items to be deleted from existing tree
    std::vector<int> indicesToRemoveFromExisting;
    for ( int i = 0; i < existingSubTreeRoot->childCount(); ++i )
    {
        UiTreeOrdering* child = existingSubTreeRoot->child( i );

        std::map<caffa::UiItem*, int>::iterator it = sourceTreeMap.find( child->activeItem() );
        if ( it == sourceTreeMap.end() )
        {
            indicesToRemoveFromExisting.push_back( i );
        }
    }

    // Delete items with largest index first from existing
    for ( std::vector<int>::reverse_iterator it = indicesToRemoveFromExisting.rbegin();
          it != indicesToRemoveFromExisting.rend();
          ++it )
    {
        this->beginRemoveRows( existingSubTreeRootModIdx, *it, *it );
        existingSubTreeRoot->removeChildren( *it, 1 );
        this->endRemoveRows();
    }

    // Build map for existing items without the deleted items
    std::map<caffa::UiItem*, int> existingTreeMap;
    for ( int i = 0; i < existingSubTreeRoot->childCount(); ++i )
    {
        UiTreeOrdering* child = existingSubTreeRoot->child( i );

        if ( child && child->activeItem() )
        {
            existingTreeMap[child->activeItem()] = i;
        }
    }

    // Check if there are any changes between existing and source
    // If no changes, update the subtree recursively
    bool anyChanges = false;
    if ( existingSubTreeRoot->childCount() == sourceSubTreeRoot->childCount() )
    {
        for ( int i = 0; i < existingSubTreeRoot->childCount(); ++i )
        {
            if ( existingSubTreeRoot->child( i )->activeItem() != sourceSubTreeRoot->child( i )->activeItem() )
            {
                anyChanges = true;
                break;
            }
        }
    }
    else
    {
        anyChanges = true;
    }

    if ( !anyChanges )
    {
        // Notify Qt that the toggle/name/icon etc might have been changed
        emitDataChanged( existingSubTreeRootModIdx );

        // No changes to list of children at this level, call update on all children
        for ( int i = 0; i < existingSubTreeRoot->childCount(); ++i )
        {
            updateSubTreeRecursive( index( i, 0, existingSubTreeRootModIdx ),
                                    existingSubTreeRoot->child( i ),
                                    sourceSubTreeRoot->child( i ) );
        }
    }
    else
    {
        std::vector<RecursiveUpdateData> recursiveUpdateData;
        std::vector<UiTreeOrdering*>     newMergedOrdering;

        emit layoutAboutToBeChanged();
        {
            // Detect items to be moved from source to existing
            // Merge items from existing and source into newMergedOrdering using order in sourceSubTreeRoot
            std::vector<int> indicesToRemoveFromSource;
            for ( int i = 0; i < sourceSubTreeRoot->childCount(); ++i )
            {
                UiTreeOrdering*                         sourceChild = sourceSubTreeRoot->child( i );
                std::map<caffa::UiItem*, int>::iterator it          = existingTreeMap.find( sourceChild->activeItem() );
                if ( it != existingTreeMap.end() )
                {
                    newMergedOrdering.push_back( existingSubTreeRoot->child( it->second ) );

                    int rowIndexToBeUpdated = static_cast<int>( newMergedOrdering.size() - 1 );
                    recursiveUpdateData.push_back(
                        RecursiveUpdateData( rowIndexToBeUpdated, existingSubTreeRoot->child( it->second ), sourceChild ) );
                }
                else
                {
                    newMergedOrdering.push_back( sourceChild );

                    indicesToRemoveFromSource.push_back( i );
                }
            }

            // Delete new items from source because they have been moved into newMergedOrdering
            for ( std::vector<int>::reverse_iterator it = indicesToRemoveFromSource.rbegin();
                  it != indicesToRemoveFromSource.rend();
                  ++it )
            {
                // Use the removeChildrenNoDelete() to remove the pointer from the list without deleting the pointer
                sourceSubTreeRoot->removeChildrenNoDelete( *it, 1 );
            }

            // Delete all items from existingSubTreeRoot, as the complete list is present in newMergedOrdering
            existingSubTreeRoot->removeChildrenNoDelete( 0, (int)existingSubTreeRoot->childCount() );

            // First, reorder all items in existing tree, as this operation is valid when later emitting the signal
            // layoutChanged() Insert of new items before issuing this signal causes the tree items below the inserted
            // item to collapse
            for ( size_t i = 0; i < newMergedOrdering.size(); i++ )
            {
                if ( existingTreeMap.find( newMergedOrdering[i]->activeItem() ) != existingTreeMap.end() )
                {
                    existingSubTreeRoot->appendChild( newMergedOrdering[i] );
                }
            }
        }

        emit layoutChanged();

        // Insert new items into existingSubTreeRoot
        for ( size_t i = 0; i < newMergedOrdering.size(); i++ )
        {
            if ( existingTreeMap.find( newMergedOrdering[i]->activeItem() ) == existingTreeMap.end() )
            {
                this->beginInsertRows( existingSubTreeRootModIdx, static_cast<int>( i ), static_cast<int>( i ) );
                existingSubTreeRoot->insertChild( static_cast<int>( i ), newMergedOrdering[i] );
                this->endInsertRows();
            }
        }

        for ( size_t i = 0; i < recursiveUpdateData.size(); i++ )
        {
            // Using the index() function here is OK, as new items has been inserted in the previous for-loop
            // This code used to be executed before insertion of new items, and caused creation of invalid indices
            QModelIndex mi = index( recursiveUpdateData[i].m_row, 0, existingSubTreeRootModIdx );
            CAFFA_ASSERT( mi.isValid() );

            updateSubTreeRecursive( mi, recursiveUpdateData[i].m_existingChild, recursiveUpdateData[i].m_sourceChild );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTreeViewQModel::updateEditorsForSubTree( UiTreeOrdering* root )
{
    if ( !root ) return;

    if ( !root->editor() )
    {
        auto treeItemEditor = std::make_unique<UiTreeItemEditor>( root->activeItem() );
        root->setEditor( std::move( treeItemEditor ) );
        CAFFA_ASSERT( root->editor() );
    }

    UiTreeItemEditor* treeItemEditor = dynamic_cast<UiTreeItemEditor*>( root->editor() );
    if ( treeItemEditor )
    {
        treeItemEditor->setTreeViewEditor( m_treeViewEditor );
    }

    for ( int i = 0; i < root->childCount(); ++i )
    {
        updateEditorsForSubTree( root->child( i ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::list<QModelIndex> UiTreeViewQModel::allIndicesRecursive( const QModelIndex& current ) const
{
    std::list<QModelIndex> currentAndDescendants;
    currentAndDescendants.push_back( current );

    int rows = rowCount( current );
    int cols = columnCount( current );
    for ( int row = 0; row < rows; ++row )
    {
        for ( int col = 0; col < cols; ++col )
        {
            QModelIndex            childIndex = index( row, col, current );
            std::list<QModelIndex> subList    = allIndicesRecursive( childIndex );
            currentAndDescendants.insert( currentAndDescendants.end(), subList.begin(), subList.end() );
        }
    }
    return currentAndDescendants;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiTreeOrdering* caffa::UiTreeViewQModel::treeItemFromIndex( const QModelIndex& index ) const
{
    if ( !index.isValid() )
    {
        return m_treeOrderingRoot.get();
    }

    CAFFA_ASSERT( index.internalPointer() );

    UiTreeOrdering* treeItem = static_cast<UiTreeOrdering*>( index.internalPointer() );

    return treeItem;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QModelIndex caffa::UiTreeViewQModel::findModelIndex( const UiItem* object ) const
{
    QModelIndex foundIndex;
    int         numRows = rowCount( QModelIndex() );
    int         r       = 0;
    while ( r < numRows && !foundIndex.isValid() )
    {
        foundIndex = findModelIndexRecursive( index( r, 0, QModelIndex() ), object );
        ++r;
    }
    return foundIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QModelIndex caffa::UiTreeViewQModel::findModelIndexRecursive( const QModelIndex& currentIndex, const UiItem* item ) const
{
    if ( !currentIndex.isValid() )
    {
        UiTreeOrdering* treeItem = m_treeOrderingRoot.get();
        if ( treeItem->activeItem() == item ) return currentIndex;
    }
    else if ( currentIndex.internalPointer() )
    {
        UiTreeOrdering* treeItem = static_cast<UiTreeOrdering*>( currentIndex.internalPointer() );
        if ( treeItem->activeItem() == item ) return currentIndex;
    }

    int row;
    for ( row = 0; row < rowCount( currentIndex ); ++row )
    {
        QModelIndex foundIndex = findModelIndexRecursive( index( row, 0, currentIndex ), item );
        if ( foundIndex.isValid() ) return foundIndex;
    }
    return QModelIndex();
}

//--------------------------------------------------------------------------------------------------
/// An invalid parent index is implicitly meaning the root item, and not "above" root, since
/// we are not showing the root item itself
//--------------------------------------------------------------------------------------------------
QModelIndex UiTreeViewQModel::index( int row, int column, const QModelIndex& parentIndex ) const
{
    if ( !m_treeOrderingRoot ) return QModelIndex();

    UiTreeOrdering* parentItem = nullptr;

    if ( !parentIndex.isValid() )
        parentItem = m_treeOrderingRoot.get();
    else
        parentItem = static_cast<UiTreeOrdering*>( parentIndex.internalPointer() );

    CAFFA_ASSERT( parentItem );

    if ( parentItem->childCount() <= row )
    {
        return QModelIndex();
    }

    UiTreeOrdering* childItem = parentItem->child( row );
    if ( childItem )
        return createIndex( row, column, childItem );
    else
        return QModelIndex();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QModelIndex UiTreeViewQModel::parent( const QModelIndex& childIndex ) const
{
    if ( !childIndex.isValid() ) return QModelIndex();

    UiTreeOrdering* childItem = static_cast<UiTreeOrdering*>( childIndex.internalPointer() );
    if ( !childItem ) return QModelIndex();

    UiTreeOrdering* parentItem = childItem->parent();
    if ( !parentItem ) return QModelIndex();

    if ( m_treeOrderingRoot.get() == parentItem ) return QModelIndex();

    return createIndex( (int)parentItem->indexInParent(), 0, parentItem );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int UiTreeViewQModel::rowCount( const QModelIndex& parentIndex ) const
{
    if ( !m_treeOrderingRoot ) return 0;

    if ( parentIndex.column() > 0 ) return 0;

    UiTreeOrdering* parentItem = this->treeItemFromIndex( parentIndex );

    return (int)parentItem->childCount();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int UiTreeViewQModel::columnCount( const QModelIndex& parentIndex ) const
{
    if ( !m_treeOrderingRoot ) return 0;

    return 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QVariant UiTreeViewQModel::data( const QModelIndex& index, int role ) const
{
    UiTreeOrdering* uitreeOrdering = nullptr;

    if ( !index.isValid() )
    {
        uitreeOrdering = m_treeOrderingRoot.get();
    }
    else
    {
        uitreeOrdering = static_cast<UiTreeOrdering*>( index.internalPointer() );
    }

    if ( !uitreeOrdering )
    {
        return QVariant();
    }

    bool isObjRep = uitreeOrdering->isRepresentingObject();

    if ( role == Qt::DisplayRole && !uitreeOrdering->isValid() )
    {
        QString str;

#ifdef DEBUG
        str = "Invalid uiordering";
#endif

        return QVariant( str );
    }

    if ( role == Qt::DisplayRole || role == Qt::EditRole )
    {
        if ( isObjRep )
        {
            auto object = uitreeOrdering->object();
            if ( object )
            {
                std::string txt                  = "undefined";
                auto        userDescriptionField = object->userDescriptionField();
                if ( userDescriptionField )
                {
                    caffa::FieldUiCapability* uiFieldHandle = userDescriptionField->capability<FieldUiCapability>();
                    if ( uiFieldHandle )
                    {
                        Variant uiValue = uiFieldHandle->uiValue();
                        txt             = uiValue.value<std::string>();
                    }
                }
                else if ( object->capability<ObjectUiCapability>() )
                {
                    txt = object->capability<ObjectUiCapability>()->uiName();
                }

                if ( m_treeViewEditor->isAppendOfClassNameToUiItemTextEnabled() )
                {
                    txt += " - ";
                    txt += typeid( *object ).name();
                }

                return QString::fromStdString( txt );
            }
            else
            {
                return QVariant();
            }
        }

        if ( uitreeOrdering->activeItem() )
        {
            return QVariant( QString::fromStdString( uitreeOrdering->activeItem()->uiName() ) );
        }
        else
        {
            return QVariant();
        }
    }
    else if ( role == Qt::ToolTipRole )
    {
        if ( uitreeOrdering->activeItem() )
        {
            return QString::fromStdString( uitreeOrdering->activeItem()->uiToolTip() );
        }
        else
        {
            return QVariant();
        }
    }
    else if ( role == Qt::WhatsThisRole )
    {
        if ( uitreeOrdering->activeItem() )
        {
            return QString::fromStdString( uitreeOrdering->activeItem()->uiWhatsThis() );
        }
        else
        {
            return QVariant();
        }
    }
    else if ( role == Qt::CheckStateRole )
    {
        if ( isObjRep )
        {
            auto object = uitreeOrdering->object();
            if ( object && object->objectToggleField() )
            {
                caffa::FieldUiCapability* uiFieldHandle = object->objectToggleField()->capability<FieldUiCapability>();
                if ( uiFieldHandle )
                {
                    bool isToggledOn = uiFieldHandle->uiValue().value<bool>();
                    if ( isToggledOn )
                    {
                        return Qt::Checked;
                    }
                    else
                    {
                        return Qt::Unchecked;
                    }
                }
                else
                {
                    return QVariant();
                }
            }
        }
    }

    return QVariant();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool UiTreeViewQModel::setData( const QModelIndex& index, const QVariant& value, int role /*= Qt::EditRole*/ )
{
    if ( !index.isValid() )
    {
        return false;
    }

    UiTreeOrdering* treeItem = UiTreeViewQModel::treeItemFromIndex( index );
    CAFFA_ASSERT( treeItem );

    if ( !treeItem->isRepresentingObject() ) return false;

    auto object = treeItem->object();
    if ( object )
    {
        if ( role == Qt::EditRole && object->userDescriptionField() )
        {
            FieldUiCapability* userDescriptionUiField = object->userDescriptionField()->capability<FieldUiCapability>();
            if ( userDescriptionUiField )
            {
                UiCommandSystemProxy::instance()->setUiValueToField( userDescriptionUiField, value.value<Variant>() );
            }

            return true;
        }
        else if ( role == Qt::CheckStateRole && object->objectToggleField() &&
                  !object->objectToggleField()->capability<FieldUiCapability>()->isUiReadOnly() )
        {
            bool toggleOn = ( value == Qt::Checked );

            FieldUiCapability* toggleUiField = object->objectToggleField()->capability<FieldUiCapability>();
            if ( toggleUiField )
            {
                UiCommandSystemProxy::instance()->setUiValueToField( toggleUiField, toggleOn );
            }

            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// Enable edit of this item if we have a editable user description field for a object
/// Disable edit for other items
//--------------------------------------------------------------------------------------------------
Qt::ItemFlags UiTreeViewQModel::flags( const QModelIndex& index ) const
{
    if ( !index.isValid() )
    {
        return Qt::ItemIsEnabled;
    }

    Qt::ItemFlags flagMask = QAbstractItemModel::flags( index );

    UiTreeOrdering* treeItem = treeItemFromIndex( index );
    CAFFA_ASSERT( treeItem );

    if ( treeItem->isRepresentingObject() )
    {
        auto object = treeItem->object();
        if ( object )
        {
            if ( object->userDescriptionField() &&
                 !object->userDescriptionField()->capability<FieldUiCapability>()->isUiReadOnly() )
            {
                flagMask = flagMask | Qt::ItemIsEditable;
            }

            if ( object->objectToggleField() )
            {
                flagMask = flagMask | Qt::ItemIsUserCheckable;
            }
        }
    }

    if ( treeItem->isValid() )
    {
        if ( treeItem->activeItem()->isUiReadOnly() )
        {
            flagMask = flagMask & ( ~Qt::ItemIsEnabled );
        }
    }

    if ( m_dragDropInterface )
    {
        Qt::ItemFlags dragDropFlags = m_dragDropInterface->flags( index );
        flagMask |= dragDropFlags;
    }

    return flagMask;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QVariant UiTreeViewQModel::headerData( int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole */ ) const
{
    if ( role != Qt::DisplayRole ) return QVariant();

    if ( section < m_columnHeaders.size() )
    {
        return m_columnHeaders[section];
    }

    return QVariant();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiItem* UiTreeViewQModel::uiItemFromModelIndex( const QModelIndex& index ) const
{
    UiTreeOrdering* treeItem = this->treeItemFromIndex( index );
    if ( treeItem )
    {
        return treeItem->activeItem();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void UiTreeViewQModel::setDragDropInterface( UiDragDropInterface* dragDropInterface )
{
    m_dragDropInterface = dragDropInterface;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiDragDropInterface* UiTreeViewQModel::dragDropInterface()
{
    return m_dragDropInterface;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList UiTreeViewQModel::mimeTypes() const
{
    if ( m_dragDropInterface )
    {
        return m_dragDropInterface->mimeTypes();
    }
    else
    {
        return QAbstractItemModel::mimeTypes();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QMimeData* UiTreeViewQModel::mimeData( const QModelIndexList& indexes ) const
{
    if ( m_dragDropInterface )
    {
        return m_dragDropInterface->mimeData( indexes );
    }
    else
    {
        return QAbstractItemModel::mimeData( indexes );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool UiTreeViewQModel::dropMimeData( const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent )
{
    if ( m_dragDropInterface )
    {
        return m_dragDropInterface->dropMimeData( data, action, row, column, parent );
    }
    else
    {
        return QAbstractItemModel::dropMimeData( data, action, row, column, parent );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Qt::DropActions UiTreeViewQModel::supportedDropActions() const
{
    if ( m_dragDropInterface )
    {
        return m_dragDropInterface->supportedDropActions();
    }
    else
    {
        return QAbstractItemModel::supportedDropActions();
    }
}

} // end namespace caffa
