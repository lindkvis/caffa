//##################################################################################################
//
//   Caffa Web Interface
//   Copyright (C) 2014 Ceetron Solutions AS
//   Copyright (C) Gaute Lindkvist
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
#include "cafPdmWebTreeViewWModel.h"

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 4267 4275 4564 )
#endif

#include "cafField.h"
#include "cafObject.h"
#include "cafPdmWebTreeItemEditor.h"
#include "cafPdmWebTreeViewEditor.h"
#include "cafUiCommandSystemProxy.h"
//#include "cafUiDragDropInterface.h"
#include "cafUiTreeOrdering.h"

#include <Wt/WEvent.h>
#include <Wt/WTreeView.h>

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmWebTreeViewWModel::PdmWebTreeViewWModel( PdmWebTreeViewEditor* treeViewEditor )
{
    m_treeOrderingRoot = nullptr;
    // m_dragDropInterface = nullptr;

    m_treeViewEditor = treeViewEditor;
}

//--------------------------------------------------------------------------------------------------
/// Will populate the tree with the contents of the Pdm data structure rooted at rootItem.
/// Will not show the rootItem itself, only the children and downwards
//--------------------------------------------------------------------------------------------------
void PdmWebTreeViewWModel::setPdmItemRoot( UiItem* rootItem )
{
    // Check if we are already watching this root
    if ( rootItem && m_treeOrderingRoot && m_treeOrderingRoot->activeItem() == rootItem )
    {
        this->updateSubTree( rootItem );
        return;
    }

    PdmUiTreeOrdering* newRoot = nullptr;
    FieldUiCapability* field   = dynamic_cast<FieldUiCapability*>( rootItem );

    if ( field )
    {
        newRoot = new PdmUiTreeOrdering( field->fieldHandle() );
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

    CAF_ASSERT( newRoot || rootItem == nullptr ); // Only fields, objects or NULL is allowed.

    // if (newRoot) newRoot->debugDump(0);

    this->resetTree( newRoot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebTreeViewWModel::resetTree( PdmUiTreeOrdering* newRoot )
{
    if ( m_treeOrderingRoot )
    {
        delete m_treeOrderingRoot;
    }

    m_treeOrderingRoot = newRoot;

    updateEditorsForSubTree( m_treeOrderingRoot );

    modelReset().emit();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebTreeViewWModel::setColumnHeaders( const std::vector<std::string>& columnHeaders )
{
    m_columnHeaders = columnHeaders;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebTreeViewWModel::emitDataChanged( const Wt::WModelIndex& index )
{
    dataChanged().emit( index, index );
}

//--------------------------------------------------------------------------------------------------
/// Refreshes the UI-tree below the supplied root UiItem
//--------------------------------------------------------------------------------------------------
void PdmWebTreeViewWModel::updateSubTree( UiItem* pdmRoot )
{
    // Build the new "Correct" Tree

    PdmUiTreeOrdering* newTreeRootTmp = nullptr;
    FieldUiCapability* field          = dynamic_cast<FieldUiCapability*>( pdmRoot );
    if ( field )
    {
        newTreeRootTmp = new PdmUiTreeOrdering( field->fieldHandle() );
    }
    else
    {
        ObjectUiCapability* obj = dynamic_cast<ObjectUiCapability*>( pdmRoot );
        if ( obj )
        {
            newTreeRootTmp = new PdmUiTreeOrdering( obj->objectHandle() );
        }
    }

    ObjectUiCapability::expandUiTree( newTreeRootTmp );

#if CAF_PDM_TREE_VIEW_DEBUG_PRINT
    std::cout << std::endl << "New Stuff: " << std::endl;
    newTreeRootTmp->debugDump( 0 );
#endif

    // Find the corresponding entry for "root" in the existing Ui tree

    Wt::WModelIndex existingSubTreeRootModIdx = findModelIndex( pdmRoot );

    PdmUiTreeOrdering* existingSubTreeRoot = nullptr;
    if ( existingSubTreeRootModIdx.isValid() )
    {
        existingSubTreeRoot = treeItemFromIndex( existingSubTreeRootModIdx );
    }
    else
    {
        existingSubTreeRoot = m_treeOrderingRoot;
    }

#if CAF_PDM_TREE_VIEW_DEBUG_PRINT
    std::cout << std::endl << "Old :" << std::endl;
    existingSubTreeRoot->debugDump( 0 );
#endif

    updateSubTreeRecursive( existingSubTreeRootModIdx, existingSubTreeRoot, newTreeRootTmp );

    delete newTreeRootTmp;

    updateEditorsForSubTree( existingSubTreeRoot );

#if CAF_PDM_TREE_VIEW_DEBUG_PRINT
    std::cout << std::endl << "Result :" << std::endl;
    existingSubTreeRoot->debugDump( 0 );
#endif
}

class RecursiveUpdateData
{
public:
    RecursiveUpdateData( int row, PdmUiTreeOrdering* existingChild, PdmUiTreeOrdering* sourceChild )
        : m_row( row )
        , m_existingChild( existingChild )
        , m_sourceChild( sourceChild ){};

    int                m_row;
    PdmUiTreeOrdering* m_existingChild;
    PdmUiTreeOrdering* m_sourceChild;
};

//--------------------------------------------------------------------------------------------------
/// Makes the existingSubTreeRoot tree become identical to the tree in sourceSubTreeRoot,
/// calling begin..() end..() to make the UI update accordingly.
/// This assumes that all the items have a pointer an unique Object
//--------------------------------------------------------------------------------------------------
void PdmWebTreeViewWModel::updateSubTreeRecursive( const Wt::WModelIndex& existingSubTreeRootModIdx,
                                                   PdmUiTreeOrdering*     existingSubTreeRoot,
                                                   PdmUiTreeOrdering*     sourceSubTreeRoot )
{
    // Build map for source items
    std::map<caf::UiItem*, int> sourceTreeMap;
    for ( int i = 0; i < sourceSubTreeRoot->childCount(); ++i )
    {
        PdmUiTreeOrdering* child = sourceSubTreeRoot->child( i );

        if ( child && child->activeItem() )
        {
            sourceTreeMap[child->activeItem()] = i;
        }
    }

    // Detect items to be deleted from existing tree
    std::vector<int> indicesToRemoveFromExisting;
    for ( int i = 0; i < existingSubTreeRoot->childCount(); ++i )
    {
        PdmUiTreeOrdering* child = existingSubTreeRoot->child( i );

        std::map<caf::UiItem*, int>::iterator it = sourceTreeMap.find( child->activeItem() );
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
    std::map<caf::UiItem*, int> existingTreeMap;
    for ( int i = 0; i < existingSubTreeRoot->childCount(); ++i )
    {
        PdmUiTreeOrdering* child = existingSubTreeRoot->child( i );

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
        std::vector<PdmUiTreeOrdering*>  newMergedOrdering;

        layoutAboutToBeChanged().emit();
        {
            // Detect items to be moved from source to existing
            // Merge items from existing and source into newMergedOrdering using order in sourceSubTreeRoot
            std::vector<int> indicesToRemoveFromSource;
            for ( int i = 0; i < sourceSubTreeRoot->childCount(); ++i )
            {
                PdmUiTreeOrdering*                    sourceChild = sourceSubTreeRoot->child( i );
                std::map<caf::UiItem*, int>::iterator it          = existingTreeMap.find( sourceChild->activeItem() );
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
            existingSubTreeRoot->removeChildrenNoDelete( 0, existingSubTreeRoot->childCount() );

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

        layoutChanged().emit();

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
            Wt::WModelIndex mi = index( recursiveUpdateData[i].m_row, 0, existingSubTreeRootModIdx );
            CAF_ASSERT( mi.isValid() );

            updateSubTreeRecursive( mi, recursiveUpdateData[i].m_existingChild, recursiveUpdateData[i].m_sourceChild );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmWebTreeViewWModel::updateEditorsForSubTree( PdmUiTreeOrdering* root )
{
    if ( !root ) return;

    if ( !root->editor() )
    {
        auto treeItemEditor = std::make_unique<PdmWebTreeItemEditor>( root->activeItem() );
        root->setEditor( std::move( treeItemEditor ) );
        CAF_ASSERT( root->editor() );
    }

    PdmWebTreeItemEditor* treeItemEditor = dynamic_cast<PdmWebTreeItemEditor*>( root->editor() );
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
std::list<Wt::WModelIndex> PdmWebTreeViewWModel::allIndicesRecursive( const Wt::WModelIndex& current ) const
{
    std::list<Wt::WModelIndex> currentAndDescendants;
    currentAndDescendants.push_back( current );

    int rows = rowCount( current );
    int cols = columnCount( current );
    for ( int row = 0; row < rows; ++row )
    {
        for ( int col = 0; col < cols; ++col )
        {
            Wt::WModelIndex            childIndex = index( row, col, current );
            std::list<Wt::WModelIndex> subList    = allIndicesRecursive( childIndex );
            currentAndDescendants.insert( currentAndDescendants.end(), subList.begin(), subList.end() );
        }
    }
    return currentAndDescendants;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiTreeOrdering* caf::PdmWebTreeViewWModel::treeItemFromIndex( const Wt::WModelIndex& index ) const
{
    if ( !index.isValid() )
    {
        return m_treeOrderingRoot;
    }

    CAF_ASSERT( index.internalPointer() );

    PdmUiTreeOrdering* treeItem = static_cast<PdmUiTreeOrdering*>( index.internalPointer() );

    return treeItem;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Wt::WModelIndex caf::PdmWebTreeViewWModel::findModelIndex( const UiItem* object ) const
{
    Wt::WModelIndex foundIndex;
    int             numRows = rowCount( Wt::WModelIndex() );
    int             r       = 0;
    while ( r < numRows && !foundIndex.isValid() )
    {
        foundIndex = findModelIndexRecursive( index( r, 0, Wt::WModelIndex() ), object );
        ++r;
    }
    return foundIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Wt::WModelIndex caf::PdmWebTreeViewWModel::findModelIndexRecursive( const Wt::WModelIndex& currentIndex,
                                                                    const UiItem*          pdmItem ) const
{
    if ( currentIndex.internalPointer() )
    {
        PdmUiTreeOrdering* treeItem = static_cast<PdmUiTreeOrdering*>( currentIndex.internalPointer() );
        if ( treeItem->activeItem() == pdmItem ) return currentIndex;
    }

    int row;
    for ( row = 0; row < rowCount( currentIndex ); ++row )
    {
        Wt::WModelIndex foundIndex = findModelIndexRecursive( index( row, 0, currentIndex ), pdmItem );
        if ( foundIndex.isValid() ) return foundIndex;
    }
    return Wt::WModelIndex();
}

//--------------------------------------------------------------------------------------------------
/// An invalid parent index is implicitly meaning the root item, and not "above" root, since
/// we are not showing the root item itself
//--------------------------------------------------------------------------------------------------
Wt::WModelIndex PdmWebTreeViewWModel::index( int row, int column, const Wt::WModelIndex& parentIndex ) const
{
    if ( !m_treeOrderingRoot ) return Wt::WModelIndex();

    PdmUiTreeOrdering* parentItem = nullptr;

    if ( !parentIndex.isValid() )
        parentItem = m_treeOrderingRoot;
    else
        parentItem = static_cast<PdmUiTreeOrdering*>( parentIndex.internalPointer() );

    CAF_ASSERT( parentItem );

    if ( parentItem->childCount() <= row )
    {
        return Wt::WModelIndex();
    }

    PdmUiTreeOrdering* childItem = parentItem->child( row );
    if ( childItem )
        return createIndex( row, column, childItem );
    else
        return Wt::WModelIndex();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Wt::WModelIndex PdmWebTreeViewWModel::parent( const Wt::WModelIndex& childIndex ) const
{
    if ( !childIndex.isValid() ) return Wt::WModelIndex();

    PdmUiTreeOrdering* childItem = static_cast<PdmUiTreeOrdering*>( childIndex.internalPointer() );
    if ( !childItem ) return Wt::WModelIndex();

    PdmUiTreeOrdering* parentItem = childItem->parent();
    if ( !parentItem ) return Wt::WModelIndex();

    if ( parentItem == m_treeOrderingRoot ) return Wt::WModelIndex();

    return createIndex( parentItem->indexInParent(), 0, parentItem );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int PdmWebTreeViewWModel::rowCount( const Wt::WModelIndex& parentIndex ) const
{
    if ( !m_treeOrderingRoot ) return 0;

    if ( parentIndex.column() > 0 ) return 0;

    PdmUiTreeOrdering* parentItem = this->treeItemFromIndex( parentIndex );

    return parentItem->childCount();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int PdmWebTreeViewWModel::columnCount( const Wt::WModelIndex& parentIndex ) const
{
    if ( !m_treeOrderingRoot ) return 0;

    return 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Wt::cpp17::any PdmWebTreeViewWModel::data( const Wt::WModelIndex& index, Wt::ItemDataRole role ) const
{
    if ( !index.isValid() )
    {
        return Wt::cpp17::any();
    }

    PdmUiTreeOrdering* uitreeOrdering = static_cast<PdmUiTreeOrdering*>( index.internalPointer() );
    if ( !uitreeOrdering )
    {
        return Wt::cpp17::any();
    }

    bool isObjRep = uitreeOrdering->isRepresentingObject();

    if ( role == Wt::ItemDataRole::Display && !uitreeOrdering->isValid() )
    {
        std::string str;

#ifdef DEBUG
        str = "Invalid uiordering";
#endif

        return str;
    }

    if ( role == Wt::ItemDataRole::Display || role == Wt::ItemDataRole::Edit )
    {
        if ( isObjRep )
        {
            ObjectHandle* object = uitreeOrdering->object();
            if ( object )
            {
                Variant v;
                if ( object->userDescriptionField() )
                {
                    caf::FieldUiCapability* uiFieldHandle =
                        object->userDescriptionField()->capability<FieldUiCapability>();
                    if ( uiFieldHandle )
                    {
                        v = uiFieldHandle->uiValue();
                    }
                }
                else
                {
                    v = object->classKeywordStatic();
                }

                std::string txt = v.value<std::string>();

                if ( m_treeViewEditor->isAppendOfClassNameToUiItemTextEnabled() )
                {
                    if ( object )
                    {
                        txt += " - ";
                        txt += typeid( *object ).name();
                    }
                }

                return txt;
            }
            else
            {
                return Wt::cpp17::any();
            }
        }

        if ( uitreeOrdering->activeItem() )
        {
            return uitreeOrdering->activeItem()->uiName();
        }
        else
        {
            return Wt::cpp17::any();
        }
    }
    else if ( role == Wt::ItemDataRole::Decoration )
    {
        if ( uitreeOrdering->activeItem() && uitreeOrdering->activeItem()->uiIconProvider() )
        {
            return uitreeOrdering->activeItem()->uiIconProvider()->iconResourceString();
        }
        else
        {
            return Wt::cpp17::any();
        }
    }
    else if ( role == Wt::ItemDataRole::ToolTip )
    {
        if ( uitreeOrdering->activeItem() )
        {
            return uitreeOrdering->activeItem()->uiToolTip();
        }
        else
        {
            return Wt::cpp17::any();
        }
    }
    else if ( role == Wt::ItemDataRole::User )
    {
        if ( uitreeOrdering->activeItem() )
        {
            return uitreeOrdering->activeItem()->uiWhatsThis();
        }
        else
        {
            return Wt::cpp17::any();
        }
    }
    else if ( role == Wt::ItemDataRole::Checked )
    {
        if ( isObjRep )
        {
            auto object = uitreeOrdering->object();
            if ( object && object->objectToggleField() )
            {
                caf::FieldUiCapability* uiFieldHandle = object->objectToggleField()->capability<FieldUiCapability>();
                if ( uiFieldHandle )
                {
                    bool isToggledOn = uiFieldHandle->uiValue().value<bool>();
                    return Wt::cpp17::any( isToggledOn );
                }
                else
                {
                    return Wt::cpp17::any();
                }
            }
        }
    }

    return Wt::cpp17::any();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmWebTreeViewWModel::setData( const Wt::WModelIndex& index,
                                    const Wt::cpp17::any&  value,
                                    Wt::ItemDataRole       role /*= Wt::ItemDataRole::Edit*/ )
{
    if ( !index.isValid() )
    {
        return false;
    }

    PdmUiTreeOrdering* treeItem = PdmWebTreeViewWModel::treeItemFromIndex( index );
    CAF_ASSERT( treeItem );

    if ( !treeItem->isRepresentingObject() ) return false;

    auto object = treeItem->object();
    if ( object )
    {
        if ( role == Wt::ItemDataRole::Edit && object->userDescriptionField() )
        {
            FieldUiCapability* userDescriptionUiField = object->userDescriptionField()->capability<FieldUiCapability>();
            if ( userDescriptionUiField )
            {
                Wt::WString text = Wt::cpp17::any_cast<Wt::WString>( value );
                Variant     value( text.narrow() );
                UiCommandSystemProxy::instance()->setUiValueToField( userDescriptionUiField, value );
            }

            return true;
        }
        else if ( role == Wt::ItemDataRole::Checked && object->objectToggleField() &&
                  !object->objectToggleField()->capability<FieldUiCapability>()->isUiReadOnly() )
        {
            bool toggleOn = Wt::cpp17::any_cast<bool>( value );

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
/// Enable edit of this item if we have a editable user description field for a pdmObject
/// Disable edit for other items
//--------------------------------------------------------------------------------------------------
Wt::WFlags<Wt::ItemFlag> PdmWebTreeViewWModel::flags( const Wt::WModelIndex& index ) const
{
    if ( !index.isValid() )
    {
        return Wt::WFlags<Wt::ItemFlag>();
    }

    Wt::WFlags<Wt::ItemFlag> flagMask = Wt::WAbstractItemModel::flags( index );

    PdmUiTreeOrdering* treeItem = treeItemFromIndex( index );
    CAF_ASSERT( treeItem );

    if ( treeItem->isRepresentingObject() )
    {
        ObjectHandle* object = treeItem->object();
        if ( object )
        {
            flagMask |= Wt::ItemFlag::Selectable;
            if ( object->userDescriptionField() &&
                 !object->userDescriptionField()->capability<FieldUiCapability>()->isUiReadOnly() )
            {
                flagMask = flagMask | Wt::ItemFlag::Editable;
            }

            if ( object->objectToggleField() )
            {
                flagMask = flagMask | Wt::ItemFlag::UserCheckable;
            }
        }
    }

    if ( treeItem->isValid() )
    {
        if ( treeItem->activeItem()->isUiReadOnly() )
        {
            flagMask = flagMask & ~Wt::WFlags<Wt::ItemFlag>( Wt::ItemFlag::Editable );
        }
    }

    /* if (m_dragDropInterface)
    {
        Wt::ItemFlags dragDropFlags = m_dragDropInterface->flags(index);
        flagMask |= dragDropFlags;
    } */

    return flagMask;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Wt::cpp17::any PdmWebTreeViewWModel::headerData( int              section,
                                                 Wt::Orientation  orientation,
                                                 Wt::ItemDataRole role /*= Wt::ItemDataRole::Display */ ) const
{
    if ( role != Wt::ItemDataRole::Display ) return Wt::cpp17::any();

    if ( section < m_columnHeaders.size() )
    {
        return m_columnHeaders[section];
    }

    return Wt::cpp17::any();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
UiItem* PdmWebTreeViewWModel::uiItemFromModelIndex( const Wt::WModelIndex& index ) const
{
    PdmUiTreeOrdering* treeItem = this->treeItemFromIndex( index );
    if ( treeItem )
    {
        return treeItem->activeItem();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
/* void PdmWebTreeViewWModel::setDragDropInterface( PdmUiDragDropInterface* dragDropInterface )
{
    m_dragDropInterface = dragDropInterface;
} */

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
/* PdmUiDragDropInterface* PdmWebTreeViewWModel::dragDropInterface()
{
    return m_dragDropInterface;
} */

#ifdef _MSC_VER
#pragma warning( pop )
#endif
