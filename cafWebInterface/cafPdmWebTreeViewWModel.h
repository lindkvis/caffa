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

#include "cafUiTreeItem.h"

#include <Wt/WAbstractItemModel.h>
#include <QStringList>

#include <string>
#include <vector>

namespace caf
{

class PdmObjectHandle;
class PdmUiItem;
class PdmWebTreeViewEditor;
class PdmUiTreeOrdering;
class PdmUiDragDropInterface;

//==================================================================================================
//
// This class is intended to replace UiTreeModelPdm (cafUiTreeModelPdm)
//
//==================================================================================================
class PdmWebTreeViewWModel : public Wt::WAbstractItemModel
{
public:
    explicit PdmWebTreeViewWModel(PdmWebTreeViewEditor* treeViewEditor);

    void                    setPdmItemRoot(PdmUiItem* rootItem);
    void                    updateSubTree(PdmUiItem* subTreeRoot);

    void                    setColumnHeaders(const QStringList& columnHeaders);
    void                    setUiConfigName(const QString& uiConfigName) { m_uiConfigName = uiConfigName; }
 
    // These are supposed to be used from the Editor only, and to implement selection support.
 
    PdmUiItem*              uiItemFromModelIndex(const Wt::WModelIndex& index) const;
    Wt::WModelIndex         findModelIndex(const PdmUiItem* object) const;

    void                    setDragDropInterface(PdmUiDragDropInterface* dragDropInterface);
    PdmUiDragDropInterface* dragDropInterface();

    std::list<Wt::WModelIndex>  allIndicesRecursive(const Wt::WModelIndex& current = Wt::WModelIndex()) const;
    
private:
    void                    updateSubTreeRecursive(const Wt::WModelIndex& uiSubTreeRootModelIdx, PdmUiTreeOrdering* uiModelSubTreeRoot, PdmUiTreeOrdering* updatedPdmSubTreeRoot);

    PdmUiTreeOrdering*      treeItemFromIndex(const Wt::WModelIndex& index) const;
    Wt::WModelIndex         findModelIndexRecursive(const Wt::WModelIndex& currentIndex, const PdmUiItem * object) const;

    void                    resetTree(PdmUiTreeOrdering* root);
    void                    emitDataChanged(const Wt::WModelIndex& index);
    void                    updateEditorsForSubTree(PdmUiTreeOrdering* root);

    PdmUiTreeOrdering*      m_treeOrderingRoot;
    QStringList             m_columnHeaders;
    QString                 m_uiConfigName;

    PdmWebTreeViewEditor*    m_treeViewEditor;

    PdmUiDragDropInterface* m_dragDropInterface;

private:

    // Overrides from Wt::WAbstractItemModel

    Wt::WModelIndex index(int row, int column, const Wt::WModelIndex &parentIndex = Wt::WModelIndex( )) const override;
    Wt::WModelIndex parent(const Wt::WModelIndex &index) const override;

    int             rowCount(const Wt::WModelIndex &parentIndex = Wt::WModelIndex( ) ) const override;
    int             columnCount(const Wt::WModelIndex &parentIndex = Wt::WModelIndex( ) ) const override;

    Wt::cpp17::any  data(const Wt::WModelIndex &index, Wt::ItemDataRole role = Wt::ItemDataRole::Display ) const override;
    bool            setData(const Wt::WModelIndex &index, const Wt::cpp17::any &value, Wt::ItemDataRole role = Wt::ItemDataRole::Edit) override;
    Wt::cpp17::any  headerData(int section, Wt::Orientation orientation, Wt::ItemDataRole role = Wt::ItemDataRole::Display) const override;

    Wt::WFlags<Wt::ItemFlag>   flags(const Wt::WModelIndex &index) const override;

/*    std::vector<std::string> acceptDropMimeTypes() const override;
    QMimeData*      mimeData(const Wt::WModelIndexList &indexes) const override;
    bool            dropEvent(const Wt::MimeData *data, Qt::DropAction action, int row, int column, const Wt::WModelIndex &parent) override;
    Qt::DropActions supportedDropActions() const override; */
};



} // End of namespace caf
