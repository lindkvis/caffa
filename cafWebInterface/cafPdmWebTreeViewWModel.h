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
#pragma once

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 4267 4275 4564 )
#endif

#include "cafUiTreeItem.h"

#include <Wt/WAbstractItemModel.h>

#include <string>
#include <vector>

namespace caf
{
class ObjectHandle;
class UiItem;
class PdmWebTreeViewEditor;
class PdmUiTreeOrdering;

//==================================================================================================
//
// This class is intended to replace UiTreeModelPdm (cafUiTreeModelPdm)
//
//==================================================================================================
class PdmWebTreeViewWModel : public Wt::WAbstractItemModel
{
public:
    explicit PdmWebTreeViewWModel( PdmWebTreeViewEditor* treeViewEditor );

    void setPdmItemRoot( UiItem* rootItem );
    void updateSubTree( UiItem* subTreeRoot );

    void setColumnHeaders( const std::vector<std::string>& columnHeaders );

    // These are supposed to be used from the Editor only, and to implement selection support.

    UiItem*         uiItemFromModelIndex( const Wt::WModelIndex& index ) const;
    Wt::WModelIndex findModelIndex( const UiItem* object ) const;

    // void                    setDragDropInterface( PdmUiDragDropInterface* dragDropInterface );
    // PdmUiDragDropInterface* dragDropInterface();

    std::list<Wt::WModelIndex> allIndicesRecursive( const Wt::WModelIndex& current = Wt::WModelIndex() ) const;

private:
    void updateSubTreeRecursive( const Wt::WModelIndex& uiSubTreeRootModelIdx,
                                 PdmUiTreeOrdering*     uiModelSubTreeRoot,
                                 PdmUiTreeOrdering*     updatedPdmSubTreeRoot );

    PdmUiTreeOrdering* treeItemFromIndex( const Wt::WModelIndex& index ) const;
    Wt::WModelIndex    findModelIndexRecursive( const Wt::WModelIndex& currentIndex, const UiItem* object ) const;

    void resetTree( PdmUiTreeOrdering* root );
    void emitDataChanged( const Wt::WModelIndex& index );
    void updateEditorsForSubTree( PdmUiTreeOrdering* root );

    PdmUiTreeOrdering*       m_treeOrderingRoot;
    std::vector<std::string> m_columnHeaders;

    PdmWebTreeViewEditor* m_treeViewEditor;

    //    PdmUiDragDropInterface* m_dragDropInterface;

private:
    // Overrides from Wt::WAbstractItemModel

    Wt::WModelIndex index( int row, int column, const Wt::WModelIndex& parentIndex = Wt::WModelIndex() ) const override;
    Wt::WModelIndex parent( const Wt::WModelIndex& index ) const override;

    int rowCount( const Wt::WModelIndex& parentIndex = Wt::WModelIndex() ) const override;
    int columnCount( const Wt::WModelIndex& parentIndex = Wt::WModelIndex() ) const override;

    Wt::cpp17::any data( const Wt::WModelIndex& index, Wt::ItemDataRole role = Wt::ItemDataRole::Display ) const override;
    bool           setData( const Wt::WModelIndex& index,
                            const Wt::cpp17::any&  value,
                            Wt::ItemDataRole       role = Wt::ItemDataRole::Edit ) override;
    Wt::cpp17::any headerData( int              section,
                               Wt::Orientation  orientation,
                               Wt::ItemDataRole role = Wt::ItemDataRole::Display ) const override;

    Wt::WFlags<Wt::ItemFlag> flags( const Wt::WModelIndex& index ) const override;

    /*    std::vector<std::string> acceptDropMimeTypes() const override;
        QMimeData*      mimeData(const Wt::WModelIndexList &indexes) const override;
        bool            dropEvent(const Wt::MimeData *data, Qt::DropAction action, int row, int column, const
       Wt::WModelIndex &parent) override; Qt::DropActions supportedDropActions() const override; */
};

} // End of namespace caf

#ifdef _MSC_VER
#pragma warning( pop )
#endif
